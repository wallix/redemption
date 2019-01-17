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


#pragma once

#include <iostream>
#include <new>
#include <vector>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <cstring> //memset
#include <cassert>
#include <cstdint>

#include "regex_utils.hpp"
#include "utils/sugar/noncopyable.hpp"

namespace re {
#ifdef DISPLAY_TRACE
    enum { g_trace_active = 1 };
#else
    enum { g_trace_active = 0 };
#endif

#define RE_SHOW !::re::g_trace_active ? void() : void

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

        REDEMPTION_NON_COPYABLE(StateMachine2);

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

            this->st_list = reinterpret_cast<StateList*>(mem); /*NOLINT*/
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem); /*NOLINT*/
            mem += nb_st_range_list;
            this->l1.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem)); /*NOLINT*/
            mem += nb_step_range_list;
            this->l2.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem)); /*NOLINT*/
            mem += nb_step_range_list;
            this->traces = reinterpret_cast<const char**>(mem); /*NOLINT*/
            mem += nb_traces;
            this->idx_trace_free = reinterpret_cast<unsigned*>(mem); /*NOLINT*/
            mem += nb_idx_trace_free;
            this->reindex_trace = reinterpret_cast<unsigned*>(mem); /*NOLINT*/
            mem += nb_reindex_trace;
            this->nums = reinterpret_cast<unsigned*>(mem); /*NOLINT*/
            mem += nb_nums;
            this->caps_type = reinterpret_cast<bool*>(mem); /*NOLINT*/
            mem += nb_cap_type;
            this->mini_sts = reinterpret_cast<MinimalState*>(mem); /*NOLINT*/

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

            this->st_list = reinterpret_cast<StateList*>(mem); /*NOLINT*/
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem); /*NOLINT*/
            mem += nb_st_range_list;
            this->l1.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem)); /*NOLINT*/
            mem += nb_step_range_list;
            this->l2.set_array(reinterpret_cast<StepRangeList::StepRange*>(mem)); /*NOLINT*/
            mem += nb_step_range_list;
            this->nums = reinterpret_cast<unsigned*>(mem); /*NOLINT*/
            mem += nb_nums;
            this->mini_sts = reinterpret_cast<MinimalState*>(mem); /*NOLINT*/

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

            this->st_list = reinterpret_cast<StateList*>(mem); /*NOLINT*/
            mem += nb_st_list;
            this->st_range_list = reinterpret_cast<RangeList*>(mem); /*NOLINT*/
            mem += nb_st_range_list;
            this->nums = reinterpret_cast<unsigned*>(mem); /*NOLINT*/
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
        StateMachine2(const state_list_t & sts, const State * root, unsigned nb_capture,
                      bool copy_states = false, bool minimal_mem = false)
        : root(root)
        , nb_states(sts.size())
        , nb_capture(nb_capture)
        , nodes(sts.size() - nb_capture)
        , idx_trace(-1u)
        , reindex_trace(nullptr)
        , idx_trace_free(nullptr)
        , pidx_trace_free(nullptr)
        , traces(nullptr)
        , mini_sts_last(nullptr)
        , st_list(nullptr)
        , st_range_list(nullptr)
        , step_id(1)
        , first_last(false)
        , consumer(nullptr)
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
                    using tab2_st_t = StateList*[2];
                    tab2_st_t& tab = reinterpret_cast<tab2_st_t&>(*first); /*NOLINT*/
                    //overlap
                    StateList * cpfirst = tab[0];
                    StateList * cplast = tab[1];
                    for (; cpfirst != cplast; ++cpfirst, ++firstdest) {
                        *firstdest = *cpfirst;
                        cpfirst->st = nullptr;
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
                            first->next = nullptr;
                        }
                    }
                }
                StateList * first = this->st_range_beginning.first;
                StateList * last = this->st_range_beginning.last;
                for (; first != last; ++first) {
                    if (first->next && (first->next->first == first->next->last || first->next->st_num == -1u)) {
                        first->next = nullptr;
                    }
                }
            }

            if (this->st_range_list == this->st_range_list_last && !root->out1 && !root->out2) {
                if (minimal_mem) {
                    ::operator delete(this->st_list);
                    if (copy_states) {
                        void * const mem = ::operator new(sizeof(MinimalState) + this->nb_states * sizeof * this->nums);
                        this->st_list = static_cast<StateList*>(mem);
                        this->nums = reinterpret_cast<unsigned*>(static_cast<MinimalState*>(mem) + 1); /*NOLINT*/
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
                using state_iterator = state_list_t::const_iterator;
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

            this->st_range_beginning.st_num = 0;

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
                this->root = reinterpret_cast<State*>(this->mini_sts); /*NOLINT*/
                this->mini_sts->type = 0;
                this->mini_sts->num = 0;
                this->mini_sts_last = this->mini_sts + 1;
            }
            else if (copy_states) {
                {
                    const size_t size_mini_sts = (this->nb_states - this->nb_capture) * sizeof(MinimalState);
                    char_int * str = reinterpret_cast<char_int*>(  /*NOLINT*/
                        reinterpret_cast<char*>(this->mini_sts) + size_mini_sts  /*NOLINT*/
                        + size_mini_sts % sizeof(intmax_t)
                    );
                    state_list_t::const_iterator first = sts.begin();
                    state_list_t::const_iterator last = sts.end() - this->nb_capture;
                    MinimalState * first2 = this->mini_sts;
                    for (; first != last; ++first) {
                        if (*first == this->root) {
                            this->root = reinterpret_cast<State*>(first2); /*NOLINT*/
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
                        l->st = reinterpret_cast<State*>(msts); /*NOLINT*/
                    }
                }

                StateList * l = this->st_range_beginning.first;
                StateList * rlast = this->st_range_beginning.last;
                for (; l != rlast; ++l) {
                    MinimalState * msts = this->mini_sts;
                    while (msts->num != l->st->num) {
                        ++msts;
                    }
                    l->st = reinterpret_cast<State*>(msts); /*NOLINT*/
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

            this->sl_beginning.next = &this->st_range_beginning;
        }

        ~StateMachine2()
        {
            ::operator delete(this->st_list);
        }

        unsigned mark_count() const
        {
            return this->nb_capture;
        }

        unsigned node_count() const
        {
            return this->nodes;
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
                    l->next_is_finish = l->st->is_finish() || l->next == nullptr || this->next_is_finish(l->st->out1);
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
                if (st->is_cap()) {
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
                if (st->is_cap()) {
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
                        using tab2_st_t = StateList*[2];
                        tab2_st_t& tab = reinterpret_cast<tab2_st_t&>(*this->st_range_beginning.last); /*NOLINT*/
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
            return nullptr;
        }

        void init_range_list(RangeList* l, const State * st, unsigned is_first = true)
        {
            /**///std::cout << (__FUNCTION__) << std::endl;
            l->st_num = st->num;
            this->push_state(l, st, -1u, is_first ? 0 : 2);
            /**///std::cout << "-- " << (l) << std::endl;
            for (StateList * first = l->first, * last = l->last; first < last; ++first) {
                /**///std::cout << first->st->num << ("\t") << first->st << ("\t") << first->next << std::endl;
                if (nullptr == first->st->out1) {
                    continue ;
                }
                if (first->st->out1->is_finish()) {
                    first->next = nullptr;
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
        enum {
            match_fail = 0,
            match_success = 1,
            match_undetermined = 2
        };

        using range_t = std::pair<const char *, const char *>;
        using range_matches = std::vector<range_t>;

        class DefaultMatchTracer
        {
            StateMachine2 & sm;

        public:
            explicit DefaultMatchTracer(StateMachine2 & sm)
            : sm(sm)
            {}

            unsigned new_id(unsigned old_id) const
            { return this->sm.pop_idx_trace(old_id); }

            bool open(unsigned /*idx*/, const char * /*s*/, unsigned /*num_cap*/) const
            { return true; }

            bool close(unsigned /*idx*/, const char * /*s*/, unsigned /*num_cap*/) const
            { return true; }

            void fail(unsigned idx) const
            { this->sm.push_idx_trace(idx); }

            void good(unsigned idx) const
            { this->sm.set_idx_trace(idx); }

            using range_idx_trace = std::pair<const unsigned *, const unsigned *>;

            range_idx_trace range_idx_trace_reserved() const
            { return range_idx_trace(this->sm.idx_trace_free, this->sm.pidx_trace_free); }
        };

    private:
        template<bool> struct ExactMatch { };
        template<bool> struct ActiveCapture { };
        template<bool> struct ActivePartOfText { };

    public:
        bool exact_search(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match(s, step_limit, DefaultMatchTracer(*this), pos,
                               ExactMatch<true>(), ActiveCapture<false>(), ActivePartOfText<false>());
        }

        bool exact_search_with_trace(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->exact_search(s, step_limit, pos);
            }
            return this->match(s, step_limit, DefaultMatchTracer(*this), pos,
                               ExactMatch<true>(), ActiveCapture<true>(), ActivePartOfText<false>());
        }

        template<typename Tracer>
        bool exact_search_with_trace(const char * s, unsigned step_limit, Tracer tracer, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->exact_search(s, step_limit, pos);
            }
            return this->match(s, step_limit, tracer, pos,
                               ExactMatch<true>(), ActiveCapture<true>(), ActivePartOfText<false>());
        }

        bool search(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match(s, step_limit, DefaultMatchTracer(*this), pos,
                               ExactMatch<false>(), ActiveCapture<false>(), ActivePartOfText<false>());
        }

        bool search_with_trace(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->search(s, step_limit, pos);
            }
            return this->match(s, step_limit, DefaultMatchTracer(*this), pos,
                               ExactMatch<false>(), ActiveCapture<true>(), ActivePartOfText<false>());
        }

        template<typename Tracer>
        bool search_with_trace(const char * s, unsigned step_limit, Tracer tracer, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->search(s, step_limit, pos);
            }
            return this->match(s, step_limit, tracer, pos,
                               ExactMatch<false>(), ActiveCapture<true>(), ActivePartOfText<false>());
        }


        unsigned part_of_text_exact_search(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match_run(s, step_limit, DefaultMatchTracer(*this), pos,
                                   ExactMatch<true>(), ActiveCapture<false>(), ActivePartOfText<true>());
        }

        unsigned part_of_text_exact_search_with_trace(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_exact_search(s, step_limit, pos);
            }
            return this->match_run(s, step_limit, DefaultMatchTracer(*this), pos,
                                   ExactMatch<true>(), ActiveCapture<true>(), ActivePartOfText<true>());
        }

        template<typename Tracer>
        unsigned part_of_text_exact_search_with_trace(const char * s, unsigned step_limit, Tracer tracer,
                                                      size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_exact_search(s, step_limit, pos);
            }
            return this->match_run(s, step_limit, tracer, pos,
                                   ExactMatch<true>(), ActiveCapture<true>(), ActivePartOfText<true>());
        }

        unsigned part_of_text_search(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (0 == this->nb_states) {
                return !*s;
            }
            return this->match_run(s, step_limit, DefaultMatchTracer(*this), pos,
                                   ExactMatch<false>(), ActiveCapture<false>(), ActivePartOfText<true>());
        }

        unsigned part_of_text_search_with_trace(const char * s, unsigned step_limit, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_search(s, step_limit, pos);
            }
            return this->match_run(s, step_limit, DefaultMatchTracer(*this), pos,
                                   ExactMatch<false>(), ActiveCapture<true>(), ActivePartOfText<true>());
        }

        template<typename Tracer>
        unsigned part_of_text_search_with_trace(const char * s, unsigned step_limit, Tracer tracer,
                                                size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_search(s, step_limit, pos);
            }
            return this->match_run(s, step_limit, tracer, pos,
                                   ExactMatch<false>(), ActiveCapture<true>(), ActivePartOfText<true>());
        }


        unsigned part_of_text_start(const char * s, size_t * pos = nullptr)
        {
            if (0 == this->nb_states) {
                return !*s ? match_success : match_fail;
            }
            return this->match_start(s, DefaultMatchTracer(*this), pos, ActiveCapture<false>());
        }

        unsigned part_of_text_start_with_trace(const char * s, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_start(s, pos);
            }
            return this->match_start(s, DefaultMatchTracer(*this), pos, ActiveCapture<true>());
        }

        template<typename Tracer>
        unsigned part_of_text_start_with_trace(const char * s, Tracer tracer, size_t * pos = nullptr)
        {
            if (this->nb_capture == 0) {
                return this->part_of_text_start(s, pos);
            }
            return this->match_start(s, tracer, pos, ActiveCapture<true>());
        }


        bool part_of_text_finish(size_t * pos = nullptr)
        {
            return this->match_finish(DefaultMatchTracer(*this), pos, ActiveCapture<false>(),
                                      ActivePartOfText<true>());
        }

        bool part_of_text_finish_with_trace(size_t * pos = nullptr)
        {
            return this->match_finish(DefaultMatchTracer(*this), pos, ActiveCapture<true>(),
                                      ActivePartOfText<true>());
        }

        template<typename Tracer>
        bool part_of_text_finish_with_trace(Tracer tracer, size_t * pos = nullptr)
        {
            return this->match_finish(tracer, pos, ActiveCapture<true>(), ActivePartOfText<true>());
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
                    ranges.push_back(range_t(nullptr,nullptr));
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
                const StateList * stl;
                unsigned consume;
                unsigned idx;
                unsigned num_close;
                unsigned real_count_consume;
            };

            using iterator = StepRange *;

            StepRange & next_uninitialized()
            {
                assert(unsigned(this->last - this->list) != this->nodes);
                return *this->last++;
            }

            void pop()
            { --this->last; }

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

        using StepRangeIterator = StepRangeList::iterator;

        static unsigned part_of_text_search_check(const State & st, unsigned pos, char_int c,
                                             utf8_consumer consumer,
                                             bool & count_consume_is_one)
        {
            const Sequence & seq = st.data.sequence;
            if (c == seq.s[pos]) {
                const char_int * s = seq.s + pos + 1;
                while (*s && *s == consumer.bumpc()) {
                    ++s;
                }
                count_consume_is_one = seq.len == 1;
                return (*s && consumer.valid())
                ? 0
                : s - (seq.s + pos) + ((size_t(s - seq.s) == seq.len) ? 0 : 1);
            }
            return 0;
        }

        static unsigned part_of_text_search_check(const State & st, char_int c,
                                             utf8_consumer consumer,
                                             bool & count_consume_is_one)
        {
            if (st.type == SEQUENCE) {
                return part_of_text_search_check(st, 0, c, consumer, count_consume_is_one);
            }
            count_consume_is_one = true;
            return st.data.range.contains(c) ? 1 : 0;
        }

        template<typename Tracer>
        void fail_trace(unsigned idx, Tracer tracer)
        {
            RE_SHOW(std::cout << "\t\033[35mx " << idx << "\033[0m\n");
            tracer.fail(idx);
        }

        template<typename Tracer>
        bool set_trace_open(unsigned idx, unsigned num_open, Tracer tracer)
        {
            if (num_open != -1u) {
                unsigned num = num_open;
                const char * & trace = this->traces[idx * this->nb_capture + num];
                if (!trace) {
                    RE_SHOW(std::cout << "\t" << idx << "\t(open) "<< num << std::endl);
                    if (!tracer.open(idx, this->s, num)) {
                        this->fail_trace(idx, tracer);
                        return false;
                    }
                    trace = this->s;
                }
            }
            return true;
        }

        template<typename Tracer>
        bool set_trace_close(unsigned idx, unsigned num_close, Tracer tracer)
        {
            if (num_close != -1u) {
                unsigned num = num_close;
                if (!tracer.close(idx, this->s, num)) {
                    this->fail_trace(idx, tracer);
                    return false;
                }
                RE_SHOW(std::cout << "\t" << idx << "\t(close) "<< num << std::endl);
                this->traces[idx * this->nb_capture + num] = this->consumer.str();
            }
            return true;
        }

        template<typename Tracer, bool exact_match, bool active_capture, bool active_part_of_text>
        unsigned step(char_int c, Tracer tracer,
                      ExactMatch<exact_match> /*match*/,
                      ActiveCapture<active_capture> /*capture*/,
                      ActivePartOfText<active_part_of_text> /*part*/)
        {
            StepRangeList & l1 = *this->pal1;
            StepRangeList & l2 = *this->pal2;

            unsigned new_trace;
            unsigned count_consume;
            bool count_consume_is_one = false;
            for (StepRangeIterator ifirst = l1.begin(), ilast = l1.end(); ifirst != ilast; ++ifirst) /* NOLINT(modernize-loop-convert) */{
                ++this->step_count;
                if (active_capture) {
                    new_trace = 0;
                }

                if (ifirst->consume) {
                    if (!--ifirst->consume) {
                        const StateList & stl = *ifirst->stl;

                        if (active_part_of_text && ifirst->real_count_consume != stl.st->data.sequence.len) {
                            {
                                unsigned pos = ifirst->real_count_consume;
                                const Sequence & seq = stl.st->data.sequence;
                                if (c == seq.s[pos]) {
                                    count_consume = 1;
                                    count_consume_is_one = seq.len == 1;
                                }
                                else {
                                    count_consume = 0;
                                }
                            }

                            if (!count_consume || (count_consume_is_one && !stl.next && consumer.valid() && (exact_match || stl.is_terminate))) {
                                if (g_trace_active && stl.next) {
                                    RE_SHOW(std::cout << "\t\033[35mx " << stl.next->st_num << "\033[0m\n");
                                }
                                continue;
                            }

                            RE_SHOW(this->display_elem_state_list(stl, active_capture ? ifirst->idx : 0));

                            ifirst->consume = count_consume;
                            ifirst->real_count_consume += count_consume;
                            if (stl.st->data.sequence.len == ifirst->real_count_consume) {
                                ifirst->consume = 0;
                            }

                            if (active_capture && count_consume_is_one) {
                                if (!this->set_trace_close(ifirst->idx, ifirst->num_close, tracer)) {
                                    continue;
                                }
                            }

                            if (((exact_match ? stl.next_is_finish && !consumer.valid() : stl.next_is_finish)
                                && (stl.st->type != SEQUENCE
                                 || stl.st->data.sequence.len == ifirst->real_count_consume))
                            || (stl.is_terminate && count_consume_is_one)) {
                                const unsigned ret = (active_capture ? ifirst->idx : 0);
                                if (ret != -1u) {
                                    RE_SHOW(std::cout << "\033[36mconsumer += " << ifirst->consume << "\033[0m\n");
                                    this->consumer.s += ifirst->consume;
                                }
                                return ret;
                            }
                        }
                        else {
                            if (!stl.next && consumer.valid() && (exact_match || stl.is_terminate)) {
                                if (g_trace_active && stl.next) {
                                    RE_SHOW(std::cout << "\t\033[35mx " << (stl.next->st_num) << "\033[0m\n");
                                }
                                continue;
                            }

                            if (active_capture) {
                                if (!this->set_trace_close(ifirst->idx, ifirst->num_close, tracer)) {
                                    continue;
                                }
                            }

                            if (stl.next_is_finish || stl.is_terminate) {
                                const unsigned ret = (active_capture ? ifirst->idx : 0);
                                if (ret != -1u) {
                                    RE_SHOW(std::cout << "\033[36mconsumer += " << ifirst->consume << "\033[0m\n");
                                    this->consumer.s += ifirst->consume;
                                }
                                return ret;
                            }

                            if(active_capture) {
                                ifirst->consume = 0;
                            }
                        }
                    }
                    else if (active_part_of_text && ifirst->real_count_consume) {
                        const StateList & stl = *ifirst->stl;
                        unsigned pos = ifirst->real_count_consume;
                        const Sequence & seq = stl.st->data.sequence;
                        if (c != seq.s[pos]) {
                            continue;
                        }
                        ++ifirst->real_count_consume;
                    }

                    RE_SHOW(std::cout << "\t\033[35mreinsert (" << ifirst->consume << ")");
                    if (active_capture) {
                        RE_SHOW(std::cout << " idx: " << ifirst->idx);
                    }
                    RE_SHOW(std::cout << "\033[0m\n");

                    StepRangeList::StepRange & sr = l2.next_uninitialized();
                    sr.stl = ifirst->stl;
                    sr.consume = ifirst->consume;
                    if (active_capture) {
                        sr.idx = ifirst->idx;
                        sr.num_close = ifirst->num_close;
                        ++new_trace;
                    }
                    if (active_part_of_text) {
                        sr.real_count_consume = ifirst->real_count_consume;
                    }
                    continue ;
                }

                const RangeList & rl = *ifirst->stl->next;
                StateList * first = rl.first;
                StateList * last = rl.last;

                for (; first != last; ++first) {
                    ++this->step_count;

                    if (first->next && this->nums[first->next->st_num] == this->step_id) {
                        RE_SHOW(std::cout << "\t\033[35mdup " << (rl.st_num) << "\033[0m\n");
                        continue ;
                    }

                    if ((count_consume = active_part_of_text
                        ? part_of_text_search_check(*first->st, c, consumer, count_consume_is_one)
                        : first->st->check(c, consumer))
                    ) {
                        RE_SHOW(this->display_elem_state_list(*first, active_capture ? ifirst->idx : 0));

                        if (first->next) {
                            this->nums[first->next->st_num] = this->step_id;
                        }

                        if (!active_part_of_text) {
                            count_consume_is_one = count_consume == 1;
                        }

                        if (count_consume_is_one && !first->next && consumer.valid() && (exact_match || first->is_terminate)) {
                            RE_SHOW(std::cout << "\t\033[35mx " << (rl.st_num) << "\033[0m\n");
                            continue;
                        }

                        StepRangeList::StepRange & sr = l2.next_uninitialized();
                        sr.stl = first;
                        sr.consume = count_consume - 1;
                        if (active_part_of_text) {
                            sr.real_count_consume = 1;
                        }

                        if (active_capture) {
                            unsigned idx = ifirst->idx;
                            if (new_trace) {
                                idx = tracer.new_id(ifirst->idx);
                            }

                            if (!this->set_trace_open(idx, first->num_open, tracer)) {
                                l2.pop();
                                continue ;
                            }
                            if ((
                                count_consume_is_one ||
                                (first->st->type == SEQUENCE &&
                                 first->st->data.sequence.len == count_consume)
                            ) && !this->set_trace_close(idx, first->num_close, tracer)) {
                                l2.pop();
                                continue ;
                            }
                            RE_SHOW(std::cout << "\t\033[32m" << ifirst->idx << " -> " << idx << "\033[0m\n");
                            sr.idx = idx;
                            sr.num_close = first->num_close;
                            ++new_trace;
                        }

                        if (((exact_match ? first->next_is_finish && !consumer.valid() : first->next_is_finish)
                            && (!active_part_of_text || first->st->type != SEQUENCE
                             || first->st->data.sequence.len+1 == count_consume))
                        || (first->is_terminate && count_consume_is_one && !consumer.valid())) {
                            //l2.pop_back();
                            const unsigned ret = (active_capture ? ifirst->idx : 0);
                            if (ret != -1u) {
                                RE_SHOW(std::cout << "\033[36mconsumer += " << sr.consume << "\033[0m\n");
                                this->consumer.s += sr.consume;
                            }
                            return ret;
                        }
                    }
                }

                if (active_capture && 0 == new_trace) {
                    this->fail_trace(ifirst->idx, tracer);
                }
            }

            return -1u;
        }

        template<unsigned State> struct MatchState { static const unsigned value = State; };
        using MatchStart = MatchState<1>;
        using MatchRun = MatchState<2>;
        using MatchFinish = MatchState<4>;
        using MatchImpl = MatchState<3>;

        template<typename Tracer, bool active_capture>
        unsigned match_start(const char * s, Tracer tracer, size_t * ppos,
                             ActiveCapture<active_capture> /*capture*/)
        {
            return match_impl(s, 0, tracer, ppos,
                              ExactMatch<false>(),
                              ActiveCapture<active_capture>(),
                              ActivePartOfText<false>(),
                              MatchStart()
            );
        }


        template<typename Tracer, bool active_capture, bool active_part_of_text>
        bool match_finish(Tracer tracer, size_t * ppos, ActiveCapture<active_capture> /*capture*/,
                          ActivePartOfText<active_part_of_text> /*part*/)
        {
            RE_SHOW(std::cout << ("finish") << std::endl);
            return match_impl(s, 0, tracer, ppos,
                              ExactMatch<false>(),
                              ActiveCapture<active_capture>(),
                              ActivePartOfText<active_part_of_text>(),
                              MatchFinish()
            ) == match_success;
        }

        void set_pos(size_t * ppos) const
        {
            if (ppos) {
                *ppos = this->consumer.str() - this->start_s;
            }
        }


        template<typename Tracer, bool exact_match, bool active_capture, bool active_part_of_text>
        unsigned match_run(const char * s, unsigned step_limit, Tracer tracer, size_t * ppos,
                           ExactMatch<exact_match> /*match*/,
                           ActiveCapture<active_capture> /*capture*/,
                           ActivePartOfText<active_part_of_text> /*part*/)
        {
            return match_impl(s, step_limit, tracer, ppos,
                              ExactMatch<exact_match>(),
                              ActiveCapture<active_capture>(),
                              ActivePartOfText<active_part_of_text>(),
                              MatchRun()
                             );
        }

        template<typename Tracer, bool exact_match, bool active_capture, bool active_part_of_text>
        bool match(const char * s, unsigned step_limit, Tracer tracer, size_t * ppos,
                        ExactMatch<exact_match> /*match*/,
                        ActiveCapture<active_capture> /*capture*/,
                        ActivePartOfText<active_part_of_text> /*part*/)
        {
            return match_impl(s, step_limit, tracer, ppos,
                              ExactMatch<exact_match>(),
                              ActiveCapture<active_capture>(),
                              ActivePartOfText<active_part_of_text>(),
                              MatchImpl()
                             ) == match_success;
        }

        template<typename Tracer, bool exact_match, bool active_capture, bool active_part_of_text, unsigned R>
        unsigned match_impl(const char * s, unsigned step_limit, Tracer tracer, size_t * ppos,
                            ExactMatch<exact_match> /*match*/,
                            ActiveCapture<active_capture> /*capture*/,
                            ActivePartOfText<active_part_of_text> /*part*/,
                            MatchState<R> /*match*/)
        {
            if (R & MatchStart::value) {
                if (ppos) {
                    *ppos = 0;
                }

                if (this->st_range_beginning.st_num == -1u) {
                    return match_success;
                }

                if (this->st_range_beginning.first == this->st_range_beginning.last) {
                    const unsigned ret = this->first_last && *s ? match_fail : match_success;
                    if (active_capture && ret) {
                        if (!(tracer.open(0, s, 0) && tracer.close(0, s, 0))) {
                            return match_fail;
                        }
                        this->traces[0] = s;
                        this->traces[(this->reindex_trace[1]+1)/2] = s;
                        tracer.good(0);
                    }
                    return ret;
                }

                RE_SHOW(this->display_dfa());

                this->step_count = 0;
                this->step_id = 1;

                this->l1.clear();
                this->l2.clear();
                this->reset_id();
                if (active_capture) {
                    this->reset_trace();
                }

                this->start_s = s;

                this->pal1 = &this->l1;
                this->pal2 = &this->l2;

                {
                    StepRangeList::StepRange & sr = this->l1.next_uninitialized();
                    sr.stl = &this->sl_beginning;
                    sr.consume = 0;
                    if (active_capture) {
                        sr.idx = *--this->pidx_trace_free;
                    }
                }

                if (R == MatchStart::value) {
                    return match_undetermined;
                }
            }

            if (R & MatchRun::value) {
                this->s = s;
                this->consumer.str(s);

                while (this->consumer.valid()) {
                    RE_SHOW(std::cout << "\033[01;31mc: '" << utf8_char(consumer.getc()) << "'\033[0m\n");
                    const unsigned result = this->step(this->consumer.bumpc(), tracer,
                                                       ExactMatch<exact_match>(),
                                                       ActiveCapture<active_capture>(),
                                                       ActivePartOfText<active_part_of_text>());
                    if (-1u != result) {
                        if (active_capture) {
                            tracer.good(result);
                        }
                        this->set_pos(ppos);
                        return match_success;
                    }
                    if (exact_match && this->pal2->empty()) {
                        this->set_pos(ppos);
                        return match_fail;
                    }
                    if (this->step_count >= step_limit) {
                        this->set_pos(ppos);
                        return match_fail;
                    }
                    ++this->step_id;
                    std::swap(this->pal1, this->pal2);
                    this->pal2->clear();
                    if (!exact_match) {
                        if (this->st_range_list == this->st_range_list_last) {
                            if (active_capture) {
                                this->s = this->consumer.str();
                            }
                            break;
                        }
                        StepRangeList::StepRange & sr = this->pal1->next_uninitialized();
                        sr.stl = &this->sl_beginning;
                        sr.consume = 0;
                        if (active_capture) {
                            --this->pidx_trace_free;
                            assert(this->pidx_trace_free >= this->idx_trace_free);
                            RE_SHOW(std::cout << "\t\033[32m-> " << *this->pidx_trace_free << "\033[0m\n");
                            sr.idx = *this->pidx_trace_free;
                        }
                    }
                    if (active_capture) {
                        this->s = this->consumer.str();
                    }
                }

                if (R == MatchRun::value) {
                    return match_undetermined;
                }
            }

            RE_SHOW(std::cout << "\033[35mconsumer.valid(): '" << consumer.valid() << "'\033[0m" << std::endl);

            //MatchFinish
            {
                struct SetPos {
                    StateMachine2 & sm;
                    size_t * ppos;

                    SetPos(StateMachine2 & sm, size_t * ppos)
                    : sm(sm)
                    , ppos(ppos)
                    {}

                    ~SetPos()
                    { this->sm.set_pos(this->ppos); }
                } auto_set_pos(*this, ppos);

                if (!consumer.valid()) {
                    StepRangeIterator ifirst = this->pal1->begin();
                    StepRangeIterator ilast = this->pal1->end();
                    for (; ifirst != ilast; ++ifirst) {
                        if (ifirst->consume) {
                            if (ifirst->consume != 1) {
                                continue ;
                            }

                            const StateList & stl = *ifirst->stl;

                            if (active_part_of_text && stl.st->type == SEQUENCE && stl.st->data.sequence.len != ifirst->real_count_consume) {
                                continue ;
                            }

                            if (active_capture) {
                                if (!this->set_trace_close(ifirst->idx, ifirst->num_close, tracer)) {
                                    continue ;
                                }
                            }
                            if (!stl.next || stl.is_terminate) {
                                if (active_capture) {
                                    tracer.good(ifirst->idx);
                                }
                                return match_success;
                            }
                        }

                        const RangeList & rl = *ifirst->stl->next;

                        if (this->nums[rl.st_num] == this->step_id) {
                            continue;
                        }
                        this->nums[rl.st_num] = this->step_id;

                        StateList * first = rl.first;
                        StateList * last = rl.last;

                        for (; first != last; ++first) {
                            if (first->st->is_terminate()) {
                                if (active_capture) {
                                    tracer.good(ifirst->idx);
                                }
                                return match_success;
                            }
                        }
                    }
                }

                return match_fail;
            }
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
        StateList sl_beginning;

        unsigned step_id;
        unsigned step_count;
        bool first_last;

        //BEGIN only for match and step
        utf8_consumer consumer;
        StepRangeList * pal1;
        StepRangeList * pal2;
        const char * start_s;
        const char * s;
        //END
    };
}  // namespace re

