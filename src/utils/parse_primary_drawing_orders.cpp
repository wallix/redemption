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

#include "utils/parse_primary_drawing_orders.hpp"
#include "utils/sugar/chars_to_int.hpp"


PrimaryDrawingOrdersSupport parse_primary_drawing_orders(char const* orders, bool bEnableLog) noexcept
{
    PrimaryDrawingOrdersSupport primary_orders;

    LOG_IF(bEnableLog, LOG_INFO, "RDP PrimaryDrawingOrders=\"%s\"", orders);

    char const* p = orders;

    chars_to_int_result<int> result;
    while (p != (result = string_to_int<int>(p)).ptr) {
        if (REDEMPTION_UNLIKELY(result.ec != std::errc())) {
            result.val = 0xffff; // invalid value
        }

        LOG_IF(bEnableLog, LOG_INFO, "RDP OrderNumber=%d", result.val);
        char const* type = nullptr;

        switch (result.val)
        {
#define CASE(TS, Name)          \
    case TS:                    \
        type = Name;            \
        primary_orders.set(TS); \
        break

        CASE(TS_NEG_DSTBLT_INDEX, "DstBlt");
        CASE(TS_NEG_PATBLT_INDEX, "PatBlt");
        CASE(TS_NEG_SCRBLT_INDEX, "ScrBlt");
        CASE(TS_NEG_MEMBLT_INDEX, "MemBlt");
        CASE(TS_NEG_MEM3BLT_INDEX, "Mem3Blt");
        CASE(TS_NEG_LINETO_INDEX, "LineTo");
        CASE(TS_NEG_MULTIDSTBLT_INDEX, "MultiDstBlt");
        CASE(TS_NEG_MULTIPATBLT_INDEX, "MultiPatBlt");
        CASE(TS_NEG_MULTISCRBLT_INDEX, "MultiScrBlt");
        CASE(TS_NEG_MULTIOPAQUERECT_INDEX, "MultiOpaqueRect");
        CASE(TS_NEG_POLYGON_SC_INDEX, "PolygonSC");
        CASE(TS_NEG_POLYGON_CB_INDEX, "PolygonCB");
        CASE(TS_NEG_POLYLINE_INDEX, "Polyline");
        CASE(TS_NEG_ELLIPSE_SC_INDEX, "EllipseSC");
        CASE(TS_NEG_ELLIPSE_CB_INDEX, "EllipseCB");
#undef CASE
        default:;
        }

        if (type) {
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=%s", type);
        }
        else {
            LOG(LOG_WARNING, "Unknown RDP PrimaryDrawingOrder=%.*s",
                int(result.ptr - p), p);
        }

        p = result.ptr;
        while (*p == ' ' || *p == '\t' || *p == ',') {
            ++p;
        }
    }

    return primary_orders;
}
