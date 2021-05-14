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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/widget/scroll.hpp"
#include "mod/internal/widget/flat_button.hpp"

namespace
{
    Dimension get_optimal_button_dim(const Font& font, bool is_horizontal)
    {
        Dimension dim = WidgetFlatButton::get_optimal_dim(1, font, is_horizontal ? "▶" : "▲", 3, 2);

        dim.w += 1;
        dim.h += 2;

        return dim;
    }
}

WidgetScrollBar::WidgetScrollBar(
    gdi::GraphicApi & drawable, Widget& parent,
    NotifyApi* notifier, bool horizontal,
    int group_id, BGRColor fgcolor, BGRColor bgcolor, BGRColor focuscolor,
    Font const & font, bool rail_style, int maxvalue)
: Widget(drawable, parent, notifier, group_id)
, horizontal(horizontal)
, fg_color(encode_color24()(fgcolor))
, bg_color(encode_color24()(bgcolor))
, focus_color(encode_color24()(focuscolor))
, font(font)
, max_value(maxvalue)
, event(horizontal ? NOTIFY_HSCROLL : NOTIFY_VSCROLL)
, button_width_or_height(this->horizontal
    ? get_optimal_button_dim(this->font, this->horizontal).w
    : get_optimal_button_dim(this->font, this->horizontal).h)
, rail_style(rail_style)
{}

void WidgetScrollBar::compute_step_value()
{
    this->step_value =
        (this->horizontal ?
            ((this->scroll_bar_rect.cx - cursor_button_rect.cx) / this->max_value) :
            ((this->scroll_bar_rect.cy - cursor_button_rect.cy) / this->max_value));
}

unsigned int WidgetScrollBar::get_current_value() const
{
    return static_cast<unsigned int>(this->current_value);
}

void WidgetScrollBar::set_current_value(unsigned int cv)
{
    this->current_value = static_cast<int>(
        std::min<int>(cv, this->max_value));
}

void WidgetScrollBar::set_max_value(unsigned int maxvalue)
{
    this->max_value = static_cast<int>(maxvalue);

    this->compute_step_value();
}

void WidgetScrollBar::update_cursor_button_rects()
{
    if (this->horizontal) {
        if (!this->cx()) {
            return;
        }

        this->cursor_button_rect.x  = this->x() + this->button_width_or_height - (this->rail_style ? 0 : 1) +
                                            (this->cx() - this->button_width_or_height * 2 + ((this->rail_style ? 0 : 1) * 2) - this->button_width_or_height) *
                                                this->current_value / this->max_value;
        this->cursor_button_rect.y  = this->y() + (this->rail_style ? 0 : 1);
        this->cursor_button_rect.cx = this->button_width_or_height;
        this->cursor_button_rect.cy = this->cy() - ((this->rail_style ? 0 : 1) * 2);
    }
    else {
        if (!this->cy()) {
            return;
        }

        this->cursor_button_rect.x  = this->x() + (this->rail_style ? 0 : 1);
        this->cursor_button_rect.y  = this->y() + this->button_width_or_height - (this->rail_style ? 0 : 1) +
                                            (this->cy() - this->button_width_or_height * 2 + ((this->rail_style ? 0 : 1) * 2) - this->button_width_or_height) *
                                                this->current_value / this->max_value;
        this->cursor_button_rect.cx = this->cx() - ((this->rail_style ? 0 : 1) * 2);
        this->cursor_button_rect.cy = this->button_width_or_height;
    }
}

