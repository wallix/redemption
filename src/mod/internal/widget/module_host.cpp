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

#include "mod/internal/widget/module_host.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "utils/region.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/mod_api.hpp"

#include "RAIL/client_execute.hpp" // TODO only for BORDER_WIDTH_HEIGHT

#include <type_traits>


class RDPEllipseCB;
class RDPEllipseSC;
class RDPPolygonCB;
class RDPPolygonSC;


WidgetModuleHost::ModuleHolder::ModuleHolder(
    WidgetModuleHost& host, std::unique_ptr<mod_api>&& managed_mod)
: host(host)
, managed_mod(std::move(managed_mod))
{
    assert(this->managed_mod);
}

gdi::GraphicApi & WidgetModuleHost::proxy_gd(gdi::GraphicApi& gd)
{
    this->drawable_ptr = &gd;
    return *this;
}

// Callback
void WidgetModuleHost::ModuleHolder::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name,
    InStream& chunk, size_t length,
    uint32_t flags)
{
    this->managed_mod->send_to_mod_channel(
        front_channel_name,
        chunk,
        length,
        flags
    );
}

void WidgetModuleHost::ModuleHolder::send_auth_channel_data(const char * string_data)
{
    this->managed_mod->send_auth_channel_data(string_data);
}

void WidgetModuleHost::ModuleHolder::send_checkout_channel_data(const char * string_data)
{
    this->managed_mod->send_checkout_channel_data(string_data);
}

// mod_api

void WidgetModuleHost::ModuleHolder::draw_event(time_t now, gdi::GraphicApi& gd)
{
    this->host.drawable_ptr = &gd;

    this->managed_mod->draw_event(now, this->host);

    this->host.drawable_ptr = &this->host.drawable_ref;
}

bool WidgetModuleHost::ModuleHolder::is_up_and_running() const
{
    return this->managed_mod->is_up_and_running();
}

bool WidgetModuleHost::ModuleHolder::is_auto_reconnectable()
{
    return this->managed_mod->is_auto_reconnectable();
}

// RdpInput

void WidgetModuleHost::ModuleHolder::rdp_input_invalidate(Rect r)
{
    this->managed_mod->rdp_input_invalidate(r);
}

void WidgetModuleHost::ModuleHolder::rdp_input_mouse(
    int device_flags, int x, int y, Keymap2* keymap)
{
    this->managed_mod->rdp_input_mouse(device_flags, x, y, keymap);
}

void WidgetModuleHost::ModuleHolder::rdp_input_scancode(
    long param1, long param2, long param3, long param4, Keymap2* keymap)
{
    this->managed_mod->rdp_input_scancode(param1, param2, param3, param4, keymap);
}

