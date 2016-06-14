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

#pragma once

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

    bool disable_filter = false;

    subrect_t subrect(const Rect & rect) const
    {
        const Rect inter = rect.intersect(this->fg_rect);
        return {
            Rect(rect.x, rect.y, rect.cx, inter.y - rect.y),
            Rect(inter.right(), inter.y, rect.right() - inter.right(), inter.cy),
            Rect(rect.x, inter.bottom(), rect.cx, rect.bottom() - inter.bottom()),
            Rect(rect.x, inter.y, inter.x - rect.x, inter.cy)
        };
    }

    void draw_fg(Rect const & rect)
    {
        class AutoDisabler {
            bool & ref;
        public:
            explicit AutoDisabler(bool & ref) : ref(ref) {
                this->ref = true;
            }
            ~AutoDisabler() {
                this->ref = false;
            }
        } auto_disabler(this->disable_filter);

        this->drawable_fn(this->mod, rect.intersect(this->fg_rect));
    }

    using drawable_function_type = std::function<void(mod_api & mod, const Rect & clip)>;

    const Rect fg_rect;
    mod_api & mod;
    drawable_function_type drawable_fn;

    bool bogus_refresh_rect_ex = false;

public:
    mod_osd(
        /*FrontAPI & front, uint16_t front_width, uint16_t front_height, Font const & font,*/
        mod_api & mod, Rect const & rect, bool bogus_refresh_rect_ex, drawable_function_type f, bool call_f = true)
    : mod_osd::base_type(mod.get_front_width(), mod.get_front_height())
    , fg_rect(Rect(0, 0, mod.get_front_width(), mod.get_front_height()).intersect(rect))
    , mod(mod)
    , drawable_fn(std::move(f))
    , bogus_refresh_rect_ex(bogus_refresh_rect_ex)
    {
        if (call_f) {
            this->draw_fg(this->fg_rect);
        }
// TODO         this->set_gd(mod, this);
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
// TODO         this->set_gd(mod, this);
    }

    ~mod_osd() override {
        if (this->is_active()) {
            this->remove_osd();
        }
    }

    const Rect & fg() const
    {
        return this->fg_rect;
    }

    bool is_active() const
    {
// TODO        return this->get_gd(this->mod) == this;
        return true;
    }

    void swap_active()
    {
        if (this->is_active()) {
// TODO             this->set_gd(this->mod, &this->mod);
            this->remove_osd();
        }
        else {
            this->redraw_osd();
// TODO             this->set_gd(this->mod, this);
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
        if (this->disable_filter) {
            this->mod.draw(cmd, clip, args...);
            return;
        }

        auto const & rect = clip_from_cmd(cmd).intersect(clip);
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

        if (!this->disable_filter && rectBmp.has_intersection(this->fg_rect)) {
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
        if (this->disable_filter) {
            this->mod.draw(cmd, clip);
            return;
        }

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

    void remove_osd()
    {
        if (this->bogus_refresh_rect_ex) {
            this->mod.rdp_suppress_display_updates();
            this->mod.rdp_allow_display_updates(0, 0,
                this->mod.get_front_width(), this->mod.get_front_height());
        }
        this->mod.rdp_input_invalidate(this->fg_rect);
    }

    void redraw_osd()
    {
        this->mod.begin_update();
        this->draw_fg(this->fg_rect);
        this->mod.end_update();
    }

public:
    void begin_update() override {
        this->mod.begin_update();
    }

    void end_update() override {
        this->mod.end_update();
    }

    void draw_event(time_t now, GraphicApi & drawable) override {
        this->mod.draw_event(now, drawable);
    }

    void rdp_input_invalidate(const Rect & r) override {
        if (!this->disable_filter && r.has_intersection(this->fg_rect)) {
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

    void send_disconnect_ultimatum() override {
        this->mod.send_disconnect_ultimatum();
    }

    void rdp_input_invalidate2(array_view<Rect> vr) override {
        this->mod.rdp_input_invalidate2(vr);
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top,
            uint16_t right, uint16_t bottom) override {
        Rect const intersect = this->fg_rect.intersect(Rect(left, top, right - left + 1, bottom - top + 1));
        if (!intersect.isempty()) {
            this->mod.begin_update();
            this->draw_fg(intersect);
            this->mod.end_update();
        }
        this->mod.rdp_allow_display_updates(left, top, right, bottom);
    }

    void rdp_suppress_display_updates() override {
        this->mod.rdp_suppress_display_updates();
    }

    void refresh_context(Inifile& ini) override {
        this->mod.refresh_context(ini);
    }

    void set_palette(const BGRPalette& palette) override {
        this->mod.set_palette(palette);
    }

    wait_obj * get_secondary_event() override {
        return this->mod.get_secondary_event();
    }

    wait_obj * get_asynchronous_task_event(int & out_fd) override {
        return this->mod.get_asynchronous_task_event(out_fd);
    }

    void process_asynchronous_task() override {
        this->mod.process_asynchronous_task();
    }

    wait_obj * get_session_probe_launcher_event() override {
        return this->mod.get_session_probe_launcher_event();
    }

    void process_session_probe_launcher() override {
        this->mod.process_session_probe_launcher();
    }

//    void server_draw_text_deprecated(
//        const Font& font, int16_t x, int16_t y, const char* text,
//        uint32_t fgcolor, uint32_t bgcolor, const Rect& clip
//    ) override {
//        mod.server_draw_text_poubelle(font, x, y, text, fgcolor, bgcolor, clip);
//    }
};

