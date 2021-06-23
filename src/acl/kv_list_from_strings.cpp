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

#include "acl/kv_list_from_strings.hpp"

#include <cassert>


KVListFromStrings::KVListFromStrings(array_view<std::string> str_paires) noexcept
: strings(str_paires)
{
    assert(strings.size() % 2 == 0);
}

KVListFromStrings::Next::operator bool () const noexcept
{
    return !this->empty();
}

KVListFromStrings::Next KVListFromStrings::next() noexcept
{
    if (strings.size() > 1) {
        // WrmChunkType::SESSION_UPDATE is limited to 255 entry
        auto n = std::min(strings.size() / 2, std::size(kv_buf));
        auto first = strings.first(n * 2);
        auto* pkv = kv_buf;
        // WrmChunkType::SESSION_UPDATE packet len
        int data_len = 0;
        for (size_t i = 0, c = first.size() / 2; i < c; ++i) {
            *pkv = KVLog{first[i * 2], first[i * 2 + 1]};
            // 2 bytes for value len, 1 byte for key len
            data_len += int(pkv->key.size() + pkv->value.size()) + 3;
            // maximal size of WrmChunkType::SESSION_UPDATE
            if (data_len > 1024 * 16 - 10) {
                break;
            }
            ++pkv;
        }
        auto dist = std::size_t(pkv - kv_buf);
        assert(dist != 0);
        strings = strings.from_offset(dist * 2);
        return Next{{kv_buf, pkv}};
    }

    return Next{};
}
