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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/RDP/capabilities/general.hpp"

// 2.2.9.1.1.3.1.2 Bitmap Update (TS_UPDATE_BITMAP)
// ------------------------------------------------
// The TS_UPDATE_BITMAP structure contains one or more rectangular clippings
//  taken from the server-side screen frame buffer (see [T128] section 8.17).
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        shareDataHeader                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |     bitmapData (variable)     |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//  containing information about the packet. The type subfield of the pduType
//  field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be set to
//  PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST be
//  set to PDUTYPE2_UPDATE (2).

// bitmapData (variable): The actual bitmap update data, as specified in
//  section 2.2.9.1.1.3.1.2.1.

// 2.2.9.1.1.3.1.2.1 Bitmap Update Data (TS_UPDATE_BITMAP_DATA)
// ------------------------------------------------------------
// The TS_UPDATE_BITMAP_DATA structure encapsulates the bitmap data that
//  defines a Bitmap Update (section 2.2.9.1.1.3.1.2).
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           updateType          |        numberRectangles       |
// +-------------------------------+-------------------------------+
// |                     rectangles (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// updateType (2 bytes): A 16-bit, unsigned integer. The update type. This
//  field MUST be set to UPDATETYPE_BITMAP (0x0001).

enum {
      UPDATETYPE_BITMAP = 0x0001
};

// numberRectangles (2 bytes): A 16-bit, unsigned integer. The number of
//  screen rectangles present in the rectangles field.

// rectangles (variable): Variable-length array of TS_BITMAP_DATA (section
//  2.2.9.1.1.3.1.2.2) structures, each of which contains a rectangular
//  clipping taken from the server-side screen frame buffer. The number of
//  screen clippings in the array is specified by the numberRectangles field.

// 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
// ----------------------------------------------
// The TS_BITMAP_DATA structure wraps the bitmap data for a screen area
//  rectangle containing a clipping taken from the server-side screen frame
//  buffer.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |            destLeft           |            destTop            |
// +-------------------------------+-------------------------------+
// |           destRight           |           destBottom          |
// +-------------------------------+-------------------------------+
// |              width            |             height            |
// +-------------------------------+-------------------------------+
// |           bitsPerPixel        |             flags             |
// +-------------------------------+-------------------------------+
// |           bitmapLength        |   bitmapComprHdr (optional)   |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |  bitmapDataStream (variable)  |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+

// destLeft (2 bytes): A 16-bit, unsigned integer. Left bound of the
//  rectangle.

// destTop (2 bytes): A 16-bit, unsigned integer. Top bound of the rectangle.

// destRight (2 bytes): A 16-bit, unsigned integer. Inclusive right bound of
//  the rectangle.

// destBottom (2 bytes): A 16-bit, unsigned integer. Inclusive bottom bound
//  of the rectangle.

// width (2 bytes): A 16-bit, unsigned integer. The width of the rectangle.

// height (2 bytes): A 16-bit, unsigned integer. The height of the rectangle.

// bitsPerPixel (2 bytes): A 16-bit, unsigned integer. The color depth of the
//  rectangle data in bits-per-pixel.

// flags (2 bytes): A 16-bit, unsigned integer. The flags describing the
//  format of the bitmap data in the bitmapDataStream field.

// +---------------------------+-----------------------------------------------+
// | Flags                     | Meaning                                       |
// +---------------------------+-----------------------------------------------+
// | BITMAP_COMPRESSION        | Indicates that the bitmap data is compressed. |
// | 0x0001                    | The bitmapComprHdr field MUST be present if   |
// |                           | the NO_BITMAP_COMPRESSION_HDR (0x0400) flag   |
// |                           | is not set.                                   |
// +---------------------------+-----------------------------------------------+
// | NO_BITMAP_COMPRESSION_HDR | Indicates that the bitmapComprHdr field is    |
// | 0x0400                    | not present (removed for bandwidth            |
// |                           | efficiency to save 8 bytes).                  |
// +---------------------------+-----------------------------------------------+

enum {
      BITMAP_COMPRESSION        = 0x0001
//    , NO_BITMAP_COMPRESSION_HDR = 0x0400
};

// bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//  the data in the bitmapComprHdr and bitmapDataStream fields.

