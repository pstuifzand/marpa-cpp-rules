#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <iomanip>
#include "util.h"
#include "marpa-cpp/marpa.hpp"
#include "symbol_table.h"
#include "error.h"
#include "read_file.h"
#include "grammar_bits.h"
#include "evaluator.h"

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
    indexed_table<grammar_rule>& rules,
    const indexed_table<std::string>& names,
    const indexed_table<std::vector<int>>& names_names,
    const indexed_table<std::string>& code_blocks,
    const indexed_table<std::string>& strings,
    const indexed_table<token_rule>& token_rules
    ) {

    using std::cout;

    cout << "\tusing rule = marpa::grammar::rule_id;\n";

    int last_lhs   = -1;
    int last_count = 0;

    for (auto& rule : rules) {
        if (rule.lhs == last_lhs) {
            ++last_count;
            rule.lhs_count = last_count;
        }
        else {
            last_lhs = rule.lhs;
            last_count = 0;
            rule.lhs_count = last_count;
        }
    }
    
    for (auto rule : rules) {
        if (rule.rhs.min == 2) {
            cout << "\trule rule_id_" << names[rule.lhs] << "_" << rule.lhs_count << ";\n";
        }
        else {
            cout << "\trule rule_id_" << names[rule.lhs] << ";\n";
        }
    }

    for (auto name : names) {
        cout << "\tmarpa::grammar::symbol_id R_" << std::setw(6) << std::left << name << ";\n";
    }
    cout << "\n\n";

    // generate grammar
    cout << "void create_grammar(marpa::grammar& g) {\n";
    for (auto name : names) {
        cout << "\tR_" << std::setw(6) << std::left << name << " = g.new_symbol();\n";
    }

    for (auto rule : rules) {
        if (rule.rhs.min == 2) {
            cout << "\trule_id_" << names[rule.lhs] << "_" << rule.lhs_count << "  = g.add_rule(R_" << names[rule.lhs] << ", {";
            for (auto j : names_names[rule.rhs.names_names_idx]) {
                cout << "R_" << names[j] << ", ";
            }
            cout << "});\n";
        }
        else {
            cout << "\trule_id_" << names[rule.lhs] << " = g.new_sequence(R_" << names[rule.lhs] << ", R_" << names[rule.rhs.names_names_idx] << ", " << rule.rhs.sep << ", " << rule.rhs.min << ", 0);\n";
        }
    }
    cout << "\tg.start_symbol(R_" << names[0] << ");\n";

    const char* lines[] = {
        "if (g.precompute() < 0) {\t",
        "    std::cout << \"precompute() failed\\n\";\n",
        "    std::cout << marpa_errors[g.error()] << \"\\n\";\n",
        "    std::cout << \"\\n\";\n",
        "    exit(1);\t",
        "}\n",
    };

    for (const char* line : lines) {
        cout << line;
    }

    cout << "}\n";
    
    cout << "void create_tokens(std::vector<std::tuple<std::string, marpa::grammar::symbol_id, int>>& tokens) {\n";
    for (token_rule r : token_rules) {
        cout << "\ttokens.emplace_back(\"" << strings[r.str] << "\", R_" << names[r.lhs] << ", 0);\n";
    }
    cout << "}\n";


    cout << "void evaluate_rules(marpa::grammar& g, marpa::recognizer& r, marpa::value& v, std::vector<int>& stack) {\n";
    cout << "\tusing rule = marpa::grammar::rule_id;\n";
    cout << "\trule rule_id = v.rule();\n";

    // generate evaluators
    int not_first = 0;
    for (auto rule : rules) {
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

        if (not_first) cout << "\telse ";

        cout << "\tif (rule_id == rule_id_" << names[rule.lhs];
        if (rule.rhs.min == 2) {
            cout << "_" << rule.lhs_count;
        }
        cout << ") {\n";
        cout << block << "\n";
        cout << "\t}\n";
        not_first++;
    }
    cout << "\n";
    cout << "}\n";
}

struct context {
    std::string                     pre_block;
    std::string                     post_block;
    indexed_table<grammar_rule>     rules;
    indexed_table<grammar_rhs>      lrhs;
    indexed_table<std::vector<int>> names_names;
    indexed_table<token_rule>       token_rules;
    indexed_table<std::string>      names;
    indexed_table<std::string>      strings;
    indexed_table<std::string>      code_blocks;

    void initial() {
        lrhs.add(grammar_rhs{});
    }
    void inactive() {}

