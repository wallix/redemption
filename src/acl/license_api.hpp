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

#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/noncopyable.hpp"

struct LicenseApi : noncopyable
{
    virtual ~LicenseApi() = default;

    // The functions shall return empty bytes_view to indicate the error.
    virtual bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, writable_bytes_view out, bool enable_log) = 0;

    virtual bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, bytes_view in, bool enable_log) = 0;
};

struct NullLicenseStore : LicenseApi
{
    bytes_view get_license(char const* client_name, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, writable_bytes_view out, bool enable_log) override
    {
        (void)client_name;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)enable_log;

        return bytes_view(out.data(), 0);
    }

    bool put_license(char const* client_name, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, bytes_view in, bool enable_log) override
    {
        (void)client_name;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)in;
        (void)enable_log;

        return false;
    }
};