void WidgetModuleHost::ModuleHolder::rdp_input_synchronize(
    uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
{
    this->managed_mod->rdp_input_synchronize(time, device_flags,
        param1, param2);
}

void WidgetModuleHost::ModuleHolder::rdp_input_up_and_running()
{
    this->managed_mod->rdp_input_up_and_running();
}

void WidgetModuleHost::ModuleHolder::refresh(Rect r)
{
    this->managed_mod->refresh(r);
}

Dimension WidgetModuleHost::ModuleHolder::get_dim() const
{
    return this->managed_mod->get_dim();
}


struct WidgetModuleHost::Impl
{
    inline static gdi::GraphicApi& get_drawable(WidgetModuleHost const& wmh) noexcept
    {
        return *wmh.drawable_ptr;
    }

    template<class Cmd>
    static void draw_impl(WidgetModuleHost const& wmh, const Cmd& cmd)
    {
        get_drawable(wmh).draw(cmd);
    }

    template<class Cmd>
    using cmd_not_implementing_move = std::integral_constant<bool,
        std::is_same<Cmd, RDPBitmapData >::value ||
        std::is_same<Cmd, RDPEllipseCB  >::value ||
        std::is_same<Cmd, RDPEllipseSC  >::value ||
        std::is_same<Cmd, RDPPolygonCB  >::value ||
        std::is_same<Cmd, RDPPolygonSC  >::value
    >;

    inline static Rect compute_clip(WidgetModuleHost const& wmh, const Rect clip) noexcept
    {
        return clip
            .offset(wmh.x() - wmh.mod_visible_rect.x, wmh.y() - wmh.mod_visible_rect.y)
            .intersect(wmh.vision_rect);
    }

    template<class Cmd, class... Args, typename std::enable_if<
        !cmd_not_implementing_move<Cmd>::value, bool
    >::type = 1>
    static void draw_impl(WidgetModuleHost const& wmh, const Cmd& cmd, const Rect clip, const Args&... args)
    {
        Rect new_clip = compute_clip(wmh, clip);
        if (new_clip.isempty()) { return; }

        Cmd new_cmd = cmd;
        new_cmd.move(wmh.x() - wmh.mod_visible_rect.x, wmh.y() - wmh.mod_visible_rect.y);

        get_drawable(wmh).draw(new_cmd, new_clip, args...);
    }

    template<class Cmd, class... Args, typename std::enable_if<
        cmd_not_implementing_move<Cmd>::value, bool
    >::type = 1>
    static void draw_impl(WidgetModuleHost const& wmh, const Cmd& cmd, const Rect clip, const Args&... args)
    {
        Rect new_clip = compute_clip(wmh, clip);
        if (new_clip.isempty()) { return; }

        get_drawable(wmh).draw(cmd, new_clip, args...);
    }

    static void draw_impl(WidgetModuleHost const& wmh, const RDPBitmapData& bitmap_data, const Bitmap& bmp)
    {
        Rect boundary(
            bitmap_data.dest_left, bitmap_data.dest_top,
            bitmap_data.dest_right - bitmap_data.dest_left + 1,
            bitmap_data.dest_bottom - bitmap_data.dest_top + 1
        );

        draw_impl(wmh, RDPMemBlt(0, boundary, 0xCC, 0, 0, 0), boundary, bmp);
    }

    static void draw_impl(WidgetModuleHost & wmh, const RDPScrBlt& cmd, const Rect clip)
    {
        Rect new_clip = compute_clip(wmh, clip);
        if (new_clip.isempty()) { return; }

        RDPScrBlt new_cmd = cmd;
        new_cmd.move(wmh.x() - wmh.mod_visible_rect.x, wmh.y() - wmh.mod_visible_rect.y);

        const Rect src_rect(cmd.srcx, cmd.srcy, cmd.rect.cx, cmd.rect.cy);

        if (wmh.mod_visible_rect.contains(src_rect)) {
            get_drawable(wmh).draw(new_cmd, new_clip);
        }
        else {
            wmh.rdp_input_invalidate(new_clip);
        }
    }

    static void draw_impl(WidgetModuleHost & wmh, const RDP::RDPMultiScrBlt& cmd, const Rect clip)
    {
        Rect new_clip = compute_clip(wmh, clip);
        if (new_clip.isempty()) { return; }

        RDP::RDPMultiScrBlt new_cmd = cmd;
        new_cmd.move(wmh.x() - wmh.mod_visible_rect.x, wmh.y() - wmh.mod_visible_rect.y);

        Rect src_rect(cmd.nXSrc, cmd.nYSrc, cmd.rect.cx, cmd.rect.cy);

        const signed int delta_x = cmd.nXSrc - cmd.rect.x;
        const signed int delta_y = cmd.nYSrc - cmd.rect.y;

        Rect sub_dest_rect;

        for (uint8_t i = 0; i < cmd.nDeltaEntries; i++) {
            sub_dest_rect.x  += cmd.deltaEncodedRectangles[i].leftDelta;
            sub_dest_rect.y  += cmd.deltaEncodedRectangles[i].topDelta;
            sub_dest_rect.cx  = cmd.deltaEncodedRectangles[i].width;
            sub_dest_rect.cy  = cmd.deltaEncodedRectangles[i].height;

            const Rect sub_src_rect(sub_dest_rect.x + delta_x,
                              sub_dest_rect.y + delta_y,
                              sub_dest_rect.cx, sub_dest_rect.cy);

            LOG(LOG_INFO, "Rect(%d %d %u %u)", sub_src_rect.x, sub_src_rect.y, sub_src_rect.cx, sub_src_rect.cy);

            src_rect = src_rect.disjunct(sub_src_rect);
        }

        LOG(LOG_INFO, "Rect(%d %d %u %u)", wmh.mod_visible_rect.x, wmh.mod_visible_rect.y, wmh.mod_visible_rect.cx, wmh.mod_visible_rect.cy);
        LOG(LOG_INFO, "Rect(%d %d %u %u)", src_rect.x, src_rect.y, src_rect.cx, src_rect.cy);

        if (wmh.mod_visible_rect.contains(src_rect)) {
            get_drawable(wmh).draw(new_cmd, new_clip);
        }
        else {
            wmh.rdp_input_invalidate(new_clip);
        }
    }
};


void WidgetModuleHost::draw(RDP::FrameMarker    const & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(RDPDestBlt          const & cmd, Rect clip)  { Impl::draw_impl(*this, cmd, clip); }
void WidgetModuleHost::draw(RDPMultiDstBlt      const & cmd, Rect clip)  { Impl::draw_impl(*this, cmd, clip); }
void WidgetModuleHost::draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPScrBlt           const & cmd, Rect clip)  { Impl::draw_impl(*this, cmd, clip); }
void WidgetModuleHost::draw(RDP::RDPMultiScrBlt const & cmd, Rect clip)  { Impl::draw_impl(*this, cmd, clip); }
void WidgetModuleHost::draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx)  { Impl::draw_impl(*this, cmd, clip, color_ctx); }
void WidgetModuleHost::draw(RDPBitmapData       const & cmd, Bitmap const & bmp)  { Impl::draw_impl(*this, cmd, bmp); }
void WidgetModuleHost::draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp)  { Impl::draw_impl(*this, cmd, clip, bmp);}
void WidgetModuleHost::draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp)  { Impl::draw_impl(*this, cmd, clip, color_ctx, bmp); }
void WidgetModuleHost::draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache)  { Impl::draw_impl(*this, cmd, clip, color_ctx, gly_cache); }

