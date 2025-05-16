cmake_minimum_required(VERSION 3.31.6)
project(lang)
set(CMAKE_CXX_STANDARD 23)
add_executable(lang lang.cpp logging.hpp lexer.hpp
        eval.hpp
        definitions.hpp
        parser.hpp)