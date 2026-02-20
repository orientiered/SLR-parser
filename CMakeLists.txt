cmake_minimum_required(VERSION 3.16)
project(SLR)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(FLEX)
FLEX_TARGET(Scanner src/lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/scanner.cc)

add_subdirectory(googletest)

# ================================ PARSER LIB =============================

add_library(parser_lib STATIC src/syntax_analyzer.cpp src/ast.cpp ${FLEX_Scanner_OUTPUTS})
target_include_directories(parser_lib PUBLIC include)

# ================================ PARSER =================================


set(exec_name slr.exe)

add_executable(${exec_name} src/main.cpp)
target_link_libraries(${exec_name} parser_lib)
target_include_directories(${exec_name} PUBLIC include)

# ================================ UNIT TESTS ============================
set(unit_test_exec_name unit_test.exe)

add_executable(${unit_test_exec_name} tests/parser_tests.cpp)

target_include_directories(${unit_test_exec_name} PUBLIC include googletest/googletest/include)
target_link_libraries(${unit_test_exec_name} parser_lib gtest_main)

enable_testing()

include(GoogleTest)
gtest_discover_tests(${unit_test_exec_name}
    PROPERTIES
        LABELS "parser_end2end"
)
