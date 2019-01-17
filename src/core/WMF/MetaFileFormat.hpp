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
   Author(s): Clément Moroldo
*/


#pragma once

#include <cstdlib>

#include "utils/log.hpp"
#include "utils/stream.hpp"



namespace MFF {

enum {
   META_EOF                   = 0x0000,
   META_REALIZEPALETTE        = 0x0035,
   META_SETPALENTRIES         = 0x0037,
   META_SETBKMODE             = 0x0102,
   META_SETMAPMODE            = 0x0103,
   META_SETROP2               = 0x0104,
   META_SETRELABS             = 0x0105,
   META_SETPOLYFILLMODE       = 0x0106,
   META_SETSTRETCHBLTMODE     = 0x0107,
   META_SETTEXTCHAREXTRA      = 0x0108,
   META_RESTOREDC             = 0x0127,
   META_RESIZEPALETTE         = 0x0139,
   META_DIBCREATEPATTERNBRUSH = 0x0142,
   META_SETLAYOUT             = 0x0149,
   META_SETBKCOLOR            = 0x0201,
   META_SETTEXTCOLOR          = 0x0209,
   META_OFFSETVIEWPORTORG     = 0x0211,
   META_LINETO                = 0x0213,
   META_MOVETO                = 0x0214,
   META_OFFSETCLIPRGN         = 0x0220,
   META_FILLREGION            = 0x0228,
   META_SETMAPPERFLAGS        = 0x0231,
   META_SELECTPALETTE         = 0x0234,
   META_POLYGON               = 0x0324,
   META_POLYLINE              = 0x0325,
   META_SETTEXTJUSTIFICATION  = 0x020A,
   META_SETWINDOWORG          = 0x020B,
   META_SETWINDOWEXT          = 0x020C,
   META_SETVIEWPORTORG        = 0x020D,
   META_SETVIEWPORTEXT        = 0x020E,
   META_OFFSETWINDOWORG       = 0x020F,
   META_SCALEWINDOWEXT        = 0x0410,
   META_SCALEVIEWPORTEXT      = 0x0412,
   META_EXCLUDECLIPRECT       = 0x0415,
   META_INTERSECTCLIPRECT     = 0x0416,
   META_ELLIPSE               = 0x0418,
   META_FLOODFILL             = 0x0419,
   META_FRAMEREGION           = 0x0429,
   META_ANIMATEPALETTE        = 0x0436,
   META_TEXTOUT               = 0x0521,
   META_POLYPOLYGON           = 0x0538,
   META_EXTFLOODFILL          = 0x0548,
   META_RECTANGLE             = 0x041B,
   META_SETPIXEL              = 0x041F,
   META_ROUNDRECT             = 0x061C,
   META_PATBLT                = 0x061D,
   META_SAVEDC                = 0x001E,
   META_PIE                   = 0x081A,
   META_STRETCHBLT            = 0x0B23,
   META_ESCAPE                = 0x0626,
   META_INVERTREGION          = 0x012A,
   META_PAINTREGION           = 0x012B,
   META_SELECTCLIPREGION      = 0x012C,
   META_SELECTOBJECT          = 0x012D,
   META_SETTEXTALIGN          = 0x012E,
   META_ARC                   = 0x0817,
   META_CHORD                 = 0x0830,
   META_BITBLT                = 0x0922,
   META_EXTTEXTOUT            = 0x0a32,
   META_SETDIBTODEV           = 0x0d33,
   META_DIBBITBLT             = 0x0940,
   META_DIBSTRETCHBLT         = 0x0b41,
   META_STRETCHDIB            = 0x0f43,
   META_DELETEOBJECT          = 0x01f0,
   META_CREATEPALETTE         = 0x00f7,
   META_CREATEPATTERNBRUSH    = 0x01F9,
   META_CREATEPENINDIRECT     = 0x02FA,
   META_CREATEFONTINDIRECT    = 0x02FB,
   META_CREATEBRUSHINDIRECT   = 0x02FC,
   META_CREATEREGION          = 0x06FF
};

inline static const char * get_RecordType_name(uint32_t RecordType) {
    switch (RecordType) {
        case META_EOF:                   return "META_EOF";
        case META_REALIZEPALETTE:        return "META_REALIZEPALETTE";
        case META_SETPALENTRIES:         return "META_SETPALENTRIES";
        case META_SETBKMODE:             return "META_SETBKMODE";
        case META_SETMAPMODE:            return "META_SETMAPMODE";
        case META_SETROP2:               return "META_SETROP2";
        case META_SETRELABS:             return "META_SETRELABS";
        case META_SETPOLYFILLMODE:       return "META_SETPOLYFILLMODE";
        case META_SETSTRETCHBLTMODE:     return "META_SETSTRETCHBLTMODE";
        case META_SETTEXTCHAREXTRA:      return "META_SETTEXTCHAREXTRA";
        case META_RESTOREDC:             return "META_RESTOREDC";
        case META_RESIZEPALETTE:         return "META_RESIZEPALETTE";
        case META_DIBCREATEPATTERNBRUSH: return "META_DIBCREATEPATTERNBRUSH";
        case META_SETLAYOUT:             return "META_SETLAYOUT";
        case META_SETBKCOLOR:            return "META_SETBKCOLOR";
        case META_SETTEXTCOLOR:          return "META_SETTEXTCOLOR";
        case META_OFFSETVIEWPORTORG:     return "META_OFFSETVIEWPORTORG";
        case META_LINETO:                return "META_LINETO";
        case META_MOVETO:                return "META_MOVETO";
        case META_OFFSETCLIPRGN:         return "META_OFFSETCLIPRGN";
        case META_FILLREGION:            return "META_FILLREGION";
        case META_SETMAPPERFLAGS:        return "META_SETMAPPERFLAGS";
        case META_SELECTPALETTE:         return "META_SELECTPALETTE";
        case META_POLYGON:               return "META_POLYGON";
        case META_POLYLINE:              return "META_POLYLINE";
        case META_SETTEXTJUSTIFICATION:  return "META_SETTEXTJUSTIFICATION";
        case META_SETWINDOWORG:          return "META_SETWINDOWORG";
        case META_SETWINDOWEXT:          return "META_SETWINDOWEXT";
        case META_SETVIEWPORTORG:        return "META_SETVIEWPORTORG";
        case META_SETVIEWPORTEXT:        return "META_SETVIEWPORTEXT";
        case META_OFFSETWINDOWORG:       return "META_OFFSETWINDOWORG";
        case META_SCALEWINDOWEXT:        return "META_SCALEWINDOWEXT";
        case META_SCALEVIEWPORTEXT:      return "META_SCALEVIEWPORTEXT";
        case META_EXCLUDECLIPRECT:       return "META_EXCLUDECLIPRECT";
        case META_INTERSECTCLIPRECT:     return "META_INTERSECTCLIPRECT";
        case META_ELLIPSE:               return "META_ELLIPSE";
        case META_FLOODFILL:             return "META_FLOODFILL";
        case META_FRAMEREGION:           return "META_FRAMEREGION";
        case META_ANIMATEPALETTE:        return "META_ANIMATEPALETTE";
        case META_TEXTOUT:               return "META_TEXTOUT";
        case META_POLYPOLYGON:           return "META_POLYPOLYGON";
        case META_EXTFLOODFILL:          return "META_EXTFLOODFILL";
        case META_RECTANGLE:             return "META_RECTANGLE";
        case META_SETPIXEL:              return "META_SETPIXEL";
        case META_ROUNDRECT:             return "META_ROUNDRECT";
        case META_PATBLT:                return "META_PATBLT";
        case META_SAVEDC:                return "META_SAVEDC";
        case META_PIE:                   return "META_PIE";
        case META_STRETCHBLT:            return "META_STRETCHBLT";
        case META_ESCAPE:                return "META_ESCAPE";
        case META_INVERTREGION:          return "META_INVERTREGION";
        case META_PAINTREGION:           return "META_PAINTREGION";
        case META_SELECTCLIPREGION:      return "META_SELECTCLIPREGION";
        case META_SELECTOBJECT:          return "META_SELECTOBJECT";
        case META_SETTEXTALIGN:          return "META_SETTEXTALIGN";
        case META_ARC:                   return "META_ARC";
        case META_CHORD:                 return "META_CHORD";
        case META_BITBLT:                return "META_BITBLT";
        case META_EXTTEXTOUT:            return "META_EXTTEXTOUT";
        case META_SETDIBTODEV:           return "META_SETDIBTODEV";
        case META_DIBBITBLT:             return "META_DIBBITBLT";
        case META_DIBSTRETCHBLT:         return "META_DIBSTRETCHBLT";
        case META_STRETCHDIB:            return "META_STRETCHDIB";
        case META_DELETEOBJECT:          return "META_DELETEOBJECT";
        case META_CREATEPALETTE:         return "META_CREATEPALETTE";
        case META_CREATEPATTERNBRUSH:    return "META_CREATEPATTERNBRUSH";
        case META_CREATEPENINDIRECT:     return "META_CREATEPENINDIRECT";
        case META_CREATEFONTINDIRECT:    return "META_CREATEFONTINDIRECT";
        case META_CREATEBRUSHINDIRECT:   return "META_CREATEBRUSHINDIRECT";
        case META_CREATEREGION:          return "META_CREATEREGION";
    }

    return "<unknown>";
}

// 2.1.1.1 RecordType Enumeration

// The RecordType Enumeration defines the types of records that can be used in WMF metafiles.

// META_EOF:  This record specifies the end of the file, the last record in the metafile.

// META_REALIZEPALETTE:  This record maps entries from the logical palette that is defined in the playback device context to the system palette.

// META_SETPALENTRIES:  This record defines red green blue (RGB) color values in a range of entries in the logical palette that is defined in the playback device context.

// META_SETBKMODE:  This record defines the background raster operation mix mode in the playback device context. The background mix mode is the mode for combining pens, text, hatched brushes, and interiors of filled objects with background colors on the output surface.

// META_SETMAPMODE:  This record defines the mapping mode in the playback device context. The mapping mode defines the unit of measure used to transform page-space coordinates into coordinates of the output device, and also defines the orientation of the device's x and y axes.

// META_SETROP2:  This record defines the foreground raster operation mix mode in the playback device context. The foreground mix mode is the mode for combining pens and interiors of filled objects with foreground colors on the output surface.

// META_SETRELABS:  This record is undefined and MUST be ignored.

// META_SETPOLYFILLMODE:  This record defines polygon fill mode in the playback device context for graphics operations that fill polygons.

// META_SETSTRETCHBLTMODE:  This record defines the bitmap stretching mode in the playback device context.

// META_SETTEXTCHAREXTRA:  This record defines inter-character spacing for text justification in the playback device context. Spacing is added to the white space between each character, including break characters, when a line of justified text is output.

// META_RESTOREDC:  This record restores the playback device context from a previously saved device context.

// META_RESIZEPALETTE:  This record redefines the size of the logical palette that is defined in the playback device context.

// META_DIBCREATEPATTERNBRUSH:  This record defines a brush with a pattern specified by a device-independent bitmap (DIB).

// META_SETLAYOUT:  This record defines the layout orientation in the playback device context.<2>

// META_SETBKCOLOR:  This record sets the background color in the playback device context to a specified color, or to the nearest physical color if the device cannot represent the specified color.

// META_SETTEXTCOLOR:  This record defines the text color in the playback device context.

// META_OFFSETVIEWPORTORG:  This record moves the viewport origin in the playback device context by using specified horizontal and vertical offsets.

// META_LINETO:  This record draws a line from the output position that is defined in the playback device context up to, but not including, a specified point.

// META_MOVETO:  This record sets the output position in the playback device context to a specified point.

// META_OFFSETCLIPRGN:  This record moves the clipping region that is defined in the playback device context by specified offsets.

// META_FILLREGION:  This record fills a region by using a specified brush.

// META_SETMAPPERFLAGS:  This record defines the algorithm that the font mapper uses when it maps logical fonts to physical fonts.

// META_SELECTPALETTE:  This record specifies the logical palette in the playback device context.

// META_POLYGON:  This record paints a polygon consisting of two or more vertices connected by straight lines. The polygon is outlined by using the pen and filled by using the brush and polygon fill mode; these are defined in the playback device context.

// META_POLYLINE:  This record draws a series of line segments by connecting the points in a specified array.

// META_SETTEXTJUSTIFICATION:  This record defines the amount of space to add to break characters in a string of justified text.

// META_SETWINDOWORG:  This record defines the output window origin in the playback device context.

// META_SETWINDOWEXT:  This record defines the horizontal and vertical extents of the output window in the playback device context.

// META_SETVIEWPORTORG:  This record defines the viewport origin in the playback device context.

// META_SETVIEWPORTEXT:  This record defines the horizontal and vertical extents of the viewport in the playback device context.

// META_OFFSETWINDOWORG:  This record moves the output window origin in the playback device context by using specified horizontal and vertical offsets.

// META_SCALEWINDOWEXT:  This record scales the horizontal and vertical extents of the output window that is defined in the playback device context by using the ratios formed by specified multiplicands and divisors.

// META_SCALEVIEWPORTEXT:  This record scales the horizontal and vertical extents of the viewport that is defined in the playback device context by using the ratios formed by specified multiplicands and divisors.

// META_EXCLUDECLIPRECT:  This record sets the clipping region that is defined in the playback device context to the existing clipping region minus a specified rectangle.

// META_INTERSECTCLIPRECT:  This record sets the clipping region that is defined in the playback device context to the intersection of the existing clipping region and a specified rectangle.

// META_ELLIPSE:  This record defines an ellipse. The center of the ellipse is the center of a specified bounding rectangle. The ellipse is outlined by using the pen and is filled by using the brush; these are defined in the playback device context.

// META_FLOODFILL:  This record fills an area of the display surface with the brush that is defined in the playback device context.

// META_FRAMEREGION:  This record defines a border around a specified region by using a specified brush.

// META_ANIMATEPALETTE:  This record redefines entries in the logical palette that is defined in the playback device context.

// META_TEXTOUT:  This record outputs a character string at a specified location using the font, background color, and text color; these are defined in the playback device context.

// META_POLYPOLYGON:  This record paints a series of closed polygons. Each polygon is outlined by using the pen and filled by using the brush and polygon fill mode; these are defined in the playback device context. The polygons drawn in this operation can overlap.

// META_EXTFLOODFILL:  This record fills an area with the brush that is defined in the playback device context.

// META_RECTANGLE:  This record paints a rectangle. The rectangle is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_SETPIXEL:  This record sets the pixel at specified coordinates to a specified color.

// META_ROUNDRECT:  This record draws a rectangle with rounded corners. The rectangle is outlined by using the current pen and filled by using the current brush.

// META_PATBLT:  This record paints the specified rectangle by using the brush that is currently selected into the playback device context. The brush color and the surface color or colors are combined using the specified raster operation.

// META_SAVEDC:  This record saves the playback device context for later retrieval.

// META_PIE:  This record draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials. The pie is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_STRETCHBLT:  This record specifies the transfer of a block of pixels according to a raster operation, with possible expansion or contraction.

// META_ESCAPE:  This record makes it possible to access capabilities of a particular printing device that are not directly available through other WMF records.

// META_INVERTREGION:  This record inverts the colors in a specified region.

// META_PAINTREGION:  This record paints a specified region by using the brush that is defined in the playback device context.

// META_SELECTCLIPREGION:  This record specifies the clipping region in the playback device context.

// META_SELECTOBJECT:  This record specifies a graphics object in the playback device context. The new object replaces the previous object of the same type, if one is defined.

// META_SETTEXTALIGN:  This record defines the text-alignment values in the playback device context.

// META_ARC:  This record draws an elliptical arc.

// META_CHORD:  This record draws a chord, which is a region bounded by the intersection of an ellipse and a line segment. The chord is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_BITBLT:  This record specifies the transfer of a block of pixels according to a raster operation.

// META_EXTTEXTOUT:  This record outputs a character string by using the font, background color, and text color; these are defined in the playback device context. Optionally, dimensions can be provided for clipping, opaquing, or both.

// META_SETDIBTODEV:  This record sets a block of pixels using device-independent color data.

// META_DIBBITBLT:  This record specifies the transfer of a block of pixels in device-independent format according to a raster operation.

// META_DIBSTRETCHBLT:  This record specifies the transfer of a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.

// META_STRETCHDIB:  This record specifies the transfer of color data from a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.

// META_DELETEOBJECT:  This record deletes a graphics object, which can be a pen, brush, font, region, or palette.

// META_CREATEPALETTE:  This record defines a logical palette.

// META_CREATEPATTERNBRUSH:  This record defines a brush with a pattern specified by a DIB.

// META_CREATEPENINDIRECT:  This record defines a pen with specified style, width, and color.

// META_CREATEFONTINDIRECT:  This record defines a font with specified characteristics.

// META_CREATEBRUSHINDIRECT:  This record defines a brush with specified style, color, and pattern.

// META_CREATEREGION:  This record defines a region.

// The high-order byte of the WMF record type values SHOULD be ignored for all record types except the following.<3>

//     META_BITBLT

//     META_DIBBITBLT

//     META_DIBSTRETCHBLT

//     META_POLYGON

//     META_POLYLINE

//     META_SETPALENTRIES

//     META_STRETCHBLT

// The meanings of the high-order bytes of these record type fields are specified in the respective sections that define them.

// A record type is not defined for the WMF Header record, because only one can be present as the first record in the metafile.




// 2.1.1.18 MetafileType Enumeration

// The MetafileType Enumeration specifies where the metafile is stored.

enum {
    MEMORYMETAFILE = 0x0001,
    DISKMETAFILE   = 0x0002
};

// MEMORYMETAFILE:  Metafile is stored in memory.

// DISKMETAFILE:  Metafile is stored on disk.



// 2.1.1.19 MetafileVersion Enumeration

// The MetafileVersion Enumeration defines values that specify support for device-independent bitmaps (DIBs) in metafiles.

enum {
    METAVERSION100 = 0x0100,
    METAVERSION300 = 0x0300
};

// METAVERSION100:  DIBs are not supported.

// METAVERSION300:  DIBs are supported.

