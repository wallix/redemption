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


#pragma once

#include "utils/colors.hpp"
#include "cfgloader.hpp"
#include "utils/parse.hpp"

struct Theme final
{
    struct {
        int bgcolor = DARK_BLUE_BIS;
        int fgcolor = WHITE;
        int separator_color = LIGHT_BLUE;
        int focus_color = WINBLUE;
        int error_color = YELLOW;
        bool logo = false;
        char logo_path[1024] {};
    } global;

    struct {
        int bgcolor = WHITE;
        int fgcolor = BLACK;
        int focus_color = WINBLUE;
    } edit;

    struct {
        int fgcolor = BLACK;
        int bgcolor = LIGHT_YELLOW;
        int border_color = BLACK;
    } tooltip;

    struct {
        int bgcolor = PALE_BLUE;
        int fgcolor = BLACK;
    } selector_line1;
    struct {
        int bgcolor = LIGHT_BLUE;
        int fgcolor = BLACK;
    } selector_line2;
    struct {
        int bgcolor = MEDIUM_BLUE;
        int fgcolor = WHITE;
    } selector_selected;
    struct {
        int bgcolor = WINBLUE;
        int fgcolor = WHITE;
    } selector_focus;
    struct {
        int bgcolor = MEDIUM_BLUE;
        int fgcolor = WHITE;
    } selector_label;

    void set_logo_path(const char * logopath) {
        strncpy(this->global.logo_path, logopath, sizeof(this->global.logo_path));
        this->global.logo_path[sizeof(this->global.logo_path) - 1] = 0;
    }
};


struct ThemeHolder final : public ConfigurationHolder
{
    ThemeHolder(Theme & theme)
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
                this->theme.global.logo = Parse(reinterpret_cast<const uint8_t *>(value)).bool_from_cstr();
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