// bitmapComprHdr (8 bytes): Optional Compressed Data Header structure
//  (section 2.2.9.1.1.3.1.2.3) specifying the bitmap data in the
//  bitmapDataStream. This field MUST be present if the BITMAP_COMPRESSION
//  (0x0001) flag is present in the Flags field, but the
//  NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not.

    // cbCompFirstRowSize (2 bytes): A 16-bit, unsigned integer. The field MUST
    // be set to 0x0000.

    // cbCompMainBodySize (2 bytes): A 16-bit, unsigned integer. The size in bytes
    // of the compressed bitmap data (which follows this header).

    // cbScanWidth (2 bytes): A 16-bit, unsigned integer. The width of the bitmap
    // (which follows this header) in pixels (this value MUST be divisible by 4).

    // cbUncompressedSize (2 bytes): A 16-bit, unsigned integer. The size in bytes
    // of the bitmap data (which follows this header) after it has been decompressed.

// bitmapDataStream (variable): A variable-length array of bytes describing a
//  bitmap image. Bitmap data is either compressed or uncompressed, depending
//  on whether the BITMAP_COMPRESSION flag is present in the Flags field.
//  Uncompressed bitmap data is formatted as a bottom-up, left-to-right
//  series of pixels. Each pixel is a whole number of bytes. Each row
//  contains a multiple of four bytes (including up to three bytes of
//  padding, as necessary). Compressed bitmaps not in 32 bpp format are
//  compressed using Interleaved RLE and encapsulated in an RLE Compressed
//  Bitmap Stream structure (section 2.2.9.1.1.3.1.2.4), while compressed
//  bitmaps at a color depth of 32 bpp are compressed using RDP 6.0 Bitmap
//  Compression and stored inside an RDP 6.0 Bitmap Compressed Stream
//  structure ([MS-RDPEGDI] section 2.2.2.5.1).

struct RDPBitmapData {
    // Bitmap Data (TS_BITMAP_DATA)
    uint16_t dest_left{0};
    uint16_t dest_top{0};
    uint16_t dest_right{0};
    uint16_t dest_bottom{0};
    uint16_t width{0};
    uint16_t height{0};
    uint16_t bits_per_pixel{0};
    uint16_t flags{0};

    uint16_t bitmap_length{0};

    // Compressed Data Header (TS_CD_HEADER)
    uint16_t cb_comp_main_body_size{0};
    uint16_t cb_scan_width{0};
    uint16_t cb_uncompressed_size{0};

    RDPBitmapData() = default;

    void emit(OutStream & stream) const {
        unsigned expected;

        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            expected = 26; /* destLeft(2) + destTop(2) + destRight(2) +
                              destBottom(2) + width(2) + height(2) +
                              bitsPerPixel(2) + flags(2) + bitmapLength(2) +
                              cbCompFirstRowSize(2) + cbCompMainBodySize(2) +
                              cbScanWidth(2) + cbUncompressedSize(2) */
        }
        else {
            expected = 18; /* destLeft(2) + destTop(2) + destRight(2) +
                              destBottom(2) + width(2) + height(2) +
                              bitsPerPixel(2) + flags(2) + bitmapLength(2) */
        }

        if (!stream.has_room(expected)) {
            LOG( LOG_ERR
               , "BitmapData::emit - stream too small, need=%u, remains=%zu"
               , expected, stream.tailroom());
            throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
        }

