#pragma once
#include <chrono>
#include <fstream>

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
  Lexer() = default;
  explicit Lexer(std::string &str) : file_str{str} { tokenize(str); }

  std::vector<Token> parsed_tokens;

  [[nodiscard]] bool done() const {
    return current_token >= parsed_tokens.size();
  }

  void remove_comments() {
    for (size_t i = 0 ;i < file_str.size(); i++) {
      if (file_str[i] == '/' && file_str[i + 1] == '/') {
        if (const size_t l = file_str.find( '\n', i); l != std::string::npos) {
          file_str.erase(i, l - i);
        } else {
          file_str.erase(i, 2);
        }

      }
    }
  }

  static bool any_token(const std::string &str, Token &token) {
    int i = 0;
    for (auto &t : tokens) {
      if (t == str) {
        token.type = static_cast<TokenType>(i);
        return true;
      }
      i++;
    }

    token.type = unknown;

    return false;
  }

  static bool is_float(const std::string &str) {
    if (!str.contains('.'))
      return false;
    return std::ranges::none_of(
        str, [](const char c) { return (c < 48 || c > 57) && c != '.'; });
  }
  static bool is_int(const std::string &str) {
    return std::ranges::none_of(
        str, [](const char c) { return c < 48 || c > 57; });
  }

  void print_tokens() {
    std::print("[");
    for (auto &t : parsed_tokens) {
      std::print("\'{}\' ", t.str);
    }
    std::println("]");
  }

  std::pair<Lexer, Lexer> bisect() {
    Lexer left = *this;
    Lexer right = *this;

    left.current_token = 0;
    right.current_token = 0;

    left.parsed_tokens.resize(current_token);
    right.parsed_tokens = std::vector<Token>(
        parsed_tokens.begin() + current_token, parsed_tokens.end());

    return {left, right};
  }

  void advance(size_t x) {
    current_token = x;
    current_token = std::min(current_token, parsed_tokens.size() - 1);
  }

  Lexer slice(const size_t beg, const size_t end) {
    Lexer l;
    l.parsed_tokens = std::vector<Token>(parsed_tokens.begin() + beg,
                                         parsed_tokens.begin() + end);
    l.curr_line = curr_line;
    l.curr_char = curr_char;
    return l;
  }

  std::optional<size_t> find_next(const TokenType t) {
    size_t pos = 1;
    while (current_token + pos <= parsed_tokens.size() - 1) {
      if (look_ahead(pos).type == t) {
        return current_token + pos;
      }
      pos++;
    }

    return std::nullopt;
  }

  Token next_token() {
    if (curr_pos >= file_str.size())
      return {"", eof};
    std::string buff{};
    Token t, tt;
    bool parsing_string = false;
    bool line_break = false;
    while (curr_pos < file_str.size()) {
      const char c = file_str[curr_pos++];
      curr_char++;
      if ((c == ' ' || c == '\n') && !buff.empty()) {
        if (c == '\n') {
          line_break = true;
        }
        break;
      }
      if (c == ' ') {
        continue;
      }
      if (c == '\n' && parsing_string == false) {
        curr_line++;
        line_break = true;
        continue;
      }
      if (c == '\"' && parsing_string == false) {
        parsing_string = true;
      } else if (c == '\"') {
        parsing_string = false;
        buff += c;
        t.type = string_literal;
        break;
      }
      if (curr_pos >= file_str.size())
        return {"", eof};
      buff += c;

      if ((parsing_string == false && any_token(buff, t) &&
           file_str.size() > curr_pos &&
           (file_str[curr_pos] == ' ' ||
            !any_token(file_str.substr(curr_pos, 1), tt))) ||
          any_token(file_str.substr(curr_pos, 1), tt)) {

        if (buff[0] == '-' && file_str[curr_pos + 1] != '>') continue;
          break;
      }
    }

    if (t.type == unknown) {
      if (is_int(buff)) {
        t.type = int_literal;
        t.value = static_cast<int>(strtol(buff.c_str(), nullptr, 10));
        // std::println("{}", buff.c_str() );
      } else if (is_float(buff))
        t.type = float_literal;
      else
        t.type = id;
    }

    t.str = buff;

    if (t.str.empty()) {
      t.type = none;
    }
    t.line_number = curr_line;
    t.char_number = curr_char - t.str.size();
    if (line_break) {
      curr_char = 0;
      curr_line++;
    }
    return t;
  }

  [[nodiscard]] bool expect(const TokenType t) const {
    if (current_token >= parsed_tokens.size())
      return false;
    return t == parsed_tokens[current_token].type;
  }

  Token get() {
    if (current_token >= parsed_tokens.size())
      return {};
    return parsed_tokens[current_token];
  }

  Token next() {
    if (current_token >= parsed_tokens.size()) {

      return {.type = eof};
    }
    return parsed_tokens[current_token++];
  }

  [[nodiscard]] size_t count() const { return parsed_tokens.size(); }

  Token look_ahead(const size_t x = 1) {
    if (current_token + x >= parsed_tokens.size())
      return {};
    return parsed_tokens[current_token + x];
  }

  void tokenize(std::string &filename) {

    std::ifstream file(filename.c_str());
#if DEBUG_MODE == 1
    auto start = std::chrono::steady_clock::now();
#endif
    file_str = utils::read_entire_file(file);
    remove_comments();
    for (;;) {
      Token t = next_token();
      parsed_tokens.push_back(t);

      if (t.type == none)
        continue;

#if DEBUG_MODE == 1
      std::println("{:5} -> {:15} ./{}:{}:{}", t.str, token_names[t.type],
                   filename, t.line_number, t.char_number + 1);
#endif
      if (t.type == eof)
        break;
    }
#if DEBUG_MODE == 1
    auto end = std::chrono::steady_clock::now();
    std::println(
        "[INFO] file lexing took {}",
        std::chrono::duration_cast<std::chrono::microseconds>(end - start));
    for (auto &t : parsed_tokens) {
      std::println("'{}' line: {} char: {}", t.str, t.line_number,
                   t.char_number);
    }
#endif
  }
};

} // namespace lexer
