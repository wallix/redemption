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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "utils/hexadecimal_string_to_buffer.hpp"
#include "utils/sugar/chars_to_int.hpp"

#include <cassert>

bool hexadecimal_string_to_buffer(chars_view in, writable_bytes_view out) noexcept
{
    assert(in.size() <= out.size() * 2);
    assert(in.size() % 2 == 0);

    unsigned err = 0;
    uint8_t const* p = bytes_view{in}.data();
    for (uint8_t& outc : out) {
        uint8_t c1 = detail::hexadecimal_char_to_int(*p++);
        uint8_t c2 = detail::hexadecimal_char_to_int(*p++);
        err |= c1 | c2;
        outc = uint8_t((c1 << 4) | c2);
    }
    return err != 0xff;
}

bool hexadecimal_string_to_buffer(chars_view in, uint8_t* out) noexcept
{
    return hexadecimal_string_to_buffer(in, {out, in.size() / 2});
}
