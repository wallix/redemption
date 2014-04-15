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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#ifndef REDEMPTION_PUBLIC_CAPTURE_OSD_CAPTURE_HPP
#define REDEMPTION_PUBLIC_CAPTURE_OSD_CAPTURE_HPP

#include "RDP/RDPGraphicDevice.hpp"

#include <vector>
#include <algorithm>


class OSDCapture : public RDPGraphicDevice
{
    struct buffer_type
    {
        Rect region;
        uint8_t * buf;

        buffer_type(const Rect & rect)
        : region(rect)
        , buf(new uint8_t[rect.cx * Drawable::Bpp * rect.cy])
        {}

        std::size_t rowsize() const
        {
            return this->region.cx * Drawable::Bpp;
        }

        std::size_t height() const
        {
            return this->region.cy;
        }

        ~buffer_type()
        {
            delete[] this->buf;
        }
    } buf;
    RDPGraphicDevice & gd;
    Drawable & out;

    static void swap_region(uint8_t * src, uint8_t * dest,
                            unsigned w, unsigned h,
                            unsigned src_step, unsigned dest_step)
    {
        for (unsigned i = 0; i < h; ++i) {
            std::swap_ranges(src, src + w, dest);
            src += src_step;
            dest += dest_step;
        }
    }

    static void copy_region(const uint8_t * src, uint8_t * dest,
                            unsigned w, unsigned h,
                            unsigned src_step, unsigned dest_step)
    {
        for (unsigned i = 0; i < h; ++i) {
            std::copy(src, src + w, dest);
            src += src_step;
            dest += dest_step;
        }
    }

    struct region_saver
    {
        const Rect clip;
        Drawable & out;
        buffer_type & buf;

        region_saver(const Rect & dest, Drawable & out, buffer_type & buf)
        : clip(buf.region.intersect(dest))
        , out(out)
        , buf(buf)
        {
            this->swap_region();
        }

        ~region_saver()
        {
            this->swap_region();
        }

        void swap_region()
        {
            OSDCapture::swap_region(this->out.first_pixel(this->clip), this->buf.buf,
                                    this->clip.cx * Drawable::Bpp, this->clip.cy,
                                    this->out.rowsize, this->buf.rowsize());
        }
    };

public:
    OSDCapture(RDPGraphicDevice & gd, Drawable & out, Drawable const & in, unsigned destx, unsigned desty)
    : buf(Rect(destx, desty, in.width, in.height).intersect(out.width, out.height))
    , gd(gd)
    , out(out)
    {
        uint8_t * out_buf = out.first_pixel(buf.region);
        this->copy_region(out_buf, this->buf.buf, buf.rowsize(), buf.height(), out.rowsize, buf.rowsize());
        this->copy_region(in.first_pixel(), out_buf, buf.rowsize(), buf.height(), in.rowsize, out.rowsize);
    }

    virtual ~OSDCapture()
    {
        this->copy_region(this->buf.buf, out.first_pixel(buf.region),
                          buf.rowsize(), buf.height(), buf.rowsize(), out.rowsize);
    }

    Rect const & rect() const
    {
        return this->buf.region;
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        Rect rect1 = clip.intersect(cmd.rect);
        Rect rect2 = clip.offset(cmd.srcx - cmd.rect.x, cmd.srcy - cmd.rect.y);
        Rect rect3;
        if (rect1.y > rect2.y) {
            rect3 = rect1.x < rect2.x
            ? Rect(rect1.x, rect1.y, rect2.right() - rect1.x, rect2.bottom() - rect1.y)
            : Rect(rect2.x, rect1.y, rect1.right() - rect2.x, rect2.bottom() - rect1.y);
            rect1.cy -= rect3.cy;
            rect2.cy -= rect3.cy;
            rect2.y += rect3.cy;
        }
        else if (rect1.y < rect2.y) {
            rect3 = rect1.x < rect2.x
            ? Rect(rect1.x, rect2.y, rect2.right() - rect1.x, rect1.bottom() - rect2.y)
            : Rect(rect2.x, rect2.y, rect1.right() - rect2.x, rect1.bottom() - rect2.y);
            rect2.cy -= rect3.cy;
            rect1.cy -= rect3.cy;
            rect1.y += rect3.cy;
        }
        region_saver saver1(rect1, this->out, this->buf);
        region_saver saver2(rect2, this->out, this->buf);
        region_saver saver3(rect3, this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        region_saver saver(clip.intersect(cmd.rect), this->out, this->buf);
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        region_saver saver(clip.intersect(cmd.rect), this->out, this->buf);
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.startx, cmd.starty, cmd.endx - cmd.startx, cmd.endy - cmd.starty)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        region_saver saver(clip.intersect(Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        region_saver saver(clip.intersect(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        region_saver saver(clip.intersect(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaEncodedPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaEncodedPoints[i].yDelta);
        }
        region_saver saver(clip.intersect(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.el.centerx - cmd.el.radiusx,
                                               cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.el.centerx - cmd.el.radiusx,
                                               cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2)),
                           this->out, this->buf);
        this->gd.draw(cmd, clip);
    }

    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        this->gd.set_row(rownum, data);
    }

    virtual void input(const timeval & now, Stream & input_data_32)
    {
        this->gd.input(now, input_data_32);
    }

    virtual void snapshot(const timeval & now, int mouse_x, int mouse_y,
                          bool ignore_frame_in_timeval)
    {
        this->gd.snapshot(now, mouse_x, mouse_y, ignore_frame_in_timeval);
    }

    virtual void server_set_pointer(const Pointer & cursor)
    {
        this->gd.server_set_pointer(cursor);
    }

    virtual void send_pointer(int cache_idx, const uint8_t * data,
                              const uint8_t * mask, int hotspot_x, int hotspot_y)
    {
        this->gd.send_pointer(cache_idx, data, mask, hotspot_x, hotspot_y);
    }

    virtual void set_pointer(int cache_idx)
    {
        this->gd.set_pointer(cache_idx);
    }

    virtual void set_pointer_display()
    {
        this->gd.set_pointer_display();
    }

    virtual void flush()
    {
        this->gd.flush();
    }
};

#endif
