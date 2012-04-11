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

   RDP Capabilities :

*/

#if !defined(__RDP_CAPABILITIES_ORDER_HPP__)
#define __RDP_CAPABILITIES_ORDER_HPP__

#include "constants.hpp"


// 2.2.7.1.3 Order Capability Set (TS_ORDER_CAPABILITYSET)
// =======================================================

// The TS_ORDER_CAPABILITYSET structure advertises support for primary drawing
// order-related capabilities and is based on the capability set specified in
// [T128] section 8.2.5 (for more information about primary drawing orders, see
// [MS-RDPEGDI] section 2.2.2.2.1.1). This capability is sent by both client and
// server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
// capability set. This field MUST be set to CAPSTYPE_ORDER (3).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of
// the capability data, including the size of the capabilitySetType and
// lengthCapability fields.

// terminalDescriptor (16 bytes): A 16-element array of 8-bit, unsigned integers.
// Terminal descriptor. This field is ignored and SHOULD be set to all zeros.

// pad4octetsA (4 bytes): A 32-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// desktopSaveXGranularity (2 bytes): A 16-bit, unsigned i46028714a40ade0e9a85f88c942dd9431fb21d44nteger. X granularity
// used in conjunction with the SaveBitmap Primary Drawing Order (see
// [MS-RDPEGDI] section 2.2.2.2.1.1.2.12). This value is ignored and assumed to
// be 1.

// desktopSaveYGranularity (2 bytes): A 16-bit, unsigned integer. Y granularity
// used in conjunction with the SaveBitmap Primary Drawing Order (see
// [MS-RDPEGDI] section 2.2.2.2.1.1.2.12). This value is ignored and assumed to
// be 20.

// pad2octetsA (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// maximumOrderLevel (2 bytes): A 16-bit, unsigned integer. Maximum order level.
// This value is ignored and SHOULD be set to ORD_LEVEL_1_ORDERS (1).

// numberFonts (2 bytes): A 16-bit, unsigned integer. Number of fonts. This
// value is ignored and SHOULD be set to 0.

// orderFlags (2 bytes): A 16-bit, unsigned integer. A 16-bit unsigned integer.
// Support for drawing order options.

// +--------------------------------+------------------------------------------+
// |              Flag              |             Meaning                      |
// +--------------------------------+------------------------------------------+
// | 0x0002 NEGOTIATEORDERSUPPORT   | Indicates support for specifying         |
// |                                | supported drawing orders in the          |
// |                                | orderSupport field. This flag MUST be    |
// |                                | set.                                     |
// +--------------------------------+------------------------------------------+
// | 0x0008 ZEROBOUNDSDELTASSUPPORT | Indicates support for the                |
// |                                | TS_ZERO_BOUNDS_DELTAS (0x20) flag (see   |
// |                                | [MS-RDPEGDI] section 2.2.2.2.1.1.2). The |
// |                                | client MUST set this flag.               |
// +--------------------------------+------------------------------------------+
// | 0x0020 COLORINDEXSUPPORT       | Indicates support for sending color      |
// |                                | indices (not RGB values) in orders.      |
// +--------------------------------+------------------------------------------+
// | 0x0040 SOLIDPATTERNBRUSHONLY   | Indicates that this party can receive    |
// |                                | only solid and pattern brushes.          |
// +--------------------------------+------------------------------------------+
// | 0x0080 ORDERFLAGS_EXTRA_FLAGS  | Indicates that the orderSupportExFlags   |
// |                                | field contains valid data.               |
// +--------------------------------+------------------------------------------+

// orderSupport (32 bytes): An array of 32 bytes indicating support for various
// primary drawing orders. The indices of this array are the negotiation indices
// for the primary orders specified in [MS-RDPEGDI] section 2.2.2.2.1.1.2.

