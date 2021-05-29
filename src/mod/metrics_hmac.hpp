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

#include "system/ssl_sha256.hpp"
#include "gdi/screen_info.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/strutils.hpp"

#include <array>
#include <string>

#include <cstdio>


class MetricsHmacSha256Encrypt
{
    std::array<char, SslSha256::DIGEST_LENGTH*2+1> dest;

public:
    MetricsHmacSha256Encrypt(bytes_view src, bytes_view key_crypt)
    {
        SslHMAC_Sha256 sha256(key_crypt);
        sha256.update(src);
        uint8_t sig[SslSha256::DIGEST_LENGTH];
        sha256.final(make_writable_sized_array_view(sig));

        static_assert(sizeof(sig) * 2 + 1 == sizeof(dest));

        auto p = std::begin(dest);
        for (uint8_t c : sig) {
            p = int_to_fixed_hexadecimal_upper_chars(p, c);
        }
        *p = 0;
    }

    operator chars_view() const noexcept
    {
        return this->av();
    }

    [[nodiscard]] chars_view av() const noexcept
    {
        return {dest.data(), dest.size()-1u};
    }
};


inline MetricsHmacSha256Encrypt hmac_user(
    chars_view user, chars_view key)
{
    return MetricsHmacSha256Encrypt(user, key);
}

inline MetricsHmacSha256Encrypt hmac_account(
    chars_view account, chars_view key)
{
    return MetricsHmacSha256Encrypt(account, key);
}

inline MetricsHmacSha256Encrypt hmac_device_service(
    chars_view device, std::string service, chars_view key)
{
    str_append(service, ' ', device);
    return MetricsHmacSha256Encrypt(service, key);
}

inline MetricsHmacSha256Encrypt hmac_client_info(
    std::string client_host, ScreenInfo const& info,
    chars_view key)
{
    char session_info[128];
    int session_info_size = ::snprintf(session_info, sizeof(session_info), "%d%u%u",
        underlying_cast(info.bpp), info.width, info.height);
    client_host.append(session_info, session_info_size);
    return MetricsHmacSha256Encrypt(client_host, key);
}
