#include <iostream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <regex.h>
#include <cstdlib>
#include <cassert>
#include <algorithm>

namespace rndfa {
    const unsigned ANY_CHARACTER = 1 << 8;
    const unsigned SPLIT = 1 << 9;
    const unsigned CAPTURE_OPEN = 1 << 10;
    const unsigned CAPTURE_CLOSE = 1 << 11;

    struct StateBase
    {
        StateBase(unsigned c, StateBase * out1 = 0, StateBase * out2 = 0)
        : c(c)
        , id(0)
        , num(0)
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
        unsigned id;
        unsigned num;

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

    //NOTE rename to Regex ?
    class StateMachine
    {
    public:
        explicit StateMachine(StateBase * st)
        : nb_context_state(0)
        , st_first(st)
        , st_last(0)
        , vec()
        , captures()
        , traces(0)
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        {
            this->push_state(st);

            if (!this->vec.empty())
            {
                if (!this->captures.empty()) {
                    this->traces = new Trace[this->nb_context_state * this->nb_context_state * sizeof this->traces[0]];
                    this->idx_trace_free = new unsigned[this->nb_context_state];
                }

                l1.reserve(this->nb_context_state);
                l2.reserve(this->nb_context_state);
            }
        }

        ~StateMachine()
        {
            delete [] this->traces;
            delete [] this->idx_trace_free;
        }

    private:
        struct Trace
        {
            unsigned size;
            Trace()
            :size(0)
            {}
        };

    public:
        typedef std::pair<const Trace *, const Trace *> TraceRange;

        TraceRange get_trace() const
        {
            const Trace * strace = this->traces + this->idx_trace * this->nb_context_state;
            return TraceRange(strace, strace + this->nb_context_state);
        }

        typedef std::pair<unsigned, unsigned> range_t;
        typedef std::vector<range_t> range_list;

        range_list exact_match(const char * s)
        {
            range_list ranges;

            if (this->st_first) {
                if (Matching(*this).exact_match(s)) {
                    this->append_match_result(ranges);
                }
            }

            return ranges;
        }

        bool exact_search(const char * s)
        {
            if (!this->st_first) {
                return false;
            }
            return Matching(*this).exact_match(s); ///TODO exact_search
        }

        bool exact_search_with_trace(const char * s)
        {
            if (!this->st_first) {
                return false;
            }
            return Matching(*this).exact_match(s);
        }

        range_list match_result()
        {
            range_list ret;
            this->append_match_result(ret);
            return ret;
        }

        void append_match_result(range_list& ranges)
        {
            ranges.reserve(this->captures.size());

            typedef std::vector<StateBase*>::reverse_iterator iterator;
            iterator match_first = this->captures.rbegin();
            iterator match_last = this->captures.rend();
            unsigned pmatch = -1u;
            unsigned n = 0;
            unsigned p = 0;
            for (TraceRange trace = this->get_trace(); trace.first < trace.second; ++trace.first, ++n) {
                if (match_first != match_last && n == (*match_first)->num) {
                    if (pmatch == -1u) {
                        pmatch = p;
                    }
                    else {
                        ranges.push_back(range_t(pmatch, p-pmatch));
                        pmatch = -1u;
                    }
                    ++match_first;
                }
                p += trace.first->size;
            }
        }

