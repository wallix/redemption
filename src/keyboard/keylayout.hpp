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

#pragma once

#include "utils/sugar/zstring_view.hpp"
#include "utils/sugar/bounded_array_view.hpp"

#include <cstdint>


//====================================
// SCANCODES PHYSICAL LAYOUT REFERENCE
//====================================
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

// http://kbdlayout.info/


struct KeyLayout
{
    static KeyLayout const& null_layout() noexcept;

    enum class KbdId : uint32_t;

    enum class RCtrlIsCtrl : bool;

    using unicode_t = uint16_t;

    static constexpr unicode_t DK = unicode_t(1) << (sizeof(unicode_t) * 8 - 1);

    struct DKeyTable
    {
        struct DKey
        {
            unicode_t second;
            unicode_t result;
        };

        struct Meta
        {
            uint16_t size;
            unicode_t accent;
        };

        union Data
        {
            Meta meta;
            DKey dkey;
        };

        Data const* data;

        explicit operator bool () const noexcept
        {
            return data;
        }

        unicode_t accent() const noexcept
        {
            return data[0].meta.accent;
        }

        array_view<DKey> dkeys() const noexcept
        {
            static_assert(sizeof(DKey) == sizeof(Data));
            static_assert(alignof(DKey) == alignof(Data));
            return array_view{reinterpret_cast<DKey const*>(&data[1]), data[0].meta.size};
        }

        unicode_t find_composition(unicode_t unicode) const noexcept
        {
            for (auto& dkey : dkeys()) {
                if (dkey.second == unicode) {
                    return dkey.result;
                }
            }
            return 0;
        }
    };

    struct Mods
    {
        enum : unsigned
        {
            Shift,
            Control,
            Menu,
            NumLock,
            CapsLock,
            OEM_8,
        };
    };

    KbdId kbdid;
    RCtrlIsCtrl right_ctrl_is_ctrl;
    zstring_view name;

    sized_array_view<sized_array_view<unicode_t, 256>, 64> keymap_by_mod;
    sized_array_view<sized_array_view<DKeyTable, 128>, 64> dkeymap_by_mod;
};


// null_layout() implementation

namespace detail
{
    inline constexpr KeyLayout::unicode_t null_layout_unicodes[256] {};
    inline constexpr KeyLayout::DKeyTable null_layout_keytables[128] {};
    inline constexpr sized_array_view<KeyLayout::unicode_t, 256> null_layout_unicode_by_mods[]
    {
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes, null_layout_unicodes, null_layout_unicodes,
        null_layout_unicodes,
    };
    inline constexpr sized_array_view<KeyLayout::DKeyTable, 128> null_layout_keytable_by_mods[]
    {
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables, null_layout_keytables, null_layout_keytables,
        null_layout_keytables,
    };

    inline constexpr KeyLayout null_layout_layout{
        KeyLayout::KbdId(0),
        KeyLayout::RCtrlIsCtrl(true),
        "null"_zv,
        detail::null_layout_unicode_by_mods,
        detail::null_layout_keytable_by_mods
    };
}

inline KeyLayout const& KeyLayout::null_layout() noexcept
{
    return detail::null_layout_layout;
}