 inline static const char * get_MetaVersion_name(uint32_t MetaVersion) {
    switch (MetaVersion) {
        case METAVERSION100: return "METAVERSION100";
        case METAVERSION300: return "METAVERSION300";
    }

    return "<unknown>";
}


enum : int {
      METAFILE_HEADERS_SIZE          = 130
    , META_DIBSTRETCHBLT_HEADER_SIZE = 66
    , METAFILE_WORDS_HEADER_SIZE     = 59                   //(METAFILE_HEADERS_SIZE/2) -6
    , META_HEADER_SIZE               = 9

    /*, META_SETMAPMODE_WORDS_SIZE   = 4
    , META_SETWINDOWEXT_WORDS_SIZE = 5
    , META_SETWINDOWORG_WORDS_SIZE = 5*/
    , BITMAPINFO_HEADER_SIZE       = 40

};



    // 3.2.1 META_HEADER Example

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |             Type              |          HeaderSize           |
    // +-------------------------------+-------------------------------+
    // |           Version             |             Size              |
    // +-------------------------------+-------------------------------+
    // |             ...               |        NumberOfObjects        |
    // +-------------------------------+-------------------------------+
    // |                     metaFileData (variable)                   |
    // +-------------------------------+-------------------------------+
    // |       NumberOfMembers         |                               |
    // +-------------------------------+-------------------------------+

