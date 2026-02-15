#include <ostream>
#include "syntax_analyzer.hpp"

State_t CLOSURE(State_t I) {
    State_t result = I;

    while (true) {
        size_t cur_size = result.size();

        // collecting all non-terminals that are candidates for
        // new productions in closure
        std::set<Symbol> newLhs{};
        for (auto item: result) {
            Symbol curSym = getItemSymbol(item);
            if (!isTerm(curSym)) newLhs.insert(curSym);
        }

        // looping over all productions and adding them to closure if
        // we found previously it's lhs
        for (int i = 0; i < grammar.size(); i++) {
            if (newLhs.contains(grammar[i].lhs)) {
                result.insert(Item{i, 0});
            }
        }

        if (result.size() == cur_size) {
            // no new items
            break;
        }
    }

    return result;
}


std::ostream& operator<<(std::ostream& os, Item item) {
    const char *sym_to_text[] = {
        "E0", "E", "T", "F",
        "num", "id", "+", "-", "*", "/", "(", ")", "$"
    };

    const Production& prod = grammar[item.id];
    os << sym_to_text[prod.lhs] << " -> ";
    for (int i = 0; i < prod.rhs.size(); i++) {
        if (i == item.dotPos) os << "· ";
        os << sym_to_text[prod.rhs[i]] << " ";
    }


    return os;
}

State_t GOTO(State_t I, Symbol s) {
    State_t result{};

    for (auto item: I) {
        if (getItemSymbol(item) == s) {
            result.insert({item.id, item.dotPos+1});
        }
    }

    return CLOSURE(result);
}

std::vector<State_t> BUILD_CANONIC_STATES() {
    // starting from closure({E0->E})
    std::vector<State_t> result = {CLOSURE({Item{0, 0} })};

    for (int i = 0; i < result.size(); i++) {
        // collecting all symbols after dot
        // other symbols will produce empty goto

        std::set<Symbol> symbols_for_goto;
        for (Item item: result[i]) {
            symbols_for_goto.insert(getItemSymbol(item));
        }
        symbols_for_goto.erase(END); // $ must not be included

        for (Symbol s: symbols_for_goto) {
            State_t new_state = GOTO(result[i], s);
            if (new_state.empty()) continue;

            if (std::find(result.begin(), result.end(), new_state) == result.end() ) {
                result.push_back(new_state);
            }
        }



    }
    return result;
}