void WidgetModuleHost::draw(const RDP::RAIL::NewOrExistingWindow            & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::WindowIcon                     & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::CachedIcon                     & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::DeletedWindow                  & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::DeletedNotificationIcons       & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(const RDP::RAIL::NonMonitoredDesktop            & cmd)  { Impl::draw_impl(*this, cmd); }

void WidgetModuleHost::draw(RDPColCache   const & cmd)  { Impl::draw_impl(*this, cmd); }
void WidgetModuleHost::draw(RDPBrushCache const & cmd)  { Impl::draw_impl(*this, cmd); }

void WidgetModuleHost::set_pointer(Pointer const & pointer)
{
    Rect rect = this->get_rect();
    rect.x  += (BORDER_WIDTH_HEIGHT - 1);
    rect.cx -= (BORDER_WIDTH_HEIGHT - 1) * 2;
    rect.cy -= (BORDER_WIDTH_HEIGHT - 1);

    if (rect.contains_pt(this->current_pointer_pos_x, this->current_pointer_pos_y)) {
        Impl::get_drawable(*this).set_pointer(pointer);
    }

    this->current_pointer = pointer;
}

WidgetModuleHost::WidgetModuleHost(
    gdi::GraphicApi& drawable, Widget& parent,
    NotifyApi* notifier,
    std::unique_ptr<mod_api>&& managed_mod, Font const & font,
    const GCC::UserData::CSMonitor& cs_monitor,
    uint16_t front_width, uint16_t front_height,
    int group_id)
: WidgetParent(drawable, parent, notifier, group_id)
, module_holder(*this, std::move(managed_mod))
, drawable_ptr(&drawable)
, drawable_ref(drawable)
, hscroll(drawable, *this, this, true, BLACK,
    BGRColor(0x606060), BGRColor(0xF0F0F0), BGRColor(0xCDCDCD), font, true)
, vscroll(drawable, *this, this, false, BLACK,
    BGRColor(0x606060), BGRColor(0xF0F0F0), BGRColor(0xCDCDCD), font, true)
, monitors(cs_monitor)
{
    this->pointer_flag = Pointer::POINTER_CUSTOM;

    this->impl = &composite_array;

    this->tab_flag   = NORMAL_TAB;
    this->focus_flag = NORMAL_FOCUS;

    Dimension dim = this->hscroll.get_optimal_dim();
    this->hscroll_height = dim.h;
    this->hscroll.set_wh(this->hscroll.cx(), this->hscroll_height);

    dim = this->vscroll.get_optimal_dim();
    this->vscroll_width = dim.w;
    this->vscroll.set_wh(this->vscroll_width, this->vscroll.cy());

    this->monitor_one.monitorCount              = 1;
    this->monitor_one.monitorDefArray[0].left   = 0;
    this->monitor_one.monitorDefArray[0].top    = 0;
    this->monitor_one.monitorDefArray[0].right  = front_width - 1;
    this->monitor_one.monitorDefArray[0].bottom = front_height - 1;
}

void WidgetModuleHost::update_rects()
{
    const Dimension module_dim = this->module_holder.get_dim();

    this->vision_rect = this->get_rect();

    const bool old_hscroll_added = this->hscroll_added;
    const bool old_vscroll_added = this->vscroll_added;

    if ((this->cx() >= module_dim.w) && (this->cy() >= module_dim.h)) {
        this->hscroll_added = false;
        this->vscroll_added = false;

        this->mod_visible_rect = Rect(0, 0, module_dim.w, module_dim.h);
    }
    else {
        if (this->vision_rect.cx < module_dim.w) {
            this->vision_rect.cy -= this->hscroll_height;

            this->hscroll_added = true;
        }
        else {
            this->hscroll_added = false;
        }

        if (this->vision_rect.cy < module_dim.h) {
            this->vision_rect.cx -= this->vscroll_width;

            this->vscroll_added = true;
        }
        else {
            this->vscroll_added = false;
        }

        if ((this->vision_rect.cx < module_dim.w) && !this->hscroll_added) {
            this->vision_rect.cy -= this->hscroll_height;

            this->hscroll_added = true;
        }

        if ((this->vision_rect.cy < module_dim.h) && !this->vscroll_added) {
            this->vision_rect.cx -= this->vscroll_width;

            this->vscroll_added = true;
        }
    }

    if (old_hscroll_added != this->hscroll_added) {
        if (this->hscroll_added) {
            this->add_widget(&this->hscroll);
        }
        else {
            this->remove_widget(&this->hscroll);
        }

    }
    if (old_vscroll_added != this->vscroll_added) {
        if (this->vscroll_added) {
            this->add_widget(&this->vscroll);
        }
        else {
            this->remove_widget(&this->vscroll);
        }

    }

    this->mod_visible_rect.cx = std::min<uint16_t>(
        this->vision_rect.cx, module_dim.w);
    this->mod_visible_rect.cy = std::min<uint16_t>(
        this->vision_rect.cy, module_dim.h);

    if (this->hscroll_added) {
        const unsigned int new_max_value = module_dim.w - this->vision_rect.cx;

        this->hscroll.set_max_value(new_max_value);

        if (this->mod_visible_rect.x > static_cast<int>(new_max_value)) {
            this->mod_visible_rect.x = static_cast<int>(new_max_value);
        }

        this->hscroll.set_current_value(this->mod_visible_rect.x);

        this->hscroll.set_xy(this->vision_rect.x, this->vision_rect.y + this->vision_rect.cy);
        this->hscroll.set_wh(this->vision_rect.cx, this->hscroll_height);
    }
    if (this->vscroll_added) {
        const unsigned int new_max_value = module_dim.h - this->vision_rect.cy;

        this->vscroll.set_max_value(new_max_value);

        if (this->mod_visible_rect.y > static_cast<int>(new_max_value)) {
            this->mod_visible_rect.y = static_cast<int>(new_max_value);
        }

        this->vscroll.set_current_value(this->mod_visible_rect.y);

        this->vscroll.set_xy(this->vision_rect.x + this->vision_rect.cx, this->vision_rect.y);
        this->vscroll.set_wh(this->vscroll_width, this->vision_rect.cy);
    }
}

void WidgetModuleHost::screen_copy(Rect old_rect, Rect new_rect)
{
    if (old_rect == new_rect) {
        return;
    }

    RDPScrBlt cmd(new_rect, 0xCC, old_rect.x, old_rect.y);

    Impl::get_drawable(*this).draw(cmd, new_rect);

    GCC::UserData::CSMonitor& cs_monitor = this->monitors;
    if (!cs_monitor.monitorCount) {
        cs_monitor = this->monitor_one;
    }

    SubRegion region;

    region.add_rect(old_rect);

    for (auto const & monitor_def
        : make_array_view(cs_monitor.monitorDefArray, cs_monitor.monitorCount)
    ) {
        Rect intersect_rect = old_rect.intersect(Rect(
            monitor_def.left,
            monitor_def.top,
            monitor_def.right  - monitor_def.left + 1,
            monitor_def.bottom - monitor_def.top  + 1
        ));
        if (!intersect_rect.isempty()) {
            region.subtract_rect(intersect_rect);
        }
    }

    for (const Rect & rect : region.rects) {
        this->rdp_input_invalidate(rect.offset(new_rect.x - old_rect.x, new_rect.y - old_rect.y));
    }
}

void WidgetModuleHost::set_xy(int16_t x, int16_t y)
{
    Rect old_rect = this->get_rect();

    WidgetParent::set_xy(x, y);

    this->update_rects();

    if (!old_rect.isempty()) {
        Rect new_rect = this->get_rect();

        this->screen_copy(old_rect, new_rect);
    }
    else {
        this->rdp_input_invalidate(this->get_rect());
    }
}

void WidgetModuleHost::set_wh(uint16_t w, uint16_t h)
{
    Rect old_mod_visible_rect = this->mod_visible_rect;
    Rect old_rect             = this->get_rect();

    if (this->hscroll_added) {
        old_rect.cy -= this->hscroll.cy();
    }
    if (this->vscroll_added) {
        old_rect.cx -= this->vscroll.cx();
    }

    WidgetParent::set_wh(w, h);

    this->update_rects();

    Rect new_mod_visible_rect = this->mod_visible_rect;
    Rect new_rect = this->get_rect();

    Rect intersect_mod_visible_rect = new_mod_visible_rect.intersect(old_mod_visible_rect);

    old_rect.x  += intersect_mod_visible_rect.x - old_mod_visible_rect.x;
    old_rect.y  += intersect_mod_visible_rect.y - old_mod_visible_rect.y;
    old_rect.cx  = intersect_mod_visible_rect.cx;
    old_rect.cy  = intersect_mod_visible_rect.cy;

    new_rect.x  += intersect_mod_visible_rect.x - new_mod_visible_rect.x;
    new_rect.y  += intersect_mod_visible_rect.y - new_mod_visible_rect.y;
    new_rect.cx  = intersect_mod_visible_rect.cx;
    new_rect.cy  = intersect_mod_visible_rect.cy;

    SubRegion region;

    region.add_rect(this->get_rect());

    if (!old_rect.isempty()) {
        this->screen_copy(old_rect, new_rect);

        region.subtract_rect(new_rect);
    }

    for (const Rect & rect : region.rects) {
        this->rdp_input_invalidate(rect);
    }

    if (this->hscroll_added) {
        this->hscroll.rdp_input_invalidate(this->hscroll.get_rect());
    }
    if (this->vscroll_added) {
        this->vscroll.rdp_input_invalidate(this->vscroll.get_rect());
    }
}

// NotifyApi

void WidgetModuleHost::notify(Widget* /*widget*/, NotifyApi::notify_event_t event)
{
    Widget* parentWidget = &this->parent;
    while (&parentWidget->parent != &parentWidget->parent.parent) {
        parentWidget = &parentWidget->parent;
    }

    if (event == NOTIFY_HSCROLL) {
        if (this->hscroll_added) {
            int16_t old_mod_visible_rect_x = this->mod_visible_rect.x;

            this->mod_visible_rect.x = this->hscroll.get_current_value();

            Rect visible_vision_rect = this->vision_rect.intersect(parentWidget->get_rect());
            int16_t offset_x = old_mod_visible_rect_x - this->mod_visible_rect.x;

            Rect dest_rect = visible_vision_rect.offset(offset_x, 0).intersect(visible_vision_rect);
            Rect src_rect  = dest_rect.offset(-offset_x, 0);

            if (src_rect.intersect(dest_rect).isempty()) {
                this->module_holder.rdp_input_invalidate(dest_rect.offset(
                        -this->x() + this->mod_visible_rect.x,
                        -this->y() + this->mod_visible_rect.y
                    ));
            }
            else {
                this->screen_copy(src_rect, dest_rect);

                SubRegion region;

                region.add_rect(visible_vision_rect);
                region.subtract_rect(dest_rect);

                assert(region.rects.size() == 1);

                this->module_holder.rdp_input_invalidate(region.rects[0].offset(
                        -this->x() + this->mod_visible_rect.x,
                        -this->y() + this->mod_visible_rect.y
                    ));
            }
        }
    }
    else if (event == NOTIFY_VSCROLL) {
        if (this->vscroll_added) {
            int16_t old_mod_visible_rect_y = this->mod_visible_rect.y;

            this->mod_visible_rect.y = this->vscroll.get_current_value();

            Rect visible_vision_rect = this->vision_rect.intersect(parentWidget->get_rect());
            int16_t offset_y = old_mod_visible_rect_y - this->mod_visible_rect.y;

            Rect dest_rect = visible_vision_rect.offset(0, offset_y).intersect(visible_vision_rect);
            Rect src_rect  = dest_rect.offset(0, -offset_y);

            if (src_rect.intersect(dest_rect).isempty()) {
                this->module_holder.rdp_input_invalidate(dest_rect.offset(
                        -this->x() + this->mod_visible_rect.x,
                        -this->y() + this->mod_visible_rect.y
                    ));
            }
            else {
                this->screen_copy(src_rect, dest_rect);

                SubRegion region;

                region.add_rect(visible_vision_rect);
                region.subtract_rect(dest_rect);

                assert(region.rects.size() == 1);

                this->module_holder.rdp_input_invalidate(region.rects[0].offset(
                        -this->x() + this->mod_visible_rect.x,
                        -this->y() + this->mod_visible_rect.y
                    ));
            }
        }
    }
}

// RdpInput

void WidgetModuleHost::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->drawable.begin_update();

        SubRegion region;

        region.add_rect(rect_intersect);
        if (!this->mod_visible_rect.isempty()) {
            Rect mod_vision_rect(this->vision_rect.x, this->vision_rect.y,
                this->mod_visible_rect.cx, this->mod_visible_rect.cy);
            region.subtract_rect(mod_vision_rect);
        }
        if (this->hscroll_added) {
            Rect hscroll_rect = this->hscroll.get_rect();
            if (!hscroll_rect.isempty()) {
                region.subtract_rect(hscroll_rect);
            }
        }
        if (this->vscroll_added) {
            Rect vscroll_rect = this->vscroll.get_rect();
            if (!vscroll_rect.isempty()) {
                region.subtract_rect(vscroll_rect);
            }
        }

        ::fill_region(this->drawable, region, BLACK);


        Rect mod_update_rect = clip.intersect(this->vision_rect);
        mod_update_rect = mod_update_rect.offset(-this->x() + this->mod_visible_rect.x, -this->y() + this->mod_visible_rect.y);
        if (!mod_update_rect.isempty()) {
            this->module_holder.rdp_input_invalidate(mod_update_rect);
        }

        if (this->hscroll_added) {
            this->hscroll.rdp_input_invalidate(rect_intersect);
        }
        if (this->vscroll_added) {
            this->vscroll.rdp_input_invalidate(rect_intersect);
        }

        this->drawable.end_update();
    }
}

void WidgetModuleHost::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
{
    this->current_pointer_pos_x = x;
    this->current_pointer_pos_y = y;

    if (this->vision_rect.contains_pt(x, y)) {
        this->module_holder.rdp_input_mouse(device_flags, x - this->x() + this->mod_visible_rect.x, y - this->y() + this->mod_visible_rect.y, keymap);
    }

    WidgetParent::rdp_input_mouse(device_flags, x, y, keymap);
}

void WidgetModuleHost::rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
{
    this->module_holder.rdp_input_scancode(param1, param2, param3, param4, keymap);
}

void WidgetModuleHost::rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
{
    this->module_holder.rdp_input_synchronize(time, device_flags, param1, param2);
}

// Widget

void WidgetModuleHost::refresh(Rect/* clip*/)
{
    this->update_rects();
}

void WidgetModuleHost::begin_update()
{
    Impl::get_drawable(*this).begin_update();
}

void WidgetModuleHost::end_update()
{
    Impl::get_drawable(*this).end_update();
}