    // Type: 0x0001 specifies the type of metafile from the MetafileType Enumeration
    // (section 2.1.1.18) to be a metafile stored in memory.

    // HeaderSize: 0x0009 specifies the number of WORDs in this record, which is equivalent
    // to 18 (0x0012) bytes.

    // Version: 0x0300 specifies the metafile version from the MetafileVersion Enumeration
    // (section 2.1.1.19) to be a WMF metafile that supports DIBs.

    // Size: 0x00000036 specifies the number of WORDs in the entire metafile, which is
    // equivalent to 108 (0x0000006C) bytes.

    // NumberOfObjects: 0x0002 specifies the number of graphics objects that are defined in the metafile.

    // MaxRecord: 0x0000000C specifies the size in WORDs of the largest record in the
    // metafile, which is equivalent to 24 (0x00000018) bytes.

    // NumberOfMembers: 0x0000 is not used.

    // Note Based on the value of the NumberOfObjects field, a WMF Object Table (section 3.1.4.1)
    // can be created that is large enough for 2 objects.

    struct MetaHeader {

        uint16_t type = 0;
        uint16_t headerSize = 0;
        uint16_t version = 0;
        uint32_t size = 0;
        uint16_t numberOfObjects = 0;
        uint32_t maxRecord = 0;
        uint16_t numberOfMembers = 0;


