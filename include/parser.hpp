#pragma once
#include "lexer.hpp"
#include <definitions.hpp>
#include <unordered_map>

namespace parser {

bool is_any_type(const lexer::TokenType t);

Type get_type(const lexer::Token &t);

Function parse_function_header(lexer::Lexer &l);

void parse_expression(lexer::Lexer &l, const std::shared_ptr<Node> &node);

std::shared_ptr<Node> create_expression(lexer::Lexer &l,
                                        const std::shared_ptr<Node> &node);

void generate_expression(lexer::Lexer &l, std::shared_ptr<Node> node);
} // namespace parser
