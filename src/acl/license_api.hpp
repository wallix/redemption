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

#include "core/RDP/lic.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/noncopyable.hpp"

struct LicenseApi : noncopyable
{
    virtual ~LicenseApi() = default;

    // The functions shall return empty bytes_view to indicate the error.
    virtual bytes_view get_license_v1(char const* client_name, char const* target_ip, uint32_t version, char const* scope,
        char const* company_name, char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE>& hwid, writable_bytes_view out,
        bool enable_log) = 0;

    // The functions shall return empty bytes_view to indicate the error.
    virtual bytes_view get_license_v0(char const* client_name, uint32_t version, char const* scope,
        char const* company_name, char const* product_id, writable_bytes_view out,
        bool enable_log) = 0;

    virtual bool put_license(char const* client_name, char const* target_ip, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE> const& hwid, bytes_view in, bool enable_log) = 0;
};

struct NullLicenseStore : LicenseApi
{
    bytes_view get_license_v1(char const* client_name, char const* target_ip, uint32_t version, char const* scope,
        char const* company_name, char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE>& hwid, writable_bytes_view out,
        bool enable_log) override
    {
        (void)client_name;
        (void)target_ip;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)hwid;
        (void)enable_log;

        return bytes_view(out.data(), 0);
    }

    bytes_view get_license_v0(char const* client_name, uint32_t version, char const* scope,
        char const* company_name, char const* product_id, writable_bytes_view out,
        bool enable_log) override
    {
        (void)client_name;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)enable_log;

        return bytes_view(out.data(), 0);
    }

    bool put_license(char const* client_name, char const* target_ip, uint32_t version, char const* scope, char const* company_name,
        char const* product_id, std::array<uint8_t, LIC::LICENSE_HWID_SIZE> const& hwid, bytes_view in, bool enable_log) override
    {
        (void)client_name;
        (void)target_ip;
        (void)version;
        (void)scope;
        (void)company_name;
        (void)product_id;
        (void)hwid;
        (void)in;
        (void)enable_log;

        return false;
    }
};
