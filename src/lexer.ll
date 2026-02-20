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
%option yylineno
%%

[0-9]+   {
    add_token<TokenType::NUMBER>(yytext);
    yycol += yyleng;
    return static_cast<int>(TokenType::NUMBER);
}

[-+*/()] {
    add_token<TokenType::OPERATOR>(yytext);
    yycol += yyleng;
    return static_cast<int>(TokenType::OPERATOR);
}

[a-zA-Z]+ {
    add_token<TokenType::IDENTIFIER>(yytext);
    yycol += yyleng;
    return static_cast<int>(TokenType::IDENTIFIER);
}

[ \t]   { yycol += yyleng; }
[\n]    { yycol = 0; }

.        {
    add_token<TokenType::UNKNOWN>(yytext);
    yycol += yyleng;
    return static_cast<int>(TokenType::UNKNOWN);
}

%%

// ===== User code =====

int mathFlexLexer::yylex() {
    //! Error
    return 0;
}
