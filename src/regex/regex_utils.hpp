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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include <vector>

#include "regex_state.hpp"

namespace re {

    struct IsNotCapture
    {
        bool operator()(const State * st) const
        {
            return ! st->is_cap();
        }
    };

    struct IsEpsilone
    {
        bool operator()(State * st) const
        {
            return st->is_epsilone();
        }
    };

    using const_state_list_t = std::vector<const State *>;
    using state_list_t = std::vector<State *>;

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

    template<typename Deleter>
    inline void remove_epsilone(state_list_t & states, Deleter deleter)
    {
        state_list_t::iterator last = states.end();
        if (std::find_if(states.begin(), last, IsEpsilone()) != last) {
            for (state_list_t::iterator first = states.begin(); first != last; ++first) {
                State * nst = (*first)->out1;
                while (nst && nst->is_epsilone()) {
                    nst = nst->out1;
                }
                (*first)->out1 = nst;

                nst = (*first)->out2;
                while (nst && nst->is_epsilone()) {
                    nst = nst->out1;
                }
                (*first)->out2 = nst;
            }
            state_list_t::iterator first = states.begin();
            while (first != last && !(*first)->is_epsilone()) {
                ++first;
            }
            state_list_t::iterator result = first;
            for (; first != last; ++first) {
                if ((*first)->is_epsilone()) {
                    deleter(*first);
                }
                else {
                    *result = *first;
                    ++result;
                }
            }
            states.resize(result - states.begin());
        }
    }

    struct StateDeleter
    {
        void operator()(State * st) const
        {
            delete st; /*NOLINT*/
        }
    };
}  // namespace re

