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

#include <vector>
#include <algorithm>
#include <cassert>

#include "regex_parser.hpp"

namespace re {

    class StatesValue
    {
        std::vector<unsigned> nums;

    public:
        const state_list_t & states;

        StatesValue(const state_list_t & sts, unsigned /*nb_cap*/)
        : nums(sts.size(), 0)
        , states(sts)
        {}

        ~StatesValue()
        {}

        void reset_nums()
        {
            std::fill(this->nums.begin(), this->nums.end(), 0);
        }

        void set_num_at(const State * st, unsigned count)
        {
            assert(this->states.size() > st->num);
            this->nums[st->num] = count;
        }

        unsigned get_num_at(const State * st) const
        {
            assert(this->states.size() > st->num);
            return this->nums[st->num];
        }
    };

}

