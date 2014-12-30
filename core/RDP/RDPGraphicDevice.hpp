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
   Copyright (C) Wallix 2011-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.
*/

#ifndef _REDEMPTION_CORE_RDP_RDPGRAPHICDEVICE_HPP_
#define _REDEMPTION_CORE_RDP_RDPGRAPHICDEVICE_HPP_

#include <cstddef>

#include "noncopyable.hpp"
#include "log.hpp"

class BGRPalette;
class RDPDestBlt;
class RDPMultiDstBlt;
class RDPPatBlt;
class RDPOpaqueRect;
class RDPMultiOpaqueRect;
class RDPScrBlt;
class RDPMemBlt;
class RDPMem3Blt;
class RDPLineTo;
class RDPGlyphIndex;
class RDPPolygonSC;
class RDPPolygonCB;
class RDPPolyline;
class RDPEllipseSC;
class RDPEllipseCB;
class RDPColCache;
class RDPGlyphCache;
class RDPBrushCache;

class RDPBitmapData;
class Pointer;
class Rect;
class Bitmap;
class GlyphCache;

namespace RDP {
    class RDPMultiPatBlt;
    class RDPMultiScrBlt;
    class FrameMarker;
}

struct RDPGraphicDevice : noncopyable {
    virtual void draw(const RDPDestBlt          & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPMultiDstBlt      & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPPatBlt           & cmd, const Rect & clip) = 0;
    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPOpaqueRect       & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPMultiOpaqueRect  & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPScrBlt           & cmd, const Rect & clip) = 0;
    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPMemBlt           & cmd, const Rect & clip, const Bitmap & bmp) = 0;
    virtual void draw(const RDPMem3Blt          & cmd, const Rect & clip, const Bitmap & bmp) = 0;
    virtual void draw(const RDPLineTo           & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPGlyphIndex       & cmd, const Rect & clip, const GlyphCache * gly_cache) = 0;
    virtual void draw(const RDPPolygonSC        & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPPolygonCB        & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPPolyline         & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPEllipseSC        & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPEllipseCB        & cmd, const Rect & clip) = 0;

    TODO("The 3 methods below should not exist and cache access be done before calling drawing orders")
    virtual void draw(const RDPColCache   & cmd) {}
    virtual void draw(const RDPBrushCache & cmd) {}

    virtual void draw(const RDP::FrameMarker & order) = 0;

    virtual void draw( const RDPBitmapData & bitmap_data, const uint8_t * data, std::size_t size
                     , const Bitmap & bmp) = 0;

    virtual void server_set_pointer(const Pointer & cursor) = 0;

    virtual void set_mod_palette(const BGRPalette & palette) {};

    virtual void flush() = 0;

protected:
    // this to avoid calling constructor of base abstract class
    RDPGraphicDevice() {}

public:
    // we choosed to make destructor virtual to allow destructing object
    // through pointer of base class. As this class is interface only
    // it does not looks really usefull.
    virtual ~RDPGraphicDevice() {}
};

#endif
