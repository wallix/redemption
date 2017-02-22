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

#include "acl/auth_api.hpp"
#include "regex/regex.hpp"
#include "utils/log.hpp"

#include <memory>
#include <cstring>

namespace utils {

using std::size_t;

struct MatchFinder
{
public:
    struct NamedRegex {
        re::Regex   regex;
        std::string name;
    };

    class NamedRegexArray {
        std::unique_ptr<NamedRegex[]> regexes;
        size_t                        len = 0;

    public:
        typedef NamedRegex * iterator;

        void resize(size_t newlen)
        {
            this->regexes.reset(new NamedRegex[newlen]);
            this->len = newlen;
        }

        void shrink(size_t newlen)
        {
            assert(newlen <= this->len);
            if (!newlen) {
                this->regexes.reset();
            }
            this->len = newlen;
        }

        NamedRegex * begin() const
        { return this->regexes.get(); }

        NamedRegex * end() const
        { return this->regexes.get() + this->len; }

        size_t size() const
        { return this->len; }

        bool empty() const
        { return !this->len; }
    };

    enum ConfigureRegexes {
          OCR         = 0
        , KBD_INPUT   = 1
    };

    static void configure_regexes(ConfigureRegexes conf_regex, const char * filters_list,
                                  NamedRegexArray & regexes_filter_ref, int verbose,
                                  bool is_capturing = false)
    {
        if (!filters_list || !*filters_list) {
            return ;
        }

        char * tmp_filters = new(std::nothrow) char[strlen(filters_list) + 1];
        if (!tmp_filters) {
            return ; // insufficient memory
        }

        std::unique_ptr<char[]> auto_free(tmp_filters);

        strcpy(tmp_filters, filters_list);

        char     * separator;
        char     * filters[64];
        unsigned   filter_number = 0;

        if (verbose) {
            LOG(LOG_INFO, "filters=\"%s\"", tmp_filters);
        }

        while (*tmp_filters) {
            if ((*tmp_filters == '\x01') || (*tmp_filters == '\t') || (*tmp_filters == ' ')) {
                tmp_filters++;
                continue;
            }

            separator = strchr(tmp_filters, '\x01');
            if (separator) {
                *separator = 0;
            }

            if (verbose) {
                LOG(LOG_INFO, "filter=\"%s\"", tmp_filters);
            }

            if (((conf_regex == ConfigureRegexes::OCR) && ((*tmp_filters != '$') ||
                                                           (strcasestr(tmp_filters, "$ocr:") == tmp_filters) ||
                                                           (strcasestr(tmp_filters, "$kbd-ocr:") == tmp_filters) ||
                                                           (strcasestr(tmp_filters, "$ocr-kbd:") == tmp_filters))) ||
                ((conf_regex == ConfigureRegexes::KBD_INPUT) && ((strcasestr(tmp_filters, "$kbd:") == tmp_filters) ||
                                                                 (strcasestr(tmp_filters, "$kbd-ocr:") == tmp_filters) ||
                                                                 (strcasestr(tmp_filters, "$ocr-kbd:") == tmp_filters)))) {
                if (((conf_regex == ConfigureRegexes::OCR) && (*tmp_filters == '$')) ||
                    (conf_regex == ConfigureRegexes::KBD_INPUT)) {
                    if (*(tmp_filters + 4) == ':') {
                        tmp_filters += 5;   // strlen("$ocr:") or strlen("$kdb:")
                    }
                    else {
                        REDASSERT(*(tmp_filters + 8) == ':');
                        tmp_filters += 9;   // strlen("$kbd-ocr:") or strlen("$ocr-kbd:")
                    }
                }

                filters[filter_number] = tmp_filters;
                filter_number++;
                if (filter_number >= (sizeof(filters) / sizeof(filters[0]))) {
                    break;
                }
            }

            if (!separator) {
                break;
            }

            tmp_filters = separator + 1;
        }

        if (verbose) {
            LOG(LOG_INFO, "filter number=%d", filter_number);
        }

        if (filter_number) {
            std::string capturing_regex;
            regexes_filter_ref.resize(filter_number);
            NamedRegex * pregex = regexes_filter_ref.begin();
            for (unsigned i = 0; i < filter_number; i++) {
                if (verbose) {
                    LOG(LOG_INFO, "Regex=\"%s\"", filters[i]);
                }
                pregex->name = filters[i];
                if (is_capturing) {
                    capturing_regex = '(';
                    capturing_regex += filters[i];
                    capturing_regex += ')';
                    pregex->regex.reset(capturing_regex.c_str());
                }
                else {
                    pregex->regex.reset(filters[i]);
                }
                if (pregex->regex.message_error()) {
                    // TODO notification that the regex is too complex for us
                    LOG(LOG_ERR, "Regex: %s err %s at position %zu" , filters[i],
                        pregex->regex.message_error(), pregex->regex.position_error());
                }
                else {
                    ++pregex;
                }
            }
            regexes_filter_ref.shrink(pregex - regexes_filter_ref.begin());
        }
    }

    static void report(auth_api & authentifier, bool is_pattern_kill,
        ConfigureRegexes conf_regex, const char * pattern, const char * data) {
        char message[4096];

        snprintf(message, sizeof(message), "$%s:%s|%s",
            ((conf_regex == ConfigureRegexes::OCR) ? "ocr" : "kbd" ), pattern, data);

        std::string extra = "pattern='";
        extra += message;
        extra += "'";
        authentifier.log4(false,
            (is_pattern_kill ? "KILL_PATTERN_DETECTED" : "NOTIFY_PATTERN_DETECTED"),
            extra.c_str());

        authentifier.report(
            (is_pattern_kill ? "FINDPATTERN_KILL" : "FINDPATTERN_NOTIFY"),
            message);
    }
};

}