        stream.out_uint16_le(this->dest_left);
        stream.out_uint16_le(this->dest_top);
        stream.out_uint16_le(this->dest_right);
        stream.out_uint16_le(this->dest_bottom);
        stream.out_uint16_le(this->width);
        stream.out_uint16_le(this->height);
        stream.out_uint16_le(this->bits_per_pixel);
        stream.out_uint16_le(this->flags);
        stream.out_uint16_le(this->bitmap_length);

        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            stream.out_uint16_le(0x0000);   /* cbCompFirstRowSize (2 bytes) */
            stream.out_uint16_le(this->cb_comp_main_body_size);
            stream.out_uint16_le(this->cb_scan_width);
            stream.out_uint16_le(this->cb_uncompressed_size);
        }
    }

    void receive(InStream & stream) {
        unsigned expected = 18; /* destLeft(2) + destTop(2) + destRight(2) +
                                   destBottom(2) + width(2) + height(2) +
                                   bitsPerPixel(2) + flags(2) + bitmapLength(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_ERR
               , "BitmapData::receive TS_BITMAP_DATA - Truncated data, need=%u, remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->dest_left      = stream.in_uint16_le();
        this->dest_top       = stream.in_uint16_le();
        this->dest_right     = stream.in_uint16_le();
        this->dest_bottom    = stream.in_uint16_le();
        this->width          = stream.in_uint16_le();
        this->height         = stream.in_uint16_le();
        this->bits_per_pixel = stream.in_uint16_le();
        this->flags          = stream.in_uint16_le();
        this->bitmap_length  = stream.in_uint16_le();

        assert(   (this->bits_per_pixel == 32)
                  || (this->bits_per_pixel == 24)
                  || (this->bits_per_pixel == 16)
                  || (this->bits_per_pixel == 15)
                  || (this->bits_per_pixel == 8 ));

        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            expected = 8; /* cbCompFirstRowSize(2) + cbCompMainBodySize(2) +
                             cbScanWidth(2) + cbUncompressedSize(2) */
            if (!stream.in_check_rem(expected)) {
                LOG( LOG_ERR
                   , "BitmapData::receive TS_CD_HEADER - Truncated data, need=18, remains=%zu"
                   , stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            stream.in_skip_bytes(2);    /* cbCompFirstRowSize (2 bytes) */

            this->cb_comp_main_body_size = stream.in_uint16_le();
            this->cb_scan_width          = stream.in_uint16_le();
            this->cb_uncompressed_size   = stream.in_uint16_le();
        }
    }

    uint16_t struct_size() const {
        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            return 26;
        }

        return 18;
    }

    uint16_t bitmap_size() const {
        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            return this->cb_comp_main_body_size;
        }

        return this->bitmap_length;
    }

    void log(int level) const {
        if (    (this->flags & BITMAP_COMPRESSION)
            && !(this->flags & NO_BITMAP_COMPRESSION_HDR)) {
            LOG( level
               , "BitmapUpdate(destLeft=%u, destTop=%u, destRight=%u, destBottom=%u, "
                 "width=%u, height=%u, bitsPerPixel=%u, flags=0x%04X, bitmapLength=%u, "
                 "cbCompMainBodySize=%u, cbScanWidth=%u, cbUncompressedSize=%u)"
               , this->dest_left
               , this->dest_top
               , this->dest_right
               , this->dest_bottom
               , this->width
               , this->height
               , this->bits_per_pixel
               , this->flags
               , this->bitmap_length
               , this->cb_comp_main_body_size
               , this->cb_scan_width
               , this->cb_uncompressed_size
               );
        }
        else {
            LOG( level
               , "BitmapUpdate(destLeft=%u, destTop=%u, destRight=%u, destBottom=%u, "
                 "width=%u, height=%u, bitsPerPixel=%u, flags=0x%04X, bitmapLength=%u)"
               , this->dest_left
               , this->dest_top
               , this->dest_right
               , this->dest_bottom
               , this->width
               , this->height
               , this->bits_per_pixel
               , this->flags
               , this->bitmap_length
               );
        }
    }

    void move(int offset_x, int offset_y) {
        this->dest_left   += offset_x;
        this->dest_top    += offset_y;
        this->dest_right  += offset_x;
        this->dest_bottom += offset_y;
    }
};  // struct RDPBitmapData

// 2.2.9.1.1.3.1.2.3 Compressed Data Header (TS_CD_HEADER)
// -------------------------------------------------------
// The TS_CD_HEADER structure is used to describe compressed bitmap data.
//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        cbCompFirstRowSize       |      cbCompMainBodySize     |
// +---------------------------------+-----------------------------+
// |           cbScanWidth           |      cbUncompressedSize     |
// +---------------------------------+-----------------------------+

// cbCompFirstRowSize (2 bytes): A 16-bit, unsigned integer. The field MUST
//  be set to 0x0000.

// cbCompMainBodySize (2 bytes): A 16-bit, unsigned integer. The size in
//  bytes of the compressed bitmap data (which follows this header).

// cbScanWidth (2 bytes): A 16-bit, unsigned integer. The width of the bitmap
//  (which follows this header) in pixels (this value MUST be divisible by
//  4).

// cbUncompressedSize (2 bytes): A 16-bit, unsigned integer. The size in
//  bytes of the bitmap data (which follows this header) after it has been
//  decompressed.

// 2.2.9.1.1.3.1.2.4 RLE Compressed Bitmap Stream (RLE_BITMAP_STREAM)
// ------------------------------------------------------------------
// The RLE_BITMAP_STREAM structure contains a stream of bitmap data
//  compressed using Interleaved Run-Length Encoding (RLE). Bitmap data
//  compressed by the server MUST follow a Compressed Data Header (section
//  2.2.9.1.1.3.1.2.3) structure unless the exclusion of this header has been
//  specified in the General Capability Set (section 2.2.7.1.1).

