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

struct AuthApi : noncopyable
{
    virtual void set_auth_channel_target(const char * target) = 0;

    virtual void set_auth_error_message(const char * error_message) = 0;

    virtual void disconnect_target() = 0;

    virtual void new_remote_mod() = 0;
    virtual void delete_remote_mod() = 0;

    virtual void set_pm_request(const char * request) = 0;

    virtual ~AuthApi() = default;
};


struct NullAuthentifier : AuthApi
{
    void set_auth_channel_target(const char * target) override
    {
        (void)target;
    }

    void set_auth_error_message(const char * error_message) override
    {
        (void)error_message;
    }

    void disconnect_target() override
    {}

    void new_remote_mod() override
    {}

    void delete_remote_mod() override
    {}

    void set_pm_request(const char * request) override
    {
        (void)request;
    }
};
