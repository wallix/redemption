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

#include "RDP/capabilities.hpp"


BOOST_AUTO_TEST_CASE(TestCapabilityBitmapEmit)
{
    BitmapCaps bitmap_caps;
    bitmap_caps.preferredBitsPerPixel = 24;
    bitmap_caps.desktopWidth = 800;
    bitmap_caps.desktopHeight = 600;
    bitmap_caps.bitmapCompressionFlag = 1;

    BOOST_CHECK_EQUAL(bitmap_caps.capabilityType, (uint16_t)RDP_CAPSET_BITMAP);
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

    BOOST_CHECK_EQUAL(bitmap_caps2.capabilityType, (uint16_t)RDP_CAPSET_BITMAP);
    BOOST_CHECK_EQUAL(bitmap_caps2.len, (uint16_t)RDP_CAPLEN_BITMAP);

    BOOST_CHECK_EQUAL((uint16_t)RDP_CAPSET_BITMAP, stream.in_uint16_le());
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
