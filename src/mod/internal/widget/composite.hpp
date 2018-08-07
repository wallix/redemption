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
class NotifyApi;

void fill_region(gdi::GraphicApi & drawable, const SubRegion & region, BGRColor bg_color);

class CompositeContainer
{
public:
    virtual ~CompositeContainer() = default;

    enum { invalid_iterator = 0 };

    using iterator = void *;

    virtual iterator add(Widget * w) = 0;
    virtual void remove(const Widget * w) = 0;

    virtual Widget * get(iterator iter) const = 0;

    virtual iterator get_first() = 0;
    virtual iterator get_last() = 0;

    virtual iterator get_previous(iterator iter) = 0;
    virtual iterator get_next(iterator iter) = 0;

    virtual iterator find(const Widget * w) = 0;

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

    iterator add(Widget * w) override;
    void remove(const Widget * w) override;

    Widget * get(iterator iter) const override;

    iterator get_first() override;
    iterator get_last() override;

    iterator get_previous(iterator iter) override;
    iterator get_next(iterator iter) override;

    iterator find(const Widget * w) override;

    void clear()  override;
};  // class CompositeArray


class WidgetParent : public Widget
{
    Widget * pressed;

    BGRColor bg_color;

    int old_mouse_x = 0;
    int old_mouse_y = 0;

protected:
    CompositeContainer * impl;

public:
    Widget * current_focus;

    WidgetParent(gdi::GraphicApi & drawable, Widget & parent,
                 NotifyApi * notifier, int group_id = 0);

    ~WidgetParent() override;

    void set_widget_focus(Widget * new_focused, int reason);

    void focus(int reason) override;
    void blur() override;

    Widget * get_next_focus(Widget * w);
    Widget * get_previous_focus(Widget * w);

    virtual void add_widget(Widget * w);
    virtual void remove_widget(Widget * w);
    virtual void clear();

    virtual void invalidate_children(Rect clip);

    virtual void refresh_children(Rect clip);

    virtual void draw_inner_free(Rect clip, BGRColor bg_color);

    virtual BGRColor get_bg_color() const
    {
        return this->bg_color;
    }

    virtual void set_bg_color(BGRColor color)
    {
        this->bg_color = color;
    }

    void move_xy(int16_t x, int16_t y) override;

    void move_children_xy(int16_t x, int16_t y);

    bool next_focus() override;
    bool previous_focus() override;

    Widget * widget_at_pos(int16_t x, int16_t y) override;

    void rdp_input_invalidate(Rect clip) override;
    void refresh(Rect clip) override;

    void rdp_input_scancode(long param1, long param2, long param3,
                            long param4, Keymap2 * keymap) override;

    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;
};


// WidgetComposite is a WidgetParent and use Delegation to an implementation of CompositeInterface
class WidgetComposite: public WidgetParent
{
    CompositeArray composite_array;

public:
    WidgetComposite(gdi::GraphicApi & drawable, Widget & parent,
                    NotifyApi * notifier, int group_id = 0)
    : WidgetParent(drawable, parent, notifier, group_id)
    {
        this->impl = & composite_array;
    }
};