        MetaHeader() = default;

        MetaHeader(uint16_t type, uint16_t version, const std::size_t data_length)
          : type(type)
          , headerSize(9)
          , version(version)
          , size((data_length/2) + METAFILE_WORDS_HEADER_SIZE)
          , maxRecord((data_length + META_DIBSTRETCHBLT_HEADER_SIZE)/2)
        {}

        void emit(OutStream & stream) const {
            stream.out_uint16_le(this->type);
            stream.out_uint16_le(this->headerSize);
            stream.out_uint16_le(this->version);
            stream.out_uint32_le(this->size);
            stream.out_uint16_le(this->numberOfObjects);
            stream.out_uint32_le(this->maxRecord);
            stream.out_uint16_le(this->numberOfMembers);
        }

        void recv(InStream & stream) {
            this->type = stream.in_uint16_le();
            this->headerSize = stream.in_uint16_le();
            assert(this->headerSize == 9);
            this->version = stream.in_uint16_le();
            this->size = stream.in_uint32_le();
            assert(this->size >=  METAFILE_WORDS_HEADER_SIZE);
            this->numberOfObjects = stream.in_uint16_le();
            this->maxRecord = stream.in_uint32_le();
            assert(this->maxRecord >= META_DIBSTRETCHBLT_HEADER_SIZE);
            this->numberOfMembers = stream.in_uint16_le();
            assert(this->numberOfMembers == 0);
        }

