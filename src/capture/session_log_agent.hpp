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

#include <string>
#include <algorithm>

#include "utils/log.hpp"
#include "gdi/capture_probe_api.hpp"
#include "capture/session_meta.hpp"


using std::begin;
using std::end;

namespace {
    template<std::size_t N>
    inline bool cstr_equal(char const (&s1)[N], array_view_const_char s2) {
        return N - 1 == s2.size() && std::equal(s1, s1 + N - 1, begin(s2));
    }

    template<std::size_t N>
    void str_append(std::string & s, char const (&s2)[N]) {
        s.append(s2, N-1);
    }

    inline void str_append(std::string & s, array_view_const_char const & s2) {
        s.append(s2.data(), s2.size());
    }

    template<class... S>
    void str_append(std::string & s, S const & ... strings) {
        (void)std::initializer_list<int>{
            (str_append(s, strings), 0)...
        };
    }
}

inline void agent_data_extractor(std::string & line, array_view_const_char data)
{
    using Av = array_view_const_char;

    auto find = [](Av & s, char c) {
        auto p = std::find(begin(s), end(s), c);
        return p == end(s) ? nullptr : p;
    };

    auto separator = find(data, '=');

    if (separator) {
        auto right = [](Av s, char const * pos) { return Av(begin(s), pos - begin(s)); };
        auto left = [](Av s, char const * pos) { return Av(pos + 1, begin(s) - (pos + 1)); };

        auto order = left(data, separator);
        auto parameters = right(data, separator);

        auto line_with_1_var = [&](Av var1) {
            str_append(
                line,
                "type=\"", order, "\" ",
                Av(var1.data(), var1.size()-1), "=\"", parameters, "\""
            );
        };
        auto line_with_2_var = [&](Av var1, Av var2) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                str_append(
                    line,
                    "type=\"", order, "\" ",
                    Av(var1.data(), var1.size()-1), "=\"", left(parameters, subitem_separator), "\" ",
                    Av(var2.data(), var2.size()-1), "=\"", right(parameters, subitem_separator), "\""
                );
            }
        };
        auto line_with_3_var = [&](Av var1, Av var2, Av var3) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                auto text = left(parameters, subitem_separator);
                auto remaining = right(parameters, subitem_separator);
                if (auto subitem_separator2 = find(remaining, '\x01')) {
                    str_append(
                        line,
                        "type=\"", order, "\" ",
                        Av(var1.data(), var1.size()-1), "=\"", text, "\" ",
                        Av(var2.data(), var2.size()-1), "=\"", left(remaining, subitem_separator2), "\" ",
                        Av(var3.data(), var3.size()-1), "=\"", right(remaining, subitem_separator2), "\""
                    );
                }
            }
        };

        // TODO used string_id: switch (sid(order)) { case "string"_sid: ... }
        if (cstr_equal("PASSWORD_TEXT_BOX_GET_FOCUS", order)
         || cstr_equal("UAC_PROMPT_BECOME_VISIBLE", order)) {
            line_with_1_var("status");
        }
        else if (cstr_equal("INPUT_LANGUAGE", order)) {
            line_with_2_var("identifier", "display_name");
        }
        else if (cstr_equal("NEW_PROCESS", order)
              || cstr_equal("COMPLETED_PROCESS", order)) {
            line_with_1_var("command_line");
        }
        else if (cstr_equal("OUTBOUND_CONNECTION_BLOCKED", order)) {
            line_with_2_var("rule", "application_name");
        }
        else if (cstr_equal("FOREGROUND_WINDOW_CHANGED", order)) {
            line_with_3_var("windows", "class", "command_line");
        }
        else if (cstr_equal("BUTTON_CLICKED", order)) {
            line_with_2_var("windows", "button");
        }
        else if (cstr_equal("EDIT_CHANGED", order)) {
            line_with_2_var("windows", "edit");
        }
        else {
            LOG(LOG_WARNING,
                "MetaDataExtractor(): Unexpected order. Data=\"%*s\"",
                int(data.size()), data.data());
            return;
        }
    }

    if (line.empty()) {
        LOG(LOG_WARNING,
            "MetaDataExtractor(): Invalid data format. Data=\"%*s\"",
            int(data.size()), data.data());
        return;
    }
}

class SessionLogAgent : public gdi::CaptureProbeApi
{
    std::string line;
    SessionMeta & session_meta;

public:
    SessionLogAgent(SessionMeta & session_meta)
    : session_meta(session_meta)
    {}

    void session_update(const timeval& now, array_view_const_char message) override {
        line.clear();
        agent_data_extractor(this->line, message);
        if (!this->line.empty()) {
            this->session_meta.send_line(now.tv_sec, this->line);
        }
    }

    void possible_active_window_change() override {
    }
};
