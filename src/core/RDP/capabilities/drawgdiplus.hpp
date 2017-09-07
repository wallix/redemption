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

   RDP Capabilities : Draw GDI+ Cache Capability Set ([MS-RDPEGDI] section 2.2.1.3)

*/


#pragma once

#include "core/RDP/capabilities/common.hpp"
#include "utils/stream.hpp"

// 2.2.1.3 Draw GDI+ Capability Set (TS_DRAW_GDIPLUS_CAPABILITYSET)
//=================================================================

// The TS_DRAW_GDIPLUS_CAPABILITYSET structure is used to advertise the level of GDI+ 1.1
// rendering and caching support and the GDI+ cache configuration. This capability is sent by both
// client and server. However, only the client initializes the GdipVersion, GdipCacheEntries,

// GdipCacheChunkSize, and GdipImageCacheProperties fields. The server uses this capability set
//    only to advertise its level of GDI+ 1.1 rendering and caching support.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//    field MUST be set to CAPSTYPE_DRAWGDIPLUS (0x0016).
//
// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length, in bytes, of the capability
//    data, including the size of the capabilitySetType and lengthCapability fields.
//
// drawGDIPlusSupportLevel (4 bytes): A 32-bit, unsigned integer. The level of support for
//    GDI+ 1.1 remoting. This field MUST be set to one of the following values.
//    +---------------------------+----------------------------+
//    | TS_DRAW_GDIPLUS_DEFAULT   | GDI+ 1.1 is not supported. |
//    | 0x00000000                |                            |
//    +---------------------------+----------------------------+
//    | TS_DRAW_GDIPLUS_SUPPORTED | GDI+ 1.1 is supported.     |
//    | 0x00000001                |                            |
//    +---------------------------+----------------------------+

// GdipVersion (4 bytes): A 32-bit, unsigned integer. The build number of the underlying GDI+
//    1.1 subsystem. Only the client-to-server instance of the GDI+ Capability Set MUST contain a
//    valid value for this field.

// drawGdiplusCacheLevel (4 bytes): A 32-bit, unsigned integer. The level of support for the
//    caching of GDI+ 1.1 rendering primitives. This field MUST be set to one of the following
//    values.
//    +-------------------------------------+-------------------------------------------------+
//    | TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT | Caching of GDI+ 1.1 rendering primitives is not |
//    | 0x00000000                          |supported.                                       |
//    +-------------------------------------+-------------------------------------------------+
//    | TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE     | Caching of GDI+ 1.1 rendering primitives is     |
//    | 0x00000001                          |        supported.                               |
//    +-------------------------------------+-------------------------------------------------+

// GdipCacheEntries (10 bytes): A GDI+ Cache Entries (section 2.2.1.3.1) structure that
//    specifies the total number of entries within the GDI+ Graphics, Pen, Brush, Image, and Image
//    Attributes caches. Only the client-to-server instance of the GDI+ Capability Set MUST contain
//    a valid value for this field.

// GdipCacheChunkSize (8 bytes): A GDI+ Cache Chunk Size structure that specifies the size of
//    individual entries in the GDI+ Graphics, Brush, Pen, and Image Attributes caches. Only the
//    client-to-server instance of the GDI+ Capability Set MUST contain a valid value for this field.

// GdipImageCacheProperties (6 bytes): A GDI+ Image Cache Properties structure that
//    contains sizing information for the GDI+ Image cache. Only the client-to-server instance of
//    the GDI+ Capability Set MUST contain a valid value for this field.


// 2.2.1.3.1 GDI+ Cache Entries (TS_GDIPLUS_CACHE_ENTRIES)
//==========================================================
// The TS_GDIPLUS_CACHE_ENTRIES structure specifies the total number of cache entries for the
//  GDI+ Graphics, Brush, Pen, Image, and Image Attributes caches.

// GdipGraphicsCacheEntries (2 bytes): A 16-bit, unsigned integer. The total number of entries
//    allowed in the GDI+ Graphics cache. The maximum allowed value is 10 entries.

