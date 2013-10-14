/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
 */

#ifndef REDEMPTION_REGEX_HPP
#define REDEMPTION_REGEX_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <stdint.h>

///TODO regex compiler ("..." -> C++)

namespace rndfa {
    using std::size_t;
    typedef uint32_t char_int;

    struct utf_char
    {
        utf_char(char_int c)
        : utfc(c)
        {}

        char_int utfc;
    };

    inline std::ostream& operator<<(std::ostream& os, utf_char utf_c)
    {
        char c[] = {
            char((utf_c.utfc & 0XFF000000) >> 24),
            char((utf_c.utfc & 0X00FF0000) >> 16),
            char((utf_c.utfc & 0X0000FF00) >> 8),
            char((utf_c.utfc & 0X000000FF)),
        };
        if (c[0]) {
            return os.write(c, 4);
        }
        else if (c[1]) {
            return os.write(c+1, 3);
        }
        else if (c[2]) {
            return os.write(c+2, 2);
        }
        else {
            os.write(c+3, 1);
        }
        return os;
    }

    inline std::string& operator+=(std::string& str, utf_char utf_c)
    {
        char c[] = {
            char((utf_c.utfc & 0XFF000000) >> 24),
            char((utf_c.utfc & 0X00FF0000) >> 16),
            char((utf_c.utfc & 0X0000FF00) >> 8),
            char((utf_c.utfc & 0X000000FF)),
        };
        if (c[0]) {
            str += c[0];
            str += c[1];
            str += c[2];
        }
        else if (c[1]) {
            str += c[1];
            str += c[2];
        }
        else if (c[2]) {
            str += c[2];
        }
        str += c[3];
        return str;
    }

    class utf_consumer
    {
    public:
        utf_consumer(const char * str)
        : s(reinterpret_cast<const unsigned char *>(str))
        {}

        char_int bumpc()
        {
            char_int c = *this->s;
            ++this->s;
            if (*this->s >> 6 == 2) {
                c <<= 8;
                c |= *this->s;
                ++this->s;
                if (*this->s >> 6 == 2) {
                    c <<= 8;
                    c |= *this->s;
                    ++this->s;
                    if (*this->s >> 6 == 2) {
                        c <<= 8;
                        c |= *this->s;
                        ++this->s;
                    }
                }
            }
            return c;
        }

        char_int getc() const
        {
            return utf_consumer(this->str()).bumpc();
        }

        bool valid() const
        {
            return *this->s;
        }

        const char * str() const
        {
            return reinterpret_cast<const char *>(s);
        }

        void str(const char * str)
        {
            s = reinterpret_cast<const unsigned char *>(str);
        }

        const unsigned char * s;
    };

    inline bool utf_contains(const char * str, char_int c)
    {
        utf_consumer consumer(str);
        while (consumer.valid()) {
            if (consumer.bumpc() == c) {
                return true;
            }
        }
        return false;
    }

    const unsigned NORMAL           = 0;
    const unsigned ANY_CHARACTER    = 1 << 8;
    const unsigned SPLIT            = 1 << 9;
    const unsigned CAPTURE_OPEN     = 1 << 10;
    const unsigned CAPTURE_CLOSE    = 1 << 11;
    const unsigned SPECIAL_CHECK    = 1 << 12;
    const unsigned EPSILONE         = 1 << 13;
    const unsigned FIRST            = 1 << 14;
    const unsigned LAST             = 1 << 15;
    const unsigned FINISH              = 1 << 16;

    struct StateBase
    {
        StateBase(unsigned type, char_int c = 0, StateBase * out1 = 0, StateBase * out2 = 0)
        : utfc(c)
        , type(type)
        , id(0)
        , num(0)
        , out1(out1)
        , out2(out2)
        {}

        virtual ~StateBase()
        {}

        virtual bool check(char_int c)
        {
            /**///std::cout << num << ": " << char(this->c & ANY_CHARACTER ? '.' : this->c&0xFF);
            return (this->type == ANY_CHARACTER || this->utfc == c);
        }

        virtual StateBase * clone() const
        {
            return new StateBase(this->type, this->utfc);
        }

        virtual void display(std::ostream& os) const
        {
            if (this->type == ANY_CHARACTER) {
                os << "any";
            }
            else if (this->type & (SPLIT|CAPTURE_CLOSE|CAPTURE_OPEN)){
                os << (this->is_split() ? "(split)" : this->type == CAPTURE_OPEN ? "(open)" : "(close)");
            }
            else if (this->type == FINISH) {
                os << "@";
            }
            else {
                os << "'" << utf_char(this->utfc) << "'";
            }
        }

        bool is_border() const
        { return this->type & (FIRST|LAST); }

        bool is_cap() const
        { return this->type & (CAPTURE_OPEN|CAPTURE_CLOSE); }

        bool is_cap_open() const
        { return this->type == CAPTURE_OPEN; }

        bool is_cap_close() const
        { return this->type == CAPTURE_CLOSE; }

        bool is_split() const
        { return this->type == SPLIT; }

        bool is_epsilone() const
        { return this->type == EPSILONE; }

        bool is_finish() const
        { return this->type == FINISH; }

        char_int utfc;

        unsigned type;
        unsigned id;
        unsigned num;

        StateBase *out1;
        StateBase *out2;
    };

    inline std::ostream& operator<<(std::ostream& os, StateBase& st)
    {
        st.display(os);
        return os;
    }

    typedef StateBase State;

    struct StateFinish : StateBase
    {
        StateFinish()
        : StateBase(FINISH)
        {}

        virtual ~StateFinish()
        {}

        virtual bool check(char_int /*c*/)
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return true;
        }

        virtual StateBase * clone() const
        {
            return new StateFinish;
        }