// +---------------------------+-----------------------------------------------+
// |   Negotiation index       |         Primary drawing order or orders       |
// +---------------------------+-----------------------------------------------+
// | 0x00 TS_NEG_DSTBLT_INDEX  | DstBlt Primary Drawing Order (see [MS-RDPEGDI]|
// |                           | section 2.2.2.2.1.1.2.1).                     |
// +---------------------------+-----------------------------------------------+
// | 0x01 TS_NEG_PATBLT_INDEX  | PatBlt Primary Drawing Order (see [MS-RDPEGDI]|
// |                           | section 2.2.2.2.1.1.2.3) and OpaqueRect       |
// |                           | Primary Drawing Order (see [MS-RDPEGDI]       |
// |                           | section 2.2.2.2.1.1.2.5).                     |
// +---------------------------+-----------------------------------------------+
// | 0x02 TS_NEG_SCRBLT_INDEX  | ScrBlt Primary Drawing Order (see [MS-RDPEGDI]|
// |                           | section 2.2.2.2.1.1.2.7).                     |
// +---------------------------+-----------------------------------------------+
// | 0x03 TS_NEG_MEMBLT_INDEX  | MemBlt Primary Drawing Order (see [MS-RDPEGDI]|
// |                           | section 2.2.2.2.1.1.2.9).                     |
// +---------------------------+-----------------------------------------------+
// | 0x04 TS_NEG_MEM3BLT_INDEX | Mem3Blt Primary Drawing Order (see            |
// |                           | [MS-RDPEGDI] section 2.2.2.2.1.1.2.10).       |
// +---------------------------+-----------------------------------------------+
// | 0x05 UnusedIndex1         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x06 UnusedIndex2         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x07                      | DrawNineGrid Primary Drawing Order (see       |
// | TS_NEG_DRAWNINEGRID_INDEX | [MS-RDPEGDI] section 2.2.2.2.1.1.2.21).       |
// +---------------------------+-----+-----------------------------------------+
// | 0x08 TS_NEG_LINETO_INDEX        | LineTo Primary Drawing Order (see       |
// |                                 | [MS-RDPEGDI] section 2.2.2.2.1.1.2.11). |
// +---------------------------------+-----------------------------------------+
// | 0x09                            | MultiDrawNineGrid Primary Drawing Order |
// | TS_NEG_MULTI_DRAWNINEGRID_INDEX | (see [MS-RDPEGDI] section               |
// |                                 | 2.2.2.2.1.1.2.22).                      |
// +---------------------------+-----+-----------------------------------------+
// | 0x0A UnusedIndex3         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x0B                      | SaveBitmap Primary Drawing Order (see         |
// | TS_NEG_SAVEBITMAP_INDEX   | [MS-RDPEGDI] section 2.2.2.2.1.1.2.12).       |
// +---------------------------+-----------------------------------------------+
// | 0x0C UnusedIndex4         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x0D UnusedIndex5         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x0E UnusedIndex6         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+---+-------------------------------------------+
// | 0x0F TS_NEG_MULTIDSTBLT_INDEX | MultiDstBlt Primary Drawing Order (see    |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.1.2.2).    |
// +-------------------------------+-------------------------------------------+
// | 0x10 TS_NEG_MULTIPATBLT_INDEX |MultiPatBlt Primary Drawing Order (see     |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.1.2.4).    |
// +-------------------------------+-------------------------------------------+
// | 0x11 TS_NEG_MULTISCRBLT_INDEX |MultiScrBlt Primary Drawing Order (see     |
// |                               | [MS-RDPEGDI] section 2.2.2.2.1.1.2.8).    |
// +-------------------------------+---+---------------------------------------+
// | 0x12 TS_NEG_MULTIOPAQUERECT_INDEX | MultiOpaqueRect Primary Drawing Order |
// |                                   | (see [MS-RDPEGDI] section             |
// |                                   |  2.2.2.2.1.1.2.6).                    |
// +------------------------------+----+---------------------------------------+
// | 0x13 TS_NEG_FAST_INDEX_INDEX |FastIndex Primary Drawing Order (see        |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.14).    |
// +------------------------------+--------------------------------------------+
// | 0x14 TS_NEG_POLYGON_SC_INDEX | PolygonSC Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 246028714a40ade0e9a85f88c942dd9431fb21d44.2.2.2.1.1.2.16) and |
// |                              | PolygonCB Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.17).    |
// +------------------------------+--------------------------------------------+
// | 0x15 TS_NEG_POLYGON_CB_INDEX | PolygonCB Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.17) and |
// |                              | PolygonSC Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.16).    |
// +----------------------------+-+--------------------------------------------+
// | 0x16 TS_NEG_POLYLINE_INDEX | Polyline Primary Drawing Order (see          |
// |                            | [MS-RDPEGDI] section 2.2.2.2.1.1.2.18).      |
// +----------------------------+----------------------------------------------+
// | 0x17 UnusedIndex7          | The contents of the byte at this index MUST  |
// |                            | be ignored.                                  |
// +----------------------------+-+--------------------------------------------+
// | 0x18 TS_NEG_FAST_GLYPH_INDEX | FastGlyph Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.15).    |
// +------------------------------+--------------------------------------------+
// | 0x19 TS_NEG_ELLIPSE_SC_INDEX | EllipseSC Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.19) and |
// |                              | EllipseCB Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.20).    |
// +------------------------------+--------------------------------------------+
// | 0x1A TS_NEG_ELLIPSE_CB_INDEX | EllipseCB Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.20) and |
// |                              | EllipseSC Primary Drawing Order (see       |
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.19).    |
// +---------------------------+--+--------------------------------------------+
// | 0x1B TS_NEG_INDEX_INDEX   | GlyphIndex Primary Drawing Order (see         |
// |                           | [MS-RDPEGDI] section 2.2.2.2.1.1.2.13).       |
// +---------------------------+-----------------------------------------------+
// | 0x1C UnusedIndex8         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x1D UnusedIndex9         | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x1E UnusedIndex10        | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+
// | 0x1F UnusedIndex11        | The contents of the byte at this index MUST be|
// |                           | ignored.                                      |
// +---------------------------+-----------------------------------------------+

