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

#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "mod/internal/widget/scroll.hpp"
#include "mod/internal/widget/composite.hpp"
#include "mod/mod_api.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/sugar/not_null_ptr.hpp"

#include <memory>

class WidgetModuleHost : public WidgetParent, public gdi::GraphicApi
{
public:
    WidgetModuleHost(
        gdi::GraphicApi& drawable, Widget& parent,
        NotifyApi* notifier,
        /*TODO not_null_ptr<>*/ std::unique_ptr<mod_api>&& managed_mod, Font const & font,
        const GCC::UserData::CSMonitor& cs_monitor,
        uint16_t front_width, uint16_t front_height,
        int group_id = 0); /*NOLINT*/

    gdi::GraphicApi& proxy_gd(gdi::GraphicApi& gd);

    void draw(RDP::FrameMarker    const & cmd) override;
    void draw(RDPDestBlt          const & cmd, Rect clip) override;
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override;
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPScrBlt           const & cmd, Rect clip) override;
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override;
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override;
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override;
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override;
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override;
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override;
    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {}
    void draw(RDPSetSurfaceCommand const & /*cmd*/, RDPSurfaceContent const &/*content*/) override {}

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override;
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override;
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override;
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override;
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override;
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override;
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override;
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override;

    void draw(RDPColCache   const & cmd) override;
    void draw(RDPBrushCache const & cmd) override;

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override;

    mod_api& get_managed_mod()
    {
        return this->module_holder;
    }

    [[nodiscard]] const mod_api& get_managed_mod() const
    {
        return this->module_holder;
    }

    void set_xy(int16_t x, int16_t y) override;

    void set_wh(uint16_t w, uint16_t h) override;

    using WidgetParent::set_wh;

    [[nodiscard]] const Pointer* get_pointer() const override
    {
        return &this->current_pointer;
    }

    // NotifyApi

    void notify(Widget* /*widget*/, NotifyApi::notify_event_t event) override;

    // RdpInput

    void rdp_input_invalidate(Rect clip) override;

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override;

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override;

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override;

    // Widget

    void refresh(Rect/* clip*/) override;

    void begin_update() override;

    void end_update() override;

private:
    class Impl;

    void update_rects();

    void screen_copy(Rect old_rect, Rect new_rect);


    class ModuleHolder : public mod_api
    {
    private:
        const std::unique_ptr<mod_api> managed_mod;

    public:
        ModuleHolder(/*TODO not_null_ptr<>*/ std::unique_ptr<mod_api>&& managed_mod);

        std::string module_name() override {return "Module Holder []";}

        // Callback
        void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name,
                                 InStream& chunk, size_t length,
                                 uint32_t flags) override;

        void create_shadow_session(const char * userdata, const char * type) override;
        void send_auth_channel_data(const char * data) override;
        void send_checkout_channel_data(const char * data) override;

        // mod_api

        [[nodiscard]] bool is_up_and_running() const override;

        bool is_auto_reconnectable() const override;

        bool server_error_encountered() const override;

        // RdpInput

        void rdp_input_invalidate(Rect r) override;

        void rdp_input_mouse(int device_flags, int x, int y,
                             Keymap2* keymap) override;

        void rdp_input_scancode(long param1, long param2, long param3,
                                long param4, Keymap2* keymap) override;

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags,
                                   int16_t param1, int16_t param2) override;

        void rdp_gdi_up_and_running() override;

        void rdp_gdi_down() override;

        void refresh(Rect r) override;

        [[nodiscard]] Dimension get_dim() const override;
    } module_holder;

    CompositeArray composite_array;

    not_null_ptr<gdi::GraphicApi> drawable_ptr;

    WidgetScrollBar hscroll;
    WidgetScrollBar vscroll;

    unsigned int hscroll_height = 0;
    unsigned int vscroll_width  = 0;

    bool hscroll_added = false;
    bool vscroll_added = false;

    Rect mod_visible_rect;

    Rect vision_rect;

    GCC::UserData::CSMonitor monitors;

    GCC::UserData::CSMonitor monitor_one;

    Pointer current_pointer;

    int current_pointer_pos_x = 0;
    int current_pointer_pos_y = 0;
};