        virtual void display(std::ostream& os) const
        {
            os << "Finish";
        }
    };

    StateFinish state_finish = StateFinish();

    struct StateRange : StateBase
    {
        StateRange(char_int r1, char_int r2, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, r1, out1, out2)
        , rend(r2)
        {}

        virtual ~StateRange()
        {}

        virtual bool check(char_int c)
        {
            /**///std::cout << char(this->c&0xFF) << "-" << char(rend);
            return this->utfc <= c && c <= rend;
        }

        virtual StateBase * clone() const
        {
            return new StateRange(this->utfc, this->rend);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << utf_char(this->utfc) << "-" << utf_char(this->rend) << "]";
        }

        char_int rend;
    };

    template<char Identifier, typename Trait>
    struct StateIdentifier : StateBase
    {
        StateIdentifier(StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, 0, out1, out2)
        {}

        virtual ~StateIdentifier()
        {}

        virtual StateIdentifier * clone() const
        {
            return new StateIdentifier;
        }

        virtual void display(std::ostream& os) const
        {
            os << "\\" << Identifier;
        }

        virtual bool check(char_int c)
        { return Trait::check(c); }
    };

    struct IdentifierDigitTrait
    {
        static bool check(char_int c)
        {
            return ('0' <= c && c <= '9');
        }
    };

    struct IdentifierWordTrait
    {
        static bool check(char_int c)
        {
            return ('a' <= c && c <= 'z')
                || ('A' <= c && c <= 'Z')
                || ('0' <= c && c <= '9')
                || c == '_';
        }
    };

    struct IdentifierSpaceTrait
    {
        static bool check(char_int c)
        {
            return (' ' == c || '\t' == c || '\n' == c);
        }
    };

    template<typename Trait>
    struct IdentifierNoTrait
    {
        static bool check(char_int c)
        {
            return !Trait::check(c);
        }
    };

    typedef IdentifierNoTrait<IdentifierWordTrait>  IdentifierNoWordTrait;
    typedef IdentifierNoTrait<IdentifierDigitTrait> IdentifierNoDigitTrait;
    typedef IdentifierNoTrait<IdentifierSpaceTrait> IdentifierNoSpaceTrait;

    typedef StateIdentifier<'w', IdentifierWordTrait>       StateWord;
    typedef StateIdentifier<'W', IdentifierNoWordTrait>     StateNoWord;
    typedef StateIdentifier<'d', IdentifierDigitTrait>      StateDigit;
    typedef StateIdentifier<'D', IdentifierNoDigitTrait>    SateNoDigit;
    typedef StateIdentifier<'s', IdentifierSpaceTrait>      StateSpace;
    typedef StateIdentifier<'S', IdentifierNoSpaceTrait>    StateNoSpace;

    struct StateCharacters : StateBase
    {
        StateCharacters(const std::string& s, StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(NORMAL, 0, out1, out2)
        , str(s)
        {}

        virtual ~StateCharacters()
        {}

        virtual bool check(char_int c)
        {
            /**///std::cout << str;
            return utf_contains(this->str.c_str(), c);
        }

        virtual StateBase * clone() const
        {
            return new StateCharacters(this->str);
        }

        virtual void display(std::ostream& os) const
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

        virtual bool check(char_int /*c*/)
        {
            /**///std::cout << (this->c == FIRST ? "^" : "$");
            return false;
        }

        virtual StateBase * clone() const
        {
            return new StateBorder(this->type == FIRST);
        }

        virtual void display(std::ostream& os) const
        {
            os << (this->type == FIRST ? "^" : "$");
        }
    };

    struct StateMultiTest : StateBase
    {
        struct Checker {
            virtual bool check(char_int c) = 0;
            virtual Checker * clone() const = 0;
            virtual void display(std::ostream& os) const = 0;
            virtual ~Checker() {}
        };

        std::vector<Checker*> checkers;
        typedef std::vector<Checker*>::iterator checker_iterator;
        typedef std::vector<Checker*>::const_iterator checker_const_iterator;
        bool result_true_check;


        StateMultiTest(StateBase* out1 = 0, StateBase* out2 = 0)
        : StateBase(SPECIAL_CHECK, 0, out1, out2)
        , checkers()
        , result_true_check(true)
        {}

        virtual ~StateMultiTest()
        {
            for (checker_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                delete *first;
            }
        }

        virtual bool check(char_int c)
        {
            for (checker_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                if ((*first)->check(c)) {
                    return this->result_true_check;
                }
            }
            return !this->result_true_check;
        }

        virtual StateBase * clone() const
        {
            StateMultiTest * ret = new StateMultiTest;
            ret->result_true_check = this->result_true_check;
            for (checker_const_iterator first = this->checkers.begin(), last = this->checkers.end(); first != last; ++first) {
                ret->push_checker((*first)->clone());
            }
            return ret;
        }

        virtual void display(std::ostream& os) const
        {
            checker_const_iterator first = this->checkers.begin();
            checker_const_iterator last = this->checkers.end();
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

        virtual bool check(char_int c)
        {
            return utf_contains(this->str.c_str(), c);
        }

        virtual Checker * clone() const
        {
            return new CheckerString(this->str);
        }

        virtual void display(std::ostream& os) const
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

        virtual bool check(char_int c)
        {
            return this->begin <= c && c <= this->end;
        }

        virtual Checker* clone() const
        {
            return new CheckerInterval(this->begin, this->end);
        }

        virtual void display(std::ostream& os) const
        {
            os << "[" << utf_char(this->begin) << "-" << utf_char(this->end) << "]";
        }

        char_int begin;
        char_int end;
    };

    template<char Identifier, typename Trait>
    struct CheckerIdentifier : StateMultiTest::Checker
    {
        CheckerIdentifier()
        {}

        virtual ~CheckerIdentifier()
        {}

        virtual bool check(char_int c)
        {
            return Trait::check(c);
        }

        virtual Checker* clone() const
        {
            return new CheckerIdentifier;
        }

        virtual void display(std::ostream& os) const
        {
            os << "\\" << Identifier;
        }
    };

    typedef CheckerIdentifier<'w', IdentifierWordTrait>     CheckerWord;
    typedef CheckerIdentifier<'W', IdentifierNoWordTrait>   CheckerNoWord;
    typedef CheckerIdentifier<'d', IdentifierDigitTrait>    CheckerDigit;
    typedef CheckerIdentifier<'D', IdentifierNoDigitTrait>  CheckerNoDigit;
    typedef CheckerIdentifier<'s', IdentifierSpaceTrait>    CheckerSpace;
    typedef CheckerIdentifier<'S', IdentifierNoSpaceTrait>  CheckerNoSpace;


    class StateMachine2
    {
        class RangeList;
        class StateList;

        struct is_begin_state {
            bool operator()(const StateList& stl) const
            {
                return stl.st->type == FIRST;
            }
        };

        StateMachine2(const StateMachine2&) /*= delete*/;

    public:
        explicit StateMachine2(StateBase * st = 0)
        : nb_capture(0)
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        , captures(0)
        , pcaptures(0)
        , traces(0)
        , states()
        , l1()
        , l2()
        , st_list(0)
        , st_range_list(0)
        {
            this->push_state(st);
            if (this->states.empty()) {
                return ;
            }
            this->nb_capture /= 2;
            l1.set_parray(new StateListByStep::Info[this->states.size() * 2]);
            l2.set_parray(l1.array + this->states.size());

            if (!this->states.empty())
            {
                {
                    const unsigned matrix_size = this->states.size() * this->states.size();
                    this->st_list = new StateList[matrix_size];
                    std::memset(this->st_list, 0, matrix_size * sizeof * this->st_list);
                }

                {
                    const unsigned size = this->states.size();
                    this->st_range_list = new RangeList[size];
                    this->st_range_list_last = this->st_range_list;
                    for (unsigned n = 0; n < size; ++n) {
                        RangeList& l = *this->st_range_list_last;
                        ++this->st_range_list_last;
                        l.st = 0;
                        l.first = this->st_list + n * this->states.size();
                        l.last = l.first;
                    }
                }

                if (this->nb_capture) {
                    const unsigned col = this->states.size() - this->nb_capture * 2;
                    const unsigned matrix_size = col * this->nb_capture * 2;

                    this->captures = new StateBase const *[this->nb_capture * 2 + matrix_size];
                    this->traces = reinterpret_cast<const char **>(this->captures + this->nb_capture * 2);

                    this->idx_trace_free = new unsigned[col+1];

                    this->pcaptures = this->captures;

                    for (state_iterator first = this->states.begin(), last = this->states.end(); first != last; ++first) {
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
                if (st_range_beginning.first != this->st_range_list->first) {
                    this->st_range_beginning.last = this->st_range_beginning.first;
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

                    this->st_range_list->first += (this->st_range_beginning.last - this->st_range_beginning.first);
                }
                else {
                    this->st_range_beginning.st = this->st_range_list->st;
                    this->st_range_beginning.first = this->st_range_list->first;
                    this->st_range_beginning.last = this->st_range_list->last;
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
            assert(this->pidx_trace_free <= this->idx_trace_free + this->states.size() - this->nb_capture * 2);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

        void push_state(RangeList* l, StateBase * st, unsigned step)
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
                if (first->st->out1 == &state_finish) {
                    first->next = 0;
                    continue;
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
        bool exact_search(const char * s, unsigned step_limit, bool check_end = true)
        {
            if (this->states.empty()) {
                return false;
            }
            return Searching(*this).exact_search(s, step_limit, check_end);
        }

        bool exact_search_with_trace(const char * s, unsigned step_limit,
                                     bool check_end = true)
        {
            if (this->nb_capture == 0) {
                return exact_search(s, step_limit, check_end);
            }
            return Matching(*this).match(s, step_limit, DefaultMatchTracer(),
                                         Matching::is_exact());
        }

        template<typename Tracer>
        bool exact_search_with_trace(const char * s, unsigned step_limit,
                                     Tracer tracer, bool check_end = true)
        {
            if (this->nb_capture == 0) {
                return exact_search(s, step_limit, check_end);
            }
            return Matching(*this).match(s, step_limit, tracer, Matching::is_exact());
        }

        bool search(const char * s, unsigned step_limit)
        {
            if (this->states.empty()) {
                return false;
            }
            return Searching(*this).search(s, step_limit);
        }

        bool search_with_trace(const char * s, unsigned step_limit)
        {
            if (this->nb_capture == 0) {
                return search(s, step_limit);
            }
            return Matching(*this).match(s, step_limit, DefaultMatchTracer(),
                                         Matching::is_not_exact());
        }

        template<typename Tracer>
        bool search_with_trace(const char * s, unsigned step_limit, Tracer tracer)
        {
            if (this->nb_capture == 0) {
                return search(s, step_limit);
            }
            return Matching(*this).match(s, step_limit, tracer, Matching::is_not_exact());
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
                st->num = this->states.size();
                if (st->is_cap()) {
                    st->num = this->nb_capture;
                    ++this->nb_capture;
                }
                this->states.push_back(st);
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

            size_t size() const
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

            RangeList * step(char_int c, RangeList * l, unsigned & step_count)
            {
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    ++step_count;
                    if (!first->st->is_cap() && first->st->check(c)) {
#ifdef DISPLAY_TRACE
                        this->sm.display_elem_state_list(*first, 0);
#endif
                        if ( ! first->next && first->st->is_finish()) {
                            continue ;
                        }
                        return first->next;
                    }
                }

                return reinterpret_cast<RangeList*>(1);
            }

            bool exact_search(const char * s, unsigned step_limit, bool check_end = true)
            {
#ifdef DISPLAY_TRACE
                this->sm.display_dfa();
#endif

                this->sm.reset_id();

                RangeList * l = &this->sm.st_range_beginning;

                utf_consumer consumer(s);

                unsigned step_count = 0;

                while (consumer.valid() && l > reinterpret_cast<void*>(1)){
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m\n";
#endif
                    l = this->step(consumer.bumpc(), l, step_count);
                    if (step_count >= step_limit) {
                        return false;
                    }
                    ++this->step_id;
                }

                if ((0 == l && (check_end ? !consumer.valid() : 1))) {
                    return true;
                }
                if (reinterpret_cast<void*>(1) == l || consumer.valid()) {
                    return false;
                }

                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    if (first->st->type == LAST || first->st->is_finish()) {
                        return true;
                    }
                }

                return false;
            }

            unsigned step(char_int c, StateListByStep * l1, StateListByStep * l2, unsigned & step_count)
            {
                for (Info* ifirst = l1->begin(), * ilast = l1->end(); ifirst != ilast ; ++ifirst) {
                    ++step_count;
                    if (ifirst->rl->st->id == this->step_id) {
                        /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                        continue;
                    }
                    ifirst->rl->st->id = this->step_id;

                    for (StateList * first = ifirst->rl->first, * last = ifirst->rl->last; first != last; ++first) {
                        ++step_count;
                        if (!first->st->is_cap() && first->st->check(c)) {
#ifdef DISPLAY_TRACE
                            this->sm.display_elem_state_list(*first, 0);
#endif
                            if (0 == first->next) {
                                return 0;
                            }
                            l2->push_back(first->next);
                        }
                    }
                }
                return -1u;
            }

            bool search(const char * s, unsigned step_limit)
            {
#ifdef DISPLAY_TRACE
                this->sm.display_dfa();
#endif

                this->sm.l1.clear();
                this->sm.l2.clear();
                this->sm.reset_id();

                StateListByStep * pal1 = &this->sm.l1;
                StateListByStep * pal2 = &this->sm.l2;
                this->sm.l1.push_back(&this->sm.st_range_beginning);

                utf_consumer consumer(s);

                unsigned step_count = 0;

                while (consumer.valid()) {
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m\n";
#endif
                    if (0 == this->step(consumer.bumpc(), pal1, pal2, step_count)) {
                        return true;
                    }
                    if (step_count >= step_limit) {
                        return false;
                    }
                    ++this->step_id;
                    std::swap(pal1, pal2);
                    pal2->clear();
                    pal1->push_back(this->sm.st_range_list);
                }

                for (Info* ifirst = pal1->begin(), * ilast = pal1->end(); ifirst != ilast ; ++ifirst) {
                    for (StateList * first = ifirst->rl->first, * last = ifirst->rl->last; first != last; ++first) {
                        if (first->st->type == LAST || first->st->is_finish()) {
                            return true;
                        }
                    }
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
            const unsigned size = this->states.size() - this->nb_capture * 2;
            //pidx_trace_free ∈ [0, size+1]
            for (unsigned i = 0; i != size; ++i, ++this->pidx_trace_free) {
                *this->pidx_trace_free = i;
            }
            std::memset(this->traces, 0,
                        size * this->nb_capture * 2 * sizeof this->traces[0]);
        }

    public:
        void display_elem_state_list(const StateList& e, unsigned idx) const
        {
            std::cout << "\t\033[33m" << idx << "\t" << e.st->num << "\t" << e.st->utfc << "\t"
            << *e.st << "\t" << (e.next) << "\033[0m" << std::endl;
        }

        void display_dfa(const RangeList * l, const RangeList * last) const
        {
            for (; l < last && l->first != l->last; ++l) {
                std::cout << l << "  st: " << l->st->num << (l->st->is_cap() ? " (cap)\n" : "\n");
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    std::cout << "\t" << first->st->num << "\t" << first->st->utfc << "\t"
                    << *first->st << "\t" << first->next << ("\n");
                }
            }
        }

        void display_dfa() const
        {
            if (this->st_range_beginning.first != this->st_range_beginning.last) {
                std::cout << ("beginning") << std::endl;
                this->display_dfa(&this->st_range_beginning, &this->st_range_beginning+1);
                std::cout << ("\nrange") << std::endl;
            }

            this->display_dfa(this->st_range_list, this->st_range_list_last);
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
            unsigned step(const char * s, char_int c,
                          StateListByStep * l1, StateListByStep * l2,
                          Tracer& tracer, unsigned & step_count)
            {
                for (Info* ifirst = l1->begin(), * ilast = l1->end(); ifirst != ilast ; ++ifirst) {
                    if (ifirst->rl->st->id == this->step_id) {
#ifdef DISPLAY_TRACE
                        std::cout << "\t\033[35mdup " << (ifirst->idx) << "\033[0m\n";
#endif
                        tracer.fail(ifirst->idx);
                        this->sm.push_idx_trace(ifirst->idx);
                        continue;
                    }

                    unsigned new_trace = 0;
                    ifirst->rl->st->id = this->step_id;
                    StateList * first = ifirst->rl->first;
                    StateList * last = ifirst->rl->last;

                    for (; first != last; ++first) {
                        ++step_count;
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

                        if (first->st->check(c)) {
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
            bool match(const char * s, unsigned step_limit, Tracer tracer, ExactMatch)
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

                utf_consumer consumer(s);

                unsigned step_count = 0;

                while (consumer.valid()) {
#ifdef DISPLAY_TRACE
                    std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m" << std::endl;
#endif
                    if (-1u != (this->sm.idx_trace = this->step(s, consumer.bumpc(), pal1, pal2, tracer, step_count))) {
                        tracer.good(this->sm.idx_trace);
                        return false == ExactMatch::value || !consumer.getc();
                    }
                    if (pal2->empty()) {
                        break;
                    }
                    if (step_count >= step_limit) {
                        return false;
                    }
                    ++this->step_id;
                    std::swap(pal1, pal2);
                    pal2->clear();
                    if (false == ExactMatch::value) {
                        --this->sm.pidx_trace_free;
                        assert(this->sm.pidx_trace_free >= this->sm.idx_trace_free);
#ifdef DISPLAY_TRACE
                        std::cout << "\t\033[32m-> " << *this->sm.pidx_trace_free << "\033[0m" << std::endl;
#endif
                        pal1->push_back(this->sm.st_range_list, *this->sm.pidx_trace_free);
                    }
                    s = consumer.str();
                }

                if (!consumer.valid()) {
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

                            if (first->st->type == LAST) {
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
        state_list states;
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

    inline StateBase * new_character(char_int c, StateBase * out1 = 0)
    {
        return new StateBase(NORMAL, c, out1);
    }

    inline StateBase * new_any(StateBase * out1 = 0)
    {
        return new StateBase(ANY_CHARACTER, 0, out1);
    }

    inline StateBase * new_split(StateBase * out1 = 0, StateBase * out2 = 0)
    {
        return new StateBase(SPLIT, 0, out1, out2);
    }

    inline StateBase * c2st(char_int c)
    {
        switch (c) {
            case 'd': return new StateDigit;
            case 'D': return new SateNoDigit;
            case 'w': return new StateWord;
            case 'W': return new StateNoWord;
            case 's': return new StateSpace;
            case 'S': return new StateNoSpace;
            case 'n': return new_character('\n');
            case 't': return new_character('\t');
            case 'r': return new_character('\r');
            //case 'v': return new_character('\v');
            default : return new_character(c);
        }
    }

    inline const char * check_interval(char_int a, char_int b)
    {
        bool valid = ('0' <= a && a <= '9' && '0' <= b && b <= '9')
                  || ('a' <= a && a <= 'z' && 'a' <= b && b <= 'z')
                  || ('A' <= a && a <= 'Z' && 'A' <= b && b <= 'Z');
        return (valid && a <= b) ? 0 : "range out of order in character class";
    }

    inline StateBase * str2stchar(utf_consumer & consumer, char_int c, const char * & msg_err)
    {
        if (c == '\\' && consumer.valid()) {
            return c2st(consumer.bumpc());
        }

        if (c == '[') {
            StateMultiTest * st = new StateMultiTest;
            std::string str;
            if (consumer.valid() && (c = consumer.bumpc()) != ']') {
                if (c == '^') {
                    st->result_true_check = false;
                    c = consumer.bumpc();
                }
                if (c == '-') {
                    str += '-';
                    c = consumer.bumpc();
                }
                const unsigned char * cs = consumer.s;
                while (consumer.valid() && c != ']') {
                    const unsigned char * p = consumer.s;
                    char_int prev_c = c;
                    while (c != ']' && c != '-') {
                        if (c == '\\') {
                            char_int cc = consumer.bumpc();
                            switch (cc) {
                                case 'd': st->push_checker(new CheckerDigit); break;
                                case 'D': st->push_checker(new CheckerNoDigit); break;
                                case 'w': st->push_checker(new CheckerWord); break;
                                case 'W': st->push_checker(new CheckerNoWord); break;
                                case 's': st->push_checker(new CheckerSpace); break;
                                case 'S': st->push_checker(new CheckerNoSpace); break;
                                case 'n': str += '\n'; break;
                                case 't': str += '\t'; break;
                                case 'r': str += '\r'; break;
                                //case 'v': str += '\v'; break;
                                default : str += utf_char(cc); break;
                            }
                        }
                        else {
                            str += utf_char(c);
                        }

                        if ( ! consumer.valid()) {
                            break;
                        }

                        prev_c = c;
                        c = consumer.bumpc();
                    }

                    if (c == '-') {
                        if (cs == consumer.s) {
                            str += '-';
                        }
                        else if (!consumer.valid()) {
                            msg_err = "missing terminating ]";
                            return 0;
                        }
                        else if (consumer.getc() == ']') {
                            str += '-';
                            consumer.bumpc();
                        }
                        else if (consumer.s == p) {
                            str += '-';
                        }
                        else {
                            c = consumer.bumpc();
                            if ((msg_err = check_interval(prev_c, c))) {
                                return 0;
                            }
                            if (str.size()) {
                                str.erase(str.size()-1);
                            }
                            st->push_checker(new CheckerInterval(prev_c, c));
                            cs = consumer.s;
                            if (consumer.valid()) {
                                c = consumer.bumpc();
                            }
                        }
                    }
                }
            }

            if (!str.empty()) {
                st->push_checker(new CheckerString(str));
            }

            if (c != ']') {
                msg_err = "missing terminating ]";
                delete st;
                st = 0;
            }

            return st;
        }

        return c == '.' ? new_any() : new_character(c);
    }

    inline bool is_range_repetition(const char * s)
    {
        const char * begin = s;
        while (*s && '0' <= *s && *s <= '9') {
            ++s;
        }
        if (begin == s || !*s || (*s != ',' && *s != '}')) {
            return false;
        }
        if (*s == '}') {
            return true;
        }
        begin = ++s;
        while (*s && '0' <= *s && *s <= '9') {
            ++s;
        }
        return *s && *s == '}';
    }

    inline bool is_meta_char(utf_consumer & consumer, char_int c)
    {
        return c == '*' || c == '+' || c == '?' || c == '|' || c == '(' || c == ')' || c == '^' || c == '$' || (c == '{' && is_range_repetition(consumer.str()));
    }

    struct StateDeleter
    {
        void operator()(StateBase * st) const
        {
            delete st;
        }
    };

    inline void append_state(StateBase * st, std::vector<StateBase*>& sts)
    {
        if (st && st->id != -4u) {
            st->id = -4u;
            sts.push_back(st);
            append_state(st->out1, sts);
            append_state(st->out2, sts);
        }
    }

    inline void append_state_whitout_finish(StateBase * st, std::vector<StateBase*>& sts)
    {
        if (st && st->id != -4u && st != &state_finish) {
            st->id = -4u;
            sts.push_back(st);
            append_state_whitout_finish(st->out1, sts);
            append_state_whitout_finish(st->out2, sts);
        }
    }

    inline void free_st(StateBase * st)
    {
        std::vector<StateBase*> sts;
        append_state_whitout_finish(st, sts);
        std::for_each(sts.begin(), sts.end(), StateDeleter());
    }

    struct ContextClone {
        std::vector<StateBase*> sts;
        std::vector<StateBase*> sts2;
        const StateBase * st;
        size_t p2;
        const bool definied_out1;
        const bool definied_out2;

        ContextClone(const StateBase * st_base)
        : sts()
        , sts2()
        , st(st_base)
        , definied_out1(st->out1)
        , definied_out2(st->out2)
        {
            if (this->definied_out1) {
                append_state(st->out1, this->sts);
            }
            if (this->definied_out2) {
                this->p2 = this->sts.size();
                append_state(st->out2, this->sts);
            }
            this->sts2.reserve(this->sts.size());
        }

        StateBase * clone()
        {
            StateBase * replicant = this->simple_clone(this->st);
            if (this->definied_out1 || this->definied_out2) {
                typedef std::vector<StateBase*>::iterator iterator;
                iterator last = this->sts.end();
                for (iterator first = this->sts.begin(), first2 = sts2.begin(); last != this->sts.end(); ++first, ++first2) {
                    *first2 = this->simple_clone(*first);
                    (*first)->id = 0;
                }
                for (iterator first = this->sts.begin(), first2 = sts2.begin(); last != this->sts.end(); ++first, ++first2) {
                    if ((*first)->out1) {
                        (*first2)->out1 = this->sts2[std::find(this->sts.begin(), this->sts.end(), (*first)->out1) - this->sts.begin()];
                    }
                    if ((*first)->out2) {
                        (*first2)->out2 = this->sts2[std::find(this->sts.begin(), this->sts.end(), (*first)->out2) - this->sts.begin()];
                    }
                }
                if (this->definied_out1) {
                    replicant->out1 = this->sts2[0];
                }
                if (this->definied_out2) {
                    replicant->out2 = this->sts2[p2];
                }
            }
            return replicant;
        }

        StateBase * simple_clone(const StateBase * st)
        {
            return st == &state_finish ? &state_finish : st->clone();
        }
    };

    typedef std::pair<StateBase*, StateBase**> IntermendaryState;

    inline IntermendaryState intermendary_str2reg(utf_consumer & consumer,
                                                  bool & has_epsilone,
                                                  const char * & msg_err,
                                                  int recusive = 0, bool ismatch = true)
    {
        struct FreeState {
            static IntermendaryState invalide(StateBase& st)
            {
                free_st(st.out2);
                return IntermendaryState(0,0);
            }
        };

        struct selected {
            static StateBase ** next_pst(StateBase ** pst)
            {
                if (*pst && *pst != &state_finish) {
                    pst = &(*pst)->out1;
                }
                return pst;
            }
        };

        StateBase st(0);
        StateBase ** pst = &st.out1;
        StateBase ** st_one = 0;
        StateBase ** prev_st_one = 0;
        StateBase * bst = &st;

        StateBase ** besplit[50] = {0};
        StateBase *** pesplit = besplit;

        char_int c = consumer.bumpc();

        while (c) {
            /**///std::cout << "c: " << (c) << std::endl;
            if (c == '^' || c == '$') {
                pst = selected::next_pst(pst);
                *pst = new StateBorder(c == '^');
                if (st_one) {
                    *st_one = *pst;
                    prev_st_one = st_one;
                    st_one = 0;
                }

                if ((c = consumer.bumpc()) && !is_meta_char(consumer, c)) {
                    pst = &(*pst)->out1;
                }
                continue;
            }

            if (!is_meta_char(consumer, c)) {
                pst = selected::next_pst(pst);
                if (!(*pst = str2stchar(consumer, c, msg_err))) {
                    return FreeState::invalide(st);
                }

                if (st_one) {
                    *st_one = *pst;
                    prev_st_one = st_one;
                    st_one = 0;
                }

                while ((c = consumer.bumpc()) && !is_meta_char(consumer, c)) {
                    pst = &(*pst)->out1;
                    if (!(*pst = str2stchar(consumer, c, msg_err))) {
                        return FreeState::invalide(st);
                    }
                }
            }
            else {
                if (c != '(' && c != ')' && (bst->out1 == 0 || bst->out1->is_border())) {
                    msg_err = "nothing to repeat";
                    return FreeState::invalide(st);
                }
                switch (c) {
                    case '?': {
                        StateBase ** tmp = st_one;
                        st_one = &(*pst)->out1;
                        *pst = new_split(0, *pst);
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                            prev_st_one = tmp;
                        }
                        pst = &(*pst)->out1;
                        break;
                    }
                    case '*':
                        *pst = new_split(&state_finish, *pst);
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                            prev_st_one = 0;
                        }
                        (*pst)->out2->out1 = *pst;
                        pst = &(*pst)->out1;
                        break;
                    case '+':
                        (*pst)->out1 = new_split(&state_finish, *pst);
                        pst = &(*pst)->out1->out1;
                        break;
                    case '|':
                        *pesplit = *pst ? &(*pst)->out1 : pst;
                        ++pesplit;
                        bst->out1 = new_split(0, bst->out1);
                        bst = bst->out1;
                        if (st_one) {
                            *pesplit = st_one;
                            st_one = 0;
                            ++pesplit;
                        }
                        pst = &bst->out1;
                        break;
                    case '{': {
                        /**///std::cout << ("{") << std::endl;
                        char * end = 0;
                        unsigned m = strtoul(consumer.str(), &end, 10);
                        /**///std::cout << ("end ") << *end << std::endl;
                        /**///std::cout << "m: " << (m) << std::endl;
                        if (*end != '}') {
                            /**///std::cout << ("reste") << std::endl;
                            if (*(end+1) == '}') {
                                /**///std::cout << ("infini") << std::endl;
                                if (m == 1) {
                                    (*pst)->out1 = new_split(0, *pst);
                                    pst = &(*pst)->out1->out1;
                                }
                                else if (m) {
                                    ContextClone cloner(*pst);
                                    pst = selected::next_pst(pst);
                                    while (--m) {
                                        *pst = cloner.clone();
                                        pst = &(*pst)->out1;
                                    }
                                    *pst = new_split(0, cloner.clone());
                                    (*pst)->out2->out1 = *pst;
                                    pst = &(*pst)->out1;
                                }
                                else {
                                    *pst = new_split(0, *pst);
                                    if (prev_st_one) {
                                        *prev_st_one = *pst;
                                        prev_st_one = 0;
                                    }
                                    (*pst)->out2->out1 = *pst;
                                    pst = &(*pst)->out1;
                                }
                            }
                            else {
                                /**///std::cout << ("range") << std::endl;
                                unsigned n = strtoul(end+1, &end, 10);
                                if (m > n || (0 == m && 0 == n)) {
                                    msg_err = "numbers out of order in {} quantifier";
                                    return FreeState::invalide(st);
                                }
                                /**///std::cout << "n: " << (n) << std::endl;
                                n -= m;
                                if (n > 50) {
                                    msg_err = "numbers too large in {} quantifier";
                                    return FreeState::invalide(st);
                                }
                                if (0 == m) {
                                    --end;
                                    /**///std::cout << ("m = 0") << std::endl;
                                    if (n != 1) {
                                        /**///std::cout << ("n != 1") << std::endl;
                                        ContextClone cloner(*pst);
                                        StateBase ** tmp = st_one;
                                        st_one = &(*pst)->out1;
                                        *pst = new_split(0, *pst);
                                        if (prev_st_one) {
                                            *prev_st_one = *pst;
                                            prev_st_one = tmp;
                                        }
                                        pst = &(*pst)->out1;

                                        while (--n) {
                                            *pst = cloner.clone();
                                            *st_one = *pst;
                                            prev_st_one = st_one;
                                            st_one = &(*pst)->out1;
                                            *pst = new_split(0, *pst);
                                            *prev_st_one = *pst;
                                            pst = &(*pst)->out1;
                                        }
                                    }
                                }
                                else {
                                    --end;
                                    ContextClone cloner(*pst);
                                    pst = selected::next_pst(pst);
                                    while (--m) {
                                        *pst = cloner.clone();
                                        pst = &(*pst)->out1;
                                    }

                                    StateBase * split = new_split();
                                    while (n--) {
                                        *pst = new_split(0, split);
                                        (*pst)->out1 = cloner.clone();
                                        (*pst)->out1->out1 = split;
                                        pst = &(*pst)->out2;
                                    }
                                    pst = &split->out1;
                                }
                            }
                        }
                        else if (0 == m) {
                            msg_err = "numbers is 0 in {}";
                            return FreeState::invalide(st);
                        }
                        else {
                            /**///std::cout << ("fixe ") << m << std::endl;
                            ContextClone cloner(*pst);
                            pst = selected::next_pst(pst);
                            while (--m) {
                                /**///std::cout << ("clone") << std::endl;
                                *pst = cloner.clone();
                                pst = &(*pst)->out1;
                            }
                            end -= 1;
                        }
                        consumer.str(end + 1 + 1);
                        /**///std::cout << "'" << (*consumer.s) << "'" << std::endl;
                        break;
                    }
                    case ')':
                        if (0 == recusive) {
                            msg_err = "unmatched parentheses";
                            return FreeState::invalide(st);
                        }

                        if (ismatch) {
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(CAPTURE_CLOSE);
                        }
                        else if (besplit != pesplit) {
                            has_epsilone = true;
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(ismatch ? CAPTURE_CLOSE : EPSILONE);
                        }

                        if (st_one) {
                            *st_one = *pst;
                            prev_st_one = st_one;
                            st_one = 0;
                        }
                        if (prev_st_one) {
                            *prev_st_one = *pst;
                        }

                        for (StateBase *** first = besplit; first != pesplit; ++first) {
                            if (**first) {
                                (**first)->out1 = *pst;
                            }
                            else {
                                (**first) = *pst;
                            }
                        }

                        return IntermendaryState(st.out1, pst);
                        break;
                    default:
                        return FreeState::invalide(st);
                    case '(':
                        if (*consumer.s == '?' && *(consumer.s+1) == ':') {
                            if (!*consumer.s || !(*consumer.s+1) || !(*consumer.s+2)) {
                                msg_err = "unmatched parentheses";
                                return FreeState::invalide(st);
                            }
                            consumer.s += 2;
                            IntermendaryState intermendary = intermendary_str2reg(consumer, has_epsilone, msg_err, recusive+1, false);
                            if (intermendary.first) {
                                pst = selected::next_pst(pst);
                                *pst= intermendary.first;
                                if (st_one) {
                                    *st_one = *pst;
                                    prev_st_one = st_one;
                                    st_one = 0;
                                }
                                pst = intermendary.second;
                            }
                            else if (0 == intermendary.second) {
                                return FreeState::invalide(st);
                            }
                            break;
                        }
                        IntermendaryState intermendary = intermendary_str2reg(consumer, has_epsilone, msg_err, recusive+1);
                        if (intermendary.first) {
                            pst = selected::next_pst(pst);
                            *pst = new StateBase(CAPTURE_OPEN, 0, intermendary.first);
                            if (st_one) {
                                *st_one = *pst;
                                prev_st_one = st_one;
                                st_one = 0;
                            }
                            pst = intermendary.second;
                        }
                        else if (0 == intermendary.second) {
                            return FreeState::invalide(st);
                        }
                        break;
                }
                c = consumer.bumpc();
            }
        }

        if (0 != recusive) {
            msg_err = "unmatched parentheses";
            return FreeState::invalide(st);
        }
        return IntermendaryState(st.out1, pst);
    }

    inline void remove_epsilone(StateBase * st, std::vector<StateBase*>& epsilone_sts)
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

    inline StateBase* str2reg(const char * s, const char * * msg_err = 0,
                              size_t * pos_err = 0)
    {
        bool has_epsilone = false;
        const char * err = 0;
        utf_consumer consumer(s);
        StateBase * st = intermendary_str2reg(consumer, has_epsilone, err).first;
        if (err) {
            if (msg_err) {
                *msg_err = err;
            }
            if (pos_err) {
                *pos_err = consumer.str() - s;
            }
        }
        else if (has_epsilone) {
            typedef std::vector<StateBase*> states_t;
            states_t removed;
            remove_epsilone(st, removed);
            std::for_each(removed.begin(), removed.end(), StateDeleter());
        }
        return st;
    }

    inline void display_state(StateBase * st, unsigned depth = 0)
    {
        if (st && st->id != -1u-1u) {
            std::string s(depth, '\t');
            std::cout
            << s << "\033[33m" << st << "\t" << st->num << "\t" << st->utfc << "\t"
            << *st << "\033[0m\n\t" << s << st->out1 << "\n\t" << s << st->out2 << "\n";
            st->id = -1u-1u;
            display_state(st->out1, depth+1);
            display_state(st->out2, depth+1);
        }
    }


    inline bool st_exact_step(std::vector<StateBase*>& l1,
                              std::vector<StateBase*>& l2,
                              size_t c, utf_consumer & consumer, unsigned count)
    {
        struct add {
            static bool impl(std::vector<StateBase*>& l, StateBase * st,
                             bool is_end, unsigned count)
            {
                if (st->id == count) {
                    return false;
                }
                st->id = count;
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, is_end, count)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, is_end, count)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, is_end, count);
                    }
                }
                else if (st) {
                    if (st->is_finish() || st->type == LAST) {
                        if (is_end) {
                            return true;
                        }
                    }
                    else {
                        l.push_back(st);
                    }
                }
                return false;
            }
        };

        const bool is_end = ! consumer.valid();
        typedef std::vector<StateBase*>::iterator iterator;
        for (iterator first = l1.begin(), last = l1.end(); first != last; ++first) {
            if ( ! (*first)->is_cap() && (*first)->check(c)) {
                if ( ! (*first)->out1 && ! (*first)->out2 && is_end) {
                    return true;
                }
                if ((*first)->out1 && add::impl(l2, (*first)->out1, is_end, count)) {
                    return true;
                }
                if ((*first)->out2 && add::impl(l2, (*first)->out2, is_end, count)) {
                    return true;
                }
            }
        }
        return false;
    }

    inline void add_first(std::vector<StateBase*>& l,
                          std::vector<StateBase*>& lfirst, StateBase * st)
    {
        if (st->is_split()) {
            if (st->out1)
                add_first(l, lfirst, st->out1);
            if (st->out2)
                add_first(l, lfirst, st->out2);
        }
        else if (st->is_cap()) {
            if (st->out1) {
                return add_first(l, lfirst, st->out1);
            }
        }
        else if (st->type == FIRST) {
            if (st->out1)
                add_first(lfirst, lfirst, st->out1);
        }
        else {
            l.push_back(st);
        }
    }

    inline bool st_exact_search(StateBase * st, const char * s)
    {
        if ( ! st) {
            return false;
        }
        std::vector<StateBase*> l1;
        add_first(l1, l1, st);
        if (l1.empty()) {
            return false;
        }
        utf_consumer consumer(s);
        std::vector<StateBase*> l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid() && !(res = st_exact_step(l1, l2, consumer.bumpc(), consumer, ++count))) {
            if (l2.empty()) {
                return false;
            }
            l1.swap(l2);
            l2.clear();
        }

        if (consumer.valid()) {
            return false;
        }
        if (res) {
            return true;
        }
        return false;
    }

    inline bool st_step(std::vector<StateBase*>& l1,
                        std::vector<StateBase*>& l2, size_t c, unsigned count)
    {
        struct add {
            static bool impl(std::vector<StateBase*>& l, StateBase * st, unsigned count) {
                if (st->is_finish()) {
                    return true;
                }
                if (st->id == count) {
                    return false;
                }
                st->id = count;
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, count)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, count)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, count);
                    }
                }
                else if (st) {
                    l.push_back(st);
                }
                return false;
            }
        };

        typedef std::vector<StateBase*>::iterator iterator;
        for (iterator first = l1.begin(), last = l1.end(); first != last; ++first) {
            if ( ! (*first)->is_cap() && (*first)->check(c)) {
                if ( ! (*first)->out1 && ! (*first)->out2) {
                    return true;
                }
                if ((*first)->out1) {
                    if (add::impl(l2, (*first)->out1, count)) {
                        return true;
                    }
                }
                if ((*first)->out2) {
                    if (add::impl(l2, (*first)->out2, count)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    inline bool st_search(StateBase * st, const char * s)
    {
        if ( ! st) {
            return false;
        }
        std::vector<StateBase*> lst;
        std::vector<StateBase*> l1;
        add_first(lst, l1, st);
        if (l1.empty() && lst.empty()) {
            return false;
        }
        utf_consumer consumer(s);
        std::vector<StateBase*> l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid()) {
            typedef std::vector<StateBase*>::iterator iterator;
            for (iterator first = lst.begin(), last = lst.end(); first != last; ++first) {
                if ((*first)->id != count) {
                    l1.push_back(*first);
                }
            }
            if (l1.empty()) {
                return false;
            }
            l2.clear();
            if ((res = st_step(l1, l2, consumer.bumpc(), ++count))) {
                break ;
            }
            l1.swap(l2);
        }
        if (res) {
            return true;
        }
        if (consumer.valid()) {
            return false;
        }
        typedef std::vector<StateBase*>::iterator iterator;
        for (iterator first = l1.begin(), last = l1.end(); first != last; ++first) {
            if ((*first)->type == LAST) {
                return true;
            }
        }
        return false;
    }

    inline void st_reset_id(const std::vector<StateBase*> & sts)
    {
        typedef std::vector<StateBase*>::const_iterator iterator;
        for (iterator first = sts.begin(), last = sts.end(); first != last; ++first) {
            (*first)->id = 0;
        }
    }


    class Regex
    {
    public:
        struct Parser {
            const char * err;
            size_t pos_err;
            StateBase * st;

            Parser(StateBase * st = 0)
            : err(0)
            , pos_err(0)
            , st(st)
            {}

            Parser(const char * s)
            : err(0)
            , pos_err(0)
            , st(str2reg(s, &this->err, &this->pos_err))
            {}
        };

    private:
        Parser parser;
        StateMachine2 sm;
        unsigned step_limit;

    public:
        Regex(const char * s, unsigned step_limit = 10000)
        : parser(s)
        , sm(this->parser.st)
        , step_limit(step_limit)
        {}

        Regex(unsigned step_limit = 10000)
        : parser()
        , sm(0)
        , step_limit(step_limit)
        {}

        Regex(StateBase * st, unsigned step_limit = 10000)
        : parser(st)
        , sm(st)
        , step_limit(step_limit)
        {}

        void reset(const char * s)
        {
            this->sm.~StateMachine2();
            free_st(this->parser.st);
            new (&this->parser) Parser(s);
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

        size_t position_error() const
        {
            return this->parser.pos_err;
        }

        typedef StateMachine2::range_matches range_matches;

        range_matches exact_match(const char * s, bool check_end = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit, check_end)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches exact_match(const char * s, Tracer tracer, bool check_end = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit, tracer, check_end)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        range_matches match(const char * s)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches match(const char * s, Tracer tracer)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit, tracer)) {
                this->sm.append_match_result(ret);
            }
            return ret;
        }

        bool exact_search(const char * s, bool check_end = true)
        {
            return this->sm.exact_search(s, this->step_limit, check_end);
        }

        bool search(const char * s)
        {
            return this->sm.search(s, this->step_limit);
        }

        bool exact_search_with_matches(const char * s, bool check_end = true)
        {
            return this->sm.exact_search_with_trace(s, this->step_limit, check_end);
        }

        template<typename Tracer>
        bool exact_search_with_matches(const char * s, Tracer tracer, bool check_end = true)
        {
            return this->sm.exact_search_with_trace(s, tracer, check_end, this->step_limit);
        }

        bool search_with_matches(const char * s)
        {
            return this->sm.search_with_trace(s, this->step_limit);
        }

        template<typename Tracer>
        bool search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.search_with_trace(s, this->step_limit, tracer);
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

#endif
