#include "lexer.hpp"

#include <any>
#include <memory>
#include <print>
#include <string>

#include <eval.hpp>
#include <ranges>

bool is_numeric(const std::any &x) {
  return x.type() == typeid(int) || x.type() == typeid(float) ||
         x.type() == typeid(char);
}

bool is_string(const std::any &x) {
  return x.type() == typeid(std::string) || x.type() == typeid(std::wstring) ||
         x.type() == typeid(const char *) || x.type() == typeid(char *) ||
         x.type() == typeid(wchar_t *);
}

bool try_eval(const std::any &x, const std::any &y) {
  return (is_numeric(x) && is_numeric(y)) || (is_string(x) && is_string(y));
}

std::any eval_numeric(const std::any &x, const std::any &y,
                      const BinOpType op) {
  std::any result;
  if (x.type() == typeid(int) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(float))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<float>(y), op);
  if (x.type() == typeid(float) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<float>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(float) && y.type() == typeid(float))
    return eval_numeric_op(std::any_cast<float>(x), std::any_cast<float>(y),
                           op);
  if (x.type() == typeid(char) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<char>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(char))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<char>(y), op);
  if (x.type() == typeid(char) && y.type() == typeid(float))
    return eval_numeric_op(std::any_cast<char>(x), std::any_cast<float>(y),
                           op);
  if (x.type() == typeid(float) && y.type() == typeid(char))
    return eval_numeric_op(std::any_cast<float>(x), std::any_cast<char>(y),
                           op);
  if (x.type() == typeid(bool) && y.type() == typeid(bool))
    return eval_numeric_op(std::any_cast<bool>(x), std::any_cast<bool>(y), op);
  if (x.type() == typeid(bool) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<bool>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(bool))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<bool>(y), op);
  return result;
}

std::any eval_binop(const std::any &x, const std::any &y, BinOpType op) {
  std::any result;
  if (is_numeric(x)) {
    result = eval_numeric(x, y, op);
  } else if (x.type() == typeid(std::string)) {
    result = std::any_cast<std::string>(x) + std::any_cast<std::string>(y);
  }

  return result;
}

void print_any(std::any x) {
  if (x.type() == typeid(int)) {
    std::println("{}", std::any_cast<int>(x));
  } else if (x.type() == typeid(double)) {
    std::println("{}", std::any_cast<double>(x));
  } else if (x.type() == typeid(char)) {
    std::println("{}", std::any_cast<char>(x));
  } else if (x.type() == typeid(std::string)) {
    std::println("{}", std::any_cast<std::string>(x));
  }
}

std::string interpolate_string(std::string literal) {
  std::string str;

  for (int i = 0; i < literal.size(); i++) {
    char c = literal[i];
    if (c == '$') {
      std::string var_name;
      c = literal[++i];
      lexer::Token tt;
      std::string tstr = {c};
      while (c != ' ' && c != '\\' && c != '\n'&& c != '\0' && i < literal.size() && !lexer::Lexer::any_token(tstr, tt)) {
        var_name += c;
        c = literal[++i];
        tstr = {c};
      }
      std::any value;
      if (State::vars.contains(var_name)) {
        value = eval(State::vars[var_name]);
      } else if (State::scope_variables.contains(var_name)) {
        value = eval(State::scope_variables[var_name]);
      }

      if (value.type() == typeid(int)) {
        str += std::format("{}", std::any_cast<int>(value));
      }
      if (value.type() == typeid(float)) {
        str += std::format("{}", std::any_cast<float>(value));
      }
      if (c != '\0')
        str += c;
      continue;
    }
    str += c;
  }
  return str;
}

std::any eval(const std::shared_ptr<Node> &node) {
  std::any ret_value;
  switch (node->type) {
  case NodeType::ROOT_NODE:
    for (const auto &n : node->body)
      eval(n);
    break;
  case NodeType::FUNCTION_CALL: {
    auto prev_vars = State::scope_variables;
    State::scope_variables.clear();
    int arg_count = node->function.arguments.size();
    size_t count = 0;
    auto n = node->body.begin();
    for (const auto &name : node->function.arguments | std::views::keys) {
      if (count >= arg_count || (*n)->type != NodeType::FUNCTION_CALL_PARAM)
        break;
      State::scope_variables[name] = std::make_shared<Node>();
      State::scope_variables[name]->value = eval((*n)->body.front());
      State::scope_variables[name]->type = NodeType::IDENTIFIER;
      State::scope_variables[name]->name = name;
      std::advance(n, 1);
      count++;
    }

    if (count != arg_count) {
      std::println("[ERROR] when calling function {}: parameter count missmatch", node->function.name);
      exit(1);
    }

    ret_value = eval(*n);

    State::scope_variables = prev_vars;

    //if (ret_value.has_value())
    //  print_any(ret_value);
    return ret_value;
  } break;
  case NodeType::BINOP:
    switch (node->binop_type) {
    case BinOpType::ASSIGNMENT:
      node->left->value = eval(node->right);
      return node->left->value;
    default:
      return eval_binop(eval(node->left), eval(node->right), node->binop_type);
    }
  case NodeType::FUNCTION_BODY:
    for (auto &n : node->body) {
      ret_value = eval(n);
      if (n->type == NodeType::RETURN)
        break;
    }
    if (node->function.return_type == VOID)
      ret_value.reset();
    return ret_value;

  case NodeType::LOOP_BODY:
    for (auto &n : node->body) {
      ret_value = eval(n);
    }
    if (node->function.return_type == VOID)
      ret_value.reset();
    return ret_value;
  case NodeType::VARIABLE_DECLARATION:
    for (const auto &n : node->body)
      node->value = eval(n);
    break;
  case NodeType::LITERAL:
    if (node->value.type() == typeid(int))
      return std::any_cast<int>(node->value);
    if (node->value.type() == typeid(float))
      return std::any_cast<float>(node->value);
    if (node->value.type() == typeid(std::string))
      return std::any_cast<std::string>(node->value);
  case NodeType::RETURN:
    for (const auto &n : node->body)
      node->value = eval(n);
    return node->value;
  case NodeType::IDENTIFIER:
    if (State::scope_variables.contains(node->name)) {
      node->value = State::scope_variables[node->name]->value;
      return State::scope_variables[node->name]->value;
    }
    if (State::vars.contains(node->name)) {
      node->value = State::vars[node->name]->value;
      return State::vars[node->name]->value;
    }
  case NodeType::EXPRESSION:
    for (const auto &n : node->body)
      return eval(n);
    break;
  case NodeType::PRINT:
    for (const auto &arg : node->body) {
      if (arg->type == NodeType::LITERAL) {

        auto literal = std::any_cast<std::string>(arg->value);
        std::println("{}", interpolate_string(literal));
      }
      else if (arg->type == NodeType::IDENTIFIER) {
        std::string literal = std::any_cast<std::string>(
                      std::any_cast<std::shared_ptr<Node>>(arg->value)->value);
        std::println(
            "{}", interpolate_string(literal));
      }
    }
    break;

  case NodeType::LOOP_DECLARATION: {
    int loops = std::any_cast<int>(eval(node->condition));
    State::scope_variables["_index"];
    State::scope_variables["_index"] = std::make_shared<Node>();
    State::scope_variables["_index"]->type = NodeType::LITERAL;
    State::scope_variables["_index"]->name = "_index";
    for (int i = 0; i < loops; i++) {
      State::scope_variables["_index"]->value = i;
      ret_value = eval(node->body.front());
    }
    return ret_value;
  }
  default:
    break;
  }
  return 0;
}
