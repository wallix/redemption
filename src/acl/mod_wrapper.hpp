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

#include "mod/null/null.hpp"
#include "mod/mod_api.hpp"
#include "transport/socket_transport.hpp"
#include "acl/mod_wrapper.hpp"

struct ModWrapper
{
    SocketTransport * psocket_transport;
    null_mod no_mod;
    mod_api* mod = &no_mod;

    mod_api* get_mod()
    {
        return this->mod;
    }

    bool has_mod() const {
        return (this->mod != &this->no_mod);
    }

    void remove_mod()
    {
        delete this->mod;
        this->mod = &this->no_mod;
    }

    bool is_up_and_running() const {
        return this->has_mod() && this->mod->is_up_and_running();
    }

    [[nodiscard]] mod_api const* get_mod() const
    {
        return this->mod;
    }

    // TODO: merge with set_mod() (use nullptr for not connected sockets)
    void set_psocket_transport(SocketTransport * psocket_transport)
    {
        this->psocket_transport = psocket_transport;
    }

    void set_mod(mod_api* mod)
    {
        // TODO: check we are using no_mod, otherwise it is an error
        this->mod = mod;
    }
    
    [[nodiscard]] bool has_pending_data() const
    {
        return this->psocket_transport && this->psocket_transport->has_pending_data();
    }

    [[nodiscard]] SocketTransport* get_socket() const noexcept
    {
        return this->psocket_transport;
    }

    void disable_osd()
    {
        this->mod_osd.is_disable_by_input = false;
        auto const protected_rect = this->mod_osd.get_protected_rect();
        this->mod_osd.set_protected_rect(Rect{});

        if (this->mod_osd.bogus_refresh_rect_ex) {
            this->mod->rdp_suppress_display_updates();
            this->mod->rdp_allow_display_updates(0, 0,
                this->mod_osd.client_info.screen_info.width,
                this->mod_osd.client_info.screen_info.height);
        }

        if (this->mod_osd.winapi) {
            this->mod_osd.winapi->destroy_auxiliary_window();
        }

        this->mod->rdp_input_invalidate(protected_rect);
    }

    void clear_osd_message()
    {
        if (!this->mod_osd.get_protected_rect().isempty()) {
            this->disable_osd();
        }
    }

    void osd_message_fn(std::string message, bool is_disable_by_input)
    {
        if (message != this->mod_osd.get_message()) {
            this->clear_osd_message();
        }
        if (!message.empty()) {
            this->mod_osd.set_message(std::move(message), is_disable_by_input);
            this->mod_osd.draw_osd_message();
        }
    }

    bool try_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
        (void)param1;
        (void)param2;
        (void)param3;
        (void)param4;
        if (this->mod_osd.is_disable_by_input
         && keymap->nb_kevent_available() > 0
//             && !(param3 & SlowPath::KBDFLAGS_DOWN)
         && keymap->top_kevent() == Keymap2::KEVENT_INSERT
        ) {
            keymap->get_kevent();
            this->disable_osd();
            return true;
        }
        return false;
    }

    bool try_input_mouse(int device_flags, int x, int y, Keymap2 * /*unused*/)
    {
        if (this->mod_osd.is_disable_by_input
         && this->mod_osd.get_protected_rect().contains_pt(x, y)
         && device_flags == (MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN)) {
            this->disable_osd();
            return true;
        }
        return false;
    }

    bool try_input_invalidate(const Rect r)
    {
        if (!this->mod_osd.get_protected_rect().isempty() && r.has_intersection(this->mod_osd.get_protected_rect())) {
            auto rects = gdi::subrect4(r, this->mod_osd.get_protected_rect());
            auto p = std::begin(rects);
            auto e = std::remove_if(p, std::end(rects), [](Rect const & rect) {
                return rect.isempty();
            });
            if (p != e) {
                mod_wrapper.mod->rdp_input_invalidate2({p, e});
                this->mod_osd.clip = r.intersect(this->get_protected_rect());
            }
            return true;
        }
        return false;
    }

    bool try_input_invalidate2(array_view<Rect const> vr)
    {
        // TODO PERF multi opaque rect
        bool ret = false;
        for (Rect const & r : vr) {
            if (!this->try_input_invalidate(r)) {
                this->mod->rdp_input_invalidate(r);
            }
            else {
                ret = true;
            }
        }
        return ret;
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap)
    {
        if (!this->try_input_scancode(param1, param2, param3, param4, keymap)) {
            this->mod->rdp_input_scancode(param1, param2, param3, param4, keymap);
        }
    }

    void rdp_input_unicode(uint16_t unicode, uint16_t flag)
    {
        this->mod->rdp_input_unicode(unicode, flag);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
    {
        if (!this->try_input_mouse(device_flags, x, y, keymap)) {
            this->mod->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_invalidate(Rect r)
    {
        if (!this->try_input_invalidate(r)) {
            this->mod->rdp_input_invalidate(r);
        }
    }

    void rdp_input_invalidate2(array_view<Rect const> vr)
    {
        if (!this->try_input_invalidate2(vr)) {
            this->mod->rdp_input_invalidate2(vr);
        }
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    { this->mod->rdp_input_synchronize(time, device_flags, param1, param2); }

    void rdp_input_up_and_running()
    { this->mod->rdp_input_up_and_running(); }

    void rdp_allow_display_updates(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
    { this->mod->rdp_allow_display_updates(left, top, right, bottom); }

    void rdp_suppress_display_updates()
    { this->mod->rdp_suppress_display_updates(); }

    void refresh(Rect r)
    {
        this->mod->refresh(r);
    }

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags)
    { this->mod->send_to_mod_channel(front_channel_name, chunk, length, flags); }

    void send_auth_channel_data(const char * data)
    { this->mod->send_auth_channel_data(data); }

    void send_checkout_channel_data(const char * data)
    { this->mod->send_checkout_channel_data(data); }

    [[nodiscard]] bool is_up_and_running() const
    { return this->mod->is_up_and_running(); }

    void disconnect()
    { this->mod->disconnect(); }

    void display_osd_message(std::string const & message)
    { this->mod->display_osd_message(message); }

    [[nodiscard]] Dimension get_dim() const
    {
        return this->mod->get_dim();
    }

};