// A compressed bitmap is sent as a series of compression orders that
//  instruct the decoder how to reassemble a compressed bitmap (a particular
//  bitmap can have many valid compressed representations). A compression
//  order consists of an order header, followed by an optional encoded run
//  length, followed by optional data associated with the compression order.
//  Some orders require the decoder to refer to the previous scanline of
//  bitmap data and because of this fact the first scanline sometimes
//  requires special cases for decoding.

// Standard Compression Orders begin with a one-byte order header. The high
//  order bits of this header contain a code identifier, while the low order
//  bits store the unsigned length of the associated run (unless otherwise
//  specified). There are two forms of Standard Compression Orders:
//
//  * The regular form contains a 3-bit code identifier and a 5-bit run
//     length.
//
//  * The lite form contains a 4-bit code identifier and a 4-bit run length.

// For both the regular and lite forms a run length of zero indicates an
//  extended run (a MEGA run), where the byte following the order header
//  contains the encoded length of the associated run. The encoded run length
//  is calculated using the following formula (unless otherwise specified):
//
//  EncodedMegaRunLength = RunLength - (MaximumNonMegaRunLength + 1)

// The maximum run length that can be stored in a non-MEGA regular order is
//  31, while a non-MEGA lite order can only store a maximum run length of
//  15.

// Extended Compression Orders begin with a one-byte order header which
//  contains an 8-bit code identifier. There are two types of Extended
//  Compression Orders:

//  * The MEGA_MEGA type stores the unsigned length of the associated run in
//     the two bytes following the order header (in little-endian order).
//
//  * The single-byte type is used to encode short, commonly occurring
//     foreground/background sequences and single black or white pixels.

// Pseudo-code describing how to decompress a compressed bitmap stream can be
//  found in section 3.1.9.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |             rleCompressedBitmapStream (variable)              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// rleCompressedBitmapStream (variable): An array of compression codes
//  describing compressed structures in the bitmap.
//
//  Background Run Orders
//
//  A Background Run Order encodes a run of pixels where each pixel in the
//   run matches the uncompressed pixel on the previous scanline. If there is
//   no previous scanline then each pixel in the run MUST be black.
//
//  When encountering back-to-back background runs, the decompressor MUST
//   write a one-pixel foreground run to the destination buffer before
//   processing the second background run if both runs occur on the first
//   scanline or after the first scanline (if the first run is on the first
//   scanline, and the second run is on the second scanline, then a one-pixel
//   foreground run MUST NOT be written to the destination buffer). This
//   one-pixel foreground run is counted in the length of the run.

//  The run length encodes the number of pixels in the run. There is no data
//   associated with Background Run Orders.

// +------------------+------------------------------------------------------- +
// | Code Identifier  | Meaning                                                |
// +------------------+------------------------------------------------------- +
// | REGULAR_BG_RUN   | The compression order encodes a regular-form           |
// | 0x0              | background run. The run length is stored in the five   |
// |                  | low-order bits of the order header byte. If this       |
// |                  | value is zero, then the run length is encoded in the   |
// |                  | byte following the order header and MUST be            |
// |                  | incremented by 32 to give the final value.             |
// +------------------+------------------------------------------------------- +
// | MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA background   |
// | 0xF0             | run. The run length is stored in the two bytes         |
// |                  | following the order header (in little-endian format).  |
// +------------------+------------------------------------------------------- +

//  Foreground Run Orders
//
//  A Foreground Run Order encodes a run of pixels where each pixel in the
//   run matches the uncompressed pixel on the previous scanline XOR'ed with
//   the current foreground color. The initial foreground color MUST be
//   white. If there is no previous scanline, then each pixel in the run MUST
//   be set to the current foreground color.
//
//  The run length encodes the number of pixels in the run.
//
//  If the order is a "set" variant, then in addition to encoding a run of
//   pixels, the order also encodes a new foreground color (in little-endian
//   format) in the bytes following the optional run length. The current
//   foreground color MUST be updated with the new value before writing the
//   run to the destination buffer.

