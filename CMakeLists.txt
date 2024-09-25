cmake_minimum_required(VERSION 3.24.0)

project(austlisp CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./vendor/cxxopts/)

set(SOURCE_CODE_FILE
  "./src/main.cpp"
  "./src/lisp.hpp"
  "./src/eval.hpp"
  "./src/env.cpp"
  "./src/env.hpp"
  "./src/lexical.hpp")

add_executable(${PROJECT_NAME} ${SOURCE_CODE_FILE})
