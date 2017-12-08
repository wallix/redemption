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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Common code for all capabilities
*/


#pragma once

#include <cstdint>
#include <iosfwd>
#include<string>

#include "utils/sugar/std_stream_proto.hpp"


class InStream;
class OutStream;

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

enum CAPSTYPE : uint16_t {
    // General Capability Set ([MS-RDPBCGR] section 2.2.7.1.1)
    CAPSTYPE_GENERAL                    = 1,
    // Bitmap Capability Set ([MS-RDPBCGR] section 2.2.7.1.2)
    CAPSTYPE_BITMAP                     = 2,
    // Order Capability Set ([MS-RDPBCGR] section 2.2.7.1.3)
    CAPSTYPE_ORDER                      = 3,
    // Revision 1 Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.1)
    CAPSTYPE_BITMAPCACHE                = 4,
    // Control Capability Set ([MS-RDPBCGR] section 2.2.7.2.2)
    CAPSTYPE_CONTROL                    = 5,
    // Window Activation Capability Set ([MS-RDPBCGR] section 2.2.7.2.3)
    CAPSTYPE_ACTIVATION                 = 7,
    // Pointer Capability Set ([MS-RDPBCGR] section 2.2.7.1.5)
    CAPSTYPE_POINTER                    = 8,
    // Share Capability Set ([MS-RDPBCGR] section 2.2.7.2.4)
    CAPSTYPE_SHARE                      = 9,
    // Color Table Cache Capability Set (see [MS-RDPEGDI] section 2.2.1.1)
    CAPSTYPE_COLORCACHE                 = 10,
    // Sound Capability Set ([MS-RDPBCGR] section 2.2.7.1.11)
    CAPSTYPE_SOUND                      = 12,
    // Input Capability Set ([MS-RDPBCGR] section 2.2.7.1.6)
    CAPSTYPE_INPUT                      = 13,
    // Font Capability Set ([MS-RDPBCGR] section 2.2.7.2.5)
    CAPSTYPE_FONT                       = 14,
    // Brush Capability Set ([MS-RDPBCGR] section 2.2.7.1.7)
    CAPSTYPE_BRUSH                      = 15,
    // Glyph Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.8)
    CAPSTYPE_GLYPHCACHE                 = 16,
    // Offscreen Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.9)
    CAPSTYPE_OFFSCREENCACHE             = 17,
    // Bitmap Cache Host Support Capability Set ([MS-RDPBCGR] section 2.2.7.2.1)
    CAPSTYPE_BITMAPCACHE_HOSTSUPPORT    = 18,
    // Revision 2 Bitmap Cache Capability Set ([MS-RDPBCGR] section 2.2.7.1.4.2)
    CAPSTYPE_BITMAPCACHE_REV2           = 19,
    // Virtual Channel Capability Set ([MS-RDPBCGR] section 2.2.7.1.10)
    CAPSTYPE_VIRTUALCHANNEL             = 20,
    // DrawNineGrid Cache Capability Set ([MS-RDPEGDI] section 2.2.1.2)
    CAPSTYPE_DRAWNINEGRIDCACHE          = 21,
    // Draw GDI+ Cache Capability Set ([MS-RDPEGDI] section 2.2.1.3)
    CAPSTYPE_DRAWGDIPLUS                = 22,
    // Remote Programs Capability Set ([MS-RDPERP] section 2.2.1.1.1)
    CAPSTYPE_RAIL                       = 23,
    // Window List Capability Set ([MS-RDPERP] section 2.2.1.1.2)
    CAPSTYPE_WINDOW                     = 24,
    // Desktop Composition Extension Capability Set ([MS-RDPBCGR] section 2.2.7.2.8)
    CAPSETTYPE_COMPDESK                 = 25,
    // Multifragment Update Capability Set ([MS-RDPBCGR] section 2.2.7.2.6)
    CAPSETTYPE_MULTIFRAGMENTUPDATE      = 26,
    // Large Pointer Capability Set ([MS-RDPBCGR] section 2.2.7.2.7)
    CAPSETTYPE_LARGE_POINTER            = 27,
    // Surface Commands Capability Set ([MS-RDPBCGR] section 2.2.7.2.9)
    CAPSETTYPE_SURFACE_COMMANDS         = 28,
    // Bitmap Codecs Capability Set ([MS-RDPBCGR] section 2.2.7.2.10)
    CAPSETTYPE_BITMAP_CODECS            = 29,
    // Frame Acknowledge Capability Set (([MS-RDPRFX] section 2.2.1.3))
    CAPSETTYPE_FRAME_ACKNOWLEDGE        = 30
};