// +----------------------+----------------------------------------------------+
// | Code Identifier      | Meaning                                            |
// +----------------------+----------------------------------------------------+
// | REGULAR_FG_RUN       | The compression order encodes a regular-form       |
// | 0x1                  | foreground run. The run length is stored in the    |
// |                      | five low-order bits of the order header byte. If   |
// |                      | this value is zero, then the run length is encoded |
// |                      | in the byte following the order header and MUST be |
// |                      | incremented by 32 to give the final value.         |
// +----------------------+----------------------------------------------------+
// | MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA          |
// | 0xF1                 | foreground run. The run length is stored in the    |
// |                      | two bytes following the order header (in           |
// |                      | little-endian format).                             |
// +----------------------+----------------------------------------------------+
// | LITE_SET_FG_FG_RUN   | The compression order encodes a "set" variant      |
// | 0xC                  | lite-form foreground run. The run length is stored |
// |                      | in the four low-order bits of the order header     |
// |                      | byte. If this value is zero, then the run length   |
// |                      | is encoded in the byte following the order header  |
// |                      | and MUST be incremented by 16 to give the final    |
// |                      | value.                                             |
// +----------------------+----------------------------------------------------+
// | MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set" variant      |
// | 0xF6                 | MEGA_MEGA foreground run. The run length is stored |
// |                      | in the two bytes following the order header (in    |
// |                      | little-endian format).                             |
// +----------------------+----------------------------------------------------+

//  Dithered Run Orders
//
//  A Dithered Run Order encodes a run of pixels which is composed of two
//   alternating colors. The two colors are encoded (in little-endian format)
//   in the bytes following the optional run length.

//  The run length encodes the number of pixel-pairs in the run (not pixels).

// +------------------------+--------------------------------------------------+
// | Code Identifier        | Meaning                                          |
// +------------------------+--------------------------------------------------+
// | LITE_DITHERED_RUN      | The compression order encodes a lite-form        |
// | 0xE                    | dithered run. The run length is stored in the    |
// |                        | four low-order bits of the order header byte. If |
// |                        | this value is zero, then the run length is       |
// |                        | encoded in the byte following the order header   |
// |                        | and MUST be incremented by 16 to give the final  |
// |                        | value.                                           |
// +------------------------+--------------------------------------------------+
// | MEGA_MEGA_DITHERED_RUN | The compression order encodes a MEGA_MEGA        |
// | 0xF8                   | dithered run. The run length is stored in the    |
// |                        | two bytes following the order header (in         |
// |                        | little-endian format).                           |
// +------------------------+--------------------------------------------------+

//  Color Run Orders
//
//  A Color Run Order encodes a run of pixels where each pixel is the same
//   color. The color is encoded (in little-endian format) in the bytes
//   following the optional run length.
//
//  The run length encodes the number of pixels in the run.

// +---------------------+-----------------------------------------------------+
// | Code Identifier     | Meaning                                             |
// +---------------------+-----------------------------------------------------+
// | REGULAR_COLOR_RUN   | The compression order encodes a regular-form color  |
// | 0x3                 | run. The run length is stored in the five low-order |
// |                     | bits of the order header byte. If this value is     |
// |                     | zero, then the run length is encoded in the byte    |
// |                     | following the order header and MUST be incremented  |
// |                     | by 32 to give the final value.                      |
// +---------------------+-----------------------------------------------------+
// | MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA color     |
// | 0xF3                | run. The run length is stored in the two bytes      |
// |                     | following the order header (in little-endian        |
// |                     | format).                                            |
// +---------------------+-----------------------------------------------------+

//  Foreground / Background Image Orders
//
//  A Foreground/Background Image Order encodes a binary image where each
//   pixel in the image that is not on the first scanline fulfills exactly
//   one of the following two properties:
//
//   (a) The pixel matches the uncompressed pixel on the previous scanline
//    XOR'ed with the current foreground color.
//
//   (b) The pixel matches the uncompressed pixel on the previous scanline.
//
//   If the pixel is on the first scanline then it fulfills exactly one of
//    the following two properties:
//
//   (c) The pixel is the current foreground color.
//
//   (d) The pixel is black.
//
//  The binary image is encoded as a sequence of byte-sized bitmasks which
//   follow the optional run length (the last bitmask in the sequence can be
//   smaller than one byte in size). If the order is a "set" variant then the
//   bitmasks MUST follow the bytes which specify the new foreground color.
//   Each bit in the encoded bitmask sequence represents one pixel in the
//   image. A bit that has a value of 1 represents a pixel that fulfills
//   either property (a) or (c), while a bit that has a value of 0 represents
//   a pixel that fulfills either property (b) or (d). The individual
//   bitmasks MUST each be processed from the low-order bit to the high-order
//   bit.
//
//  The run length encodes the number of pixels in the run.
//
//  If the order is a "set" variant, then in addition to encoding a binary
//   image, the order also encodes a new foreground color (in little-endian
//   format) in the bytes following the optional run length. The current
//   foreground color MUST be updated with the new value before writing the
//   run to the destination buffer.

