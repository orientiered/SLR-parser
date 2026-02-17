#include <algorithm>
#include <ostream>
#include "syntax_analyzer.hpp"

using State_t = SyntaxAnalyzer::State_t;

template <typename T>
static void merge_inplace(std::set<T>& dest, const std::set<T>& src) {
    for (const T& elem: src)
        dest.insert(elem);
}

State_t SyntaxAnalyzer::state_closure(State_t I) {
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

std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Symbol s) {
    const char * const sym_to_text[] = {
        "EPS",
        "E0", "E", "T", "F",
        "num", "id", "+", "-", "*", "/", "(", ")", "$"
    };

    os << sym_to_text[s];
    return os;
}


std::ostream& SyntaxAnalyzer::Item::printItem(std::ostream& os) {


    const Production& prod = ::SyntaxAnalyzer::grammar[id];
    os << prod.lhs << " -> ";
    for (int i = 0; i < prod.rhs.size(); i++) {
        if (i == dotPos) os << "· ";
        os << prod.rhs[i] << " ";
    }

    if (dotPos == prod.rhs.size()) os << "· ";


    return os;
}

std::ostream& operator<<(std::ostream& os, SyntaxAnalyzer::Item item) {
    return item.printItem(os);
}

State_t SyntaxAnalyzer::state_goto(State_t I, Symbol s) {
    State_t result{};

    for (auto item: I) {
        if (getItemSymbol(item) == s) {
            result.insert({item.id, item.dotPos+1});
        }
    }

    return state_closure(result);
}

std::vector<State_t> SyntaxAnalyzer::build_canonic_states() {
    // starting from closure({E0->E})
    std::vector<State_t> result = {state_closure({Item{0, 0} })};

    for (int i = 0; i < result.size(); i++) {
        // collecting all symbols after dot
        // other symbols will produce empty goto

        std::set<Symbol> symbols_for_goto;
        for (Item item: result[i]) {
            symbols_for_goto.insert(getItemSymbol(item));
        }
        symbols_for_goto.erase(END); // $ must not be included

        for (Symbol s: symbols_for_goto) {
            State_t new_state = state_goto(result[i], s);
            if (new_state.empty()) continue;


            auto new_state_it = std::find(result.begin(), result.end(), new_state);
            int new_state_idx = new_state_it - result.begin();

            // saving information for goto's
            state_transitions[{i, s}] = new_state_idx;

            // pushing new state if its not in states
            if (new_state_it == result.end())
                result.push_back(new_state);
        }

    }
    return result;
}

std::set<SyntaxAnalyzer::Symbol> SyntaxAnalyzer::first_of_string(const std::vector<Symbol>& rhs) {
    std::set<Symbol> temp = {}; //? maybe EPS must be here
    bool nullable = true;

    // looping over all symbols in rhs of rule
    for (int i = 0; i < rhs.size(); i++) {
        if (!nullable) break;

        bool temp_has_eps = temp.contains(EPS);

        std::set<Symbol>& Xi_first = FIRST[rhs[i]];

        merge_inplace(temp, Xi_first);
        // merging FIRST[X_i] without END symbol
        if (!temp_has_eps) temp.erase(EPS);

        if (!Xi_first.contains(EPS)) {
            nullable = false;
        }

    }

    if (nullable) temp.insert(EPS);

    return temp;
}