void WidgetScrollBar::update_rects()
{
    if (this->horizontal) {
        this->left_or_top_button_rect.x  = this->x();
        this->left_or_top_button_rect.y  = this->y();
        this->left_or_top_button_rect.cx = this->button_width_or_height;
        this->left_or_top_button_rect.cy = this->cy();

        this->right_or_bottom_button_rect.x  = this->x() + this->cx() - this->button_width_or_height;
        this->right_or_bottom_button_rect.y  = this->y();
        this->right_or_bottom_button_rect.cx = this->button_width_or_height;
        this->right_or_bottom_button_rect.cy = this->cy();

        this->scroll_bar_rect.x  = this->x() + this->button_width_or_height;
        this->scroll_bar_rect.y  = this->y();
        this->scroll_bar_rect.cx = this->cx() - this->button_width_or_height * 2;
        this->scroll_bar_rect.cy = this->cy();
    }
    else {
        this->left_or_top_button_rect.x  = this->x();
        this->left_or_top_button_rect.y  = this->y();
        this->left_or_top_button_rect.cx = this->cx();
        this->left_or_top_button_rect.cy = this->button_width_or_height;

        this->right_or_bottom_button_rect.x  = this->x();
        this->right_or_bottom_button_rect.y  = this->y() + this->cy() - this->button_width_or_height;
        this->right_or_bottom_button_rect.cx = this->cx();
        this->right_or_bottom_button_rect.cy = this->button_width_or_height;

        this->scroll_bar_rect.x  = this->x();
        this->scroll_bar_rect.y  = this->y() + this->button_width_or_height;
        this->scroll_bar_rect.cx = this->cx();
        this->scroll_bar_rect.cy = this->cy() - this->button_width_or_height * 2;
    }

    this->update_cursor_button_rects();

    this->compute_step_value();
}

// Widget

void WidgetScrollBar::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();

        auto draw_button = [&](Rect const& rect, bool has_focus, char const* button){
            WidgetFlatButton::draw(rect_intersect, rect, this->drawable,
                false, (this->mouse_down && has_focus), button,
                this->fg_color, this->bg_color,
                this->focus_color, gdi::ColorCtx::depth24(),
                Rect(),
                WidgetFlatButton::State::Normal,
                (this->rail_style ? 0 : 2),
                this->font,
                (this->rail_style ? 2 : 0),
                (this->rail_style ? 1 : -1));
        };

        auto draw_cursor_button = [&](bool has_focus, char const* button, int xtext, int ytext){
            WidgetFlatButton::draw(rect_intersect, this->cursor_button_rect, this->drawable,
                false, (this->mouse_down && has_focus), button,
                this->fg_color, this->bg_color,
                this->focus_color, gdi::ColorCtx::depth24(),
                Rect(),
                WidgetFlatButton::State::Normal,
                (this->rail_style ? 0 : 1),
                this->font,
                xtext,
                ytext);
        };

        auto draw_rect = [&](RDPColor color, Rect const& rect){
            this->drawable.draw(
                RDPOpaqueRect(rect_intersect.intersect(rect), color),
                this->get_rect(),
                gdi::ColorCtx::depth24()
            );
        };

        if (this->horizontal) {
            draw_button(this->left_or_top_button_rect,
                        this->selected_button == BUTTON_LEFT_OR_TOP, "◀");
            draw_button(this->right_or_bottom_button_rect,
                        this->selected_button == BUTTON_RIGHT_OR_BOTTOM, "▶");

            draw_rect(this->bg_color, this->scroll_bar_rect);

            if (!this->rail_style) {
                auto rect = this->scroll_bar_rect;

                rect.cy = 1;
                draw_rect(this->fg_color, rect);

                rect.y += this->cy() - 1;
                draw_rect(this->fg_color, rect);

                draw_cursor_button(this->selected_button == BUTTON_CURSOR, "▤", 0, -1);
            }
            else {
                draw_rect(this->fg_color, this->cursor_button_rect);
            }
        }
        else {
            draw_button(this->left_or_top_button_rect,
                        this->selected_button == BUTTON_LEFT_OR_TOP, "▲");
            draw_button(this->right_or_bottom_button_rect,
                        this->selected_button == BUTTON_RIGHT_OR_BOTTOM, "▼");

            draw_rect(this->bg_color, this->scroll_bar_rect);

            if (!this->rail_style) {
                auto rect = this->scroll_bar_rect;

                rect.cx = 1;
                draw_rect(this->fg_color, rect);

                rect.x += this->cx() - 1;
                draw_rect(this->fg_color, rect);

                draw_cursor_button(this->selected_button == BUTTON_CURSOR, "▥", -1, 0);
            }
            else {
                draw_rect(this->fg_color, this->cursor_button_rect);
            }
        }

        this->drawable.end_update();
    }
}

