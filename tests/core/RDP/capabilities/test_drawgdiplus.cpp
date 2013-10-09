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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean

   Unit test to RDP DrawGdiPlus object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCapabilityDrawGdiPlus
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityDrawGdiPlusEmit)
{
    DrawGdiPlusCaps drawgdiplus_caps;
    drawgdiplus_caps.drawGDIPlusSupportLevel = 5;
    drawgdiplus_caps.GdipVersion = 42;
    drawgdiplus_caps.drawGdiplusCacheLevel = 2;

    drawgdiplus_caps.gdiPCacheEntries->GdipGraphicsCacheEntries = 10;
    drawgdiplus_caps.gdiPCacheEntries->GdipBrushCacheEntries = 5;
    drawgdiplus_caps.gdiPCacheEntries->GdipPenCacheEntries = 5;
    drawgdiplus_caps.gdiPCacheEntries->GdipImageCacheEntries = 10;
    drawgdiplus_caps.gdiPCacheEntries->GdipImageAttributesCacheEntries = 2;

    drawgdiplus_caps.gdiPCacheChunkSize->GdipGraphicsCacheChunkSize = 512;
    drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectBrushCacheChunkSize = 2048;
    drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectPenCacheChunkSize = 1024;
    drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectImageAttributesCacheChunkSize = 64;

    drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheChunkSize = 4096;
    drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheTotalSize = 256;
    drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheMaxSize = 128;

    BOOST_CHECK_EQUAL(drawgdiplus_caps.capabilityType, static_cast<uint16_t>(CAPSTYPE_DRAWGDIPLUS));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.len, static_cast<uint16_t>(CAPLEN_DRAWGDIPLUS));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.drawGDIPlusSupportLevel, static_cast<uint32_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.GdipVersion, static_cast<uint32_t>(42));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.drawGdiplusCacheLevel, static_cast<uint32_t>(2));

    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheEntries->GdipGraphicsCacheEntries, static_cast<uint16_t>(10));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheEntries->GdipBrushCacheEntries, static_cast<uint16_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheEntries->GdipPenCacheEntries, static_cast<uint16_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheEntries->GdipImageCacheEntries, static_cast<uint16_t>(10));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheEntries->GdipImageAttributesCacheEntries, static_cast<uint16_t>(2));

    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipGraphicsCacheChunkSize, static_cast<uint16_t>(512));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectBrushCacheChunkSize, static_cast<uint16_t>(2048));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectPenCacheChunkSize, static_cast<uint16_t>(1024));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectImageAttributesCacheChunkSize, static_cast<uint16_t>(64));

    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheChunkSize, static_cast<uint16_t>(4096));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheTotalSize, static_cast<uint16_t>(256));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPImageCacheProperties->GdipObjectImageCacheMaxSize, static_cast<uint16_t>(128));

    BStream stream(1024);
    drawgdiplus_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.get_data();

    DrawGdiPlusCaps drawgdiplus_caps2;

    BOOST_CHECK_EQUAL(drawgdiplus_caps2.capabilityType, static_cast<uint16_t>(CAPSTYPE_DRAWGDIPLUS));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.len, static_cast<uint16_t>(CAPLEN_DRAWGDIPLUS));

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_DRAWGDIPLUS, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_DRAWGDIPLUS, stream.in_uint16_le());
    drawgdiplus_caps2.recv(stream, CAPLEN_DRAWGDIPLUS);

    BOOST_CHECK_EQUAL(drawgdiplus_caps2.drawGDIPlusSupportLevel, static_cast<uint32_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.GdipVersion, static_cast<uint32_t>(42));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.drawGdiplusCacheLevel, static_cast<uint32_t>(2));

    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheEntries->GdipGraphicsCacheEntries, static_cast<uint16_t>(10));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheEntries->GdipBrushCacheEntries, static_cast<uint16_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheEntries->GdipPenCacheEntries, static_cast<uint16_t>(5));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheEntries->GdipImageCacheEntries, static_cast<uint16_t>(10));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheEntries->GdipImageAttributesCacheEntries, static_cast<uint16_t>(2));

    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPCacheChunkSize->GdipGraphicsCacheChunkSize, static_cast<uint16_t>(512));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectBrushCacheChunkSize, static_cast<uint16_t>(2048));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectPenCacheChunkSize, static_cast<uint16_t>(1024));
    BOOST_CHECK_EQUAL(drawgdiplus_caps.gdiPCacheChunkSize->GdipObjectImageAttributesCacheChunkSize, static_cast<uint16_t>(64));

    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPImageCacheProperties->GdipObjectImageCacheChunkSize, static_cast<uint16_t>(4096));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPImageCacheProperties->GdipObjectImageCacheTotalSize, static_cast<uint16_t>(256));
    BOOST_CHECK_EQUAL(drawgdiplus_caps2.gdiPImageCacheProperties->GdipObjectImageCacheMaxSize, static_cast<uint16_t>(128));
}
