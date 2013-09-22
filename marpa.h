#ifndef MARPA_H
#define MARPA_H

extern "C" {
#include <marpa.h>
#include <marpa_api.h>
}

namespace marpa {

class grammar {
    public:
        typedef int earleme;
        typedef int error_code;
        typedef int symbol_id;
        typedef int rule_id;
        typedef int event_type;
        typedef int rank;

    private:
        typedef Marpa_Grammar handle_type;
    public:
        grammar() : handle(marpa_g_new(0)) {}

        grammar(const grammar& g) {
            marpa_g_ref(g.handle);
            this->handle = g.handle;
        }

        grammar& operator=(const grammar& g) {
            marpa_g_ref(g.handle);
            this->handle = g.handle;
            return *this;
        }

        ~grammar() {
            marpa_g_unref(handle);
        }

        symbol_id start_symbol() const {
            return marpa_g_start_symbol(handle);
        }

        symbol_id start_symbol(symbol_id s) {
            return marpa_g_start_symbol_set(handle, s);
        }

        symbol_id new_symbol() {
            return marpa_g_symbol_new(handle);
        }

        rule_id new_rule(symbol_id lhs_id, symbol_id* rhs_ids, int length) {
            return marpa_g_rule_new(handle, lhs_id, rhs_ids, length);
        }

        rule_id add_rule(symbol_id lhs_id, std::initializer_list<symbol_id> rhs) {
            grammar::rule_id rhs_ids[rhs.size()]; 
            std::copy(rhs.begin(), rhs.end(), rhs_ids);
            return new_rule(lhs_id, rhs_ids, rhs.size());
        }

        rule_id new_sequence(symbol_id lhs_id, symbol_id rhs_id, symbol_id separator_id, int min, int flags) {
            return marpa_g_sequence_new(handle, lhs_id, rhs_id, separator_id, min, flags);
        }

        int precompute() {
            return marpa_g_precompute(handle);
        }

        int error() {
            return marpa_g_error(handle, 0);
        }
    public:
        handle_type internal_handle() { return handle; }
    private:
        handle_type handle;
};

class recognizer {
    private:
        typedef Marpa_Recognizer handle_type;

        handle_type handle;
    public:
        typedef int earley_set_id;

        handle_type internal_handle() {
            return handle;
        }
    public:
        recognizer(grammar& g)
            : handle(marpa_r_new(g.internal_handle())) {
            start_input();
        }

        ~recognizer() {
            marpa_r_unref(handle);
        }

        void start_input() {
            marpa_r_start_input(handle);
        }

        int alternative(grammar::symbol_id sym_id, int value, int length) {
            return marpa_r_alternative(handle, sym_id, value, length);
        }

        grammar::earleme earleme_complete() {
            return marpa_r_earleme_complete(handle);
        }

        earley_set_id latest_earley_set() {
            return marpa_r_latest_earley_set(handle);
        }

        int read(grammar::symbol_id sym_id, int value, int length) {
            int error = alternative(sym_id, value, length);
            if (error != MARPA_ERR_NONE) {
                return error;
            }
            //std::cout << sym_id << " " << value << " " << length << "\n";
            return earleme_complete();
        }
    private:
        recognizer& operator=(const recognizer&);
        recognizer(const recognizer&);
};

class bocage {
    private:
        typedef Marpa_Bocage handle_type;
        handle_type handle;
    public:
        handle_type internal_handle() { return handle; }
    public:
        bocage(recognizer& r, recognizer::earley_set_id set_id) : handle(marpa_b_new(r.internal_handle(), set_id)) {}
        ~bocage() { marpa_b_unref(handle); }
    private:
        bocage& operator=(const bocage&);
        bocage(const bocage&);
};

class order {
    private:
        typedef Marpa_Order handle_type;
        handle_type handle;
    public:
        handle_type internal_handle() { return handle; }
    public:
        order(bocage& b) : handle(marpa_o_new(b.internal_handle())) {}
    private:
        order& operator=(const order&);
        order(const order&);
};

class tree {
    private:
        typedef Marpa_Tree handle_type;
        handle_type handle;
    public:
        handle_type internal_handle() { return handle; }
    public:
        tree(order& o) : handle(marpa_t_new(o.internal_handle())) {}

        int next() { return marpa_t_next(handle); }
    private:
        tree& operator=(const tree&);
        tree(const tree&);
};

class value {
    public:
        typedef int step_type;
    private:
        typedef Marpa_Value handle_type;

        handle_type handle;
    public:
        handle_type internal_handle() { return handle; }
    public:
        value(tree& t) : handle(marpa_v_new(t.internal_handle())) {}

        step_type step() { return marpa_v_step(handle); }
        void rule_is_valued(grammar::rule_id rule, int value) { marpa_v_rule_is_valued_set(handle, rule, value); }

        int result() { return marpa_v_result(handle); }
        int arg_0() { return marpa_v_arg_0(handle); }
        int arg_n() { return marpa_v_arg_n(handle); }
        int token_value() { return marpa_v_token_value(handle); }
        int rule() { return marpa_v_rule(handle); }
    private:
        value& operator=(const value&);
        value(const value&);
};

class event {
};

}

#endif
