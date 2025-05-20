#include <definitions.hpp>

std::unordered_map<std::string, std::shared_ptr<Node>> State::vars;
std::unordered_map<std::string, std::shared_ptr<Node>> State::functions;
std::unordered_map<std::string, std::shared_ptr<Node>> State::scope_variables;