        void log() const {
            LOG(LOG_INFO, "Meta Header: type=0x%04x(2 bytes) headerSize= %d(2 bytes) version=0x%04x(2 bytes):%s size=%u(4 bytes) numberOfObjects=%u(2 bytes) maxRecord=%u(4 bytes) numberOfMembers=%u(2 bytes)", this->type, this->headerSize, this->version, get_MetaVersion_name(this->version), this->size, this->numberOfObjects, this->maxRecord, this->numberOfMembers);
        }

    };



    // 2.3 WMF Records

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                           RecordSize                          |
    // +-------------------------------+-------------------------------+
    // |        RecordFunction         |           rdParam             |
    // +-------------------------------+-------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // RecordSize (4 bytes): A 32-bit unsigned integer that defines the number of 16-bit WORDs
    // in the record.

    // RecordFunction (2 bytes): A 16-bit unsigned integer that defines the type of this record.
    // The low-order byte MUST match the low-order byte of one of the values in the RecordType Enumeration.

    // rdParam (variable): An optional place holder that is provided for record-specific fields.

    struct Record {

        uint32_t recordSize;
        uint16_t recordFunction;

        Record(const uint32_t recordSize, const uint16_t recordFunction)
          : recordSize(recordSize)
          , recordFunction(recordFunction)
        {}

        void emit(OutStream & stream) const {
            stream.out_uint32_le(this->recordSize);
            stream.out_uint16_le(this->recordFunction);
        }

        void recv(InStream & stream) {
            this->recordSize = stream.in_uint32_le();
            this->recordFunction = stream.in_uint16_le();
        }

        void log() const {
            LOG(LOG_INFO, "Record Header: recordSize=%u(4 bytes) recordFunction=0x%04x(2 bytes):%s", this->recordSize, this->recordFunction, MFF::get_RecordType_name(this->recordFunction));
        }
    };



    // 2.3.5.17 META_SETMAPMODE Record

    // The META_SETMAPMODE record defines the mapping mode in the playback device context. The mapping mode defines the unit of measure used to transform page-space units into device-space units, and also defines the orientation of the device's x and y axes.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          RecordSize                           |
    // +-------------------------------+-------------------------------+
    // |        RecordFunction         |            MapMode            |
    // +-------------------------------+-------------------------------+

    // RecordSize (4 bytes): A 32-bit unsigned integer that defines the number of WORDs in the WMF record.

    // RecordFunction (2 bytes): A 16-bit unsigned integer that defines this WMF record type. The lower byte MUST match the lower byte of the RecordType Enumeration table value META_SETMAPMODE.

    // MapMode (2 bytes): A 16-bit unsigned integer that defines the mapping mode. This MUST be one of the values enumerated in the MapMode Enumeration table.

    // See section 2.3.5 for the specification of other State record types.

    struct MetaSetMapMod : Record {

        uint16_t mappingMode{0};

        explicit MetaSetMapMod()
          : Record(4, MFF::META_SETMAPMODE)

        {}

        explicit MetaSetMapMod(const uint16_t mappingMode)
          : Record(4, MFF::META_SETMAPMODE)
          , mappingMode(mappingMode)
        {}

        void emit(OutStream & stream) const {
            Record::emit(stream);
            stream.out_uint16_le(this->mappingMode);
        }

        void recv(InStream & stream) {
            Record::recv(stream);
            assert(this->recordSize == 4);
            this->mappingMode = stream.in_uint16_le();
        }

        void log() const {
            Record::log();
            LOG(LOG_INFO, "Meta Set Map Mod: mappingMode=0x%04x(2 bytes)", this->mappingMode);
        }

    };



    // 2.3.5.30 META_SETWINDOWEXT Record

    // The META_SETWINDOWEXT record defines the horizontal and vertical extents of the output window in the playback device context.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          RecordSize                           |
    // +-------------------------------+-------------------------------+
    // |        RecordFunction         |                 Y             |
    // +-------------------------------+-------------------------------+
    // |                X              |                               |
    // +-------------------------------+-------------------------------+

    // RecordSize (4 bytes):  A 32-bit unsigned integer that defines the number of WORDs in the WMF record.

    // RecordFunction (2 bytes):  A 16-bit unsigned integer that defines this WMF record type. The lower byte MUST match the lower byte of the RecordType Enumeration table value META_SETWINDOWEXT.

    // Y (2 bytes):  A 16-bit signed integer that defines the vertical extent of the window in logical units.

    // X (2 bytes):  A 16-bit signed integer that defines the horizontal extent of the window in logical units.

    // See section 2.3.5 for the specification of other State record types.

    struct MetaSetWindowExt : Record {

        uint16_t height{0};
        uint16_t width{0};

        MetaSetWindowExt()
          : Record(5, MFF::META_SETWINDOWEXT)

        {}

        MetaSetWindowExt(const uint16_t height, const uint16_t width)
          : Record(5, MFF::META_SETWINDOWEXT)
          , height( - height)
          , width(width)
        {}

