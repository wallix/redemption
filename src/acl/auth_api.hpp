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
    Copyright (C) Wallix 2017
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "utils/sugar/noncopyable.hpp"
#include "gdi/screen_info.hpp"

#include <cstdint>
#include <string>

struct AuthApi : noncopyable
{
    virtual void set_rd_shadow_available() = 0;

    virtual void set_rd_shadow_invitation(uint32_t error_code, const char * error_message, const char * request, const char * id, const char * addr, uint16_t port) = 0;

    virtual void set_smartcard_login(const char * login) = 0;

    virtual void set_server_cert(std::string const& blob_str) = 0;

    virtual void set_screen_info(ScreenInfo screen_info) = 0;

    virtual void set_auth_info(std::string const& username, std::string const& domain, std::string const& password) = 0;

    virtual void set_recording_started() = 0;

    virtual void set_rt_ready() = 0;

    virtual void set_native_session_id(unsigned int session_id) = 0;

    virtual void set_pm_request(const char * request) = 0;

    virtual void set_disconnect_target() = 0;

    virtual void set_auth_error_message(const char * error_message) = 0;

    virtual void set_auth_channel_target(const char * target) = 0;

    virtual ~AuthApi() = default;

};


struct NullAuthentifier : AuthApi
{
    void set_pm_request(const char * /*request*/) override {}
    void set_disconnect_target() override {}
    void set_auth_error_message(const char * /*error_message*/) override {}
    void set_auth_channel_target(const char * /*target*/) override {}
    void set_native_session_id(unsigned int /*session_id*/) override {}
    void set_rd_shadow_available() override {}
    void set_rd_shadow_invitation(uint32_t /*error_code*/, const char * /*error_message*/, const char * /*userdata*/, const char * /*id*/, const char * /*addr*/, uint16_t /*port*/) override {}
    void set_smartcard_login(const char * /*login*/) override {}
    void set_server_cert(std::string const& /*blob_str*/) override {}
};
