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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Poelen Jonathan, Raphael Zhou, Meng Tan
*/

#pragma once

#include <utility>

#include "core/RDP/capabilities/cap_glyphcache.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/image_frame_api.hpp"
#include "gdi/resize_api.hpp"

#include "utils/drawable_pointer.hpp"
#include "utils/drawable.hpp"


class RDPDrawable
: public gdi::GraphicApi, public gdi::ImageFrameApi, public gdi::ResizeApi
{
    using Color = Drawable::Color;

    Drawable drawable;
    uint8_t  save_mouse[3072];   // 32 lines * 32 columns * 3 bytes per pixel = 3072 octets
    uint16_t save_mouse_x;
    uint16_t save_mouse_y;
public:
    int mouse_cursor_pos_x;
    int mouse_cursor_pos_y;
    int mouse_cursor_hotspot_x;
    int mouse_cursor_hotspot_y;
private:
    bool dont_show_mouse_cursor;
    const DrawablePointer * current_pointer;
    DrawablePointer dynamic_pointer;
    DrawablePointer default_pointer;

    int frame_start_count;
    BGRPalette mod_palette_rgb;

    uint8_t fragment_cache[MAXIMUM_NUMBER_OF_FRAGMENT_CACHE_ENTRIES][1 /* size */ + MAXIMUM_SIZE_OF_FRAGMENT_CACHE_ENTRIE];

    unsigned int last_update_index = 1;

public:
    RDPDrawable(const uint16_t width, const uint16_t height);

    void resize(uint16_t width, uint16_t height) override
    {
        this->drawable.resize(width, height);
    }

    ConstImageView get_image_view() const override
    {
        return gdi::get_image_view(this->drawable);
    }

    ImageView get_mutable_image_view() override
    {
        return gdi::get_mutable_image_view(this->drawable);
    }

    uint8_t * first_pixel() noexcept
    {
        return this->drawable.first_pixel();
    }

    const uint8_t * data() const noexcept
    {
        return this->drawable.data();
    }

    uint16_t width() const noexcept
    {
        return this->drawable.width();
    }

    uint16_t height() const noexcept
    {
        return this->drawable.height();
    }

    static constexpr uint8_t bpp() noexcept
    {
        return Drawable::bpp();
    }

    unsigned size() const noexcept
    {
        return this->drawable.size();
    }

    size_t rowsize() const noexcept
    {
        return this->drawable.rowsize();
    }

    size_t pix_len() const noexcept
    {
        return this->drawable.pix_len();
    }

    void set_mouse_cursor_pos(int x, int y)
    {
        this->mouse_cursor_pos_x = x;
        this->mouse_cursor_pos_y = y;

    }

    void show_mouse_cursor(bool x)
    {
        this->dont_show_mouse_cursor = !x;
    }

    void prepare_image_frame() override
    {}

    // TODO FIXME temporary
    //@{
    Drawable & impl() noexcept
    {
        return this->drawable;
    }

    const Drawable & impl() const noexcept
    {
        return this->drawable;
    }
    //@}

    void set_row(size_t rownum, const uint8_t * data, size_t data_length) override
    {
        this->drawable.set_row(rownum, data, data_length);
    }

    void draw(RDPColCache   const & /*cmd*/) override;
    void draw(RDPBrushCache const & /*cmd*/) override;
    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPScrBlt & cmd, Rect clip) override;
    void draw(const RDPDestBlt & cmd, Rect clip) override;
    void draw(RDPNineGrid const &  /*cmd*/, Rect  /*rect*/, gdi::ColorCtx  /*color_ctx*/, Bitmap const &  /*bmp*/) override;
    void draw(const RDPMultiDstBlt & cmd, Rect clip) override;
    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override;
    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPMemBlt & cmd_, Rect clip, const Bitmap & bmp) override;
    void draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bmp) override;

    /*
     *
     *            +----+----+
     *            |\   |   /|  4 cases.
     *            | \  |  / |  > Case 1 is the normal case
     *            |  \ | /  |  > Case 2 has a negative coeff
     *            | 3 \|/ 2 |  > Case 3 and 4 are the same as
     *            +----0---->x    Case 1 and 2 but one needs to
     *            | 4 /|\ 1 |     exchange begin and end.
     *            |  / | \  |
     *            | /  |  \ |
     *            |/   |   \|
     *            +----v----+
     *                 y
     *  Anyway, we base the line drawing on bresenham's algorithm
     */
    void draw(const RDPLineTo & lineto, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override;
    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override;
    void draw(const RDP::FrameMarker & order) override;

    bool logical_frame_ended()
    {
        return this->drawable.logical_frame_ended;
    }

    void trace_mouse();
    void clear_mouse();

    void draw(const RDP::RAIL::NewOrExistingWindow            & /*unused*/) override {}
    void draw(const RDP::RAIL::WindowIcon                     & /*unused*/) override {}
    void draw(const RDP::RAIL::CachedIcon                     & /*unused*/) override {}
    void draw(const RDP::RAIL::DeletedWindow                  & /*unused*/) override {}
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & /*unused*/) override {}
    void draw(const RDP::RAIL::DeletedNotificationIcons       & /*unused*/) override {}
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & /*unused*/) override {}
    void draw(const RDP::RAIL::NonMonitoredDesktop            & /*unused*/) override {}

    void set_pointer(const Pointer & cursor) override;

    void set_palette(const BGRPalette & palette) override
    {
        this->mod_palette_rgb = palette;
    }

    unsigned int get_last_update_index() const noexcept override
    {
        return this->last_update_index;
    }

    bool reset(unsigned int /*x*/, unsigned int /*y*/,
        unsigned int /*out_width*/, unsigned int /*out_height*/) noexcept override
    { return false; }

    Rect get_rect() const override
    {
        return Rect(0, 0, this->drawable.width(), this->drawable.height());
    }
};
