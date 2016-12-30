/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/internal/widget2/composite.hpp"
#include "mod/internal/widget2/scroll.hpp"
#include "mod/mod_api.hpp"

class WidgetModuleHost : public WidgetParent,
    public gdi::GraphicBase<WidgetModuleHost>
{
private:
    class ModuleHolder : public mod_api
    {
    private:
        WidgetModuleHost& host;

        std::unique_ptr<mod_api> managed_mod;

    public:
        ModuleHolder(WidgetModuleHost& host,
                     std::unique_ptr<mod_api> managed_mod)
        : host(host)
        , managed_mod(std::move(managed_mod))
        {
            REDASSERT(this->managed_mod);
        }

        // Callback
        void send_to_mod_channel(const char* front_channel_name,
                                 InStream& chunk, size_t length,
                                 uint32_t flags) override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->send_to_mod_channel(
                        front_channel_name,
                        chunk,
                        length,
                        flags
                    );
            }
        }

        // mod_api

        void draw_event(time_t now, gdi::GraphicApi& drawable) override
        {
            if (this->managed_mod)
            {
                this->host.drawable_ptr = &drawable;

                this->managed_mod->draw_event(now, this->host);

                this->host.drawable_ptr = nullptr;
            }
        }

        wait_obj& get_event() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_event();
            }

            return mod_api::get_event();
        }

        int get_fd() const override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_fd();
            }

            return INVALID_SOCKET;
        }

        void get_event_handlers(std::vector<EventHandler>& out_event_handlers)
            override
        {
            if (this->managed_mod)
            {
                this->managed_mod->get_event_handlers(out_event_handlers);
            }
        }

        bool is_up_and_running() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->is_up_and_running();
            }

            return mod_api::is_up_and_running();
        }

        void send_to_front_channel(const char* const mod_channel_name,
                                   const uint8_t* data, size_t length,
                                   size_t chunk_size, int flags) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->send_to_front_channel(mod_channel_name,
                    data, length, chunk_size, flags);
            }
        }

        // RdpInput

        void rdp_input_invalidate(const Rect& r) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_invalidate(r);
            }
        }

        void rdp_input_mouse(int device_flags, int x, int y,
                             Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_mouse(device_flags, x, y,
                    keymap);
            }
        }

        void rdp_input_scancode(long param1, long param2, long param3,
                                long param4, Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_scancode(param1, param2, param3,
                    param4, keymap);
            }
        }

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags,
                                   int16_t param1, int16_t param2) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_synchronize(time, device_flags,
                    param1, param2);
            }
        }

        void rdp_input_up_and_running() override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_up_and_running();
            }
        }

        Dimension get_dim() const override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_dim();
            }

            return mod_api::get_dim();
        }
    } module_holder;

    CompositeArray composite_array;

    gdi::GraphicApi* drawable_ptr = nullptr;

    gdi::GraphicApi& drawable_ref;

    WidgetHScrollBar hscroll;

    unsigned int hscroll_height = 0;

    bool hscroll_added = false;

    Rect mod_visible_rect;

    Rect vision_rect;

public:
    WidgetModuleHost(gdi::GraphicApi& drawable, Widget2& parent,
                     NotifyApi* notifier,
                     std::unique_ptr<mod_api> managed_mod, Font const & font,
                     Theme const & theme, int group_id = 0)
    : WidgetParent(drawable, parent, notifier, group_id)
    , gdi::GraphicBase<WidgetModuleHost>(gdi::GraphicDepth::unspecified())
    , module_holder(*this, std::move(managed_mod))
    , drawable_ref(drawable)
    , hscroll(drawable, *this, this, 0, theme.global.fgcolor, theme.global.bgcolor, theme.global.focus_color, font)
    , order_depth_(gdi::GraphicDepth::unspecified())
    {
        this->impl = &composite_array;

        this->tab_flag   = NORMAL_TAB;
        this->focus_flag = NORMAL_FOCUS;

        Dimension dim = this->hscroll.get_optimal_dim();
        this->hscroll_height = dim.h;
        this->hscroll.set_cy(this->hscroll_height);
    }

    mod_api& get_managed_mod()
    {
        return this->module_holder;
    }

private:
    gdi::GraphicApi& get_drawable()
    {
        if (this->drawable_ptr)
        {
            return *this->drawable_ptr;
        }

        return this->drawable_ref;
    }

    void update_rects() {
        const Dimension module_dim = this->module_holder.get_dim();

        this->vision_rect = this->get_rect();

        if ((this->cx() >= module_dim.w) && (this->cy() >= module_dim.h)) {
            if (this->hscroll_added) {
                this->remove_widget(&this->hscroll);

                this->hscroll_added = false;
            }

            this->mod_visible_rect = Rect(0, 0, module_dim.w, module_dim.h);
        }
        else {
            if (this->vision_rect.cx < module_dim.w) {
                this->vision_rect.cy -= this->hscroll_height;

                if (!this->hscroll_added) {
                    this->add_widget(&this->hscroll);

                    this->hscroll_added = true;
                }
            }
        }

        this->mod_visible_rect.cx = std::min<uint16_t>(this->vision_rect.cx, module_dim.w);
        this->mod_visible_rect.cy = std::min<uint16_t>(
            this->vision_rect.cy - (this->hscroll_added ? this->hscroll_height : 0),
            module_dim.h);

        if (this->hscroll_added) {
            if (this->mod_visible_rect.x + this->mod_visible_rect.cx < this->vision_rect.cx) {
                this->mod_visible_rect.x = this->vision_rect.cx - this->mod_visible_rect.cx;
            }

            this->hscroll.set_max_value(module_dim.w - this->vision_rect.cx);
            this->hscroll.set_current_value(this->mod_visible_rect.x);

            this->hscroll.set_xy(this->vision_rect.x, this->vision_rect.y + this->vision_rect.cy);
            this->hscroll.set_wh(this->vision_rect.cx, this->hscroll_height);
        }
    }

