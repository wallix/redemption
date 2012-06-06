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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Common code for all capabilities

*/

#if !defined(__RDP_CAPABILITIES_COMMON_HPP__)
#define __RDP_CAPABILITIES_COMMON_HPP__

#include <stdint.h>
#include "log.hpp"
#include "stream.hpp"
#include "RDP/orders/RDPOrdersCommon.hpp"

// 2.2.1.13.1.1.1 Capability Set (TS_CAPS_SET)
// ===========================================
// The TS_CAPS_SET structure is used to describe the type and size of a
// capability set exchanged between clients and servers. All capability sets
// conform to this basic structure (see section 2.2.7).

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type identifier
// of the capability set.

// +-----------------------------------------+---------------------------------+
// | 0x0001 CAPSTYPE_GENERAL                 | General Capability Set          |
// |                                         | (section 2.2.7.1.1)             |
// +-----------------------------------------+---------------------------------+
// | 0x0002 CAPSTYPE_BITMAP                  | Bitmap Capability Set           |
// |                                         | (section 2.2.7.1.2)             |
// +-----------------------------------------+---------------------------------+
// | 0x0003 CAPSTYPE_ORDER                   | Order Capability Set            |
// |                                         | (section 2.2.7.1.3)             |
// +-----------------------------------------+---------------------------------+
// | 0x0004 CAPSTYPE_BITMAPCACHE             | Revision 1 Bitmap Cache         |
// |                                         | Capability Set                  |
// |                                         | (section 2.2.7.1.4.1)           |
// +-----------------------------------------+---------------------------------+
// | 0x0005 CAPSTYPE_CONTROL                 | Control Capability Set          |
// |                                         | (section 2.2.7.2.2)             |
// +-----------------------------------------+---------------------------------+
// | 0x0007 CAPSTYPE_ACTIVATION              | Window Activation Capability Set|
// |                                         | (section 2.2.7.2.3)             |
// +-----------------------------------------+---------------------------------+
// | 0x0008 CAPSTYPE_POINTER                 | Pointer Capability Set          |
// |                                         | (section 2.2.7.1.5)             |
// +-----------------------------------------+---------------------------------+
// | 0x0009 CAPSTYPE_SHARE                   | Share Capability Set            |
// |                                         | (section 2.2.7.2.4)             |
// +-----------------------------------------+---------------------------------+
// | 0x000A CAPSTYPE_COLORCACHE              | Color Table Cache Capability Set|
// |                                         | (see [MS-RDPEGDI]               |
// |                                         | section 2.2.1.1)                |
// +-----------------------------------------+---------------------------------+
// | 0x000C CAPSTYPE_SOUND                   | Sound Capability Set            |
// |                                         | (section 2.2.7.1.11)            |
// +-----------------------------------------+---------------------------------+
// | 0x000D CAPSTYPE_INPUT                   | Input Capability Set            |
// |                                         | (section 2.2.7.1.6)             |
// +-----------------------------------------+---------------------------------+
// | 0x000E CAPSTYPE_FONT                    | Font Capability Set             |
// |                                         | (section 2.2.7.2.5)             |
// +-----------------------------------------+---------------------------------+
// | 0x000F CAPSTYPE_BRUSH                   | Brush Capability Set            |
// |                                         | (section 2.2.7.1.7)             |
// +-----------------------------------------+---------------------------------+
// | 0x0010 CAPSTYPE_GLYPHCACHE              | Glyph Cache Capability Set      |
// |                                         | (section 2.2.7.1.8)             |
// +-----------------------------------------+---------------------------------+
// | 0x0011 CAPSTYPE_OFFSCREENCACHE          | Offscreen Bitmap Cache          |
// |                                         | Capability Set                  |
// |                                         | (section 2.2.7.1.9)             |
// +-----------------------------------------+---------------------------------+
// | 0x0012 CAPSTYPE_BITMAPCACHE_HOSTSUPPORT | Bitmap Cache Host Support       |
// |                                         | Capability Set                  |
// |                                         | (section 2.2.7.2.1)             |
// +-----------------------------------------+---------------------------------+
// | 0x0013 CAPSTYPE_BITMAPCACHE_REV2        | Revision 2 Bitmap Cache         |
// |                                         | Capability Set                  |
// |                                         | (section 2.2.7.1.4.2)           |
// +-----------------------------------------+---------------------------------+
// | 0x0014 CAPSTYPE_VIRTUALCHANNEL          | Virtual Channel Capability Set  |
// |                                         | (section 2.2.7.1.10)            |
// +-----------------------------------------+---------------------------------+
// | 0x0015 CAPSTYPE_DRAWNINEGRIDCACHE       | DrawNineGrid Cache Capability   |
// |                                         | Set ([MS-RDPEGDI]               |
// |                                         | section 2.2.1.2)                |
// +-----------------------------------------+---------------------------------+
// | 0x0016 CAPSTYPE_DRAWGDIPLUS             | Draw GDI+ Cache Capability Set  |
// |                                         | ([MS-RDPEGDI] section 2.2.1.3)  |
// +-----------------------------------------+---------------------------------+
// | 0x0017 CAPSTYPE_RAIL                    | Remote Programs Capability Set  |
// |                                         | ([MS-RDPERP] section 2.2.1.1.1) |
// +-----------------------------------------+---------------------------------+
// | 0x0018 CAPSTYPE_WINDOW                  | Window List Capability Set      |
// |                                         | ([MS-RDPERP] section 2.2.1.1.2) |
// +-----------------------------------------+---------------------------------+
// | 0x0019 CAPSETTYPE_COMPDESK              | Desktop Composition Extension   |
// |                                         | Capability Set                  |
// |                                         | (section 2.2.7.2.8)             |
// +-----------------------------------------+---------------------------------+
// | 0x001A CAPSETTYPE_MULTIFRAGMENTUPDATE   | Multifragment Update Capability |
// |                                         | Set (section 2.2.7.2.6)         |
// +-----------------------------------------+---------------------------------+
// | 0x001B CAPSETTYPE_LARGE_POINTER         | Large Pointer Capability Set    |
// |                                         | (section 2.2.7.2.7)             |
// +-----------------------------------------+---------------------------------+
// | 0x001C CAPSETTYPE_SURFACE_COMMANDS      | Surface Commands Capability Set |
// |                                         | (section 2.2.7.2.9)             |
// +-----------------------------------------+---------------------------------+
// | 0x001D CAPSETTYPE_BITMAP_CODECS         | Bitmap Codecs Capability Set    |
// |                                         | (section 2.2.7.2.10)            |
// +-----------------------------------------+---------------------------------+

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//  of the capability data, including the size of the capabilitySetType and
// lengthCapability fields.

