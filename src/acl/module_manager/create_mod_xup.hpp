/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2019
  Author(s): Christophe Grosjean

*/

#pragma once

#include "mod/xup/xup.hpp"
#include "acl/mod_wrapper.hpp"

#include "transport/socket_transport.hpp"
#include "acl/mod_pack.hpp"

class XupModWithSocket final : public mod_api
{
    SocketTransport socket_transport;
public:
    xup_mod mod;
private:
    ModWrapper & mod_wrapper;
    Inifile & ini;

public:
    XupModWithSocket(ModWrapper & mod_wrapper, Inifile & ini,
        const char * name, unique_fd sck, uint32_t verbose, std::string * error_message,
        TimeBase& time_base, GraphicFdContainer & graphic_fd_events_, FrontAPI& front, uint16_t front_width, uint16_t front_height, BitsPerPixel context_bpp)
    : socket_transport( name, std::move(sck)
                     , ini.get<cfg::context::target_host>().c_str()
                     , ini.get<cfg::context::target_port>()
                     , std::chrono::milliseconds(ini.get<cfg::globals::mod_recv_timeout>())
                     , to_verbose_flags(verbose), error_message)
    , mod(this->socket_transport, time_base, graphic_fd_events_, front, front_width, front_height, context_bpp)
    , mod_wrapper(mod_wrapper)
    , ini(ini)
    {
        this->mod_wrapper.target_info_is_shown = false;
        this->mod_wrapper.set_mod_transport(&this->socket_transport);
    }

    ~XupModWithSocket()
    {
        this->mod_wrapper.set_mod_transport(nullptr);
        log_proxy::target_disconnection(
            this->ini.template get<cfg::context::auth_error_message>().c_str());
    }

    // from RdpInput
    void rdp_gdi_up_and_running(ScreenInfo &) override {}

    void rdp_gdi_down() override {}

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->mod.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    // from RdpInput
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        this->mod.rdp_input_mouse(device_flags, x, y, keymap);
    }

    // from RdpInput
    void rdp_input_unicode(uint16_t unicode, uint16_t flag) override {
        this->mod.rdp_input_unicode(unicode, flag);
    }

    // from RdpInput
    void rdp_input_invalidate(const Rect r) override
    {
        this->mod.rdp_input_invalidate(r);
    }

    // from RdpInput
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        return this->mod.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    void refresh(Rect clip) override
    {
        return this->mod.refresh(clip);
    }

    // from mod_api
    [[nodiscard]] bool is_up_and_running() const override { return false; }

    // from mod_api
    // support auto-reconnection
    bool is_auto_reconnectable() override {
        return this->mod.is_auto_reconnectable();
    }

    // from mod_api
    void disconnect() override 
    {
        return this->mod.disconnect();
    }

    // from mod_api
    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        return this->mod.move_size_widget(left, top, width, height);
    }

    // from mod_api
    bool disable_input_event_and_graphics_update(bool disable_input_event, bool disable_graphics_update) override 
    {
        return this->mod.disable_input_event_and_graphics_update(disable_input_event, disable_graphics_update);
    }

    // from mod_api
    void send_input(int time, int message_type, int device_flags, int param1, int param2) override 
    {
        return this->mod.send_input(time, message_type, device_flags, param1, param2);
    }

    // from mod_api
    [[nodiscard]] Dimension get_dim() const override 
    {
        return this->mod.get_dim();
    }

    // from mod_api
    void log_metrics() override 
    {
        return this->mod.log_metrics();
    }

    // from mod_api
    void DLP_antivirus_check_channels_files() override
    {
        return this->mod.DLP_antivirus_check_channels_files(); 
    }
    
    void send_to_mod_channel(CHANNELS::ChannelNameId /*front_channel_name*/, InStream & /*chunk*/, std::size_t /*length*/, uint32_t /*flags*/) override {}

};