void WidgetScrollBar::set_xy(int16_t x, int16_t y)
{
    Widget::set_xy(x, y);
    this->update_rects();
}

void WidgetScrollBar::set_wh(uint16_t w, uint16_t h)
{
    Widget::set_wh(w, h);
    this->update_rects();
}

Dimension WidgetScrollBar::get_optimal_dim() const
{
    return get_optimal_button_dim(this->font, this->horizontal);
}

// RdpInput
void WidgetScrollBar::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
        this->mouse_down = true;

        if (this->left_or_top_button_rect.contains_pt(x, y)) {
            this->selected_button = BUTTON_LEFT_OR_TOP;

            const int old_value = this->current_value;

            this->current_value -= this->step_value;

            if (this->current_value < 0) {
                this->current_value = 0;
            }

            if (old_value != this->current_value) {
                this->update_cursor_button_rects();

                this->send_notify(this->event);
            }
        }
        else if (this->cursor_button_rect.contains_pt(x, y)) {
            this->selected_button = BUTTON_CURSOR;

            this->old_mouse_x_or_y         = (this->horizontal ? x : y);
            this->old_cursor_button_x_or_y = (this->horizontal ? this->cursor_button_rect.x : this->cursor_button_rect.y);
        }
        if (this->right_or_bottom_button_rect.contains_pt(x, y)) {
            this->selected_button = BUTTON_RIGHT_OR_BOTTOM;

            const int old_value = this->current_value;

            this->current_value += this->step_value;

            if (this->current_value > this->max_value) {
                this->current_value = this->max_value;
            }

            if (old_value != this->current_value) {
                this->update_cursor_button_rects();

                this->send_notify(this->event);
            }
        }

        this->rdp_input_invalidate(this->get_rect());
    }
    else if (device_flags == MOUSE_FLAG_BUTTON1) {
        this->mouse_down               = false;
        this->selected_button          = BUTTON_NONE;
        this->old_mouse_x_or_y         = 0;
        this->old_cursor_button_x_or_y = 0;

        this->rdp_input_invalidate(this->get_rect());
    }
    else if (device_flags == MOUSE_FLAG_MOVE) {
        if (this->mouse_down && (BUTTON_CURSOR == this->selected_button)) {
            const int old_value = this->current_value;

            const int min_button_x_or_y = (this->horizontal ? this->x() : this->y()) + this->button_width_or_height - 1;
            const int max_button_x_or_y = min_button_x_or_y +
                                            ((this->horizontal ? this->cx() : this->cy()) - this->button_width_or_height * 2 + 2 - this->button_width_or_height);

            const int min_x_or_y = min_button_x_or_y + (this->old_mouse_x_or_y - this->old_cursor_button_x_or_y);
            const int max_x_or_y = max_button_x_or_y + (this->old_mouse_x_or_y - this->old_cursor_button_x_or_y);

            if ((this->horizontal ? x : y) < min_x_or_y) {
                this->current_value = 0;
            }
            else if ((this->horizontal ? x : y) >= max_x_or_y) {
                this->current_value = this->max_value;
            }
            else {
                this->current_value =
                    ((this->horizontal ? x : y) - min_x_or_y) * this->max_value / (max_x_or_y - min_x_or_y);
            }

            if (old_value != this->current_value) {
                this->update_cursor_button_rects();

                this->send_notify(this->event);
            }

            this->rdp_input_invalidate(this->get_rect());
        }
    }
    else {
        this->Widget::rdp_input_mouse(device_flags, x, y, keymap);
    }
}
