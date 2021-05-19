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
 *   Copyright (C) Wallix 2010-2021
 *   Author(s): Proxies Team
 */

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/font.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/widget/vertical_scroll_text.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "utils/utf.hpp"
#include "keyboard/keymap2.hpp"

namespace
{
    constexpr char const* top_button_char = "▲";
    constexpr char const* cursor_button_char = "▥";
    constexpr char const* bottom_button_char = "▼";

    Dimension get_optimal_button_dim(const Font& font)
    {
        UTF8toUnicodeIterator unicode_iter(top_button_char);
        auto const& glyph = font.glyph_or_unknown(*unicode_iter);
        return Dimension(glyph.width + 8, glyph.height + 12);
    }
}

WidgetVerticalScrollText::WidgetVerticalScrollText(
    gdi::GraphicApi& drawable, Widget& parent,
    NotifyApi* notifier, int group_id, std::string text,
    Color fgcolor, Color bgcolor, Color focuscolor,
    Font const & font, uint16_t xtext, uint16_t ytext)
: Widget(drawable, parent, notifier, group_id)
, fg_color(fgcolor)
, bg_color(bgcolor)
, focus_color(focuscolor)
, font(font)
, x_text(xtext)
, y_text(ytext)
, button_dim(get_optimal_button_dim(this->font))
, text(std::move(text))
{
    this->tab_flag = IGNORE_TAB;
    this->focus_flag = IGNORE_FOCUS;
}

void WidgetVerticalScrollText::set_xy(int16_t x, int16_t y)
{
    Widget::set_xy(x, y);
}

void WidgetVerticalScrollText::set_wh(uint16_t w, uint16_t h)
{
    Widget::set_wh(w, h);

    this->has_scroll = false;

    if (this->text.empty()) {
        return ;
    }

    uint16_t const cx = this->cx() - this->x_text * 2;
    uint16_t const cy = this->cy();
    uint16_t const glyph_cy = this->font.max_height() + this->y_text;

    bool const force_scroll = (cx / 4) * (cy / glyph_cy)
                              < this->text.size() / 4 /* worst case: 4 bytes by character */;

    if (!force_scroll) {
        this->line_metrics = gdi::MultiLineTextMetrics(this->font, this->text.c_str(), cx);
    }

    // show scroll bar
    if (force_scroll || cy < this->line_metrics.lines().size() * glyph_cy) {
        this->has_scroll = true;
        uint16_t const new_cx = cx - this->button_dim.w;
        this->line_metrics = gdi::MultiLineTextMetrics(this->font, this->text.c_str(), new_cx);

        const int text_h = int(this->line_metrics.lines().size() * glyph_cy - this->y_text);
        const int total_scroll_h = std::max(cy - this->button_dim.h * 2, 1);

        this->page_h = std::max(cy / glyph_cy - 1, 1) * glyph_cy;
        this->total_h = text_h - this->page_h;
        this->cursor_button_h = std::max(uint16_t(this->page_h * total_scroll_h / text_h),
                                         this->button_dim.h);
        this->scroll_h = std::max(total_scroll_h - this->cursor_button_h, 1);
        this->cursor_button_y = int16_t(this->button_dim.h - 1);
        this->current_y = 0;
    }
}

Dimension WidgetVerticalScrollText::get_optimal_dim() const
{
    return Dimension(
        uint16_t(this->line_metrics.max_width() + this->x_text * 2),
        uint16_t((this->font.max_height() + this->y_text) * this->line_metrics.lines().size()
                 + this->y_text)
    );
}

void WidgetVerticalScrollText::_update_cursor_button_y()
{
    this->cursor_button_y = this->scroll_h * this->current_y / this->total_h
                          + this->button_dim.h;
}

void WidgetVerticalScrollText::scroll_down()
{
    if (!this->has_scroll) {
        return ;
    }
    this->_scroll_down();
}

void WidgetVerticalScrollText::scroll_up()
{
    if (!this->has_scroll) {
        return ;
    }
    this->_scroll_up();
}

