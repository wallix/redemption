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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Meng Tan
 */

#include <cstring>
#include <string_view>

#include "configs/config.hpp"
#include "utils/load_theme.hpp"
#include "utils/theme.hpp"

namespace
{
    inline std::pair<bool, long int>
    to_long_int_base16_or_10(std::string_view str_view) noexcept
    {
        if (str_view.empty()) {
            return {};
        }

        auto converter = [](const char *str, int base)
        {
            long int value = 0;
            bool success = false;
            char *endptr = nullptr;

            errno = 0;
            value = strtol(str, &endptr, base);
            success = !(endptr == str || *endptr != '\0' || errno);
            return std::pair<bool, long int>{ success, value };
        };

        std::size_t size = str_view.size();

        if (size > 2 && str_view[0] == '0' && str_view[1] == 'x')
        {
            return converter(str_view.data(), 16);
        }

        if (size > 1 && str_view[0] == '#')
        {
            return converter(str_view.data() + 1, 16);
        }

        return converter(str_view.data(), 10);
    }

    BGRColor color_from_cstr(std::string_view str_view) noexcept
    {
        BGRColor bgr;

        if (false); /*NOLINT*/
#define ELSE_COLOR(COLOR_NAME)                        \
        else if (::strncasecmp(#COLOR_NAME,           \
                               str_view.data(),       \
                               str_view.size()) == 0) \
        {                                             \
            bgr = COLOR_NAME;                         \
        }
        ELSE_COLOR(BLACK)
        ELSE_COLOR(GREY)
        ELSE_COLOR(MEDIUM_GREY)
        ELSE_COLOR(DARK_GREY)
        ELSE_COLOR(ANTHRACITE)
        ELSE_COLOR(WHITE)
        ELSE_COLOR(BLUE)
        ELSE_COLOR(DARK_BLUE)
        ELSE_COLOR(CYAN)
        ELSE_COLOR(DARK_BLUE_WIN)
        ELSE_COLOR(DARK_BLUE_BIS)
        ELSE_COLOR(MEDIUM_BLUE)
        ELSE_COLOR(PALE_BLUE)
        ELSE_COLOR(LIGHT_BLUE)
        ELSE_COLOR(WINBLUE)
        ELSE_COLOR(RED)
        ELSE_COLOR(DARK_RED)
        ELSE_COLOR(MEDIUM_RED)
        ELSE_COLOR(PINK)
        ELSE_COLOR(GREEN)
        ELSE_COLOR(WABGREEN)
        ELSE_COLOR(WABGREEN_BIS)
        ELSE_COLOR(DARK_WABGREEN)
        ELSE_COLOR(INV_DARK_WABGREEN)
        ELSE_COLOR(DARK_GREEN)
        ELSE_COLOR(INV_DARK_GREEN)
        ELSE_COLOR(LIGHT_GREEN)
        ELSE_COLOR(INV_LIGHT_GREEN)
        ELSE_COLOR(PALE_GREEN)
        ELSE_COLOR(INV_PALE_GREEN)
        ELSE_COLOR(MEDIUM_GREEN)
        ELSE_COLOR(INV_MEDIUM_GREEN)
        ELSE_COLOR(YELLOW)
        ELSE_COLOR(LIGHT_YELLOW)
        ELSE_COLOR(ORANGE)
        ELSE_COLOR(LIGHT_ORANGE)
        ELSE_COLOR(PALE_ORANGE)
        ELSE_COLOR(BROWN)
#undef ELSE_COLOR
        else if (auto res = to_long_int_base16_or_10(str_view); res.first)
        {
            bgr = BGRasRGBColor(BGRColor(res.second));
        }
        return bgr;
    }
}

void load_theme(Theme& theme, Inifile& ini) noexcept
{
    if (!ini.get<cfg::theme::enable_theme>()) {
        return;
    }

    theme.global.bgcolor =
        color_from_cstr(ini.get<cfg::theme::bgcolor>());
    theme.global.fgcolor =
        color_from_cstr(ini.get<cfg::theme::fgcolor>());
    theme.global.separator_color =
        color_from_cstr(ini.get<cfg::theme::separator_color>());
    theme.global.focus_color =
        color_from_cstr(ini.get<cfg::theme::focus_color>());
    theme.global.error_color =
        color_from_cstr(ini.get<cfg::theme::error_color>());
    theme.global.logo =
        ini.get<cfg::theme::logo>();
    theme.global.logo_path =
        ini.get<cfg::theme::logo_path>();

    theme.edit.bgcolor =
        color_from_cstr(ini.get<cfg::theme::edit_bgcolor>());
    theme.edit.fgcolor =
        color_from_cstr(ini.get<cfg::theme::edit_fgcolor>());
    theme.edit.focus_color =
        color_from_cstr(ini.get<cfg::theme::edit_focus_color>());

    theme.tooltip.bgcolor =
        color_from_cstr(ini.get<cfg::theme::tooltip_bgcolor>());
    theme.tooltip.fgcolor =
        color_from_cstr(ini.get<cfg::theme::tooltip_fgcolor>());
    theme.tooltip.border_color =
        color_from_cstr(ini.get<cfg::theme::tooltip_border_color>());

    theme.selector_line1.bgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_line1_bgcolor>());
    theme.selector_line1.fgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_line1_fgcolor>());

    theme.selector_line2.bgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_line2_bgcolor>());
    theme.selector_line2.fgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_line2_fgcolor>());

    theme.selector_selected.bgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_selected_bgcolor>());
    theme.selector_selected.fgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_selected_fgcolor>());

    theme.selector_focus.bgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_focus_bgcolor>());
    theme.selector_focus.fgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_focus_fgcolor>());

    theme.selector_label.bgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_label_bgcolor>());
    theme.selector_label.fgcolor =
        color_from_cstr(ini.get<cfg::theme::selector_label_fgcolor>());
}
