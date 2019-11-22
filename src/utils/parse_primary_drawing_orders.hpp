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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#pragma once

#include "core/RDP/capabilities/order.hpp"

enum class OnlyThoseSupportedByModRdp : bool { No, Yes };

PrimaryDrawingOrdersSupport parse_primary_drawing_orders(char const* orders, bool bEnableLog,
    OnlyThoseSupportedByModRdp only_those_supported_by_mod_rdp) noexcept;

inline static constexpr auto order_indexes_supported() noexcept
{
    // Apparently, these primary drawing orders are supported
    // by both rdesktop and xfreerdp :
    // TS_NEG_DSTBLT_INDEX
    // TS_NEG_PATBLT_INDEX
    // TS_NEG_SCRBLT_INDEX
    // TS_NEG_MEMBLT_INDEX
    // TS_NEG_LINETO_INDEX
    // others orders may not be supported.

    return [](auto... xs) noexcept {
        return std::array<OrdersIndexes, sizeof...(xs)>{xs...};
    }(
        TS_NEG_DSTBLT_INDEX,
        TS_NEG_PATBLT_INDEX,
        TS_NEG_SCRBLT_INDEX,
        TS_NEG_MEMBLT_INDEX,
        TS_NEG_MEM3BLT_INDEX,
        // TS_NEG_DRAWNINEGRID_INDEX,
        TS_NEG_LINETO_INDEX,
        // TS_NEG_MULTI_DRAWNINEGRID_INDEX,
        // TS_NEG_SAVEBITMAP_INDEX,
        TS_NEG_MULTIDSTBLT_INDEX,
        TS_NEG_MULTIPATBLT_INDEX,
        TS_NEG_MULTISCRBLT_INDEX,
        TS_NEG_MULTIOPAQUERECT_INDEX,
        // TS_NEG_FAST_GLYPH_INDEX,
        TS_NEG_POLYGON_SC_INDEX,
        TS_NEG_POLYGON_CB_INDEX,
        TS_NEG_POLYLINE_INDEX,
        // TS_NEG_FAST_GLYPH_INDEX,
        TS_NEG_ELLIPSE_SC_INDEX,
        TS_NEG_ELLIPSE_CB_INDEX,
        TS_NEG_GLYPH_INDEX
    );
}
