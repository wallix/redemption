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

#ifndef REDEMPTION_PUBLIC_MOD_MOD_OSD_HPP
#define REDEMPTION_PUBLIC_MOD_MOD_OSD_HPP

#include "mod_api.hpp"

#include <functional>


class mod_osd : public mod_api
{
    template<class Command, class... OtherArgs>
    void split_draw(const Rect & cmd_rect, Command const & cmd, const Rect & clip, OtherArgs const &... other_args)
    {
        const Rect rect = cmd_rect.intersect(clip);
        if (this->fg_rect.contains(rect) || rect.isempty()) {
            //nada
        }
        else if (rect.has_intersection(this->fg_rect)) {
            const subrect_t rect4 = this->subrect(rect);
            this->mod.begin_update();
            if (!rect4.top.isempty()) {
                this->mod.draw(cmd, rect4.top, other_args...);
            }
            if (!rect4.right.isempty()) {
                this->mod.draw(cmd, rect4.right, other_args...);
            }
            if (!rect4.bottom.isempty()) {
                this->mod.draw(cmd, rect4.bottom, other_args...);
            }
            if (!rect4.left.isempty()) {
                this->mod.draw(cmd, rect4.left, other_args...);
            }
            this->mod.end_update();
        }
        else {
            this->mod.draw(cmd, clip, other_args...);
        }
    }

    struct subrect_t {
        Rect top;
        Rect right;
        Rect bottom;
        Rect left;
    };

    subrect_t subrect(const Rect & rect) const
    {
        const Rect sect = rect.intersect(this->fg_rect);
        return {
            Rect(rect.x, rect.y, rect.cx, sect.y - rect.y),
            Rect(sect.right(), sect.y, rect.right() - sect.right(), sect.cy),
            Rect(rect.x, sect.bottom(), rect.cx, rect.bottom() - sect.bottom()),
            Rect(rect.x, sect.y, sect.x - rect.x, sect.cy)
        };
    }

    void draw_fg(Rect const & rect)
    {
        this->dispatch_draw_fg(this->mod, rect, this->fg_rect);
    }

    const Rect fg_rect;
    mod_api & mod;
    FrontAPI & front;
    std::function<void(mod_api & mod, const Rect & rect, const Rect & clip)> dispatch_draw_fg;

public:
    mod_osd(
        FrontAPI & front, mod_api & mod, const Rect & rect,
        std::function<void(mod_api & mod, const Rect & rect, const Rect & clip)> f,
        bool call_f = true
    )
    : mod_api(mod.get_front_width(), mod.get_front_height())
    , fg_rect(Rect(0, 0, mod.get_front_width(), mod.get_front_height()).intersect(rect))
    , mod(mod)
    , front(front)
    , dispatch_draw_fg(std::move(f))
    {
        if (call_f) {
            this->draw_fg(this->fg_rect);
        }
        this->set_gd(mod, this);
    }

private:
    struct dispatch_bmp_draw {
        const Bitmap bmp;
        const int bmp_srcx;
        const int bmp_srcy;

        void operator()(mod_api & mod, const Rect & rect, const Rect & clip) const {
            mod.draw(RDPMemBlt(0, rect, 0xCC, this->bmp_srcx, this->bmp_srcy, 0), clip, this->bmp);
        }
    };
public:
    mod_osd(FrontAPI & front, mod_api & mod, const Bitmap& bmp, int x = 0, int y = 0)
    : mod_api(mod.get_front_width(), mod.get_front_height())
    , fg_rect(Rect(0, 0, mod.get_front_width(), mod.get_front_height()).intersect(Rect(x,y,bmp.cx(),bmp.cy())))
    , mod(mod)
    , front(front)
    , dispatch_draw_fg(dispatch_bmp_draw{bmp, x - this->fg_rect.x, y - this->fg_rect.y})
    {
        this->draw_fg(this->fg_rect);
        this->set_gd(mod, this);
    }

    virtual ~mod_osd()
    {
        if (this->is_active()) {
            this->mod.rdp_input_invalidate(this->fg_rect);
        }
    }

    const Rect & fg() const
    {
        return this->fg_rect;
    }

    bool is_active() const
    {
        return this->get_gd(this->mod) == this;
    }

    void swap_active()
    {
        if (this->is_active()) {
            this->set_gd(this->mod, &this->mod);
            this->mod.rdp_input_invalidate(this->fg_rect);
        }
        else {
            this->mod.begin_update();
            this->draw_fg(this->fg_rect);
            this->mod.end_update();
            this->set_gd(this->mod, this);
        }
    }

    virtual wait_obj& get_event()
    {
        return this->mod.get_event();
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->split_draw(cmd.rect, cmd, clip);
    }

private:
    void subrect_input_invalidate(const Rect & rect)
    {
        subrect_t rect4 = this->subrect(rect);

        if (!rect4.top.isempty()){
            this->mod.rdp_input_invalidate(rect4.top);
        }
        if (!rect4.right.isempty()){
            this->mod.rdp_input_invalidate(rect4.right);
        }
        if (!rect4.bottom.isempty()){
            this->mod.rdp_input_invalidate(rect4.bottom);
        }
        if (!rect4.left.isempty()){
            this->mod.rdp_input_invalidate(rect4.left);
        }
    }

public:
    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        const Rect drect = cmd.rect.intersect(clip);
        const int deltax = cmd.srcx - cmd.rect.x;
        const int deltay = cmd.srcy - cmd.rect.y;
        const int srcx = drect.x + deltax;
        const int srcy = drect.y + deltay;
        const Rect srect(srcx, srcy, drect.cx, drect.cy);