    int convert(int token_value) { return token_value; }
};

void func_rules(context* ctxt, int* first, int* last, int* out) {
    std::cout << "Output rules\n";
    //std::cout << "* This file is generated from " << argv[1] << " by " << argv[0] << ", do not edit. */\n";
    std::cout << ctxt->pre_block << "\n";
    output_rules(ctxt->rules, ctxt->names, ctxt->names_names, ctxt->code_blocks, ctxt->strings, ctxt->token_rules);
    std::cout << ctxt->post_block << "\n";
}

void func_lhs_op_rhs(context* ctxt, int* first, int* last, int* out) {
    int lhs = *first;
    first++;
    first++;
    int rhs = *first;
    first++;
    grammar_rhs rhs_s = ctxt->lrhs[rhs];
    ctxt->rules.add(grammar_rule{lhs, rhs_s, 0});
}

void func_lhs_op_rhs_code(context* ctxt, int* first, int* last, int* out)
{
    int lhs = *first; first++;
    first++;
    int rhs = *first; first++;
    int code = *first++;
    grammar_rhs rhs_s = ctxt->lrhs[rhs];
    ctxt->rules.add(grammar_rule{ lhs, rhs_s, code });
}

void func_lhs_strop_string(context* ctxt, int* first, int* last, int* out) {
    int lhs = *first; first++;
    first++;
    int str = *first; first++;
    ctxt->token_rules.add(token_rule{ lhs, str });
}

void func_name(context* ctxt, int* first, int* last, int* out) {
    *out = *first;
}

void func_names(context* ctxt, int* first, int* last, int* out) {
    int n = *first;
    *out = ctxt->lrhs.add(grammar_rhs{n, 2, -1 });
}

void func_name_min(context* ctxt, int* first, int* last, int* out) {
    int n = *first; first++;
    int min = *first; first++;
    *out = ctxt->lrhs.add(grammar_rhs{n, min, -1 });
}

void func_null(context* ctxt, int* first, int* last, int* out) {
    *out = 0;
}

void func_name_min_sep(context* ctxt, int* first, int* last, int* out) {
    int n = *first; first++;
    int min = *first; first++;
    int sep = *first; first++;
    *out = ctxt->lrhs.add(grammar_rhs{n, min, sep});
}

void func_names_seq(context* ctxt, int* first, int* last, int* out) {
    std::vector<int> nms{first, last};
    *out = ctxt->names_names.add(nms);
}

struct grammar_symbols {
    marpa::grammar::symbol_id R_rules, R_rule, R_lhs, R_rhs, R_names, R_nquote;
    marpa::grammar::symbol_id T_bnfop, T_name, T_min, T_null, T_code, T_strop, T_string;

    grammar_symbols(marpa::grammar& g) :
        R_rules(g.new_symbol()), R_rule(g.new_symbol()), R_lhs(g.new_symbol()),
        R_rhs(g.new_symbol()), R_names(g.new_symbol()), R_nquote(g.new_symbol()),
        T_bnfop(g.new_symbol()), T_name(g.new_symbol()), T_min(g.new_symbol()),
        T_null(g.new_symbol()), T_code(g.new_symbol()), T_strop(g.new_symbol()),
        T_string(g.new_symbol())
    {
    }
};


