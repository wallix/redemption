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
 *   Foundation, Inc.; 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan,
 *              Loïc Michaux
 */

#ifndef REDEMPTION_PUBLIC_MOD_OSD_MOD_HPP
#define REDEMPTION_PUBLIC_MOD_OSD_MOD_HPP

#include "mod_api.hpp"

class osd_mod : public mod_api
{
    struct region_saver
    {
        const Rect clip;
        unsigned srcx;
        unsigned srcy;
        mod_api & mod;
        Bitmap & fg;

        region_saver(const Rect & dest, mod_api & mod, unsigned srcx, unsigned srcy, Bitmap & fg, Bitmap & bg)
        : clip(Rect(dest.x, dest.y, fg.cx, fg.cy).intersect(dest))
        , srcx(srcx)
        , srcy(srcy)
        , mod(mod)
        , fg(fg)
        {
            if (!this->clip.isempty()) {
                this->mod.draw(RDPMemBlt(0, clip, 0x55, clip.x - srcx, clip.y - srcy, 0), clip, bg);
            }
        }

        ~region_saver()
        {
            if (!this->clip.isempty()) {
                this->mod.draw(RDPMemBlt(0, clip, 0x55, clip.x - srcx, clip.y - srcy, 0), clip, fg);
            }
        }

    private:
        region_saver(region_saver const &);
    };

    Bitmap bg;
    Bitmap fg;
    unsigned destx;
    unsigned desty;
    mod_api & mod;

public:
    osd_mod(int bpp, mod_api & mod, Drawable const & out, Drawable const & in, unsigned destx, unsigned desty)
    : mod_api(mod.front_width, mod.front_height)
    , bg(in.data, in.width, in.height, bpp, Rect(0,0,in.width, in.height))
    , fg(out.data, out.width, out.height, bpp, Rect(destx,desty,in.width, in.height))
    , destx(destx)
    , desty(desty)
    , mod(mod)
    {
        const Rect rect(destx, desty, fg.cx, fg.cy);
        mod.draw(RDPMemBlt(0, rect, 0x55, 0, 0, 0), rect, fg);
        RDPBmpCache(this->fg);
    }

    virtual ~osd_mod()
    {
        const Rect rect(destx, desty, bg.cx, bg.cy);
        mod.draw(RDPMemBlt(0, rect, 0x55, 0, 0, 0), rect, bg);
    }

    virtual wait_obj& get_event()
    {
        return this->mod.get_event();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
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
        region_saver saver1(rect1, this->mod, this->destx, this->desty, this->fg, this->bg);
        region_saver saver2(rect2, this->mod, this->destx, this->desty, this->fg, this->bg);
        region_saver saver3(rect3, this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        region_saver saver(clip.intersect(cmd.rect), this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        region_saver saver(clip.intersect(cmd.rect), this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        region_saver saver(clip.intersect(cmd.rect), this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.startx, cmd.starty, cmd.endx - cmd.startx, cmd.endy - cmd.starty)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        region_saver saver(clip.intersect(Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip, gly_cache);
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
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
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
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
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
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.el.centerx - cmd.el.radiusx,
                                               cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        region_saver saver(clip.intersect(Rect(cmd.el.centerx - cmd.el.radiusx,
                                               cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2)),
                          this->mod, this->destx, this->desty, this->fg, this->bg);
        this->mod.draw(cmd, clip);
    }

    //@{
    virtual void draw(const RDPBrushCache & cmd) { this->mod.draw(cmd); }
    virtual void draw(const RDPColCache   & cmd) { this->mod.draw(cmd); }
    virtual void draw(const RDPGlyphCache & cmd) { this->mod.draw(cmd); }

    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
                      size_t size, const Bitmap & bmp)
    {
        this->mod.draw(bitmap_data, data, size, bmp);
    }
    //@}

    virtual void begin_update()
    {
        this->mod.begin_update();
    }

    virtual void end_update()
    {
        this->mod.begin_update();
    }

    virtual void draw_event(time_t now)
    {
        this->mod.draw_event(now);
    }

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->mod.rdp_input_invalidate(r);
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->mod.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {
        this->mod.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->mod.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    virtual void send_to_front_channel(const char*const mod_channel_name, uint8_t* data, size_t length,
                                       size_t chunk_size, int flags)
    {
        this->mod.send_to_front_channel(mod_channel_name, data, length, chunk_size, flags);
    }

    virtual void set_row(size_t rownum, const uint8_t * data)
    {
        this->mod.set_row(rownum, data);
    }

    virtual void input(const timeval & now, Stream & input_data_32)
    {
        this->mod.input(now, input_data_32);
    }

    virtual void snapshot(const timeval & now, int mouse_x, int mouse_y,
                          bool ignore_frame_in_timeval)
    {
        this->mod.snapshot(now, mouse_x, mouse_y, ignore_frame_in_timeval);
    }

    virtual void server_set_pointer(const Pointer & cursor)
    {
        this->mod.server_set_pointer(cursor);
    }

    virtual void send_pointer(int cache_idx, const uint8_t * data,
                              const uint8_t * mask, int hotspot_x, int hotspot_y)
    {
        this->mod.send_pointer(cache_idx, data, mask, hotspot_x, hotspot_y);
    }

    virtual void set_pointer(int cache_idx)
    {
        this->mod.set_pointer(cache_idx);
    }

    virtual void set_pointer_display()
    {
        this->mod.set_pointer_display();
    }

    virtual void flush()
    {
        this->mod.flush();
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        this->mod.server_draw_text(x, y, text, bgcolor, fgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height)
    {
        this->mod.text_metrics(text, width, height);
    }
};

#endif
