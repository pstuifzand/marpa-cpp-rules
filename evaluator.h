#ifndef EVALUATOR_H
#define EVALUATOR_H

template <class T, class C>
class evaluator {
    public:
        typedef T        value_type;
        typedef C        context_type;

        typedef typename std::vector<value_type>::iterator                          iterator;
        typedef typename std::vector<value_type>::const_iterator                    const_iterator;
        typedef void (*function_type) (context_type*, int*, int*, int*);
    private:
        std::vector<value_type>    stack;
        std::vector<function_type> rule_functions;
        context_type*              conv;
    public:
        evaluator(context_type* c) : conv(c) {}

        void initial_step(marpa::value& v) {
            stack.resize(1);
            conv->initial();
        }

        void token_step(marpa::value& v) {
            stack.resize(std::max((std::vector<int>::size_type)v.result()+1, stack.size()));
            auto out = &stack[v.result()];
            *out = conv->convert(v.token_value());
        }

        void rule_step(marpa::value& v) {
            marpa::grammar::rule_id rule = v.rule();
            stack.resize(std::max((std::vector<int>::size_type)v.result()+1, stack.size()));

            auto out = &stack[v.result()];

            auto first  = &stack[v.arg_0()];
            auto last   = &stack[v.arg_n() + 1];

            call_rule_function(rule, first, last, out);
        }

        // not sure...
        void nulling_symbol_step(marpa::value& v) {
            auto out = &stack[v.result()];
            *out = conv->convert(v.token_value());
        }

        void inactive_step(marpa::value& v) {
            conv->inactive();
        }

        void set_rule_func(marpa::grammar::rule_id id, function_type func) {
            rule_functions.resize(id+1);
            rule_functions[id] = func;
        }

        void call_rule_function(marpa::grammar::rule_id rule, int* first, int* last, int* out) {
            rule_functions[rule](conv, first, last, out);
        }

};

template <typename E>
void evaluate_steps(E* e, marpa::value& v) {
    for (;;) {
        marpa::value::step_type type = v.step();

        switch (type) {
            case MARPA_STEP_INITIAL:
                e->initial_step(v);
                break;
            case MARPA_STEP_TOKEN:
                e->token_step(v);
                break;
            case MARPA_STEP_RULE:
                e->rule_step(v);
                break;
            case MARPA_STEP_NULLING_SYMBOL:
                e->nulling_symbol_step(v);
                break;
            case MARPA_STEP_INACTIVE:
                e->inactive_step(v);
                return;
        }
    }
}

#endif
