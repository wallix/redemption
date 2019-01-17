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

#include "capture/utils/match_finder.hpp"
#include "core/report_message_api.hpp"
#include "regex/regex.hpp"
#include "utils/log.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/pattutils.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/difftimeval.hpp"


namespace utils
{

bool MatchFinder::NamedRegex::search(const char * str)
{
    return this->is_exact_search ? this->regex.exact_search(str) : this->regex.search(str);
}


void MatchFinder::NamedRegexArray::resize(size_t newlen)
{
    this->regexes = std::make_unique<NamedRegex[]>(newlen);
    this->len = newlen;
}

void MatchFinder::NamedRegexArray::shrink(size_t newlen) noexcept
{
    assert(newlen <= this->len);
    if (!newlen) {
        this->regexes.reset();
    }
    this->len = newlen;
}

MatchFinder::NamedRegex const & MatchFinder::NamedRegexArray::operator[](std::size_t i) const noexcept
{
    assert(i < this->len);
    return this->regexes[i];
}

MatchFinder::NamedRegex & MatchFinder::NamedRegexArray::operator[](std::size_t i) noexcept
{
    assert(i < this->len);
    return this->regexes[i];
}

MatchFinder::NamedRegex * MatchFinder::NamedRegexArray::begin() const noexcept
{
    return this->regexes.get();
}

MatchFinder::NamedRegex * MatchFinder::NamedRegexArray::end() const noexcept
{
    return this->regexes.get() + this->len;
}

size_t MatchFinder::NamedRegexArray::size() const noexcept
{
    return this->len;
}

bool MatchFinder::NamedRegexArray::empty() const noexcept
{
    return !this->len;
}


/**
 * \param filters_list  filters separated by '\\x01' character
 * \see \a get_pattern_value
 * With \c conf_regex = KBD_INPUT, exact-content and exact-regex are respectively equivalent to content and regex
 */
void MatchFinder::configure_regexes(
    ConfigureRegexes conf_regex, const char * filters_list,
    NamedRegexArray & regexes_filter_ref, int verbose,
    bool is_capturing)
{
    if (!filters_list || !*filters_list) {
        return ;
    }

    std::string tmp_filters = filters_list;

    using Cat = PatternValue::Cat;
    struct Pattern { Cat cat; char const * filter; };

    Pattern  filters[64];
    unsigned filter_number = 0;

    for (auto rng : get_line(tmp_filters, string_pattern_separator)) {
        array_view_char av{rng.begin(), rng.end()};
        av.data()[av.size()] = '\0';

        if (verbose) {
            LOG(LOG_INFO, "filter=\"%s\"", av.data());
        }

        PatternValue const pattern_value = get_pattern_value(av);
        if (not pattern_value.pattern.empty() && (
            (pattern_value.is_ocr && conf_regex == ConfigureRegexes::OCR)
            || (pattern_value.is_kbd && conf_regex == ConfigureRegexes::KBD_INPUT)
        )) {
            filters[filter_number++] = {pattern_value.cat, pattern_value.pattern.data()};
            if (filter_number >= (sizeof(filters) / sizeof(filters[0]))) {
                break;
            }
        }
    }

    if (verbose) {
        LOG(LOG_INFO, "filter number=%u", filter_number);
    }

    if (filter_number) {
        std::string capturing_regex;
        regexes_filter_ref.resize(filter_number);
        NamedRegex * pregex = regexes_filter_ref.begin();
        for (unsigned i = 0; i < filter_number; i++) {
            auto & filter = filters[i];
            if (verbose) {
                LOG(LOG_INFO, "Regex=\"%s\"", filter.filter);
            }

            pregex->name = filter.filter;
            pregex->is_exact_search
                = (conf_regex == ConfigureRegexes::OCR
                && (filter.cat == Cat::is_exact_reg || filter.cat == Cat::is_exact_str));

            char const * c_str_filter = filter.filter;
            std::string reg_pattern;
            if (filter.cat == Cat::is_str || filter.cat == Cat::is_exact_str) {
                while (*c_str_filter) {
                    switch (*c_str_filter) {
                        case '{':
                        case '}':
                        case '[':
                        case ']':
                        case '(':
                        case ')':
                        case '|':
                        case '\\':
                        case '^':
                        case '$':
                        case '.':
                        case '?':
                        case '+':
                        case '*':
                            reg_pattern += '\\';
                    }
                    reg_pattern += *c_str_filter;
                    ++c_str_filter;
                }
                c_str_filter = reg_pattern.c_str();
            }

            if (is_capturing) {
                capturing_regex = '(';
                capturing_regex += c_str_filter;
                capturing_regex += ')';
                pregex->regex.reset(capturing_regex.c_str());
            }
            else {
                pregex->regex.reset(c_str_filter);
            }
            if (pregex->regex.message_error()) {
                // TODO notification that the regex is too complex for us
                LOG(LOG_ERR, "Regex: %s err %s at position %zu" , c_str_filter,
                    pregex->regex.message_error(), pregex->regex.position_error());
            }
            else {
                ++pregex;
            }
        }
        regexes_filter_ref.shrink(pregex - regexes_filter_ref.begin());
    }
}

void MatchFinder::report(
    ReportMessageApi & report_message, bool is_pattern_kill,
    ConfigureRegexes conf_regex, const char * pattern, const char * data)
{
    char message[4096];

    snprintf(message, sizeof(message), "$%s:%s|%s",
        ((conf_regex == ConfigureRegexes::OCR) ? "ocr" : "kbd" ), pattern, data);
    array_view_char(message).back() = '\0';

    auto pattern_detection_type = (is_pattern_kill ? "KILL_PATTERN_DETECTED" : "NOTIFY_PATTERN_DETECTED");
    auto info = key_qvalue_pairs({
        {"type", pattern_detection_type},
        {"pattern", message},
    });

    ArcsightLogInfo arc_info;
    arc_info.name = pattern_detection_type;
    arc_info.signatureID = ArcsightLogInfo::MATCH_FINDER;
    arc_info.message = info;

    report_message.log6(info, arc_info, tvtime());

    report_message.report(
        (is_pattern_kill ? "FINDPATTERN_KILL" : "FINDPATTERN_NOTIFY"),
        message);
}

} // namespace utils

