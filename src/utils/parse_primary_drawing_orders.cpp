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

PrimaryDrawingOrdersSupport parse_primary_drawing_orders(char const* orders, bool bEnableLog) noexcept
{
    PrimaryDrawingOrdersSupport primary_orders;

    LOG_IF(bEnableLog, LOG_INFO, "RDP PrimaryDrawingOrders=\"%s\"", orders);

    char * end;
    char const * p = orders;
    for (long order_number = std::strtol(p, &end, 0);
        p != end;
        order_number = std::strtol(p, &end, 0))
    {
        LOG_IF(bEnableLog, LOG_INFO, "RDP OrderNumber=%ld", order_number);

        switch (order_number)
        {
        case TS_NEG_DSTBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=DstBlt");

            primary_orders.set(TS_NEG_DSTBLT_INDEX);
            break;
        case TS_NEG_PATBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=PatBlt");

            primary_orders.set(TS_NEG_PATBLT_INDEX);
            break;
        case TS_NEG_SCRBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=ScrBlt");

            primary_orders.set(TS_NEG_SCRBLT_INDEX);
            break;
        case TS_NEG_MEMBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=MemBlt");

            primary_orders.set(TS_NEG_MEMBLT_INDEX);
            break;
        case TS_NEG_MEM3BLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=Mem3Blt");

            primary_orders.set(TS_NEG_MEM3BLT_INDEX);
            break;
        case TS_NEG_LINETO_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=LineTo");

            primary_orders.set(TS_NEG_LINETO_INDEX);
            break;

        case TS_NEG_MULTIDSTBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=MultiDstBlt");

            primary_orders.set(TS_NEG_MULTIDSTBLT_INDEX);
            break;
        case TS_NEG_MULTIPATBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=MultiPatBlt");

            primary_orders.set(TS_NEG_MULTIPATBLT_INDEX);
            break;
        case TS_NEG_MULTISCRBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=MultiScrBlt");

            primary_orders.set(TS_NEG_MULTISCRBLT_INDEX);
            break;
        case TS_NEG_MULTIOPAQUERECT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=MultiOpaqueRect");

            primary_orders.set(TS_NEG_MULTIOPAQUERECT_INDEX);
            break;
        case TS_NEG_POLYGON_SC_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=PolygonSC");

            primary_orders.set(TS_NEG_POLYGON_SC_INDEX);
            break;
        case TS_NEG_POLYGON_CB_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=PolygonCB");

            primary_orders.set(TS_NEG_POLYGON_CB_INDEX);
            break;
        case TS_NEG_POLYLINE_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=Polyline");

            primary_orders.set(TS_NEG_POLYLINE_INDEX);
            break;
        case TS_NEG_ELLIPSE_SC_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=EllipseSC");

            primary_orders.set(TS_NEG_ELLIPSE_SC_INDEX);
            break;
        case TS_NEG_ELLIPSE_CB_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP Order=EllipseCB");

            primary_orders.set(TS_NEG_ELLIPSE_SC_INDEX);
            break;
        default:
            LOG(LOG_WARNING, "Unknown RDP PrimaryDrawingOrder=%ld", order_number);
        }

        p = end;
        while (*p == ' ' || *p == '\t' || *p == ',') {
            ++p;
        }
    }

    return primary_orders;
}
