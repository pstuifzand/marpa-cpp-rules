#ifndef GRAMMAR_BITS_H
#define GRAMMAR_BITS_H

struct grammar_rhs {
    int names_names_idx;
    int min; // 0 == *, 1 == +, 2 == names_names_idx
    int sep;
    friend bool operator==(const grammar_rhs& a, const grammar_rhs& b);
};

bool operator==(const grammar_rhs& a, const grammar_rhs& b) {
    return a.names_names_idx == b.names_names_idx && a.min == b.min;
}

struct grammar_rule {
    int lhs;
    grammar_rhs rhs;
    int code;
    int lhs_count;

    friend bool operator==(const grammar_rule& a, const grammar_rule& b);
};
bool operator==(const grammar_rule& a, const grammar_rule& b) {
    return a.lhs == b.lhs && a.rhs == b.rhs && a.code == b.code;
}

struct token_rule {
    int lhs;
    int str;
    friend bool operator==(const token_rule& a, const token_rule& b);
};

bool operator==(const token_rule& a, const token_rule& b) {
    return a.lhs == b.lhs && a.str == b.str;
}

#endif
