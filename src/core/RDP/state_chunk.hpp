/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/

#pragma once


#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
// TODO missing RDP orders
// include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
// include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
// include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"

struct StateChunk {
    RDPOrderCommon          common;
    RDPDestBlt              destblt;
    RDPMultiDstBlt          multidstblt;
    RDPMultiOpaqueRect      multiopaquerect;
    RDP::RDPMultiPatBlt     multipatblt;
    RDP::RDPMultiScrBlt     multiscrblt;
    RDPPatBlt               patblt;
    RDPScrBlt               scrblt;
    RDPOpaqueRect           opaquerect;
    RDPMemBlt               memblt;
    RDPMem3Blt              mem3blt;
    RDPLineTo               lineto;
    RDPGlyphIndex           glyphindex;
    RDPPolyline             polyline;
    RDPEllipseSC            ellipseSC;

    StateChunk()
    : common(RDP::PATBLT, Rect(0, 0, 1, 1))
    , destblt(Rect(), 0)
    , multidstblt()
    , multiopaquerect()
    , multipatblt()
    , multiscrblt()
    , patblt(Rect(), 0, RDPColor{}, RDPColor{}, RDPBrush())
    , scrblt(Rect(), 0, 0, 0)
    , opaquerect(Rect(), RDPColor{})
    , memblt(0, Rect(), 0, 0, 0, 0)
    , mem3blt(0, Rect(), 0, 0, 0, RDPColor{}, RDPColor{}, RDPBrush(), 0)
    , lineto(0, 0, 0, 0, 0, RDPColor{}, 0, RDPPen(0, 0, RDPColor{}))
    , glyphindex( 0, 0, 0, 0, RDPColor{}, RDPColor{}, Rect(0, 0, 1, 1), Rect(0, 0, 1, 1), RDPBrush(), 0, 0, 0
                , byte_ptr_cast(""))
    , polyline()
    , ellipseSC()
    {}
};