        void emit(OutStream & stream) const {
            Record::emit(stream);
            stream.out_uint16_le(this->height);
            stream.out_uint16_le(this->width);
        }

        void recv(InStream & stream) {
            Record::recv(stream);
            assert(this->recordSize == 5);
            this->height = stream.in_uint16_le();
            this->width = stream.in_uint16_le();
        }

        void log() const {
            Record::log();
            LOG(LOG_INFO, "Meta Set Window Ext: height=0x%04x(2 bytes) width=0x%04x(2 bytes)", this->height, this->width);
        }
    };



    // 2.3.5.31 META_SETWINDOWORG Record

    // The META_SETWINDOWORG record defines the output window origin in the playback device context.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          RecordSize                           |
    // +-------------------------------+-------------------------------+
    // |        RecordFunction         |                 Y             |
    // +-------------------------------+-------------------------------+
    // |                X              |                               |
    // +-------------------------------+-------------------------------+

    // RecordSize (4 bytes):  A 32-bit unsigned integer that defines the number of WORDs in the WMF record.

    // RecordFunction (2 bytes):  A 16-bit unsigned integer that defines this WMF record type. The lower byte MUST match the lower byte of the RecordType Enumeration table value META_SETWINDOWORG.

    // Y (2 bytes):  A 16-bit signed integer that defines the y-coordinate, in logical units.

    // X (2 bytes):  A 16-bit signed integer that defines the x-coordinate, in logical units.

    // See section 2.3.5 for the specification of other State record types.

    struct MetaSetWindowOrg : Record {

        uint16_t yOrg{0};
        uint16_t xOrg{0};

        MetaSetWindowOrg()
          : Record(5, MFF::META_SETWINDOWORG)

        {}

        MetaSetWindowOrg(const uint16_t yOrg, const uint16_t xOrg)
          : Record(5, MFF::META_SETWINDOWORG)
          , yOrg(yOrg)
          , xOrg(xOrg)
        {}

        void emit(OutStream & stream) const {
            Record::emit(stream);
            stream.out_uint16_le(yOrg);
            stream.out_uint16_le(xOrg);
        }

        void recv(InStream & stream) {
            Record::recv(stream);
            assert(this->recordSize == 5);
            this->yOrg = stream.in_uint16_le();
            this->xOrg = stream.in_uint16_le();
        }

        void log() const {
            Record::log();
            LOG(LOG_INFO, "Meta Set Window Org: yOrg=%d(2 bytes) xOrg=%d(2 bytes)", this->yOrg, this->xOrg);
        }
    };




    // [MS-WMF]
    // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                     DIBHeaderInfo (variable)                  |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                        Colors (variable)                      |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                    BitmapBuffer (variable)                    |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // DIBHeaderInfo (variable): Either a BitmapCoreHeader Object (section 2.2.2.2) or a BitmapInfoHeader
    // Object (section 2.2.2.3) that specifies information about the image.

    // The first 32 bits of this field is the HeaderSize value. If it is 0x0000000C, then this is a
    // BitmapCoreHeader; otherwise, this is a BitmapInfoHeader.

    // Colors (variable): An optional array of either RGBQuad Objects (section 2.2.2.20) or 16-bit unsigned
    // integers that define a color table.

    // The size and contents of this field SHOULD be determined from the metafile record or object that
    // contains this DeviceIndependentBitmap and from information in the DIBHeaderInfo field. See ColorUsage
    // Enumeration (section 2.1.1.6) and BitCount Enumeration (section 2.1.1.3) for additional details.


    // BitmapBuffer (variable): A buffer containing the image, which is not required to be contiguous with the
    // DIB header, unless this is a packed bitmap.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                    UndefinedSpace (variable)                  |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+
    // |                        aData (variable)                       |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // UndefinedSpace (variable): An optional field that MUST be ignored. If this DIB is a packed bitmap,
    // this field MUST NOT be present.

    // aData (variable): An array of bytes that define the image.

    //      The size and format of this data is determined by information in the DIBHeaderInfo field.
    // If it is a BitmapCoreHeader, the size in bytes MUST be calculated as follows:

    //              (((Width * Planes * BitCount + 31) & ~31) / 8) * abs(Height)

    //      This formula SHOULD also be used to calculate the size of aData when DIBHeaderInfo is a BitmapInfoHeader
    // Object, using values from that object, but only if its Compression value is BI_RGB, BI_BITFIELDS, or BI_CMYK.

    //      Otherwise, the size of aData MUST be the BitmapInfoHeader Object value ImageSize.

    // 2.2.2.3 BitmapInfoHeader Object

    // The BitmapInfoHeader Object contains information about the dimensions and color format of a device-independent
    // bitmap (DIB).

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                           HeaderSize                          |
    // +---------------------------------------------------------------+
    // |                             Width                             |
    // +---------------------------------------------------------------+
    // |                            Height                             |
    // +-------------------------------+-------------------------------+
    // |          Planes               |           BitCount            |
    // +-------------------------------+-------------------------------+
    // |                         Compression                           |
    // +---------------------------------------------------------------+
    // |                          ImageSize                            |
    // +---------------------------------------------------------------+
    // |                         XPelsPerMeter                         |
    // +---------------------------------------------------------------+
    // |                         YPelsPerMeter                         |
    // +---------------------------------------------------------------+
    // |                          ColorUsed                            |
    // +---------------------------------------------------------------+
    // |                        ColorImportant                         |
    // +---------------------------------------------------------------+

    // HeaderSize (4 bytes): A 32-bit unsigned integer that defines the size of this object, in bytes.