// If an order is supported, the byte at the given index MUST contain the value
// 0x01. Any order not supported by the client causes the server to spend more
// time and bandwidth using workarounds, such as other primary orders or simply
// sending screen bitmap data in a Bitmap Update (see sections 2.2.9.1.1.3.1.2
// and 2.2.9.1.2.1.2). If no primary drawing orders are supported, this array
// MUST be initialized to all zeros.

// textFlags (2 bytes): A 16-bit, unsigned integer. Values in this field MUST be
// ignored.

// orderSupportExFlags (2 bytes): A 16-bit, unsigned integer. Extended order
// support flags.

// +-------------------------------------------+-------------------------------+
// |                 Flag                      |             Meaning           |
// +-------------------------------------------+-------------------------------+
// | 0x0002                                    | The Cache Bitmap (Revision 3) |
// | ORDERFLAGS_EX_CACHE_BITMAP_REV3_SUPPORT   | Secondary Drawing Order       |
// |                                           | ([MS-RDPEGDI] section         |
// |                                           | 2.2.2.2.1.2.8) is supported.  |
// +-------------------------------------------+-------------------------------+
// | 0x0004                                    | The Frame Marker Alternate    |
// | ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT | Secondary Drawing Order       |
// |                                           | ([MS-RDPEGDI] section         |
// |                                           | 2.2.2.2.1.3.7) is supported.  |
// +-------------------------------------------+-------------------------------+

// pad4octetsB (4 bytes): A 32-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// desktopSaveSize (4 bytes): A 32-bit, unsigned integer. The maximum usable size
// of bitmap space for bitmap packing in the SaveBitmap Primary Drawing Order
// (see [MS-RDPEGDI] section 2.2.2.2.1.1.2.12). This field is ignored by the
// client and assumed to be 230400 bytes (480 * 480).

// pad2octetsC (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// pad2octetsD (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// textANSICodePage (2 bytes): A 16-bit, unsigned integer. ANSI code page
// descriptor being used by the client (for a list of code pages, see [MSDN-CP]).
// This field is ignored by the client and SHOULD be set to 0 by the server.

// pad2octetsE (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

static inline void cs_out_order_caps(Stream & stream)
{
    LOG(LOG_INFO, "Sending order caps to server");

    stream.out_uint16_le(RDP_CAPSET_ORDER);
    stream.out_uint16_le(RDP_CAPLEN_ORDER);
    stream.out_clear_bytes(20); /* Terminal desc, pad */
    stream.out_uint16_le(1); /* Cache X granularity */
    stream.out_uint16_le(20); /* Cache Y granularity */
    stream.out_uint16_le(0); /* Pad */
    stream.out_uint16_le(1); /* Max order level */
    stream.out_uint16_le(0x147); /* Number of fonts */
    stream.out_uint16_le(0x2a); /* Capability flags */

    char order_caps[32];

    memset(order_caps, 0, 32);
    TODO(" use symbolic constants for order numerotation")
    order_caps[RDP::DESTBLT] = 1; /* dest blt */
    order_caps[RDP::PATBLT] = 1; /* pat blt */
    order_caps[RDP::SCREENBLT] = 1; /* screen blt */
    order_caps[3] = 1; /* memblt */
    order_caps[4] = 0; /* todo triblt */
    order_caps[8] = 1; /* line */
    order_caps[9] = 1; /* line */
    order_caps[10] = 1; /* rect */
    order_caps[11] = 0; /* todo desksave */
    order_caps[RDP::MEMBLT] = 1; /* memblt another above */
    order_caps[RDP::TRIBLT] = 0; /* triblt another above */
    order_caps[20] = 0; /* todo polygon */
    order_caps[21] = 0; /* todo polygon2 */
    order_caps[RDP::POLYLINE] = 0; /* todo polyline */
    order_caps[25] = 0; /* todo ellipse */
    order_caps[26] = 0; /* todo ellipse2 */
    order_caps[RDP::GLYPHINDEX] = 1; /* text2 */
    stream.out_copy_bytes(order_caps, 32); /* Orders supported */

    stream.out_uint16_le(0x6a1); /* Text capability flags */
    stream.out_clear_bytes(6); /* Pad */
    stream.out_uint32_le(0 * 0x38400); /* Desktop cache size, for desktop_save */
    stream.out_uint32_le(0); /* Unknown */
    stream.out_uint32_le(0x4e4); /* Unknown */
}

    /*****************************************************************************/
