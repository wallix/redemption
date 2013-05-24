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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   RDPGraphicDevice is an abstract class that describe a device able to
   proceed RDP Drawing Orders. How the drawing will be actually done
   depends on the implementation.
   - It may be sent on the wire,
   - Used to draw on some internal bitmap,
   - etc.

*/

#ifndef _REDEMPTION_CORE_RDP_RDPGRAPHICDEVICE_HPP_
#define _REDEMPTION_CORE_RDP_RDPGRAPHICDEVICE_HPP_
#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"

struct RDPGraphicDevice
{
    virtual void set_row(size_t rownum, const uint8_t * data) {}
    
    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) = 0;
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPDestBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPPatBlt & cmd, const Rect &clip) = 0;
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) = 0;
    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) = 0;
    virtual void draw(const RDPLineTo& cmd, const Rect & clip) = 0;
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) = 0;

    TODO("The 3 methods below should not exist and cache access be done before calling drawing orders")
    virtual void draw(const RDPBrushCache & cmd) {};
    virtual void draw(const RDPColCache & cmd) {};
    virtual void draw(const RDPGlyphCache & cmd) {};

    virtual void input(const timeval & now, Stream & input_data_32) {};

    virtual void snapshot(const timeval & now, int x, int y, bool pointer_already_displayed, bool no_timestamp) {}

protected:
    // this to avoid calling constructor of base abstract class
    RDPGraphicDevice() {}

public:
    // we choosed to make destructor virtual to allow destructing object
    // through pointer of base class. As this class is interface only
    // it does not looks really usefull.
    virtual ~RDPGraphicDevice() {};
};

#endif