// GdipBrushCacheEntries (2 bytes): A 16-bit, unsigned integer. The total number of entries
//    allowed in the GDI+ Brush cache. The maximum allowed value is 5 entries.

// GdipPenCacheEntries (2 bytes): A 16-bit, unsigned integer. The total number of entries
//    allowed in the GDI+ Pen cache. The maximum allowed value is 5 entries.

// GdipImageCacheEntries (2 bytes): A 16-bit, unsigned integer. The total number of entries
//    allowed in the GDI+ Image cache. The maximum allowed value is 10 entries.

// GdipImageAttributesCacheEntries (2 bytes): A 16-bit, unsigned integer. The total number
//    of entries allowed in the GDI+ Image Attributes cache. The maximum allowed value is 2
//    entries.

// 2.2.1.3.2  GDI+ Cache Chunk Size (TS_GDIPLUS_CACHE_CHUNK_SIZE)
//===============================================================
// The TS_GDIPLUS_CACHE_CHUNK_SIZE structure specifies the maximum size of individual entries in
//    the GDI+ Graphics, Brush, Pen, and Image Attributes caches.

// GdipGraphicsCacheChunkSize (2 bytes): A 16-bit, unsigned integer. The maximum size in
//    bytes of a GDI+ Graphics cache entry. The maximum allowed value is 512 bytes.

// GdipObjectBrushCacheChunkSize (2 bytes): A 16-bit, unsigned integer. The maximum size
//    in bytes of a GDI+ Brush cache entry. The maximum allowed value is 2,048 bytes.

// GdipObjectPenCacheChunkSize (2 bytes): A 16-bit, unsigned integer. The maximum size in
//    bytes of a GDI+ Pen cache entry. The maximum allowed value is 1,024 bytes.

// GdipObjectImageAttributesCacheChunkSize (2 bytes): A 16-bit, unsigned integer. The
//    maximum size in bytes of a GDI+ Image Attributes cache entry. The maximum allowed value
//    is 64 bytes.

// 2.2.1.3.3 GDI+ Image Cache Properties (TS_GDIPLUS_IMAGE_CACHE_PROPERTIES)
//==========================================================================
// The TS_GDIPLUS_IMAGE_CACHE_PROPERTIES structure contains sizing information for the GDI+
// Image cache.

// GdipObjectImageCacheChunkSize (2 bytes): A 16-bit, unsigned integer. The maximum size
//    in bytes of a chunk in the GDI+ Image cache. The maximum allowed value is 4,096 bytes.

// GdipObjectImageCacheTotalSize (2 bytes): A 16-bit, unsigned integer. The total number of
//    chunks in the GDI+ Image cache. The maximum allowed value is 256 chunks.

// GdipObjectImageCacheMaxSize (2 bytes): A 16-bit, unsigned integer. The total number of
//    chunks that can be used by an entry in the GDI+ Image cache. The maximum allowed value is
//    128 chunks.

//==============================================================================

enum {
      TS_DRAW_GDIPLUS_DEFAULT
     ,TS_DRAW_GDIPLUS_SUPPORTED
     };

enum {
       TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT
     , TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE
     };

//==============================================================================

struct GdiPCacheEntries {

    uint16_t GdipGraphicsCacheEntries;
    uint16_t GdipBrushCacheEntries;
    uint16_t GdipPenCacheEntries;
    uint16_t GdipImageCacheEntries;
    uint16_t GdipImageAttributesCacheEntries;

    GdiPCacheEntries()
    : GdipGraphicsCacheEntries(0)        // 0 by default (max 10 entries)
    , GdipBrushCacheEntries(0)           // 0 by default (max  5 entries)
    , GdipPenCacheEntries(0)             // 0 by default (max  5 entries)
    , GdipImageCacheEntries(0)           // 0 by default (max 10 entries)
    , GdipImageAttributesCacheEntries(0) // 0 by default (max  2 entries)
    {
    }
};

