#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include "util.h"
#include "marpa.h"
#include "symbol_table.h"

extern const char* marpa_errors[92];

using namespace marpa;

struct grammar_rhs {
    int names_names_idx;
    int min; // 0 == *, 1 == +, 2 == names_names_idx
    friend bool operator==(const grammar_rhs& a, const grammar_rhs& b);
};
bool operator==(const grammar_rhs& a, const grammar_rhs& b) {
    return a.names_names_idx == b.names_names_idx && a.min == b.min;
}

struct grammar_rule {
    int lhs;
    grammar_rhs rhs;
    int code;

    friend bool operator==(const grammar_rule& a, const grammar_rule& b);
};
bool operator==(const grammar_rule& a, const grammar_rule& b) {
    return a.lhs == b.lhs && a.rhs == b.rhs && a.code == b.code;
}

void replace_variables(std::string& block, const std::string& var, const std::string& with) {
    auto it = std::search(block.begin(), block.end(), var.begin(), var.end());
    while (it != block.end()) {
        auto last = it;
        std::advance(last, std::distance(var.begin(),var.end()));
        block.replace(it, last, with);
        it = std::search(block.begin(), block.end(), var.begin(), var.end());
    }
}

void output_rules(
        const indexed_table<grammar_rule>& rules,
        const indexed_table<std::string>& names,
        const indexed_table<std::vector<int>>& names_names,
        const indexed_table<std::string>& code_blocks) {

    /*
    std::cout << "rules.size() = " << rules.size() << "\n";
    std::cout << "names.size() = " << names.size() << "\n";
    std::cout << "names_names.size() = " << names_names.size() << "\n";
    */

    for (auto rule : rules) {
        std::cout << names[rule.lhs] << " ::=";
        if (rule.rhs.min == 2) {
            for (auto j : names_names[rule.rhs.names_names_idx]) {
                std::cout << " " << names[j];
            }
        }
        else {
            int j = rule.rhs.names_names_idx;
            std::cout << " " << names[j];
        }

        if (rule.rhs.min == 0) {
            std::cout << "*";
        }
        else if (rule.rhs.min == 1) {
            std::cout << "+";
        }

        std::string block = code_blocks[rule.code];

        replace_variables(block, "$$", "stack[v.result()]");
        replace_variables(block, "$0", "stack[v.arg_0()]");
        replace_variables(block, "$1", "stack[v.arg_0()+1]");
        replace_variables(block, "$2", "stack[v.arg_0()+2]");
        replace_variables(block, "$3", "stack[v.arg_0()+3]");
        replace_variables(block, "$4", "stack[v.arg_0()+4]");
        replace_variables(block, "$5", "stack[v.arg_0()+5]");
        replace_variables(block, "$6", "stack[v.arg_0()+6]");
        replace_variables(block, "$7", "stack[v.arg_0()+7]");
        replace_variables(block, "$8", "stack[v.arg_0()+8]");
        replace_variables(block, "$9", "stack[v.arg_0()+9]");
        replace_variables(block, "$N", "stack[v.arg_n()+1]");

        std::string ss = "$$";

        std::cout << block << "\n";

        std::cout << "\n";
    }
    std::cout << "\n";
}

void read_file(const std::string& filename, std::string& input);

template <typename I>
I match(I first, I last, I s_first, I s_last) {
    std::pair<I,I> p = std::mismatch(first, last, s_first);
    if (p.second == s_last) {
        return p.first;
    }
    return first;
}

