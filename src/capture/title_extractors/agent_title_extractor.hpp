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

#include <cstdio>
#include <string>
#include <algorithm>

#include "title_extractor_api.hpp"
#include "acl/auth_api.hpp"


class AgentTitleExtractor : public TitleExtractorApi
{
    std::string title;
    bool has_title = false;

public:
    AgentTitleExtractor() = default;

    array_view_const_char extract_title() override {
        if (this->has_title) {
            this->has_title = false;
            return this->title;
        }
        return {};
    }

    void session_update(array_view_const_char message) {
        constexpr auto prefix_window_changed = cstr_array_view("FOREGROUND_WINDOW_CHANGED=");
        if (prefix_window_changed.size() < message.size()
         && std::equal(prefix_window_changed.begin(), prefix_window_changed.end(), message.begin())) {
            auto begin_title = message.begin() + prefix_window_changed.size();
            auto pos_separator = std::find(begin_title, message.end(), '\x01');
            if (pos_separator != message.end() && pos_separator != begin_title) {
                this->title.assign(begin_title, pos_separator);
                this->has_title = true;
                //LOG(LOG_INFO, "AgentExtractor::session_update: agent_data=\"%s\"", begin_title);
            }
        }
    }
};
