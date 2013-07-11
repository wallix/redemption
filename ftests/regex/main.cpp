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
#include <boost/preprocessor/cat.hpp>

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
            return Matching(*this).exact_match(s);
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
                std::memset(this->sm.traces, 0,
                            this->sm.nb_context_state * this->sm.nb_context_state * sizeof this->sm.traces[0]);
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
        : nb_capture(0)
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        , st_first(st)
        , captures(0)
        , pcaptures(0)
        , traces(0)
        , vec()
        , l1()
        , l2()
        {
            this->push_state(st);

            if (!this->vec.empty())
            {
                const unsigned matrix_size = this->vec.size() * this->vec.size();
                this->st_list = new StateList[matrix_size];
                std::memset(this->st_list, 0, matrix_size * sizeof * this->st_list);

                this->st_range_list = new RangeList[this->vec.size()];
                for (unsigned n = 0; n < this->vec.size(); ++n) {
                    RangeList& l = this->st_range_list[n];
                    l.st = 0;
                    l.first = this->st_list + n * this->vec.size();
                    l.last = l.first;
                }

                if (this->nb_capture) {
                    this->captures = new StateBase const *[this->nb_capture*2];
                    this->pcaptures = this->captures;
                    this->traces = new unsigned[matrix_size + this->vec.size()];
                    this->idx_trace_free = this->traces + matrix_size;
                }

                {
                    unsigned step = 0;
                    this->init_list(this->st_range_list, st, step);
                }

                l1.reserve(this->vec.size());
                l2.reserve(this->vec.size());
            }
        }

        ~StateMachine2()
        {
            delete [] this->st_list;
            delete [] this->st_range_list;
            delete [] this->traces;
            delete [] this->captures;
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            unsigned * from = this->traces + cp_idx * this->vec.size();
            unsigned * to = this->traces + *this->pidx_trace_free * this->vec.size();
            for (unsigned * last = to + this->vec.size(); to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->vec.size());
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
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
                    if (st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                        *this->pcaptures = st;
                        ++this->pcaptures;
                    }
                    l->last->st = st;
                    ++l->last;
                }
            }
        }

        RangeList* find_range_list(StateBase * st)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            for (RangeList * l = this->st_range_list; l < this->st_range_list + this->vec.size(); ++l) {
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
#if 0
                /**///std::cout << first->st->num << ("\t");
                if (first->st->c & ANY_CHARACTER) {
                    /**///std::cout << "any";
                }
                else {
                    /**///std::cout << "'" << char(first->st->c & 0xff) << "'";
                }
                /**///std::cout << ("\t") << first->next << std::endl;
#endif
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
                    while (le < this->st_range_list + this->vec.size() && le->st) {
                        ++le;
                    }
                    first->next = le;
                    init_list(le, first->st->out1, ++step);
                }
            }

            struct sorting {
                static bool cmp(const StateList&, const StateList& b) {
                    return b.st->c & ANY_CHARACTER;
                }
            };
            std::sort<>(l->first, l->last, &sorting::cmp);
        }

    public:
        typedef std::pair<const unsigned *, const unsigned *> TraceRange;

        TraceRange get_trace() const
        {
            const unsigned * strace = this->traces + this->idx_trace * this->vec.size();
            return TraceRange(strace, strace + this->vec.size());
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
            return Searching(*this).exact_match(s);
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

        void append_match_result(range_list& ranges) const
        {
            ranges.reserve(this->pcaptures - this->captures);

            StateBase const * * match_first = this->captures;
            unsigned pmatch = -1u;
            unsigned n = 0;
            unsigned p = 0;
            TraceRange trace = this->get_trace();
            while (trace.first < trace.second && match_first != this->pcaptures) {
                if (n == (*match_first)->num) {
                    if (pmatch == -1u) {
                        pmatch = p;
                    }
                    else {
                        ranges.push_back(range_t(pmatch, p-pmatch));
                        pmatch = -1u;
                    }
                    ++match_first;
                }
                p += *trace.first;
                ++trace.first;
                ++n;
            }
        }

    private:
        void push_state(StateBase * st)
        {
            if (st && st->id == 0) {
                st->id = -1u;
                st->num = this->vec.size();
                this->vec.push_back(st);
                this->push_state(st->out1);
                if (st->c == SPLIT) {
                    this->push_state(st->out2);
                }
                else if (st->c == CAPTURE_OPEN) {
                    ++this->nb_capture;
                }
            }
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

        struct Searching
        {
            StateMachine2 &sm;

            Searching(StateMachine2& sm)
            : sm(sm)
            {}

            typedef StateListByStep::Info Info;

            RangeList * step(const char *s, RangeList * l)
            {
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    if (first->st->check(*s)) {
#if 0
                            this->sm.display_elem_state_list(first);
#endif
                        while (first->next && first->next->st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                            first = first->next->first;
                        }
                        return first->next;
                    }
                }
                return 0;
            }

            bool exact_match(const char * s)
            {
#if 0
                this->sm.display_dfa();
#endif

                RangeList * l = this->sm.st_range_list;

                for(; *s && l; ++s){
                    /**///std::cout << "\033[01;31mc: '" << *s << "'\033[0m\n";
                    l = this->step(s, l);
                }

                return 0 == l/* || (l->first->st->c == ANY_CHARACTER && 0 == l->first->next)*/;
            }
        };

    private:
        void reset_trace()
        {
            for (state_iterator first = this->vec.begin(), last = this->vec.end(); first != last; ++first) {
                (*first)->id = 0;
            }
            this->pidx_trace_free = this->idx_trace_free;
            for (unsigned i = 0; i < this->vec.size(); ++i, ++this->pidx_trace_free) {
                *this->pidx_trace_free = i;
            }
            std::memset(this->traces, 0,
                        this->vec.size() * this->vec.size() * sizeof this->traces[0]);
        }

    public:
        void display_elem_state_list(const StateList * first) const
        {
            std::cout << "\t\033[33m" << first->st->c << "\t";
            if (first->st->c & ANY_CHARACTER) {
                std::cout << "any\t";
            }
            else {
                std::cout << "'" << char(first->st->c & 0xff) << "'\t";
            }
            std::cout << (first->next) << "\033[0m\n";
        }

        void display_dfa() const
        {
            RangeList * l = this->st_range_list;
            for (; l < this->st_range_list + this->vec.size() && l->first != l->last; ++l) {
                std::cout << l << "\n";
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    std::cout << "\t" << first->st->c << "\t";
                    if (first->st->c & ANY_CHARACTER) {
                        std::cout << "any";
                    }
                    else if (first->st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                        std::cout << (first->st->c == CAPTURE_OPEN ? "(" : ")");
                    }
                    else {
                        std::cout << "'" << char(first->st->c & 0xff) << "'";
                    }
                    std::cout << "\t" << first->next << ("\n");
                }
            }
            std::cout << std::endl;
        }

    private:
        struct Matching
        {
            StateMachine2 &sm;
            unsigned step_id;

            Matching(StateMachine2& sm)
            : sm(sm)
            , step_id(1)
            {}

            typedef StateListByStep::Info Info;

            unsigned step(const char *s, StateListByStep * l1, StateListByStep * l2)
            {
                unsigned new_trace = 0;
                for (Info * ifirst = l1->begin(), * ilast = l1->end(); ifirst != ilast ; ++ifirst) {
                    if (ifirst->rl->st->id == this->step_id) {
                        continue;
                    }
                    ifirst->rl->st->id = this->step_id;
                    StateList * first = ifirst->rl->first;
                    StateList * last = ifirst->rl->last;
                    for (; first != last; ++first) {
                        if (first->st->id == this->step_id) {
                            continue;
                        }
                        first->st->id = this->step_id;
                        if (first->st->check(*s)) {
#if 0
                            this->sm.display_elem_state_list(first);
#endif

                            if (0 == first->next) {
                                //std::cout << "idx: " << (ifirst->idx) << std::endl;
                                return ifirst->idx;
                            }

                            StateList * p = first;

                            while (p->next && p->next->st->c & (CAPTURE_OPEN|CAPTURE_CLOSE)) {
                                p = p->next->first;
                            }

                            for (Info * x = l2->begin(), * e = l2->end(); x < e; ++x) {
                                if (x->rl == p->next){
                                    continue;
                                }
                            }

                            /**///std::cout << "\t" << first->next << std::endl;
                            const unsigned idx = (new_trace == 0)
                                ? ifirst->idx
                                : this->sm.pop_idx_trace(ifirst->idx);
                            l2->push_back(p->next, idx);
                            ++this->sm.traces[idx * this->sm.vec.size() + first->st->num];
                            ++new_trace;
                            p->next->st->id = this->step_id;
                        }
                    }
                }

                //BEGIN free id trace
                for (Info * first = l1->begin(), * last = l1->end(); first != last; ++first) {
                    struct find {
                        static bool impl(Info * first2, Info * last2, unsigned idx){
                            for (; first2 != last2; ++first2) {
                                if (idx == first2->idx) {
                                    return true;
                                }
                            }
                            return false;
                        }
                    };
                    if (!find::impl(l2->begin(), l2->end(), first->idx)) {
                        this->sm.push_idx_trace(first->idx);
                    }
                }
                //END

                return -1u;
            }

            bool exact_match(const char * s)
            {
#if 0
                this->sm.display_dfa();
#endif

                this->sm.l1.clear();
                this->sm.l2.clear();
                this->sm.reset_trace();

                StateListByStep * pal1 = &this->sm.l1;
                StateListByStep * pal2 = &this->sm.l2;
                pal1->push_back(this->sm.st_range_list, *--this->sm.pidx_trace_free);

                for(; *s; ++s){
                    /**///std::cout << "\033[01;31mc: '" << *s << "'\033[0m\n";
                    if (-1u != (this->sm.idx_trace = this->step(s, pal1, pal2))) {
                        return true;
                    }
                    ++this->step_id;
                    std::swap<>(pal1, pal2);
                    pal2->clear();
                }

                return false;
            }
        };

        friend class Matching;
        friend class Searching;

        typedef std::vector<StateBase*> state_list;
        typedef state_list::iterator state_iterator;

        unsigned nb_capture;
        unsigned idx_trace;
        unsigned * idx_trace_free;
        unsigned * pidx_trace_free;
        StateBase * st_first;
        StateBase const ** captures;
        StateBase const ** pcaptures;
        unsigned * traces;
        state_list vec;
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

