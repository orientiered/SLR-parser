#include "lexer.hpp"
#include <iostream>

int main() {
    mathLexer lexer;          // create our lexer
    lexer.yylex();           // scan entire input, filling lexer.tokens

    // Print all collected tokens
    for (const auto& tok : lexer.tokens) {
        std::cout << "Token: ";
        switch (tok.type) {
            case NUMBER:   std::cout << "NUMBER  " << tok.lexeme
                                     << " (value=" << tok.int_val << ")"; break;
            case OPERATOR: std::cout << "OPERATOR" << tok.lexeme
                                     << " (char='" << tok.op_char << "')"; break;
            case UNKNOWN:  std::cout << "UNKNOWN " << tok.lexeme; break;
        }
        std::cout << std::endl;
    }
    return 0;
}
