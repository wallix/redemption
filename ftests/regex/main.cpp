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
#include <stdexcept>
#include <boost/preprocessor/cat.hpp>

///TODO transition multi-characters
///TODO regex compiler ("..." -> C++)

namespace rndfa {
    const unsigned ANY_CHARACTER = 1 << 8;
    const unsigned SPLIT = 1 << 9;
    const unsigned CAPTURE_OPEN = 1 << 10;
    const unsigned CAPTURE_CLOSE = 1 << 11;
    const unsigned SPECIAL_CHECK = 1 << 12;
    const unsigned EPSILONE = 1 << 13;
    const unsigned FIRST = 1 << 14;
    const unsigned LAST = 1 << 15;

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

        virtual bool check(int c)
        {
            /**///std::cout << num << ": " << char(this->c & ANY_CHARACTER ? '.' : this->c&0xFF);
            return (this->c & ANY_CHARACTER) || (this->c&0xFF) == c;
        }

        virtual void display(std::ostream& os)
        {
            if (this->c & ANY_CHARACTER) {
                os << "any";
            }
            else if (this->c & (SPLIT|CAPTURE_CLOSE|CAPTURE_OPEN)){
                os << (this->is_split() ? "(split)" : this->c == CAPTURE_OPEN ? "(open)" : "(close)");
            }
            else {
                os << "'" << char(this->c & 0xff) << "'";
            }
        }

        bool is_border() const
        { return this->c & (FIRST|LAST); }

        bool is_cap() const
        { return this->c & (CAPTURE_OPEN|CAPTURE_CLOSE); }

        bool is_cap_open() const
        { return this->c == CAPTURE_OPEN; }

        bool is_cap_close() const
        { return this->c == CAPTURE_CLOSE; }

        bool is_split() const
        { return this->c == SPLIT; }

        bool is_epsilone() const
        { return this->c == EPSILONE; }

        unsigned c;
        unsigned id;
        unsigned num;