void tokenize(context& ctxt, marpa::recognizer& r, const std::string& input, const grammar_symbols& rt)
{
    std::string code_start{"{{"};
    std::string code_end{"}}"};

    ctxt.strings.add("");
    ctxt.code_blocks.add(""); // empty code block

    std::string sep = "%%";
    auto sep_pos = std::search(input.begin(), input.end(), sep.begin(), sep.end());
    if (sep_pos == input.end()) {
        // missing %%
    }

    ctxt.pre_block = std::string{input.begin(), sep_pos };
    std::string::const_iterator it = sep_pos+2;

    sep_pos = std::search(it, input.end(), sep.begin(), sep.end());
    if (sep_pos == input.end()) {
        //missing %%
    }

    ctxt.post_block = std::string{ sep_pos + 2, input.end() };

    std::vector<std::tuple<std::string, marpa::grammar::symbol_id, int>> tokens{
        std::make_tuple("::=",  rt.T_bnfop, 0),
        std::make_tuple("null", rt.T_null,  0),
        std::make_tuple("*",    rt.T_min,   0),
        std::make_tuple("+",    rt.T_min,   1),
        std::make_tuple("~",    rt.T_strop, 0),
    };

    while (it != sep_pos) {

        it = skip(it, sep_pos, isspace);

        if (*it == '#') {
            it = std::find(it, sep_pos, '\n');
            if (it != sep_pos) ++it;
            continue;
        }

        if (isalpha(*it)) {
            auto begin = it;
            it = std::find_if_not(begin, sep_pos, isalpha);
            std::string id(begin, it);
            int idx = ctxt.names.add(id);
            r.read(rt.T_name, idx, 1);
            continue;
        }
        if (*it == '"') {
            it++;
            auto begin = it;
            it = std::find_if_not(begin, sep_pos, [](char v) { return v != '"'; });
            std::string str(begin, it);
            int idx = ctxt.strings.add(str);
            r.read(rt.T_string, idx, 1);
            it++;
            continue;
        }

        bool found = false;
        for (auto t : tokens) {
            auto new_it = match(it, sep_pos, std::get<0>(t).cbegin(), std::get<0>(t).cend());
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
            r.read(rt.T_code, ctxt.code_blocks.add(code_block), 1);
            it = end + 2;
            continue;
        }

        if (it == sep_pos) {
            break;
        }

        std::cout << "Unknown tokens starting here\n[" << std::string(it, sep_pos) << "]\n";
        exit(1);
    }
}

int main(int argc, char** argv)
{
    marpa::grammar g;

    /* DEFINE GRAMMAR */
    grammar_symbols rt{g};

    g.start_symbol(rt.R_rules);

    using rule = marpa::grammar::rule_id;

    rule rule_id_rules   = g.new_sequence(rt.R_rules, rt.R_rule, -1, 1, 0);
    rule rule_id_rule_0  = g.add_rule(rt.R_rule, { rt.R_lhs, rt.T_bnfop, rt.R_rhs });
    rule rule_id_rule_1  = g.add_rule(rt.R_rule, { rt.R_lhs, rt.T_bnfop, rt.R_rhs, rt.T_code });
    rule rule_id_rule_2  = g.add_rule(rt.R_rule, { rt.R_lhs, rt.T_strop, rt.T_string });
    rule rule_id_lhs_0   = g.add_rule(rt.R_lhs,  { rt.T_name });
    rule rule_id_rhs_0   = g.add_rule(rt.R_rhs,  { rt.R_names });
    rule rule_id_rhs_1   = g.add_rule(rt.R_rhs,  { rt.T_name, rt.T_min });
    rule rule_id_rhs_2   = g.add_rule(rt.R_rhs,  { rt.T_null });
    rule rule_id_rhs_3   = g.add_rule(rt.R_rhs,  { rt.T_name, rt.T_min, rt.T_name });
    rule rule_id_names_0 = g.new_sequence(rt.R_names, rt.T_name, -1, 1, 0);

    /* END OF GRAMMAR */

    if (g.precompute() < 0) {
        std::cout << "precompute() failed\n";
        std::cout << marpa_errors[g.error()] << "\n";
        std::cout << "\n";
        exit(1);
    }

    marpa::recognizer r{g};

    context ctxt;

    /* READ TOKENS */
    std::string input;

    read_file(argv[1], input);

    tokenize(ctxt, r, input, rt);

    marpa::bocage b{r, r.latest_earley_set()};
    if (g.error() != MARPA_ERR_NONE) {
        std::cout << marpa_errors[g.error()] << "\n";
        return 1;
    }

    marpa::order o{b};
    marpa::tree t{o};

    /* Evaluate trees */
    while (t.next() >= 0) {
        marpa::value v{t};
        g.set_valued_rules(v);

        evaluator<int, context> e(&ctxt);

        e.set_rule_func(rule_id_rules, func_rules);
        e.set_rule_func(rule_id_rule_0, func_lhs_op_rhs);
        e.set_rule_func(rule_id_rule_1, func_lhs_op_rhs_code);
        e.set_rule_func(rule_id_rule_2, func_lhs_strop_string);
        e.set_rule_func(rule_id_lhs_0, func_name);
        e.set_rule_func(rule_id_rhs_0, func_names);
        e.set_rule_func(rule_id_rhs_1, func_name_min);
        e.set_rule_func(rule_id_rhs_2, func_null);
        e.set_rule_func(rule_id_rhs_3, func_name_min_sep);
        e.set_rule_func(rule_id_names_0, func_names_seq);

        evaluate_steps(&e, v);
    }

    return 0;
}
