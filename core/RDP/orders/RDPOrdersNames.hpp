/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Common parts and constants

*/

#ifndef _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSNAMES_HPP_
#define _REDEMPTION_CORE_RDP_ORDERS_RDPORDERSNAMES_HPP_

#include "RDP/orders/RDPOrdersCommon.hpp"

namespace RDP {
    static const char * ordernames[28] = {
        "DESTBLT",
        "PATBLT",
        "SCREENBLT",
        "ORDER3",
        "ORDER4",
        "ORDER5",
        "ORDER6",
        "ORDER7",
        "ORDER8",
        "LINE",
        "RECT",
        "DESKSAVE",
        "ORDER12",
        "MEMBLT",
        "TRIBLT",
        "ORDER15",
        "ORDER16",
        "ORDER17",
        "ORDER18",
        "ORDER19",
        "ORDER20",
        "ORDER21",
        "POLYLINE",
        "ORDER23",
        "ORDER24",
        "ORDER25",
        "ORDER26",
        "GLYPHINDEX"
    };

} /* namespace */

#endif
