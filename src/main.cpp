#include "lexer.hpp"
#include <iostream>

#include "syntax_analyzer.hpp"

int main() {
    mathLexer lexer;
    // lexer.yylex();
    SyntaxAnalyzer parser;

    parser.init();

    parser.dump("docs/action_goto.csv");
    // auto state = CLOSURE({{0, 0}});
    // for (auto item: state) {
    //     std::cout << item << "\n";
    // }

    for (Token tok = lexer(); tok.type_ != TokenType::END; tok = lexer()) {
        std::cout << "Token: ";
        switch (tok.type_) {
            case TokenType::END:    std::cout << "$"; break;
            case TokenType::NUMBER:   std::cout << "NUMBER  " << tok.lexeme_
                                     << " (value=" << tok.int_val << ")"; break;
            case TokenType::OPERATOR: std::cout << "OPERATOR" << tok.lexeme_
                                     << " (char='" << tok.op_char << "')"; break;
            case TokenType::IDENTIFIER:
                           std::cout << "IDENTIFIER " << tok.lexeme_; break;
            case TokenType::UNKNOWN:  std::cout << "UNKNOWN " << tok.lexeme_; break;
        }
        std::cout << std::endl;
    }
    return 0;
}
