/*
SPDX-FileCopyrightText: 2022 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "mod/mod_api.hpp"
#include "core/callback.hpp"
#include "gdi/osd_api.hpp"
#include "gdi/protected_graphics.hpp"
#include "gdi/text_metrics.hpp"
#include "utils/ref.hpp"
#include "utils/timebase.hpp"
#include "utils/sugar/not_null_ptr.hpp"


class ClientInfo;
class ClientExecute;
class BGRPalette;
class windowing_api;
class mod_api;
class Inifile;


class ModWrapper final : public gdi::OsdApi, private Callback
{
public:
   explicit ModWrapper(
       mod_api& mod, CRef<TimeBase> time_base, CRef<BGRPalette> palette,
       gdi::GraphicApi& graphics, CRef<ClientInfo> client_info,
       CRef<Font> glyphs, CRef<ClientExecute> rail_client_execute, Inifile& ini)
    : gfilter(graphics, static_cast<RdpInput&>(*this), Rect{})
    , client_info(client_info)
    , rail_client_execute(rail_client_execute)
    , palette(palette)
    , ini(ini)
    , glyphs(glyphs)
    , modi(&mod)
    , time_base(time_base)
    {}

    Callback& get_callback() noexcept
    {
        return static_cast<Callback&>(*this);
    }

    gdi::GraphicApi& get_graphics() noexcept
    {
        return this->gfilter;
    }

    void display_osd_message(std::string_view message,
                             gdi::OsdMsgUrgency omu = gdi::OsdMsgUrgency::NORMAL) override;

    mod_api& get_mod() noexcept
    {
        return *this->modi;
    }

    mod_api const& get_mod() const noexcept
    {
        return *this->modi;
    }

    void set_time_close(MonotonicTimePoint t)
    {
        this->end_time_session = t;
    }

    void clear_osd_message(bool redraw = true);

    void set_mod(mod_api& new_mod, windowing_api* winapi, bool enable_osd);

private:
    void rdp_input_invalidate(Rect r) override;

    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override;

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override;

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        this->get_mod().rdp_input_unicode(flag, unicode);
    }

    void rdp_input_synchronize(KeyLocks locks) override
    {
        this->get_mod().rdp_input_synchronize(locks);
    }

    void rdp_gdi_up_and_running() override
    {
        this->get_mod().rdp_gdi_up_and_running();
    }

    void rdp_gdi_down() override
    {
        this->get_mod().rdp_gdi_down();
    }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom) override
    {
        this->get_mod().rdp_allow_display_updates(left, top, right, bottom);
    }

    void rdp_suppress_display_updates() override
    {
        this->get_mod().rdp_suppress_display_updates();
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags) override
    {
        this->get_mod().send_to_mod_channel(front_channel_name, chunk, length, flags);
    }

    [[nodiscard]]
    Rect get_protected_rect() const
    {
        return this->gfilter.get_protected_rect();
    }

    void set_protected_rect(Rect const rect)
    {
        this->gfilter.set_protected_rect(rect);
    }

    void draw_osd_message(bool disable_by_input);

    void disable_osd(bool redraw);


    struct Graphics final : gdi::ProtectedGraphics
    {
        using gdi::ProtectedGraphics::ProtectedGraphics;
    } gfilter;

    bool target_info_is_shown = false;
    bool enable_osd = false;
    bool is_disable_by_input = false;
    bool bogus_refresh_rect_ex = false;

    ClientInfo const& client_info;
    ClientExecute const& rail_client_execute;
    BGRPalette const& palette;

    windowing_api* winapi = nullptr;

    Inifile& ini;

    std::string osd_message;

    RDPColor color;
    RDPColor background_color;
    Font const& glyphs;

    not_null_ptr<mod_api> modi;

    MonotonicTimePoint end_time_session {};
    TimeBase const& time_base;

    gdi::MultiLineTextMetrics line_metrics;
};
