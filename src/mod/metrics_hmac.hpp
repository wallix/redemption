/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2018
*   Author(s): Clément Moroldo
*/

#pragma once

#include "system/linux/system/ssl_sha256.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/bytes_view.hpp"

#include <array>
#include <string>
#include <cstdio>


class MetricsHmacSha256Encrypt
{
    std::array<char, SslSha256::DIGEST_LENGTH*2+1> dest;

public:
    MetricsHmacSha256Encrypt(const_bytes_view src, const_bytes_view key_crypt)
    {
        SslHMAC_Sha256 sha256(key_crypt.data(), key_crypt.size());
        sha256.update(src.data(), src.size());
        uint8_t sig[SslSha256::DIGEST_LENGTH];
        sha256.final(sig);

        static_assert(sizeof(sig) * 2 + 1 == sizeof(dest));

        const char * hex = "0123456789ABCDEF";
        auto p = std::begin(dest);
        for (uint8_t c : sig) {
            *p = hex[(c>>4) & 0xF];
            ++p;
            *p = hex[c & 0xF];
            ++p;
        }
        *p = 0;
    }

    operator array_view_const_char() const noexcept
    {
        return this->av();
    }

    array_view_const_char av() const noexcept
    {
        return {dest.data(), dest.size()-1u};
    }
};


inline MetricsHmacSha256Encrypt hmac_user(array_view_const_char user, array_view_const_char key)
{
    return MetricsHmacSha256Encrypt(user, key);
}

inline MetricsHmacSha256Encrypt hmac_account(array_view_const_char account, array_view_const_char key)
{
    return MetricsHmacSha256Encrypt(account, key);
}

inline MetricsHmacSha256Encrypt hmac_device_service(array_view_const_char device, array_view_const_char service, array_view_const_char key)
{
	std::string device_service(service.data(), service.size());
    device_service += " ";
    device_service.append(device.data(), device.size());
    return MetricsHmacSha256Encrypt(service, key);
}

inline MetricsHmacSha256Encrypt hmac_client_info(std::string client_host, unsigned bpp, unsigned width, unsigned height, array_view_const_char key)
{
    char session_info[128];
    int session_info_size = ::snprintf(session_info, sizeof(session_info), "%u%u%u",
        bpp, width, height);
    client_host.append(std::string(session_info, session_info_size));
    return MetricsHmacSha256Encrypt(client_host, key);
}


