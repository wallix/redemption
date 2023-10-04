/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <string>
#include <string_view>

namespace cfg_generators
{

inline void json_quoted(std::string& buffer, std::string_view s)
{
    for (char c : s) {
        if ('"' == c || '\\' == c) {
            buffer += '\\';
            buffer += c;
        }
        else if ('\n' == c) {
            buffer += '\\';
            buffer += 'n';
        }
        else {
            buffer += c;
        }
    }
}

}
