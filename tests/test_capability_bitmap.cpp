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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityBitmap
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"
#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityBitmapEmit)
{
    BitmapCaps bitmap_caps;
    bitmap_caps.preferredBitsPerPixel = 24;
    bitmap_caps.desktopWidth = 800;
    bitmap_caps.desktopHeight = 600;
    bitmap_caps.bitmapCompressionFlag = 1;

    BOOST_CHECK_EQUAL(bitmap_caps.capabilityType, (uint16_t)CAPSTYPE_BITMAP);
    BOOST_CHECK_EQUAL(bitmap_caps.len, (uint16_t)RDP_CAPLEN_BITMAP);
    BOOST_CHECK_EQUAL(bitmap_caps.preferredBitsPerPixel, (uint16_t)24);
    BOOST_CHECK_EQUAL(bitmap_caps.receive1BitPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.receive4BitsPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.receive8BitsPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.desktopWidth, (uint16_t)800);
    BOOST_CHECK_EQUAL(bitmap_caps.desktopHeight, (uint16_t)600);
    BOOST_CHECK_EQUAL(bitmap_caps.pad2octets, (uint16_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps.desktopResizeFlag, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.bitmapCompressionFlag, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.highColorFlags, (uint8_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps.drawingFlags, (uint8_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps.multipleRectangleSupport, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps.pad2octetsB, (uint16_t)0);

    Stream stream(1024);
    bitmap_caps.emit(stream);
    stream.end = stream.p;
    stream.p = stream.data;

    BitmapCaps bitmap_caps2;

    BOOST_CHECK_EQUAL(bitmap_caps2.capabilityType, (uint16_t)CAPSTYPE_BITMAP);
    BOOST_CHECK_EQUAL(bitmap_caps2.len, (uint16_t)RDP_CAPLEN_BITMAP);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_BITMAP, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)RDP_CAPLEN_BITMAP, stream.in_uint16_le());
    bitmap_caps2.recv(stream);

    BOOST_CHECK_EQUAL(bitmap_caps2.preferredBitsPerPixel, (uint16_t)24);
    BOOST_CHECK_EQUAL(bitmap_caps2.receive1BitPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.receive4BitsPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.receive8BitsPerPixel, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.desktopWidth, (uint16_t)800);
    BOOST_CHECK_EQUAL(bitmap_caps2.desktopHeight, (uint16_t)600);
    BOOST_CHECK_EQUAL(bitmap_caps2.pad2octets, (uint16_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps2.desktopResizeFlag, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.bitmapCompressionFlag, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.highColorFlags, (uint8_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps2.drawingFlags, (uint8_t)0);
    BOOST_CHECK_EQUAL(bitmap_caps2.multipleRectangleSupport, (uint16_t)1);
    BOOST_CHECK_EQUAL(bitmap_caps2.pad2octetsB, (uint16_t)0);

}


//BOOST_AUTO_TEST_CASE(TestCapabilityGlyphSupportEmit)
//{
////    uint8_t glyphCache[40];
////    uint32_t fragCache;
////    uint16_t glyphSupportLevel;
////    uint16_t pad2octets;


//    GlyphSupportCaps glyphcache_caps;
////    glyphcache_caps.glyphCache = 0;
////    glyphcache_caps.fragCache = 0;
////    glyphcache_caps.glyphSupportLevel = 0;
////    glyphcache_caps.pad2octets = 0;

//    printf("\n\n__________GLYPHSUPPORT_NEW_________\n");

//    Stream stream(1024);
//    glyphcache_caps.emit(stream);
////    stream.end = stream.p;
////    stream.p = stream.data;

//    hexdump((const char *) stream.data, stream.p - stream.data);
//}


//BOOST_AUTO_TEST_CASE(TestCapabilityOrderEmit)
//{
////    {
////        printf("\n\n__________ORDER_OLD__________\n");

////        Stream stream(1024);
////        stream.out_uint16_le(CAPSTYPE_ORDER);
////        stream.out_uint16_le(RDP_CAPLEN_ORDER);
////        stream.out_clear_bytes(20); /* Terminal desc, pad */
////        stream.out_uint16_le(1); /* Cache X granularity */
////        stream.out_uint16_le(20); /* Cache Y granularity */
////        stream.out_uint16_le(0); /* Pad */
////        stream.out_uint16_le(1); /* Max order level */
////        stream.out_uint16_le(0x147); /* Number of fonts */
////        stream.out_uint16_le(0x2a); /* Capability flags */

////        char order_caps[32];

////        memset(order_caps, 0, 32);
////        TODO(" use symbolic constants for order numerotation")
////        order_caps[RDP::DESTBLT] = 1; /* dest blt */
////        order_caps[RDP::PATBLT] = 1; /* pat blt */
////        order_caps[RDP::SCREENBLT] = 1; /* screen blt */
////        order_caps[3] = 1; /* memblt */
////        order_caps[4] = 0; /* todo triblt */
////        order_caps[8] = 1; /* line */
////        order_caps[9] = 1; /* line */
////        order_caps[10] = 1; /* rect */
////        order_caps[11] = 0; /* todo desksave */
////        order_caps[RDP::MEMBLT] = 1; /* memblt another above */
////        order_caps[RDP::TRIBLT] = 0; /* triblt another above */
////        order_caps[20] = 0; /* todo polygon */
////        order_caps[21] = 0; /* todo polygon2 */
////        order_caps[RDP::POLYLINE] = 0; /* todo polyline */
////        order_caps[25] = 0; /* todo ellipse */
////        order_caps[26] = 0; /* todo ellipse2 */
////        order_caps[RDP::GLYPHINDEX] = 1; /* text2 */
////        stream.out_copy_bytes(order_caps, 32); /* Orders supported */

////        stream.out_uint16_le(0x6a1); /* Text capability flags */
////        stream.out_clear_bytes(6); /* Pad */
////        stream.out_uint32_le(0 * 0x38400); /* Desktop cache size, for desktop_save */
////        stream.out_uint32_le(0); /* Unknown */
////        stream.out_uint32_le(0x4e4); /* Unknown */

////        hexdump((const char *) stream.data, stream.p - stream.data);
////    }

//    {
//        printf("\n\n__________ORDER_NEW__________\n");

//    // =====================================================================================================================
//        Stream stream(1024);
//        OrderCaps order_caps;
//        order_caps.numberFonts = 0x147;
//        order_caps.orderFlags = 0x2a;

//        order_caps.orderSupport[TS_NEG_DSTBLT_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_PATBLT_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_SCRBLT_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_MEMBLT_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_MEM3BLT_INDEX] = 1;
////            order_caps.orderSupport[UnusedIndex1] = 1;
////            order_caps.orderSupport[UnusedIndex2] = 1;
////            order_caps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_LINETO_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX] = 1;
//        order_caps.orderSupport[UnusedIndex3] = 1;
////            order_caps.orderSupport[TS_NEG_SAVEBITMAP_INDEX] = 0;
////            order_caps.orderSupport[UnusedIndex4] = 1;
//        order_caps.orderSupport[UnusedIndex5] = 1;
////            order_caps.orderSupport[UnusedIndex6] = 1;
////            order_caps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_MULTIPATBLT_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_MULTISCRBLT_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_FAST_INDEX_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_POLYGON_SC_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_POLYGON_CB_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_POLYLINE_INDEX] = 1;
////            order_caps.orderSupport[UnusedIndex7] = 1;
////            order_caps.orderSupport[TS_NEG_FAST_GLYPH_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX] = 1;
////            order_caps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX] = 1;
//        order_caps.orderSupport[TS_NEG_INDEX_INDEX] = 1;
////            order_caps.orderSupport[UnusedIndex8] = 1;
////            order_caps.orderSupport[UnusedIndex9] = 1;
////            order_caps.orderSupport[UnusedIndex10] = 1;
////            order_caps.orderSupport[UnusedIndex11] = 1;

//        order_caps.textFlags = 0x6a1;
//        order_caps.textANSICodePage = 0x4;
//        order_caps.pad2octetsE = 0xe4;

//        order_caps.emit(stream);

//        hexdump((const char *) stream.data, stream.p - stream.data);
//    }
//}
