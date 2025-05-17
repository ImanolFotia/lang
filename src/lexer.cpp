#include <lexer.hpp>

#include <any>
#include <chrono>
#include <fstream>
#include <print>

#include <utils.hpp>

#include <algorithm>

namespace lexer {

[[nodiscard]] bool Lexer::done() const {
  return current_token >= parsed_tokens.size();
}

void Lexer::remove_comments() {
  for (size_t i = 0; i < file_str.size(); i++) {
    if (file_str[i] == '/' && file_str[i + 1] == '/') {
      if (const size_t l = file_str.find('\n', i); l != std::string::npos) {
        file_str.erase(i, l - i);
      } else {
        file_str.erase(i, 2);
      }
    }
  }
}

bool Lexer::any_token(const std::string &str, Token &token) {
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

bool Lexer::is_float(const std::string &str) {
  if (!str.contains('.'))
    return false;
  return std::ranges::none_of(
      str, [](const char c) { return (c < 48 || c > 57) && c != '.'; });
}
bool Lexer::is_int(const std::string &str) {
  return std::ranges::none_of(str,
                              [](const char c) { return c < 48 || c > 57; });
}

void Lexer::print_tokens() {
  std::print("[");
  for (auto &t : parsed_tokens) {
    std::print("\'{}\' ", t.str);
  }
  std::println("]");
}
void Lexer::move(const int x) { current_token += x; }

std::pair<Lexer, Lexer> Lexer::bisect() {
  Lexer left = *this;
  Lexer right = *this;

  left.current_token = 0;
  right.current_token = 0;

  left.parsed_tokens.resize(current_token);
  right.parsed_tokens = std::vector<Token>(
      parsed_tokens.begin() + current_token, parsed_tokens.end());

  return {left, right};
}

void Lexer::advance(size_t x) {
  current_token = x;
  current_token = std::min(current_token, parsed_tokens.size() - 1);
}

Lexer Lexer::slice(const size_t beg, const size_t end) {
  Lexer l;
  l.parsed_tokens = std::vector<Token>(parsed_tokens.begin() + beg,
                                       parsed_tokens.begin() + end);
  l.curr_line = curr_line;
  l.curr_char = curr_char;
  return l;
}

std::optional<size_t> Lexer::find_next(const TokenType t) {
  size_t pos = 1;
  while (current_token + pos <= parsed_tokens.size() - 1) {
    if (look_ahead(pos).type == t) {
      return current_token + pos;
    }
    pos++;
  }

  return std::nullopt;
}

Token Lexer::next_token() {
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

      if (buff[0] == '-' && file_str[curr_pos + 1] != '>')
        continue;
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
    line_break = false;
  }
  return t;
}

[[nodiscard]] bool Lexer::expect(const TokenType t) const {
  if (current_token >= parsed_tokens.size())
    return false;
  return t == parsed_tokens[current_token].type;
}

Token Lexer::get() {
  if (current_token >= parsed_tokens.size())
    return {};
  return parsed_tokens[current_token];
}

Token Lexer::next() {
  if (current_token >= parsed_tokens.size()) {

    return {.type = eof};
  }
  return parsed_tokens[current_token++];
}

[[nodiscard]] size_t Lexer::count() const { return parsed_tokens.size(); }

Token Lexer::look_ahead(const size_t x) {
  if (current_token + x >= parsed_tokens.size())
    return {};
  return parsed_tokens[current_token + x];
}

void Lexer::tokenize(std::string &filename) {

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
    std::println("'{}' line: {} char: {}", t.str, t.line_number, t.char_number);
  }
#endif
}
};
