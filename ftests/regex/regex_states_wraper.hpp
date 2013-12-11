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

#ifndef REDEMPTION_REGEX_STATE_WRAPPER_HPP
#define REDEMPTION_REGEX_STATE_WRAPPER_HPP

#include <ostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "regex_state.hpp"

namespace re {

    typedef std::vector<const State*> const_state_list_t;
    typedef std::vector<State*> state_list_t;

    inline void append_state(State * st, state_list_t& sts)
    {
        //if (st && st->num != -1u) {
        //    st->num = -1u;
        //    sts.push_back(st);
        //    append_state(st->out1, sts);
        //    append_state(st->out2, sts);
        //}

        if (!st) {
            return ;
        }

        state_list_t stack;
        stack.reserve(16);
        sts.reserve(32);

        st->num = -1u;
        sts.push_back(st);
        stack.push_back(st);

        while (!stack.empty()) {
            st = stack.back();
            while (st->out1 && st->out1->num != -1u) {
                st = st->out1;
                stack.push_back(st);
                sts.push_back(st);
                st->num = -1u;
            }
            st = st->out2;
            if (st && st->num != -1u) {
                stack.push_back(st);
                sts.push_back(st);
                st->num = -1u;
                continue;
            }
            stack.pop_back();
            while (!stack.empty()) {
                st = stack.back()->out2;
                if (st && st->num != -1u) {
                    stack.push_back(st);
                    sts.push_back(st);
                    st->num = -1u;
                    break;
                }
                stack.pop_back();
            }
        }
    }

    struct StateDeleter
    {
        void operator()(State * st) const
        {
            delete st;
        }
    };

    struct IsEpsilone
    {
        bool operator()(State * st) const
        {
            return st->is_epsilone();
        }
    };

    class StatesWrapper
    {
        std::vector<unsigned> nums;

        struct IsCapture {
            bool operator()(const State * st) const
            {
                return ! st->is_cap();
            }
        };
    public:
        state_list_t states;
        State * root;
        unsigned nb_capture;

        explicit StatesWrapper()
        : root(0)
        , nb_capture(0)
        {}

        explicit StatesWrapper(State * st)
        : root(st)
        , nb_capture(0)
        {
            append_state(st, this->states);
        }

        void reset(State * st)
        {
            std::for_each(this->states.begin(), this->states.end(), StateDeleter());
            this->states.clear();
            this->root = st;
            append_state(st, this->states);

            state_list_t::iterator last = this->states.end();
            if (std::find_if(this->states.begin(), last, IsEpsilone()) != last) {
                for (state_list_t::iterator first = this->states.begin(); first != last; ++first) {
                    State * nst = (*first)->out1;
                    while (nst && nst->is_epsilone()) {
                        nst = nst->out1;
                    }
                    (*first)->out1 = nst;
                }
                state_list_t::iterator first = this->states.begin();
                while (first != last && !(*first)->is_epsilone()) {
                    ++first;
                }
                state_list_t::iterator result = first;
                for (; first != last; ++first) {
                    if ((*first)->is_epsilone()) {
                        delete *first;
                    }
                    else {
                        *result = *first;
                        ++result;
                    }
                }
                this->states.resize(result - this->states.begin());
            }

            this->init_nums();
        }

        void init_nums()
        {
            state_list_t::iterator first = this->states.begin();
            state_list_t::iterator last = this->states.end();
            state_list_t::iterator first_cap = std::stable_partition(first, last, IsCapture());
            this->nb_capture = last - first_cap;

            for (unsigned n = 0; first != last; ++first, ++n) {
                (*first)->num = n;
            }

            this->nums.resize(this->states.size(), 0);
        }

        ~StatesWrapper()
        {
            std::for_each(this->states.begin(), this->states.end(), StateDeleter());
        }

        void reset_nums()
        {
            std::fill(this->nums.begin(), this->nums.end(), 0);
        }

        void set_num_at(const State * st, unsigned count)
        {
            assert(st == this->states[st->num]);
            assert(this->states.size() > st->num);
            this->nums[st->num] = count;
        }

        unsigned get_num_at(const State * st) const
        {
            assert(st == this->states[st->num]);
            assert(this->states.size() > st->num);
            return this->nums[st->num];
        }

        size_t size() const
        {
            return this->states.size();
        }

    private:
        StatesWrapper(const StatesWrapper &);
        StatesWrapper& operator=(const StatesWrapper &);
    };

}

#endif
