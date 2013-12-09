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

#ifndef REDEMPTION_FTESTS_REGEX_REGEX_AUTOMATE_HPP
#define REDEMPTION_FTESTS_REGEX_REGEX_AUTOMATE_HPP

#include <iostream>
#include <new>
#include <vector>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <cstring> //memset
#include <cassert>
#include <stdint.h>

#include "regex_utils.hpp"

namespace re {

    class StateMachine2
    {
        class RangeList;
        class StateList;
        class StepRangeList;

        struct MinimalState
        {
            unsigned type;
            unsigned num;

            union {
                Range range;
                Sequence sequence;
            } data;
        };

        StateMachine2(const StateMachine2&) /*= delete*/;

        void initialize_memory_with_capture(size_t nb_st_list,
                                            size_t nb_st_range_list,
                                            size_t nb_idx_trace_free,
                                            size_t nb_reindex_trace,
                                            size_t nb_traces,
                                            size_t byte_mini_sts, size_t byte_mini_sts_seq)
        {
            nb_st_list *= sizeof(StateList);
            nb_st_list += nb_st_list % sizeof(intmax_t);

            nb_st_range_list *= sizeof(RangeList);
            nb_st_range_list += nb_st_range_list % sizeof(intmax_t);

            size_t nb_step_range_list = this->nodes * sizeof(StepRangeList::StepRange);
            nb_step_range_list += nb_step_range_list % sizeof(intmax_t);

            nb_traces *= sizeof(char*);
            nb_traces += nb_traces % sizeof(intmax_t);

            nb_idx_trace_free *= sizeof(unsigned);
            nb_reindex_trace *= sizeof(unsigned);
            size_t nb_nums = this->nb_states * sizeof(unsigned);
            nb_nums += (nb_nums + nb_idx_trace_free + nb_reindex_trace) % sizeof(unsigned);

            size_t nb_cap_type = this->nb_capture * sizeof(bool);
            nb_cap_type += nb_cap_type % sizeof(intmax_t);

            byte_mini_sts += byte_mini_sts % sizeof(intmax_t);

            char * mem = static_cast<char*>(::operator new(
                nb_st_list + nb_st_range_list + nb_step_range_list * 2
                + nb_traces + nb_idx_trace_free + nb_reindex_trace + nb_nums
                + nb_cap_type + byte_mini_sts + byte_mini_sts_seq
            ));

            this->st_list = reinterpret_cast<StateList*>(mem);
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem);
            mem += nb_st_range_list;
            this->l1.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem));
            mem += nb_step_range_list;
            this->l2.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem));
            mem += nb_step_range_list;
            this->traces = reinterpret_cast<const char**>(mem);
            mem += nb_traces;
            this->idx_trace_free = reinterpret_cast<unsigned*>(mem);
            mem += nb_idx_trace_free;
            this->reindex_trace = reinterpret_cast<unsigned*>(mem);
            mem += nb_reindex_trace;
            this->nums = reinterpret_cast<unsigned*>(mem);
            mem += nb_nums;
            this->caps_type = reinterpret_cast<bool*>(mem);
            mem += nb_cap_type;
            this->mini_sts = reinterpret_cast<MinimalState*>(mem);

#ifndef NDEBUG
            this->l1.nodes = this->nodes;
            this->l2.nodes = this->nodes;
#endif
        }

        void initialize_memory_without_capture(size_t nb_st_list, size_t nb_st_range_list,
                                               size_t byte_mini_sts, size_t byte_mini_sts_seq)
        {
            nb_st_list *= sizeof(StateList);
            nb_st_list += nb_st_list % sizeof(intmax_t);

            nb_st_range_list *= sizeof(RangeList);
            nb_st_range_list += nb_st_range_list % sizeof(intmax_t);

            size_t nb_step_range_list = this->nodes * sizeof(StepRangeList::StepRange);
            nb_step_range_list += nb_step_range_list % sizeof(intmax_t);

            size_t nb_nums = this->nb_states * sizeof(unsigned);
            nb_nums += nb_nums % sizeof(intmax_t);

            byte_mini_sts += byte_mini_sts % sizeof(intmax_t);

            char * mem = static_cast<char*>(::operator new(
                nb_st_list + nb_st_range_list + nb_step_range_list * 2
                + nb_nums + byte_mini_sts + byte_mini_sts_seq
            ));

            this->st_list = reinterpret_cast<StateList*>(mem);
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem);
            mem += nb_st_range_list;
            this->l1.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem));
            mem += nb_step_range_list;
            this->l2.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem));
            mem += nb_step_range_list;
            this->nums = reinterpret_cast<unsigned*>(mem);
            mem += nb_nums;
            this->mini_sts = reinterpret_cast<MinimalState*>(mem);

#ifndef NDEBUG
            this->l1.nodes = this->nodes;
            this->l2.nodes = this->nodes;
