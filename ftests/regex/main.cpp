#include <iostream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <regex.h>

namespace rndfa {
    unsigned s_num = 0; /*unless*/

    const unsigned ANY_CHARACTER = 1 << 8;
    const unsigned SPLIT = 1 << 9;
    const unsigned CAPTURE_OPEN = 1 << 10;
    const unsigned CAPTURE_CLOSE = 1 << 11;

    struct StateBase
    {
        StateBase(unsigned c, StateBase * out1 = 0, StateBase * out2 = 0)
        : c(c)
        , id(0)
        , id_trace(0)
        , num(s_num++)
        , out1(out1)
        , out2(out2)
        {}

        virtual ~StateBase(){}

        virtual bool check(int c) const
        {
            /**///std::cout << num << ": " << char(this->c & ANY_CHARACTER ? '.' : this->c&0xFF);
            return (this->c & ANY_CHARACTER) || (this->c&0xFF) == c;
        }

        unsigned c;
        int id;
        int id_trace;
        unsigned num; /*unless*/

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

        virtual ~StateRange(){}

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

        virtual ~StateCharacters(){}

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

    struct StateCapture : StateBase
    {
        /*
         * CAPTURE_OPEN: begin_str, idx_prev_info
         * CAPTURE_CLOSE: end_str, num_prev_capture
         */
        struct Data {
            const char * s;
            unsigned idx_prev;
            unsigned previous_num_cap;
            unsigned previous_idx_tab;
        };

        StateCapture(bool is_open, unsigned num_cap, StateBase* out1 = 0)
        : StateBase(is_open ? CAPTURE_OPEN : CAPTURE_CLOSE, out1)
        , infos(new Data[20]) ///TODO dynamic 20 => ~num node
        , num(num_cap)
        {}

        virtual ~StateCapture(){}

        /*
         * CAPTURE_OPEN: begin_str, idx_prev_info
         * CAPTURE_CLOSE: end_str, num_prev_capture
         */
        Data * infos;
        unsigned num;
    };

    struct StateMachine {
        StateBase * first;
        StateBase * last;

        //state
        //single
        //...
    };




    State * state(State * out1, State * out2 = 0) {
        return new State(SPLIT, out1, out2);
    }

    State * single(int c, State * out = 0) {
        return new State(c, out);
    }

    State * any(State * out = 0) {
        return new State(ANY_CHARACTER, out);
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

    struct StateStep {
//         StateStep(State * state, int cap)
//         : st(state)
//         , cap(cap)
//         {}

        StateBase * st;
        unsigned idx;
    };

    struct state_list_t {
        //StateBase* array[20];
        StateStep array[20]; //TODO dynamic 20 => num node
        unsigned pos;

        state_list_t()
        : pos(0)
        {}

        void push_back(StateBase* val, unsigned idx)
        {
            this->array[this->pos].st = val;
            this->array[this->pos].idx = idx;
            ++this->pos;
        }

        StateStep& operator[](int n)
        { return this->array[n]; }

        void reserve(int) const
        {}

        std::size_t size() const
        { return this->pos; }

        void clear()
        { this->pos = 0; }
    };

    struct StateTrace {
        unsigned size;
        StateTrace()
        :size(0)
        {}
    };

    StateTrace * trace = 0;
    unsigned * step_free = 0;
    unsigned * pstep_free = 0;

    unsigned next_idx_free(unsigned cp_idx)
    {
        --pstep_free;
        if (pstep_free < step_free) {
            std::cerr << __LINE__ << std::endl; throw 0;
        }
        StateTrace * from = trace + cp_idx * s_num;
        StateTrace * to = trace + *pstep_free * s_num;
        //memcpy
        for (StateTrace * last = to + s_num; to < last; ++to, ++from) {
            *to = *from;
        }
        return *pstep_free;
    }

    void free_idx(unsigned n)
    {
        if (pstep_free > step_free+s_num) {
            std::cerr << __LINE__ << std::endl; throw 0;
        }
        *pstep_free = n;
        ++pstep_free;
    }

    void addstate(state_list_t& l, StateBase *st, int n, unsigned idx, bool b = false)
    {
        if (st && st->id != n) {
            if (st->c == SPLIT) {
                addstate(l, st->out1, n, idx, b);
                addstate(l, st->out2, n, idx, true);
            }
            else if (st->c & CAPTURE_OPEN) {
                //trace[idx * s_num + st->num].size = 1;
                addstate(l, st->out1, n, idx, b);
            }
            else if (st->c & CAPTURE_CLOSE) {
                //trace[idx * s_num + st->num].size = 1;
                addstate(l, st->out1, n, idx, b);
            }
            else {
                /**///std::cout << ("  add");
                st->id = n;
                if (b) {
                    idx = next_idx_free(idx);
                }
                l.push_back(st, idx);
            }
        }
    }

    void step(state_list_t& clist, const char *s, state_list_t& nlist, int n)
    {
        for (std::size_t i = 0; i < clist.size(); ++i) {
            /**///std::cout << ("!!!\n");
            StateStep& sst = clist[i];

            if (sst.st->check(*s)) {
                ++trace[sst.idx * s_num + sst.st->num].size;
                /**///std::cout << ("  -- ok");
                addstate(nlist, sst.st->out1, n, sst.idx);
            }/*
            else {
                free_idx(sst.idx);
            }*/
            /**///std::cout << "\n";
        }
        for (std::size_t i = 0; i < clist.size(); ++i) {
            bool b = false;
            for (std::size_t ii = 0; ii < nlist.size(); ++ii) {
                if (clist[i].idx == nlist[ii].idx) {
                    b = true;
                    break;
                }
            }
            if (!b) {
                free_idx(clist[i].idx);
            }
        }
        /**///std::cout << (pstep_free - step_free) << " " << (nlist.size()) << std::endl;
    }

    unsigned idx_trace;

    bool exact_match(StateBase * st, const State * last, const char * s) {
        if (!st) {
            return false;
        }

        int n = 1;

        //s_num *= 2; ///TODO what Oo ??
        step_free = new unsigned[s_num];
        pstep_free = step_free;
        for (unsigned i = 0; i < s_num; ++i, ++pstep_free) {
            *pstep_free = i;
        }
        trace = new StateTrace[s_num * s_num];

        state_list_t l1, l2, *pl1 = &l1, *pl2 = &l2;
        //l1.reserve(20);
        //l2.reserve(20);

        //std::size_t max = l1.size();

        /**///std::cout << (pstep_free - step_free) << std::endl;
        addstate(l1, st, n, *--pstep_free);
        /**///std::cout << (pstep_free - step_free) << " " << (l1.size()) << std::endl;

        //std::cout << ("start\n") << std::endl;

        for(; *s && 0 == last->id; ++s){
            /**///std::cout << "\nc: " << *s << "\n";
            step(*pl1, s, *pl2, ++n);
            pl1->clear();
            std::swap<>(pl1, pl2);
            //max = std::max(pl1->size(), max);
        }

        if (last->id != 0) {
            for (std::size_t i = 0; i < pl1->size(); ++i) {
                if ((*pl1)[i].st == last) {
                    idx_trace = (*pl1)[i].idx;
                    break;
                }
            }
        }

        ////BEGIN check: (pstep_free - step_free) == s_num
        //for (std::size_t i = 0; i < pl1->size(); ++i) {
        //    free_idx((*pl1)[i].idx);
        //}
        //std::cout << (pstep_free - step_free) << std::endl;
        ////END check

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
        State any(ANY_CHARACTER);
        State more(SPLIT, &any, st);
        any.out1 = &more;
        return exact_match(&more, last, s);
    }
}

void nfa_in_vec(std::vector<rndfa::State*> & vec,
                std::vector<rndfa::StateCapture*> & vec_match,
                rndfa::State * st){
    if (st && st->id == 0) {
        st->id = 1;
        nfa_in_vec(vec, vec_match, st->out1);
        if (st->c == rndfa::SPLIT) {
            nfa_in_vec(vec, vec_match, st->out1);
            nfa_in_vec(vec, vec_match, st->out2);
        }
        else {
            vec.push_back(st);
            if (st->c & (rndfa::CAPTURE_OPEN|rndfa::CAPTURE_CLOSE)) {
                vec_match.push_back((rndfa::StateCapture*)st);
            }
        }
    }
}

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

