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


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

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
// |                              | [MS-RDPEGDI] section 2.2.2.2.1.1.2.16) and |
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

enum {
      NB_ORDER_SUPPORT = 0x20
};

enum {
      ORD_LEVEL_1_ORDERS = 0x01
};

enum {
      NEGOTIATEORDERSUPPORT = 0x02
    , ZEROBOUNDSDELTASSUPPORT = 0x08
    , COLORINDEXSUPPORT = 0x20
    , SOLIDPATTERNBRUSHONLY = 0x40
    , ORDERFLAGS_EXTRA_FLAGS = 0x80
};

enum {
      ORDERFLAGS_EX_CACHE_BITMAP_REV3_SUPPORT = 0x02
    , ORDERFLAGS_EX_ALTSEC_FRAME_MARKER_SUPPORT = 0x04
};


enum OrdersIndexes {
      TS_NEG_DSTBLT_INDEX = 0x00
    , TS_NEG_PATBLT_INDEX = 0x01
    , TS_NEG_SCRBLT_INDEX = 0x02
    , TS_NEG_MEMBLT_INDEX = 0x03
    , TS_NEG_MEM3BLT_INDEX = 0x04
    , UnusedIndex1 = 0x05
    , UnusedIndex2 = 0x06
    , TS_NEG_DRAWNINEGRID_INDEX = 0x07
    , TS_NEG_LINETO_INDEX = 0x08
    , TS_NEG_MULTI_DRAWNINEGRID_INDEX = 0x09
    , UnusedIndex3 = 0x0A
    , TS_NEG_SAVEBITMAP_INDEX = 0x0B
    , UnusedIndex4 = 0x0C
    , UnusedIndex5 = 0x0D
    , UnusedIndex6 = 0x0E
    , TS_NEG_MULTIDSTBLT_INDEX = 0x0F
    , TS_NEG_MULTIPATBLT_INDEX = 0x10
    , TS_NEG_MULTISCRBLT_INDEX = 0x11
    , TS_NEG_MULTIOPAQUERECT_INDEX = 0x12
    , TS_NEG_FAST_INDEX_INDEX = 0x13
    , TS_NEG_POLYGON_SC_INDEX = 0x14
    , TS_NEG_POLYGON_CB_INDEX = 0x15
    , TS_NEG_POLYLINE_INDEX = 0x16
    , UnusedIndex7 = 0x17
    , TS_NEG_FAST_GLYPH_INDEX = 0x18
    , TS_NEG_ELLIPSE_SC_INDEX = 0x19
    , TS_NEG_ELLIPSE_CB_INDEX = 0x1A
    , TS_NEG_INDEX_INDEX = 0x1B
    , UnusedIndex8 = 0x1C
    , UnusedIndex9 = 0x1D
    , UnusedIndex10 = 0x1E
    , UnusedIndex11 = 0x1F
};

enum {
    CAPLEN_ORDER = 88
};


struct OrderCaps : public Capability {

    uint8_t terminalDescriptor[16];
    uint32_t pad4octetsA;
    uint16_t desktopSaveXGranularity;
    uint16_t desktopSaveYGranularity;
    uint16_t pad2octetsA;
    uint16_t maximumOrderLevel;
    uint16_t numberFonts;
    uint16_t orderFlags;
    uint8_t orderSupport[NB_ORDER_SUPPORT];
    uint16_t textFlags;
    uint16_t orderSupportExFlags;
    uint32_t pad4octetsB;
    uint32_t desktopSaveSize;
    uint16_t pad2octetsC;
    uint16_t pad2octetsD;
    uint16_t textANSICodePage;
    uint16_t pad2octetsE ;

