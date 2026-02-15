cmake_minimum_required(VERSION 3.16)
project(SLR)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(FLEX)
FLEX_TARGET(Scanner src/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.cc)

set(exec_name slr)

add_executable(${exec_name} src/main.cpp src/syntax_analyzer.cpp ${FLEX_Scanner_OUTPUTS})
target_include_directories(${exec_name} PUBLIC include gen)
