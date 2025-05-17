#pragma once
#include <chrono>
#include <fstream>
#include <any>
#include <print>

#include "utils.hpp"

#include <algorithm>

namespace lexer {
enum TokenType : int {
  id = 0,
  assign,
  plus,
  minus,
  divide,
  multiply,
  open_paren,
  close_paren,
  open_brace,
  close_brace,
  ret,
  int_literal,
  float_literal,
  string_literal,
  unknown,
  eof,
  fn,
  arrow,
  type_int,
  type_float,
  type_string,
  type_char,
  type_bool,
  type_void,
  semicolon,
  comma,
  print,
  none
};

const std::vector<std::string> tokens = {
    "",       "=",    "+",    "-",    "/", "*", "(",     ")",  "{",   "}",
    "return", "",     "",     "",     "",  "",  "fn",    "->", "int", "float",
    "string", "char", "bool", "void", ";", ",", "print", ""};

const std::vector<std::string> token_names = {"id",
                                              "assign",
                                              "plus",
                                              "minus",
                                              "divide",
                                              "multiply",
                                              "open paren",
                                              "close paren",
                                              "open bracket",
                                              "close bracket",
                                              "return",
                                              "int literal",
                                              "float literal",
                                              "string literal",
                                              "unknown",
                                              "eof",
                                              "function",
                                              "arrow",
                                              "type int",
                                              "type float",
                                              "type string",
                                              "type char",
                                              "type bool",
                                              "type void",
                                              "semicolon",
                                              "comma",
                                              "print",
                                              "none"};

struct Token {
  std::string str{};
  TokenType type = id;
  std::any value;
  int line_number = 0;
  int char_number = 0;
};

struct Lexer {
  size_t curr_pos = 0;
  int curr_line = 1;
  int curr_char = 0;
  size_t current_token = 0;
  std::string file_str{};
  std::vector<Token> parsed_tokens;

  Lexer() = default;
  explicit Lexer(std::string &str) : file_str{str} { tokenize(str); }


  [[nodiscard]] bool done() const;
  void remove_comments();
  static bool any_token(const std::string &str, Token &token);

  static bool is_float(const std::string &str) ;
  static bool is_int(const std::string &str);
  void print_tokens();
  std::pair<Lexer, Lexer> bisect() ;
  void advance(size_t x);
  void move(int x);

  Lexer slice(const size_t beg, const size_t end);
  std::optional<size_t> find_next(const TokenType t);
  Token next_token();

  [[nodiscard]] bool expect(const TokenType t) const;

  Token get();

  Token next();

  [[nodiscard]] size_t count() const;

  Token look_ahead(const size_t x = 1);

  void tokenize(std::string &filename);
};

} // namespace lexer
