#pragma once

#include <lexer.hpp>
#include <map>
#include <memory>
#include <set>
#include <vector>

/*
E0 -> E
E -> E [+-] T | T
T -> T [*\/] F | F
F -> ( E ) | id | num
*/

struct Node {};

class SyntaxAnalyzer {
public:
    /* ============= SYMBOLS AND PRODUCTION ====================== */
    enum Symbol {
        EPS, // special symbol
        E0, E, T, F,
        NUM, ID, PLUS, MINUS, MUL, DIV, LBRACKET, RBRACKET, END
    };
    Symbol token_to_symbol(const Token& tok);

    const std::vector<Symbol> allSymbols = {
        E0, E, T, F,
        NUM, ID, PLUS, MINUS, MUL, DIV, LBRACKET, RBRACKET, END
    };

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

    /* ================= LANGUAGE GRAMMAR RULES ================ */

    const static inline Symbol start_symbol = E0;
    const static inline std::vector<Production> grammar = {
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
    /* ================= ITEM ======================== */
    // @brief Production from grammar + dot position

    struct Item {
        int id;
        int dotPos;
        bool operator<(const Item& other) const {
            if (id != other.id) return id < other.id;
            return dotPos < other.dotPos;
        }
        bool operator==(const Item& other) const {
            return id == other.id && dotPos == other.dotPos;
        }
        std::ostream& printItem(std::ostream& os);
    };



    inline Symbol getItemSymbol(Item item) {
        const std::vector<Symbol>& rhs = grammar[item.id].rhs;
        if (item.dotPos == rhs.size()) return END;

        return rhs[item.dotPos];
    }

    /* ================ HELPERS =================== */

    using State_t = std::set<Item>;

    State_t state_closure(State_t I);

    State_t state_goto(State_t I, Symbol s);


    std::set<Symbol> first_of_string(const std::vector<Symbol>& rhs);

    std::vector<State_t> build_canonic_states();
    int computeFirst();
    int computeFollow();
    int build_action_goto();

    /* ================ ACTION TABLE ============================ */
private:
    std::map<Symbol, std::set<Symbol>> FIRST;
    std::map<Symbol, std::set<Symbol>> FOLLOW;

    std::vector<State_t> states; // canonic states

    enum ActionType { ERROR=0, GOTO, SHIFT, REDUCE, ACCEPT };

    struct ActionEntry {
        ActionType type;
        int val;

        ActionEntry(): type(ERROR), val(-1) {} // ERROR must be default
        ActionEntry(ActionType t, int v): type(t), val(v) {}
        bool operator==(const ActionEntry& other) {
            return type == other.type && val == other.val;
        }
    };

    std::ostream& printAction(std::ostream& os, const ActionEntry& entry);

    // unified action and goto table
    std::map<std::pair<int, Symbol>, int> state_transitions;
    std::vector<std::map<Symbol, ActionEntry>> action_goto;

    mathLexer lexer;
    /* ================ PARSING STATE =========================== */
    std::vector<std::pair<int, Symbol>> stateStack;
    std::vector<std::shared_ptr<Node>> ast;

    void report_error(int state, const Token& tok);

public:
    int init();
    int parse();

    void dump(const std::string action_goto_path);

};

std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Symbol item);
std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Item item);
