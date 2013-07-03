#include <iostream>
#include <vector>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <regex.h>


namespace rndfa {
    int s_num = 0; /*unless*/
    struct StateBase
    {
        static const int ANY_CHARACTER = 1 << 8;
        static const int SPLIT = 1 << 9;

        StateBase(int c, StateBase * out1 = 0, StateBase * out2 = 0)
        : c(c)
        , id(0)
        , num(s_num++)
        , out1(out1)
        , out2(out2)
        {}

        virtual bool check(int c) const
        {
            /**///std::cout << num << ": " << char(this->c & StateBase::ANY_CHARACTER ? '.' : this->c&0xFF);
            return (this->c & StateBase::ANY_CHARACTER) || (this->c&0xFF) == c;
        }

        int c;
        int id;
        int num; /*unless*/
        StateBase *out1;
        StateBase *out2;
    };

    typedef StateBase State;

    struct StateRange : StateBase
    {
        StateRange(int r1, int r2, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(r1, out1, out2)
        , rend(r2)
        {}

        virtual bool check(int c) const
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return (this->c&0xFF) <= c && c <= rend;
        }

        int rend;
    };

    State * range(char c1, char c2, StateBase* out = 0) {
        return new StateRange(c1, c2, out);
    }

    struct StateCharacters : StateBase
    {
        StateCharacters(const char * s, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(*s, out1, out2)
        , str(s)
        {}

        virtual bool check(int c) const
        {
            /**///std::cout << str;
            return strchr(this->str, c) != 0;
        }

        const char * str;
    };

    State * characters(const char * s, StateBase* out = 0) {
        return new StateCharacters(s, out);
    }

    struct StateOpen : StateBase
    {
        StateOpen(StateBase* out1 = 0)
        : StateBase(0, out1, out2)
        {}

        virtual bool check(int c) const
        {
        }
    };

    struct StateMachine {
        StateBase * first;
        StateBase * last;

        //state
        //single
        //...
    };




    State * state(State * out1, State * out2 = 0) {
        return new State(State::SPLIT, out1, out2);
    }

    State * single(int c, State * out = 0) {
        return new State(c, out);
    }

    State * any(State * out = 0) {
        return new State(State::ANY_CHARACTER, out);
    }

    State * zero_or_more(int c, State * out = 0) {
        State * ret = state(single(c), out);
        ret->out1->out1 = ret;
        return ret;
    }

    State * zero_or_one(int c, State * out = 0) {
        return state(single(c, out), out);
    }

    State * one_or_more(int c, State * out = 0) {
        return zero_or_more(c, out)->out1;
    }

    struct state_list_t {
        StateBase* array[20];
        unsigned pos;

        state_list_t()
        : pos(0)
        {}

        void push_back(StateBase* val)
        {
            this->array[this->pos++] = val;
        }

        StateBase * operator[](int n) const
        { return this->array[n]; }

        void reserve(int) const
        {}

        std::size_t size() const
        { return this->pos; }

        void clear()
        { this->pos = 0; }
    };

    void addstate(state_list_t& l, StateBase *st, int n)
    {
        if (st && st->id != n) {
            if (st->c == State::SPLIT) {
                addstate(l, st->out1, n);
                addstate(l, st->out2, n);
            }
            else {
                /**///std::cout << ("  add");
                st->id = n;
                l.push_back(st);
            }
        }
    }

    void step(state_list_t& clist, char c, state_list_t& nlist, int n)
    {
        for (std::size_t i = 0; i < clist.size(); ++i) {
            StateBase * st = clist[i];

            //std::cout << "check " << c << " with " << st->c << " (" << char(st->c) << ")\n";
            if (st->check(c)) {
                /**///std::cout << ("  -- ok");
                addstate(nlist, st->out1, n);
            }
            /**///std::cout << "\n";
        }
    }

    bool exact_match(StateBase * st, const State * last, const char * s) {
        if (!st) {
            return false;
        }

        int n = 1;

        state_list_t l1, l2, *pl1 = &l1, *pl2 = &l2;
        //l1.reserve(20);
        //l2.reserve(20);

        //std::size_t max = l1.size();

        addstate(l1, st, n);

        //std::cout << ("start\n") << std::endl;

        for(; *s && 0 == last->id; ++s){
            /**///std::cout << "\nc: " << *s << "\n";
            step(*pl1, *s, *pl2, ++n);
            pl1->clear();
            std::swap<>(pl1, pl2);
            //max = std::max(pl1->size(), max);
        }

        /**///std::cout << *s << "  -- n: " << (n) << std::endl;

        //std::cout << "max: " << (max) << std::endl;

//         for (std::size_t i = 0; i < pl1->size(); ++i) {
//             if (0 == (*pl1)[i]->out1) {
//                 return true;
//             }
//         }
        return last->id != 0;
    }

