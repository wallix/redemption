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

#include "utils/rect.hpp"

class RDPScrBlt;
class RDPDestBlt;
class RDPPatBlt;
class RDPMemBlt;
class RDPMem3Blt;
class RDPOpaqueRect;

namespace RDP
{
    class RDPMultiScrBlt;
    class RDPMultiPatBlt;
}

class RDPGlyphIndex;

class RDPEllipseSC;
class RDPEllipseCB;

class RDPMultiDstBlt;
class RDPMultiOpaqueRect;
class RDPLineTo;

class RDPPolygonSC;
class RDPPolygonCB;
class RDPPolyline;


Rect clip_from_cmd(RDPScrBlt const & cmd);
Rect clip_from_cmd(RDPDestBlt const & cmd);
Rect clip_from_cmd(RDPPatBlt const & cmd);
Rect clip_from_cmd(RDPMemBlt const & cmd);
Rect clip_from_cmd(RDPMem3Blt const & cmd);
Rect clip_from_cmd(RDPOpaqueRect const & cmd);
Rect clip_from_cmd(RDP::RDPMultiScrBlt const & cmd);
Rect clip_from_cmd(RDP::RDPMultiPatBlt const & cmd);

Rect clip_from_cmd(RDPGlyphIndex const & cmd);

Rect clip_from_cmd(RDPEllipseSC const & cmd);
Rect clip_from_cmd(RDPEllipseCB const & cmd);

Rect clip_from_cmd(RDPMultiDstBlt const & cmd);
Rect clip_from_cmd(RDPMultiOpaqueRect const & cmd);
Rect clip_from_cmd(RDPLineTo const & cmd);

Rect clip_from_cmd(RDPPolygonSC const & cmd);
Rect clip_from_cmd(RDPPolygonCB const & cmd);
Rect clip_from_cmd(RDPPolyline const & cmd);