    using namespace rndfa;

    //State last('\0');
//     State last(ANY_CHARACTER);
//     StateRange digit('0', '9');
// //     State& last = digit;
//     State one_more(SPLIT, &digit, &last);
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
//                                        zero_or_more(ANY_CHARACTER,
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
//                                 zero_or_more(ANY_CHARACTER,
//                                              single('s',
//                                                     &digit)
//                                              )
//                                 )
//                      )
//                );
    State last(ANY_CHARACTER);
    State char_a('a', &last);

    State * st = zero_or_more(
        ANY_CHARACTER,
        single(
            ' ',
            zero_or_more(
                ANY_CHARACTER,
                single(
                    ' ',
                    new StateCapture(
                        true, 1,
                        zero_or_more(
                            ANY_CHARACTER,
                            new StateCapture(
                                false, 1,
                                single(
                                    ' ',
                                    new StateCapture(
                                        true, 2,
                                        zero_or_more(
                                            ANY_CHARACTER,
                                            new StateCapture(
                                                false, 2,
                                                single(
                                                    ' ',
                                                    zero_or_more(
                                                        ANY_CHARACTER,
                                                        &char_a
                                                    )
                                                )
                                            )
                                        )
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
    std::vector<StateCapture*> vec_match;
    nfa_in_vec(vec, vec_match, st);


    bool ismatch1 = false;
    bool ismatch2 = false;
    bool ismatch3 = false;
    double d1, d2, d3;
    //std::streambuf * dbuf = std::cout.rdbuf(0);
    const char * str = argc == 2 ? argv[1] : "abcdef";
    {
        regex_t rgx;
        //if (0 != regcomp(&rgx, "^.*b(a*b?a|[uic].*s)[0-9].*$", REG_EXTENDED)){
        if (0 != regcomp(&rgx, "^.* .* .* .* .*a$", REG_EXTENDED)){
            std::cout << ("comp error") << std::endl;
        }
        std::clock_t start_time = std::clock();
        regmatch_t regmatch;
        for (size_t i = 0; i < 1/*00000*/; ++i) {
            ismatch1 = 0 == regexec(&rgx, str, 1, &regmatch, 0);
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
        for (size_t i = 0; i < 1/*00000*/; ++i) {
            ismatch2 = 0 == regexec(&rgx, str, 1, &regmatch, 0);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 1/*00000*/; ++i) {
            for (std::vector<State*>::iterator first = vec.begin(), vlast = vec.end(); first != vlast; ++first) {
                (*first)->id = 0;
            }
            ismatch3 = match(st, &last, str);
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
    << "\n\nid_trace: " << idx_trace
    << "\n"<<std::endl;

    if (last.id != 0) {
        unsigned p = 0;
        StateTrace * strace = trace + idx_trace * s_num + s_num - 1;
        for (StateTrace * last = strace - s_num; strace > last; --strace) {
            if (strace->size) {
                (std::cout << strace->size << ": \"").write(str+p, strace->size) << "\"\n";
                p += strace->size;
            }
        }
    }
}
