#include <iostream>
#include <fstream>

#include "lexer.hpp"
#include "AST.hpp"
#include "syntax_analyzer.hpp"

int main() {
    // mathLexer lexer;
    // lexer.yylex();
    SyntaxAnalyzer parser;

    parser.init();

    parser.dump("docs/action_goto.csv");

    parser.parse();

    AST::NodePtr root = parser.getRoot();

    std::ofstream tree_file("ast.dot");

    AST::dumpTree(root, tree_file);

    tree_file.close();

    system("dot -Tsvg ast.dot -o ast.svg");

    return 0;
}
