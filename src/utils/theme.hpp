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
#include "utils/parser.hpp"


struct Theme final : public ConfigurationHolder
{
    struct {
        int bgcolor;
        int fgcolor;
        int separator_color;
        int focus_color;
        int error_color;
        bool logo;
        char logo_path[1024];
    } global;

    struct {
        int bgcolor;
        int fgcolor;
        int focus_color;
    } edit;

    struct {
        int fgcolor;
        int bgcolor;
        int border_color;
    } tooltip;

    struct {
        int bgcolor;
        int fgcolor;
    } selector_line1;
    struct {
        int bgcolor;
        int fgcolor;
    } selector_line2;
    struct {
        int bgcolor;
        int fgcolor;
    } selector_selected;
    struct {
        int bgcolor;
        int fgcolor;
    } selector_focus;
    struct {
        int bgcolor;
        int fgcolor;
    } selector_label;

    Theme() {
        this->init();
    }

    Theme(Theme const & other)
    : global(other.global)
    , edit(other.edit)
    , tooltip(other.tooltip)
    , selector_line1(other.selector_line1)
    , selector_line2(other.selector_line2)
    , selector_selected(other.selector_selected)
    , selector_focus(other.selector_focus)
    , selector_label(other.selector_label)
    {}

    Theme & operator = (Theme const & other)
    {
        this->global = other.global;
        this->edit = other.edit;
        this->tooltip = other.tooltip;
        this->selector_line1 = other.selector_line1;
        this->selector_line2 = other.selector_line2;
        this->selector_selected = other.selector_selected;
        this->selector_focus = other.selector_focus;
        this->selector_label = other.selector_label;
        return *this;
    }

    void init() {
        this->global.logo = false;
        this->global.logo_path[0] = 0;
        this->global.bgcolor = DARK_BLUE_BIS;
        this->global.fgcolor = WHITE;
        this->global.separator_color = LIGHT_BLUE;
        this->global.focus_color = WINBLUE;
        this->global.error_color = YELLOW;

        this->edit.bgcolor = WHITE;
        this->edit.fgcolor = BLACK;
        this->edit.focus_color = WINBLUE;

        this->tooltip.fgcolor = BLACK;
        this->tooltip.bgcolor = LIGHT_YELLOW;
        this->tooltip.border_color = BLACK;

        this->selector_line1.fgcolor = BLACK;
        this->selector_line1.bgcolor = PALE_BLUE;
        this->selector_line2.fgcolor = BLACK;
        this->selector_line2.bgcolor = LIGHT_BLUE;

        this->selector_selected.bgcolor = MEDIUM_BLUE;
        this->selector_selected.fgcolor = WHITE;

        this->selector_focus.bgcolor = WINBLUE;
        this->selector_focus.fgcolor = WHITE;

        this->selector_label.bgcolor = MEDIUM_BLUE;
        this->selector_label.fgcolor = WHITE;
    }

    void set_value(const char * context, const char * key, const char * value) override
    {
        if (0 == strcmp(context, "global")) {
            if (0 == strcmp(key, "bgcolor")){
                this->global.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")){
                this->global.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "separator_color")){
                this->global.separator_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")){
                this->global.focus_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "error_color")){
                this->global.error_color = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "logo")){
                this->global.logo = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "edit")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->edit.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->edit.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_color")) {
                this->edit.focus_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "tooltip")) {
            if (0 == strcmp(key, "bgcolor")) {
                this->tooltip.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "fgcolor")) {
                this->tooltip.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "border_color")) {
                this->tooltip.border_color = color_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "selector")) {
            if (0 == strcmp(key, "line1_fgcolor")) {
                this->selector_line1.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line1_bgcolor")) {
                this->selector_line1.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_fgcolor")) {
                this->selector_line2.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "line2_bgcolor")) {
                this->selector_line2.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_bgcolor")) {
                this->selector_selected.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "selected_fgcolor")) {
                this->selector_selected.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_bgcolor")) {
                this->selector_focus.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "focus_fgcolor")) {
                this->selector_focus.fgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_bgcolor")) {
                this->selector_label.bgcolor = color_from_cstr(value);
            }
            else if (0 == strcmp(key, "label_fgcolor")) {
                this->selector_label.fgcolor = color_from_cstr(value);
            }
        }
    }

    void set_logo_path(const char * logopath) {
        strncpy(this->global.logo_path, logopath, sizeof(this->global.logo_path));
        this->global.logo_path[sizeof(this->global.logo_path) - 1] = 0;
    }
};


