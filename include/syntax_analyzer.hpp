#pragma once

#include <lexer.hpp>
#include <memory>
#include <set>
#include <vector>

/*
E0 -> E
E -> E [+-] T | T
T -> T [*\/] F | F
F -> ( E ) | id | num
*/

enum Symbol {
    E0, E, T, F,
    NUM, ID, PLUS, MINUS, MUL, DIV, LBRACKET, RBRACKET, END
};

enum ActionType { ERROR, SHIFT, REDUCE, ACCEPT };

// TODO: symbol from token ctor

inline bool isTerm(Symbol s) {
    switch(s) {
        case E0: case E: case T: case F:
            return false;
        case NUM: case ID: case PLUS: case MINUS: case MUL: case DIV:
        case LBRACKET: case RBRACKET: case END:
            return true;
        default:
            std::cerr << s << " is not a valid symbol\n";
            return false;
    }
}


struct Production {
    Symbol lhs;
    std::vector<Symbol> rhs;
};

// const std::vector<Symbol> allSymbols = {
//     E0, E, T, F,
//     NUM, ID, PLUS, MINUS, MUL, DIV, LBRACKET, RBRACKET, END
// };

const std::vector<Production> grammar = {
    {E0, {E}},

    {E, {T}},
    {E, {E, PLUS, T}},
    {E, {E, MINUS, T}},

    {T, {F}},
    {T, {T, MUL, F}},
    {T, {T, DIV, F}},

    {F, {LBRACKET, E, RBRACKET}},
    {F, {ID}},
    {F, {NUM}}
};


struct Item {
    int id;
    int dotPos;
    bool operator<(const Item& other) const {
        if (id != other.id) return id < other.id;
        return dotPos < other.dotPos;
    }
};

inline Symbol getItemSymbol(Item item) {
    const std::vector<Symbol>& rhs = grammar[item.id].rhs;
    if (item.dotPos == rhs.size()) return END;

    return rhs[item.dotPos];
}

using State_t = std::set<Item>;

State_t CLOSURE(State_t I);

State_t GOTO(State_t I, Symbol s);

std::vector<State_t> BUILD_CANONIC_STATES();

std::ostream& operator<<(std::ostream& os, Item item);


struct Node {};
#include <stack>
class SyntaxAnalyzer {
    std::vector<int> stateStack;
    std::vector<std::shared_ptr<Node>> ast;

    mathLexer lexer;
};