#endif
        }

        void initialize_minimal_memory(size_t nb_st_list, unsigned nb_st)
        {
            nb_st_list *= sizeof(StateList);
            nb_st_list += nb_st_list % sizeof(intmax_t);

            size_t nb_st_range_list = nb_st * sizeof(RangeList);
            nb_st_range_list += nb_st_range_list % sizeof(intmax_t);

            size_t nb_nums = this->nb_states * sizeof(unsigned);

            char * mem = static_cast<char*>(::operator new(
                nb_st_list + nb_st_range_list + nb_nums
            ));

            this->st_list = reinterpret_cast<StateList*>(mem);
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem);
            mem += nb_st_range_list;
            this->nums = reinterpret_cast<unsigned*>(mem);
        }

        void initialize_memory(size_t nb_st_list, size_t nb_st_range_list,
                               bool cpy_sts, unsigned nb_char_seq)
        {
            const size_t size_mini_sts = cpy_sts
            ? (this->nb_states - this->nb_capture) * sizeof(MinimalState)
            : 0;
            const size_t size_mini_sts_seq = cpy_sts ? nb_char_seq * sizeof(char_int) : 0;
            if (this->nb_capture) {
                this->initialize_memory_with_capture(
                    nb_st_list, //st_list
                    nb_st_range_list, //st_range_list
                    this->nodes, //idx_trace_free
                    this->nb_capture, //reindex_trace
                    this->nodes * this->nb_capture, //traces
                    size_mini_sts,
                    size_mini_sts_seq
                );
            }
            else {
                this->initialize_memory_without_capture(
                    nb_st_list, //st_list
                    nb_st_range_list, //st_range_list
                    size_mini_sts,
                    size_mini_sts_seq
                );
            }
        }

    public:
        explicit StateMachine2(const state_list_t & sts, const State * root, unsigned nb_capture,
                               bool copy_states = false, bool minimal_mem = false)
        : root(root)
        , nb_states(sts.size())
        , nb_capture(nb_capture)
        , nodes(sts.size() - nb_capture)
        , idx_trace(-1u)
        , reindex_trace(0)
        , idx_trace_free(0)
        , pidx_trace_free(0)
        , traces(0)
        , mini_sts_last(0)
        , st_list(0)
        , st_range_list(0)
        , step_id(1)
        , first_last(false)
        {
            if (sts.empty()) {
                this->st_range_beginning.st_num = -1u;
                return ;
            }

            unsigned nb_char_seq = 0;
            unsigned nb_state_consume = 0;
            {
                state_list_t::const_iterator first = sts.begin();
                state_list_t::const_iterator last = sts.end() - this->nb_capture;
                for (; first != last; ++first) {
                    if ((*first)->is_sequence()) {
                        this->nodes += (*first)->data.sequence.len - 1;
                        nb_char_seq += (*first)->data.sequence.len + 1;
                    }
                    if ((*first)->type & (RANGE|SEQUENCE)) {
                        ++nb_state_consume;
                    }
                }
            }

            this->nodes = this->nodes - (sts.size() - nb_capture) + nb_state_consume + 1;

            size_t nb_st = nb_state_consume + 1;
            const size_t col_size = nb_state_consume + 1;
            const size_t line_size = nb_state_consume;
            const size_t matrix_size = col_size * line_size;

            if (minimal_mem) {
                this->initialize_minimal_memory(matrix_size + nb_st, nb_st);
            }
            else {
                this->initialize_memory(matrix_size + nb_st, nb_st, copy_states, nb_char_seq);
            }

            std::memset(this->st_list, 0, (matrix_size + nb_st) * sizeof * this->st_list);
            std::memset(this->nums, 0, this->nb_states * sizeof * this->nums);

            this->st_range_list_last = this->st_range_list;
            for (unsigned n = 0; n < col_size; ++n) {
                RangeList& l = *this->st_range_list_last;
                ++this->st_range_list_last;
                l.st_num = -1u;
                l.first = this->st_list + n * line_size;
                l.last = l.first;
            }

            this->st_range_beginning.first = this->st_list + matrix_size + nb_st - 1;
            this->st_range_beginning.last = this->st_range_beginning.first;
            this->init_range_list(this->st_range_list, root);
            this->init_value_state_list(this->st_list, this->st_list + matrix_size);

            const bool special_state = this->st_range_beginning.first != this->st_range_beginning.last;

            if (special_state)
            {
                StateList * first = this->st_range_beginning.last+1;
                StateList * last = this->st_range_beginning.first+1;
                StateList * firstdest = this->st_list + matrix_size;
                for (; first != last; ++first) {
                    typedef StateList * tab2_st_t[2];
                    tab2_st_t& tab = reinterpret_cast<tab2_st_t&>(*first);
                    //overlap
                    StateList * cpfirst = tab[0];
                    StateList * cplast = tab[1];
                    for (; cpfirst != cplast; ++cpfirst, ++firstdest) {
                        *firstdest = *cpfirst;
                        cpfirst->st = 0;
                    }
                }
                this->st_range_beginning.first = this->st_list + matrix_size;
                this->st_range_beginning.last = firstdest;
            }
            else {
                this->st_range_beginning.first = this->st_list + matrix_size;
                this->st_range_beginning.last = this->st_range_beginning.first;
            }

            // purge empty StateList (StateList::st = 0) and set RangeList::st_num to -1u if empty range
            {
                RangeList * l = this->st_range_list;
                for (; l < this->st_range_list_last; ++l) {
                    if (l->st_num == -1u) {
                        continue;
                    }
                    StateList * first = l->first;
                    while (first != l->last && first->st) {
                      ++first;
                    }
                    StateList * result = first;
                    for (; first != l->last; ++first) {
                      if (first->st) {
                        *result = *first;
                        ++result;
                      }
                    }
                    l->last = result;
                    if (l->first == l->last) {
                      l->st_num = -1u;
                    }
                }
            }

            while (this->st_range_list != this->st_range_list_last && -1u == this->st_range_list->st_num) {
                ++this->st_range_list;
            }

            while (this->st_range_list != this->st_range_list_last && -1u == (this->st_range_list_last-1)->st_num) {
                --this->st_range_list_last;
            }

            {
                RangeList * l = this->st_range_list;
                for (; l < this->st_range_list_last; ++l) {
                    if (l->st_num == -1u) {
                        continue;
                    }
                    for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                        if (first->next && (first->next->first == first->next->last || first->next->st_num == -1u)) {
                            first->next = 0;
                        }
                    }
                }
                StateList * first = this->st_range_beginning.first;
                StateList * last = this->st_range_beginning.last;
                for (; first != last; ++first) {
                    if (first->next && (first->next->first == first->next->last || first->next->st_num == -1u)) {
                        first->next = 0;
                    }
                }
            }

            if (this->st_range_list == this->st_range_list_last && !root->out1 && !root->out2) {
                if (minimal_mem) {
                    ::operator delete(this->st_list);
                    if (copy_states) {
                        void * const mem = ::operator new(sizeof(MinimalState) + this->nb_states * sizeof * this->nums);
                        this->st_list = static_cast<StateList*>(mem);
                        this->nums = reinterpret_cast<unsigned*>(static_cast<MinimalState*>(mem) + 1);
                        this->root = static_cast<State*>(mem);
                        this->mini_sts = static_cast<MinimalState*>(mem);
                        this->mini_sts->type = 0;
                        this->mini_sts->num = 0;
                        this->mini_sts_last = this->mini_sts + 1;
                    }
                    else {
                        void * const mem = ::operator new(this->nb_states * sizeof * this->nums);
                        this->st_list = static_cast<StateList*>(mem);
                        this->nums = static_cast<unsigned*>(mem);
                    }
                }
                else if (copy_states) {
                    ::operator delete(this->st_list);
                    void * const mem = ::operator new(sizeof(MinimalState));
                    this->st_list = static_cast<StateList*>(mem);
                    this->root = static_cast<State*>(mem);
                    this->mini_sts = static_cast<MinimalState*>(mem);
                    this->mini_sts->type = 0;
                    this->mini_sts->num = 0;
                    this->mini_sts_last = this->mini_sts + 1;
                }
                this->st_range_beginning.st_num = -1u;
                return ;
            }

            if (minimal_mem) {
                nb_st = 0;
                {
                    RangeList * first = this->st_range_list;
                    for (; first != this->st_range_list_last; ++first) {
                        nb_st += first->last - first->first;
                    }
                }
                const size_t nb_range = this->st_range_list_last - this->st_range_list;
                const size_t nb_beginning_st = this->st_range_beginning.last - this->st_range_beginning.first;
                StateList * tmp_st_list = this->st_list;
                RangeList * tmp_range_list = this->st_range_list;
                StateList * tmp_st_first_list = this->st_range_beginning.first;

                this->initialize_memory(nb_st + nb_beginning_st,
                                        nb_range, copy_states, nb_char_seq);
                std::memset(this->st_list, 0, (nb_st + nb_beginning_st) * sizeof * this->st_list);
                this->st_range_list_last = this->st_range_list + (this->st_range_list_last - tmp_range_list);

                RangeList * rlfirst = this->st_range_list;
                RangeList * cprlfirst = tmp_range_list;
                StateList * slfirst = this->st_list;
                for (; rlfirst != this->st_range_list_last; ++rlfirst, ++cprlfirst) {
                    rlfirst->st_num = cprlfirst->st_num;
                    rlfirst->first = slfirst;
                    StateList * cpslfirst = cprlfirst->first;
                    StateList * cpsllast = cprlfirst->last;
                    for (; cpslfirst != cpsllast; ++cpslfirst, ++slfirst) {
                        *slfirst = *cpslfirst;
                        if (cpslfirst->next) {
                            slfirst->next = this->st_range_list + (cpslfirst->next - tmp_range_list);
                        }
                    }
                    rlfirst->last = slfirst;
                }

                StateList * sllast = tmp_st_first_list + nb_beginning_st;
                this->st_range_beginning.first = slfirst;
                for (; tmp_st_first_list != sllast; ++slfirst, ++tmp_st_first_list) {
                    *slfirst = *tmp_st_first_list;
                    if (tmp_st_first_list->next) {
                        slfirst->next = this->st_range_list + (tmp_st_first_list->next - tmp_range_list);
                    }
                }
                this->st_range_beginning.last = slfirst;

                ::operator delete(tmp_st_list);
            }

            if (this->nb_capture) {
                unsigned * reindex = this->reindex_trace;
                bool * typefirst = this->caps_type;
                typedef state_list_t::const_iterator state_iterator;
                state_iterator stfirst = sts.end() - this->nb_capture;
                state_iterator stlast = sts.end();
                for (unsigned num_open; stfirst != stlast; ++stfirst, ++typefirst) {
                    *typefirst = (*stfirst)->type == CAPTURE_OPEN;
                    if ((*stfirst)->is_cap_close()) {
                        continue;
                    }
                    *reindex++ = (*stfirst)->num;
                    state_iterator stfirst2 = stfirst;
                    num_open = 1;
                    do {
                        ++stfirst2;
                        if ((*stfirst2)->is_cap_close()) {
                            --num_open;
                        }
                        if ((*stfirst2)->is_cap_open()) {
                            ++num_open;
                        }
                    } while (stfirst2 != stlast && num_open);
                    *reindex++ = (*stfirst2)->num;
                }
            }

            if (this->st_range_beginning.first == this->st_range_beginning.last && this->st_range_list != this->st_range_list_last) {
                this->st_range_beginning = *this->st_range_list;
            }

            if (special_state) {
                this->st_range_beginning.st_num = this->st_range_list != this->st_range_list
                ? this->st_range_list->st_num
                : this->st_range_beginning.first != this->st_range_beginning.first
                ? this->st_range_beginning.first->st->num
                : 0;
            }
            else {
                this->st_range_beginning.st_num = this->st_range_list != this->st_range_list
                ? this->st_range_list->st_num : 0;
            }

            StateList * first = this->st_range_beginning.first;
            StateList * last = this->st_range_beginning.last;
            for (; first != last; ++first) {
                if (first->st && ((first->st->out1 && first->st->out1->is_finish()) || first->st->is_terminate())) {
                    this->st_range_list = this->st_range_list_last;
                    this->st_range_beginning.st_num = -1u;
                    break;
                }
            }

            if (copy_states && -1u == this->st_range_beginning.st_num) {
                this->root = reinterpret_cast<State*>(this->mini_sts);
                this->mini_sts->type = 0;
                this->mini_sts->num = 0;
                this->mini_sts_last = this->mini_sts + 1;
            }
            else if (copy_states) {
                {
                    const size_t size_mini_sts = (this->nb_states - this->nb_capture) * sizeof(MinimalState);
                    char_int * str = reinterpret_cast<char_int*>(
                        reinterpret_cast<char*>(this->mini_sts) + size_mini_sts
                        + size_mini_sts % sizeof(intmax_t)
                    );
                    state_list_t::const_iterator first = sts.begin();
                    state_list_t::const_iterator last = sts.end() - this->nb_capture;
                    MinimalState * first2 = this->mini_sts;
                    for (; first != last; ++first) {
                        if (*first == this->root) {
                            this->root = reinterpret_cast<State*>(first2);
                        }
                        State & st = **first;
                        first2->type = st.type;
                        first2->num = st.num;
                        if (st.is_sequence()) {
                            std::copy(st.data.sequence.s, st.data.sequence.s + st.data.sequence.len + 1, str);
                            first2->data.sequence.s = str;
                            str += st.data.sequence.len + 1;
                            first2->data.sequence.len = st.data.sequence.len;
                        }
                        else {
                            first2->data.range.l = st.data.range.l;
                            first2->data.range.r = st.data.range.r;
                        }
                        ++first2;
                    }
                    this->mini_sts_last = first2;
                }

                RangeList * first = this->st_range_list;
                RangeList * last = this->st_range_list_last;
                for (; first != last; ++first) {
                    StateList * l = first->first;
                    StateList * rlast = first->last;
                    for (; l != rlast; ++l) {
                        MinimalState * msts = this->mini_sts;
                        while (msts->num != l->st->num) {
                            ++msts;
                        }
                        l->st = reinterpret_cast<State*>(msts);
                    }
                }

                StateList * l = this->st_range_beginning.first;
                StateList * rlast = this->st_range_beginning.last;
                for (; l != rlast; ++l) {
                    MinimalState * msts = this->mini_sts;
                    while (msts->num != l->st->num) {
                        ++msts;
                    }
                    l->st = reinterpret_cast<State*>(msts);
                }
            }
            this->first_last = true;

            if (0 == nb_state_consume || -1u == this->st_range_beginning.st_num) {
                if (this->st_range_beginning.first != this->st_range_beginning.last && !this->st_range_beginning.first->st && !this->nb_capture) {
                    this->st_range_beginning.st_num = -1u;
                }
                this->st_range_beginning.last = this->st_range_beginning.first;
                this->st_range_list_last = this->st_range_list;
            }
        }

