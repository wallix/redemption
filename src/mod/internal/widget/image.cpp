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
 *              Meng Tan
 */

#include "mod/internal/widget/image.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "gdi/graphic_api.hpp"
#include "core/RDP/bitmapupdate.hpp"

WidgetImage::WidgetImage(
    gdi::GraphicApi & drawable, const char * filename, Widget & parent,
    NotifyApi* notifier, int group_id
)
    : Widget(drawable, parent, notifier, group_id)
    , bmp(bitmap_from_file(filename))
{
    this->tab_flag   = IGNORE_TAB;
    this->focus_flag = IGNORE_FOCUS;
}

WidgetImage::~WidgetImage() = default;

void WidgetImage::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());
    if (rect_intersect.x < 0) {
        rect_intersect.cx -= std::min<uint16_t>(-rect_intersect.x, rect_intersect.cx);
        rect_intersect.x = 0;
    }
    if (rect_intersect.y < 0) {
        rect_intersect.cy -= std::min<uint16_t>(-rect_intersect.y, rect_intersect.cy);
        rect_intersect.y = 0;
    }

    auto draw_bitmap = [](gdi::GraphicApi & drawable, Bitmap const & bitmap, Rect const & rect ) {
        RDPBitmapData bitmap_data;

        bitmap_data.dest_left       = rect.x;
        bitmap_data.dest_top        = rect.y;
        bitmap_data.dest_right      = rect.x + rect.cx - 1;
        bitmap_data.dest_bottom     = rect.y + rect.cy - 1;
        bitmap_data.width           = bitmap.cx();
        bitmap_data.height          = bitmap.cy();
        bitmap_data.bits_per_pixel  = safe_int(bitmap.bpp());
        bitmap_data.flags           = 0;
        bitmap_data.bitmap_length   = bitmap.bmp_size();

        drawable.draw(bitmap_data, bitmap);
    };

    if (!rect_intersect.isempty()) {
        if ((rect_intersect != this->get_rect()) || (this->bmp.cx() % 4)) {
            Bitmap new_bmp(this->bmp, Rect(rect_intersect.x - this->x(),
                rect_intersect.y - this->y(), align4(rect_intersect.cx), rect_intersect.cy));

            draw_bitmap(this->drawable, new_bmp, rect_intersect);
        }
        else {
            draw_bitmap(this->drawable, this->bmp, this->get_rect());
        }
    }
}

Dimension WidgetImage::get_optimal_dim()
{
    return Dimension(this->bmp.cx(), this->bmp.cy());
}
