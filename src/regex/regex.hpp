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

#include "regex_automate.hpp"
#include "regex_parser.hpp"

namespace re
{
    class Regex
    {
        struct Parser {
            const char * err = nullptr;
            size_t pos_err = 0;
            StateParser st_parser;

            Parser() = default;

            explicit Parser(const char * s)
            {
                this->st_parser.compile(s, &this->err, &this->pos_err);
            }

            void reset(const char * s)
            {
                this->err = nullptr;
                this->pos_err = 0;
                this->st_parser.clear();
                this->st_parser.compile(s, &this->err, &this->pos_err);
            }
        };
        Parser parser;
        StateMachine2 sm;
        std::size_t pos{0};

    public:
        using flag_t = unsigned int;
        static const flag_t DEFAULT_FLAG =      0;
        static const flag_t OPTIMIZE_MEMORY =   1 << 0;
        static const flag_t MINIMAL_MEMORY =    1 << 1;

        unsigned step_limit;

        explicit Regex(unsigned step_limit = 10000)
        : parser()
        , sm(state_list_t(), nullptr, 0)
        , 
         step_limit(step_limit)
        {}

        explicit Regex(const char * s, flag_t flags = DEFAULT_FLAG, unsigned step_limit = 10000)
        : parser(s)
        , sm(this->parser.st_parser.states(),
             this->parser.st_parser.root(),
             this->parser.st_parser.nb_capture(),
             flags,
             flags & MINIMAL_MEMORY)
        , pos(0)
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
        = default;

        unsigned mark_count() const
        {
            return this->sm.mark_count();
        }

        //The index at which to start the next match.
        size_t last_index() const
        {
            return this->pos;
        }

        const char * message_error() const
        {
            return this->parser.err;
        }

        size_t position_error() const
        {
            return this->parser.pos_err;
        }

        using range_matches = StateMachine2::range_matches;

        range_matches exact_match(const char * s, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit, &this->pos)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches exact_match(const char * s, Tracer tracer, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.exact_search_with_trace(s, this->step_limit, tracer, &this->pos)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        range_matches match(const char * s, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit, &this->pos)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        template<typename Tracer>
        range_matches match(const char * s, Tracer tracer, bool all_match = true)
        {
            range_matches ret;
            if (this->sm.search_with_trace(s, this->step_limit, tracer, &this->pos)) {
                this->sm.append_match_result(ret, all_match);
            }
            return ret;
        }

        bool exact_search(const char * s)
        {
            return this->sm.exact_search(s, this->step_limit, &this->pos);
        }

        bool search(const char * s)
        {
            return this->sm.search(s, this->step_limit, &this->pos);
        }

        bool exact_search_with_matches(const char * s)
        {
            return this->sm.exact_search_with_trace(s, this->step_limit, &this->pos);
        }

        template<typename Tracer>
        bool exact_search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.exact_search_with_trace(s, tracer, this->step_limit, &this->pos);
        }

        bool search_with_matches(const char * s)
        {
            return this->sm.search_with_trace(s, this->step_limit, &this->pos);
        }

        template<typename Tracer>
        bool search_with_matches(const char * s, Tracer tracer)
        {
            return this->sm.search_with_trace(s, this->step_limit, tracer, &this->pos);
        }

        enum match_state_t {
            match_fail = StateMachine2::match_fail,
            match_success = StateMachine2::match_success,
            match_undetermined = StateMachine2::match_undetermined
        };

    private:
        template<bool exact>
        class BasicPartOfText
        {
            Regex * re = nullptr;
            unsigned res = 0;

        public:
            BasicPartOfText() = default;

            BasicPartOfText(Regex & re, const char * s)
            : re(&re)
            , res(re.sm.part_of_text_start(s, &re.pos))
            {}

            bool valid() const {
                return this->re;
            }

            unsigned state() const
            {
                return this->res;
            }

            unsigned next(const char * s)
            {
                if (exact) {
                    this->res = this->re->sm.part_of_text_exact_search(
                        s, this->re->step_limit, &this->re->pos
                    );
                }
                else {
                    this->res = this->re->sm.part_of_text_search(
                        s, this->re->step_limit, &this->re->pos
                    );
                }
                return this->res;
            }

            bool finish()
            {
                if (this->res == match_undetermined) {
                  this->res = this->re->sm.part_of_text_finish(&this->re->pos);
                }
                return this->res;
            }
        };


        template<bool exact, typename Tracer>
        class BasicPartOfTextWithCapture
        {
        protected:
            Regex & re;
            Tracer tracer;
            unsigned res;

        public:
            BasicPartOfTextWithCapture(Regex & re, const char * s, Tracer tracer = Tracer())
            : re(re)
            , tracer(tracer)
            , res(re.sm.part_of_text_start_with_trace(s, this->tracer, &re.pos))
            {}

            unsigned state() const
            {
                return this->res;
            }