REDEMPTION_OSTREAM(out, CAPSTYPE t)
{ return out << static_cast<uint16_t>(t); }


inline static const char * get_capabilitySetType_name(uint16_t capabilitySetType)
{
    switch (capabilitySetType) {
        case CAPSTYPE_GENERAL:                 return "General Capability Set";
        case CAPSTYPE_BITMAP:                  return "Bitmap Capability Set";
        case CAPSTYPE_ORDER:                   return "Order Capability Set";
        case CAPSTYPE_BITMAPCACHE:             return "Revision 1 Bitmap Cache Capability Set";
        case CAPSTYPE_CONTROL:                 return "Control Capability Set";
        case CAPSTYPE_ACTIVATION:              return "Window Activation Capability Set";
        case CAPSTYPE_POINTER:                 return "Pointer Capability Set";
        case CAPSTYPE_SHARE:                   return "Share Capability Set";
        case CAPSTYPE_COLORCACHE:              return "Color Table Cache Capability Set";
        case CAPSTYPE_SOUND:                   return "Sound Capability Set";
        case CAPSTYPE_INPUT:                   return "Input Capability Set";
        case CAPSTYPE_FONT:                    return "Font Capability Set";
        case CAPSTYPE_BRUSH:                   return "Brush Capability Set";
        case CAPSTYPE_GLYPHCACHE:              return "Glyph Cache Capability Set";
        case CAPSTYPE_OFFSCREENCACHE:          return "Offscreen Bitmap Cache Capability Set";
        case CAPSTYPE_BITMAPCACHE_HOSTSUPPORT: return "Bitmap Cache Host Support Capability Set";
        case CAPSTYPE_BITMAPCACHE_REV2:        return "Revision 2 Bitmap Cache Capability Set";
        case CAPSTYPE_VIRTUALCHANNEL:          return "Virtual Channel Capability Set";
        case CAPSTYPE_DRAWNINEGRIDCACHE:       return "DrawNineGrid Cache Capability Set";
        case CAPSTYPE_DRAWGDIPLUS:             return "Draw GDI+ Cache Capability Set";
        case CAPSTYPE_RAIL:                    return "Remote Programs Capability Set";
        case CAPSTYPE_WINDOW:                  return "Window List Capability Set";
        case CAPSETTYPE_COMPDESK:              return "Desktop Composition Extension Capability Set";
        case CAPSETTYPE_MULTIFRAGMENTUPDATE:   return "Multifragment Update Capability Set";
        case CAPSETTYPE_LARGE_POINTER:         return "Large Pointer Capability Set";
        case CAPSETTYPE_SURFACE_COMMANDS:      return "Surface Commands Capability Set";
        case CAPSETTYPE_BITMAP_CODECS:         return "Bitmap Codecs Capability Set";
        case CAPSETTYPE_FRAME_ACKNOWLEDGE:     return "Frame Acknowledge Capability Set";
    }

    return "<unknown>";
}

struct Capability
{
    CAPSTYPE capabilityType;
    uint16_t len;

    Capability(CAPSTYPE capabilityType, uint16_t len)
    : capabilityType(capabilityType)
    , len(len) {
    }
};
