#include <iostream>
#include <vector>

namespace rndfa {
    struct State
    {
        static const int ANY_CHARACTER = 256;
        static const int IGNORE = 257;


        State(int c, State * out1 = 0, State * out2 = 0)
        : c(c)
        , out1(out1)
        , out2(out2)
        {}

        int c;
        State *out1;
        State *out2;
    };

    State * state(State * out1, State * out2 = 0) {
        return new State(State::IGNORE, out1, out2);
    }

    State * single(char c, State * out = 0) {
        return new State(c, out);
    }

    State * any(State * out = 0) {
        return new State(State::ANY_CHARACTER, out);
    }

    State * zero_or_more(char c, State * out = 0) {
        State * ret = new State(c, 0, out);
        ret->out1 = ret;
        return ret;
    }

    State * zero_or_one(char c, State * out = 0) {
        State * ret = single(c, state(0, out));
        ret->out1->out1 = ret;
        return ret;
    }

    State * one_or_more(char c, State * out = 0) {
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
        state_out->out1 = ret;
        return state_in;
    }

    State * one_or_more(State * state_in, State * state_out, State * out = 0) {
        State * ret = state(state_in, out);
        state_out->out1 = state_in;
        state_out->out2 = ret;
        return ret;
    }

    typedef std::vector<State*> state_list_t;

    void addstateout(State * last, state_list_t& l, State *st, bool& ismatch)
    {
        if (st->c == State::IGNORE) {
            std::cout << ("next\n") << std::endl;
            addstateout(last, l, st->out1, ismatch);
            addstateout(last, l, st->out2, ismatch);
        }
        else {
            ismatch = st->out1 == last || 0 == st->out1;
            if (!ismatch) {
                std::cout << (st->out1) << std::endl;
                l.push_back(st->out1);
                ismatch = st->out2 == last || 0 == st->out2;
                if (!ismatch) {
                    std::cout << (st->out2) << std::endl;
                    l.push_back(st->out2);
                }
            }
        }
    }

    void step(State * last, state_list_t& clist, int c, state_list_t& nlist, bool& ismatch)
    {
        for (std::size_t i = 0; i < clist.size(); ++i) {
            State * st = clist[i];

            std::cout << "check " << c << " with " << st->c << " (" << char(st->c) << ")\n";
            if (st->c == State::ANY_CHARACTER || st->c == c) {
                std::cout << ("ok\n") << std::endl;
                addstateout(last, nlist, st, ismatch);
            }
        }
    }

    bool match(State * st, State * last, const char * s) {
        if (!st) {
            return false;
        }
        state_list_t l1, l2, *pl1 = &l1, *pl2 = &l2;
        l1.reserve(10);
        l2.reserve(10);
        bool ismatch = false;

        addstateout(last, l1, st, ismatch);

        for(; *s && !ismatch; s++){
            step(last, *pl1, *s, *pl2, ismatch);
            std::swap<>(pl1, pl2);
            pl1->clear();
        }
        return ismatch;
    }

}

int main(int argc, char **argv) {
    using namespace rndfa;

    std::ios::sync_with_stdio(false);

    State last('\0');
    //ba*b?a|a*b
    State * st = state(
        single('b',
               zero_or_more('a',
                            zero_or_one('b',
                                        single('a',
                                               &last)))),
        zero_or_more('a',
                     single('b',
                            &last))
    );

    std::cout << (
        match(st, &last, argc == 2 ? argv[1] : "abcdef")
        ? "good\n"
        : "fail\n"
    );
}
