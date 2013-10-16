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

#ifndef REDEMPTION_FTESTS_REGEX_REGEX_HPP
#define REDEMPTION_FTESTS_REGEX_REGEX_HPP

#include "regex_automate.hpp"
#include "regex_parser.hpp"

namespace re {

    class Regex
    {
        const char * err;
        size_t pos_err;
        StatesWrapper stw;
        StateMachine2 sm;
        unsigned step_limit;

    public:
        Regex(const char * s, unsigned step_limit = 10000)
        : err(0)
        , pos_err(0)
        , stw(str2reg(s, &this->err, &this->pos_err))
        , sm(this->stw)
        , step_limit(step_limit)
        {}

        Regex(StateBase * st, unsigned step_limit = 10000)
        : err(0)
        , pos_err(0)
        , stw(st)
        , sm(this->stw)
        , step_limit(step_limit)
        {}

        void reset(const char * s)
        {
            this->sm.~StateMachine2();
            this->stw.~StatesWrapper();
            this->err = 0;
            new (&this->stw) StatesWrapper(str2reg(s, &this->err, &this->pos_err));
            new (&this->sm) StateMachine2(this->stw);
        }

        ~Regex()
        {}

        const char * message_error() const
        {
            return this->err;
        }

        size_t position_error() const
        {
            return this->pos_err;
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
            display_state(this->stw);
        }
    };
}

#endif
