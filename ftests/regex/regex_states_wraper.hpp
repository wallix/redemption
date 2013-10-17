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

    typedef std::vector<const StateBase*> const_state_list_t;
    typedef std::vector<StateBase*> state_list_t;

    inline void append_state(StateBase * st, state_list_t& sts)
    {
        if (st && st->num != -1u) {
            st->num = -1u;
            sts.push_back(st);
            append_state(st->out1, sts);
            append_state(st->out2, sts);
        }
    }

    struct StateDeleter
    {
        void operator()(StateBase * st) const
        {
            delete st;
        }
    };

    class StatesWrapper
    {
        std::vector<unsigned> nums;

        struct IsCapture {
            bool operator()(const StateBase * st) const
            {
                return ! st->is_cap();
            }
        };
    public:
        state_list_t states;
        StateBase * root;
        unsigned nb_capture;

        explicit StatesWrapper()
        : root(0)
        , nb_capture(0)
        {}

        explicit StatesWrapper(StateBase * st)
        : root(st)
        , nb_capture(0)
        {
            append_state(st, this->states);
        }

        void reset(StateBase * st)
        {
            std::for_each(this->states.begin(), this->states.end(), StateDeleter());
            this->states.clear();
            this->root = st;
            append_state(st, this->states);
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

        void reset_num()
        {
            std::fill(this->nums.begin(), this->nums.end(), 0);
        }

        void set_num_at(const StateBase * st, unsigned count)
        {
            assert(st == this->states[st->num]);
            assert(this->states.size() > st->num);
            this->nums[st->num] = count;
        }

        unsigned get_num_at(const StateBase * st) const
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
