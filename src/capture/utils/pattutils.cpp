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

#include "capture/utils/pattutils.hpp"
#include "utils/log.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/strutils.hpp"

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
 * With \c pattern_rule = KBD_INPUT, exact-content and exact-regex are respectively equivalent to content and regex
 */
CapturePattern parse_capture_pattern(chars_view const pattern_rule)
{
    using PatternType = CapturePattern::PatternType;
    using Filters = CapturePattern::Filters;

    struct IsWordSeparator
    {
        bool operator == (char c) const {
            return c == '-' || c == ',';
        }
    };

    chars_view av { ltrim(pattern_rule.begin(), pattern_rule.end()), pattern_rule.end() };

    if (not av.empty() && av.front() == '$') {
        Filters filters {};
        PatternType pattern_type {};
        char const* start_pattern = "";

        auto end_option_list = std::find(av.begin()+1, av.end(), ':');
        if (end_option_list != av.end() && end_option_list+1 != av.end()) {
            chars_view options(av.begin()+1, end_option_list);
            bool is_exact = false;
            for (auto token_av : split_with(options, IsWordSeparator{})) {
                auto token = token_av.as<std::string_view>();

                using namespace std::string_view_literals;

                bool token_is_exact = (token == "exact"sv);

                if (token_is_exact) {
                    // nothing
                }
                else if (token == "ocr"sv) {
                    filters.is_ocr = true;
                    if (is_exact) {
                        pattern_type = PatternType::exact_str;
                    }
                }
                else if (token == "kbd"sv) {
                    filters.is_kbd = true;
                    if (is_exact) {
                        pattern_type = PatternType::exact_str;
                    }
                }
                else if (token == "regex"sv) {
                    pattern_type = is_exact ? PatternType::exact_reg : PatternType::reg;
                }
                else if (token == "content"sv) {
                    pattern_type = is_exact ? PatternType::exact_str : PatternType::str;
                }
                else {
                    LOG(LOG_WARNING, "Unknown filter option=\"%.*s\" at char %d in \"%.*s\"",
                        int(token.size()), token.begin(),
                        int(token.begin() - pattern_rule.begin()),
                        int(pattern_rule.size()), pattern_rule.begin());
                    return CapturePattern{};
                }

                is_exact = token_is_exact;
            }

            if (is_exact) {
                pattern_type = PatternType::exact_str;
            }
            if (not filters.is_ocr && not filters.is_kbd) {
                filters.is_ocr = true;
            }

            start_pattern = end_option_list+1;
        }

        return CapturePattern(filters, pattern_type, {start_pattern, av.end()});
    }

    return CapturePattern(
        Filters{true, false},
        PatternType::reg,
        av
    );
}

namespace
{
    bool contains_pattern(chars_view soh_separated_patterns, unsigned types)
    {
        for (auto pattern : get_lines(soh_separated_patterns, capture_pattern_separator)) {
            CapturePattern const pattern_value = parse_capture_pattern(pattern);
            if (not pattern_value.pattern().empty()
             && (static_cast<unsigned>(pattern_value.capture_type()) & types)
            ) {
                return true;
            }
        }
        return false;
    }
} // namespace

bool contains_kbd_pattern(chars_view soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, unsigned(CapturePattern::CaptureType::kbd));
}

bool contains_kbd_or_ocr_pattern(chars_view soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, unsigned(CapturePattern::CaptureType::kbd)
                                                  | unsigned(CapturePattern::CaptureType::ocr));
}

bool contains_ocr_pattern(chars_view soh_separated_patterns)
{
    return contains_pattern(soh_separated_patterns, unsigned(CapturePattern::CaptureType::ocr));
}
