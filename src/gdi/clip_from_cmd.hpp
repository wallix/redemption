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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#pragma once

#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"

#include "utils/sugar/array_view.hpp"


inline Rect clip_from_cmd(RDPScrBlt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDPDestBlt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDPPatBlt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDPMemBlt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDPMem3Blt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDPOpaqueRect const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDP::RDPMultiScrBlt const & cmd) { return cmd.rect; }
inline Rect clip_from_cmd(RDP::RDPMultiPatBlt const & cmd) { return cmd.rect; }

inline Rect clip_from_cmd(RDPGlyphIndex const & cmd) { return cmd.bk; }

inline Rect clip_from_cmd(RDPEllipseSC const & cmd) { return cmd.el.get_rect(); }
inline Rect clip_from_cmd(RDPEllipseCB const & cmd) { return cmd.el.get_rect(); }


inline Rect clip_from_cmd(RDPMultiDstBlt const & cmd) {
    return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight);
}

inline Rect clip_from_cmd(RDPMultiOpaqueRect const & cmd) {
    return Rect(cmd.nLeftRect, cmd.nTopRect, cmd.nWidth, cmd.nHeight);
}

inline Rect clip_from_cmd(RDPLineTo const & cmd) {
    const uint16_t minx = std::min(cmd.startx, cmd.endx);
    const uint16_t miny = std::min(cmd.starty, cmd.endy);
    return Rect(
        minx, miny,
        std::max(cmd.startx, cmd.endx) - minx + 1,
        std::max(cmd.starty, cmd.endy) - miny + 1
    );
}

namespace {
    template<class RngDeltaPoint>
    inline Rect clip_from_delta_list(int16_t minx, int16_t miny, const RngDeltaPoint & delta_points) {
        int16_t maxx, maxy, previousx, previousy;

        maxx = previousx = minx;
        maxy = previousy = miny;

        for (auto & delta_point : delta_points) {
            previousx += delta_point.xDelta;
            previousy += delta_point.yDelta;

            minx = std::min(minx, previousx);
            miny = std::min(miny, previousy);

            maxx = std::max(maxx, previousx);
            maxy = std::max(maxy, previousy);
        }
        return Rect(minx, miny, maxx-minx+1, maxy-miny+1);
    }
}

inline Rect clip_from_cmd(RDPPolygonSC const & cmd) {
    return clip_from_delta_list(
        cmd.xStart, cmd.yStart,
        make_array_view(cmd.deltaPoints, cmd.NumDeltaEntries)
    );
}

inline Rect clip_from_cmd(RDPPolygonCB const & cmd) {
    return clip_from_delta_list(
        cmd.xStart, cmd.yStart,
        make_array_view(cmd.deltaPoints, cmd.NumDeltaEntries)
    );
}

inline Rect clip_from_cmd(RDPPolyline const & cmd) {
    return clip_from_delta_list(
        cmd.xStart, cmd.yStart,
        make_array_view(cmd.deltaEncodedPoints, cmd.NumDeltaEntries)
    );
}