    private:
        void push_state(StateBase * st){
            if (st && st->id == 0) {
                st->id = -1u;
                this->vec.push_back(st);
                if (st->c != SPLIT) {
                    st->num = this->nb_context_state++;
                }
                this->push_state(st->out1);
                if (st->c == SPLIT) {
                    this->push_state(st->out2);
                }
                else {
                    if (st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                        this->captures.push_back(st);
                    }
                    if (0 == st->out1) {
                        this->st_last = st;
                    }
                }
            }
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            Trace * from = this->traces + cp_idx * this->nb_context_state;
            Trace * to = this->traces + *this->pidx_trace_free * this->nb_context_state;
            //memcpy
            for (Trace * last = to + this->nb_context_state; to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->nb_context_state);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

    private:
        struct StateListByStep
        {
            struct Info {
                StateBase * st;
                unsigned idx;
            };

            StateListByStep()
            : array(0)
            {}

            ~StateListByStep()
            {
                delete [] this->array;
            }

            void push_back(StateBase* val, unsigned idx)
            {
                this->parray->st = val;
                this->parray->idx = idx;
                ++this->parray;
            }

            Info& operator[](int n) const
            { return this->array[n]; }

            Info * begin() const
            { return this->array; }

            Info * end() const
            { return this->parray; }

            void reserve(unsigned size)
            {
                this->array = new Info[size];
                this->parray = this->array;
            }

            bool empty() const
            { return this->array == this->parray; }

            std::size_t size() const
            { return this->parray - this->array; }

            void clear()
            { this->parray = this->array; }

            Info * array;
            Info * parray;
        };


        struct Matching
        {
            StateMachine &sm;
            unsigned step_id;
            StateListByStep *pl1;
            StateListByStep *pl2;

            Matching(StateMachine& sm)
            : sm(sm)
            , step_id(1)
            , pl1(&sm.l1)
            , pl2(&sm.l1)
            {
                sm.l1.clear();
                sm.l2.clear();

                for (state_iterator first = this->sm.vec.begin(), last = this->sm.vec.end(); first != last; ++first) {
                    (*first)->id = 0;
                }
            }


            void addstate(StateBase *st, unsigned idx, bool b = false)
            {
                if (st && st->id != this->step_id) {
                    if (st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                        st = st->out1;
                    }

                    st->id = this->step_id;
                    if (st->c == SPLIT) {
                        this->addstate(st->out1, idx, b);
                        this->addstate(st->out2, idx, true);
                    }
                    else {
                        /**///std::cout << ("  add");
                        st->id = this->step_id;
                        //BEGIN if capture
                        if (b) {
                            idx = this->sm.pop_idx_trace(idx);
                        }
                        //END
                        this->pl2->push_back(st, idx);
                    }
                }
            }

            typedef StateListByStep::Info Info;

            void step(const char *s)
            {
                for (Info * first = this->pl1->begin(), * last = this->pl1->end(); first != last; ++first) {
                    if (first->st->check(*s)) {
                        ++this->sm.traces[first->idx * this->sm.nb_context_state + first->st->num].size;
                        /**///std::cout << ("  -- ok");
                        this->addstate(first->st->out1, first->idx);
                    }
                }

                //BEGIN if capture
                for (Info * first = this->pl1->begin(), * last = this->pl1->end(); first != last; ++first) {
                    bool b = false;
                    for (Info * first2 = this->pl2->begin(), * last2 = this->pl2->end(); first2 != last2; ++first2) {
                        if (first->idx == first2->idx) {
                            b = true;
                            break;
                        }
                    }
                    if (!b) {
                        this->sm.push_idx_trace(first->idx);
                    }
                }
                //END
                /**///std::cout << (pstep_free - step_free) << " " << (nlist.size()) << std::endl;
            }

            bool exact_match(const char * s)
            {
                /**///std::cout << "nb_context_state: " << (this->sm.nb_context_state) << "\n";
                //BEGIN if capture
                this->sm.pidx_trace_free = this->sm.idx_trace_free;
                for (unsigned i = 0; i < this->sm.nb_context_state; ++i, ++this->sm.pidx_trace_free) {
                    *this->sm.pidx_trace_free = i;
                }
                std::memset(this->sm.traces, 0, this->sm.nb_context_state * this->sm.nb_context_state * sizeof this->sm.traces[0]);
                //END

                /**///std::cout << (pstep_free - step_free) << std::endl;
                this->addstate(this->sm.st_first, *--this->sm.pidx_trace_free);
                this->pl2 = &this->sm.l2;
                /**///std::cout << (pstep_free - step_free) << " " << (l1.size()) << std::endl;

                //std::cout << ("start\n") << std::endl;

                /**///std::cout << (this->sm.st_last->id) << std::endl;
                for(; *s && 0 == this->sm.st_last->id; ++s){
                    /**///std::cout << "c: '" << *s << "'\n";
                    /**///std::cout << "\nc: " << *s << "\n";
                    ++this->step_id;
                    this->step(s);
                    if (this->pl2->empty()) {
                        break;
                    }
                    this->pl1->clear();
                    std::swap<>(this->pl1, this->pl2);
                }

                if (this->sm.st_last->id != 0) {
                    for (Info * first = this->pl1->begin(), * last = this->pl1->end(); first != last; ++first) {
                        if (first->st == this->sm.st_last) {
                            this->sm.idx_trace = first->idx;
                            break;
                        }
                    }
                }

                ////BEGIN check: (pstep_free - step_free) == this->sm.nb_context_state
                //for (std::size_t i = 0; i < pl1->size(); ++i) {
                //    free_idx((*pl1)[i].idx);
                //}
                //std::cout << (pstep_free - step_free) << std::endl;
                ////END check

                return this->sm.st_last->id != 0;
            }
        };

        friend class Matching;

        typedef std::vector<StateBase*> state_list;
        typedef state_list::iterator state_iterator;

        unsigned nb_context_state;
        StateBase * st_first;
        StateBase * st_last;
        state_list vec;
    public:
        state_list captures;
    private:
        Trace * traces;
        unsigned idx_trace;
        unsigned * idx_trace_free;
        unsigned * pidx_trace_free;

        StateListByStep l1;
        StateListByStep l2;
    };

