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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <cassert>

#include "utils/sugar/array_view.hpp"
#include "capture/utils/match_finder.hpp"
#include "acl/auth_api.hpp"


class PatternsChecker
{
    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;
    auth_api * authentifier;

public:
    PatternsChecker(
        auth_api * authentifier,
        const char * const filters_kill,
        const char * const filters_notify,
        int verbose = 0
    )
    : authentifier(authentifier)
    {
        if (this->authentifier && filters_kill) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
                filters_kill, this->regexes_filter_kill, verbose);
        }

        if (this->authentifier && filters_notify) {
            utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
                filters_notify, this->regexes_filter_notify, verbose);
        }
    }

    bool contains_pattern() const {
        return !this->regexes_filter_kill.empty() || !this->regexes_filter_notify.empty();
    }

    void operator()(array_view_const_char str) {
        assert(str.data() && str.size());
        this->check_filter(this->regexes_filter_kill, str.data());
        this->check_filter(this->regexes_filter_notify, str.data());
    }

private:
    void check_filter(utils::MatchFinder::NamedRegexArray & regexes_filter, char const * str) {
        if (regexes_filter.begin()) {
            utils::MatchFinder::NamedRegexArray::iterator first = regexes_filter.begin();
            utils::MatchFinder::NamedRegexArray::iterator last = regexes_filter.end();
            for (; first != last; ++first) {
                if (first->regex.search(str)) {
                    utils::MatchFinder::report(*this->authentifier,
                        &regexes_filter == &this->regexes_filter_kill, // pattern_kill = FINDPATTERN_KILL
                        utils::MatchFinder::ConfigureRegexes::OCR,
                        first->name.c_str(), str);
                }
            }
        }
    }
};