// +--------------------------+------------------------------------------------+
// | Code Identifier          | Meaning                                        |
// +--------------------------+------------------------------------------------+
// | REGULAR_FGBG_IMAGE       | The compression order encodes a regular-form   |
// | 0x2                      | foreground/background image. The run length    |
// |                          | is encoded in the five low-order bits of the   |
// |                          | order header byte and MUST be multiplied by 8  |
// |                          | to give the final value. If this value is      |
// |                          | zero, then the run length is encoded in the    |
// |                          | byte following the order header and MUST be    |
// |                          | incremented by 1 to give the final value.      |
// +--------------------------+------------------------------------------------+
// | MEGA_MEGA_FGBG_IMAGE     | The compression order encodes a MEGA_MEGA      |
// | 0xF2                     | foreground/background image. The run length    |
// |                          | is stored in the two bytes following the       |
// |                          | order header (in little-endian format).        |
// +--------------------------+------------------------------------------------+
// | LITE_SET_FG_FGBG_IMAGE   | The compression order encodes a "set" variant  |
// | 0xD                      | lite-form foreground/background image. The     |
// |                          | run length is encoded in the four low-order    |
// |                          | bits of the order header byte and MUST be      |
// |                          | multiplied by 8 to give the final value. If    |
// |                          | this value is zero, then the run length is     |
// |                          | encoded in the byte following the order        |
// |                          | header and MUST be incremented by 1 to give    |
// |                          | the final value.                               |
// +--------------------------+------------------------------------------------+
// | MEGA_MEGA_SET_FGBG_IMAGE | The compression order encodes a "set" variant  |
// | 0xF7                     | MEGA_MEGA foreground/background image. The     |
// |                          | run length is stored in the two bytes          |
// |                          | following the order header (in little-endian   |
// |                          | format).                                       |
// +--------------------------+------------------------------------------------+

//  Color Image Orders
//
//  A Color Image Order encodes a run of uncompressed pixels.
//
//  The run length encodes the number of pixels in the run. So, to compute
//   the actual number of bytes which follow the optional run length, the run
//   length MUST be multiplied by the color depth (in bits-per-pixel) of the
//   bitmap data.

// +-----------------------+---------------------------------------------------+
// | Code Identifier       | Meaning                                           |
// +-----------------------+---------------------------------------------------+
// | REGULAR_COLOR_IMAGE   | The compression order encodes a regular-form      |
// | 0x4                   | color image. The run length is stored in the      |
// |                       | five low-order bits of the order header byte. If  |
// |                       | this value is zero, then the run length is        |
// |                       | encoded in the byte following the order header    |
// |                       | and MUST be incremented by 32 to give the final   |
// |                       | value.                                            |
// +-----------------------+---------------------------------------------------+
// | MEGA_MEGA_COLOR_IMAGE | The compression order encodes a MEGA_MEGA color   |
// | 0xF4                  | image. The run length is stored in the two bytes  |
// |                       | following the order header (in little-endian      |
// |                       | format).                                          |
// +-----------------------+---------------------------------------------------+

//  Special Orders

// +-----------------+---------------------------------------------------------+
// | Code Identifier | Meaning                                                 |
// +-----------------+---------------------------------------------------------+
// | SPECIAL_FGBG_1  | The compression order encodes a foreground/background   |
// | 0xF9            | image with an 8-bit bitmask of 0x03.                    |
// +-----------------+---------------------------------------------------------+
// | SPECIAL_FGBG_2  | The compression order encodes a foreground/background   |
// | 0xFA            | image with an 8-bit bitmask of 0x05.                    |
// +-----------------+---------------------------------------------------------+
// | WHITE           | The compression order encodes a single white pixel.     |
// | 0xFD            |                                                         |
// +-----------------+---------------------------------------------------------+
// | BLACK           | The compression order encodes a single black pixel.     |
// | 0xFE            |                                                         |
// +-----------------+---------------------------------------------------------+