    //NOTE rename to Regex ?
    class StateMachine2
    {
        class RangeList;

        struct StateList
        {
            StateBase * st;
            RangeList * next;
        };

        StateList * st_list;

        struct RangeList
        {
            State * st;
            StateList * first;
            StateList * last;
        };

        RangeList * st_range_list;

    public:
        explicit StateMachine2(StateBase * st)
        : nb_context_state(0)
        , st_first(st)
        , st_last(0)
        , vec()
        , captures()
        , traces(0)
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        {
            unsigned nb_split = 0;
            this->push_state(st, nb_split);

            ///TODO this->nb_context_state + 1 ?
            this->st_list = new StateList[this->nb_context_state*this->nb_context_state];
            this->st_range_list = new RangeList[this->nb_context_state];

            for (unsigned n = 0; n < this->nb_context_state; ++n) {
                RangeList& l = this->st_range_list[n];
                l.st = 0;
                l.first = this->st_list + n * this->nb_context_state;
                l.last = l.first;
            }

            unsigned step = 0;
            this->init_list(this->st_range_list, st, step);

//             if (!this->vec.empty())
//             {
//                 if (!this->captures.empty()) {
//                     this->traces = new Trace[this->nb_context_state * this->nb_context_state * sizeof this->traces[0]];
//                     this->idx_trace_free = new unsigned[this->nb_context_state];
//                 }
//
//                 l1.reserve(this->nb_context_state);
//                 l2.reserve(this->nb_context_state);
//             }
        }

        ~StateMachine2()
        {
            delete [] this->traces;
            delete [] this->idx_trace_free;
        }

        void reset_id()
        {
            for (state_iterator first = this->vec.begin(), last = this->vec.end(); first != last; ++first) {
                (*first)->id = 0;
            }
        }

        void push_state(RangeList* l, StateBase * st, unsigned& step)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            if (st && st->id != step) {
                st->id = step;
                if (st->c == SPLIT) {
                    this->push_state(l, st->out1, step);
                    this->push_state(l, st->out2, step);
                }
                else {
                    l->last->st = st;
                    ++l->last;
                }
            }
        }

        RangeList* find_range_list(StateBase * st)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            for (RangeList * l = this->st_range_list; l < this->st_range_list + this->nb_context_state && l->st; ++l) {
                if (l->st == st) {
                    return l;
                }
            }
            return 0;
        }

        void init_list(RangeList* l, StateBase * st, unsigned& step)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            l->st = st;
            this->push_state(l, st, step);
            /**///std::cout << "-- " << (l) << std::endl;
            for (StateList * first = l->first, * last = l->last; first < last; ++first) {
                /**///std::cout << first->st->num << ("\t");
                if (first->st->c & ANY_CHARACTER) {
                    /**///std::cout << "any";
                }
                else {
                    /**///std::cout << "'" << char(first->st->c & 0xff) << "'";
                }
                /**///std::cout << ("\t") << first->next << std::endl;
                if (0 == first->st->out1) {
                    continue ;
                }
                RangeList * luse = this->find_range_list(first->st->out1);
                /**///std::cout << "[" << luse << "]" << std::endl;
                if (luse) {
                    first->next = luse;
                }
                else {
                    RangeList * le = l+1;
                    while (le < this->st_range_list + this->nb_context_state && le->st) {
                        ++le;
                    }
                    first->next = le;
                    init_list(le, first->st->out1, ++step);
                }
            }
            /**///std::cout << std::endl;