#if __cplusplus >= 201103L && __cplusplus != 1 || defined(__GXX_EXPERIMENTAL_CXX0X__)
        StateMachine2(StateMachine2 && other) noexcept
        {
            this->root = other.root;
            this->nums = other.nums;
            this->caps_type = other.caps_type;
            this->nb_states = other.nb_states;
            this->nb_capture = other.nb_capture;
            this->nodes = other.nodes;
            this->idx_trace = other.idx_trace;
            this->reindex_trace = other.reindex_trace;
            this->idx_trace_free = other.idx_trace_free;
            this->pidx_trace_free = other.pidx_trace_free;
            this->traces = other.traces;
            this->l1 = other.l1;
            this->l2 = other.l2;
            this->mini_sts = other.mini_sts;
            this->mini_sts_last = other.mini_sts_last;
            this->st_list = other.st_list;
            this->st_range_list = other.st_range_list;
            this->st_range_list_last = other.st_range_list_last;
            this->st_range_beginning = other.st_range_beginning;
            this->step_id = other.step_id;
            this->first_last = other.first_last;

            other.st_list = nullptr;
            other.nb_states = 0;
            other.nb_capture = 0;
            other.st_range_beginning.st_num = -1u;
        }
#endif

        ~StateMachine2()
        {
            ::operator delete(this->st_list);
        }

        unsigned mark_count() const
        {
            return this->nb_capture;
        }

    private:
        unsigned pop_idx_trace(unsigned cp_idx)
        {
            --this->pidx_trace_free;
            assert(this->pidx_trace_free >= this->idx_trace_free);
            const unsigned size = this->nb_capture;
            char const ** from = this->traces + cp_idx * size;
            char const ** to = this->traces + *this->pidx_trace_free * size;
            for (char const ** last = to + size; to < last; ++to, ++from) {
                *to = *from;
            }
            return *this->pidx_trace_free;
        }

        void push_idx_trace(unsigned n)
        {
            assert(this->pidx_trace_free <= this->idx_trace_free + this->nodes);
            *this->pidx_trace_free = n;
            ++this->pidx_trace_free;
        }

        void set_num_at(const State * st, unsigned count) const
        {
            assert(this->nb_states > st->num);
            this->nums[st->num] = count;
        }

        unsigned get_num_at(const State * st) const
        {
            assert(this->nb_states > st->num);
            return this->nums[st->num];
        }

        void init_value_state_list(StateList * l, StateList * last)
        {
            StateList * tmp = l;
            for (; l != last; ++l) {
                if (l->st) {
                    ++this->step_id;
                    l->num_close = this->get_num_close(l->st->out1);
                }
            }
            for (l = tmp; l != last; ++l) {
                if (l->st) {
                    ++this->step_id;
                    l->next_is_finish = l->st->is_finish() || l->next == 0 || this->next_is_finish(l->st->out1);
                }
            }
            for (l = tmp; l != last; ++l) {
                if (l->st) {
                    ++this->step_id;
                    l->is_terminate = l->st->is_terminate() || this->next_is_terminate(l->st->out1);
                }
            }
        }

        bool next_is_finish(const State * st)
        {
            if (!st || st->is_finish()) {
                return true;
            }
            if (this->is_valid_and_mark(st)) {
                if (st->is_split()) {
                    const bool ret = this->next_is_finish(st->out1);
                    return ( ! ret) ?this->next_is_finish(st->out2) : ret;
                }
                else if (st->is_cap()) {
                    return this->next_is_finish(st->out1);
                }
            }
            return false;
        }

        bool next_is_terminate(const State * st)
        {
            if (!st) {
                return false;
            }
            if (st->type == LAST) {
                return true;
            }
            if (this->is_valid_and_mark(st)) {
                if (st->is_split()) {
                    const bool ret = this->next_is_terminate(st->out1);
                    return ( ! ret) ?this->next_is_terminate(st->out2) : ret;
                }
                else if (st->is_cap()) {
                    return this->next_is_terminate(st->out1);
                }
            }
            return false;
        }

        unsigned get_num_close(const State * st)
        {
            if (st && st->is_cap_close()) {
                return st->num;
            }
            if (this->is_valid_and_mark(st)) {
                if (st->is_split()) {
                    const unsigned ret = this->get_num_close(st->out1);
                    return (ret == -1u) ?this->get_num_close(st->out2) : ret;
                }
            }
            return -1u;
        }

        bool is_valid_and_mark(const State * st)
        {
            if (st && this->get_num_at(st) != this->step_id) {
                this->set_num_at(st, step_id);
                return true;
            }
            return false;
        }

        void push_state(RangeList* l, const State * st, unsigned num_open = -1u, unsigned is_first = 0)
        {
            if (this->is_valid_and_mark(st)) {
                if (st->is_split()) {
                    this->push_state(l, st->out1, num_open, is_first);
                    this->push_state(l, st->out2, num_open, is_first);
                }
                else if (st->is_cap_open()) {
                    this->push_state(l, st->out1, num_open == -1u ? st->num : num_open, is_first);
                }
                else if (st->is_cap_close()) {
                    this->push_state(l, st->out1, -1u, is_first);
                }
                else if (st->type == FIRST && is_first <= 1) {
                    if (is_first) {
                        push_state(l, st->out1, num_open, 1);
                    }
                    else {
                        StateList * tmpstl = l->last;
                        push_state(l, st->out1, num_open, 1);
                        typedef StateList * tab2_st_t[2];
                        tab2_st_t& tab = reinterpret_cast<tab2_st_t&>(*this->st_range_beginning.last);
                        tab[0] = tmpstl;
                        tab[1] = l->last;
                        --this->st_range_beginning.last;
                    }
                }
                else if (!st->is_terminate()) {
                    l->last->st = st;
                    l->last->num_open = num_open;
                    l->last->num_close = -1u;
                    ++l->last;
                }
            }
        }

        RangeList* find_range_list(const State * st)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            RangeList * l = this->st_range_list;
            for (; l < this->st_range_list_last && l->st_num != -1u; ++l) {
                if (l->st_num == st->num) {
                    return l;
                }
            }
            return 0;
        }

        void init_range_list(RangeList* l, const State * st, unsigned is_first = true)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            l->st_num = st->num;
            this->push_state(l, st, -1u, is_first ? 0 : 2);
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
                    while (le < this->st_range_list_last && le->st_num != -1u) {
                        ++le;
                    }
                    first->next = le;
                    ++this->step_id;
                    this->init_range_list(le, first->st->out1, false);
                }
            }
        }

    public:
        typedef std::pair<const char *, const char *> range_t;
        typedef std::vector<range_t> range_matches;

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

    private:
        template<bool> struct ExactMatch { };
        template<bool> struct ActiveCapture { };

    public:
        bool exact_search(const char * s, unsigned step_limit)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match(s, step_limit, DefaultMatchTracer(),
                               ExactMatch<true>(), ActiveCapture<false>());
        }

        bool exact_search_with_trace(const char * s, unsigned step_limit)
        {
            if (this->nb_capture == 0) {
                return exact_search(s, step_limit);
            }
            return this->match(s, step_limit, DefaultMatchTracer(),
                               ExactMatch<true>(), ActiveCapture<true>());
        }

        template<typename Tracer>
        bool exact_search_with_trace(const char * s, unsigned step_limit, Tracer tracer)
        {
            if (this->nb_capture == 0) {
                return exact_search(s, step_limit);
            }
            return this->match(s, step_limit, tracer,
                               ExactMatch<true>(), ActiveCapture<true>());
        }

        bool search(const char * s, unsigned step_limit)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match(s, step_limit, DefaultMatchTracer(),
                               ExactMatch<false>(), ActiveCapture<false>());
        }

        bool search_with_trace(const char * s, unsigned step_limit)
        {
            if (this->nb_capture == 0) {
                return search(s, step_limit);
            }
            return this->match(s, step_limit, DefaultMatchTracer(),
                               ExactMatch<false>(), ActiveCapture<true>());
        }

        template<typename Tracer>
        bool search_with_trace(const char * s, unsigned step_limit, Tracer tracer)
        {
            if (this->nb_capture == 0) {
                return search(s, step_limit);
            }
            return this->match(s, step_limit, tracer,
                               ExactMatch<false>(), ActiveCapture<true>());
        }

        range_matches match_result(bool all = true) const
        {
            range_matches ret;
            this->append_match_result(ret, all);
            return ret;
        }

        void append_match_result(range_matches& ranges, bool all = true) const
        {
            if (this->idx_trace == -1u) {
                return ;
            }

            ranges.reserve(this->nb_capture / 2);

            const char ** trace = this->traces + this->idx_trace * this->nb_capture;
            unsigned * first = this->reindex_trace;
            unsigned * last = first + this->nb_capture;
            for (; first != last; first+=2) {
                const char * sright = *(trace + *(first + 1));
                if (sright) {
                    ranges.push_back(range_t(*(trace + *first), sright));
                }
                else if (all) {
                    ranges.push_back(range_t(0,0));
                }
            }
        }

    private:
        void set_idx_trace(unsigned idx) {
            this->idx_trace = idx;
            //recursive matching
            const char ** first = this->traces + idx * this->nb_capture;
            const char ** last = first + this->nb_capture;
            bool * typefirst = this->caps_type;
            for (; ++first != last; ++typefirst) {
                if (*typefirst == *(typefirst+1)) {
                    *first = *(first-1);
                }
            }
        }

        void reset_id() const
        {
            std::memset(this->nums, 0, this->nb_states * sizeof(*this->nums));
        }

        void reset_trace()
        {
            this->pidx_trace_free = this->idx_trace_free;
            const unsigned size = this->nodes;
            for (unsigned i = 0; i != size; ++i, ++this->pidx_trace_free) {
                *this->pidx_trace_free = i;
            }
            std::memset(this->traces, 0, size * this->nb_capture * sizeof this->traces[0]);
            this->idx_trace = -1u;
        }

    public:
        void display_elem_state_list(const StateList& e, unsigned idx) const
        {
            std::cout << "\t\033[33m" << idx << "\t" << e.st->num << "\t"
            << *e.st << "\t" << e.next << "\033[0m" << std::endl;
        }

        void display_dfa(const RangeList * l, const RangeList * last) const
        {
            for (; l < last; ++l) {
                if (l->st_num == -1u) {
                    continue;
                }
                std::cout << l << "  st: " << l->st_num
                << (l->st_num >= this->nb_states-this->nb_capture ? " (cap)\n" : "\n");
                for (StateList * first = l->first, * last = l->last; first != last; ++first) {
                    std::cout << "\t" << first->st->num << "\t"
                    << *first->st
                    << ((first->st->is_range()
                      && first->st->data.range.l == 0
                      && first->st->data.range.r == char_int(-1)
                    ) ||(first->st->is_sequence()
                      && first->st->data.sequence.len < 8
                    ) ? "\t\t\t"
                      :  (first->st->is_range()
                       && first->st->data.range.l != first->st->data.range.r
                      )||(first->st->is_sequence()
                       && first->st->data.sequence.len > 7
                      ) ? "\t"
                        : "\t\t"
                    ) << first->next << (first->next ? "\t" : "\t\t")
                    << first->next_is_finish << "\t" << first->is_terminate;
                    if (first->num_open != -1u) {
                        std::cout << "\t( " << first->num_open;
                    }
                    if (first->num_close != -1u) {
                        std::cout << "\t) " << first->num_close;
                    }
                    std::cout << "\n";
                }
            }
        }

        void display_dfa() const
        {
            std::cout << ("\tnum\tst\t\t\tnext\tnext_is_finish\tis_terminate") << std::endl;
            if (this->st_range_list == this->st_range_list_last || this->st_range_list->first != this->st_range_beginning.first) {
                std::cout << ("beginning") << std::endl;
                if (this->st_range_beginning.st_num != -1u && this->st_range_beginning.first->st) {
                    this->display_dfa(&this->st_range_beginning, &this->st_range_beginning+1);
                }
                std::cout << ("\nrange") << std::endl;
            }

            this->display_dfa(this->st_range_list, this->st_range_list_last);
            std::cout << std::endl;
        }

        void display_states() const
        {
            if (0 == this->nb_states) {
                return ;
            }

            if (this->mini_sts_last) {
                std::cout << "\033[33m(optimize out)\nnum\ttype\n";
                MinimalState * first = this->mini_sts;
                for (; first != this->mini_sts_last; ++first) {
                    std::cout << first->num << "\t" << first->type << "\n";
                }
                std::cout << "\033[0m";
                std::cout.flush();
                return ;
            }

            std::fill(this->nums, this->nums + this->nb_states, 0);
            struct Impl {
                static void display(const StateMachine2 & sm, const State * st, unsigned depth = 0) {
                    if (st && sm.get_num_at(st) != -2u) {
                        std::cout
                        << std::setw(depth) << "" << "\033[33m" << st << "\t" << st->num << "\t" << *st
                        << "\033[0m\n\t" << std::setw(depth) << "" << st->out1 << "\n\t"
                        << std::setw(depth) << "" << st->out2 << "\n";
                        sm.set_num_at(st, -2u);
                        display(sm, st->out1, depth+1);
                        display(sm, st->out2, depth+1);
                    }
                }
            };
            char oldfill = std::cout.fill('\t');
            Impl::display(*this, this->root);
            std::cout.fill(oldfill);
            std::cout.flush();
        }

    private:
        struct StepRangeList {
            struct StepRange {
                const RangeList * rl;
                unsigned consume;
                unsigned idx;
                unsigned num_close;

                StepRange(const RangeList * l, unsigned count_consume, unsigned id, unsigned numclose)
                : rl(l)
                , consume(count_consume)
                , idx(id)
                , num_close(numclose)
                {}

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

            typedef StepRange * iterator;

            void push_back(RangeList* val, unsigned count_consume)
            {
                assert((this->last - this->list) != this->nodes);
                new(this->last++) StepRange(val, count_consume);
            }

            void push_back(RangeList* val, unsigned count_consume, unsigned id)
            {
                assert((this->last - this->list) != this->nodes);
                new(this->last++) StepRange(val, count_consume, id);
            }

            void push_back(RangeList* val, unsigned count_consume, unsigned id, unsigned numclose)
            {
                assert((this->last - this->list) != this->nodes);
                new(this->last++) StepRange(val, count_consume, id, numclose);
            }

            void push_back(const StepRange & x)
            {
                assert((this->last - this->list) != this->nodes);
                new(this->last++) StepRange(x);
            }

            iterator begin()
            { return this->list; }

            iterator end()
            { return this->last; }

            bool empty() const
            { return this->list == this->last; }

            void clear()
            { this->last = this->list; }

            void set_array(StepRange * array)
            {
                this->list = array;
                this->last = array;
            }

        private:
            StepRange * list;
            StepRange * last;
#ifndef NDEBUG
        public:
            unsigned nodes;
#endif
        };

        typedef StepRangeList::iterator StepRangeIterator;

        template<typename Tracer, bool exact_match, bool active_capture>
        unsigned step(const char * s, char_int c, utf8_consumer & consumer,
                      StepRangeList & l1, StepRangeList & l2, Tracer tracer,
                      ExactMatch<exact_match>, ActiveCapture<active_capture>)
        {
            unsigned new_trace;
            unsigned count_consume;
            for (StepRangeIterator ifirst = l1.begin(), ilast = l1.end(); ifirst != ilast; ++ifirst) {
                ++this->step_count;
                if (active_capture) {
                    new_trace = 0;
                }

                if (ifirst->consume) {
                    if (!--ifirst->consume) {
                        const StateList * stl = reinterpret_cast<const StateList*>(ifirst->rl);
                        if (!stl->next && consumer.valid() && stl->is_terminate) {
#ifdef DISPLAY_TRACE
                            std::cout << "\t\033[35mx " << (ifirst->rl->st_num) << "\033[0m\n";
#endif
                            continue;
                        }

                        if (active_capture) {
                            if (ifirst->num_close != -1u) {
                                unsigned num = ifirst->num_close;
                                if (tracer.close(ifirst->idx, s, num)) {
#ifdef DISPLAY_TRACE
                                    std::cout << "\t" << ifirst->idx << " (close) " << num << std::endl;
#endif
                                    this->traces[ifirst->idx * this->nb_capture + num] = consumer.str();
                                }
                            }
                        }

                        if (stl->next_is_finish || (stl->is_terminate && !consumer.valid())) {
                            //l2.pop_back();
                            return active_capture ? ifirst->idx : 0;
                        }

                        if (stl->is_terminate && consumer.valid()) {
                            continue ;
                        }

                        ifirst->rl = stl->next;
                    }
#ifdef DISPLAY_TRACE
                    std::cout << "\t\033[35mreinsert (" << ifirst->consume << ")";
                    if (active_capture) {
                        std::cout << " idx: " << ifirst->idx;
                    }
                    std::cout << "\033[0m\n";
#endif
                    l2.push_back(*ifirst);
                    if (active_capture) {
                        ++new_trace;
                    }
                    continue ;
                }

                StateList * first = ifirst->rl->first;
                StateList * last = ifirst->rl->last;

                for (; first != last; ++first) {
                    ++this->step_count;

                    if (first->next && this->nums[first->next->st_num] == this->step_id) {
#ifdef DISPLAY_TRACE
                        std::cout << "\t\033[35mdup " << (ifirst->rl->st_num) << "\033[0m\n";
#endif
                        continue ;
                    }

                    if ((count_consume = first->st->check(c, consumer))) {
#ifdef DISPLAY_TRACE
                        this->display_elem_state_list(*first, active_capture ? ifirst->idx : 0);
#endif
                        if (first->next) {
                            this->nums[first->next->st_num] = this->step_id;
                        }

                        if (count_consume == 1 && !first->next && consumer.valid() && (exact_match || first->is_terminate)) {
#ifdef DISPLAY_TRACE
                            std::cout << "\t\033[35mx " << (ifirst->rl->st_num) << "\033[0m\n";
#endif
                            continue;
                        }

                        struct set_trace
                        {
                            static inline void open(unsigned idx, StateList & l, const char * s,
                                                    StateMachine2 & sm, Tracer tracer)
                            {
                                if (l.num_open != -1u) {
                                    unsigned num = l.num_open;
                                    if (!sm.traces[idx * sm.nb_capture + num] && tracer.open(idx, s, num)) {
#ifdef DISPLAY_TRACE
                                        std::cout << "\t(open) "<< num << std::endl;
#endif
                                        sm.traces[idx * sm.nb_capture + num] = s;
                                    }
                                }
                            }

                            static inline void close(unsigned idx, StateList & l, const char * s,
                                                     StateMachine2 & sm, Tracer tracer, utf8_consumer consumer)
                            {
                                if (l.num_close != -1u) {
                                    unsigned num = l.num_close;
                                    if (tracer.close(idx, s, num)) {
#ifdef DISPLAY_TRACE
                                        std::cout << "\t(close) "<< num << std::endl;
#endif
                                        sm.traces[idx * sm.nb_capture + num] = consumer.str();
                                    }
                                }
                            }
                        };

                        RangeList * const rl = count_consume == 1 ? first->next : reinterpret_cast<RangeList*>(first);

                        if (active_capture) {
                            unsigned idx = ifirst->idx;
                            if (new_trace) {
                                idx = this->pop_idx_trace(ifirst->idx);
                                tracer.new_id(ifirst->idx, idx);
                            }

                            set_trace::open(idx, *first, s, *this, tracer);
                            if (count_consume == 1) {
                                set_trace::close(idx, *first, s, *this, tracer, consumer);
                            }
#ifdef DISPLAY_TRACE
                            std::cout << "\t\033[32m" << ifirst->idx << " -> " << idx << "\033[0m" << std::endl;
#endif
                            l2.push_back(rl, count_consume - 1, idx, first->num_close);
                            ++new_trace;
                        }
                        else {
                            l2.push_back(rl, count_consume - 1);
                        }

                        if ((exact_match ? first->next_is_finish && !consumer.valid() : first->next_is_finish) || (first->is_terminate && count_consume == 1 && !consumer.valid())) {
                            //l2.pop_back();
                            return active_capture ? ifirst->idx : 0;
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
            if (this->st_range_beginning.st_num == -1u) {
                return true;
            }

            if (this->st_range_beginning.first == this->st_range_beginning.last) {
                bool ret = this->first_last ? !*s : true;
                if (active_capture && ret) {
                    tracer.open(0, s, 0);
                    tracer.close(0, s, 0);
                    this->traces[0] = s;
                    this->traces[(this->reindex_trace[1]+1)/2] = s;
                    this->set_idx_trace(0);
                }
                return ret;
            }

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

            utf8_consumer consumer(s);

            StepRangeList * pal1 = &this->l1;
            StepRangeList * pal2 = &this->l2;
            if (active_capture) {
                this->l1.push_back(&this->st_range_beginning, 0, *--this->pidx_trace_free);
                tracer.start(*this->pidx_trace_free);
            }
            else {
                this->l1.push_back(&this->st_range_beginning, 0);
            }

            while (consumer.valid()) {
#ifdef DISPLAY_TRACE
                std::cout << "\033[01;31mc: '" << utf8_char(consumer.getc()) << "'\033[0m" << std::endl;
#endif
                const unsigned result = this->step(s, consumer.bumpc(), consumer,
                                                   *pal1, *pal2, tracer,
                                                   ExactMatch<exact_match>(),
                                                   ActiveCapture<active_capture>());
                if (-1u != result) {
                    if (active_capture) {
                        this->set_idx_trace(result);
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
                    if (this->st_range_list == this->st_range_list_last) {
                        if (active_capture) {
                            s = consumer.str();
                        }
                        break;
                    }
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

                        if (active_capture) {
                            if (ifirst->num_close != -1u) {
                                unsigned num = ifirst->num_close;
                                if (tracer.close(ifirst->idx, s, num)) {
#ifdef DISPLAY_TRACE
                                    std::cout << "\t" << ifirst->idx << " (close) " << num << std::endl;
#endif
                                    this->traces[ifirst->idx * this->nb_capture + num] = s;
                                }
                            }
                        }

                        const StateList * stl = reinterpret_cast<const StateList*>(ifirst->rl);
                        if (!stl->next || stl->is_terminate) {
                            if (active_capture) {
                              this->set_idx_trace(ifirst->idx);
                            }
                            return true;
                        }
                        ifirst->rl = stl->next;
                    }

                    if (this->nums[ifirst->rl->st_num] == this->step_id) {
                        if (active_capture) {
                            /**///std::cout << "\t\033[35mx " << (ifirst->idx) << "\033[0m\n";
                            tracer.fail(ifirst->idx);
                            //this->push_idx_trace(ifirst->idx);
                        }
                        continue;
                    }
                    this->nums[ifirst->rl->st_num] = this->step_id;

                    StateList * first = ifirst->rl->first;
                    StateList * last = ifirst->rl->last;

                    for (; first != last; ++first) {
                        if (first->st->is_terminate()) {
                            if (active_capture) {
                                this->set_idx_trace(ifirst->idx);
                            }
                            return true;
                        }
                    }
                }
            }

            return false;
        }


        const State * root;
        unsigned * nums;
        bool * caps_type;
        unsigned nb_states;

        unsigned nb_capture;
        unsigned nodes;
        unsigned idx_trace;
        unsigned * reindex_trace;
        unsigned * idx_trace_free;
        unsigned * pidx_trace_free;
        const char ** traces;
        StepRangeList l1;
        StepRangeList l2;

        MinimalState * mini_sts;
        MinimalState * mini_sts_last;

        struct StateList
        {
            RangeList * next;
            const State * st;
            unsigned num_open;
            unsigned num_close;
            bool next_is_finish;
            bool is_terminate;
        };

        StateList * st_list;

        struct RangeList
        {
            StateList * first;
            StateList * last;
            unsigned st_num;
        };

        RangeList * st_range_list;
        RangeList * st_range_list_last;
        RangeList st_range_beginning;

        unsigned step_id;
        unsigned step_count;
        bool first_last;
    };
}

#endif
