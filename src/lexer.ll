%{
// ===== C++ header section =====
#include "lexer.hpp"

// Tell Flex that the scanning function belongs to our class,
// not to the base class.
#define YY_DECL int mathLexer::yylex()
%}

%option c++
%option noyywrap
%option prefix="math"
%%

[0-9]+    {
    tokens.push_back({
        NUMBER,
        std::string(yytext),
        std::atoi(yytext),
        0
    });
}

[-+*/]    {
    tokens.push_back({
        OPERATOR,
        std::string(yytext),
        0,
        yytext[0]
    });
}

[ \t\n]   ;               // ignore whitespace

.         {
    tokens.push_back({
        UNKNOWN,
        std::string(yytext),
        0,
        0
    });
}

%%

// ===== User code =====

int mathFlexLexer::yylex() {
    //! Error
    return 0;
}
