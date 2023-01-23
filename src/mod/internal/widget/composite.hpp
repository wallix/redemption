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

class CompositeContainer
{
public:
    virtual ~CompositeContainer() = default;


    virtual int add(Widget * w) = 0;
    virtual void remove(const Widget * w) = 0;

    [[nodiscard]] virtual Widget * get(int index) const = 0;

    virtual int get_first() = 0;
    virtual int get_last() = 0;

    virtual int get_previous(int index) = 0;
    virtual int get_next(int index) = 0;

    virtual int find(const Widget * w) = 0;

    virtual void clear() = 0;
};


class CompositeArray : public CompositeContainer
{
    enum {
        MAX_CHILDREN_COUNT = 256
    };

    Widget * child_table[MAX_CHILDREN_COUNT] {};
    size_t   children_count = 0;

public:
    CompositeArray();

    int add(Widget * w) override;
    void remove(const Widget * w) override;

    [[nodiscard]] Widget * get(int index) const override;

    int get_first() override;
    int get_last() override;

    int get_previous(int index) override;
    int get_next(int index) override;

    int find(const Widget * w) override;

    void clear()  override;
};  // class CompositeArray


class WidgetParent : public Widget
{
    Widget * pressed;

    Color bg_color;

    uint16_t old_mouse_x = 0;
    uint16_t old_mouse_y = 0;

protected:
    CompositeContainer * impl;

public:
    Widget * current_focus;

public:
    enum class HasFocus : bool
    {
        No,
        Yes,
    };

    WidgetParent(gdi::GraphicApi & drawable, Focusable focusable);

    ~WidgetParent() override;

    void set_widget_focus(Widget & new_focused, int reason);

    void focus(int reason) override;
    void blur() override;

    virtual void add_widget(Widget & w, HasFocus has_focus = HasFocus::No);
    virtual void remove_widget(Widget & w);
    virtual int  find_widget(Widget & w);
    virtual void clear();

    virtual void invalidate_children(Rect clip);

    virtual void draw_inner_free(Rect clip, Color bg_color);

    [[nodiscard]] virtual Color get_bg_color() const
    {
        return this->bg_color;
    }

    virtual void set_bg_color(Color color)
    {
        this->bg_color = color;
    }

    void move_xy(int16_t x, int16_t y) override;

    void move_children_xy(int16_t x, int16_t y);

    bool next_focus() override;
    bool previous_focus() override;

    Widget * widget_at_pos(int16_t x, int16_t y) override;

    void rdp_input_invalidate(Rect clip) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override;

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void init_focus() override;

protected:
    Widget * get_next_focus(Widget * w);
    Widget * get_previous_focus(Widget * w);
};

struct WidgetComposite : WidgetParent
{
    WidgetComposite(gdi::GraphicApi& gd)
    : WidgetParent(gd, Widget::Focusable::Yes)
    {
        impl = &composite_array;
    }

private:
    CompositeArray composite_array;
};
