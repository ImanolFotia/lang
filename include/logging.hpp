#pragma once
#include <lexer.hpp>

#include <print>
#include <string>

namespace logging {

static void expected_error(const lexer::Token &t, const std::string &expected) {
  println("[ERROR] {}:{}:{}: expected '{}', got '{}'", "test.lang", t.line_number,
          t.char_number, expected, t.str);
  exit(255);
}
} // namespace log