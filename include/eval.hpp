#pragma once
#include <any>
#include <definitions.hpp>
#include <memory>

bool is_numeric(const std::any &x);

bool is_string(const std::any &x);

bool try_eval(const std::any &x, const std::any &y);

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

std::any eval_numeric(const std::any &x, const std::any &y, const BinOpType op);

std::any eval_binop(const std::any &x, const std::any &y, BinOpType op);

void print_any(std::any x);

std::any eval(const std::shared_ptr<Node> &node);
