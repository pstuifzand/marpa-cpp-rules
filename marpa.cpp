#include <algorithm>
#include <vector>
#include "marpa.h"

namespace marpa {

grammar::grammar()
    : handle(marpa_g_new(0)) {
}

grammar::grammar(const grammar& g) {
    marpa_g_ref(g.handle);
    this->handle = g.handle;
}

grammar& grammar::operator=(const grammar& g) {
    marpa_g_ref(g.handle);
    this->handle = g.handle;
    return *this;
}

grammar::~grammar() {
    marpa_g_unref(handle);
}

void grammar::set_valued_rules(value& v) {
    for (auto r : rules) {
        v.rule_is_valued(r, 1);
    }
}

}