            struct sorting {
                static bool cmp(const StateList&, const StateList& b) {
                    return b.st->c & ANY_CHARACTER;
                }
            };
            std::sort<>(l->first, l->last, &sorting::cmp);
        }

    private:
        struct Trace
        {
            unsigned size;
            Trace()
            :size(0)
            {}
        };

    public:
        typedef std::pair<const Trace *, const Trace *> TraceRange;

        TraceRange get_trace() const
        {
            const Trace * strace = this->traces + this->idx_trace * this->nb_context_state;
            return TraceRange(strace, strace + this->nb_context_state);
        }

        typedef std::pair<unsigned, unsigned> range_t;
        typedef std::vector<range_t> range_list;

        RangeList exact_match(const char * s)
        {
            RangeList ranges;

            if (this->st_first) {
                if (Matching(*this).exact_match(s)) {
                    this->append_match_result(ranges);
                }
            }

            return ranges;
        }

        bool exact_search(const char * s)
        {
            if (!this->st_first) {
                return false;
            }
            return Matching(*this).exact_match(s); ///TODO exact_search
        }

        bool exact_search_with_trace(const char * s)
        {
            if (!this->st_first) {
                return false;
            }
            return Matching(*this).exact_match(s);
        }

        RangeList match_result()
        {
            RangeList ret;
            this->append_match_result(ret);
            return ret;
        }

        void append_match_result(RangeList& /*ranges*/)
        {
//             ranges.reserve(this->captures.size());
//
//             typedef std::vector<StateBase*>::reverse_iterator iterator;
//             iterator match_first = this->captures.rbegin();
//             iterator match_last = this->captures.rend();
//             unsigned pmatch = -1u;
//             unsigned n = 0;
//             unsigned p = 0;
//             for (TraceRange trace = this->get_trace(); trace.first < trace.second; ++trace.first, ++n) {
//                 if (match_first != match_last && n == (*match_first)->num) {
//                     if (pmatch == -1u) {
//                         pmatch = p;
//                     }
//                     else {
//                         ranges.push_back(range_t(pmatch, p-pmatch));
//                         pmatch = -1u;
//                     }
//                     ++match_first;
//                 }
//                 p += trace.first->size;
//             }
        }