            unsigned next(const char * s)
            {
                if (exact) {
                    this->res = this->re.sm
                    .part_of_text_exact_search_with_trace(s, this->re.step_limit,
                                                          this->tracer, &this->re.pos);
                }
                else {
                    this->res = this->re.sm
                    .part_of_text_search_with_trace(s, this->re.step_limit,
                                                    this->tracer, &this->re.pos);
                }
                return this->res;
            }

            bool finish()
            {
                this->res = this->re.sm.part_of_text_finish_with_trace(this->tracer,
                                                                       &this->re.pos);
                return this->res;
            }
        };

    public:
        using ExactPartOfText = BasicPartOfText<true>;
        using PartOfText = BasicPartOfText<false>;

        template<typename Tracer>
        struct ExactPartOfTextWithCapture : BasicPartOfTextWithCapture<true, Tracer>
        {
            ExactPartOfTextWithCapture(Regex & re, const char * s, Tracer tracer = Tracer())
            : BasicPartOfTextWithCapture<true, Tracer>(re, s, tracer)
            {}
        };

        template<typename Tracer>
        struct PartOfTextWithCapture : BasicPartOfTextWithCapture<false, Tracer>
        {
            PartOfTextWithCapture(Regex & re, const char * s, Tracer tracer = Tracer())
            : BasicPartOfTextWithCapture<false, Tracer>(re, s, tracer)
            {}
        };

        //class PartOfTextTracer
        //{
        //    StateMachine2::DefaultMatchTracer sm_tracer;
        //    std::vector<std::string> matches;
        //    StateMachine2 & sm;
        //    const char * s;

        //public:
        //    PartOfTextTracer(StateMachine2 & sm)
        //    : sm_tracer(sm)
        //    , matches(sm.node_count())
        //    , sm(sm)
        //    {}

        //    unsigned new_id(unsigned old_id)
        //    {
        //        const unsigned new_id = this->sm_tracer.new_id(old_id);
        //        std::copy(this->matches.begin() + old_id,
        //                  this->matches.begin() + old_id + this->sm.mark_count(),
        //                  this->matches.begin() + new_id);
        //        return new_id;
        //    }

        //    bool open(unsigned idx, const char * s, unsigned num_cap) const
        //    {
        //        (void)idx;
        //        (void)s;
        //        (void)num_cap;
        //        return true;
        //    }

        //    bool close(unsigned idx, const char * s, unsigned num_cap)
        //    {
        //        this->matches[idx * this->sm.mark_count() + num_cap].append(this->s, s - this->s);
        //        return true;
        //    }

        //    void fail(unsigned idx)
        //    {
        //        this->sm_tracer.fail(idx);
        //        for (const unsigned last = idx + this->sm.mark_count(); idx != last; ++idx) {
        //            this->matches[idx].clear();
        //        }
        //    }

        //    void good(unsigned idx) const
        //    {
        //        this->sm_tracer.good(idx);
        //    }
        //};

        //typedef StateMachine2::DefaultMatchTracer DefaultTracer;


        ExactPartOfText part_of_text_exact_search(bool str_is_empty)
        {
           return ExactPartOfText(*this, str_is_empty ? "" : " ");
        }

        PartOfText part_of_text_search(bool str_is_empty)
        {
            return PartOfText(*this, str_is_empty ? "" : " ");
        }

//         ExactPartOfTextWithCapture<DefaultTracer>
//         part_of_text_exact_search_with_capture(bool str_is_empty)
//         {
//             return ExactPartOfTextWithCapture<DefaultTracer>(*this,
//                                                              str_is_empty ? "" : " ",
//                                                              DefaultTracer(this->sm)
//                                                             );
//         }
//
//         template<typename Tracer>
//         ExactPartOfTextWithCapture<Tracer>
//         part_of_text_exact_search_with_capture(bool str_is_empty, Tracer tracer)
//         {
//             return ExactPartOfTextWithCapture<Tracer>(*this, str_is_empty ? "" : " ", tracer);
//         }
//
//         PartOfTextWithCapture<DefaultTracer>
//         part_of_text_search_with_capture(bool str_is_empty)
//         {
//             return PartOfTextWithCapture<DefaultTracer>(*this, str_is_empty ? "" : " ",
//                                                         DefaultTracer(this->sm));
//         }
//
//         template<typename Tracer>
//         PartOfTextWithCapture<Tracer>
//         part_of_text_search_with_capture(bool str_is_empty, Tracer tracer)
//         {
//             return PartOfTextWithCapture<Tracer>(*this, str_is_empty ? "" : " ", tracer);
//         }

        range_matches match_result(bool all = true) const
        {
            return this->sm.match_result(all);
        }

        range_matches & match_result(range_matches & ranges, bool all = true) const
        {
            this->sm.append_match_result(ranges, all);
            return ranges;
        }

        void display() const
        {
            this->sm.display_states();
        }
    };
}  // namespace re

