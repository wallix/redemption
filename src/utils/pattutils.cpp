/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#include "utils/pattutils.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/splitter.hpp"

#include <algorithm>

/**
 * filter format:
 * \code{regex}
 *  option = "ocr" | "kbd" | "content" | "regex" | "exact-content" | "exact-regex"
 *  option_separator = "," | "-"
 *  filter
 *      = \s* pattern
 *      | \s* "$:" pattern
 *      | \s* "$" option ( option_separator option )* ":" pattern
 * \endcode
 *
 * With \c conf_regex = KBD_INPUT, exact-content and exact-regex are respectively equivalent to content and regex
 */
PatternValue get_pattern_value(array_view_const_char const pattern_rule)
{
    using Cat = PatternValue::Cat;

    struct IsWordSeparator
    {
        bool operator == (char c) const {
            return c == '-' || c == ',';
        }
    };

    PatternValue pattern_value;
    constexpr PatternValue empty_pattern_value {};

    auto av = array_view_const_char{
        ltrim(pattern_rule.begin(), pattern_rule.end()), pattern_rule.end()};

    if (not av.empty() && av.front() == '$') {
        auto end_option_list = std::find(av.begin()+1, av.end(), ':');
        if (end_option_list != av.end() && end_option_list+1 != av.end()) {
            array_view_const_char options(av.begin()+1, end_option_list);
            bool is_exact = false;
            for (auto token : get_split(options, IsWordSeparator{})) {
                auto eq = [](range<char const*> b, array_view_const_char a) {
                    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
                };

                if (eq(token, cstr_array_view("exact"))) {
                    is_exact = true;
                }
                else {
                    if (eq(token, cstr_array_view("ocr"))) {
                        pattern_value.is_ocr = true;
                        if (is_exact) {
                            pattern_value.cat = Cat::is_exact_str;
                        }
                    }
                    else if (eq(token, cstr_array_view("kbd"))) {
                        pattern_value.is_kbd = true;
                        if (is_exact) {
                            pattern_value.cat = Cat::is_exact_str;
                        }
                    }
                    else if (eq(token, cstr_array_view("regex"))) {
                        pattern_value.cat = is_exact ? Cat::is_exact_reg : Cat::is_reg;
                    }
                    else if (eq(token, cstr_array_view("content"))) {
                        pattern_value.cat = is_exact ? Cat::is_exact_str : Cat::is_str;
                    }
                    else {
                        LOG(LOG_WARNING, "Unknown filter option=\"%.*s\" at char %d in \"%.*s\"",
                            int(token.size()), token.begin(),
                            int(token.begin() - pattern_rule.begin()),
                            int(pattern_rule.size()), pattern_rule.begin());
                        return empty_pattern_value;
                    }
                    is_exact = false;
                }
            }
            if (is_exact) {
                pattern_value.cat = Cat::is_exact_str;
            }
            if (not pattern_value.is_ocr && not pattern_value.is_kbd) {
                pattern_value.is_ocr = true;
            }

            pattern_value.pattern = {end_option_list+1, av.end()};
        }
    }
    else {
        pattern_value.pattern = av;
        pattern_value.is_ocr = true;
    }

    return pattern_value;
}

namespace
{
    bool contains_pattern(const char * soh_separated_patterns, bool check_kbd, bool check_ocr)
    {
        for (auto rng : get_line(soh_separated_patterns, string_pattern_separator)) {
            PatternValue const pattern_value = get_pattern_value({rng.begin(), rng.end()});
            if (not pattern_value.pattern.empty() && (
                (check_kbd && pattern_value.is_kbd)
            || (check_ocr && pattern_value.is_ocr)
            )) {
                return true;
            }
        }
        return false;
    }
}

bool contains_kbd_pattern(const char * soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, true, false);
}

bool contains_kbd_or_ocr_pattern(const char * soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, true, true);
}

bool contains_ocr_pattern(const char * soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, false, true);
}
