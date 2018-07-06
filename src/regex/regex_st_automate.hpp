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
#include <vector>
#include <utility>
#include <iomanip>

#include "regex_states_value.hpp"

namespace re {

    inline void display_states(StatesValue & stval, const State * root)
    {
        if (stval.states.empty()) {
            return ;
        }
        stval.reset_nums();
        struct Impl {
            static void display(StatesValue & stval, const State * st, unsigned depth = 0) {
                if (st && stval.get_num_at(st) != -2u) {
                    std::cout
                    << std::setw(depth) << "" << "\033[33m" << st << "\t" << st->num << "\t" << *st
                    << "\033[0m\n\t" << std::setw(depth) << st->out1 << "\n\t"
                    << std::setw(depth) << "" << st->out2 << "\n";
                    stval.set_num_at(st, -2u);
                    display(stval, st->out1, depth+1);
                    display(stval, st->out2, depth+1);
                }
            }
        };
        char oldfill = std::cout.fill('\t');
        Impl::display(stval, root);
        std::cout.fill(oldfill);
    }

    typedef std::pair<const State*, unsigned> st_step_elem_t;
    using st_step_range_list_t = std::vector<st_step_elem_t>;
    using st_step_range_iterator_t = st_step_range_list_t::iterator;

    inline bool st_exact_step(st_step_range_list_t & l1, st_step_range_list_t & l2,
                              StatesValue & stval, size_t c, utf8_consumer & consumer, unsigned count)
    {
        struct add {
            static bool impl(st_step_range_list_t & l, const State * st,
                             StatesValue & stval, bool is_end, unsigned count, unsigned count_consume)
            {
                if (stval.get_num_at(st) == count) {
                    return false;
                }
                stval.set_num_at(st, count);
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, stval, is_end, count, count_consume)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, stval, is_end, count, count_consume)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, stval, is_end, count, count_consume);
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
                                  StatesValue & stval, bool is_end, unsigned count)
            {
                if ( ! st->out1 && ! st->out2 && is_end) {
                    return true;
                }
                if (st->out1 && impl(l, st->out1, stval, is_end, count, 0)) {
                    return true;
                }
                if (st->out2 && impl(l, st->out2, stval, is_end, count, 0)) {
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
                    if (add::push_next(l2, first->first, stval, is_end, count)) {
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
                else if (add::push_next(l2, first->first, stval, is_end, count)) {
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

    inline bool st_exact_search(StateParser & stparser, const char * s)
    {
        if ( stparser.empty() ) {
            return false;
        }

        StatesValue stval(stparser.states(), stparser.nb_capture());

// #ifdef DISPLAY_TRACE
//         display_states(stval, stparser.root());
// #endif

        st_step_range_list_t l1;
        add_first(l1, l1, stparser.root());
        if (l1.empty()) {
            return false;
        }
        utf8_consumer consumer(s);
        st_step_range_list_t l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid() && !(res = st_exact_step(l1, l2, stval, consumer.bumpc(), consumer, ++count))) {
#ifdef DISPLAY_TRACE
            std::cout << "\033[01;31mc: '" << utf8_char(consumer.getc()) << "'\033[0m\n";
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
                        StatesValue & stval, char_int c, utf8_consumer & consumer, unsigned count)
    {
        struct add {
            static bool impl(st_step_range_list_t & l, const State * st, StatesValue & stval,
                             unsigned count, unsigned count_consume) {
                if (st->is_finish()) {
                    return true;
                }
                if (stval.get_num_at(st) == count) {
                    return false;
                }
                stval.set_num_at(st, count);
                if (st->is_split()) {
                    if (st->out1 && impl(l, st->out1, stval, count, count_consume)) {
                        return true;
                    }
                    if (st->out2 && impl(l, st->out2, stval, count, count_consume)) {
                        return true;
                    }
                }
                else if (st->is_cap()) {
                    if (st->out1) {
                        return impl(l, st->out1, stval, count, count_consume);
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
                    if (add::impl(l2, first->first->out1, stval, count, r)) {
                        return true;
                    }
                }
                if (first->first->out2) {
                    if (add::impl(l2, first->first->out2, stval, count, r)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    inline bool st_search(StateParser & stparser, const char * s)
    {
        if ( stparser.empty()) {
            return false;
        }

        StatesValue stval(stparser.states(), stparser.nb_capture());

// #ifdef DISPLAY_TRACE
//         display_states(stval, stparser.root());
// #endif

        st_step_range_list_t lst;
        st_step_range_list_t l1;
        add_first(lst, l1, stparser.root());
        if (l1.empty() && lst.empty()) {
            return false;
        }
        utf8_consumer consumer(s);
        st_step_range_list_t l2;
        bool res = false;
        unsigned count = 1;
        while (consumer.valid()) {
#ifdef DISPLAY_TRACE
            std::cout << "\033[01;31mc: '" << utf8_char(consumer.getc()) << "'\033[0m\n";
#endif
            for (st_step_range_iterator_t first = lst.begin(), last = lst.end(); first != last; ++first) {
                if (stval.get_num_at(first->first) != count) {
                    l1.push_back(*first);
                }
            }
            if (l1.empty()) {
                return false;
            }
            l2.clear();
            if ((res = st_step(l1, l2, stval, consumer.bumpc(), consumer, ++count))) {
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
}  // namespace re

