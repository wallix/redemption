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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan, Raphael Zhou
 */

#pragma once

#include "utils/colors.hpp"
#include "mod/internal/widget/widget.hpp"

class SubRegion;

void fill_region(gdi::GraphicApi & drawable, const SubRegion & region, Widget::Color bg_color);


class CompositeArray
{
public:
    CompositeArray();
    ~CompositeArray();

    void add(Widget & w);
    void remove(Widget const & w);

    Widget** find(Widget const & w);

    void clear();

    Widget** begin();
    Widget** end();

public:
    int count = 0;
    int capacity;
    Widget ** p;
    Widget * fixed_table[16];
};  // class CompositeArray


class WidgetComposite : public Widget
{
    Widget * pressed;

    Color bg_color;

    uint16_t old_mouse_x = 0;
    uint16_t old_mouse_y = 0;

    CompositeArray widgets;

public:
    Widget * current_focus;

public:
    enum class HasFocus : bool
    {
        No,
        Yes,
    };

    WidgetComposite(gdi::GraphicApi & drawable, Focusable focusable);

    ~WidgetComposite() override;

    void add_widget(Widget & w, HasFocus has_focus = HasFocus::No);
    void remove_widget(Widget & w);
    bool contains_widget(Widget & w);
    void clear_widget();

    void set_widget_focus(Widget & new_focused, int reason);

    void focus(int reason) override;
    void blur() override;

    virtual void invalidate_children(Rect clip);

    virtual void draw_inner_free(Rect clip, Color bg_color);

    void move_xy(int16_t x, int16_t y) override;

    bool next_focus() override;
    bool previous_focus() override;

    Widget * widget_at_pos(int16_t x, int16_t y) override;

    void rdp_input_invalidate(Rect clip) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void init_focus() override;

    Color get_bg_color() const
    {
        return this->bg_color;
    }

protected:
    void move_children_xy(int16_t x, int16_t y);

    void set_bg_color(Color color)
    {
        this->bg_color = color;
    }

    Widget * get_next_focus(Widget * w);
    Widget * get_previous_focus(Widget * w);
};
