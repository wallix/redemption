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

#include "mod/mod_api.hpp"
#include "core/front_api.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/bitmap.hpp"

#include "gdi/clip_from_cmd.hpp"

#include <functional>


class mod_osd : public gdi::GraphicBase<mod_osd, mod_api>
{
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
        this->drawable_fn(this->mod, rect, this->fg_rect);
    }

    using drawable_function_type = std::function<void(mod_api & mod, const Rect & rect, const Rect & clip)>;

    const Rect fg_rect;
    mod_api & mod;
    drawable_function_type drawable_fn;

public:
    mod_osd(mod_api & mod, Rect const & rect, drawable_function_type f, bool call_f = true)
    : mod_osd::base_type(mod.get_front_width(), mod.get_front_height())
    , fg_rect(Rect(0, 0, mod.get_front_width(), mod.get_front_height()).intersect(rect))
    , mod(mod)
    , drawable_fn(std::move(f))
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

        void operator()(mod_api & mod, Rect const & rect, Rect const & clip) const {
            mod.draw(RDPMemBlt(0, rect, 0xCC, this->bmp_srcx, this->bmp_srcy, 0), clip, this->bmp);
        }
    };
public:
    mod_osd(mod_api & mod, Bitmap const & bmp, int x = 0, int y = 0)
    : mod_osd::base_type(mod.get_front_width(), mod.get_front_height())
    , fg_rect(Rect(0, 0, mod.get_front_width(), mod.get_front_height()).intersect(Rect(x,y,bmp.cx(),bmp.cy())))
    , mod(mod)
    , drawable_fn(dispatch_bmp_draw{bmp, x - this->fg_rect.x, y - this->fg_rect.y})
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

protected:
    friend gdi::GraphicCoreAccess;

    template<class Command>
    void draw_impl(Command const & cmd) {
        this->mod.draw(cmd);
    }

    template<class Command, class... Args>
    void draw_impl(Command const & cmd, Rect const & clip, Args const &... args) {
        auto const & rect = clip_from_cmd(cmd);
        if (this->fg_rect.contains(rect) || rect.isempty()) {
            //nada
        }
        else if (rect.has_intersection(this->fg_rect)) {
            const subrect_t rect4 = this->subrect(rect);
            this->mod.begin_update();
            if (!rect4.top.isempty()) {
                this->mod.draw(cmd, rect4.top, args...);
            }
            if (!rect4.right.isempty()) {
                this->mod.draw(cmd, rect4.right, args...);
            }
            if (!rect4.bottom.isempty()) {
                this->mod.draw(cmd, rect4.bottom, args...);
            }
            if (!rect4.left.isempty()) {
                this->mod.draw(cmd, rect4.left, args...);
            }
            this->mod.end_update();
        }
        else {
            this->mod.draw(cmd, clip, args...);
        }
    }

    void draw_impl(const RDPBitmapData & bitmap_data, const Bitmap & bmp) {
        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top
                    , bitmap_data.dest_right - bitmap_data.dest_left + 1
                    , bitmap_data.dest_bottom - bitmap_data.dest_top + 1);

        if (rectBmp.has_intersection(this->fg_rect)) {
            const subrect_t rect4 = this->subrect(rectBmp);

            auto draw_bitmap_rect = [this, &rectBmp, &bmp](Rect const & rect) {
                if (!rect.isempty()) {
                    this->mod.draw(
                        RDPMemBlt(0, rect, 0xCC, rect.x - rectBmp.x, rect.y - rectBmp.y, 0),
                        rect, bmp
                    );
                }
            };

            //this->mod.sync();
            this->mod.begin_update();
            draw_bitmap_rect(rect4.top);
            draw_bitmap_rect(rect4.right);
            draw_bitmap_rect(rect4.bottom);
            draw_bitmap_rect(rect4.left);
            this->mod.end_update();
        }
        else {
            this->mod.draw(bitmap_data, bmp);
        }
    }

    void draw_impl(const RDPScrBlt & cmd, const Rect & clip) {
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

    void set_pointer(const Pointer & cursor) override {
        this->mod.set_pointer(cursor);
    }

    void sync() override {
        this->mod.sync();
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

    void set_palette(const BGRPalette& palette) override {
        this->mod.set_palette(palette);
    }
};

#endif