struct GdiPCacheChunkSize {

    uint16_t GdipGraphicsCacheChunkSize;
    uint16_t GdipObjectBrushCacheChunkSize;
    uint16_t GdipObjectPenCacheChunkSize;
    uint16_t GdipObjectImageAttributesCacheChunkSize;

    GdiPCacheChunkSize()
    : GdipGraphicsCacheChunkSize(0)              // 0 by default (max  512 bytes)
    , GdipObjectBrushCacheChunkSize(0)           // 0 by default (max 2048 bytes)
    , GdipObjectPenCacheChunkSize(0)             // 0 by default (max 1024 bytes)
    , GdipObjectImageAttributesCacheChunkSize(0) // 0 by default (max   64 bytes)
    {
    }
};

struct GdiPImageCacheProperties {

    uint16_t GdipObjectImageCacheChunkSize;
    uint16_t GdipObjectImageCacheTotalSize;
    uint16_t GdipObjectImageCacheMaxSize;

    GdiPImageCacheProperties()
    : GdipObjectImageCacheChunkSize(0)  // 0 by default (max 4096 bytes)
    , GdipObjectImageCacheTotalSize(0)  // 0 by default (max  256 chunks)
    , GdipObjectImageCacheMaxSize(0)    // 0 by default (max  128 chunks)
    {
    }
};

enum {
    CAPLEN_DRAWGDIPLUS = 40
};

struct DrawGdiPlusCaps : public Capability {

    uint32_t drawGDIPlusSupportLevel;
    uint32_t GdipVersion;
    uint32_t drawGdiplusCacheLevel;

    GdiPCacheEntries gdiPCacheEntries;
    GdiPCacheChunkSize gdiPCacheChunkSize;
    GdiPImageCacheProperties gdiPImageCacheProperties;


    DrawGdiPlusCaps()
    : Capability(CAPSTYPE_DRAWGDIPLUS, CAPLEN_DRAWGDIPLUS)
    , drawGDIPlusSupportLevel(TS_DRAW_GDIPLUS_DEFAULT) // from a specific list of values (see enum)
    , GdipVersion(0) // CS : MUST be a valid version number
                     // SC : whatever (not used)
    , drawGdiplusCacheLevel(TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT) // from a specific list of values (see enum)
    {
    }

    void emit(OutStream & stream) const
    {
//        LOG(LOG_INFO, "DrawGdiPlus caps emit not implemented");
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint32_le(this->drawGDIPlusSupportLevel);
        stream.out_uint32_le(this->GdipVersion);
        stream.out_uint32_le(this->drawGdiplusCacheLevel);

        stream.out_uint16_le(this->gdiPCacheEntries.GdipGraphicsCacheEntries);
        stream.out_uint16_le(this->gdiPCacheEntries.GdipBrushCacheEntries);
        stream.out_uint16_le(this->gdiPCacheEntries.GdipPenCacheEntries);
        stream.out_uint16_le(this->gdiPCacheEntries.GdipImageCacheEntries);
        stream.out_uint16_le(this->gdiPCacheEntries.GdipImageAttributesCacheEntries);

        stream.out_uint16_le(this->gdiPCacheChunkSize.GdipGraphicsCacheChunkSize);
        stream.out_uint16_le(this->gdiPCacheChunkSize.GdipObjectBrushCacheChunkSize);
        stream.out_uint16_le(this->gdiPCacheChunkSize.GdipObjectPenCacheChunkSize);
        stream.out_uint16_le(this->gdiPCacheChunkSize.GdipObjectImageAttributesCacheChunkSize);

        stream.out_uint16_le(this->gdiPImageCacheProperties.GdipObjectImageCacheChunkSize);
        stream.out_uint16_le(this->gdiPImageCacheProperties.GdipObjectImageCacheTotalSize);
        stream.out_uint16_le(this->gdiPImageCacheProperties.GdipObjectImageCacheMaxSize);
    }

