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
#include "front_api.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "RDP/bitmapupdate.hpp"
#include "bitmap.hpp"

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

    ~mod_osd() override {
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

    wait_obj& get_event() override {
        return this->mod.get_event();
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
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
    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
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

    void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        this->split_draw(cmd.rect, cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight), cmd, clip);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        this->split_draw(cmd.rect, cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect &clip) override {
        this->split_draw(cmd.rect, cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->split_draw(cmd.rect, cmd, clip, bmp);
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) override {
        this->split_draw(cmd.rect, cmd, clip, bmp);
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.startx, cmd.starty, cmd.endx - cmd.startx, cmd.endy - cmd.starty), cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) override {
        this->split_draw(Rect(cmd.glyph_x, cmd.glyph_y - cmd.bk.cy, cmd.bk.cx, cmd.bk.cy), cmd, clip, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        int16_t endx = 0;
        int16_t endy = 0;
        for (unsigned i = 0; i < cmd.NumDeltaEntries; i++) {
            endx = std::max(endx, cmd.deltaEncodedPoints[i].xDelta);
            endy = std::max(endy, cmd.deltaEncodedPoints[i].yDelta);
        }
        this->split_draw(Rect(cmd.xStart, cmd.yStart, endx - cmd.xStart, endy - cmd.yStart), cmd, clip);
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.el.center_x() - cmd.el.radius_x(),
                              cmd.el.center_y() - cmd.el.radius_y(), cmd.el.radius_x() * 2, cmd.el.radius_y() * 2),
                         cmd, clip);
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        this->split_draw(Rect(cmd.el.center_x() - cmd.el.radius_x(),
                              cmd.el.center_y() - cmd.el.radius_y(), cmd.el.radius_x() * 2, cmd.el.radius_y() * 2),
                         cmd, clip);
    }

    void draw(const RDPBrushCache & cmd) override { this->mod.draw(cmd); }
    void draw(const RDPColCache   & cmd) override { this->mod.draw(cmd); }

private:
    void draw_bitmap_rect(Rect const & rect, Rect const & rectBmp, Bitmap const & bmp)
    {
        if (!rect.isempty()) {
            this->mod.draw(RDPMemBlt(0, rect, 0xCC, rect.x - rectBmp.x, rect.y - rectBmp.y, 0), rect, bmp);
        }
    }

public:
    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
                      size_t size, const Bitmap & bmp) override {
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

    void draw(const RDP::FrameMarker& order) override {
        this->mod.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        this->mod.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        this->mod.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        this->mod.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        this->mod.draw(order);
    }

    void begin_update() override {
        this->mod.begin_update();
    }

    void end_update() override {
        this->mod.end_update();
    }

    void draw_event(time_t now) override {
        this->mod.draw_event(now);
    }

    void rdp_input_invalidate(const Rect & r) override {
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

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {
        this->mod.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {
        this->mod.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {
        this->mod.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void send_to_front_channel(const char*const mod_channel_name, uint8_t const * data, size_t length,
                                       size_t chunk_size, int flags) override {
        this->mod.send_to_front_channel(mod_channel_name, data, length, chunk_size, flags);
    }

    void server_set_pointer(const Pointer & cursor) override {
        this->mod.server_set_pointer(cursor);
    }

    void flush() override {
        this->mod.flush();
    }

    void send_to_mod_channel(const char * const front_channel_name, InStream & chunk, size_t length, uint32_t flags) override {
        this->mod.send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    // Interface for session to send back to mod_rdp for tse virtual channel target data (asked previously)
    void send_auth_channel_data(const char * data) override {
        this->mod.send_auth_channel_data(data);
    }

    // Client calls this member function when it became up and running.
    void rdp_input_up_and_running() override {
        this->mod.rdp_input_up_and_running();
    }

    void disconnect() override {
        this->mod.disconnect();
    }

    bool is_up_and_running() override {
        return this->mod.is_up_and_running();
    }

    void rdp_input_invalidate2(const DArray< Rect >& vr) override {
        this->mod.rdp_input_invalidate2(vr);
    }

    void refresh_context(Inifile& ini) override {
        this->mod.refresh_context(ini);
    }

    void set_mod_palette(const BGRPalette& palette) override {
        this->mod.set_mod_palette(palette);
    }
};

#endif
