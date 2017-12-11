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
*   Copyright (C) Wallix 2012-2015
*   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier,
*              Jonathan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "regex/regex.hpp"

#include <memory>
#include <string>

class ReportMessageApi;

namespace utils {

using std::size_t;

struct MatchFinder
{
    struct NamedRegex
    {
        re::Regex   regex;
        std::string name;
        bool is_exact_search;

        bool search(char const * str);
    };

    class NamedRegexArray
    {
        std::unique_ptr<NamedRegex[]> regexes;
        size_t                        len = 0;

    public:
        using iterator = NamedRegex *;

        void resize(size_t newlen);
        void shrink(size_t newlen) noexcept;
        NamedRegex const & operator[](std::size_t i) const noexcept;
        NamedRegex & operator[](std::size_t i) noexcept;
        NamedRegex * begin() const noexcept;
        NamedRegex * end() const noexcept;
        size_t size() const noexcept;
        bool empty() const noexcept;
    };

    enum ConfigureRegexes
    {
        OCR       = 0
      , KBD_INPUT = 1
    };

    /**
     * \param filters_list  filters separated by '\\x01' character
     * \see \a get_pattern_value
     * With \c conf_regex = KBD_INPUT, exact-content and exact-regex are respectively equivalent to content and regex
     */
    static void configure_regexes(
        ConfigureRegexes conf_regex, const char * filters_list,
        NamedRegexArray & regexes_filter_ref, int verbose,
        bool is_capturing = false);

    static void report(
        ReportMessageApi & report_message, bool is_pattern_kill,
        ConfigureRegexes conf_regex, const char * pattern, const char * data);
};

}
