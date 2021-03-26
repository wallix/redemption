/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "gdi/graphic_api.hpp"
#include "gdi/resize_api.hpp"

#include <vector>


struct RailScreenVisibility
{
    RailScreenVisibility(bool verbose) noexcept;
    ~RailScreenVisibility();

    void update(const RDP::RAIL::NewOrExistingWindow & cmd);
    void update(const RDP::RAIL::DeletedWindow & cmd);

    Rect get_join_visibility_rect() const;

private:
    struct WindowRecord;
    struct WindowVisibilityRectRecord;

    bool verbose;
    std::vector<WindowRecord> windows;
    std::vector<WindowVisibilityRectRecord> window_visibility_rects;
};


struct RailScreenComputation final
: gdi::NullGraphic
, gdi::ResizeApi
{
    RailScreenComputation(uint16_t width, uint16_t height, bool verbose) noexcept
    : screen(verbose)
    , width(width)
    , height(height)
    {}

    Rect get_max_image_frame_rect() const noexcept
    {
        return this->max_image_frame_rect;
    }

    Dimension get_min_image_frame_dim() const noexcept
    {
        return this->min_image_frame_dim;
    }

    void visibility_rect_event(Rect rect);

    void resize(uint16_t width, uint16_t height) override;

    void draw(const RDP::RAIL::NewOrExistingWindow & cmd) override;

    void draw(const RDP::RAIL::DeletedWindow & cmd) override;

    void draw(const RDP::RAIL::NonMonitoredDesktop & cmd) override;

    using gdi::NullGraphic::draw;

private:
    void update_rects(Rect rect);

    RailScreenVisibility screen;
    Rect max_image_frame_rect;
    Dimension min_image_frame_dim;
    uint16_t width;
    uint16_t height;
};
