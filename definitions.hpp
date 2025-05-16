#pragma once
#include <any>
#include <list>
#include <memory>
#include <unordered_map>

enum Type : int { INT = 0, FLOAT, STRING, BOOL, CHAR, VOID, UNKNOWN };

const std::string TypeNames[] = {"INT",  "FLOAT", "STRING", "BOOL",
                                 "CHAR", "VOID",  "UNKNOWN"};

struct Variable {
  std::string name;
  Type type;
  std::any value;
};


enum class NodeType {
  ROOT_NODE,
  FUNCTION_DECLARATION,
  ASSIGNMENT,
  IDENTIFIER,
  FUNCTION_CALL,
  FUNCTION_BODY,
  VARIABLE_DECLARATION,
  VARIABLE_ASSIGNMENT,
  RETURN,
  BINOP,
  LITERAL,
  EXPRESSION,
  PRINT,
  NONE
};

enum class BinOpType {
  PLUS,
  MINUS,
  MUL,
  DIV,
  ASSIGNMENT,
  EQUALS,
  LESS_THAN,
  MORE_THAN,
  MORE_EQUALS,
  LESS_EQUAL,
  OR,
  AND,
  XOR,
  NAND,
  NONE,
  NOT_EQUALS
};

struct Function {
  std::string name;
  Type return_type = INT;
  std::unordered_map<std::string, Type> arguments;
  bool single_expression = false;
};

struct Node {
  std::shared_ptr<Node> left = nullptr;
  std::shared_ptr<Node> right = nullptr;
  std::list<std::shared_ptr<Node>> body;
  std::shared_ptr<Node> condition = nullptr;
  std::shared_ptr<Node> parent = nullptr;
  std::any value = 0;
  Type value_type = UNKNOWN;
  NodeType type = NodeType::NONE;
  BinOpType binop_type = BinOpType::NONE;
  std::string name{};
  Function function;
  std::any return_value;
  bool expression = false;

  std::shared_ptr<Node> append(const NodeType type) {
    auto &node = body.emplace_back();
    node = std::make_shared<Node>();
    node->type = type;
    return node;
  }
};
