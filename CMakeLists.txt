cmake_minimum_required(VERSION 3.31.6)
project(lang)
set(CMAKE_CXX_STANDARD 23)
include_directories(lang PUBLIC include)
add_compile_options(-g -ggdb -O2)
add_executable(lang src/state.cpp lang.cpp include/logging.hpp include/lexer.hpp
        src/parser.cpp
        src/eval.cpp
        src/lexer.cpp
)