#pragma once
#include <any>
#include <list>
#include <memory>
#include <string>

#include "definitions.hpp"

static std::unordered_map<std::string, std::shared_ptr<Node>> vars;
static std::unordered_map<std::string, std::shared_ptr<Node>> functions;

static bool is_numeric(const std::any &x) {
  return x.type() == typeid(int) || x.type() == typeid(double) ||
         x.type() == typeid(char);
}

static bool is_string(const std::any &x) {
  return x.type() == typeid(std::string) || x.type() == typeid(std::wstring) ||
         x.type() == typeid(const char *) || x.type() == typeid(char *) ||
         x.type() == typeid(wchar_t *);
}

static bool try_eval(const std::any &x, const std::any &y) {
  return (is_numeric(x) && is_numeric(y)) || (is_string(x) && is_string(y));
}

template <typename X, typename Y>
static std::common_type_t<X, Y> eval_numeric_op(X x, Y y, const BinOpType op) {
  switch (op) {
  case BinOpType::PLUS:
    return x + y;
  case BinOpType::MINUS:
    return x - y;
  case BinOpType::MUL:
    return x * y;
  case BinOpType::DIV:
    return x / y;
  case BinOpType::AND:
    return x && y;
  case BinOpType::EQUALS:
    return x == y;
  case BinOpType::LESS_EQUAL:
    return x <= y;
  case BinOpType::MORE_EQUALS:
    return x >= y;
  case BinOpType::OR:
    return x || y;
  case BinOpType::NOT_EQUALS:
    return x != y;
  default:
    break;
  }
  return 0;
}

static std::any eval_numeric(const std::any &x, const std::any &y,
                             const BinOpType op) {
  std::any result;
  if (x.type() == typeid(int) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(double))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<double>(y), op);
  if (x.type() == typeid(double) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<double>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(double) && y.type() == typeid(double))
    return eval_numeric_op(std::any_cast<double>(x), std::any_cast<double>(y),
                           op);
  if (x.type() == typeid(char) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<char>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(char))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<char>(y), op);
  if (x.type() == typeid(char) && y.type() == typeid(double))
    return eval_numeric_op(std::any_cast<char>(x), std::any_cast<double>(y),
                           op);
  if (x.type() == typeid(double) && y.type() == typeid(char))
    return eval_numeric_op(std::any_cast<double>(x), std::any_cast<char>(y),
                           op);
  if (x.type() == typeid(bool) && y.type() == typeid(bool))
    return eval_numeric_op(std::any_cast<bool>(x), std::any_cast<bool>(y), op);
  if (x.type() == typeid(bool) && y.type() == typeid(int))
    return eval_numeric_op(std::any_cast<bool>(x), std::any_cast<int>(y), op);
  if (x.type() == typeid(int) && y.type() == typeid(bool))
    return eval_numeric_op(std::any_cast<int>(x), std::any_cast<bool>(y), op);
  return result;
}

static std::any eval_binop(const std::any &x, const std::any &y, BinOpType op) {
  std::any result;
  if (is_numeric(x)) {
    result = eval_numeric(x, y, op);
  } else if (x.type() == typeid(std::string)) {
    result = std::any_cast<std::string>(x) + std::any_cast<std::string>(y);
  }

  return result;
}

static void print_any(std::any x) {
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

static std::any eval(const std::shared_ptr<Node> &node) {
  std::any ret_value;
  switch (node->type) {
  case NodeType::ROOT_NODE:
    for (const auto &n : node->body)
      eval(n);
    break;
  case NodeType::FUNCTION_CALL:
    for (const auto &n : node->body)
      ret_value = eval(n);
    if (ret_value.has_value())
      print_any(ret_value);
    break;
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
    if (node->function.return_type == VOID) ret_value.reset();
    return ret_value;
  case NodeType::VARIABLE_DECLARATION:
    for (const auto &n : node->body)
      node->value = eval(n);
    break;
  case NodeType::LITERAL:
    return std::any_cast<int>(node->value);
  case NodeType::RETURN:
    for (const auto &n : node->body)
      node->value = eval(n);
    return node->value;
  case NodeType::IDENTIFIER:
    node->value = vars[node->name]->value;
    return vars[node->name]->value;
  case NodeType::EXPRESSION:
    for (const auto &n : node->body)
      return eval(n);
    break;
  case NodeType::PRINT:
    std::print("{}", std::any_cast<int>(node->value));
    break;
  default:
    break;
  }
  return 0;
}
