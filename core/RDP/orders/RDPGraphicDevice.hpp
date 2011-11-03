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

#if !defined(__RDP_GRAPHIC_DEVICE_HPP__)
#define __RDP_GRAPHIC_DEVICE_HPP__

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"

struct RDPGraphicDevice
{
    RDPGraphicDevice() {}
    ~RDPGraphicDevice() {}
    virtual void flush() = 0;
    virtual void send(const RDPOpaqueRect & cmd, const Rect & clip) = 0;
    virtual void send(const RDPScrBlt & cmd, const Rect &clip) = 0;
    virtual void send(const RDPDestBlt & cmd, const Rect &clip) = 0;
    virtual void send(const RDPPatBlt & cmd, const Rect &clip) = 0;
    virtual void send(const RDPMemBlt & cmd, const Rect & clip) = 0;
    virtual void send(const RDPLineTo& cmd, const Rect & clip) = 0;
    virtual void send(const RDPGlyphIndex & cmd, const Rect & clip) = 0;
    virtual void send(const RDPBrushCache & cmd) = 0;
    virtual void send(const RDPColCache & cmd) = 0;
    virtual void send(const RDPBmpCache & cmd) = 0;
    virtual void send(const RDPGlyphCache & cmd) = 0;
};

#endif
