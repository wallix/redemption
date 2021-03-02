/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "configs/config.hpp"
#include "utils/load_theme.hpp"
#include "utils/theme.hpp"

void load_theme(Theme& theme, Inifile& ini) noexcept
{
    theme.global.enable_theme = ini.get<cfg::theme::enable_theme>();
    if (!theme.global.enable_theme) {
        return;
    }

    auto to_bgr = [&ini](auto i) {
        ::configs::spec_types::rgb rgb = ini.get<decltype(i)>();
        return BGRColor(BGRasRGBColor(BGRColor(rgb.to_rrggbb())));
    };

    theme.global.bgcolor = to_bgr(cfg::theme::bgcolor());
    theme.global.fgcolor = to_bgr(cfg::theme::fgcolor());
    theme.global.separator_color = to_bgr(cfg::theme::separator_color());
    theme.global.focus_color = to_bgr(cfg::theme::focus_color());
    theme.global.error_color = to_bgr(cfg::theme::error_color());
    theme.global.logo_path = ini.get<cfg::theme::logo_path>();

    theme.edit.bgcolor = to_bgr(cfg::theme::edit_bgcolor());
    theme.edit.fgcolor = to_bgr(cfg::theme::edit_fgcolor());
    theme.edit.focus_color = to_bgr(cfg::theme::edit_focus_color());

    theme.tooltip.bgcolor = to_bgr(cfg::theme::tooltip_bgcolor());
    theme.tooltip.fgcolor = to_bgr(cfg::theme::tooltip_fgcolor());
    theme.tooltip.border_color = to_bgr(cfg::theme::tooltip_border_color());

    theme.selector_line1.bgcolor = to_bgr(cfg::theme::selector_line1_bgcolor());
    theme.selector_line1.fgcolor = to_bgr(cfg::theme::selector_line1_fgcolor());

    theme.selector_line2.bgcolor = to_bgr(cfg::theme::selector_line2_bgcolor());
    theme.selector_line2.fgcolor = to_bgr(cfg::theme::selector_line2_fgcolor());

    theme.selector_selected.bgcolor = to_bgr(cfg::theme::selector_selected_bgcolor());
    theme.selector_selected.fgcolor = to_bgr(cfg::theme::selector_selected_fgcolor());

    theme.selector_focus.bgcolor = to_bgr(cfg::theme::selector_focus_bgcolor());
    theme.selector_focus.fgcolor = to_bgr(cfg::theme::selector_focus_fgcolor());

    theme.selector_label.bgcolor = to_bgr(cfg::theme::selector_label_bgcolor());
    theme.selector_label.fgcolor = to_bgr(cfg::theme::selector_label_fgcolor());
}
