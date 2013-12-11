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

#ifndef REDEMPTION_REGEX_AUTOMATE_HPP
#define REDEMPTION_REGEX_AUTOMATE_HPP

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cassert>

#include "regex_states_wraper.hpp"

namespace re {

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
        explicit StateMachine2(StatesWrapper & state_wrapper)
        : stw(state_wrapper)
        , nodes(this->stw.size())
        , idx_trace(-1u)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        , captures(0)
        , pcaptures(0)
        , traces(0)
        , st_list(0)
        , st_range_list(0)
        , len_states(this->stw.size(), 0) //unused
        {
            if (this->stw.states.empty()) {
                return ;
            }

            {
                state_list_t::iterator first = this->stw.states.begin();
                state_list_t::iterator last = this->stw.states.end();
                std::vector<unsigned>::iterator lenit = this->len_states.begin();
                for (; first != last; ++first, ++lenit) {
                    if ((*first)->is_sequence()) {
                        *lenit = (*first)->data.sequence.len;
                        this->nodes += *lenit - 1;
                    }
                }
            }

            const size_t count_st = ++this->nodes;
            l1.reserve(count_st);
            l2.reserve(count_st);

            {
                const size_t matrix_size = count_st * count_st;
                this->st_list = new StateList[matrix_size];
                std::memset(this->st_list, 0, matrix_size * sizeof * this->st_list);
            }

            {
                this->st_range_list = new RangeList[count_st];
                this->st_range_list_last = this->st_range_list;
                for (unsigned n = 0; n < count_st; ++n) {
                    RangeList& l = *this->st_range_list_last;
                    ++this->st_range_list_last;
                    l.st = 0;
                    l.first = this->st_list + n * count_st;
                    l.last = l.first;
                }
            }

            if (this->stw.nb_capture) {
                const unsigned col = count_st - this->stw.nb_capture;
                const unsigned matrix_size = col * this->stw.nb_capture;

                this->captures = new State const *[this->stw.nb_capture + matrix_size];
                this->traces = reinterpret_cast<const char **>(this->captures + this->stw.nb_capture);

                this->idx_trace_free = new unsigned[col+1];

                this->pcaptures = std::copy(this->stw.states.end() - this->stw.nb_capture,
                                            this->stw.states.end(),
                                            this->captures);
            }

            {
                unsigned step = 1;
                this->init_list(this->st_range_list, this->stw.root, step);
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

        ~StateMachine2()
        {
            delete [] this->st_list;
            delete [] this->st_range_list;
            delete [] this->captures;
            delete [] this->idx_trace_free;
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            const unsigned size = this->stw.nb_capture;
            char const ** from = this->traces + cp_idx * size;
            char const ** to = this->traces + *this->pidx_trace_free * size;
            for (char const ** last = to + size; to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->nodes - this->stw.nb_capture);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

        void push_state(RangeList* l, const State * st, unsigned step)
        {
            if (st && this->stw.get_num_at(st) != step) {
                this->stw.set_num_at(st, step);
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

        RangeList* find_range_list(const State * st)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            for (RangeList * l = this->st_range_list; l < this->st_range_list_last && l->st; ++l) {
                if (l->st == st) {
                    return l;
                }
            }
            return 0;
        }

        void init_list(RangeList* l, const State * st, unsigned& step)
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
                if (first->st->out1->is_finish()) {
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
                    this->init_list(le, first->st->out1, ++step);
                }
            }
        }

    public:
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

        template<bool V> struct ExactMatch { static const bool value = V; };
        template<bool V> struct ActiveCapture { static const bool value = V; };

    public:
        bool exact_search(const char * s, unsigned step_limit, bool check_end = true)
        {
            if (this->stw.states.empty()) {
                return false;
            }
            if (check_end) {
                return this->match(s, step_limit, DefaultMatchTracer(),
                                   ExactMatch<true>(), ActiveCapture<false>());
            }
            else {
                return this->match(s, step_limit, DefaultMatchTracer(),
                                   ExactMatch<false>(), ActiveCapture<false>());
            }
        }

        bool exact_search_with_trace(const char * s, unsigned step_limit,
                                     bool check_end = true)
        {
            if (this->stw.nb_capture == 0) {
                return exact_search(s, step_limit, check_end);
            }
            return this->match(s, step_limit, DefaultMatchTracer(),
                               ExactMatch<true>(), ActiveCapture<true>());
        }

        template<typename Tracer>
        bool exact_search_with_trace(const char * s, unsigned step_limit,
                                     Tracer tracer, bool check_end = true)
        {
            if (this->stw.nb_capture == 0) {
                return exact_search(s, step_limit, check_end);
            }
            return this->match(s, step_limit, tracer, ExactMatch<true>(), ActiveCapture<true>());
        }

        bool search(const char * s, unsigned step_limit)
        {
            if (this->stw.states.empty()) {
                return false;
            }
            return this->match(s, step_limit, DefaultMatchTracer(), ExactMatch<false>(), ActiveCapture<false>());
        }

        bool search_with_trace(const char * s, unsigned step_limit)
        {
            if (this->stw.nb_capture == 0) {
                return search(s, step_limit);
            }
            return this->match(s, step_limit, DefaultMatchTracer(), ExactMatch<false>(), ActiveCapture<true>());
        }

        template<typename Tracer>
        bool search_with_trace(const char * s, unsigned step_limit, Tracer tracer)
        {
            if (this->stw.nb_capture == 0) {
                return search(s, step_limit);
            }
            return this->match(s, step_limit, tracer, ExactMatch<false>(), ActiveCapture<true>());
        }

        range_matches match_result()
        {
            range_matches ret;
            this->append_match_result(ret);
            return ret;
        }

        void append_match_result(range_matches& ranges) const
        {
            if (this->idx_trace == -1u) {
                return ;
            }

            ranges.reserve(this->stw.nb_capture / 2);

            const char ** trace = this->traces + this->idx_trace * this->stw.nb_capture;

            const State ** pst = this->captures;
            while (pst < this->pcaptures) {
                while ((*pst)->is_cap_close()) {
                    if (++pst >= this->pcaptures) {
                        return ;
                    }
                }
                const State ** pbst = pst;
                unsigned n = 1;
                while (++pst < this->pcaptures && ((*pst)->is_cap_open() ? ++n : --n)) {
                }
                /**///std::cout << (trace[pst - this->captures] - trace[pbst - this->captures]) << std::endl;
                if (trace[pst - this->captures]) {
                    ranges.push_back(range_t(
                        trace[pbst - this->captures],
                        trace[pst - this->captures]
                    ));
                }
                pst = ++pbst;
            }
        }

    private:
        void reset_id()
        {
            for (RangeList * l = this->st_range_list; l < this->st_range_list_last; ++l) {
                this->stw.set_num_at(l->st, 0);
            }
        }

        void reset_trace()
        {
            this->pidx_trace_free = this->idx_trace_free;
            const unsigned size = this->nodes - this->stw.nb_capture;
            //NOTE pidx_trace_free ∈ [0, size+1]
            for (unsigned i = 0; i != size; ++i, ++this->pidx_trace_free) {
                *this->pidx_trace_free = i;
            }
            std::memset(this->traces, 0,
                        size * this->stw.nb_capture * sizeof this->traces[0]);
            this->idx_trace = -1u;
        }

    public:
        void display_elem_state_list(const StateList& e, unsigned idx) const
        {
            std::cout << "\t\033[33m" << idx << "\t" << e.st->num << "\t"
            << *e.st << "\t" << (e.next) << "\033[0m" << std::endl;
        }

        void display_dfa(const RangeList * l, const RangeList * last) const
        {
            for (; l < last && l->first != l->last; ++l) {
                std::cout << l << "  st: " << l->st->num << (l->st->is_cap() ? " (cap)\n" : "\n");
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    std::cout << "\t" << first->st->num << "\t"
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
        struct StepRangeList {
            struct StepRange {
                const RangeList * rl;
                unsigned consume;
                unsigned idx;

                StepRange(const RangeList * l, unsigned count_consume, unsigned id)
                : rl(l)
                , consume(count_consume)
                , idx(id)
                {}

                StepRange(const RangeList * l, unsigned count_consume)
                : rl(l)
                , consume(count_consume)
                {}
            };

            typedef std::vector<StepRange> container_type;
            typedef container_type::iterator iterator;

            void push_back(RangeList* val, unsigned count_consume, unsigned id = 0)
            { this->list.push_back(StepRange(val, count_consume, id)); }

            void push_back(const StepRange & x)
            { this->list.push_back(x); }

            iterator begin()
            { return this->list.begin(); }

            iterator end()
            { return this->list.end(); }

            bool empty() const
            { return this->list.empty(); }

            void clear()
            { this->list.clear(); }

            void reserve(std::size_t count)
            { this->list.reserve(count); }

            container_type list;
        };

        typedef StepRangeList::iterator StepRangeIterator;

        template<typename Tracer, bool exact_match, bool active_capture>
        unsigned step(const char * s, char_int c, utf_consumer & consumer,
                      StepRangeList & l1, StepRangeList & l2, Tracer tracer,
                      ExactMatch<exact_match>, ActiveCapture<active_capture>)
        {
            unsigned new_trace;
            for (StepRangeIterator ifirst = l1.begin(), ilast = l1.end(); ifirst != ilast; ++ifirst) {
                ++this->step_count;
                if (active_capture) {
                    new_trace = 0;
                }

                if (ifirst->consume) {
                    if (!--ifirst->consume && (exact_match == false || !consumer.valid())) {
                        return active_capture ? ifirst->idx : 0;
                    }
#ifdef DISPLAY_TRACE
                    std::cout << "\t\033[35mreinsert (" << ifirst->consume << ") " << (ifirst->idx) << "\033[0m\n";
#endif
                    l2.push_back(*ifirst);
                    if (active_capture) {
                        ++new_trace;
                    }
                    continue ;
                }

                if (this->stw.get_num_at(ifirst->rl->st) == this->step_id) {
#ifdef DISPLAY_TRACE
                    std::cout << "\t\033[35mdup " << (ifirst->idx) << "\033[0m\n";
#endif
                    if (active_capture) {
                        tracer.fail(ifirst->idx);
                        this->push_idx_trace(ifirst->idx);
                    }
                    continue;
                }

                this->stw.set_num_at(ifirst->rl->st, this->step_id);
                StateList * first = ifirst->rl->first;
                StateList * last = ifirst->rl->last;

                for (; first != last; ++first) {
                    ++this->step_count;

                    if (!active_capture && first->st->is_cap()) {
                        if (!exact_match && first->next == 0) {
                            return 0;
                        }
                        continue ;
                    }

                    if (active_capture) {
                        unsigned num = first->st->num - (this->stw.size() - this->stw.nb_capture);
                        if (first->st->is_cap_open()) {
                            if (!this->traces[ifirst->idx * this->stw.nb_capture + num] && tracer.open(ifirst->idx, s, num)) {
#ifdef DISPLAY_TRACE
                                std::cout << ifirst->idx << "  " << *first->st << "  " << num << std::endl;
#endif
                                this->traces[ifirst->idx * this->stw.nb_capture + num] = s;
                            }
                            continue ;
                        }

                        if (first->st->is_cap_close()) {
                            if (tracer.close(ifirst->idx, s, num)) {
#ifdef DISPLAY_TRACE
                                std::cout << ifirst->idx << "  " << *first->st << "  " << num << std::endl;
#endif
                                this->traces[ifirst->idx * this->stw.nb_capture + num] = s;
                            }
                            if (!consumer.valid() && first->st->out1 && first->st->out1->type == LAST) {
                                ++this->traces[ifirst->idx * this->stw.nb_capture + num];
                                //return ifirst->idx;
                            }
                            continue ;
                        }
                    }

                    if ( ! active_capture && first->st->is_finish()) {
                        if ( ! exact_match) {
                            return 0;
                        }
                        continue;
                    }

                    if (unsigned count_consume = first->st->check(c, consumer)) {
#ifdef DISPLAY_TRACE
                        this->display_elem_state_list(*first, ifirst->idx);
#endif

                        if (0 == first->next) {
                            /**///std::cout << "idx: " << (ifirst->idx) << std::endl;
                            if (count_consume > 1) {
                                if (active_capture) {
                                    l2.push_back(first->next, count_consume - 1, ifirst->idx);
                                }
                                else {
                                    l2.push_back(first->next, count_consume - 1);
                                }
                                continue;
                            }
                            if (exact_match == false || !consumer.valid()) {
                                return active_capture ? ifirst->idx : 0;
                            }
                            continue;
                        }

                        if (active_capture) {
                            unsigned idx = ifirst->idx;
                            if (new_trace) {
                                idx = this->pop_idx_trace(ifirst->idx);
                                tracer.new_id(ifirst->idx, idx);
                            }
#ifdef DISPLAY_TRACE
                            std::cout << "\t\033[32m" << ifirst->idx << " -> " << idx << "\033[0m" << std::endl;
#endif
                            l2.push_back(first->next, count_consume - 1, idx);
                            ++new_trace;
                        }
                        else {
                            l2.push_back(first->next, count_consume - 1);
                        }
                    }
                }

                if (active_capture && 0 == new_trace) {
#ifdef DISPLAY_TRACE
                    std::cout << "\t\033[35mx " << ifirst->idx << "\033[0m" << std::endl;
#endif
                    tracer.fail(ifirst->idx);
                    this->push_idx_trace(ifirst->idx);
                }
            }

            return -1u;
        }

        template<typename Tracer, bool exact_match, bool active_capture>
        bool match(const char * s, unsigned step_limit, Tracer tracer,
                   ExactMatch<exact_match>, ActiveCapture<active_capture>)
        {
#ifdef DISPLAY_TRACE
            this->display_dfa();
#endif
            this->step_count = 0;
            this->step_id = 1;

            this->l1.clear();
            this->l2.clear();
            this->reset_id();
            if (active_capture) {
                this->reset_trace();
            }

            StepRangeList * pal1 = &this->l1;
            StepRangeList * pal2 = &this->l2;
            if (active_capture) {
                this->l1.push_back(&this->st_range_beginning, 0, *--this->pidx_trace_free);
                tracer.start(*this->pidx_trace_free);
            }
            else {
                this->l1.push_back(&this->st_range_beginning, 0);
            }

            utf_consumer consumer(s);

            while (consumer.valid()) {
#ifdef DISPLAY_TRACE
                std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m" << std::endl;
#endif
                const unsigned result = this->step(s, consumer.bumpc(), consumer,
                                                   *pal1, *pal2, tracer,
                                                   ExactMatch<exact_match>(),
                                                   ActiveCapture<active_capture>());
                if (-1u != result) {
                    if (active_capture) {
                        this->idx_trace = result;
                        tracer.good(result);
                    }
                    return false == exact_match || !consumer.valid();
                }
                if (exact_match == true && pal2->empty()) {
                    return false;
                }
                if (this->step_count >= step_limit) {
                    return false;
                }
                ++this->step_id;
                std::swap(pal1, pal2);
                pal2->clear();
                if (false == exact_match) {
                    if (active_capture) {
                        --this->pidx_trace_free;
                        assert(this->pidx_trace_free >= this->idx_trace_free);
#ifdef DISPLAY_TRACE
                        std::cout << "\t\033[32m-> " << *this->pidx_trace_free << "\033[0m" << std::endl;
#endif
                        pal1->push_back(this->st_range_list, 0, *this->pidx_trace_free);
                    }
                    else {
                        pal1->push_back(this->st_range_list, 0);
                    }
                }
                if (active_capture) {
                    s = consumer.str();
                }
            }

#ifdef DISPLAY_TRACE
                std::cout << "\033[35mconsumer.valid(): '" << consumer.valid() << "'\033[0m" << std::endl;
#endif

            if (!consumer.valid()) {
                for (StepRangeIterator ifirst = pal1->begin(), ilast = pal1->end(); ifirst != ilast; ++ifirst) {
                    if (ifirst->consume) {
                        if (ifirst->consume != 1) {
                            continue ;
                        }

                        if (ifirst->rl == 0) {
                            return true;
                        }
                    }

                    if (active_capture) {
                        if (this->stw.get_num_at(ifirst->rl->st) == this->step_id) {
                            /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                            tracer.fail(ifirst->idx);
                            //this->push_idx_trace(ifirst->idx);
                            continue;
                        }
                        this->stw.set_num_at(ifirst->rl->st, this->step_id);
                    }

                    StateList * first = ifirst->rl->first;
                    StateList * last = ifirst->rl->last;

                    for (; first != last; ++first) {
                        if (first->st->is_cap_close()) {
                            if (!active_capture) {
                                if (first->next == 0) {
                                    return true;
                                }
                                continue ;
                            }
                            unsigned num = first->st->num - (this->stw.size() - this->stw.nb_capture);
                            if (tracer.close(ifirst->idx, s, num)) {
#ifdef DISPLAY_TRACE
                                std::cout << ifirst->idx << "  " << *first->st << "  " << num << std::endl;
#endif
                                if (0 == first->next) {
                                    this->idx_trace = ifirst->idx;
                                    this->traces[ifirst->idx * this->stw.nb_capture + num] = s;
                                    return true;
                                }
                            }
                            continue ;
                        }

                        if (first->st->is_terminate()) {
                            if (active_capture) {
                                this->idx_trace = ifirst->idx;
                            }
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        StatesWrapper & stw;

        unsigned nodes;
        unsigned idx_trace;
        unsigned * idx_trace_free;
        unsigned * pidx_trace_free;
        const State ** captures;
        const State ** pcaptures;
        const char ** traces;
        StepRangeList l1;
        StepRangeList l2;

        struct StateList
        {
            const State * st;
            RangeList * next;
        };

        StateList * st_list;

        struct RangeList
        {
            const State * st;
            StateList * first;
            StateList * last;
        };

        RangeList * st_range_list;
        RangeList * st_range_list_last;
        RangeList st_range_beginning;

        std::vector<unsigned> len_states;

        unsigned step_id;
        unsigned step_count;
    };

    inline void display_state(StatesWrapper & stw, State * st, unsigned depth = 0)
    {
        if (st && stw.get_num_at(st) != -2u) {
            std::string s(depth, '\t');
            std::cout
            << s << "\033[33m" << st << "\t" << st->num << "\t" << *st
            << "\033[0m\n\t" << s << st->out1 << "\n\t" << s << st->out2 << "\n";
            stw.set_num_at(st, -2u);
            display_state(stw, st->out1, depth+1);
            display_state(stw, st->out2, depth+1);
        }
    }

    inline void display_state(StatesWrapper & stw)
    {
        stw.reset_nums();
        display_state(stw, stw.root);
    }


    typedef std::pair<const State*, unsigned> st_step_elem_t;
    typedef std::vector<st_step_elem_t> st_step_range_list_t;
    typedef st_step_range_list_t::iterator st_step_range_iterator_t;

    inline bool st_exact_step(st_step_range_list_t & l1, st_step_range_list_t & l2,
                              StatesWrapper & stw, size_t c, utf_consumer & consumer, unsigned count)
    {
        struct add {
            static bool impl(st_step_range_list_t & l, const State * st,
                             StatesWrapper & stw, bool is_end, unsigned count, unsigned count_consume)
            {
                if (stw.get_num_at(st) == count) {
                    return false;
                }
                stw.set_num_at(st, count);
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, stw, is_end, count, count_consume)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, stw, is_end, count, count_consume)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, stw, is_end, count, count_consume);
                    }
                }
                else if (st) {
                    if (st->is_terminate()) {
                        if (count_consume > 0) {
                            l.push_back(st_step_elem_t(st, count_consume));
                        }
                        else if (is_end) {
                            return true;
                        }
                    }
                    else {
                        l.push_back(st_step_elem_t(st, count_consume));
                    }
                }
                return false;
            }

            static bool push_next(st_step_range_list_t & l, const State * st,
                                  StatesWrapper & stw, bool is_end, unsigned count)
            {
                if ( ! st->out1 && ! st->out2 && is_end) {
                    return true;
                }
                if (st->out1 && impl(l, st->out1, stw, is_end, count, 0)) {
                    return true;
                }
                if (st->out2 && impl(l, st->out2, stw, is_end, count, 0)) {
                    return true;
                }
                return false;
            }
        };

        const bool is_end = ! consumer.valid();
        unsigned r;
        for (st_step_range_iterator_t first = l1.begin(), last = l1.end(); first != last; ++first) {
#ifdef DISPLAY_TRACE
            std::cout << (*first->first) << std::endl;
#endif
            if (first->second) {
#ifdef DISPLAY_TRACE
                std::cout << "continue " << first->second << std::endl;
#endif
                if (!--first->second) {
                    if (add::push_next(l2, first->first, stw, is_end, count)) {
                        return true;
                    }
                }
                else {
                    l2.push_back(*first);
                }
                continue;
            }
            if ( ! first->first->is_cap() && (r = first->first->check(c, consumer))) {
#ifdef DISPLAY_TRACE
                std::cout << "ok" << std::endl;
#endif
                if (r != 1) {
                    l2.push_back(st_step_elem_t(first->first, r-1));
                }
                else if (add::push_next(l2, first->first, stw, is_end, count)) {
                    return true;
                }
            }
        }
        return false;
    }

    inline void add_first(st_step_range_list_t & l, st_step_range_list_t & lfirst, const State * st)
    {
        if (st->is_split()) {
            if (st->out1) {
                add_first(l, lfirst, st->out1);
            }
            if (st->out2) {
                add_first(l, lfirst, st->out2);
            }
        }
        else if (st->is_cap()) {
            if (st->out1) {
                add_first(l, lfirst, st->out1);
            }
        }
        else if (st->type == FIRST) {
            if (st->out1) {
                add_first(lfirst, lfirst, st->out1);
            }
        }
        else {
            l.push_back(st_step_elem_t(st, 0));
        }
    }

    inline bool st_exact_search(StatesWrapper & stw, const char * s)
    {
        if ( ! stw.root) {
            return false;
        }

// #ifdef DISPLAY_TRACE
//         display_state(stw);
// #endif

        st_step_range_list_t l1;
        add_first(l1, l1, stw.root);
        if (l1.empty()) {
            return false;
        }
        utf_consumer consumer(s);
        st_step_range_list_t l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid() && !(res = st_exact_step(l1, l2, stw, consumer.bumpc(), consumer, ++count))) {
#ifdef DISPLAY_TRACE
            std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m\n";
#endif
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

    inline bool st_step(st_step_range_list_t & l1, st_step_range_list_t & l2,
                        StatesWrapper & stw, char_int c, utf_consumer & consumer, unsigned count)
    {
        struct add {
            static bool impl(st_step_range_list_t & l, const State * st, StatesWrapper & stw,
                             unsigned count, unsigned count_consume) {
                if (st->is_finish()) {
                    return true;
                }
                if (stw.get_num_at(st) == count) {
                    return false;
                }
                stw.set_num_at(st, count);
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, stw, count, count_consume)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, stw, count, count_consume)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, stw, count, count_consume);
                    }
                }
                else if (st) {
                    l.push_back(st_step_elem_t(st, count_consume));
                }
                return false;
            }
        };

