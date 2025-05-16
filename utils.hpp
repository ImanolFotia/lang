#pragma once

#include <fstream>
#include <string>

namespace utils {

static std::string read_entire_file(std::ifstream &f) {
  std::string output{};
  while (!f.eof()) {
    char buffer[1024] = {};
    f.read(buffer, 1024);
    const size_t num_bytes = f.gcount();
    if (num_bytes == 0)
      break;
    output += std::string(buffer, num_bytes);
  }
  return output;
}

} // namespace utils