int main()
{
    grammar g;

    /* DEFINE GRAMMAR */
    grammar::symbol_id R_rules       = g.new_symbol();
    grammar::symbol_id R_rule        = g.new_symbol();
    grammar::symbol_id R_lhs         = g.new_symbol();
    grammar::symbol_id R_rhs         = g.new_symbol();
    grammar::symbol_id R_names       = g.new_symbol();

    grammar::symbol_id T_bnfop       = g.new_symbol();
    grammar::symbol_id T_name        = g.new_symbol();
    grammar::symbol_id T_min         = g.new_symbol();
    grammar::symbol_id T_null        = g.new_symbol();
    grammar::symbol_id T_code        = g.new_symbol();

    g.start_symbol(R_rules);

    using rule = grammar::rule_id;

    rule rule_id_rules   = g.new_sequence(R_rules, R_rule, -1, 1, 0);
    rule rule_id_rule_0  = g.add_rule(R_rule, { R_lhs, T_bnfop, R_rhs });
    rule rule_id_rule_1  = g.add_rule(R_rule, { R_lhs, T_bnfop, R_rhs, T_code });
    rule rule_id_lhs_0   = g.add_rule(R_lhs,  { T_name });
    rule rule_id_rhs_0   = g.add_rule(R_rhs,  { R_names });
    rule rule_id_rhs_1   = g.add_rule(R_rhs,  { T_name, T_min });
    rule rule_id_rhs_2   = g.add_rule(R_rhs,  { T_null });
    rule rule_id_names_0 = g.new_sequence(R_names, T_name, -1, 1, 0);

    /* END OF GRAMMAR */

    if (g.precompute() < 0) {
        std::cout << "precompute() failed\n";
        std::cout << marpa_errors[g.error()] << "\n";
        std::cout << "\n";
        exit(1);
    }

    recognizer r{g};

    /* READ TOKENS */
    std::string input;

    read_file("marpa.txt", input);

    std::string bnf_op{"::="};
    std::string null{"null"};
    std::string code_start{"{{"};
    std::string code_end{"}}"};

    indexed_table<std::string> names;
    indexed_table<std::string> code_blocks;
    code_blocks.add(""); // empty code block

    std::string sep = "%%";
    auto sep_pos = std::search(input.begin(), input.end(), sep.begin(), sep.end());
    if (sep_pos == input.end()) {
    }

    std::string pre_block{input.begin(), sep_pos };
    std::string::iterator it = sep_pos+2;

    sep_pos = std::search(it, input.end(), sep.begin(), sep.end());
    if (sep_pos == input.end()) {
        //missing %%
    }

    std::string post_block{sep_pos+2, input.end() };

    std::vector<std::tuple<std::string, grammar::symbol_id, int>> tokens{
        std::make_tuple("::=", T_bnfop, 0),
        std::make_tuple("null", T_null, 0),
        std::make_tuple("*", T_min, 0),
        std::make_tuple("+", T_min, 1),
    };

    while (it != sep_pos) {

        // skip ws
        if (isspace(*it)) {
            it++;
            continue;
        }

        if (isalpha(*it)) {
            auto begin = it;
            it = std::find_if_not(it, sep_pos, isalpha);
            std::string id(begin, it);
            int idx = names.add(id);
            r.read(T_name, idx, 1);
            continue;
        }
        if (*it == '#') {
            while (it != sep_pos && *it != '\n') {
                it++;
            }
            continue;
        }

        bool found = false;
        for (auto t : tokens) {
            auto new_it = match(it, sep_pos, std::get<0>(t).begin(), std::get<0>(t).end());
            if (new_it != it) {
                r.read(std::get<1>(t), std::get<2>(t), 1);
                it = new_it;
                found = true;
                break;
            }
        }
        if (found) continue;

        auto p = std::mismatch(it, sep_pos, code_start.begin());
        if (p.second == code_start.end()) {
            auto end = std::search(p.first, sep_pos, code_end.begin(), code_end.end());
            if (end == sep_pos) {
                // error
            }
            std::string code_block(p.first, end);
            r.read(T_code, code_blocks.add(code_block), 1);
            it = end + 2;
            continue;
        }

        std::cout << "Unknown tokens starting here\n" << std::string(it, sep_pos) << "\n";
        exit(1);
    }

    bocage b{r, r.latest_earley_set()};
    if (g.error() != MARPA_ERR_NONE) {
        std::cout << marpa_errors[g.error()] << "\n";
        return 1;
    }

    order o{b};
    tree t{o};

    /* Evaluate trees */
    while (t.next() >= 0) {
        value v{t};

        v.rule_is_valued(rule_id_rules, 1);
        v.rule_is_valued(rule_id_rule_0, 1);
        v.rule_is_valued(rule_id_rule_1, 1);
        v.rule_is_valued(rule_id_lhs_0, 1);
        v.rule_is_valued(rule_id_rhs_0, 1);
        v.rule_is_valued(rule_id_rhs_1, 1);
        v.rule_is_valued(rule_id_rhs_2, 1);
        v.rule_is_valued(rule_id_names_0, 1);

        std::vector<int> stack;
        stack.resize(128);

        indexed_table<grammar_rule>     rules;
        indexed_table<grammar_rhs>      lrhs;
        indexed_table<std::vector<int>> names_names;

        for (;;) {
            value::step_type type = v.step();

            switch (type) {
                case MARPA_STEP_INITIAL:
                    stack.resize(1);
                    lrhs.add(grammar_rhs{});
                    break;
                case MARPA_STEP_TOKEN: {
                    stack.resize(std::max((std::vector<int>::size_type)v.result()+1, stack.size()));
                    stack[v.result()] = v.token_value();
                    break;
                }
                case MARPA_STEP_RULE: {
                    grammar::rule_id rule = v.rule();
                    stack.resize(std::max((std::vector<int>::size_type)v.result()+1, stack.size()));

                    /* BEGIN OF RULE SEMANTICS */
                    if (rule == rule_id_rules) {   // list of rules
                        // done...
                        output_rules(rules, names, names_names, code_blocks);
                    }
                    else if (rule == rule_id_rule_0) { // lhs ::= rhs
                        int lhs = stack[v.arg_0()];
                        int rhs = stack[v.arg_0()+2];
                        grammar_rhs rhs_s = lrhs[rhs];
                        rules.add(grammar_rule{lhs, rhs_s, 0});
                    }
                    else if (rule == rule_id_rule_1) { // lhs ::= rhs  code
                        int lhs = stack[v.arg_0()];
                        int rhs = stack[v.arg_0()+2];
                        int code = stack[v.arg_0()+3];
                        grammar_rhs rhs_s = lrhs[rhs];
                        rules.add(grammar_rule{lhs, rhs_s, code });
                    }
                    else if (rule == rule_id_lhs_0) {  // name
                        stack[v.result()] = stack[v.arg_0()];
                    }
                    else if (rule == rule_id_rhs_0) {  // names
                        int n = stack[v.arg_0()];
                        stack[v.result()] = lrhs.add(grammar_rhs{n, 2});
                    }
                    else if (rule == rule_id_rhs_1) {  // name (*|+)
                        int n = stack[v.arg_0()];
                        int min = stack[v.arg_0()+1];
                        stack[v.result()] = lrhs.add(grammar_rhs{n, min});
                    }
                    else if (rule == rule_id_rhs_2) {  // null
                        stack[v.result()] = 0;
                    }
                    else if (rule == rule_id_names_0) { // sequence of names
                        std::vector<int> nms{&stack[v.arg_0()], &stack[v.arg_n() + 1]};
                        stack[v.result()] = names_names.add(nms);
                    }
                    /* END OF RULE SEMANTICS */
                    break;
                }
                case MARPA_STEP_NULLING_SYMBOL: {
                    int res    = v.result();
                    stack[res] = v.token_value(); 
                    break;
                }
                case MARPA_STEP_INACTIVE:
                    goto END;
            }
        }
        END: ;
    }

    std::cout << pre_block << "\n";
    std::cout << post_block << "\n";
    return 0;
}