        StateBase *out1;
        StateBase *out2;
    };

    std::ostream& operator<<(std::ostream& os, StateBase& st)
    {
        st.display(os);
        return os;
    }

    typedef StateBase State;

    struct StateRange : StateBase
    {
        StateRange(int r1, int r2, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(r1, out1, out2)
        , rend(r2)
        {}

        virtual ~StateRange()
        {}

        virtual bool check(int c)
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return (this->c&0xFF) <= c && c <= rend;
        }

        virtual void display(std::ostream& os)
        {
            os << "[" << char(this->c) << "-" << char(this->rend) << "]";
        }

        int rend;
    };

    State * range(char c1, char c2, StateBase* out = 0) {
        return new StateRange(c1, c2, out);
    }

    struct StateCharacters : StateBase
    {
        StateCharacters(const std::string& s, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(s[0], out1, out2)
        , str(s)
        {}

        virtual ~StateCharacters()
        {}

        virtual bool check(int c)
        {
            /**///std::cout << str;
            return this->str.find(c) != std::string::npos;
        }

        virtual void display(std::ostream& os)
        {
            os << "[" << this->str << "]";
        }

        std::string str;
    };

    struct StateBorder : StateBase
    {
        StateBorder(bool is_first)
        : StateBase(is_first ? FIRST : LAST)
        {}

        virtual ~StateBorder()
        {}

        virtual bool check(int /*c*/)
        {
            /**///std::cout << (this->c == FIRST ? "^" : "$");
            return false;
        }

        virtual void display(std::ostream& os)
        {
            os << (this->c == FIRST ? "^" : "$");
        }
    };

    struct StateMultiTest : StateBase
    {
        struct Checker {
            virtual bool check(int c) = 0;
            virtual void display(std::ostream& os) = 0;
        };

        std::vector<Checker*> checkers;
        typedef std::vector<Checker*>::iterator checker_iterator;
        bool result_true_check;


        StateMultiTest(StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(SPECIAL_CHECK, out1, out2)
        , checkers()
        , result_true_check(true)
        {}

        virtual ~StateMultiTest()
        {
            for (checker_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                delete *first;
            }
        }

        virtual bool check(int c)
        {
            for (checker_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                if ((*first)->check(c)) {
                    return this->result_true_check;
                }
            }
            return !this->result_true_check;
        }

        virtual void display(std::ostream& os)
        {
            checker_iterator first = this->checkers.begin();
            checker_iterator last = this->checkers.end();
            if (first != last) {
                (*first)->display(os);
                while (++first != last) {
                    os << "|";
                    (*first)->display(os);
                }
            }
        }

        void push_checker(Checker * checker)
        {
            this->checkers.push_back(checker);
        }
    };

    struct CheckerString : StateMultiTest::Checker
    {
        CheckerString(const std::string& s)
        : str(s)
        {}

        virtual ~CheckerString()
        {}

        virtual bool check(int c)
        {
            return this->str.find(c) != std::string::npos;
        }

        virtual void display(std::ostream& os)
        {
            os << "[" << this->str << "]";
        }

        std::string str;
    };

    struct CheckerInterval : StateMultiTest::Checker
    {
        CheckerInterval(int first, int last)
        : begin(first)
        , end(last)
        {}

        virtual ~CheckerInterval()
        {}

        virtual bool check(int c)
        {
            return this->begin <= c && c <= this->end;
        }

        virtual void display(std::ostream& os)
        {
            os << "[" << char(this->begin) << "-" << char(this->end) << "]";
        }

        int begin;
        int end;
    };

    State * characters(const char * s, StateBase* out = 0) {
        return new StateCharacters(s, out);
    }

    class StateMachine2
    {
        class RangeList;
        class StateList;

        struct is_begin_state {
            bool operator()(const StateList& stl) const
            {
                return stl.st->c == FIRST;
            }
        };

        struct is_non_free_list {
            bool operator()(const StateList& stl) const
            {
                std::cout << (stl.next->first == stl.next->last) << std::endl;
                return stl.next->first != stl.next->last;
            }
        };

    public:
        explicit StateMachine2(StateBase * st)
        : nb_capture(0)
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        , captures(0)
        , pcaptures(0)
        , traces(0)
        , vec()
        , l1()
        , l2()
        {
            this->push_state(st);
            if (this->vec.empty()) {
                return ;
            }
            this->nb_capture /= 2;
            l1.set_parray(new StateListByStep::Info[this->vec.size() * 2]);
            l2.set_parray(l1.array + this->vec.size());

            if (!this->vec.empty())
            {
                {
                    const unsigned matrix_size = this->vec.size() * this->vec.size();
                    this->st_list = new StateList[matrix_size];
                    std::memset(this->st_list, 0, matrix_size * sizeof * this->st_list);
                }

                {
                    const unsigned size = this->vec.size();
                    this->st_range_list = new RangeList[size];
                    this->st_range_list_last = this->st_range_list;
                    for (unsigned n = 0; n < size; ++n) {
                        RangeList& l = *this->st_range_list_last;
                        ++this->st_range_list_last;
                        l.st = 0;
                        l.first = this->st_list + n * this->vec.size();
                        l.last = l.first;
                    }
                }

                if (this->nb_capture) {
                    const unsigned col = this->vec.size() - this->nb_capture * 2;
                    const unsigned matrix_size = col * this->nb_capture * 2;

                    this->captures = new StateBase const *[this->nb_capture * 2 + matrix_size];
                    this->traces = reinterpret_cast<const char **>(this->captures + this->nb_capture * 2);

                    this->idx_trace_free = new unsigned[col];

                    this->pcaptures = this->captures;

                    for (state_iterator first = this->vec.begin(), last = this->vec.end(); first != last; ++first) {
                        if ((*first)->is_cap()) {
                            *this->pcaptures = *first;
                            ++this->pcaptures;
                        }
                    }
                }

                {
                    unsigned step = 0;
                    this->init_list(this->st_range_list, st, step);
                }

                while (this->st_range_list != this->st_range_list_last && 0 == (this->st_range_list_last-1)->st) {
                    --this->st_range_list_last;
                }

                this->st_range_beginning.st = 0;
                this->st_range_beginning.first = std::partition(
                    this->st_range_list->first,
                    this->st_range_list->last,
                    is_begin_state()
                );
                this->st_range_beginning.last = this->st_range_list->last;
                if (st_range_beginning.first != this->st_range_list->first) {
                    StateList * l = st_range_list->first;
                    StateList * rlast = st_range_list->last;
                    this->st_range_beginning.st = l->st;
                    for (; l != rlast; ++l) {
                        StateList * first = l->next->first;
                        StateList * last = l->next->last;
                        for (; first != last; ++first) {
                            st_range_beginning.last->next = first->next;
                            st_range_beginning.last->st = first->st;
                            ++st_range_beginning.last;
                        }
                    }
                    this->st_range_beginning.last = std::partition(
                        this->st_range_beginning.first,
                        this->st_range_beginning.last,
                        is_non_free_list()
                    );
                    this->st_range_list->first = this->st_range_beginning.first;
                }
                else {
                    st_range_beginning.st = this->st_range_list->st;
                    st_range_beginning.first = this->st_range_list->first;
                    st_range_beginning.last = this->st_range_list->last;
                }
            }
        }

        ~StateMachine2()
        {
            delete [] this->st_list;
            delete [] this->st_range_list;
            delete [] this->captures;
            delete [] this->idx_trace_free;
            delete [] this->l1.array;
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            const unsigned size = (this->nb_capture * 2);
            char const ** from = this->traces + cp_idx * size;
            char const ** to = this->traces + *this->pidx_trace_free * size;
            for (char const ** last = to + size; to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->vec.size() - this->nb_capture * 2);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

        void push_state(RangeList* l, StateBase * st, unsigned& step)
        {
            if (st && st->id != step) {
                st->id = step;
                if (st->is_split()) {
                    this->push_state(l, st->out1, step);
                    this->push_state(l, st->out2, step);
                }
                else {
                    l->last->st = st;
                    ++l->last;
                    if (st->is_cap()) {
                        this->push_state(l, st->out1, step);
                    }
                }
            }
        }

        RangeList* find_range_list(StateBase * st)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            for (RangeList * l = this->st_range_list; l < this->st_range_list_last && l->st; ++l) {
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
                /**///std::cout << first->st->num << ("\t") << first->st << ("\t") << first->next << std::endl;
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
                    while (le < this->st_range_list_last && le->st) {
                        ++le;
                    }
                    first->next = le;
                    init_list(le, first->st->out1, ++step);
                }
            }
        }

    public:
        typedef std::pair<const char **, const char **> TraceRange;

        TraceRange get_trace() const
        {
            char const ** strace = this->traces + this->idx_trace * (this->nb_capture * 2);
            return TraceRange(strace, strace + (this->nb_capture * 2));
        }

        typedef std::pair<const char *, const char *> range_t;
        typedef std::vector<range_t> range_matches;

    private:
        struct DefaultMatchTracer
        {
            void start(unsigned /*idx*/) const
            {}

            void new_id(unsigned /*old_id*/, unsigned /*new_id*/) const
            {}

            bool open(unsigned /*idx*/, const char *, unsigned /*num_cap*/) const
            { return true; }

            bool close(unsigned /*idx*/, const char *, unsigned /*num_cap*/) const
            { return true; }

            void fail(unsigned /*idx*/) const
            {}

            void good(unsigned /*idx*/) const
            {}
        };

    public:
        bool exact_search(const char * s)
        {
            if (this->vec.empty()) {
                return false;
            }
            return Searching(*this).exact_search(s);
        }

        bool exact_search_with_trace(const char * s)
        {
            if (this->nb_capture == 0) {
                return exact_search(s);
            }
            return Matching(*this).match(s, DefaultMatchTracer(), Matching::is_exact());
        }

        template<typename Tracer>
        bool exact_search_with_trace(const char * s, Tracer tracer)
        {
            if (this->nb_capture == 0) {
                return exact_search(s);
            }
            return Matching(*this).match<Tracer&>(s, tracer, Matching::is_exact());
        }

        bool search(const char * s)
        {
            if (this->vec.empty()) {
                return false;
            }
            return Searching(*this).search(s);
        }

        bool search_with_trace(const char * s)
        {
            if (this->nb_capture == 0) {
                return exact_search(s);
            }
            return Matching(*this).match(s, DefaultMatchTracer(), Matching::is_not_exact());
        }

        template<typename Tracer>
        bool search_with_trace(const char * s, Tracer tracer)
        {
            if (this->nb_capture == 0) {
                return exact_search(s);
            }
            return Matching(*this).match<Tracer&>(s, tracer, Matching::is_not_exact());
        }

        range_matches match_result()
        {
            range_matches ret;
            this->append_match_result(ret);
            return ret;
        }

        void append_match_result(range_matches& ranges) const
        {
            ranges.reserve(this->nb_capture);
            TraceRange trace = this->get_trace();

            const StateBase ** pst = this->captures;
            while (pst < this->pcaptures) {
                while ((*pst)->is_cap_close()) {
                    if (++pst >= this->pcaptures) {
                        return ;
                    }
                }
                const StateBase ** pbst = pst;
                unsigned n = 1;
                while (++pst < this->pcaptures && ((*pst)->is_cap_open() ? ++n : --n)) {
                }
                if (trace.first[pst - this->captures]) {
                    ranges.push_back(range_t(
                        trace.first[pbst - this->captures],
                        trace.first[pst - this->captures]
                    ));
                }
                pst = ++pbst;
            }
        }

    private:
        void push_state(StateBase * st)
        {
            if (st && st->id != -1u) {
                st->id = -1u;
                st->num = this->vec.size();
                if (st->is_cap()) {
                    st->num = this->nb_capture;
                    ++this->nb_capture;
                }
                this->vec.push_back(st);
                this->push_state(st->out1);
                if (st->is_split()) {
                    this->push_state(st->out2);
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

            void push_back(RangeList* val, unsigned idx)
            {
                this->parray->rl = val;
                this->parray->idx = idx;
                ++this->parray;
            }

            void push_back(RangeList* val)
            {
                this->parray->rl = val;
                ++this->parray;
            }

            Info& operator[](int n) const
            { return this->array[n]; }

            Info * begin() const
            { return this->array; }

            Info * end() const
            { return this->parray; }

            void set_parray(Info * p)
            {
                this->array = p;
                this->parray = p;
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
            unsigned step_id;

            Searching(StateMachine2& sm)
            : sm(sm)
            , step_id(1)
            {}

            typedef StateListByStep::Info Info;

            RangeList * step(const char *s, RangeList * l)
            {
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    if (first->st->c == LAST && !(s+1)) {
                        return 0;
                    }
                    if (!first->st->is_cap() && first->st->check(*s)) {
#ifdef DISPLAY_TRACE
                        this->sm.display_elem_state_list(*first, 0);
#endif
                        return first->next;
                    }
                }

                return (RangeList*)1;
            }

            bool exact_search(const char * s)
            {
#ifdef DISPLAY_TRACE
                this->sm.display_dfa();
#endif

                this->sm.reset_id();

                RangeList * l = &this->sm.st_range_beginning;

                if (*s) {
                    if ((l = this->step(s, l)) <= (RangeList*)1) {
                        return false;
                    }
                    ++this->step_id;
                }

                for(; *s && l > (void*)1; ++s){
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << *s << "'\033[0m\n";
#endif
                    l = this->step(s, l);
                    ++this->step_id;
                }

                if (0 == l) {
                    return true;
                }

                if ((RangeList*)1 == l || *s) {
                    return false;
                }

                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    if (first->st->c == LAST) {
                        return true;
                    }
                }

                return false;
            }

            unsigned step(const char *s, StateListByStep * l1, StateListByStep * l2)
            {
                for (Info* ifirst = l1->begin(), * ilast = l1->end(); ifirst != ilast ; ++ifirst) {
                    if (ifirst->rl->st->id == this->step_id) {
                        /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                        continue;
                    }
                    ifirst->rl->st->id = this->step_id;

                    for (StateList * first = ifirst->rl->first, * last = ifirst->rl->last; first != last; ++first) {
                        if (first->st->c == LAST && !(s+1)) {
                            return 0;
                        }
                        if (0 == first->next) {
                            return 0;
                        }
                        l2->push_back(first->next);
                    }
                }
                return -1u;
            }

            bool search(const char * s)
            {
                if (this->sm.st_range_list->first == this->sm.st_range_list->last) {
                    return this->exact_search(s);
                }
#ifdef DISPLAY_TRACE
                this->sm.display_dfa();
#endif

                this->sm.l1.clear();
                this->sm.l2.clear();
                this->sm.reset_id();

                StateListByStep * pal1 = &this->sm.l1;
                StateListByStep * pal2 = &this->sm.l2;
                this->sm.l1.push_back(&this->sm.st_range_beginning);

                for(; *s; ++s){
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << *s << "'\033[0m\n";
#endif
                    if (0 == this->step(s, pal1, pal2)) {
                        return true;
                    }
                    ++this->step_id;
                    std::swap(pal1, pal2);
                    pal2->clear();
                    pal1->push_back(this->sm.st_range_list);
                }

                return false;
            }
        };

    private:
        void reset_id()
        {
            for (RangeList * l = this->st_range_list; l < this->st_range_list_last; ++l) {
                l->st->id = 0;
            }
        }

        void reset_trace()
        {
            this->pidx_trace_free = this->idx_trace_free;
            const unsigned size = this->vec.size() - this->nb_capture * 2;
            for (unsigned i = 0; i < size; ++i, ++this->pidx_trace_free) {
                *this->pidx_trace_free = i;
            }
            std::memset(this->traces, 0,
                        size * this->nb_capture * 2 * sizeof this->traces[0]);
        }

    public:
        void display_elem_state_list(const StateList& e, unsigned idx) const
        {
            std::cout << "\t\033[33m" << idx << "\t" << e.st->num << "\t" << e.st->c << "\t"
            << *e.st << "\t" << (e.next) << "\033[0m" << std::endl;
        }

        void display_dfa() const
        {
            RangeList * l = this->st_range_list;
            for (; l < this->st_range_list_last && l->first != l->last; ++l) {
                std::cout << l << "  st: " << l->st->num << (l->st->is_cap() ? " (cap)\n" : "\n");
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    std::cout << "\t" << first->st->num << "\t" << first->st->c << "\t"
                    << *first->st << "\t" << first->next << ("\n");
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

            template<typename Tracer>
            unsigned step(const char *s, StateListByStep * l1, StateListByStep * l2,
                          Tracer& tracer)
            {
                for (Info* ifirst = l1->begin(), * ilast = l1->end(); ifirst != ilast ; ++ifirst) {
                    if (ifirst->rl->st->id == this->step_id) {
                        /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                        tracer.fail(ifirst->idx);
                        this->sm.push_idx_trace(ifirst->idx);
                        continue;
                    }

                    unsigned new_trace = 0;
                    ifirst->rl->st->id = this->step_id;
                    StateList * first = ifirst->rl->first;
                    StateList * last = ifirst->rl->last;

                    for (; first != last; ++first) {
                        if (first->st->is_cap_open()) {
                            if (!this->sm.traces[ifirst->idx * (this->sm.nb_capture * 2) + first->st->num] && tracer.open(ifirst->idx, s, first->st->num)) {
#ifdef DISPLAY_TRACE
                                std::cout << ifirst->idx << "  " << *first->st << "  " << first->st->num << std::endl;
#endif
                                ++this->sm.traces[ifirst->idx * (this->sm.nb_capture * 2) + first->st->num] = s;

                            }
                            continue ;
                        }

                        if (first->st->is_cap_close()) {
                            if (tracer.close(ifirst->idx, s, first->st->num)) {
#ifdef DISPLAY_TRACE
                                std::cout << ifirst->idx << "  " << *first->st << "  " << first->st->num << std::endl;
#endif
                                ++this->sm.traces[ifirst->idx * (this->sm.nb_capture * 2) + first->st->num] = s;
                            }
                            continue ;
                        }

                        if (first->st->c == LAST && !(s+1)) {
                            return ifirst->idx;
                        }

                        if (first->st->check(*s)) {
#ifdef DISPLAY_TRACE
                            this->sm.display_elem_state_list(*first, ifirst->idx);
#endif

                            if (0 == first->next) {
                                /**///std::cout << "idx: " << (ifirst->idx) << std::endl;
                                return ifirst->idx;
                            }

                            unsigned idx = ifirst->idx;
                            if (new_trace) {
                                idx = this->sm.pop_idx_trace(ifirst->idx);
                                tracer.new_id(ifirst->idx, idx);
                            }
#ifdef DISPLAY_TRACE
                            std::cout << "\t\033[32m" << ifirst->idx << " -> " << idx << "\033[0m" << std::endl;
#endif
                            l2->push_back(first->next, idx);
                            ++new_trace;
                        }
                    }
                    if (0 == new_trace) {
#ifdef DISPLAY_TRACE
                        std::cout << "\t\033[35mx " << ifirst->idx << "\033[0m" << std::endl;
#endif
                        tracer.fail(ifirst->idx);
                        this->sm.push_idx_trace(ifirst->idx);
                    }
                }

                return -1u;
            }

            struct is_exact { static const bool value = true; };
            struct is_not_exact { static const bool value = false; };

            template<typename Tracer, typename ExactMatch>
            bool match(const char * s, Tracer tracer, ExactMatch)
            {
#ifdef DISPLAY_TRACE
                this->sm.display_dfa();
#endif

                this->sm.l1.clear();
                this->sm.l2.clear();
                this->sm.reset_id();
                this->sm.reset_trace();

                StateListByStep * pal1 = &this->sm.l1;
                StateListByStep * pal2 = &this->sm.l2;
                this->sm.l1.push_back(&this->sm.st_range_beginning, *--this->sm.pidx_trace_free);
                tracer.start(*this->sm.pidx_trace_free);

                for(; *s; ++s) {
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << *s << "'\033[0m" << std::endl;
#endif
                    if (-1u != (this->sm.idx_trace = this->step(s, pal1, pal2, tracer))) {
                        tracer.good(this->sm.idx_trace);
                        return false == ExactMatch::value || !*(s+1);
                    }
                    if (pal2->empty()) {
                        break;
                    }
                    ++this->step_id;
                    std::swap(pal1, pal2);
                    pal2->clear();
                    if (false == ExactMatch::value) {
                        --this->sm.pidx_trace_free;
                        assert(this->sm.pidx_trace_free >= this->sm.idx_trace_free);
                        this->sm.l1.push_back(&this->sm.st_range_beginning, *this->sm.pidx_trace_free);
                    }
                }

                if (!*s) {
                    for (Info* ifirst = pal1->begin(), * ilast = pal1->end(); ifirst != ilast ; ++ifirst) {
                        if (ifirst->rl->st->id == this->step_id) {
                            /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                            tracer.fail(ifirst->idx);
                            this->sm.push_idx_trace(ifirst->idx);
                            continue;
                        }

                        ifirst->rl->st->id = this->step_id;
                        StateList * first = ifirst->rl->first;
                        StateList * last = ifirst->rl->last;

                        for (; first != last; ++first) {
                            if (first->st->is_cap_close()) {
                                if (tracer.close(ifirst->idx, s, first->st->num)) {
#ifdef DISPLAY_TRACE
                                    std::cout << ifirst->idx << "  " << *first->st << "  " << first->st->num << std::endl;
#endif
                                    if (0 == first->st->out1) {
                                        this->sm.idx_trace = ifirst->idx;
                                        ++this->sm.traces[ifirst->idx * (this->sm.nb_capture * 2) + first->st->num] = s;
                                        return true;
                                    }
                                }
                                continue ;
                            }

                            if (first->st->c == LAST) {
                                this->sm.idx_trace = ifirst->idx;
                                return true;
                            }
                        }
                    }
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
        const StateBase ** captures;
        const StateBase ** pcaptures;
        const char ** traces;
        state_list vec;
        StateListByStep l1;
        StateListByStep l2;

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
        RangeList * st_range_list_last;
        RangeList st_range_beginning;
    };


    State * state(State * out1, State * out2 = 0) {
        return new StateBase(SPLIT, out1, out2);
    }

    State * single(int c, State * out = 0) {
        return new StateBase(c, out);
    }

    State * any(State * out = 0) {
        return new StateBase(ANY_CHARACTER, out);
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

    int c2rgxc(int c)
    {
        switch (c) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case 'v': return '\v';
            default : return c;
        }
    }

    const char * check_interval(int a, int b)
    {
        bool valid = ('0' <= a && a <= '9' && '0' <= b && b <= '9')
        || ('a' <= a && a <= 'z' && 'a' <= b && b <= 'z')
        || ('A' <= a && a <= 'Z' && 'A' <= b && b <= 'Z');
        return (valid && a <= b) ? 0 : "range out of order in character class";
    }

    StateBase * str2stchar(const char *& s, const char * last, const char * & msg_err)
    {
        if (*s == '\\' && s+1 != last) {
            return new StateBase(c2rgxc(*++s));
        }

        if (*s == '[') {
            StateMultiTest * st = new StateMultiTest;
            std::string str;
            if (++s != last && *s != ']') {
                if (*s == '^') {
                    st->result_true_check = false;
                    ++s;
                }
                if (*s == '-') {
                    str += '-';
                    ++s;
                }
                const char * c = s;
                while (s != last && *s != ']') {
                    const char * p = s;
                    while (++s != last && *s != ']' && *s != '-') {
                        if (*s == '\\' && s+1 != last) {
                            str += c2rgxc(*++s);
                        }
                        else {
                            str += *s;
                        }
                    }

                    if (*s == '-') {
                        if (c == s) {
                            str += '-';
                        }
                        else if (s+1 == last) {
                            msg_err = "missing terminating ]";
                            return 0;
                        }
                        else if (*(s+1) == ']') {
                            str += '-';
                            ++s;
                        }
                        else if (s == p) {
                            str += '-';
                        }
                        else {
                            if ((msg_err = check_interval(*(s-1), *(s+1)))) {
                                return 0;
                            }
                            if (str.size() > 1) {
                                str.erase(str.size()-1);
                            }
                            st->push_checker(new CheckerInterval(*(s-1), *(s+1)));
                            c = ++s + 1;
                        }
                    }
                }
            }

            if (!str.empty()) {
                st->push_checker(new CheckerString(str));
            }

            if (*s != ']') {
                msg_err = "missing terminating ]";
                delete st;
                st = 0;
            }

            return st;
        }

        return new StateBase(*s == '.' ? ANY_CHARACTER : *s);
    }

    bool is_meta_char(int c)
    {
        return c == '*' || c == '+' || c == '?' || c == '|' || c == '(' || c == ')' || c == '^' || c == '$';
    }

    struct StateDeleter
    {
        void operator()(StateBase * st) const
        {
            delete st;
        }
    };

    void append_state(StateBase * st, std::vector<StateBase*>& sts)
    {
        if (st && st->id != -4u) {
            st->id = -4u;
            append_state(st->out1, sts);
            append_state(st->out2, sts);
            sts.push_back(st);
        }
    }

    void free_st(StateBase * st)
    {
        std::vector<StateBase*> sts;
        append_state(st, sts);
        std::for_each(sts.begin(), sts.end(), StateDeleter());
    }

    typedef std::pair<StateBase*, StateBase**> IntermendaryState;

    IntermendaryState intermendary_str2reg(const char *& s, const char * last,
                                           bool & has_epsilone, const char * & msg_err,
                                           int recusive = 0, bool ismatch = true)
    {
        struct FreeState {
            static IntermendaryState invalide(StateBase& st)
            {
                free_st(st.out2);
                return IntermendaryState(0,0);
            }
        };

        StateBase st(0);
        StateBase ** pst = &st.out2;
        StateBase * bst = &st;

        StateBase ** besplit[50] = {0};
        StateBase *** pesplit = besplit;

        while (s != last) {
            if (*s == '^' || *s == '$') {
                if (*pst) {
                    pst = &(*pst)->out1;
                }
                *pst = new StateBorder(*s == '^');
                ++s;
                if (s != last && !is_meta_char(*s)) {
                    pst = &(*pst)->out1;
                }
                continue;
            }

            if (!is_meta_char(*s)) {
                if (!(*pst = str2stchar(s, last, msg_err))) {
                    return FreeState::invalide(st);
                }
                while (++s != last && !is_meta_char(*s)) {
                    pst = &(*pst)->out1;
                    if (!(*pst = str2stchar(s, last, msg_err))) {
                        return FreeState::invalide(st);
                    }
                }
            }
            else {
                if (*s != '(' && *s != ')' && (bst->out2 == 0 || bst->out2->is_border())) {
                    msg_err = "nothing to repeat";
                    return FreeState::invalide(st);
                }
                switch (*s) {
                    case '?':
                        *pst = new StateBase(SPLIT, *pst);
                        pst = &(*pst)->out2;
                        break;
                    case '*':
                        *pst = new StateBase(SPLIT, *pst);
                        (*pst)->out1->out1 = *pst;
                        pst = &(*pst)->out2;
                        break;
                    case '+':
                        (*pst)->out1 = new StateBase(SPLIT, *pst);
                        pst = &(*pst)->out1->out2;
                        break;
                    case '|':
                        *pesplit = pst;
                        ++pesplit;
                        bst->out2 = new StateBase(SPLIT, bst->out2);
                        bst = bst->out2;
                        pst = &bst->out2;
                        break;
                    case ')':
                        if (0 == recusive) {
                            msg_err = "unmatched parentheses";
                            return FreeState::invalide(st);
                        }

                        if (ismatch) {
                            if (*pst) {
                                pst = &(*pst)->out1;
                            }
                            *pst = new StateBase(CAPTURE_CLOSE);
                        }
                        else if (besplit != pesplit) {
                            has_epsilone = true;
                            if (*pst) {
                                pst = &(*pst)->out1;
                            }
                            *pst = new StateBase(ismatch ? CAPTURE_CLOSE : EPSILONE);
                        }

                        for (StateBase *** first = besplit; first != pesplit; ++first) {
                            if (**first) {
                                (**first)->out1 = *pst;
                            }
                            else {
                                (**first) = *pst;
                            }
                        }

                        return IntermendaryState(st.out2, *pst ? &(*pst)->out1 : pst);
                        break;
                    default:
                        //TODO impossible
                        std::cout << ("error") << std::endl;
                        break;
                    case '(':
                        if (*(s+1) == '?' && *(s+2) == ':') {
                            if (s+2 >= last) {
                                msg_err = "unmatched parentheses";
                                return FreeState::invalide(st);
                            }
                            IntermendaryState intermendary = intermendary_str2reg(s+=3, last, has_epsilone, msg_err, recusive+1, false);
                            if (intermendary.first) {
                                if (*pst) {
                                    pst = &(*pst)->out1;
                                }
                                *pst= intermendary.first;
                                pst = intermendary.second;
                            }
                            else if (0 == intermendary.second) {
                                return FreeState::invalide(st);
                            }
                            break;
                        }
                        IntermendaryState intermendary = intermendary_str2reg(++s, last, has_epsilone, msg_err, recusive+1);
                        if (intermendary.first) {
                            if (*pst) {
                                pst = &(*pst)->out1;
                            }
                            *pst = new StateBase(CAPTURE_OPEN, intermendary.first);
                            pst = intermendary.second;
                        }
                        else if (0 == intermendary.second) {
                            return FreeState::invalide(st);
                        }
                        break;
                }
                ++s;
            }
        }
        if (0 != recusive) {
            msg_err = "unmatched parentheses";
            return FreeState::invalide(st);
        }
        return IntermendaryState(st.out2, pst);
    }

    void remove_epsilone(StateBase * st, std::vector<StateBase*>& epsilone_sts)
    {
        if (st && st->id != -3u) {
            st->id = -3u;
            StateBase * nst = st->out1;
            while (nst && nst->is_epsilone()) {
                if (nst->id != -3u) {
                    epsilone_sts.push_back(nst);
                    nst->id = -3u;
                }
                nst = nst->out1;
            }
            st->out1 = nst;
            remove_epsilone(st->out1, epsilone_sts);
            remove_epsilone(st->out2, epsilone_sts);
        }
    }

    StateBase* str2reg(const char * s, const char * last, const char * * msg_err = 0)
    {
        bool has_epsilone = false;
        const char * err = 0;
        StateBase * st = intermendary_str2reg(s, last, has_epsilone, err).first;
        if (err) {
            if (msg_err) {
                *msg_err = err;
            }
            has_epsilone = false;
        }
        else if (has_epsilone) {
            typedef std::vector<StateBase*> states_t;
            states_t removed;
            remove_epsilone(st, removed);
            std::for_each(removed.begin(), removed.end(), StateDeleter());
        }
        return st;
    }

    StateBase* str2reg(const char * s, const char * * msg_err = 0)
    {
        return str2reg(s, s + strlen(s), msg_err);
    }

    void display_state(StateBase * st, unsigned depth = 0)
    {
        if (st && st->id != -1u-1u) {
            std::string s(depth, '\t');
            std::cout
            << s << "\033[33m" << st << "\t" << st->num << "\t" << st->c << "\t"
            << *st << "\033[0m\n\t" << s << st->out1 << "\n\t" << s << st->out2 << "\n";
            st->id = -1u-1u;
            display_state(st->out1, depth+1);
            display_state(st->out2, depth+1);
        }
    }

    class Regex
    {
        struct Parser {
            const char * err;
            unsigned pos_err;
            StateBase * st;

            Parser(StateBase * st = 0)
            : err(0)
            , pos_err(0)
            , st(st)
            {}

            Parser(const char * s, const char * last)
            : err(0)
            , pos_err(0)
            , st(0)
            {
                bool has_epsilone = false;
                const char * tmp = s;
                this->st = intermendary_str2reg(s, last, has_epsilone, this->err).first;
                if (this->err) {
                    this->pos_err = s - tmp;
                }
                else if (has_epsilone) {
                    typedef std::vector<StateBase*> states_t;
                    states_t removed;
                    remove_epsilone(st, removed);
                    std::for_each(removed.begin(), removed.end(), StateDeleter());
                }
            }
        };
        Parser parser;
        StateMachine2 sm;

    public:
        Regex(const char * s)
        : parser(s, s + strlen(s))
        , sm(this->parser.st)
        {}

        Regex()
        : parser()
        , sm(0)
        {}

        Regex(StateBase * st)
        : parser(st)
        , sm(st)
        {}

        void reset(const char * s)
        {
            this->sm.~StateMachine2();
            free_st(this->parser.st);
            new (&this->parser) Parser(s, s + strlen(s));
            new (&this->sm) StateMachine2(this->parser.st);
        }

        ~Regex()
        {
            free_st(this->parser.st);
        }

        const char * message_error() const
        {
            return this->parser.err;
        }

        unsigned position_error() const
        {
            return this->parser.pos_err;
        }

        typedef StateMachine2::range_matches range_matches;

        range_matches exact_match(const char * s)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches exact_match(const char * s, Tracer tracer)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace<Tracer&>(s, tracer)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        range_matches match(const char * s)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches match(const char * s, Tracer tracer)
        {
            range_matches ret;
            if (this->sm.search_with_trace<Tracer&>(s, tracer)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        bool exact_search(const char * s)
        {
            return this->sm.exact_search(s);
        }

        bool search(const char * s)
        {
            return this->sm.search(s);
        }

        bool exact_search_with_matches(const char * s)
        {
            return this->sm.exact_search_with_trace(s);
        }

        template<typename Tracer>
        bool exact_search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.exact_search_with_trace<Tracer&>(s, tracer);
        }

        bool search_with_matches(const char * s)
        {
            return this->sm.search_with_trace(s);
        }

        template<typename Tracer>
        bool search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.search_with_trace<Tracer&>(s, tracer);
        }

        range_matches match_result()
        {
            return this->sm.match_result();
        }

        void display()
        {
            display_state(this->parser.st);
        }
    };
}


struct Tracer {
    void start(unsigned idx) const
    {
        std::cout << ("start:\tidx: ") << idx << "\n";
    }

    void new_id(unsigned old_id, unsigned new_id) const
    {
        std::cout << ("new_id:\told: ") << old_id << "\tnew: " << new_id << "\n";
    }

    bool open(unsigned idx, const char * s, unsigned num_cap) const
    {
        std::cout << "open:\tidx: " << idx << "\tc: " << *s << "\tcap: " << num_cap << "\n";
        return true;
    }

    bool close(unsigned idx, const char * s, unsigned num_cap) const
    {
        std::cout << "close:\tidx: " << idx << "\tc: " << *s << "\tcap: " << num_cap << "\n";
        return true;
    }

    void fail(unsigned idx) const
    {
        std::cout << ("fail:\tidx: ") << idx << "\n";
    }

    void good(unsigned idx) const
    {
        std::cout << ("good:\tidx: ") << idx << "\n";
    }
};

int main(int argc, char **argv) {
    std::ios::sync_with_stdio(false);

    using namespace rndfa;

    if (argc < 2) {
        std::cerr << argv[0] << (" regex") << std::endl;
    }
    const char * rgxstr = argv[1];
    const char * msg = 0;
    StateBase * st = str2reg(argv[1], &msg);
    if (msg) {
        std::cerr << msg << std::endl;
        return 2;
    }
    display_state(st);
    std::cout.flush();

    if (argc < 3) {
        return 0;
    }
    else {
        argv[1] = argv[2];
    }

    Regex regex(st);

    //display_state(st);

    regex_t rgx;
    if (0 != regcomp(&rgx, rgxstr, REG_EXTENDED)) {
        std::cout << ("regcomp error") << std::endl;
        regcomp(&rgx, "", REG_EXTENDED);
    }
    regmatch_t regmatch[3];

    bool ismatch1 = false;
    bool ismatch2 = false;
    bool ismatch3 = false;
    bool ismatch4 = false;
    double d1, d2, d3, d4;

    const char * str = argc > 1 ? argv[1] : "abcdef";

#ifndef ITERATION
# define ITERATION 100000
#endif
    {
        regexec(&rgx, str, 1, regmatch, 0); //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch1 = 0 == regexec(&rgx, str, 1, regmatch, 0);
        }
        d1 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch2 = regex.search(str);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::streambuf * dbuf = std::cout.rdbuf(0);
    {
        struct test {
            inline static bool
            impl(regex_t& rgx, const char * s, regmatch_t * m, unsigned size) {
                const char * str = s;
                while (0 == regexec(&rgx, s, size, m, 0)) {
                    if (!m[0].rm_eo) {
                        break;
                    }
                    for (unsigned i = 1; i < size; i++) {
                        if (m[i].rm_so == -1) {
                            break;
                        }
                        int start = m[i].rm_so + (s - str);
                        int finish = m[i].rm_eo + (s - str);
                        //std::cout.write(str+start, finish-start) << "\n";
                    }
                    s += m[0].rm_eo;
                }
                return 0 == *s;
            }
        };
        test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0])); //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch3 = test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0]));
        }
        d3 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            if ((ismatch4 = regex.exact_search_with_matches(str))) {
                Regex::range_matches match_result = regex.match_result();
                typedef Regex::range_matches::iterator iterator;
                for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
                    //std::cout.write(str+first->first, first->second) << "\n";
                }
            }
        }
        d4 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    {
        regex.exact_search_with_matches(str, Tracer());
    }
    //std::cout.rdbuf(dbuf);

    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);
    std::cout
#if GENERATE_ST
    << "regex: " << rgxstr << "\n"
#else
    << "regex: '.* .* (.*) (.*) .*a'\n"
#endif
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
            (std::cout << "\tmatch: '").write(str+start, finish-start) << "'\n";
        }
    }
    if (ismatch4) {
        std::cout << ("with dfa\n");
        Regex::range_matches match_result = regex.match_result();
        typedef Regex::range_matches::iterator iterator;
        for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
            (std::cout << "\tmatch: '").write(first->first, first->second-first->first) << "'\n";
        }
        std::cout.flush();
    }
    regfree(&rgx);
}
