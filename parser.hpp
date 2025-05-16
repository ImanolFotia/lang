#pragma once
#include "definitions.hpp"
#include "eval.hpp"
#include "lexer.hpp"
#include "logging.hpp"

#include <cstring>

static bool is_any_type(const lexer::TokenType t) {
  return t == lexer::type_int || t == lexer::type_bool || t == lexer::type_void || t == lexer::type_float ||
         t == lexer::type_string || t == lexer::type_char;
}

static Type get_type(const lexer::Token &t) {
  if (t.type == lexer::type_int)
    return INT;
  if (t.type == lexer::type_float)
    return FLOAT;
  if (t.type == lexer::type_string)
    return STRING;
  if (t.type == lexer::type_bool)
    return BOOL;
  if (t.type == lexer::type_char)
    return CHAR;
  if (t.type == lexer::type_void)
    return VOID;
  return UNKNOWN;
}

static Function parse_function_header(lexer::Lexer &l) {
  Function fun;
  l.next();
  if (l.expect(lexer::id)) {
    fun.name = l.get().str;
    if (functions.contains(fun.name)) {
      std::println("Function {} already exists!", fun.name);
      exit(255);
    }
    l.next();
    if (l.expect(lexer::assign)) {
      l.next();
      if (l.expect(lexer::open_paren)) {
        while (l.next().type != lexer::close_paren) {
          if (is_any_type(l.get().type)) {
            const auto type = get_type(l.get());
            l.next();
            if (l.expect(lexer::id)) {
              fun.arguments[l.get().str] = type;
            } else {
              logging::expected_error(l.get(), "identifier");
            }
          }
          if (l.get().type == lexer::close_brace || l.get().type == lexer::arrow) {
            logging::expected_error(l.get(), ")");
          }
        }
        if (l.expect(lexer::arrow)) {
          l.next();
          if (is_any_type(l.get().type)) {
            fun.return_type = get_type(l.get());
            l.next();
            if (l.expect(lexer::open_brace)) {

            } else {
              logging::expected_error(l.get(), "(");
            }
          } else {
            fun.single_expression = true;
          }
        } else {
          logging::expected_error(l.get(), "->");
        }
      } else {
        logging::expected_error(l.get(), "(");
      }

    } else {
      logging::expected_error(l.get(), "=");
    }

  } else {
    logging::expected_error(l.get(), "identifier");
  }

  return fun;
}

static void parse_expression(lexer::Lexer &l, const std::shared_ptr<Node> &node) {
  if (l.parsed_tokens.empty())
    return;
  while (!l.expect(lexer::semicolon) && !l.done()) {
    switch (l.get().type) {
    case lexer::int_literal:
      node->type = NodeType::LITERAL;
      errno = 0;
      node->value = static_cast<int>(strtoll(l.get().str.c_str(), nullptr, 10));
      if (errno == ERANGE) {
        std::println("error: {}", strerror(errno));
      }
      if (l.parsed_tokens.size() <= 1)
        return;
      break;
    case lexer::minus:
      if (l.get().type == lexer::minus)
        node->binop_type = BinOpType::MINUS;
    case lexer::divide:
      if (l.get().type == lexer::divide)
      node->binop_type = BinOpType::DIV;
    case lexer::multiply:
      if (l.get().type == lexer::multiply)
      node->binop_type = BinOpType::MUL;
    case lexer::plus: {
      if (l.get().type == lexer::plus)
        node->binop_type = BinOpType::PLUS;
      node->type = NodeType::BINOP;
      node->left = std::make_shared<Node>();
      node->right = std::make_shared<Node>();
      auto ll = l.slice(0, l.current_token);
      auto rl = l.slice(l.current_token + 1, l.parsed_tokens.size());
      parse_expression(ll, node->left);
      parse_expression(rl, node->right);

      if (node->right->type == NodeType::NONE)
        logging::expected_error(l.look_ahead(1),
                                "identifier, literal or expression");
      if (node->left->type == NodeType::NONE)
        logging::expected_error(l.look_ahead(-1),
                                "identifier, literal or expression");
      return;
    }
    case lexer::id: {
      if (!vars.contains(l.get().str)) {
        std::println("[ERROR] undeclared variable {}", l.get().str);
        exit(1);
      }
      node->value = strtol(l.get().str.c_str(), nullptr, 10);
      node->type = NodeType::IDENTIFIER;
      node->name = l.get().str;
      if (l.parsed_tokens.size() <= 1)
        return;
      break;
    }
    case lexer::type_int:
    case lexer::close_brace:
      logging::expected_error(l.get(), ";");
      break;
    default:
      break;
    }
    l.next();
  }
}

static std::shared_ptr<Node> create_expression(lexer::Lexer &l,
                                        const std::shared_ptr<Node> &node) {
  const auto expr = node->append(NodeType::EXPRESSION);
  if (const auto next_semicolon = l.find_next(lexer::semicolon);
      next_semicolon == std::nullopt) {
    logging::expected_error(l.get(), ";");
  }
  return expr->append(NodeType::EXPRESSION);
}