// capabilityData (variable): Capability set data which conforms to the
// structure of the type given by the capabilitySetType field.

/* Maps to generalCapabilitySet in T.128 page 138 */

enum {
// 1 CAPSTYPE_GENERAL General Capability Set ([MS-RDPBCGR] section 2.2.7.1.1)
CAPSTYPE_GENERAL                    = 1,
RDP_CAPLEN_GENERAL                  = 0x18,

// 2 CAPSTYPE_BITMAP Bitmap Capability Set ([MS-RDPBCGR] section 2.2.7.1.2)
CAPSTYPE_BITMAP                     = 2,
RDP_CAPLEN_BITMAP                   = 0x1C,

// 3 CAPSTYPE_ORDER Order Capability Set ([MS-RDPBCGR] section 2.2.7.1.3)
CAPSTYPE_ORDER                      = 3,
RDP_CAPLEN_ORDER                    = 0x58,

// 4 CAPSTYPE_BITMAPCACHE Revision 1 Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.1)
CAPSTYPE_BITMAPCACHE                = 4,
RDP_CAPLEN_BITMAPCACHE              = 0x28,

// 5 CAPSTYPE_CONTROL Control Capability Set ([MS-RDPBCGR] section 2.2.7.2.2)
CAPSTYPE_CONTROL                    = 5,
RDP_CAPLEN_CONTROL                  = 0x0C,

// 7 CAPSTYPE_ACTIVATION Window Activation Capability Set ([MS-RDPBCGR] section 2.2.7.2.3)
CAPSTYPE_ACTIVATION                 = 7,
RDP_CAPLEN_ACTIVATION               = 0x0C,

// 8 CAPSTYPE_POINTER Pointer Capability Set ([MS-RDPBCGR] section 2.2.7.1.5)
CAPSTYPE_POINTER                    = 8,
RDP_CAPLEN_POINTER                  = 0x0a,
RDP_CAPLEN_POINTER_MONO             = 0x08,

// 9 CAPSTYPE_SHARE Share Capability Set ([MS-RDPBCGR] section 2.2.7.2.4)
CAPSTYPE_SHARE                      = 9,
RDP_CAPLEN_SHARE                    = 0x08,

// 10 CAPSTYPE_COLORCACHE Color Table Cache Capability Set (see [MS-RDPEGDI] section 2.2.1.1)
CAPSTYPE_COLORCACHE                 = 10,
RDP_CAPLEN_COLORCACHE               = 0x08,

// 12 CAPSTYPE_SOUND Sound Capability Set ([MS-RDPBCGR] section 2.2.7.1.11)
CAPSTYPE_SOUND                      = 12,
RDP_CAPLEN_SOUND                    = 0x08,

// 13 CAPSTYPE_INPUT Input Capability Set ([MS-RDPBCGR] section 2.2.7.1.6)
CAPSTYPE_INPUT                      = 13,
RDP_CAPLEN_INPUT                    = 0x58,

// 14 CAPSTYPE_FONT Font Capability Set ([MS-RDPBCGR] section 2.2.7.2.5)
CAPSTYPE_FONT                       = 14,
RDP_CAPLEN_FONT                     = 0x08,

// 15 CAPSTYPE_BRUSH Brush Capability Set ([MS-RDPBCGR] section 2.2.7.1.7)
CAPSTYPE_BRUSH                      = 15,
RDP_CAPLEN_BRUSH                    = 0x08,

// 16 CAPSTYPE_GLYPHCACHE Glyph Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.8)
CAPSTYPE_GLYPHCACHE                 = 16,
RDP_CAPLEN_GLYPHCACHE               = 0x34,

// 17 CAPSTYPE_OFFSCREENCACHE Offscreen Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.9)
CAPSTYPE_OFFSCREENCACHE             = 17,

// 18 CAPSTYPE_BITMAPCACHE_HOSTSUPPORT Bitmap Cache Host Support Capability Set ([MS-RDPBCGR] section 2.2.7.2.1)
CAPSTYPE_BITMAPCACHE_HOSTSUPPORT    = 18,
RDP_CAPLEN_BITMAPCACHE_HOSTSUPPORT  = 0x08,

// 19 CAPSTYPE_BITMAPCACHE_REV2 Revision 2 Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.2)
CAPSTYPE_BITMAPCACHE_REV2           = 19,
RDP_CAPLEN_BITMAPCACHE_REV2         = 0x28,

// 20 CAPSTYPE_VIRTUALCHANNEL Virtual Channel Capability Set ([MS-RDPBCGR] section 2.2.7.1.10)
CAPSTYPE_VIRTUALCHANNEL             = 20,
RDP_CAPLEN_VIRTUALCHANNEL           = 0x08,

// 21 CAPSTYPE_DRAWNINEGRIDCACHE DrawNineGrid Cache Capability Set ([MS-RDPEGDI] section 2.2.1.2)
CAPSTYPE_DRAWNINEGRIDCACHE          = 21,
RDP_CAPLEN_DRAWNINEGRIDCACHE        = 0x0C,

// 22 CAPSTYPE_DRAWGDIPLUS Draw GDI+ Cache Capability Set ([MS-RDPEGDI] section 2.2.1.3)
CAPSTYPE_DRAWGDIPLUS                = 22,
RDP_CAPLEN_DRAWGDIPLUS              = 0x28,

// 23 CAPSTYPE_RAIL Remote Programs Capability Set ([MS-RDPERP] section 2.2.1.1.1)
CAPSTYPE_RAIL                       = 23,

// 24 CAPSTYPE_WINDOW Window List Capability Set ([MS-RDPERP] section 2.2.1.1.2)
CAPSTYPE_WINDOW                     = 24,

// 25 CAPSETTYPE_COMPDESK Desktop Composition Extension Capability Set ([MS-RDPBCGR] section 2.2.7.2.8)
CAPSETTYPE_TYPE_COMPDESK            = 25,
RDP_CAPLEN_COMPDESK                 = 0x08,

// 26 CAPSETTYPE_MULTIFRAGMENTUPDATE Multifragment Update Capability Set ([MS-RDPBCGR] section 2.2.7.2.6)
CAPSETTYPE_TYPE_MULTIFRAGMENTUPDATE = 26,

// 27 CAPSETTYPE_LARGE_POINTER Large Pointer Capability Set ([MS-RDPBCGR] section 2.2.7.2.7)
CAPSETTYPE_TYPE_LARGE_POINTER       = 27,

// 28 CAPSETTYPE_SURFACE_COMMANDS Surface Commands Capability Set ([MS-RDPBCGR] section 2.2.7.2.9)
CAPSETTYPE_TYPE_SURFACE_COMMANDS    = 28,

// 29 CAPSETTYPE_BITMAP_CODECS Bitmap Codecs Capability Set ([MS-RDPBCGR] section 2.2.7.2.10)
CAPSETTYPE_TYPE_BITMAP_CODECS       = 29,

// 30 CAPSSETTYPE_FRAME_ACKNOWLEDGE Frame Acknowledge Capability Set (([MS-RDPRFX] section 2.2.1.3))
CAPSETTYPE_FRAME_ACKNOWLEDGE        = 30,
};


enum {
OS_MAJOR_TYPE_UNIX             = 4,
OS_MINOR_TYPE_XSERVER          = 7,

ORDER_CAP_NEGOTIATE            = 2,
ORDER_CAP_NOSUPPORT            = 4,

BMPCACHE2_FLAG_PERSIST         = ((long)1<<31),
};

struct Capability {
    uint16_t capabilityType;
    uint16_t len;

    Capability(uint16_t capabilityType, uint16_t len)
        : capabilityType(capabilityType)
        , len(len)
    {
    }

    void emit(Stream & stream)
    {
        LOG(LOG_ERR, "Capability::emit [%u, %u] Implemented by subclass",
            this->capabilityType, this->len);
    }

    void recv(Stream & stream, uint16_t len)
    {
        this->len = len;
        LOG(LOG_ERR, "Capability::recv [%u, %u] Implemented by subclass",
            this->capabilityType, this->len);
    }

    void log(const char * msg)
    {
        LOG(LOG_ERR, "Capability::log [%u, %u, %s] Implemented by subclass",
            this->capabilityType, this->len, msg);
    }
};

#endif
