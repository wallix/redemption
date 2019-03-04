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

#include "mod/rdp/parse_extra_orders.hpp"

PrimaryDrawingOrdersSupport parse_extra_orders(char const* extra_orders, RDPVerbose rdp_verbose) noexcept
{
    PrimaryDrawingOrdersSupport orders_support;

    LOG_IF(bool(rdp_verbose & RDPVerbose::basic_trace), LOG_INFO,
        "RDP Extra orders=\"%s\"", extra_orders);

    const bool verbose = bool(rdp_verbose & RDPVerbose::capabilities);

    char * end;
    char const * p = extra_orders;
    for (int order_number = std::strtol(p, &end, 0);
        p != end;
        order_number = std::strtol(p, &end, 0))
    {
        LOG_IF(verbose, LOG_INFO, "RDP Extra orders number=%d", order_number);

        switch (order_number)
        {
        case TS_NEG_MULTIDSTBLT_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=MultiDstBlt");
            orders_support.set(TS_NEG_MULTIDSTBLT_INDEX);
            break;
        case TS_NEG_MULTIOPAQUERECT_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=MultiOpaqueRect");
            orders_support.set(TS_NEG_MULTIOPAQUERECT_INDEX);
            break;
        case TS_NEG_MULTIPATBLT_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=MultiPatBlt");
            orders_support.set(TS_NEG_MULTIPATBLT_INDEX);
            break;
        case TS_NEG_MULTISCRBLT_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=MultiScrBlt");
            orders_support.set(TS_NEG_MULTISCRBLT_INDEX);
            break;
        case TS_NEG_POLYGON_SC_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=PolygonSC");
            orders_support.set(TS_NEG_POLYGON_SC_INDEX);
            break;
        case TS_NEG_POLYGON_CB_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=PolygonCB");
            orders_support.set(TS_NEG_POLYGON_CB_INDEX);
            break;
        case TS_NEG_POLYLINE_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=Polyline");
            orders_support.set(TS_NEG_POLYLINE_INDEX);
            break;
        case TS_NEG_ELLIPSE_SC_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=EllipseSC");
            orders_support.set(TS_NEG_ELLIPSE_SC_INDEX);
            break;
        case TS_NEG_ELLIPSE_CB_INDEX:
            LOG_IF(verbose, LOG_INFO, "RDP Extra orders=EllipseCB");
            orders_support.set(TS_NEG_ELLIPSE_CB_INDEX);
            break;
        default:
            LOG_IF(verbose, LOG_INFO, "RDP Unknown Extra orders");
        }

        p = end;
        while (*p == ' ' || *p == '\t' || *p == ',') {
            ++p;
        }
    }

    return orders_support;
}
