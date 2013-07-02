#include <iostream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <regex.h>


namespace rndfa {
    struct State
    {
        static const int ANY_CHARACTER = 1 << 8;
        static const int SPLIT = 1 << 9;

        State(int c, State * out1 = 0, State * out2 = 0)
        : c(c)
        , out1(out1)
        , out2(out2)
        {}

        int c;
        State *out1;
        State *out2;
    };

    struct StateMachine {

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
        State * ret = new State(c | State::SPLIT, 0, out);
        ret->out1 = ret;
        return ret;
    }

    State * zero_or_one(int c, State * out = 0) {
        return new State(c | State::SPLIT, out, out);
    }

    State * one_or_more(int c, State * out = 0) {
        State * ret = single(c, state(0, out));
        ret->out1->out1 = ret;
        return ret;
    }

    State * zero_or_more(State * state_in, State * state_out, State * out = 0) {
        State * ret = state(state_in, out);
        state_out->out1 = ret;
        return ret;
    }

    State * zero_or_one(State * state_in, State * state_out, State * out = 0) {
        State * ret = state(state_in, out);
        state_out->out1 = out;
        return ret;
    }

    State * one_or_more(State * state_in, State * state_out, State * out = 0) {
        State * ret = state(state_in, out);
        state_out->out1 = ret;
        return state_in;
    }

    struct state_list_t {
        const State* array[20];
        unsigned pos;

        state_list_t()
        : pos(0)
        {}

        void push_back(const State* val)
        {
            this->array[this->pos++] = val;
        }

        const State * operator[](int n) const
        { return this->array[n]; }

        void reserve(int) const
        {}

        std::size_t size() const
        { return this->pos; }

        void clear()
        { this->pos = 0; }
    };

    void addstate(state_list_t& l, const State *st)
    {
        if (st->c & State::SPLIT) {
            //std::cout << ("next\n");
            if (st->c != State::SPLIT) {
                //std::cout << ("push\n");
                l.push_back(st);
            }
            else {
                addstate(l, st->out1);
            }
            addstate(l, st->out2);
        }
        else {
            //std::cout << ("push\n");
            l.push_back(st);
        }
    }

    void step(state_list_t& clist, char c, state_list_t& nlist)
    {
        for (std::size_t i = 0; i < clist.size(); ++i) {
            const State * st = clist[i];

            //std::cout << "check " << c << " with " << st->c << " (" << char(st->c) << ")\n";
            if ((st->c & State::ANY_CHARACTER) || (st->c&0xFF) == c) {
                //std::cout << ("ok\n");
                addstate(nlist, st->out1);
            }
        }
    }

    bool match(const State * st, const char * s) {
        if (!st) {
            return false;
        }
        state_list_t l1, l2, *pl1 = &l1, *pl2 = &l2;
        //l1.reserve(20);
        //l2.reserve(20);

        //std::size_t max = l1.size();

        addstate(l1, st);

        //std::cout << ("start\n") << std::endl;

        for(; *s; ++s){
            step(*pl1, *s, *pl2);
            pl1->clear();
            std::swap<>(pl1, pl2);
            //std::cout << ("++s: ") << *(s+1) << "\n\n";
            //max = std::max(pl1->size(), max);
        }

        //std::cout << "max: " << (max) << std::endl;

        for (std::size_t i = 0; i < pl1->size(); ++i) {
            if (0 == (*pl1)[i]->out1) {
                return true;
            }
        }
        return false;
    }
}

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

    using namespace rndfa;

    State last('\0');
    //ba*b?a|a*b.*s
    State * st = state(
        single('b',
               zero_or_more('a',
                            zero_or_one('b',
                                        single('a',
                                               &last)))),
        zero_or_more('a',
                     single('b',
                            zero_or_more(State::ANY_CHARACTER,
                                         single('s',
                                                &last))))
    );

    bool ismatch1 = false;
    bool ismatch2 = false;
    double d1, d2;
    //std::streambuf * dbuf = std::cout.rdbuf(0);
    {
        regex_t rgx;
        if (0 != regcomp(&rgx, "^ba*b?a$|^a*b.*s$", REG_EXTENDED)){
            std::cout << ("comp error") << std::endl;
        }
        std::clock_t start_time = std::clock();
        regmatch_t regmatch;
        for (size_t i = 0; i < 100000; ++i) {
            ismatch1 = 0 == regexec(&rgx, argc == 2 ? argv[1] : "abcdef", 1, &regmatch, REG_NOTBOL|REG_NOTEOL);
        }
        d1 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch2 = match(st, argc == 2 ? argv[1] : "abcdef");
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::cout.rdbuf(dbuf);

    std::cout
    << (ismatch1 ? "good\n" : "fail\n")
    << d1 << "\n"
    << (ismatch2 ? "good\n" : "fail\n")
    << d2 << "\n"
    ;
}