    bool match(StateBase * st, const State * last, const char * s) {
        State any(State::ANY_CHARACTER);
        State more(State::SPLIT, &any, st);
        any.out1 = &more;
        return exact_match(&more, last, s);
    }
}

void nfa_in_vec(std::vector<rndfa::State*> & vec, rndfa::State * st){
    if (st && st->id == 0) {
        st->id = 1;
        nfa_in_vec(vec, st->out1);
        if (st->c == rndfa::State::SPLIT) {
            nfa_in_vec(vec, st->out1);
            nfa_in_vec(vec, st->out2);
        }
        else {
            vec.push_back(st);
        }
    }

}

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

    using namespace rndfa;

    //State last('\0');
//     State last(State::ANY_CHARACTER);
//     StateRange digit('0', '9');
// //     State& last = digit;
//     State one_more(State::SPLIT, &digit, &last);
//     digit.out1 = &one_more;
    //(ba*b?a|a*b[uic].*s)[0-9]
//     State * st = state(
//         single('b',
//                zero_or_more('a',
//                             zero_or_one('b',
//                                         single('a',
//                                                &digit)))),
//         zero_or_more('a',
//                      single('b',
//                             characters("uic",
//                                        zero_or_more(State::ANY_CHARACTER,
//                                                     single('s',
//                                                            &digit)))))
//     );
    //b(a*b?a|[uic].*s)[0-9]*
//     State * st =
//         single('b',
//                state(
//                    zero_or_one('a',
//                                zero_or_one('b',
//                                            single('a',
//                                                   &digit)
//                                           )
//                               ),
//                      characters("uic",
//                                 zero_or_more(State::ANY_CHARACTER,
//                                              single('s',
//                                                     &digit)
//                                              )
//                                 )
//                      )
//                );
    State last('a');
    State * st = zero_or_more(
        State::ANY_CHARACTER,
        single(
            ' ',
            zero_or_more(
                State::ANY_CHARACTER,
                single(
                    ' ',
                    zero_or_more(
                        State::ANY_CHARACTER,
                        single(
                            ' ',
                            zero_or_more(
                                State::ANY_CHARACTER,
                                single(
                                    ' ',
                                    zero_or_more(
                                        State::ANY_CHARACTER,
                                        &last
                                    )
                                )
                            )
                        )
                    )
                )
            )
        )
    );

    std::vector<State*> vec;
    nfa_in_vec(vec, st);


    bool ismatch1 = false;
    bool ismatch2 = false;
    bool ismatch3 = false;
    double d1, d2, d3;
    //std::streambuf * dbuf = std::cout.rdbuf(0);
    {
        regex_t rgx;
        //if (0 != regcomp(&rgx, "^.*b(a*b?a|[uic].*s)[0-9].*$", REG_EXTENDED)){
        if (0 != regcomp(&rgx, "^.* .* .* .* .*a$", REG_EXTENDED)){
            std::cout << ("comp error") << std::endl;
        }
        std::clock_t start_time = std::clock();
        regmatch_t regmatch;
        for (size_t i = 0; i < 100000; ++i) {
            ismatch1 = 0 == regexec(&rgx, argc == 2 ? argv[1] : "abcdef", 1, &regmatch, 0);
        }
        d1 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        regex_t rgx;
        //if (0 != regcomp(&rgx, "ba*b?a|b?a*b?", REG_EXTENDED)){
        if (0 != regcomp(&rgx, ".* .* .* .* .*a", REG_EXTENDED)){
            std::cout << ("comp error") << std::endl;
        }
        std::clock_t start_time = std::clock();
        regmatch_t regmatch;
        for (size_t i = 0; i < 100000; ++i) {
            ismatch2 = 0 == regexec(&rgx, argc == 2 ? argv[1] : "abcdef", 1, &regmatch, 0);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            for (std::vector<State*>::iterator first = vec.begin(), last = vec.end(); first != last; ++first) {
                (*first)->id = 0;
            }
            ismatch3 = match(st, &last, argc == 2 ? argv[1] : "abcdef");
        }
        d3 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::cout.rdbuf(dbuf);

    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);
    std::cout
    << (ismatch1 ? "good\n" : "fail\n")
    << d1 << "\n"
    << (ismatch2 ? "good\n" : "fail\n")
    << d2 << "\n"
    << (ismatch3 ? "good\n" : "fail\n")
    << d3 << "\n"
    ;
}