int SyntaxAnalyzer::computeFirst() {
    FIRST.clear();
    // initialization: terminals have itself in their first set
    for (const Symbol s: allSymbols) {
        if (isTerm(s)) {
            FIRST[s] = {s};
        } else {
            FIRST[s] = {};
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;

        // looping over all grammar rules
        for (const Production& prod: grammar) {
            std::set<Symbol> rhs_first = first_of_string(prod.rhs);

            std::size_t prev_size = FIRST[prod.lhs].size();
            merge_inplace(FIRST[prod.lhs], rhs_first);
            if (FIRST[prod.lhs].size() > prev_size)
                changed = true;

        }

    }

    return 0;
}

int SyntaxAnalyzer::computeFollow() {
    // init
    FOLLOW.clear();
    FOLLOW[start_symbol] = {END};

    bool changed = true;
    while (changed) {
        changed = false;

        for (const Production& prod: grammar) {

            for (int i = 0; i < prod.rhs.size(); i++) {
                // skipping terminals
                if (isTerm(prod.rhs[i])) continue;

                Symbol lhs = prod.lhs;
                Symbol cur = prod.rhs[i]; // nonterm
                std::size_t prev_size = FOLLOW[cur].size();

                // 1
                std::vector<Symbol> remain(prod.rhs.begin()+i+1, prod.rhs.end());
                std::set<Symbol> first_of_remain = first_of_string(remain);
                bool first_has_eps = first_of_remain.contains(EPS);

                // merging without EPS
                first_of_remain.erase(EPS);
                merge_inplace(FOLLOW[cur], first_of_remain);

                // 2
                if (remain.empty() || first_has_eps) {
                    merge_inplace(FOLLOW[cur], FOLLOW[lhs]);
                }

                if (FOLLOW[cur].size() > prev_size) changed = true;
            }
        }

    }
    return 0;
}

int SyntaxAnalyzer::build_action_goto() {
    action_goto.clear();
    action_goto.resize(states.size());

    auto report_conflict = [&](int state_idx, const Item& item, Symbol s, std::string msg) {
        const ActionEntry& entry = action_goto[state_idx][s];
        std::cout << "Grammar conflict:" << msg << "\n" <<
            "state " << state_idx << "\n" <<
            "item " << item << "\n" <<
            "problem sym" << s << "\n" <<
            "type" << ((entry.type == SHIFT) ? "SHIFT" : "REDUCE") << "\n";
    };

    for (std::size_t state_idx = 0; state_idx < states.size(); state_idx++) {

        for (const Item& item: states[state_idx]) {

            const Production& prod = grammar[item.id];

            Symbol cur_sym = getItemSymbol(item);

            if (cur_sym == END) {
                if (prod.lhs == start_symbol) {
                    action_goto[state_idx][END] = {ACCEPT, 0};
                } else {
                    for (Symbol fol_sym : FOLLOW[prod.lhs]) {
                        ActionEntry& entry = action_goto[state_idx][fol_sym];
                        if (entry.type != ERROR) {
                            report_conflict(state_idx, item, fol_sym, "REDUCE");
                        } else {
                            entry = {REDUCE, item.id};
                        }
                    }
                }
            } else {
                int j = state_transitions[{state_idx, cur_sym}];
                ActionEntry& entry = action_goto[state_idx][cur_sym];
                if (entry.type != ERROR &&
                    !(entry.type == SHIFT && entry.val == j) ) {
                    report_conflict(state_idx, item, cur_sym, "SHIFT");
                } else {
                    entry = {SHIFT, j};
                }

            }
        }

        // for (auto [pair, j]  : state_transitions) {
        //     int i = pair.first;
        //     Symbol sym = pair.second;
        //     if (state_idx == i && !isTerm(sym)) {
        //         action_goto[state_idx][sym] = {GOTO, j};
        //     }
        // }

    }

    for (auto [pair, j]  : state_transitions) {
        int i = pair.first;
        Symbol sym = pair.second;
        if (!isTerm(sym)) {
            action_goto[i][sym] = {GOTO, j};
        }
    }

    return 0;
}


int SyntaxAnalyzer::init() {
    states = build_canonic_states();
    computeFirst();
    computeFollow();
    return build_action_goto();
}

#include <fstream>

void SyntaxAnalyzer::dump(std::string action_table_path) {
    std::cout << "=============FIRST==============\n";
    for (Symbol s: allSymbols) {
        std::cout << s << " -> ";
        for (Symbol f: FIRST[s]) {
            std::cout << f << " ";
        }
        std::cout << "\n";
    }

    std::cout << "============FOLLOW==============\n";
    for (Symbol s: allSymbols) {
        std::cout << s << " -> ";
        for (Symbol f: FOLLOW[s]) {
            std::cout << f << " ";
        }
        std::cout << "\n";
    }

    std::cout << "============ STATES ===============\n";
    for (int i = 0; i < states.size(); i++) {
        std::cout << "I_" << i << ":\n";
        for (const auto& item: states[i]) {
            std::cout << "\t" << item << "\n";
        }
    }

    auto printAction = [](std::ostream& os, const ActionEntry& entry) {
        switch (entry.type) {
            case ERROR: break;
            case ACCEPT: os << "A"; break;
            case SHIFT:  os << "S"; break;
            case REDUCE: os << "R"; break;
            case GOTO:   os << "G"; break;
            default: std::cerr << "UNKNOWN ENTRY TYPE\n";
        }

        if (entry.type != ERROR)
            os << entry.val;
    };


    std::ofstream csv(action_table_path);
    if (!csv.good() ) {
        std::cerr << "Failed to open file" << action_table_path << "\n";
    } else {
        //header
        csv << "Sym ";
        for (Symbol sym: allSymbols) {
            csv << ", " << '"' << sym << '"';
        }
        csv << "\n";

        //body
        for (int i = 0; i < states.size(); i++) {
            csv << i+1;
            for (Symbol sym: allSymbols) {
                csv << ", ";
                printAction(csv, action_goto[i][sym]);
            }
            csv << "\n";
        }

    }

    csv.close();
}
