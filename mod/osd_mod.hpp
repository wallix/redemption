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
#include "../../redemption-wab.9.x/public/regex/regex.hpp"

class osd_mod : public mod_api
{
    /**
     * Emulate variadic template with 2 arguments (variadic template is feature of C++11)
     * @{
     */
    template<class Command, class OtherArg>
    void dispath_draw(Command const & cmd, const Rect & clip, OtherArg const & other_arg)
    {
        this->mod.draw(cmd, clip, other_arg);
    }

    struct no_arg {};

    template<class Command>
    void dispath_draw(Command const & cmd, const Rect & clip, no_arg)
    {
        this->mod.draw(cmd, clip);
    }

    template<class Command>
    void split_draw(const Rect & cmd_rect, Command const & cmd, const Rect & clip)
    {
        this->split_draw(cmd_rect, cmd, clip, no_arg());
    }
    //@}

    template<class Command, class OtherArg>
    void split_draw(const Rect & cmd_rect, Command const & cmd, const Rect & clip, OtherArg const & other_arg)
    {
        Rect rect = cmd_rect.intersect(clip);
        if (this->fg_rect.contains(rect) || rect.isempty()) {
            //nada
        }
        else if (rect.has_intersection(this->fg_rect)) {
            //top
            if (rect.y < this->fg_rect.y) {
                this->dispath_draw(cmd, Rect(rect.x, rect.y, rect.cx, this->fg_rect.y - rect.y), other_arg);
            }
            //bottom
            if (this->fg_rect.bottom() < rect.bottom()) {
                this->dispath_draw(cmd, Rect(rect.x, this->fg_rect.bottom(), rect.cx,
                                             rect.bottom() - this->fg_rect.bottom()),
                                   other_arg);
            }
            //left
            if (rect.x < this->fg_rect.x) {
                const int16_t y = std::max(this->fg_rect.y, rect.y);
                this->dispath_draw(cmd, Rect(rect.x, y, this->fg_rect.x - rect.x, this->fg_rect.bottom() - y),
                                   other_arg);
            }
            //right
            if (this->fg_rect.right() < rect.right()) {
                const int16_t y = std::max(this->fg_rect.y, rect.y);
                this->dispath_draw(cmd, Rect(this->fg_rect.right(), y,
                                             rect.right() - this->fg_rect.right(), this->fg_rect.bottom() - y),
                                   other_arg);
            }
        }
        else {
            this->dispath_draw(cmd, clip, other_arg);
        }
    }

    Rect fg_rect;
    mod_api & mod;

public:
    osd_mod(mod_api & mod, Rect const & rect_saver)
    : mod_api(mod.front_width, mod.front_height)
    , fg_rect(Rect(0, 0, mod.front_width, mod.front_height).intersect(rect_saver))
    , mod(mod)
    {}

    virtual ~osd_mod()
    {
        // TODO send order redraw(this->fg_rect)
    }

    virtual wait_obj& get_event()
    {
        return this->mod.get_event();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->split_draw(cmd.rect, cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        const Rect drect = cmd.rect.intersect(clip);
        const signed int srcx = drect.x + (cmd.srcx - cmd.rect.x);
        const signed int srcy = drect.y + (cmd.srcy - cmd.rect.y);
        const Rect srect(srcx, srcy, drect.cx, drect.cy);

        const bool has_intersection_src = cmd.rect.has_intersection(srect);
        const bool has_intersection_dest = cmd.rect.has_intersection(drect);
        if (!has_intersection_src && !has_intersection_dest) {
            this->mod.draw(cmd, clip);
            return ;
        }

        if (srect.y < this->fg_rect.y) {
            Rect rect(cmd.rect.x, cmd.rect.y, cmd.rect.x, this->fg_rect.y - cmd.rect.y);
            this->split_draw(rect, cmd, clip);
        }

        if (this->fg_rect.bottom() < srect.bottom()) {
            Rect rect(cmd.rect.x, this->fg_rect.bottom(), cmd.rect.x, cmd.rect.bottom() - this->fg_rect.bottom());
            this->split_draw(rect, cmd, clip);
        }

        const Rect intersec = srect.intersect(this->fg_rect);

        {
            const Rect left(srect.x, intersec.y, intersec.x - srect.x, intersec.cy);
            if (!left.isempty()) {
                this->split_draw(left.offset(srcx, srcy), cmd, clip);
            }
        }
        {
            const Rect right(intersec.right(), intersec.y, intersec.right() - srect.right(), intersec.cy);
            if (!right.isempty()) {
                this->split_draw(right.offset(srcx, srcy), cmd, clip);
            }
        }

        //TODO redraw: this->split_draw(intersec, ???, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->split_draw(cmd.rect, cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip)
    {
        this->split_draw(cmd.rect, cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        this->split_draw(cmd.rect, cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->split_draw(cmd.rect, cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->split_draw(cmd.rect, cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.startx, cmd.starty, cmd.endx - cmd.startx, cmd.endy - cmd.starty), cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->split_draw(Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy), cmd, clip, gly_cache);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip)
    {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaEncodedPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaEncodedPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.el.centerx - cmd.el.radiusx,
                              cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2),
                           cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip)
    {
        this->split_draw(Rect(cmd.el.centerx - cmd.el.radiusx,
                              cmd.el.centery - cmd.el.radiusy, cmd.el.radiusx * 2, cmd.el.radiusy * 2),
                           cmd, clip);
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