static inline void sc_in_order_caps(Stream & stream, uint16_t length)
{
    TODO("implement it, at least to trace server configuration settings");
}


static inline void sc_out_order_caps(Stream & stream)
{
        stream.out_uint16_le(RDP_CAPSET_ORDER); /* 3 */
        stream.out_uint16_le(RDP_CAPLEN_ORDER); /* 88(0x58) */
        stream.out_clear_bytes(16);
        stream.out_uint32_be(0x40420f00);
        stream.out_uint16_le(1); // desktopSaveXGranularity
        stream.out_uint16_le(20); // desktopSaveYGranularity
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); // maximumOrderLevel
        stream.out_uint16_le(0x2f); // Number of fonts
        stream.out_uint16_le(0x22); // Capability flags
        /* caps */
        stream.out_uint8(1); /* dest blt */
        stream.out_uint8(1); /* pat blt */
        stream.out_uint8(1); /* screen blt */
        stream.out_uint8(1); /* mem blt */
        stream.out_uint8(0); /* tri blt */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* nine grid */
        stream.out_uint8(1); /* line to */
        stream.out_uint8(0); /* multi nine grid */
        stream.out_uint8(1); /* rect */
        stream.out_uint8(0); /* desk save */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* multi dest blt */
        stream.out_uint8(0); /* multi pat blt */
        stream.out_uint8(0); /* multi screen blt */
        stream.out_uint8(0); /* multi rect */
        stream.out_uint8(0); /* fast index */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polyline */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* fast glyph */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ? */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */

        stream.out_uint16_le(0x6a1); // textFlags
        stream.out_clear_bytes(2); // orderSupportExFlags
        stream.out_uint32_le(0x0f4240); // pad4octetsB
        stream.out_uint32_le(0x0f4240); // desktopSaveSize
        stream.out_uint32_le(1); // pad 2octetsC, pad2octetsD
        stream.out_uint32_le(0); // textANSICodePage, pad2octetsE
}

    /*****************************************************************************/
static inline void cs_in_order_caps(Stream & stream, int len, uint32_t & desktop_cache)
{
    LOG(LOG_INFO, "capset_order");
    stream.in_skip_bytes(20); /* Terminal desc, pad */
    stream.in_skip_bytes(2); /* Cache X granularity */
    stream.in_skip_bytes(2); /* Cache Y granularity */
    stream.in_skip_bytes(2); /* Pad */
    stream.in_skip_bytes(2); /* Max order level */
    stream.in_skip_bytes(2); /* Number of fonts */
    stream.in_skip_bytes(2); /* Capability flags */
    char order_caps[32];
    memcpy(order_caps, stream.in_uint8p(32), 32); /* Orders supported */
    LOG(LOG_INFO, "dest blt-0 %d\n", order_caps[0]);
    LOG(LOG_INFO, "pat blt-1 %d\n", order_caps[1]);
    LOG(LOG_INFO, "screen blt-2 %d\n", order_caps[2]);
    LOG(LOG_INFO, "memblt-3-13 %d %d\n", order_caps[3], order_caps[13]);
    LOG(LOG_INFO, "triblt-4-14 %d %d\n", order_caps[4], order_caps[14]);
    LOG(LOG_INFO, "line-8 %d\n", order_caps[8]);
    LOG(LOG_INFO, "line-9 %d\n", order_caps[9]);
    LOG(LOG_INFO, "rect-10 %d\n", order_caps[10]);
    LOG(LOG_INFO, "desksave-11 %d\n", order_caps[11]);
    LOG(LOG_INFO, "polygon-20 %d\n", order_caps[20]);
    LOG(LOG_INFO, "polygon2-21 %d\n", order_caps[21]);
    LOG(LOG_INFO, "polyline-22 %d\n", order_caps[22]);
    LOG(LOG_INFO, "ellipse-25 %d\n", order_caps[25]);
    LOG(LOG_INFO, "ellipse2-26 %d\n", order_caps[26]);
    LOG(LOG_INFO, "text2-27 %d\n", order_caps[27]);
    LOG(LOG_INFO, "order_caps dump\n");
    stream.in_skip_bytes(2); /* Text capability flags */
    stream.in_skip_bytes(6); /* Pad */
    /* desktop cache size, usually 0x38400 */
    desktop_cache = stream.in_uint32_le();
    LOG(LOG_INFO, "desktop cache size %d\n", desktop_cache);
    stream.in_skip_bytes(4); /* Unknown */
    stream.in_skip_bytes(4); /* Unknown */
}

#endif
