#pragma once

#include <map>
#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "AST.hpp"
#include "lexer.hpp"


using reduceFunc = void(std::vector<std::variant<Token, AST::NodePtr>>& ast);
reduceFunc reduceBinOp;
reduceFunc reduceParen;
reduceFunc reduceNumId;

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

        reduceFunc *reduce;
    };

    /* ================= LANGUAGE GRAMMAR RULES ================ */

    /*
        E0 -> E
        E -> E [+-] T | T
        T -> T [*\/] F | F
        F -> ( E ) | id | num
    */

    const static inline Symbol start_symbol = E0;
    const static inline std::vector<Production> grammar = {
        {E0, {E}},

        {E, {T}},
        {E, {E, PLUS, T}, reduceBinOp},
        {E, {E, MINUS, T}, reduceBinOp},

        {T, {F}},
        {T, {T, MUL, F}, reduceBinOp},
        {T, {T, DIV, F}, reduceBinOp},

        {F, {LBRACKET, E, RBRACKET}, reduceParen},
        {F, {ID}, reduceNumId},
        {F, {NUM}, reduceNumId}
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
        std::ostream& print_item(std::ostream& os);
    };

    inline Symbol get_item_symbol(Item item) {
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
    int compute_first();
    int compute_follow();
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

    std::ostream& print_action(std::ostream& os, const ActionEntry& entry);

    // unified action and goto table
    std::map<std::pair<int, Symbol>, int> state_transitions;
    std::vector<std::map<Symbol, ActionEntry>> action_goto;

    /* ================ PARSING STATE =========================== */
    mathLexer lexer;
    std::vector<std::pair<int, Symbol>> stateStack;
    AST::NodePtr root;

    void report_error(int state, const Token& tok);
    void print_parse_state(std::ostream& os, char delimeter = '|');

public:
    /// @brief Compute action and goto tables
    int init();

    enum class ParseStatus {SUCCESS = 0, BAD_INPUT, LEXICAL_ERR, SYNTAX_ERR, FATAL_ERR};

    /// @brief Parse text and build AST
    /// @return 0 on success, positive integer otherwise
    /// Root is erased at the start of parsing
    ParseStatus parse();
    ParseStatus parse(const std::string& expr);
    ParseStatus parse(std::istream& in);
    ParseStatus parse_file(const std::string& path);

    /// @brief Get root of AST build from previous parse() call
    AST::NodePtr get_root() {
        return root;
    }

    const AST::NodePtr peek_root() {
        return root;
    }

    /// @brief Print FIRST and FOLLOW sets, canonic states to standard output
    /// Dump action/goto table as csv table to file
    void dump_tables(const std::string action_goto_path);

};

std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Symbol item);
std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Item item);