#ifndef STATEMACHINE
# define STATEMACHINE 2
#endif

#if STATEMACHINE == 1
    State last(ANY_CHARACTER);
    State char_a('a', &last);
#else
    State char_a('a');
#endif

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
                                STATEMACHINE == 1 ? CAPTURE_CLOSE : ' ',
                                single(
                                    STATEMACHINE == 1 ? ' ' : CAPTURE_CLOSE,
                                    single(
                                        CAPTURE_OPEN,
                                        zero_or_more(
                                            ANY_CHARACTER,
                                            single(
                                                STATEMACHINE == 1 ? CAPTURE_CLOSE : ' ',
                                                single(
                                                    STATEMACHINE == 1 ? ' ' : CAPTURE_CLOSE,
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

    typedef StateMachine StateMachine1;
    typedef BOOST_PP_CAT(StateMachine, STATEMACHINE) Regex;

    Regex sm(st);
    regex_t rgx;
    if (0 != regcomp(&rgx, ".* .* (.*) (.*) .*a.*", REG_EXTENDED)){
        std::cout << ("comp error") << std::endl;
    }
    regmatch_t regmatch[3];

    bool ismatch1 = false;
    bool ismatch2 = false;
    bool ismatch3 = false;
    bool ismatch4 = false;
    double d1, d2, d3, d4;

    const char * str = argc == 2 ? argv[1] : "abcdef";

    {
        regexec(&rgx, str, 1, regmatch, 0); //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch1 = 0 == regexec(&rgx, str, 1, regmatch, 0);
        }
        d1 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch2 = sm.exact_search(str);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    std::streambuf * dbuf = std::cout.rdbuf(0);
    {
        struct test {
            inline static bool
            impl(regex_t& rgx, const char * s, regmatch_t * m, unsigned size) {
                const char * str = s;
                while (0 == regexec(&rgx, s, size, m, 0)) {
                    for (unsigned i = 1; i < size; i++) {
                        if (m[i].rm_so == -1) {
                            break;
                        }
                        int start = m[i].rm_so + (s - str);
                        int finish = m[i].rm_eo + (s - str);
                        std::cout.write(str+start, finish-start) << "\n";
                    }
                    s += m[0].rm_eo;
                }
                return 0 == *s;
            }
        };
        test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0])); //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch3 = test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0]));
        }
        d3 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < 100000; ++i) {
            ismatch4 = sm.exact_search_with_trace(str);
            Regex::range_list match_result = sm.match_result();
            typedef Regex::range_list::iterator iterator;
            for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
                std::cout.write(str+first->first, first->second) << "\n";
            }
        }
        d4 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    std::cout.rdbuf(dbuf);

    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);
    std::cout
    << "regex: '.* .* (.*) (.*) .*a'\n"
    << "search:\n"
    << (ismatch1 ? "good\n" : "fail\n")
    << d1 << " sec\n"
    << (ismatch2 ? "good\n" : "fail\n")
    << d2 << " sec\n"
    << "match:\n"
    << (ismatch3 ? "good\n" : "fail\n")
    << d3 << " sec\n"
    << (ismatch4 ? "good\n" : "fail\n")
    << d4 << " sec\n"
    << std::endl;

    if (ismatch3) {
        std::cout << ("with regex.h\n");
        for (unsigned i = 1; i < sizeof(regmatch)/sizeof(regmatch[0]); i++) {
            if (regmatch[i].rm_so == -1) {
                break;
            }
            int start = regmatch[i].rm_so;
            int finish = regmatch[i].rm_eo;
            (std::cout << "\tmatch: ").write(str+start, finish-start) << "\n";
        }
    }
    if (ismatch4) {
        std::cout << ("with dfa\n");
        Regex::range_list match_result = sm.match_result();
        typedef Regex::range_list::iterator iterator;
        for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
            (std::cout << "\tmatch: ").write(str+first->first, first->second) << "\n";
        }
    }
    regfree(&rgx);
}
