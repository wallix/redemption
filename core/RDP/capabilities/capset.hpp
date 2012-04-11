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

   RDP Capabilities :

*/

#if !defined(__RDP_CAPABILITIES_CAPSET_HPP__)
#define __RDP_CAPABILITIES_CAPSET_HPP__


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

#include "constants.hpp"

#include "activate.hpp"
#include "bitmap.hpp"
#include "bmpcache2.hpp"
#include "bmpcache.hpp"
#include "colcache.hpp"
#include "control.hpp"
#include "font.hpp"
#include "general.hpp"
#include "glyphcache.hpp"
#include "input.hpp"
#include "order.hpp"
#include "pointer.hpp"
#include "share.hpp"
#include "sound.hpp"

#endif
