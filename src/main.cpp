#include <iostream>
#include <fstream>

#include "lexer.hpp"
#include "AST.hpp"
#include "syntax_analyzer.hpp"
int main() {
    SyntaxAnalyzer parser;

    int error = parser.init();
    if (error) {
        std::cerr << "Parser initialization error\n";
        return EXIT_FAILURE;
    }

    parser.dump_tables("docs/action_goto.csv");

    parser.parse("1+1*2");

    AST::NodePtr root = parser.get_root();

    std::ofstream tree_file("ast.dot");

    AST::dumpTree(root, tree_file);

    tree_file.close();

    system("dot -Tsvg ast.dot -o ast.svg");

    return 0;
}
