#include <any>
#include <memory>
#include <print>
#include <unordered_map>
#include <vector>

#define DEBUG_MODE 0

#include "eval.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"

using namespace lexer;

int main(const int argc, char **argv) {
  if (argc < 2) {
    std::println("no file/s specified");
    return 1;
  }

  std::string filename(argv[1]);

  lexer::Lexer l(filename);

  // std::println("target triple = \"x86_64-pc-linux-gnu\"");
  // std::println("declare i32 @printf(ptr noundef, ...)");
  std::vector<std::string> str_literals;

  const auto root = std::make_shared<Node>();
  root->type = NodeType::ROOT_NODE;
  root->name = "Program";

  generate_expression(l, root);
  eval(root);
  // for(auto &[key, value]: vars) {
  //   std::println("{} = {}", key, std::any_cast<int>(value->value));
  // }
  return 0;
  while (!l.done()) {
    if (l.get().type == fn) {
      l.next();
      if (l.expect(id)) {
        std::string func_name = l.get().str;
        l.next();
        if (l.expect(assign)) {
          l.next();
          if (l.next().type == open_paren && l.next().type == close_paren) {
            if (l.next().type == arrow) {
              if (l.expect(type_int) || l.expect(type_float) ||
                  l.expect(type_void)) {
                std::string type = "i32";
                std::println("define {} @{}() {{", type, func_name);
                l.next();
                if (l.next().type == open_brace) {
                  int var_count = 1;
                  std::unordered_map<std::string, int> vars;
                  while (!l.expect(close_brace) && !l.done()) {
                    std::string current_id;
                    if (l.expect(type_int)) {
                      l.next();
                      if (l.expect(id)) {
                        current_id = l.get().str;
                        vars[l.get().str] = var_count;
                        std::println("  %{} = alloca i32", var_count);
                        var_count++;
                        l.next();
                      }
                      if (l.next().type == assign) {
                        if (l.expect(int_literal)) {
                          std::println("  store i32 {}, ptr %{}",
                                       atoi(l.get().str.c_str()),
                                       vars[current_id]);
                          l.next();
                        }
                      }
                    }
                    if (l.expect(id)) {
                      current_id = l.get().str;
                      l.next();
                      if (l.expect(open_paren)) {
                        l.next();
                        std::vector<std::string> args;
                        while (!l.expect(close_paren) && !l.done()) {
                          if (l.expect(string_literal)) {
                            str_literals.push_back(l.get().str);
                            l.next();
                          }

                          else if (l.expect(id)) {
                            std::println("  %{} = load i32, ptr %{}", var_count,
                                         vars[l.get().str]);
                            args.push_back(l.get().str);
                            vars[l.get().str] = var_count++;
                            l.next();
                          }

                          else if (!l.expect(comma) && !l.expect(close_paren)) {
                            std::println("[Error] expected comma or close "
                                         "paren, got {} ./{}:{}:{}",
                                         l.get().str, filename,
                                         l.get().line_number,
                                         l.get().char_number);
                            exit(1);
                          }

                          if (l.expect(close_paren)) {
                            l.next();
                            break;
                          }
                          if (l.expect(comma))
                            l.next();
                        }
                        std::print(
                            "  %{} = call i32 (ptr, ...) @printf(ptr @.str{},",
                            var_count, str_literals.size() - 1);
                        for (auto &arg : args) {
                          std::print(" i32 %{}", vars[arg]);
                        }
                        std::println(")");
                      }
                      l.next();
                    }

                    if (l.expect(ret)) {
                      if (l.look_ahead().type == int_literal) {
                      } else if (l.look_ahead().type == id) {
                        std::println("  %{} = load i32, ptr %{}", ++var_count,
                                     vars[l.look_ahead().str]);
                        std::println("  ret i32 %{}", var_count);
                      }
                    }
                    l.next();
                  }
                  std::println("}}");
                }
              }
            }
          }
        }
      }
    }
    l.next();
  }

  /*int str_count = 0;
  for (auto &str : str_literals) {
    std::erase_if(str, [](char c) { return c == '\"'; });
    std::println("@.str{} = constant [{} x i8] c"{}\\00\"", str_count,
                 str.size() - 1, str);
    str_count++;
  }*/

  // std::println("{}", read_entire_file(file));
  /*Lexer l(read_entire_file(file));
  for(;;) {
    Token t = l.next_token();

    if(t.type == none) continue;
*/
  /*if(t.type == fn) {
    auto f = parse_function(l);
    std::println("function: {} -> {}", f.name, TypeNames[f.return_type]);
    std::println("vars:");
    for(auto &[key, value]: f.variables) {
      std::println("    {}:{}:{}", value.name, TypeNames[value.type],
  value.value.has_value()? std::any_cast<int>(value.value) : 0);
    }
  }*/
  /*
      std::println("{:5} -> {:15} (./{}:{}:{})", t.str, token_names[t.type],
    filename, t.line_number, t.char_number+1);

      if(t.type == TokenType::eof) break;
      //std::println("found token {} of type \'{}\'", t.str,
    token_names[(int)t.type]);


    }*/

  return 0;
}
