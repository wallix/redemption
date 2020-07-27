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
Copyright (C) Wallix 2010-2020
*/

#include "acl/module_manager/acl_new_line_converter.hpp"

#include <cstring>


namespace
{
    struct TbIdx
    {
        int8_t tb[255] {0};

        constexpr TbIdx()
        {
            for (auto& n : tb) {
                n = 4;
            }
            tb[int('<')] = 0;
            tb[int('b')] = -1;
            tb[int('r')] = -2;
            tb[int('>')] = -3;
        }
    };

    inline constexpr TbIdx tbidx {};
}

AclNewLineConverter::AclNewLineConverter(zstring_view msg)
: msg(msg)
{
    constexpr std::size_t needle_len = 4;

    if (msg.size() >= needle_len) {
        char const* p = msg.data();
        char const* end = p + msg.size() - needle_len;

        auto next = [end](char const* p){
            do {
                if (p[0] == '<' && p[1] == 'b' && p[2] == 'r' && p[3] == '>' ) {
                    return p;
                }
                p += needle_len;
                p += tbidx.tb[int(*p)];
            } while (p <= end);
            return p;
        };

        auto previous_p = p;
        p = next(p);

        if (p > end) {
            return ;
        }

        this->gc = std::unique_ptr<char[]>(new char[msg.size()]); /*NOLINT*/
        char* dst = this->gc.get();

        for (;;) {
            auto n = p - previous_p;
            std::memcpy(dst, previous_p, n);
            dst += n;
            *dst++ = '\n';

            if (p <= end) {
                p += needle_len;
                previous_p = p;
                p = next(p);
            }

            if (p > end) {
                n = msg.end() - previous_p;
                std::memcpy(dst, previous_p, n);
                dst += n;
                *dst = '\0';
                this->msg = zstring_view(zstring_view::is_zero_terminated{}, {this->gc.get(), dst});
                return;
            }
        }
    }
}
