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

#include "configs/io.hpp"
#include "utils/load_theme.hpp"
#include "utils/theme.hpp"
#include "utils/cfgloader.hpp"
#include "utils/fileutils.hpp"
#include "core/app_path.hpp"

#include <cstring> // strcasecmp


// should be as sorted_log_id_string in agent_data_extractor
static BGRColor color_from_cstr(const char * str)
{
    BGRColor bgr;

    if (false) {} /*NOLINT*/
# define ELSE_COLOR(COLOR_NAME) else if (0 == strcasecmp(#COLOR_NAME, str)) { bgr = COLOR_NAME; }
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
    else if ((*str == '0') && (*(str + 1) == 'x')){
        bgr = BGRasRGBColor(BGRColor(strtol(str + 2, nullptr, 16)));
    }
    else {
        bgr = BGRasRGBColor(BGRColor(strtol(str, nullptr, 10)));
    }

    return bgr;
}


// 1, yes, on, true
static bool bool_from_cstr(const char * value)
{
    bool val;
    auto err = configs::parse(val, configs::spec_type<bool>(), array_view{value, strlen(value)});
    return err ? false : val;
}


struct ThemeHolder final : public ConfigurationHolder
{
    explicit ThemeHolder(Theme & theme)
    : theme(theme)
    {}

    void set_value(const char * context, const char * key, const char * value) override
    {
        if (0 == strcmp(context, "global")) {
            if (0 == strcmp(key, "bgcolor")){
                this->theme.global.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")){
                this->theme.global.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "separator_color")){
                this->theme.global.separator_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")){
                this->theme.global.focus_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "error_color")){
                this->theme.global.error_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "logo")){
                this->theme.global.logo = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "edit")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->theme.edit.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->theme.edit.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")) {
                this->theme.edit.focus_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "tooltip")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->theme.tooltip.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->theme.tooltip.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "border_color")) {
                this->theme.tooltip.border_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "selector")) {
            if (0 == strcmp(key, "line1_fgcolor")) {
                this->theme.selector_line1.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line1_bgcolor")) {
                this->theme.selector_line1.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_fgcolor")) {
                this->theme.selector_line2.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_bgcolor")) {
                this->theme.selector_line2.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_bgcolor")) {
                this->theme.selector_selected.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_fgcolor")) {
                this->theme.selector_selected.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_bgcolor")) {
                this->theme.selector_focus.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_fgcolor")) {
                this->theme.selector_focus.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_bgcolor")) {
                this->theme.selector_label.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_fgcolor")) {
                this->theme.selector_label.fgcolor = color_from_cstr(value);
            }
        }
    }

private:
    Theme & theme;
};

void load_theme(Theme& theme, chars_view theme_name)
{
    // load theme
    char const* cfg_path = app_path(AppPath::Cfg).c_str();

    {
        char theme_path[1024] = {};
        snprintf(theme_path, 1024, "%s/themes/%.*s/" THEME_INI,
            cfg_path, int(theme_name.size()), theme_name.data());
        theme_path[sizeof(theme_path) - 1] = 0;

        configuration_load(ThemeHolder(theme), theme_path);
    }

    if (theme.global.logo) {
        char logo_path[1024] = {};
        snprintf(logo_path, 1024, "%s/themes/%.*s/" LOGO_PNG,
            cfg_path, int(theme_name.size()), theme_name.data());
        logo_path[sizeof(logo_path) - 1] = 0;
        if (!file_exist(logo_path)) {
            snprintf(logo_path, 1024, "%s/themes/%.*s/" LOGO_BMP,
                cfg_path, int(theme_name.size()), theme_name.data());
            logo_path[sizeof(logo_path) - 1] = 0;
            if (!file_exist(logo_path)) {
                theme.global.logo = false;
                return ;
            }
        }
        theme.global.logo_path = logo_path;
    }
}
