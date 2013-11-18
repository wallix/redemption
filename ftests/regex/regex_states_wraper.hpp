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

#include <vector>
#include <algorithm>
#include <cassert>

#include "regex_parser.hpp"

namespace re {

    class StatesValue
    {
        std::vector<unsigned> nums;

    public:
        unsigned nb_capture;
        state_list_t & states;

        StatesValue(state_list_t & sts)
        : states(sts)
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

        ~StatesValue()
        {}

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
    };

    class StatesWrapper
    {
    public:
        state_list_t states;
        State * root;

        explicit StatesWrapper()
        : root(0)
        {}

        explicit StatesWrapper(State * st)
        : root(st)
        {
            append_state(st, this->states);
            remove_epsilone(this->states);
        }

        void compile(const char * s, const char * * msg_err = 0, size_t * pos_err = 0)
        {
            const char * err = 0;
            utf_consumer consumer(s);
            StateAccu accu(this->states);
            unsigned num_cap = 0;
            this->root = intermendary_st_compile(accu, consumer, err, num_cap).first;
            if (msg_err) {
                *msg_err = err;
            }
            if (pos_err) {
                *pos_err = err ? consumer.str() - s : 0;
            }

            if (err) {
                accu.clear();
            }
            else {
                remove_epsilone(this->states);
            }
        }

        void reset(State * st)
        {
            this->clean();
            this->root = st;
            append_state(st, this->states);
            remove_epsilone(this->states);
        }

        ~StatesWrapper()
        {
            std::for_each(this->states.begin(), this->states.end(), StateDeleter());
        }

    private:
        StatesWrapper(const StatesWrapper &);
        StatesWrapper& operator=(const StatesWrapper &);

        void clean()
        {
            std::for_each(this->states.begin(), this->states.end(), StateDeleter());
            this->states.clear();
        }
    };

}

#endif