static void generate_expression(lexer::Lexer &l, std::shared_ptr<Node> node) {
  while (!l.expect(lexer::eof)) {
    switch (l.get().type) {
    case lexer::fn: {
      Function fun = parse_function_header(l);
      auto func_decl = node->append(NodeType::FUNCTION_DECLARATION);
      func_decl->function = fun;
      func_decl->name = fun.name;
      func_decl->parent = node;
      auto func_body = func_decl->append(NodeType::FUNCTION_BODY);
      func_body->name = fun.name + "_body";
      func_body->parent = func_decl;
      func_body->function = fun;
      functions[fun.name] = func_body;
      if (fun.single_expression) {
        auto expr_body = create_expression(l, func_body);
        auto next_semicolon = l.find_next(lexer::semicolon);
        auto nl = l.slice(l.current_token, next_semicolon.value() + 1);
        parse_expression(nl, expr_body);
        l.advance(next_semicolon.value());
      } else {
        node = func_body;
      }
      break;
    }
    case lexer::int_literal: {
      auto lit = node->append(NodeType::LITERAL);
      lit->value = static_cast<int>(strtol(l.get().str.c_str(), nullptr, 10));
      break;
    }
    case lexer::id: {
      if (vars.contains(l.get().str)) {
        std::string var_name = l.get().str;
        l.next();
        if (l.expect(lexer::assign)) {
          auto expr_body = create_expression(l, node);
          auto next_semicolon = l.find_next(lexer::semicolon);
          auto nl = l.slice(l.current_token - 1, next_semicolon.value() + 1);
          auto expr = expr_body->append(NodeType::BINOP);
          expr->binop_type = BinOpType::ASSIGNMENT;
          expr->left = vars[var_name];
          expr->left->name = var_name;
          expr->right = std::make_shared<Node>();
          parse_expression(nl, expr->right);
          l.advance(next_semicolon.value());
        } else {
          if (l.expect(lexer::semicolon)) {
            auto &single_expr = node->body.emplace_back();
            single_expr = vars[var_name];
            break;
          }
          auto expr_body = create_expression(l, node);
          auto next_semicolon = l.find_next(lexer::semicolon);
          auto nl = l.slice(l.current_token - 1, next_semicolon.value() + 1);

          parse_expression(nl, expr_body);
          l.advance(next_semicolon.value());
        }
      } else if (functions.contains(l.get().str)) {
        std::string func_name = l.get().str;
        l.next();
        if (l.next().type == lexer::open_paren && l.next().type == lexer::close_paren) {
          auto func_call_node = node->append(NodeType::FUNCTION_CALL);
          func_call_node->name = func_name;
          func_call_node->body.push_back(functions[func_name]);
        }
        if (!l.expect(lexer::semicolon)) {
          logging::expected_error(l.get(), ";");
        }
        break;
      } else {
        logging::expected_error(l.get(), "identifier");
      }
      break;
    }
    case lexer::type_int: {
      l.next();
      if (l.expect(lexer::id)) {
        std::string id_name = l.get().str;

        l.next();
        auto new_node = node->append(NodeType::VARIABLE_DECLARATION);
        new_node->name = id_name;

        auto var = new_node->append(NodeType::IDENTIFIER);

        if (vars.contains(id_name)) {
          std::println("[ERROR] variable {} is already declared.", id_name);
          exit(1);
        }
        vars[id_name] = var;
        if (l.expect(lexer::assign)) {
          l.next();
          if (l.expect(lexer::semicolon))
            logging::expected_error(l.get(),
                                    "identifier, literal or expression");
          auto next_semicolon = l.find_next(lexer::semicolon);
          if (next_semicolon == std::nullopt) {
            logging::expected_error(l.get(), ";");
          }
          auto nl = l.slice(l.current_token, next_semicolon.value() + 1);
          auto expr = new_node->append(NodeType::BINOP);
          expr->binop_type = BinOpType::ASSIGNMENT;
          expr->left = vars[id_name];
          expr->left->name = id_name;
          expr->right = std::make_shared<Node>();
          parse_expression(nl, expr->right);
          l.advance(next_semicolon.value());
        } else {
        }
      }
      break;
    }
    case lexer::close_brace: {
      node = node->parent;
      node = node->parent;
      break;
    }
    case lexer::ret: {
      auto ret_node = node->append(NodeType::RETURN);
      l.next();
      if (l.expect(lexer::semicolon)) {
        logging::expected_error(l.get(), "value, identifier or expression");
      }
      auto expr_body = create_expression(l, ret_node);
      auto next_semicolon = l.find_next(lexer::semicolon);
      auto nl = l.slice(l.current_token, next_semicolon.value() + 1);
      parse_expression(nl, expr_body);
      l.advance(next_semicolon.value());
    }
    default:
      break;
    }
    l.next();
  }
}
