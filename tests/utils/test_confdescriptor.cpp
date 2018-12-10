/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define RED_TEST_MODULE TestConfDescriptor
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/confdescriptor.hpp"
#include "core/RDP/capabilities/general.hpp"
#include "core/RDP/capabilities/cap_bitmap.hpp"
#include "core/RDP/capabilities/order.hpp"


RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile)
{
    GeneralCaps       generalcaps;
    RED_CHECK(general_caps_load(generalcaps, FIXTURES_PATH "/capsset.ini"));

    RED_CHECK_EQUAL(generalcaps.os_major,              1);
    RED_CHECK_EQUAL(generalcaps.os_minor,              3);
    RED_CHECK_EQUAL(generalcaps.protocolVersion,       512);
    RED_CHECK_EQUAL(generalcaps.compressionType,       0);
    RED_CHECK_EQUAL(generalcaps.extraflags,            1025);
    RED_CHECK_EQUAL(generalcaps.updateCapability,      0);
    RED_CHECK_EQUAL(generalcaps.remoteUnshare,         0);
    RED_CHECK_EQUAL(generalcaps.compressionLevel,      0);
    RED_CHECK_EQUAL(generalcaps.refreshRectSupport,    1);
    RED_CHECK_EQUAL(generalcaps.suppressOutputSupport, 1);
}

RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile1)
{
    BitmapCaps       bitmapcaps;
    RED_CHECK(bitmap_caps_load(bitmapcaps, FIXTURES_PATH "/capsset.ini"));

    RED_CHECK_EQUAL(bitmapcaps.preferredBitsPerPixel,    8);
    RED_CHECK_EQUAL(bitmapcaps.receive1BitPerPixel,      1);
    RED_CHECK_EQUAL(bitmapcaps.receive4BitsPerPixel,     1);
    RED_CHECK_EQUAL(bitmapcaps.receive8BitsPerPixel,     1);
    RED_CHECK_EQUAL(bitmapcaps.desktopWidth,             1024);
    RED_CHECK_EQUAL(bitmapcaps.desktopHeight,            768);
    RED_CHECK_EQUAL(bitmapcaps.desktopResizeFlag,        1);
    RED_CHECK_EQUAL(bitmapcaps.bitmapCompressionFlag,    1);
    RED_CHECK_EQUAL(bitmapcaps.highColorFlags,           0);
    RED_CHECK_EQUAL(bitmapcaps.drawingFlags,             0);
    RED_CHECK_EQUAL(bitmapcaps.multipleRectangleSupport, 1);
}

RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile2)
{
    OrderCaps       ordercaps;
    RED_CHECK(order_caps_load(ordercaps, FIXTURES_PATH "/capsset.ini"));

    RED_CHECK_EQUAL(ordercaps.desktopSaveXGranularity,                       1);
    RED_CHECK_EQUAL(ordercaps.desktopSaveYGranularity,                       20);
    RED_CHECK_EQUAL(ordercaps.maximumOrderLevel,                             1);
    RED_CHECK_EQUAL(ordercaps.numberFonts,                                   0);
    RED_CHECK_EQUAL(ordercaps.orderFlags,                                    34);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_DSTBLT_INDEX],             1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_PATBLT_INDEX],             1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_SCRBLT_INDEX],             1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MEMBLT_INDEX],             1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MEM3BLT_INDEX],            1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX],       0);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_LINETO_INDEX],             1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX], 0);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_SAVEBITMAP_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX],        1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIPATBLT_INDEX],        1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTISCRBLT_INDEX],        1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX],    1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_FAST_INDEX_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYGON_SC_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYGON_CB_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYLINE_INDEX],           1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_FAST_GLYPH_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX],         1);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_INDEX_INDEX],              1);
    RED_CHECK_EQUAL(ordercaps.orderSupportExFlags,                           0);
    RED_CHECK_EQUAL(ordercaps.desktopSaveSize,                               1000000u);
    RED_CHECK_EQUAL(ordercaps.textANSICodePage,                              0);
}



RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile3)
{
    GeneralCaps       generalcaps;
    RED_CHECK(general_caps_load(generalcaps, FIXTURES_PATH "/capsset1.ini"));

    RED_CHECK_EQUAL(generalcaps.os_major,              2);
    RED_CHECK_EQUAL(generalcaps.os_minor,              7);
    RED_CHECK_EQUAL(generalcaps.protocolVersion,       256);
    RED_CHECK_EQUAL(generalcaps.compressionType,       3);
    RED_CHECK_EQUAL(generalcaps.extraflags,            768);
    RED_CHECK_EQUAL(generalcaps.updateCapability,      5);
    RED_CHECK_EQUAL(generalcaps.remoteUnshare,         4);
    RED_CHECK_EQUAL(generalcaps.compressionLevel,      3);
    RED_CHECK_EQUAL(generalcaps.refreshRectSupport,    2);
    RED_CHECK_EQUAL(generalcaps.suppressOutputSupport, 6);
}

RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile4)
{
    BitmapCaps       bitmapcaps;
    RED_CHECK(bitmap_caps_load(bitmapcaps, FIXTURES_PATH "/capsset1.ini"));

    RED_CHECK_EQUAL(bitmapcaps.preferredBitsPerPixel,    9);
    RED_CHECK_EQUAL(bitmapcaps.receive1BitPerPixel,      8);
    RED_CHECK_EQUAL(bitmapcaps.receive4BitsPerPixel,     7);
    RED_CHECK_EQUAL(bitmapcaps.receive8BitsPerPixel,     6);
    RED_CHECK_EQUAL(bitmapcaps.desktopWidth,             1440);
    RED_CHECK_EQUAL(bitmapcaps.desktopHeight,            900);
    RED_CHECK_EQUAL(bitmapcaps.desktopResizeFlag,        5);
    RED_CHECK_EQUAL(bitmapcaps.bitmapCompressionFlag,    4);
    RED_CHECK_EQUAL(bitmapcaps.highColorFlags,           3);
    RED_CHECK_EQUAL(bitmapcaps.drawingFlags,             2);
    RED_CHECK_EQUAL(bitmapcaps.multipleRectangleSupport, 0);
}

RED_AUTO_TEST_CASE(TestConfigDescriptorFromFile5)
{
    OrderCaps       ordercaps;
    RED_CHECK(order_caps_load(ordercaps, FIXTURES_PATH "/capsset1.ini"));

    RED_CHECK_EQUAL(ordercaps.desktopSaveXGranularity,                       9);
    RED_CHECK_EQUAL(ordercaps.desktopSaveYGranularity,                       8);
    RED_CHECK_EQUAL(ordercaps.maximumOrderLevel,                             7);
    RED_CHECK_EQUAL(ordercaps.numberFonts,                                   6);
    RED_CHECK_EQUAL(ordercaps.orderFlags,                                    5);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_DSTBLT_INDEX],             9);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_PATBLT_INDEX],             8);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_SCRBLT_INDEX],             7);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MEMBLT_INDEX],             6);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MEM3BLT_INDEX],            5);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_DRAWNINEGRID_INDEX],       4);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_LINETO_INDEX],             3);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTI_DRAWNINEGRID_INDEX], 2);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_SAVEBITMAP_INDEX],         9);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIDSTBLT_INDEX],        8);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIPATBLT_INDEX],        7);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTISCRBLT_INDEX],        6);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_MULTIOPAQUERECT_INDEX],    5);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_FAST_INDEX_INDEX],         4);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYGON_SC_INDEX],         3);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYGON_CB_INDEX],         2);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_POLYLINE_INDEX],           9);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_FAST_GLYPH_INDEX],         8);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_ELLIPSE_SC_INDEX],         7);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_ELLIPSE_CB_INDEX],         6);
    RED_CHECK_EQUAL(ordercaps.orderSupport[TS_NEG_INDEX_INDEX],              5);
    RED_CHECK_EQUAL(ordercaps.orderSupportExFlags,                           9);
    RED_CHECK_EQUAL(ordercaps.desktopSaveSize,                               20000u);
    RED_CHECK_EQUAL(ordercaps.textANSICodePage,                              8);
}
