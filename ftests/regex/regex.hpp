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
        struct Parser {
            const char * err;
            size_t pos_err;
            StateParser st_parser;

            Parser()
            : err(0)
            , pos_err(0)
            {}

            Parser(const char * s)
            : err(0)
            , pos_err(0)
            {
                this->st_parser.compile(s, &this->err, &this->pos_err);
            }

            void reset(const char * s)
            {
                this->err = 0;
                this->pos_err = 0;
                this->st_parser.clear();
                this->st_parser.compile(s, &this->err, &this->pos_err);
            }
        };
        Parser parser;
        StateMachine2 sm;

    public:
        typedef unsigned flag_t;
        static const flag_t DEFAULT_FLAG =      0;
        static const flag_t OPTIMIZE_MEMORY =   1 << 0;
        static const flag_t MINIMAL_MEMORY =    1 << 1;

        unsigned step_limit;

        Regex(unsigned step_limit = 10000)
        : parser()
        , sm(state_list_t(), NULL, 0)
        , step_limit(step_limit)
        {}

        Regex(const char * s, flag_t flags = DEFAULT_FLAG, unsigned step_limit = 10000)
        : parser(s)
        , sm(this->parser.st_parser.states(),
             this->parser.st_parser.root(),
             this->parser.st_parser.nb_capture(),
             flags,
             flags & MINIMAL_MEMORY)
        , step_limit(step_limit)
        {
            if (flags) {
                this->parser.st_parser.clear_and_shrink();
            }
        }

        void reset(const char * s, flag_t flags = DEFAULT_FLAG)
        {
            this->sm.~StateMachine2();
            this->parser.reset(s);
            new (&this->sm) StateMachine2(this->parser.st_parser.states(),
                                          this->parser.st_parser.root(),
                                          this->parser.st_parser.nb_capture(),
                                          flags,
                                          flags & MINIMAL_MEMORY);
            if (flags) {
                this->parser.st_parser.clear_and_shrink();
            }
        }

        ~Regex()
        {}

        unsigned mark_count() const
        {
            return this->sm.mark_count();
        }

        const char * message_error() const
        {
            return this->parser.err;
        }

        size_t position_error() const
        {
            return this->parser.pos_err;
        }

        typedef StateMachine2::range_matches range_matches;

        range_matches exact_match(const char * s, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches exact_match(const char * s, Tracer tracer, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit, tracer)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        range_matches match(const char * s, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches match(const char * s, Tracer tracer, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit, tracer)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        bool exact_search(const char * s)
        {
            return this->sm.exact_search(s, this->step_limit);
        }

        bool search(const char * s)
        {
            return this->sm.search(s, this->step_limit);
        }

        bool exact_search_with_matches(const char * s)
        {
            return this->sm.exact_search_with_trace(s, this->step_limit);
        }

        template<typename Tracer>
        bool exact_search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.exact_search_with_trace(s, tracer, this->step_limit);
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

        range_matches match_result(bool all = true) const
        {
            return this->sm.match_result(all);
        }

        void display() const
        {
            this->sm.display_states();
        }
    };
}

#endif