    // Width (4 bytes): A 32-bit signed integer that defines the width of the DIB, in pixels. This value
    // MUST be positive.

    //          This field SHOULD specify the width of the decompressed image file, if the
    //          Compression value specifies JPEG or PNG format.<44>

    // Height (4 bytes): A 32-bit signed integer that defines the height of the DIB, in pixels. This value MUST NOT
    // be zero.

    // Planes (2 bytes): A 16-bit unsigned integer that defines the number of planes for the target device.
    // This value MUST be 0x0001.

    // BitCount (2 bytes): A 16-bit unsigned integer that defines the number of bits that define each pixel
    // and the maximum number of colors in the DIB. This value MUST be in the BitCount Enumeration (section 2.1.1.3).

    // Compression (4 bytes): A 32-bit unsigned integer that defines the compression mode of the DIB. This value
    // MUST be in the Compression Enumeration (section 2.1.1.7).

    //          This value MUST NOT specify a compressed format if the DIB is a top-down bitmap, as indicated by
    //          the Height value.

    // ImageSize (4 bytes): A 32-bit unsigned integer that defines the size, in bytes, of the image.

    //          If the Compression value is BI_RGB, this value SHOULD be zero and MUST be ignored.<45>

    //          If the Compression value is BI_JPEG or BI_PNG, this value MUST specify the size of the JPEG or PNG
    //          image buffer, respectively.

    // XPelsPerMeter (4 bytes): A 32-bit signed integer that defines the horizontal resolution, in pixels-per-meter,
    // of the target device for the DIB.

    // YPelsPerMeter (4 bytes): A 32-bit signed integer that defines the vertical resolution, in pixels-per-meter,
    // of the target device for the DIB.

    // ColorUsed (4 bytes): A 32-bit unsigned integer that specifies the number of indexes in the color table
    // used by the DIB, as follows:

    //           If this value is zero, the DIB uses the maximum number of colors that correspond to the BitCount value.

    //           If this value is nonzero and the BitCount value is less than 16, this value specifies the number
    //           of colors used by the DIB.

    //           If this value is nonzero and the BitCount value is 16 or greater, this value specifies the size
    //           of the color table used to optimize performance of the system palette.

    //           Note If this value is nonzero and greater than the maximum possible size of the color table
    //          based on the BitCount value, the maximum color table size SHOULD be assumed.

    // ColorImportant (4 bytes): A 32-bit unsigned integer that defines the number of color indexes that are
    // required for displaying the DIB. If this value is zero, all color indexes are required.

    //           A DIB is specified by a DeviceIndependentBitmap Object (section 2.2.2.9).

    //           When the array of pixels in the DIB immediately follows the BitmapInfoHeader, the DIB is a packed
    //           bitmap. In a packed bitmap, the ColorUsed value MUST be either 0x00000000 or the actual size
    //           of the color table.

    struct DibStretchBLT : public Record {

        struct BitmapInfoHeader {
            enum : uint32_t {
               PLANES_NUMBER = 0x0001
            };

            uint32_t headerSize = 40;
            uint32_t height = 0;
            uint32_t width = 0;
            uint16_t planes = 0;
            uint16_t bitCount = 0;
            uint32_t compression = 0;
            uint32_t imageSize = 0;
            uint32_t xPelsPerMeter = 0;
            uint32_t yPelsPerMeter = 0;
            uint32_t colorUsed = 0;
            uint32_t colorImportant = 0;

            BitmapInfoHeader() = default;

            BitmapInfoHeader(const std::size_t data_length, const uint16_t height, const uint16_t width, const uint16_t bitCount)
              : height( - height) // TODO UB ?
              , width(width)
              , planes(PLANES_NUMBER)
              , bitCount(bitCount)
              , imageSize(data_length)
            {}

            void emit(OutStream & stream) const {
                stream.out_uint32_le(this->headerSize);
                stream.out_uint32_le(this->width);
                stream.out_uint32_le(this->height);
                stream.out_uint16_le(this->planes);
                stream.out_uint16_le(this->bitCount);
                stream.out_uint32_le(this->compression);
                stream.out_uint32_le(this->imageSize);
                stream.out_uint32_le(this->xPelsPerMeter);
                stream.out_uint32_le(this->yPelsPerMeter);
                stream.out_uint32_le(this->colorUsed);
                stream.out_uint32_le(this->colorImportant);
            }

            void recv(InStream & stream) {
                this->headerSize = stream.in_uint32_le();
                this->width = stream.in_uint32_le();
                this->height = stream.in_uint32_le();
                this->planes = stream.in_uint16_le();
                assert(this->planes == PLANES_NUMBER);
                this->bitCount = stream.in_uint16_le();
                this->compression = stream.in_uint32_le();
                this->imageSize = stream.in_uint32_le();
                this->xPelsPerMeter = stream.in_uint32_le();
                this->yPelsPerMeter = stream.in_uint32_le();
                this->colorUsed = stream.in_uint32_le();
                this->colorImportant = stream.in_uint32_le();
            }

            void log() const {
                LOG(LOG_INFO, "Bitmap Info Header: headerSize=%d(4 bytes) width=%d(4 bytes) height=%d(4 bytes) planes=0x%04x(2 bytes) bitCount=%d(2 bytes) compression=0x%08x(4 bytes) imageSize=%d(4 bytes) xPelsPerMeter=%d(4 bytes) yPelsPerMeter=%d(4 bytes) colorUsed=%d(4 bytes) colorImportant=%d(4 bytes)", int(this->headerSize), int(this->width), int(this->height), this->planes, int(this->bitCount), this->compression, int(this->imageSize), int(this->xPelsPerMeter), int(this->yPelsPerMeter), int(this->colorUsed), int(this->colorImportant));
            }

        } bitmapInfoHeader;