        const bool has_dest_intersec_fg = drect.has_intersection(this->fg_rect);
        const bool has_src_intersec_fg = srect.has_intersection(this->fg_rect);

        if (!has_dest_intersec_fg && !has_src_intersec_fg) {
            this->mod.draw(cmd, clip);
            return ;
        }
        else {
            this->mod.begin_update();
            this->subrect_input_invalidate(drect);
            this->mod.end_update();
        }
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

    virtual void draw(const RDPBrushCache & cmd) { this->mod.draw(cmd); }
    virtual void draw(const RDPColCache   & cmd) { this->mod.draw(cmd); }
    virtual void draw(const RDPGlyphCache & cmd) { this->mod.draw(cmd); }

private:
    void draw_bitmap_rect(Rect const & rect, Rect const & rectBmp, Bitmap const & bmp)
    {
        if (!rect.isempty()) {
            this->mod.draw(RDPMemBlt(0, rect, 0xCC, rect.x - rectBmp.x, rect.y - rectBmp.y, 0), rect, bmp);
        }
    }

public:
    virtual void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
                      size_t size, const Bitmap & bmp)
    {
        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                    , bitmap_data.dest_right - bitmap_data.dest_left + 1
                    , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        if (rectBmp.has_intersection(this->fg_rect)) {
            const subrect_t rect4 = this->subrect(rectBmp);
            this->front.flush();
            this->mod.begin_update();
            this->draw_bitmap_rect(rect4.top, rectBmp, bmp);
            this->draw_bitmap_rect(rect4.right, rectBmp, bmp);
            this->draw_bitmap_rect(rect4.bottom, rectBmp, bmp);
            this->draw_bitmap_rect(rect4.left, rectBmp, bmp);
            this->mod.end_update();
            this->front.flush();
        }
        else {
            this->mod.draw(bitmap_data, data, size, bmp);
        }
    }

    virtual void draw(const RDP::FrameMarker& order)
    {
        this->mod.draw(order);
    }

    virtual void begin_update()
    {
        this->mod.begin_update();
    }

    virtual void end_update()
    {
        this->mod.end_update();
    }

    virtual void draw_event(time_t now)
    {
        this->mod.draw_event(now);
    }

    virtual void rdp_input_invalidate(const Rect & r)
    {
        if (r.has_intersection(this->fg_rect)) {
            this->mod.begin_update();
            this->subrect_input_invalidate(r);
            this->draw_fg(this->fg_rect.intersect(r));
            this->mod.end_update();
        }
        else {
            this->mod.rdp_input_invalidate(r);
        }
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

/*
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
*/

    virtual void server_set_pointer(const Pointer & cursor)
    {
        this->mod.server_set_pointer(cursor);
    }

    virtual void send_pointer(int cache_idx, const Pointer & cursor) {
        this->mod.send_pointer(cache_idx, cursor);
    }

    virtual void set_pointer(int cache_idx)
    {
        this->mod.set_pointer(cache_idx);
    }

/*
    virtual void set_pointer_display()
    {
        this->mod.set_pointer_display();
    }
*/

    virtual void flush()
    {
        this->mod.flush();
    }

    virtual void server_draw_text(int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {
        this->mod.server_draw_text(x, y, text, fgcolor, bgcolor, clip);
    }

    virtual void text_metrics(const char * text, int & width, int & height)
    {
        this->mod.text_metrics(text, width, height);
    }

    virtual void send_to_mod_channel(const char * const front_channel_name, Stream & chunk, size_t length, uint32_t flags)
    {
        this->mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    // Interface for session to send back to mod_rdp for tse virtual channel target data (asked previously)
    virtual void send_auth_channel_data(const char * data)
    {
        this->mod.send_auth_channel_data(data);
    }

    virtual void rdp_input_up_and_running()
    {
        this->mod.rdp_input_up_and_running();
    }

    // Front calls this member function when it became up and running.
    virtual void on_front_up_and_running()
    {
        this->mod.on_front_up_and_running();
    }

    virtual void disconnect()
    {
        this->mod.disconnect();
    }

    virtual bool is_up_and_running()
    {
        return this->mod.is_up_and_running();
    }

    virtual void rdp_input_invalidate2(const DArray< Rect >& vr)
    {
        this->mod.rdp_input_invalidate2(vr);
    }

    virtual void refresh_context(Inifile& ini)
    {
        this->mod.refresh_context(ini);
    }

    virtual void send_data_indication_ex(uint16_t channelId, HStream& stream)
    {
        this->mod.send_data_indication_ex(channelId, stream);
    }

    virtual void send_fastpath_data(Stream& data)
    {
        this->mod.send_fastpath_data(data);
    }

    virtual void set_mod_palette(const BGRPalette& palette)
    {
        this->mod.set_mod_palette(palette);
    }
};

#endif