void WidgetVerticalScrollText::_scroll_down()
{
    const auto old_y = this->current_y;
    const auto new_y = std::min(this->current_y + this->page_h, this->total_h);
    if (old_y != new_y) {
        this->current_y = new_y;
        this->_update_cursor_button_y();
        this->rdp_input_invalidate(this->get_rect());
    }
}

void WidgetVerticalScrollText::_scroll_up()
{
    const auto old_y = this->current_y;
    const auto new_y = std::max(this->current_y - this->page_h, 0);
    if (old_y != new_y) {
        this->current_y = new_y;
        this->_update_cursor_button_y();
        this->rdp_input_invalidate(this->get_rect());
    }
}

void WidgetVerticalScrollText::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    if (!this->has_scroll) {
        this->Widget::rdp_input_mouse(device_flags, x, y, keymap);
        return;
    }

    auto update_scroll_bar = [this]{
        this->rdp_input_invalidate(Rect(
            this->x() + this->cx() - this->button_dim.w,
            this->y(),
            this->button_dim.w,
            this->cy()
        ));
    };

    if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
        auto in_range = [](int x, int16_t rx, uint16_t rcx){
            return rx <= x && x < rx + rcx;
        };

        if (!in_range(x, this->eright() - this->button_dim.w * 2, this->cx())) {
            // outside
        }
        // cursor
        else if (in_range(y, this->y() + this->cursor_button_y, this->cursor_button_h)) {
            this->selected_button = ButtonType::Cursor;

            this->mouse_start_y = y;
            this->mouse_y = this->cursor_button_y - this->button_dim.w;

            update_scroll_bar();
        }
        // top
        else if (y < this->y() + this->cursor_button_y) {
            this->selected_button = ButtonType::Top;
            this->_scroll_up();
        }
        // bottom
        else if (y > this->y() + this->cursor_button_y + this->button_dim.h) {
            this->selected_button = ButtonType::Bottom;
            this->_scroll_down();
        }
    }
    else if (device_flags == MOUSE_FLAG_BUTTON1) {
        if (bool(this->selected_button)) {
            this->selected_button = ButtonType::None;
            update_scroll_bar();
        }
    }
    else if (device_flags == MOUSE_FLAG_MOVE) {
        if (ButtonType::Cursor == this->selected_button) {
            auto const delta = y - this->mouse_start_y;
            auto const cursor_y = this->mouse_y + delta;
            auto new_y = cursor_y * this->total_h / this->scroll_h;
            bool update = false;

            if (new_y <= 0) {
                new_y = 0;
                update = new_y != this->current_y;
            }
            else if (new_y >= this->total_h) {
                new_y = this->total_h;
                update = new_y != this->current_y;
            }
            else if (new_y != this->current_y) {
                update = std::abs(new_y - current_y) >= this->font.max_height();
            }

            if (update) {
                this->current_y = new_y;
                this->_update_cursor_button_y();
                this->rdp_input_invalidate(this->get_rect());
            }
        }
    }
    else if (device_flags & MOUSE_FLAG_WHEEL) {
        // auto delta = device_flags & 0xff;
        if (device_flags & MOUSE_FLAG_WHEEL_NEGATIVE) {
            this->_scroll_down();
        }
        else {
            this->_scroll_up();
        }
    }
    else {
        this->Widget::rdp_input_mouse(device_flags, x, y, keymap);
    }
}

void WidgetVerticalScrollText::rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2* keymap)
{
    if (!this->has_scroll) {
        Widget::rdp_input_scancode(param1, param2, param3, param4, keymap);
        return ;
    }

    if (keymap->nb_kevent_available() > 0) {
        REDEMPTION_DIAGNOSTIC_PUSH()
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wswitch-enum")
        switch (keymap->top_kevent()){
            case Keymap2::KEVENT_LEFT_ARROW:
            case Keymap2::KEVENT_UP_ARROW:
            case Keymap2::KEVENT_PGUP:
                keymap->get_kevent();
                this->_scroll_up();
                break;

            case Keymap2::KEVENT_RIGHT_ARROW:
            case Keymap2::KEVENT_DOWN_ARROW:
            case Keymap2::KEVENT_PGDOWN:
                keymap->get_kevent();
                this->_scroll_down();
                break;

            default:
                Widget::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
        }
        REDEMPTION_DIAGNOSTIC_POP()
    }
}

