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

PrimaryDrawingOrdersSupport parse_primary_drawing_orders(char const* orders, bool bEnableLog,
    OnlyThoseSupportedByModRdp only_those_supported_by_mod_rdp) noexcept
{
    PrimaryDrawingOrdersSupport orders_support;

    bool const only_those_supported_by_mod_rdp_b = (OnlyThoseSupportedByModRdp::Yes == only_those_supported_by_mod_rdp);

    const char* extra = (only_those_supported_by_mod_rdp_b ? "Extra" : "");

    LOG_IF(bEnableLog, LOG_INFO, "RDP %sPrimaryDrawingOrders=\"%s\"", extra, orders);

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
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=DstBlt", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=DstBlt (ignored)");
            }
            else {
                orders_support.set(TS_NEG_DSTBLT_INDEX);
            }
            break;
        case TS_NEG_PATBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=PatBlt", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=PatBlt (ignored)");
            }
            else {
                orders_support.set(TS_NEG_PATBLT_INDEX);
            }
            break;
        case TS_NEG_SCRBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=ScrBlt", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=ScrBlt (ignored)");
            }
            else {
                orders_support.set(TS_NEG_SCRBLT_INDEX);
            }
            break;
        case TS_NEG_MEMBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=MemBlt", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=MemBlt (ignored)");
            }
            else {
                orders_support.set(TS_NEG_MEMBLT_INDEX);
            }
            break;
        case TS_NEG_MEM3BLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=Mem3Blt", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=Mem3Blt (ignored)");
            }
            else {
                orders_support.set(TS_NEG_MEM3BLT_INDEX);
            }
            break;
        case TS_NEG_LINETO_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=LineTo", extra);
            if (only_those_supported_by_mod_rdp_b) {
                LOG(LOG_WARNING, "RDP ExtraPrimaryDrawingOrder=LineTo (ignored)");
            }
            else {
                orders_support.set(TS_NEG_LINETO_INDEX);
            }
            break;

        case TS_NEG_MULTIDSTBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=MultiDstBlt", extra);
            orders_support.set(TS_NEG_MULTIDSTBLT_INDEX);
            break;
        case TS_NEG_MULTIPATBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=MultiPatBlt", extra);
            orders_support.set(TS_NEG_MULTIPATBLT_INDEX);
            break;
        case TS_NEG_MULTISCRBLT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=MultiScrBlt", extra);
            orders_support.set(TS_NEG_MULTISCRBLT_INDEX);
            break;
        case TS_NEG_MULTIOPAQUERECT_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=MultiOpaqueRect", extra);
            orders_support.set(TS_NEG_MULTIOPAQUERECT_INDEX);
            break;
        case TS_NEG_POLYGON_SC_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=PolygonSC", extra);
            orders_support.set(TS_NEG_POLYGON_SC_INDEX);
            break;
        case TS_NEG_POLYGON_CB_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=PolygonCB", extra);
            orders_support.set(TS_NEG_POLYGON_CB_INDEX);
            break;
        case TS_NEG_POLYLINE_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=Polyline", extra);
            orders_support.set(TS_NEG_POLYLINE_INDEX);
            break;
        case TS_NEG_ELLIPSE_SC_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=EllipseSC", extra);
            orders_support.set(TS_NEG_ELLIPSE_SC_INDEX);
            break;
        case TS_NEG_ELLIPSE_CB_INDEX:
            LOG_IF(bEnableLog, LOG_INFO, "RDP %sOrder=EllipseCB", extra);
            orders_support.set(TS_NEG_ELLIPSE_SC_INDEX);
            break;
        default:
            LOG(LOG_WARNING, "Unknown RDP %sPrimaryDrawingOrder=%ld", extra, order_number);
        }

        p = end;
        while (*p == ' ' || *p == '\t' || *p == ',') {
            ++p;
        }
    }

    return orders_support;
}