        // 2.3.1.3.1 META_DIBSTRETCHBLT With Bitmap

        // This section specifies the structure of the META_DIBSTRETCHBLT record when it contains an
        // embedded device-independent bitmap (DIB).

        // Fields not specified in this section are specified in the preceding META_DIBSTRETCHBLT section.

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                           RecordSize                          |
        // +-------------------------------+-------------------------------+
        // |        RecordFunction         |           rdParam             |
        // +-------------------------------+-------------------------------+
        // |             ...               |          SrcHeight            |
        // +-------------------------------+-------------------------------+
        // |           SrcWidth            |             YSrc              |
        // +-------------------------------+-------------------------------+
        // |             XSrc              |         DestHeight            |
        // +-------------------------------+-------------------------------+
        // |          DestWidth            |             YDest             |
        // +-------------------------------+-------------------------------+
        // |            XDest              |      Target (variable)        |
        // +-------------------------------+-------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // RecordFunction (2 bytes): A 16-bit unsigned integer that defines this WMF record type.
        // The low-order byte MUST match the low-order byte of the RecordType enumeration (section 2.1.1.1)
        // value META_DIBSTRETCHBLT. The high-order byte MUST contain a value equal to the number of 16-bit
        // WORDs in the record minus the number of WORDs in the RecordSize and Target fields. That is:

        //      RecordSize - (2 + (sizeof(Target)/2))

        // Target (variable): A variable-sized DeviceIndependentBitmap Object (section 2.2.2.9) that defines
        // image content. This object MUST be specified, even if the raster operation does not require a source.

        uint32_t rasterOperation{0};
        uint16_t srcHeight{0};
        uint16_t srcWidth{0};
        uint16_t ySrc{0};
        uint16_t xSrc{0};
        uint16_t destHeight{0};
        uint16_t destWidth{0};
        uint16_t yDest{0};
        uint16_t xDest{0};

        DibStretchBLT()
        : Record(META_DIBSTRETCHBLT_HEADER_SIZE/2, MFF::META_DIBSTRETCHBLT)
        , bitmapInfoHeader(0, 0, 0, 0)

        {}

        DibStretchBLT(const std::size_t data_length, const uint16_t height, const uint16_t width, const uint16_t depth, uint32_t op)
        : Record((data_length + META_DIBSTRETCHBLT_HEADER_SIZE)/2, MFF::META_DIBSTRETCHBLT)
        , bitmapInfoHeader(data_length, height, width, depth)
        , rasterOperation(op)
        , srcHeight(height)
        , srcWidth(width)
        , ySrc(0)
        , xSrc(0)
        , destHeight( - height)
        , destWidth(width)
        , yDest(0)
        , xDest(0)
        {
            //assert( (this->srcHeight * this->srcWidth * this->bitmapInfoHeader.bitCount / 8) == int(this->bitmapInfoHeader.imageSize));
//             assert(uint16_t(this->bitmapInfoHeader.height) == this->destHeight);
//             assert(uint16_t(this->bitmapInfoHeader.width) == this->destWidth);
        }

        void emit(OutStream & stream) const {
            Record::emit(stream);
            stream.out_uint32_le(this->rasterOperation);
            stream.out_uint16_le(this->srcHeight);
            stream.out_uint16_le(this->srcWidth);
            stream.out_uint16_le(this->ySrc);
            stream.out_uint16_le(this->xSrc);
            stream.out_uint16_le(this->destHeight);
            stream.out_uint16_le(this->destWidth);
            stream.out_uint16_le(this->yDest);
            stream.out_uint16_le(this->xDest);

            this->bitmapInfoHeader.emit(stream);
        }

        void recv(InStream & stream) {
            Record::recv(stream);
            assert(this->recordSize >= META_DIBSTRETCHBLT_HEADER_SIZE/2);
            this->rasterOperation = stream.in_uint32_le();
            this->srcHeight = stream.in_uint16_le();
            this->srcWidth = stream.in_uint16_le();
            this->ySrc = stream.in_uint16_le();
            this->xSrc = stream.in_uint16_le();
            this->destHeight = stream.in_uint16_le();
            this->destWidth =  stream.in_uint16_le();
            this->yDest =  stream.in_uint16_le();
            this->xDest = stream.in_uint16_le();

            this->bitmapInfoHeader.recv(stream);
            //assert( (this->srcHeight * this->srcWidth * this->bitmapInfoHeader.bitCount / 8) == int(this->bitmapInfoHeader.imageSize));
//             assert(uint16_t(this->bitmapInfoHeader.height) == this->destHeight);
//             assert(uint16_t(this->bitmapInfoHeader.width) == this->destWidth);
        }

        void log() const {
            Record::log();
            LOG(LOG_INFO, "Dib Stretch BLT: rasterOperation=0x%08x(4 bytes) srcHeight=%d(2 bytes) srcWidth=%d(2 bytes) ySrc=%d(2bytes) xSrc=%d(2 bytes) destHeight=%d(2 bytes) destWidth=%d(2 bytes) yDest=%d(2 bytes) xDest=%d(2 bytes)", this->rasterOperation, this->srcHeight, this->srcWidth, this->ySrc, this->xSrc, this->destHeight, this->destWidth, this->yDest, this->xDest);

            this->bitmapInfoHeader.log();
        }
    };
}  // namespace MFF