    OrderCaps()
    : Capability(CAPSTYPE_ORDER, CAPLEN_ORDER)
//        , terminalDescriptor = "";
        , pad4octetsA(0) //....................... MUST be ignored
        , desktopSaveXGranularity(1) //            ignored and assumed to be 1
        , desktopSaveYGranularity(20) //.......... ignored and assumed to be 20
        , pad2octetsA(0) //                        MUST be ignored
        , maximumOrderLevel(ORD_LEVEL_1_ORDERS) // is ignored and SHOULD be set to 1
        , numberFonts(0) //                        is ignored and SHOULD be set to 0
        , orderFlags(NEGOTIATEORDERSUPPORT) //.... from a "const list"
        , textFlags(0) //                          MUST be ignored
        , orderSupportExFlags(0) //............... from a "const list"
        , pad4octetsB(0) //                        MUST be ignored
        , desktopSaveSize(0) //................... ignored and assumed to be 230.400
        , pad2octetsC(0) //                        MUST be ignored
        , pad2octetsD(0) //....................... MUST be ignored
        , textANSICodePage(0) //                   CS : Code page of client
                              //                   SC : is ignored by client and SHOULD be set to 0
        , pad2octetsE (0) //...................... MUST be ignored
    {
        memset(this->terminalDescriptor, 0, 16); // 16 bytes array ALWAYS filled with 0
        memset(this->orderSupport, 0, NB_ORDER_SUPPORT); // 32 bits array filled with 0 BY DEFAULT
    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_copy_bytes(this->terminalDescriptor, 16);
        stream.out_uint32_be(this->pad4octetsA);
        stream.out_uint16_le(this->desktopSaveXGranularity);
        stream.out_uint16_le(this->desktopSaveYGranularity);
        stream.out_uint16_le(this->pad2octetsA);
        stream.out_uint16_le(this->maximumOrderLevel);
        stream.out_uint16_le(this->numberFonts);
        stream.out_uint16_le(this->orderFlags);
        stream.out_copy_bytes(this->orderSupport, NB_ORDER_SUPPORT);
        stream.out_uint16_le(this->textFlags);
        stream.out_uint16_le(this->orderSupportExFlags);
        stream.out_uint32_le(this->pad4octetsB);
        stream.out_uint32_le(this->desktopSaveSize);
        stream.out_uint16_le(this->pad2octetsC);
        stream.out_uint16_le(this->pad2octetsD);
        stream.out_uint16_le(this->textANSICodePage);
        stream.out_uint16_le(this->pad2octetsE);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;

        /* terminalDescriptor(16) + pad4octetsA(4) + desktopSaveXGranularity(2) + desktopSaveYGranularity(2) +
         * pad2octetsA(2) + maximumOrderLevel(2) + numberFonts(2) + orderFlags(2) + orderSupport(NB_ORDER_SUPPORT) +
         * textFlags(2) + orderSupportExFlags(2) + pad4octetsB(4) + desktopSaveSize(4) + pad2octetsC(2) +
         * pad2octetsD(2) + textANSICodePage(2) + pad2octetsE(2)
         */
        const unsigned expected = 32 + NB_ORDER_SUPPORT + 20;
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated OrderCaps, need=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        stream.in_copy_bytes(this->terminalDescriptor, 16);
        this->pad4octetsA = stream.in_uint32_le();
        this->desktopSaveXGranularity = stream.in_uint16_le();
        this->desktopSaveYGranularity = stream.in_uint16_le();
        this->pad2octetsA = stream.in_uint16_le();
        this->maximumOrderLevel = stream.in_uint16_le();
        this->numberFonts = stream.in_uint16_le();
        this->orderFlags = stream.in_uint16_le();
        stream.in_copy_bytes(this->orderSupport, NB_ORDER_SUPPORT);
        this->textFlags = stream.in_uint16_le();
        this->orderSupportExFlags = stream.in_uint16_le();
        this->pad4octetsB = stream.in_uint32_le();
        this->desktopSaveSize = stream.in_uint32_le();
        this->pad2octetsC = stream.in_uint16_le();
        this->pad2octetsD = stream.in_uint16_le();
        this->textANSICodePage = stream.in_uint16_le();
        this->pad2octetsE = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s Order caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "     Order caps::terminalDescriptor %u", this->terminalDescriptor[0]);
        LOG(LOG_INFO, "     Order caps::pad4octetsA %u", this->pad4octetsA);
        LOG(LOG_INFO, "     Order caps::desktopSaveXGranularity %u", this->desktopSaveXGranularity);
        LOG(LOG_INFO, "     Order caps::desktopSaveYGranularity %u", this->desktopSaveYGranularity);
        LOG(LOG_INFO, "     Order caps::pad2octetsA %u", this->pad2octetsA);
        LOG(LOG_INFO, "     Order caps::maximumOrderLevel %u", this->maximumOrderLevel);
        LOG(LOG_INFO, "     Order caps::numberFonts %u", this->numberFonts);
        LOG(LOG_INFO, "     Order caps::orderFlags 0x%X", this->orderFlags);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_DSTBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_DSTBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_PATBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_PATBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_SCRBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_SCRBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MEMBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MEMBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MEM3BLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MEM3BLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex1] %u"
                        , this->orderSupport[UnusedIndex1]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex2] %u"
                        , this->orderSupport[UnusedIndex2]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_DRAWNINEGRID_INDEX] %u"
                        , this->orderSupport[TS_NEG_DRAWNINEGRID_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_LINETO_INDEX] %u"
                        , this->orderSupport[TS_NEG_LINETO_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] %u"
                        , this->orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex3] %u"
                        , this->orderSupport[UnusedIndex3]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_SAVEBITMAP_INDEX] %u"
                        , this->orderSupport[TS_NEG_SAVEBITMAP_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex4] %u"
                        , this->orderSupport[UnusedIndex4]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex5] %u"
                        , this->orderSupport[UnusedIndex5]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex6] %u"
                        , this->orderSupport[UnusedIndex6]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MULTIDSTBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MULTIDSTBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MULTIPATBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MULTIPATBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MULTISCRBLT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MULTISCRBLT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] %u"
                        , this->orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_FAST_INDEX_INDEX] %u"
                        , this->orderSupport[TS_NEG_FAST_INDEX_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_POLYGON_SC_INDEX] %u"
                        , this->orderSupport[TS_NEG_POLYGON_SC_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_POLYGON_CB_INDEX] %u"
                        , this->orderSupport[TS_NEG_POLYGON_CB_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_POLYLINE_INDEX] %u"
                        , this->orderSupport[TS_NEG_POLYLINE_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex7] %u"
                        , this->orderSupport[UnusedIndex7]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_FAST_GLYPH_INDEX] %u"
                        , this->orderSupport[TS_NEG_FAST_GLYPH_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_ELLIPSE_SC_INDEX] %u"
                        , this->orderSupport[TS_NEG_ELLIPSE_SC_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_ELLIPSE_CB_INDEX] %u"
                        , this->orderSupport[TS_NEG_ELLIPSE_CB_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[TS_NEG_INDEX_INDEX] %u"
                        , this->orderSupport[TS_NEG_INDEX_INDEX]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex8] %u"
                        , this->orderSupport[UnusedIndex8]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex9] %u"
                        , this->orderSupport[UnusedIndex9]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex10] %u"
                        , this->orderSupport[UnusedIndex10]);
        LOG(LOG_INFO, "     Order caps::orderSupport[UnusedIndex11] %u"
                        , this->orderSupport[UnusedIndex11]);
        LOG(LOG_INFO, "     Order caps::textFlags %u", this->textFlags);
        LOG(LOG_INFO, "     Order caps::orderSupportExFlags 0x%X", this->orderSupportExFlags);
        LOG(LOG_INFO, "     Order caps::pad4octetsB %u", this->pad4octetsB);
        LOG(LOG_INFO, "     Order caps::desktopSaveSize %u", this->desktopSaveSize);
        LOG(LOG_INFO, "     Order caps::pad2octetsC %u", this->pad2octetsC);
        LOG(LOG_INFO, "     Order caps::pad2octetsD %u", this->pad2octetsD);
        LOG(LOG_INFO, "     Order caps::textANSICodePage %u", this->textANSICodePage);
        LOG(LOG_INFO, "     Order caps::pad2octetsE %u", this->pad2octetsE);
    }

    void dump(FILE * f) const
    {
       fprintf(f, "[Order Capability Set]\n");

       fprintf(f, "desktopSaveXGranularity=%u\n",         unsigned(this->desktopSaveXGranularity));
       fprintf(f, "desktopSaveYGranularity=%u\n",         unsigned(this->desktopSaveYGranularity));
       fprintf(f, "maximumOrderLevel=%u\n",               unsigned(this->maximumOrderLevel));
       fprintf(f, "numberFonts=%u\n",                     unsigned(this->numberFonts));
       fprintf(f, "orderFlags=0x%X\n",                    unsigned(this->orderFlags));
       fprintf(f, "TS_NEG_DSTBLT_INDEX=%u\n",             unsigned(this->orderSupport[TS_NEG_DSTBLT_INDEX]));
       fprintf(f, "TS_NEG_PATBLT_INDEX=%u\n",             unsigned(this->orderSupport[TS_NEG_PATBLT_INDEX]));
       fprintf(f, "TS_NEG_SCRBLT_INDEX=%u\n",             unsigned(this->orderSupport[TS_NEG_SCRBLT_INDEX]));
       fprintf(f, "TS_NEG_MEMBLT_INDEX=%u\n",             unsigned(this->orderSupport[TS_NEG_MEMBLT_INDEX]));
       fprintf(f, "TS_NEG_MEM3BLT_INDEX=%u\n",            unsigned(this->orderSupport[TS_NEG_MEM3BLT_INDEX]));
       fprintf(f, "TS_NEG_DRAWNINEGRID_INDEX=%u\n",       unsigned(this->orderSupport[TS_NEG_DRAWNINEGRID_INDEX]));
       fprintf(f, "TS_NEG_LINETO_INDEX=%u\n",             unsigned(this->orderSupport[TS_NEG_LINETO_INDEX]));
       fprintf(f, "TS_NEG_MULTI_DRAWNINEGRID_INDEX=%u\n", unsigned(this->orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX]));
       fprintf(f, "TS_NEG_SAVEBITMAP_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_SAVEBITMAP_INDEX]));
       fprintf(f, "TS_NEG_MULTIDSTBLT_INDEX=%u\n",        unsigned(this->orderSupport[TS_NEG_MULTIDSTBLT_INDEX]));
       fprintf(f, "TS_NEG_MULTIPATBLT_INDEX=%u\n",        unsigned(this->orderSupport[TS_NEG_MULTIPATBLT_INDEX]));
       fprintf(f, "TS_NEG_MULTISCRBLT_INDEX=%u\n",        unsigned(this->orderSupport[TS_NEG_MULTISCRBLT_INDEX]));
       fprintf(f, "TS_NEG_MULTIOPAQUERECT_INDEX=%u\n",    unsigned(this->orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX]));
       fprintf(f, "TS_NEG_FAST_INDEX_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_FAST_INDEX_INDEX]));
       fprintf(f, "TS_NEG_POLYGON_SC_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_POLYGON_SC_INDEX]));
       fprintf(f, "TS_NEG_POLYGON_CB_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_POLYGON_CB_INDEX]));
       fprintf(f, "TS_NEG_POLYLINE_INDEX=%u\n",           unsigned(this->orderSupport[TS_NEG_POLYLINE_INDEX]));
       fprintf(f, "TS_NEG_FAST_GLYPH_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_FAST_GLYPH_INDEX]));
       fprintf(f, "TS_NEG_ELLIPSE_SC_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_ELLIPSE_SC_INDEX]));
       fprintf(f, "TS_NEG_ELLIPSE_CB_INDEX=%u\n",         unsigned(this->orderSupport[TS_NEG_ELLIPSE_CB_INDEX]));
       fprintf(f, "TS_NEG_INDEX_INDEX=%u\n",              unsigned(this->orderSupport[TS_NEG_INDEX_INDEX]));
       fprintf(f, "orderSupportExFlags=0x%X\n",           unsigned(this->orderSupportExFlags));
       fprintf(f, "desktopSaveSize=%u\n",                 this->desktopSaveSize);
       fprintf(f, "textANSICodePage=%u\n\n",              unsigned(this->textANSICodePage));
    }
};
