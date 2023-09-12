/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/RDP/rdp_performance_flags.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/split.hpp"
#include "utils/strutils.hpp"

#include <charconv>


inline char const* parse_performance_flags(RdpPerformanceFlags& x, chars_view flags)
{
    struct P { uint32_t flag; bool enable; std::string_view name; };
    static constexpr P pairs[] {
        {0x001, true, "wallpaper"},
        {0x004, true, "menu_animations"},
        {0x008, true, "theme"},
        {0x020, true, "mouse_cursor_shadows"},
        {0x040, true, "cursor_blinking"},
        {0x080, false, "font_smoothing"},
        {0x100, false, "desktop_composition"},
    };

    uint32_t force_present = 0;
    uint32_t force_not_present = 0;

    for (chars_view flag : get_lines(flags, ',')) {
        flag = trim(flag);
        if (flag.empty()) {
            continue;
        }

        bool enable = true;

        if (flag[0] == '-' || flag[0] == '+') {
            enable = (flag[0] == '+');
            flag = flag.drop_front(1);
            if (flag.empty()) {
                continue;
            }
        }

        // string parser
        if (flag[0] < '0' || '9' < flag[0]) {
            for (auto p : pairs) {
                if (p.name == flag.as<std::string_view>()) {
                    if (enable == p.enable) {
                        force_not_present |= p.flag;
                    }
                    else {
                        force_present |= p.flag;
                    }
                    goto next_elem;
                }
            }
        }
        // numeric parser
        else {
            auto& present = enable ? force_present : force_not_present;

            std::from_chars_result r;
            if (flag.size() > 2 && flag[0] == '0' && flag[1] == 'x') {
                flag = flag.drop_front(2);
                r = std::from_chars(flag.begin(), flag.end(), present, 16);
            }
            else {
                r = std::from_chars(flag.begin(), flag.end(), present, 10);
            }

            if (r.ec == std::errc() && r.ptr == flag.end()) {
                goto next_elem;
            }
        }

        return
            "Cannot parse performance flags because it's an invalid value."
            " Expected are wallpaper, menu_animations, theme, mouse_cursor_shadows, cursor_blinking, font_smoothing and desktop_composition."
        ;

        next_elem:;
    }

    x.force_present = force_present;
    x.force_not_present = force_not_present;

    return nullptr;
}