void WidgetVerticalScrollText::rdp_input_invalidate(Rect clip)
{
    auto const rect = this->get_rect();
    Rect rect_intersect = clip.intersect(rect);

    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();

        auto opaque_rect = [this, &rect_intersect](Rect rect, RDPColor color){
            this->drawable.draw(
                RDPOpaqueRect(rect, color),
                rect_intersect,
                gdi::ColorCtx::depth24()
            );
        };

        opaque_rect(rect, this->bg_color);

        if (this->has_scroll) {
            auto const bw = this->button_dim.w;
            auto const bh = this->button_dim.h;
            auto const rx = rect.x;
            auto const ry = rect.y;
            auto const rw = rect.cx;
            auto const rh = rect.cy;
            auto const sx = int16_t(rx + rw - bw);
            auto const cy = int16_t(this->cursor_button_y + ry);

            auto draw_button_borders = [=](int16_t y, uint16_t bh){
                // top
                opaque_rect(Rect(sx,            y,            bw,  2), this->fg_color);
                // bottom
                opaque_rect(Rect(sx,            y + bh - 2,   bw,  2), this->fg_color);
                // left
                opaque_rect(Rect(sx,            y + 2,   2,   bh - 4), this->fg_color);
                // right
                opaque_rect(Rect(sx + bw - 2,   y + 2,   2,   bh - 4), this->fg_color);
            };

            auto draw_text_button = [&](
                char const* text, ButtonType button_type, int16_t y, uint16_t bh, uint16_t dy
            ){
                bool const has_focus = (this->selected_button == button_type);
                auto const bg = has_focus ? this->focus_color : this->bg_color;
                if (has_focus) {
                    opaque_rect(Rect(sx + 2, y + 2, bw - 4, bh - 4), bg);
                }
                gdi::server_draw_text(
                    this->drawable, this->font,
                    sx + 3, y + dy + 2,
                    text, this->fg_color, bg,
                    gdi::ColorCtx::depth24(), rect);
            };

            draw_text_button(top_button_char, ButtonType::Top, ry, bh, 0);
            draw_text_button(cursor_button_char, ButtonType::Cursor, cy, this->cursor_button_h,
                             (this->cursor_button_h - this->button_dim.h) / 2);
            draw_text_button(bottom_button_char, ButtonType::Bottom, ry + rh - bh, bh, 0);

            // left scroll border
            opaque_rect(Rect(sx,          ry + bh, 1, rh - bh * 2), this->fg_color);
            // right scroll border
            opaque_rect(Rect(sx + bw - 1, ry + bh, 1, rh - bh * 2), this->fg_color);

            // top button
            draw_button_borders(ry, bh);
            // cursor button
            draw_button_borders(cy, this->cursor_button_h);
            // bottom button
            draw_button_borders(ry + rh - bh, bh);
        }

        if (!this->has_scroll
         || (this->x() <= rect_intersect.x
          && rect_intersect.x <= this->x() + this->cx() - this->button_dim.w)
        ) {
            uint16_t const glyph_cy = this->font.max_height() + this->y_text;

            auto lines = this->line_metrics.lines();
            size_t start = size_t(this->current_y) / glyph_cy;
            size_t count = this->cy() / glyph_cy + 1;
            start = std::min(start, lines.size());
            lines = lines.from_offset(std::min(start, lines.size()));
            lines = lines.first(std::min(count, lines.size()));

            int dy = this->y() + int(start) * glyph_cy - this->current_y;
            int const incy = this->y_text;
            int16_t const dx = this->x_text + rect.x;
            uint16_t const cx_text = rect.cx - (has_scroll ? this->button_dim.w : 0);
            for (auto const& line : lines) {
                dy += incy;
                gdi::server_draw_text(
                    this->drawable,
                    this->font,
                    dx,
                    int16_t(dy),
                    line.str,
                    this->fg_color,
                    this->bg_color,
                    gdi::ColorCtx::depth24(),
                    rect_intersect.intersect(
                        Rect(rect.x, int16_t(dy), cx_text, this->font.max_height())
                    )
                );
                dy += this->font.max_height();
            }
        }

        this->drawable.end_update();
    }
}