    private:
        void push_state(StateBase * st, unsigned& nb_split){
            if (st && st->id == 0) {
                st->id = -1u;
                this->vec.push_back(st);
                if (st->c != SPLIT) {
                    st->num = this->nb_context_state++;
                }
                this->push_state(st->out1, nb_split);
                if (st->c == SPLIT) {
                    ++nb_split;
                    this->push_state(st->out2, nb_split);
                }
                else {
                    if (st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                        this->captures.push_back(st);
                    }
                    if (0 == st->out1) {
                        this->st_last = st;
                    }
                }
            }
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            Trace * from = this->traces + cp_idx * this->nb_context_state;
            Trace * to = this->traces + *this->pidx_trace_free * this->nb_context_state;
            //memcpy
            for (Trace * last = to + this->nb_context_state; to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->nb_context_state);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

    private:
        struct StateListByStep
        {
            struct Info {
                RangeList * rl;
                unsigned idx;
            };

            StateListByStep()
            : array(0)
            {}

            ~StateListByStep()
            {
                delete [] this->array;
            }

            void push_back(RangeList* val, unsigned idx)
            {
                this->parray->rl = val;
                this->parray->idx = idx;
                ++this->parray;
            }

            Info& operator[](int n) const
            { return this->array[n]; }

            Info * begin() const
            { return this->array; }

            Info * end() const
            { return this->parray; }

            void reserve(unsigned size)
            {
                this->array = new Info[size];
                this->parray = this->array;
            }

            bool empty() const
            { return this->array == this->parray; }

            std::size_t size() const
            { return this->parray - this->array; }

            void clear()
            { this->parray = this->array; }

            Info * array;
            Info * parray;
        };


        struct Matching
        {
            StateMachine2 &sm;
            unsigned step_id;
            StateListByStep *pl1;
            StateListByStep *pl2;

            Matching(StateMachine2& sm)
            : sm(sm)
            , step_id(1)
            , pl1(&sm.l1)
            , pl2(&sm.l1)
            {
                sm.l1.clear();
                sm.l2.clear();
                sm.reset_id();
            }

            typedef StateListByStep::Info Info;

            RangeList * step(const char *s, RangeList * l)
            {
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    if (first->st->check(*s)) {
                        /**///std::cout << "\t" << first->st->c << "\t";
                        if (first->st->c & ANY_CHARACTER) {
                            /**///std::cout << "any";
                        }
                        else {
                            /**///std::cout << "'" << char(first->st->c & 0xff) << "'";
                        }
                        while (first->next && first->next->st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                            /**///std::cout << ("c");
                            first = first->next->first;
                        }
                        /**///std::cout << "\t" << first->next << std::endl;
                        return first->next;
                    }
                }
                return 0;
            }

            bool exact_match(const char * s)
            {

                {
                    RangeList * l = this->sm.st_range_list;
                    for (; l < this->sm.st_range_list + this->sm.nb_context_state; ++l) {
                        if (l->first == l->last) {
                            break;
                        }
                        /**///std::cout << "-- " << (l) << std::endl;
                        StateList * first = l->first;
                        for (; first < l->last; ++first) {
                            /**///std::cout << first->st->num << ("\t");
                            if (first->st->c & ANY_CHARACTER) {
                                /**///std::cout << "any";
                            }
                            else {
                                /**///std::cout << "'" << char(first->st->c & 0xff) << "'";
                            }
                            /**///std::cout << ("\t") << first->next << std::endl;
                        }
                        /**///std::cout << std::endl;
                    }
                }

                RangeList * l = this->sm.st_range_list;
                /**///std::cout << "l: " << (l) << std::endl;

                for(; *s && l; ++s){
                    /**///std::cout << "c: '" << *s << "'\n";
                    l = this->step(s, l);
                }


                return 0 == l || (l->first->st->c == ANY_CHARACTER && 0 == l->first->next);
            }
        };

        friend class Matching;

        typedef std::vector<StateBase*> state_list;
        typedef state_list::iterator state_iterator;

        unsigned nb_context_state;
        StateBase * st_first;
        StateBase * st_last;
        state_list vec;
    public:
        state_list captures;
    private:
        Trace * traces;
        unsigned idx_trace;
        unsigned * idx_trace_free;
        unsigned * pidx_trace_free;

        StateListByStep l1;
        StateListByStep l2;
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
                    single(
                        CAPTURE_OPEN,
                        zero_or_more(
                            ANY_CHARACTER,
                            single(
                                ' ',
                                single(
                                    CAPTURE_CLOSE,
                                    single(
                                        CAPTURE_OPEN,
                                        zero_or_more(
                                            ANY_CHARACTER,
                                            single(
                                                ' ',
                                                single(
                                                    CAPTURE_CLOSE,
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

    StateMachine2 sm(st);

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
        for (size_t i = 0; i < 100000; ++i) {
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
        for (size_t i = 0; i < 100000; ++i) {
            ismatch2 = 0 == regexec(&rgx, str, 1, &regmatch, 0);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch3 = sm.exact_search_with_trace(str);
        }
        d3 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::cout.rdbuf(dbuf);

    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);
    std::cout
    << "regex: '.* .* (.*) (.*) .*a'\n"
    << (ismatch1 ? "good\n" : "fail\n")
    << d1 << "\n"
    << (ismatch2 ? "good\n" : "fail\n")
    << d2 << "\n"
    << (ismatch3 ? "good\n" : "fail\n")
    << d3 << "\n"
    << std::endl;

//     if (ismatch3) {
//         typedef StateMachine::range_list range_list;
//         range_list ranges = sm.match_result();
//
//         std::cout << ("matches:\n");
//         unsigned n = 1;
//         for (range_list::iterator first = ranges.begin(), last = ranges.end(); first < last; ++first, ++n) {
//             (std::cout << n << ": \"").write(str+first->first, first->second) << "\"\n";
//         }
//     }
}
