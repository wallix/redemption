/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "RAIL/mouse_context.hpp"
#define INTERNAL_MODULE_WINDOW_ID    40000
#define INTERNAL_MODULE_WINDOW_TITLE "Wallix AdminBastion"

#define AUXILIARY_WINDOW_ID          40001

#define TITLE_BAR_HEIGHT       24
#define TITLE_BAR_BUTTON_WIDTH 37

#define INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH  640
#define INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT 480

void MouseContext::update_rects(const bool allow_resize_hosted_desktop)
{
    LOG_IF(this->verbose, LOG_INFO, "MouseContext::update_rects()");

    if ((this->window_rect.cx - 2) % 4) {
        this->window_rect.cx -= ((this->window_rect.cx - 2) % 4);
    }

    this->title_bar_rect = this->window_rect;
    this->title_bar_rect.cy = TITLE_BAR_HEIGHT;
    this->title_bar_rect.x++;
    this->title_bar_rect.y++;
    this->title_bar_rect.cx -= 2;
    this->title_bar_rect.cy--;

    this->title_bar_icon_rect    = this->title_bar_rect;
    this->title_bar_icon_rect.cx = 3 + 16 + 2;

    if (allow_resize_hosted_desktop) {
        this->resize_hosted_desktop_box_rect     = this->title_bar_rect;
        this->resize_hosted_desktop_box_rect.x  += this->title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 4;
        this->resize_hosted_desktop_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;
    }

    this->minimize_box_rect     = this->title_bar_rect;
    this->minimize_box_rect.x  += this->title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 3;
    this->minimize_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    this->maximize_box_rect     = this->title_bar_rect;
    this->maximize_box_rect.x  += this->title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 2;
    this->maximize_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    this->close_box_rect     = this->title_bar_rect;
    this->close_box_rect.x  += this->title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH;
    this->close_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    this->title_bar_rect.cx -= TITLE_BAR_BUTTON_WIDTH * (3 + (allow_resize_hosted_desktop? 1 : 0));

    this->title_bar_rect.x  += 3 + 16 + 2;
    this->title_bar_rect.cx -= 3 + 16 + 2;
}   // update_rects


Rect MouseContext::get_window_rect() const
{
    LOG_IF(this->verbose, LOG_INFO, "MouseContext::get_window_rect() -> %s", this->window_rect);
    return this->window_rect;
}

Point MouseContext::get_window_offset() const
{
    LOG_IF(this->verbose, LOG_INFO, "MouseContext::get_window_offset() -> (%d,%d)", this->window_offset_x, this->window_offset_y);
    return Point(this->window_offset_x, this->window_offset_y);
}