    void recv(InStream & stream, uint16_t len)
    {
        this->len = len;
        this->drawGDIPlusSupportLevel = stream.in_uint32_le();
        this->GdipVersion = stream.in_uint32_le();
        this->drawGdiplusCacheLevel = stream.in_uint32_le();

        this->gdiPCacheEntries.GdipGraphicsCacheEntries = stream.in_uint16_le();
        this->gdiPCacheEntries.GdipBrushCacheEntries = stream.in_uint16_le();
        this->gdiPCacheEntries.GdipPenCacheEntries = stream.in_uint16_le();
        this->gdiPCacheEntries.GdipImageCacheEntries = stream.in_uint16_le();
        this->gdiPCacheEntries.GdipImageAttributesCacheEntries = stream.in_uint16_le();

        this->gdiPCacheChunkSize.GdipGraphicsCacheChunkSize = stream.in_uint16_le();
        this->gdiPCacheChunkSize.GdipObjectBrushCacheChunkSize = stream.in_uint16_le();
        this->gdiPCacheChunkSize.GdipObjectPenCacheChunkSize = stream.in_uint16_le();
        this->gdiPCacheChunkSize.GdipObjectImageAttributesCacheChunkSize = stream.in_uint16_le();

        this->gdiPImageCacheProperties.GdipObjectImageCacheChunkSize = stream.in_uint16_le();
        this->gdiPImageCacheProperties.GdipObjectImageCacheTotalSize = stream.in_uint16_le();
        this->gdiPImageCacheProperties.GdipObjectImageCacheMaxSize = stream.in_uint16_le();
    }

    void log(const char * msg) const
    {
        LOG(LOG_INFO, "%s DrawGdiPlus caps (%u bytes)", msg, this->len);

        LOG(LOG_INFO, "DrawGdiPlus caps::drawGDIPlusSupportLevel %u", this->drawGDIPlusSupportLevel);
        LOG(LOG_INFO, "DrawGdiPlus caps::GdipVersion %u", this->GdipVersion);
        LOG(LOG_INFO, "DrawGdiPlus caps::drawGdiplusCacheLevel %u", this->drawGdiplusCacheLevel);

        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheEntries.GdipGraphicsCacheEntries %u", this->gdiPCacheEntries.GdipGraphicsCacheEntries);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheEntries.GdipBrushCacheEntries %u", this->gdiPCacheEntries.GdipBrushCacheEntries);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheEntries.GdipPenCacheEntries %u", this->gdiPCacheEntries.GdipPenCacheEntries);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheEntries.GdipImageCacheEntries %u", this->gdiPCacheEntries.GdipImageCacheEntries);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheEntries.GdipImageAttributesCacheEntries %u", this->gdiPCacheEntries.GdipImageAttributesCacheEntries);

        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheChunkSize.GdipGraphicsCacheChunkSize %u", this->gdiPCacheChunkSize.GdipGraphicsCacheChunkSize);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheChunkSize.GdipObjectBrushCacheChunkSize %u", this->gdiPCacheChunkSize.GdipObjectBrushCacheChunkSize);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheChunkSize.GdipObjectPenCacheChunkSize %u", this->gdiPCacheChunkSize.GdipObjectPenCacheChunkSize);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdiPCacheChunkSize.GdipObjectImageAttributesCacheChunkSize %u", this->gdiPCacheChunkSize.GdipObjectImageAttributesCacheChunkSize);

        LOG(LOG_INFO, "DrawGdiPlus caps::gdipImageCacheProperties->GdipObjectImageCacheChunkSize %u", this->gdiPImageCacheProperties.GdipObjectImageCacheChunkSize);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdipImageCacheProperties->GdipObjectImageCacheTotalSize %u", this->gdiPImageCacheProperties.GdipObjectImageCacheTotalSize);
        LOG(LOG_INFO, "DrawGdiPlus caps::gdipImageCacheProperties->GdipObjectImageCacheMaxSize %u", this->gdiPImageCacheProperties.GdipObjectImageCacheMaxSize);
    }
};
