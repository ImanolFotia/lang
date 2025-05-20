#include <memory>
#include <print>
#define DEBUG_MODE 0

#include <definitions.hpp>
#include <eval.hpp>
#include <lexer.hpp>
#include <parser.hpp>

using namespace lexer;

int main(const int argc, char **argv) {
  if (argc < 2) {
    std::println("no file/s specified");
    return 1;
  }
  std::string filename(argv[1]);

  lexer::Lexer l(filename);

  const auto root = std::make_shared<Node>();
  root->type = NodeType::ROOT_NODE;
  root->name = "Program";

  parser::generate_expression(l, root);
  eval(root);
  return 0;
}
