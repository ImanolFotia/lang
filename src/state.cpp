#include <definitions.hpp>

std::unordered_map<std::string, std::shared_ptr<Node>> State::vars;
std::unordered_map<std::string, std::shared_ptr<Node>> State::functions;
Test State::test;
std::string State::name = "setted name";