        unsigned r;
        for (st_step_range_iterator_t first = l1.begin(), last = l1.end(); first != last; ++first) {
            if (first->second && --first->second) {
                l2.push_back(*first);
                continue;
            }
            if ( ! first->first->is_cap() && (r = first->first->check(c, consumer))) {
#ifdef DISPLAY_TRACE
            std::cout << (*first->first) << std::endl;
#endif
                if ( ! first->first->out1 && ! first->first->out2) {
                    return true;
                }
                if (first->first->out1) {
                    if (add::impl(l2, first->first->out1, stw, count, r)) {
                        return true;
                    }
                }
                if (first->first->out2) {
                    if (add::impl(l2, first->first->out2, stw, count, r)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    inline bool st_search(StatesWrapper & stw, const char * s)
    {
        if ( ! stw.root) {
            return false;
        }

// #ifdef DISPLAY_TRACE
//         display_state(stw);
// #endif

        st_step_range_list_t lst;
        st_step_range_list_t l1;
        add_first(lst, l1, stw.root);
        if (l1.empty() && lst.empty()) {
            return false;
        }
        utf_consumer consumer(s);
        st_step_range_list_t l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid()) {
#ifdef DISPLAY_TRACE
            std::cout << "\033[01;31mc: '" << utf_char(consumer.getc()) << "'\033[0m\n";
#endif
            for (st_step_range_iterator_t first = lst.begin(), last = lst.end(); first != last; ++first) {
                if (stw.get_num_at(first->first) != count) {
                    l1.push_back(*first);
                }
            }
            if (l1.empty()) {
                return false;
            }
            l2.clear();
            if ((res = st_step(l1, l2, stw, consumer.bumpc(), consumer, ++count))) {
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
        for (st_step_range_iterator_t first = l1.begin(), last = l1.end(); first != last; ++first) {
            if (first->first->type == LAST) {
                return true;
            }
        }
        return false;
    }
}

#endif
