#pragma once

#include <iostream>
#include <string>
#include <cstdlib>

# ifndef __FLEX_LEXER_H
#  define yyFlexLexer mathFlexLexer
#  include <FlexLexer.h>
#  undef yyFlexLexer
# endif

// ---------- Token struct ----------
enum class TokenType { END = 0, NUMBER, OPERATOR, IDENTIFIER, UNKNOWN };

struct Token {
    TokenType type_;
    std::string lexeme_;
    int line_;
    int pos_;
    int int_val;
    char op_char;

    Token(TokenType T, const char *text, int line = 0, int pos = 0) :type_(T), lexeme_(text), line_(line), pos_(pos) {
        switch(T) {
        case TokenType::END:
            int_val = 0;
            break;
        case TokenType::NUMBER:
            int_val = std::atoi(text);
            break;
        case TokenType::OPERATOR:
            op_char = text[0];
            break;
        case TokenType::IDENTIFIER:
            break;
        case TokenType::UNKNOWN:
            break;
        default:
            break;
        }
    }
};

// ---------- LEXER CLASS  ----------
class mathLexer : public mathFlexLexer {
private:
    Token current_tok = Token{TokenType::UNKNOWN, ""};
    int yypos = 0;
public:
    const Token& operator()() {
        if (yylex()) {
        } else {
            current_tok = {TokenType::END, "", lineno(), yypos};
        }
        return current_tok;
    }

    // std::vector<Token> tokens;

    template<TokenType T>
    void add_token(const char *text) {
        // tokens.push_back({T, text});
        current_tok = Token{T, text, lineno(), yypos};
    }

    int yylex() override;
};