public:
    using gdi::GraphicBase<WidgetModuleHost>::draw;

    void set_cx(uint16_t cx) override {
        WidgetParent::set_cx(cx);

        this->update_rects();

        this->rdp_input_invalidate(this->get_rect());
    }

    void set_cy(uint16_t cy) override {
        WidgetParent::set_cy(cy);

        this->update_rects();

        this->rdp_input_invalidate(this->get_rect());
    }

    void set_x(int16_t x) override {
        WidgetParent::set_x(x);

        this->update_rects();

        this->rdp_input_invalidate(this->get_rect());
    }

    void set_y(int16_t y) override {
        WidgetParent::set_y(y);

        this->update_rects();

        this->rdp_input_invalidate(this->get_rect());
    }

    virtual void set_depths(gdi::GraphicDepth const & depth) {
        this->order_depth_ = depth;
    }

    virtual gdi::GraphicDepth const & order_depth() const {
        return this->order_depth_;
    }

    gdi::GraphicDepth order_depth_;

public:
    // NotifyApi
    void notify(Widget2* /*widget*/, NotifyApi::notify_event_t event) override {
        if (event == NOTIFY_HSCROLL) {
            if (this->hscroll_added) {
                this->mod_visible_rect.x = this->hscroll.get_current_value();

                Rect mod_update_rect = this->vision_rect.offset(
                    -this->x() + this->mod_visible_rect.x, -this->y() + this->mod_visible_rect.y);
                if (!mod_update_rect.isempty()) {
                    this->module_holder.rdp_input_invalidate(mod_update_rect);
                }
            }
        }
    }

    // RdpInput

    void rdp_input_invalidate(const Rect & r) override
    {
        this->begin_update();

        SubRegion region;
        region.rects.push_back(r.intersect(this->get_rect()));
        if (!this->vision_rect.isempty()) {
            region.subtract_rect(this->vision_rect);
        }
        if (this->hscroll_added) {
            Rect hscroll_rect = this->hscroll.get_rect();
            if (!hscroll_rect.isempty()) {
                region.subtract_rect(hscroll_rect);
            }
        }

        ::fill_region(this->drawable, region, 0x000000);

        Rect mod_update_rect = r.intersect(this->vision_rect);
        mod_update_rect = mod_update_rect.offset(-this->x() + this->mod_visible_rect.x, -this->y() + this->mod_visible_rect.y);
        if (!mod_update_rect.isempty()) {
            this->module_holder.rdp_input_invalidate(mod_update_rect);
        }

        if (this->hscroll_added && r.has_intersection(this->hscroll.get_rect())) {
            this->hscroll.draw(r);
        }

        this->end_update();
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        if (this->vision_rect.contains_pt(x, y)) {
            this->module_holder.rdp_input_mouse(device_flags, x - this->x() + this->mod_visible_rect.x, y - this->y() + this->mod_visible_rect.y, keymap);
        }
        else {
            WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->module_holder.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        this->module_holder.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    // Widget2

    void draw(const Rect& clip) override {
        if (this->hscroll_added) {
            this->hscroll.draw(clip);
        }
    }

private:
    // gdi::GraphicBase

    friend gdi::GraphicCoreAccess;

    void begin_update() override
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.begin_update();
    }

    void end_update() override
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.end_update();
    }

    template<class Cmd>
    void draw_impl(const Cmd& cmd) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        drawable_.draw(cmd);
    }

    template<class Cmd, class... Args>
    void draw_impl(const Cmd& cmd, const Rect& clip, const Args&... args)
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "order_id=%u", unsigned(cmd.id()));
        LOG(LOG_INFO, "");
        LOG(LOG_INFO, "");

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        drawable_.draw(cmd, new_clip, args...);
    }

    void draw_impl(const RDPBitmapData& bitmap_data, const Bitmap& bmp)
    {
        Rect boundary(bitmap_data.dest_left,
                      bitmap_data.dest_top,
                      bitmap_data.dest_right - bitmap_data.dest_left + 1,
                      bitmap_data.dest_bottom - bitmap_data.dest_top + 1
                     );

        this->draw_impl(RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bmp);
    }

    void draw_impl(const RDPDestBlt& cmd, const Rect& clip)
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPDestBlt new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }

    void draw_impl(const RDPLineTo& cmd, Rect const & clip) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPLineTo new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }

    void draw_impl(const RDPMem3Blt& cmd, const Rect& clip, const Bitmap& bmp) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPMem3Blt new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip, bmp);
    }

    void draw_impl(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPMemBlt new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip, bmp);
    }

    void draw_impl(const RDPMultiOpaqueRect& cmd, const Rect& clip) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPMultiOpaqueRect new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }

    void draw_impl(const RDPOpaqueRect& cmd, const Rect& clip) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPOpaqueRect new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }


    void draw_impl(const RDPPatBlt& cmd, const Rect& clip)
    {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPPatBlt new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }

    void draw_impl(const RDPScrBlt& cmd, const Rect& clip) {
        gdi::GraphicApi& drawable_ = this->get_drawable();

        Rect new_clip = clip.offset(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);
        new_clip = new_clip.intersect(this->vision_rect);
        if (new_clip.isempty()) { return; }

        RDPScrBlt new_cmd = cmd;

        new_cmd.move(this->x() - this->mod_visible_rect.x, this->y() - this->mod_visible_rect.y);

        drawable_.draw(new_cmd, new_clip);
    }
};
