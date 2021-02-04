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
    BGRColor fgcolor, BGRColor bgcolor, BGRColor focuscolor,
    Font const & font, uint16_t xtext, uint16_t ytext)
: Widget(drawable, parent, notifier, group_id)
, fg_color(encode_color24()(fgcolor))
, bg_color(encode_color24()(bgcolor))
, focus_color(encode_color24()(focuscolor))
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

        auto const line_on_screen = std::max(cy / glyph_cy - 1, 1);
        this->step = (int(this->line_metrics.lines().size()) + line_on_screen - 1)
                     / line_on_screen - 1;

        this->cursor_button_y = this->button_dim.h - 1;
        this->current_value = 0;
    }
}

Dimension WidgetVerticalScrollText::get_optimal_dim()
{
    return Dimension(
        this->line_metrics.max_width() + this->x_text * 2,
        (this->font.max_height() + this->y_text) * this->line_metrics.lines().size() + this->y_text
    );
}

int16_t WidgetVerticalScrollText::compute_cursor_y(int current_value) const
{
    return int16_t(this->button_dim.h - 1
                 + (this->cy() - this->button_dim.h * 3 + 2)
                   * current_value / this->step);
}

void WidgetVerticalScrollText::rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
{
    auto const old_value = this->current_value;

    auto redraw = [&]() {
        if (old_value != this->current_value) {
            this->cursor_button_y = this->compute_cursor_y(this->current_value);
            this->rdp_input_invalidate(this->get_rect());
        }
    };

    if (!this->has_scroll) {
        this->Widget::rdp_input_mouse(device_flags, x, y, keymap);
    }
    else if (device_flags == (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
        this->mouse_down = true;

        auto in_range = [](int x, int16_t rx, uint16_t rcx){
            return rx <= x && x < rx + rcx;
        };
        auto in_range_y = [=](int y, int16_t ry){
            ry += this->y();
            return in_range(y, ry, this->button_dim.h);
        };

        if (!in_range(x, this->eright() - this->button_dim.w * 2, this->cx())) {
            // outside
        }
        else if (in_range_y(y, this->cursor_button_y)) {
            this->selected_button = ButtonType::Cursor;

            this->cursor_button_diff_y = y - (this->y() + this->cursor_button_y + this->button_dim.h / 2);

            this->rdp_input_invalidate(this->get_rect());
        }
        // top
        else if (y < this->y() + this->cursor_button_y) {
            this->selected_button = ButtonType::Top;
            if (this->current_value > 0) {
                --this->current_value;
                redraw();
            }
        }
        // bottom
        else if (y > this->y() + this->cursor_button_y + this->button_dim.h) {
            this->selected_button = ButtonType::Bottom;
            if (this->current_value < this->step) {
                ++this->current_value;
                redraw();
            }
        }
    }
    else if (device_flags == MOUSE_FLAG_BUTTON1) {
        this->mouse_down          = false;
        this->selected_button     = ButtonType::None;
    }
    else if (device_flags == MOUSE_FLAG_MOVE) {
        if (this->mouse_down && (ButtonType::Cursor == this->selected_button)) {
            const int min_button_y = this->y() + this->button_dim.h - 1;
            const int max_button_y = min_button_y + this->cy() - this->button_dim.h * 3 + 2;

            const int min_y = min_button_y + this->cursor_button_diff_y;
            const int max_y = max_button_y + this->cursor_button_diff_y;

            if (y < min_y) {
                this->current_value = 0;
            }
            else if (y >= max_y) {
                this->current_value = this->step;
            }
            else {
                this->current_value = (y - min_y) * this->step / (max_y - min_y);
            }

            redraw();
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

    auto const old_value = this->current_value;

    auto redraw = [&]() {
        if (old_value != this->current_value) {
            this->cursor_button_y = this->compute_cursor_y(this->current_value);
            this->rdp_input_invalidate(this->get_rect());
        }
    };

    if (keymap->nb_kevent_available() > 0) {
        switch (keymap->top_kevent()){
            case Keymap2::KEVENT_LEFT_ARROW:
            case Keymap2::KEVENT_UP_ARROW:
            case Keymap2::KEVENT_PGUP:
                keymap->get_kevent();
                if (this->current_value > 0) {
                    --this->current_value;
                    redraw();
                }
                break;

            case Keymap2::KEVENT_RIGHT_ARROW:
            case Keymap2::KEVENT_DOWN_ARROW:
            case Keymap2::KEVENT_PGDOWN:
                keymap->get_kevent();
                if (this->current_value < this->step) {
                    ++this->current_value;
                    redraw();
                }
                break;

            default:
                Widget::rdp_input_scancode(param1, param2, param3, param4, keymap);
                break;
        }
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
            auto const sx = rx + rw - bw;
            auto const cy = int16_t(this->cursor_button_y + ry);

            auto draw_button_borders = [=](int16_t y){
                // top
                opaque_rect(Rect(sx,            y,            bw,  2), this->fg_color);
                // bottom
                opaque_rect(Rect(sx,            y + bh - 2,   bw,  2), this->fg_color);
                // left
                opaque_rect(Rect(sx,            y + 2,   2,   bh - 4), this->fg_color);
                // right
                opaque_rect(Rect(sx + bw - 2,   y + 2,   2,   bh - 4), this->fg_color);
            };

            auto draw_text_button = [&](char const* text, ButtonType button_type, int16_t y){
                bool const has_focus = (this->mouse_down && (this->selected_button == button_type));
                auto const bg = has_focus ? this->focus_color : this->bg_color;
                if (has_focus) {
                    opaque_rect(Rect(sx + 2, y + 2, bw - 4, bh - 4), bg);
                }
                gdi::server_draw_text(
                    this->drawable, this->font,
                    sx + 3, y + 2,
                    text, this->fg_color, bg,
                    gdi::ColorCtx::depth24(), rect);
            };

            draw_text_button(top_button_char, ButtonType::Top, ry);
            draw_text_button(cursor_button_char, ButtonType::Cursor, cy);
            draw_text_button(bottom_button_char, ButtonType::Bottom, ry + rh - bh);

            // left scroll border
            opaque_rect(Rect(sx,          ry + bh, 1, rh - bh * 2), this->fg_color);
            // right scroll border
            opaque_rect(Rect(sx + bw - 1, ry + bh, 1, rh - bh * 2), this->fg_color);

            // top button
            draw_button_borders(ry);
            // cursor button
            draw_button_borders(cy);
            // bottom button
            draw_button_borders(ry + rh - bh);
        }

        uint16_t const glyph_cy = this->font.max_height() + this->y_text;
        auto const line_on_screen = std::max(rect.cy / glyph_cy - 1, 1);

        auto lines = this->line_metrics.lines();
        size_t start = line_on_screen * this->current_value;
        size_t count = this->cy() / glyph_cy + 1;
        start = std::min(start, lines.size());
        lines = lines.from_offset(std::min(start, lines.size()));
        lines = lines.first(std::min(count, lines.size()));

        int dy = this->y();
        int16_t dx = this->x_text + rect.x;
        int incy = this->y_text;
        uint16_t const cx_text = rect.cx - (has_scroll ? this->button_dim.w : 0);
        for (auto const& line : lines) {
            dy += incy;
            gdi::server_draw_text(this->drawable
                                , this->font
                                , dx
                                , dy
                                , line.str
                                , this->fg_color
                                , this->bg_color
                                , gdi::ColorCtx::depth24()
                                , rect_intersect.intersect(
                                    Rect(rect.x, dy, cx_text, this->font.max_height()))
                );
            dy += this->font.max_height();
        }

        this->drawable.end_update();
    }
}
