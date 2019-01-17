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

   New RDP Orders Coder / Decoder

*/


#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/colors.hpp"
#include "utils/bitmap.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "core/RDP/orders/RDPOrdersCommon.hpp"

/* RDP bitmap cache (version 2) constants */
enum {
    BMPCACHE2_C0_CELLS             = 0x78,
    BMPCACHE2_C1_CELLS             = 0x78,
    BMPCACHE2_C2_CELLS             = 0x150,
    BMPCACHE2_NUM_PSTCELLS         = 0x9f6
};

// TODO RDPBmpCache works with a given item (id, idx) inside a given Bitmap cache, hence we should not provide it with the bitmap itself  but with the cache containing the said bitmap. That said we should also have both lowlevel bitmaps (bpp independant) and curryed bitmap with a fixed given bpp. RDPBmpCache relates to a cache containing curryed bitmaps  as the actual id to use depends off the bitmap size in bytes  itself depending from bpp. Alternatively  this bpp could be attached to the cache itself as bpp is unique for a given cache.
class RDPBmpCache {
    // [MS-RDPGDI] 2.2.2.2.1.2.2 Cache Bitmap - Revision 1 (CACHE_BITMAP_ORDER)
    // ========================================================================

    // The Cache Bitmap - Revision 1 Secondary Drawing Order is used by the
    // server to instruct the client to store a bitmap in a particular Bitmap
    // Cache entry. This order only supports memory-based bitmap caching.
    // Support for Revision 1 bitmap caching is negotiated in the Bitmap Cache
    // Capability Set (Revision 1) (see [MS-RDPBCGR] section 2.2.7.1.4.1).

    // header (6 Bytes) : The Secondary Drawing Order Header
    //  (section 2.2.2.2.1.2.1.1).

    //The embedded orderType field MUST be set to one of the following values.
    // header::orderType
    // +-----------------------------------+-----------------------------------+
    // | 0x00 TS_CACHE_BITMAP_UNCOMPRESSED | The bitmap data in the            |
    // |                                   | bitmapDataStream field is         |
    // |                                   | uncompressed.                     |
    // +-----------------------------------------------------------------------+
    // | 0x02 TS_CACHE_BITMAP_COMPRESSED   | The bitmap data in the            |
    // |                                   | bitmapDataStream field is         |
    // |                                   | compressed.                       |
    // +-----------------------------------+-----------------------------------+

    // header::extraFlags
    //  The embedded extraFlags field MAY contain the following flag.
    //  0x00000400 NO_BITMAP_COMPRESSION_HDR Indicates that the bitmapComprHdr
    //  field is not present (removed for bandwidth efficiency to save 8 bytes).

    enum {
        NO_BITMAP_COMPRESSION_HDR = 0x0400
    };


    // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
    //  which to store the bitmap data. The bitmap cache ID MUST be in the range
    //  negotiated by the Bitmap Cache Capability Set (Revision 1)
    //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

    // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in this
    //  field are arbitrary and MUST be ignored.

    // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the bitmap
    //  in pixels.

    // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
    //  bitmap in pixels.

    // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color depth
    //  of the bitmap data in bits-per-pixel. This field MUST be one of the
    //  following values.
    //  0x08 8-bit color depth.
    //  0x10 16-bit color depth.
    //  0x18 24-bit color depth.
    //  0x20 32-bit color depth.

    // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    // the data in the bitmapComprHdr and bitmapDataStream fields.

    // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the bitmap
    //  cache (specified by the cacheId field) where the bitmap MUST be stored.
    //  The bitmap cache index MUST be in the range negotiated by the Bitmap
    //  Cache Capability Set (Revision 1) (see [MS-RDPBCGR] section
    //  2.2.7.1.4.1).

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
    //  [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3) describing the bitmap data in
    //  the bitmapDataStream. This field MUST be present if the
    //  TS_CACHE_BITMAP_COMPRESSED (0x02) flag is present in the header field,
    //  but the NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not.

    // The TS_CD_HEADER structure is used to describe compressed bitmap data.

    // bitmapComprHdr::cbCompFirstRowSize (2 bytes): A 16-bit, unsigned integer. The field
    //  MUST be set to 0x0000.

    // bitmapComprHdr::cbCompMainBodySize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the compressed bitmap data (which follows this header).

    // bitmapComprHdr::cbScanWidth (2 bytes): A 16-bit, unsigned integer. The width of the
    //  bitmap (which follows this header) in pixels (this value MUST be
    //  divisible by 4).

    // bitmapComprHdr::cbUncompressedSize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the bitmap data (which follows this header) after it has been
    //  decompressed.

    // bitmapDataStream (variable): A variable-length byte array containing
    //  bitmap data (the format of this data is defined in [MS-RDPBCGR] section
    //  2.2.9.1.1.3.1.2.2).

    // Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).

    // Compressed bitmaps not in 32 bpp format are compressed using Interleaved
    // RLE and encapsulated in an RLE Compresssed Bitmap Stream structure (see
    // section 2.2.9.1.1.3.1.2.4) while compressed bitmap data at a color depth
    // of 32 bpp is compressed using RDP 6.0 Bitmap Compression and stored
    // inside an RDP 6.0 Bitmap Compressed Stream structure (see section
    // 2.2.2.5.1 in [MS-RDPEGDI]).

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.4 RLE Compressed Bitmap Stream
    // ----------------------------------------------------------

    // (RLE_BITMAP_STREAM)
    // The RLE_BITMAP_STREAM structure contains a stream of bitmap data
    // compressed using Interleaved Run-Length Encoding (RLE). Compressed
    // bitmap data MUST follow a Compressed Data Header (section
    // 2.2.9.1.1.3.1.2.3) structure unless exclusion of this header has been
    // negotiated in the General Capability Set (section 2.2.7.1.1).

    // A compressed bitmap is sent as a series of compression orders that
    // instruct the decoder how to reassemble a compressed bitmap (a particular
    // bitmap can have many valid compressed representations). A compression
    // order consists of a one-byte order header, followed by an optional
    // encoded run length, followed by optional data associated with the
    // compression order. Some orders require the decoder to refer to the
    // previous scanline of bitmap data and because of this fact the first
    // scanline sometimes requires special cases for decoding.

    // Standard Compression Orders begin with a one byte order header. The high
    // order bits of this header contain a code identifier, while the low order
    // bits store the length of the associated run (unless otherwise
    // specified).

    // There are two forms of Standard Compression Orders:
    // - The regular form contains a 3-bit code identifier and a 5-bit run
    //  length.
    // - The lite form contains a 4-bit code identifier and a 4-bit run
    //  length.

    // For both the regular and lite forms a run length of zero indicates an
    // extended run (a MEGA run), where the byte following the order header
    // contains the encoded length of the associated run. The encoded run
    // length is calculated using the following formula (unless otherwise
    // specified):

    // EncodedMegaRunLength = RunLength - (MaximumNonMegaRunLength + 1)

    // The maximum run length that can be stored in a non-MEGA regular order is
    // 31, while a non-MEGA lite order can only store a maximum run length of
    // 15.

    // Extended Compression Orders begin with a one byte order header which
    // contains an 8-bit code identifier. There are two types of Extended
    // Compression Orders:

    // The MEGA_MEGA type stores the length of the associated run in the two
    // bytes following the order header (in little-endian order). In the
    // MEGA_MEGA form the stored run length is the plain unsigned 16-bit length
    // of the run.

    // The single-byte type is used to encode short, commonly occurring
    // foreground/background sequences and single black or white pixels.

    // Pseudo-code describing how to decompress a compressed bitmap stream can
    // be found in section 3.1.9.

    // rleCompressedBitmapStream (variable): An array of compression codes
    //  describing compressed structures in the bitmap.

    // Background Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Background Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline. If there is
    // no previous scanline then each pixel in the run MUST be black.

    // When encountering back-to-back background runs, the decompressor MUST
    // write a one-pixel foreground run to the destination buffer before
    // processing the second background run if both runs occur on the first
    // scanline or after the first scanline (if the first run is on the first
    // scanline, and the second run is on the second scanline, then a one-pixel
    // foreground run MUST NOT be written to the destination buffer). This
    // one-pixel foreground run is counted in the length of the run.

    // The run length encodes the number of pixels in the run. There is no data
    // associated with Background Run Orders.

    // +-----------------------+-----------------------------------------------+
    // | 0x0 REGULAR_BG_RUN    | The compression order encodes a regular-form  |
    // |                       | background run. The run length is stored in   |
    // |                       | the five low-order bits of  the order header  |
    // |                       | byte. If this value is zero, then the run     |
    // |                       | length is encoded in the byte following the   |
    // |                       | order header and MUST be incremented by 32 to |
    // |                       | give the final value.                         |
    // +-----------------------+-----------------------------------------------+
    // | 0xF0 MEGA_MEGA_BG_RUN | The compression order encodes a MEGA_MEGA     |
    // |                       | background run. The run length is stored in   |
    // |                       | the two bytes following the order header      |
    // |                       | (in little-endian format).                    |
    // +-----------------------+-----------------------------------------------+

    // Foreground Run Orders
    // ~~~~~~~~~~~~~~~~~~~~~

    // A Foreground Run Order encodes a run of pixels where each pixel in the
    // run matches the uncompressed pixel on the previous scanline XOR’ed with
    // the current foreground color. If there is no previous scanline, then
    // each pixel in the run MUST be set to the current foreground color (the
    // initial foreground color is white).

    // The run length encodes the number of pixels in the run.
    // If the order is a "set" variant, then in addition to encoding a run of
    // pixels, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +---------------------------+-------------------------------------------+
    // | 0x1 REGULAR_FG_RUN        | The compression order encodes a           |
    // |                           | regular-form foreground run. The run      |
    // |                           | length is stored in the five low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 32 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF1 MEGA_MEGA_FG_RUN     | The compression order encodes a MEGA_MEGA |
    // |                           | foreground run. The run length is stored  |
    // |                           | in the two bytes following the order      |
    // |                           | header (in little-endian format).         |
    // +---------------------------+-------------------------------------------+
    // | 0xC LITE_SET_FG_FG_RUN    | The compression order encodes a "set"     |
    // |                           | variant lite-form foreground run. The run |
    // |                           | length is stored in the four low-order    |
    // |                           | bits of the order header byte. If this    |
    // |                           | value is zero, then the run length is     |
    // |                           | encoded in the byte following the order   |
    // |                           | header and MUST be incremented by 16 to   |
    // |                           | give the final value.                     |
    // +---------------------------+-------------------------------------------+
    // | 0xF6 MEGA_MEGA_SET_FG_RUN | The compression order encodes a "set"     |
    // |                           | variant MEGA_MEGA foreground run. The run |
    // |                           | length is stored in the two bytes         |
    // |                           | following the order header (in            |
    // |                           | little-endian format).                    |
    // +---------------------------+-------------------------------------------+

    // Dithered Run Orders
    // ~~~~~~~~~~~~~~~~~~~

    // A Dithered Run Order encodes a run of pixels which is composed of two
    // alternating colors. The two colors are encoded (in little-endian format)
    // in the bytes following the optional run length.

    // The run length encodes the number of pixel-pairs in the run (not pixels).

    // +-----------------------------+-----------------------------------------+
    // | 0xE LITE_DITHERED_RUN       | The compression order encodes a         |
    // |                             | lite-form dithered run. The run length  |
    // |                             | is stored in the four low-order bits of |
    // |                             | the order header byte. If this value is |
    // |                             | zero, then the run length is encoded in |
    // |                             | the byte following the order header and |
    // |                             | MUST be incremented by 16 to give the   |
    // |                             | final value.                            |
    // +-----------------------------+-----------------------------------------+
    // | 0xF8 MEGA_MEGA_DITHERED_RUN | The compression order encodes a         |
    // |                             | MEGA_MEGA dithered run. The run length  |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    // Color Run Orders
    // ~~~~~~~~~~~~~~~~

    // A Color Run Order encodes a run of pixels where each pixel is the same
    // color. The color is encoded (in little-endian format) in the bytes
    // following the optional run length.

    // The run length encodes the number of pixels in the run.

    // +--------------------------+--------------------------------------------+
    // | 0x3 REGULAR_COLOR_RUN    | The compression order encodes a            |
    // |                          | regular-form color run. The run length is  |
    // |                          | stored in the five low-order bits of the   |
    // |                          | order header byte. If this value is zero,  |
    // |                          | then the run length is encoded in the byte |
    // |                          | following the order header and MUST be     |
    // |                          | incremented by 32 to give the final value. |
    // +--------------------------+--------------------------------------------+
    // | 0xF3 MEGA_MEGA_COLOR_RUN | The compression order encodes a MEGA_MEGA  |
    // |                          | color run. The run length is stored in the |
    // |                          | two bytes following the order header (in   |
    // |                          | little-endian format).                     |
    // +--------------------------+--------------------------------------------+

    // Foreground / Background Image Orders
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // A Foreground/Background Image Order encodes a binary image where each
    // pixel in the image that is not on the first scanline fulfils exactly one
    // of the following two properties:

    // (a) The pixel matches the uncompressed pixel on the previous scanline
    // XOR'ed with the current foreground color.

    // (b) The pixel matches the uncompressed pixel on the previous scanline.

    // If the pixel is on the first scanline then it fulfils exactly one of the
    // following two properties:

    // (c) The pixel is the current foreground color.

    // (d) The pixel is black.

    // The binary image is encoded as a sequence of byte-sized bitmasks which
    // follow the optional run length (the last bitmask in the sequence can be
    // smaller than one byte in size). If the order is a "set" variant then the
    // bitmasks MUST follow the bytes which specify the new foreground color.
    // Each bit in the encoded bitmask sequence represents one pixel in the
    // image. A bit that has a value of 1 represents a pixel that fulfils
    // either property (a) or (c), while a bit that has a value of 0 represents
    // a pixel that fulfils either property (b) or (d). The individual bitmasks
    // MUST each be processed from the low-order bit to the high-order bit.

    // The run length encodes the number of pixels in the run.

    // If the order is a "set" variant, then in addition to encoding a binary
    // image, the order also encodes a new foreground color (in little-endian
    // format) in the bytes following the optional run length. The current
    // foreground color MUST be updated with the new value before writing
    // the run to the destination buffer.

    // +--------------------------------+--------------------------------------+
    // | 0x2 REGULAR_FGBG_IMAGE         | The compression order encodes a      |
    // |                                | regular-form foreground/background   |
    // |                                | image. The run length is encoded in  |
    // |                                | the five low-order bits of the order |
    // |                                | header byte and MUST be multiplied   |
    // |                                | by 8 to give the final value. If     |
    // |                                | this value is zero, then the run     |
    // |                                | length is encoded in the byte        |
    // |                                | following the order header and MUST  |
    // |                                | be incremented by 1 to give the      |
    // |                                | final value.                         |
    // +--------------------------------+--------------------------------------+
    // | 0xF2 MEGA_MEGA_FGBG_IMAGE      | The compression order encodes a      |
    // |                                | MEGA_MEGA foreground/background      |
    // |                                | image. The run length is stored in   |
    // |                                | the two bytes following the order    |
    // |                                | header (in little-endian format).    |
    // +--------------------------------+--------------------------------------+
    // | 0xD LITE_SET_FG_FGBG_IMAGE     | The compression order encodes a      |
    // |                                | "set" variant lite-form              |
    // |                                | foreground/background image. The run |
    // |                                | length is encoded in the four        |
    // |                                | low-order bits of the order header   |
    // |                                | byte and MUST be multiplied by 8 to  |
    // |                                | give the final value. If this value  |
    // |                                | is zero, then the run length is      |
    // |                                | encoded in the byte following the    |
    // |                                | order header and MUST be incremented |
    // |                                | by 1 to give the final value.        |
    // +--------------------------------+--------------------------------------+
    // | 0xF7 MEGA_MEGA_SET_FGBG_IMAGE  | The compression order encodes a      |
    // |                                | "set" variant MEGA_MEGA              |
    // |                                | foreground/background image. The run |
    // |                                | length is stored in the two bytes    |
    // |                                | following the order header (in       |
    // |                                | little-endian format).               |
    // +-----------------------------------------------------------------------+

    // Color Image Orders
    // ~~~~~~~~~~~~~~~~~~

    // A Color Image Order encodes a run of uncompressed pixels.

    // The run length encodes the number of pixels in the run. So, to compute
    // the actual number of bytes which follow the optional run length, the run
    // length MUST be multiplied by the color depth (in bits-per-pixel) of the
    // bitmap data.

    // +-----------------------------+-----------------------------------------+
    // | 0x4 REGULAR_COLOR_IMAGE     | The compression order encodes a         |
    // |                             | regular-form color image. The run       |
    // |                             | length is stored in the five low-order  |
    // |                             | bits of the order header byte. If this  |
    // |                             | value is zero, then the run length is   |
    // |                             | encoded in the byte following the order |
    // |                             | header and MUST be incremented by 32 to |
    // |                             | give the final value.                   |
    // +-----------------------------+-----------------------------------------+
    // | 0xF4 MEGA_MEGA_COLOR_IMAGE  | The compression order encodes a         |
    // |                             | MEGA_MEGA color image. The run length   |
    // |                             | is stored in the two bytes following    |
    // |                             | the order header (in little-endian      |
    // |                             | format).                                |
    // +-----------------------------+-----------------------------------------+

    // Special Orders
    // ~~~~~~~~~~~~~~

    // +----------------------+------------------------------------------------+
    // | 0xF9 SPECIAL_FGBG_1  | The compression order encodes a                |
    // |                      | foreground/background image with an 8-bit      |
    // |                      | bitmask of 0x03.                               |
    // +----------------------+------------------------------------------------+
    // | 0xFA SPECIAL_FGBG_2  | The compression order encodes a                |
    // |                      | foreground/background image with an 8-bit      |
    // |                      | bitmask of 0x05.                               |
    // +----------------------+------------------------------------------------+
    // | 0xFD WHITE           | The compression order encodes a single white   |
    // |                      | pixel.                                         |
    // +----------------------+------------------------------------------------+
    // | 0xFE BLACK           | The compression order encodes a single black   |
    // |                      | pixel.                                         |
    // +----------------------+------------------------------------------------+

    public:
    int      id{0};
    int      idx{0};
    Bitmap   bmp;
    bool     persistent{false};
    bool     do_not_cache{false};
    uint32_t key1{0};
    uint32_t key2{0};
    bool verbose;

    RDPBmpCache(Bitmap bmp, int id, int idx, bool persistent, bool do_not_cache, bool verbose = false)
        : id(id), idx(idx), bmp(std::move(bmp)), persistent(persistent)
        , do_not_cache(do_not_cache), key1(0), key2(0), verbose(verbose)
    {
    }

    explicit RDPBmpCache(bool verbose = false)
        :  verbose(verbose) {
    }

    void emit(BitsPerPixel session_color_depth, OutStream & stream, const int bitmap_cache_version,
        bool use_bitmap_comp, bool use_compact_packets) const
    {
        using namespace RDP;
        switch (bitmap_cache_version){
        case 0:
        case 1:
            if (use_bitmap_comp){
                if (this->verbose){
                    LOG(LOG_INFO, "/* BMP Cache compressed V1*/");
                }
                this->emit_v1_compressed(session_color_depth, stream, use_compact_packets);
            }
            else {
                if (this->verbose){
                    LOG(LOG_INFO, "/* BMP Cache raw V1 */");
                }
                this->emit_raw_v1(stream);
            }
        break;
        default:
            if (use_bitmap_comp){
                if (this->verbose){
                    LOG(LOG_INFO, "/* BMP Cache compressed V2 */");
                }
                this->emit_v2_compressed(session_color_depth, stream);
            }
            else {
                if (this->verbose){
                    LOG(LOG_INFO, "/* BMP Cache raw V2 */");
                }
                this->emit_raw_v2(stream);
            }
        }
    }

    void emit_v1_compressed(BitsPerPixel session_color_depth, OutStream & stream, bool use_compact_packets) const {
        using namespace RDP;

        int order_flags = STANDARD | SECONDARY;
        stream.out_uint8(order_flags);
        /* length after type minus 7 */
        uint32_t offset_header = stream.get_offset();
        stream.out_uint16_le(0); // placeholder for size after type - 7

         // flags : why do we put 8 ? Any value should be ok except NO_BITMAP_COMPRESSION_HDR
        stream.out_uint16_le(use_compact_packets?NO_BITMAP_COMPRESSION_HDR:8); // flags
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED); // type

        uint32_t offset_after_type = stream.get_offset();
        stream.out_uint8(this->id);
        stream.out_clear_bytes(1); /* pad */

        stream.out_uint8(this->bmp.cx());
        stream.out_uint8(this->bmp.cy());
        stream.out_uint8(safe_int(this->bmp.bpp()));

        uint32_t offset = stream.get_offset();
        stream.out_uint16_le(0); // placeholder for bufsize
        stream.out_uint16_le(this->idx);

        uint32_t offset_compression_header = stream.get_offset();
        if (!use_compact_packets){
            if (this->verbose){
                LOG(LOG_INFO, "/* Use compression headers */");
            }
            stream.out_clear_bytes(2); /* pad */
            stream.out_uint16_le(0); // placeholder for bufsize
            stream.out_uint16_le(this->bmp.bmp_size() / this->bmp.cy());
            stream.out_uint16_le(this->bmp.bmp_size()); // final size
        }

        uint32_t offset_buf_start = stream.get_offset();
        this->bmp.compress(session_color_depth, stream);
        uint32_t bufsize = stream.get_offset() - offset_buf_start;

        if (!use_compact_packets){
            stream.set_out_uint16_le(bufsize, offset_compression_header + 2);
        }

        stream.set_out_uint16_le(stream.get_offset() - offset_compression_header, offset);
        stream.set_out_uint16_le(stream.get_offset() - offset_after_type - 7, offset_header);
    }

    void emit_raw_v1(OutStream & stream) const
    {
        using namespace RDP;

        // TODO this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
        stream.out_uint16_le(9 + this->bmp.bmp_size()  - 7); // length after orderType - 7
        stream.out_uint16_le(8);        // extraFlags
        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED); // type

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).
        stream.out_uint8(this->id);

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.
        stream.out_clear_bytes(1);

        // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the
        // bitmap in pixels.
        assert(this->bmp.cx() == align4(this->bmp.cx()));
        stream.out_uint8(this->bmp.cx());

        // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
        //  bitmap in pixels.
        stream.out_uint8(this->bmp.cy());

        // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color
        //  depth of the bitmap data in bits-per-pixel. This field MUST be one
        //  of the following values.
        //  0x08 8-bit color depth.
        //  0x10 16-bit color depth.
        //  0x18 24-bit color depth.
        //  0x20 32-bit color depth.
        stream.out_uint8(safe_int(this->bmp.bpp()));

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.
        stream.out_uint16_le(this->bmp.bmp_size());

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).
        stream.out_uint16_le(this->idx);

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        // Note: we ensure bitmap with is multiple of 4, thus there won't be any padding ever
        stream.out_copy_bytes(this->bmp.data(), this->bmp.bmp_size());
    }

    enum {
        CBR2_8BPP = 0x03,
        CBR2_16BPP = 0x04,
        CBR2_24BPP = 0x05,
        CBR2_32BPP = 0x06
    };

    enum {
        CBR2_HEIGHT_SAME_AS_WIDTH = 0x01,
        CBR2_PERSISTENT_KEY_PRESENT = 0x02,
        CBR2_NO_BITMAP_COMPRESSION_HDR = 0x08,
        CBR2_DO_NOT_CACHE = 0x10
    };

    // MS-RDPEGDI 2.2.2.2.1.2.3
    // ========================
    // Cache Bitmap - Revision 2 (CACHE_BITMAP_REV2_ORDER)
    // ---------------------------------------------------
    // The Cache Bitmap - Revision 2 Secondary Drawing Order is used by the
    // server to instruct the client to store a bitmap in a particular Bitmap
    // Cache entry. This order supports persistent disk bitmap caching and uses
    // a compact encoding format. Support for Revision 2 bitmap caching is
    // negotiated in the Bitmap Cache Capability Set (Revision 2)
    // (see [MS-RDPBCGR] section 2.2.7.1.4.2).

    // header (6 bytes): A Secondary Drawing Order Header
    // (section 2.2.2.2.1.2.1.1). The embedded orderType field MUST be set to
    // one of the following values:

    // header::orderType
    // +----------------------------------------+------------------------------+
    // | 0x04 TS_CACHE_BITMAP_UNCOMPRESSED_REV2 | The bitmap data in the       |
    // |                                        | bitmapDataStream field is    |
    // |                                        | uncompressed.                |
    // +-----------------------------------------------------------------------+
    // | 0x05 TS_CACHE_BITMAP_COMPRESSED_REV2   | The bitmap data in the       |
    // |                                        | bitmapDataStream field is    |
    // |                                        | compressed.                  |
    // +----------------------------------------+------------------------------+

    // The format of the embedded extraFlags field is specified as follow :
    // bits 0 to 2  : cacheId
    // bits 3 to 6  : bitsPerPixelId
    // bits 7 to 15 : flags

    // header::extraFlags::cacheId[0..2] (3 bits)
    // cacheId (3 bits): A 3-bit, unsigned integer. The bitmap cache into which
    // to store the bitmap data. The bitmap cache ID MUST be in the range
    // negotiated by the Bitmap Cache Capability Set (Revision 2)
    // (see [MS-RDPBCGR] section 2.2.7.1.4.2).

    // header::extraFlags::bitsPerPixelId[3..6]
    // bitsPerPixelId (4 bits): A 4-bit, unsigned integer. The color depth of
    // the bitmap data in bits-per-pixel. MUST be one of the following values.

    // +--------------------+-------------------+
    // | 0x03 CBR2_8BPP     | 8 bits per pixel  |
    // +--------------------+-------------------+
    // | 0x04 CBR2_16BPP    | 16 bits per pixel |
    // +--------------------+-------------------+
    // | 0x05 CBR2_24BPP    | 24 bits per pixel |
    // +--------------------+-------------------+
    // | 0x06 CBR2_32BPP    | 32 bits per pixel |
    // +--------------------+-------------------+

    // header::extraFlags::flags[7..15]
    // A 9-bit, unsigned integer. Operational flags.

    // +----------------------------------------+------------------------------+
    // | 0x01 CBR2_HEIGHT_SAME_AS_WIDTH         | Implies that the bitmap      |
    // |                                        | height is the same as the    |
    // |                                        | bitmap width. If this flag is|
    // |                                        | set, the bitmapHeight field  |
    // |                                        | MUST NOT be present.         |
    // +----------------------------------------+------------------------------+
    // +----------------------------------------+------------------------------+
    // | 0x02 CBR2_PERSISTENT_KEY_PRESENT       | Implies that the bitmap is   |
    // |                                        | intended to be persisted,    |
    // |                                        | and the key1 and key2 fields |
    // |                                        | MUST be present.             |
    // +----------------------------------------+------------------------------+
    // | 0x08 CBR2_NO_BITMAP_COMPRESSION_HDR    | Indicates that the           |
    // |                                        | bitmapComprHdr field is not  |
    // |                                        | present (removed for         |
    // |                                        | bandwidth efficiency to save |
    // |                                        | 8 bytes).                    |
    // +----------------------------------------+------------------------------+
    // | 0x10 CBR2_DO_NOT_CACHE                 | Implies that the cacheIndex  |
    // |                                        | field MUST be ignored, and   |
    // |                                        | the bitmap MUST be placed in |
    // |                                        | the last entry of the bitmap |
    // |                                        | cache specified by cacheId   |
    // |                                        | field.                       |
    // +----------------------------------------+------------------------------+

    // key1 (4 bytes): A 32-bit, unsigned integer. The low 32 bits of the 64-bit
    //                 persistent bitmap cache key.

    // key2 (4 bytes): A 32-bit, unsigned integer. The high 32 bits of the
    //                 64-bit persistent bitmap cache key.

    // bitmapWidth (variable): A Two-Byte Unsigned Encoding (section
    //                         2.2.2.2.1.2.1.2) structure. The width of the
    //                         bitmap in pixels.

    // bitmapHeight (variable): A Two-Byte Unsigned Encoding (section
    //                          2.2.2.2.1.2.1.2) structure. The height of the
    //                          bitmap in pixels.

    // bitmapLength (variable): A Four-Byte Unsigned Encoding (section
    //                          2.2.2.2.1.2.1.4) structure. The size in bytes
    //                          of the data in the bitmapComprHdr and
    //                          bitmapDataStream fields.

    // cacheIndex (variable): A Two-Byte Unsigned Encoding (section
    //                        2.2.2.2.1.2.1.2) structure. An entry in the bitmap
    //                        cache (specified by the cacheId field) where the
    //                        bitmap MUST be stored. If the CBR2_DO_NOT_CACHE
    //                        flag is not set in the header field, the bitmap
    //                        cache index MUST be in the range negotiated by the
    //                        Bitmap Cache Capability Set (Revision 2) (see
    //                        [MS-RDPBCGR] section 2.2.7.1.4.2). Otherwise, if
    //                        the CBR2_DO_NOT_CACHE flag is set, the cacheIndex
    //                        MUST be set to BITMAPCACHE_WAITING_LIST_INDEX
    //                        (32767).

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
    //                           [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3)
    //                           describing the bitmap data in the
    //                           bitmapDataStream. This field MUST be present if
    //                           the TS_CACHE_BITMAP_COMPRESSED_REV2 (0x05) flag
    //                           is present in the header field, but the
    //                           CBR2_NO_BITMAP_COMPRESSION_HDR (0x08) flag is
    //                           not.

    // bitmapDataStream (variable): A variable-length byte array containing
    //                              bitmap data (the format of this data is
    //                              defined in [MS-RDPBCGR] section
    //                              2.2.9.1.1.3.1.2.2).

    enum {
          BITMAPCACHE_WAITING_LIST_INDEX = 32767
    };

    void emit_v2_compressed(BitsPerPixel session_color_depth, OutStream & stream) const
    {
        using namespace RDP;

        const int Bpp = nb_bytes_per_pixel(this->bmp.bpp());

        stream.out_uint8(STANDARD | SECONDARY);

        uint32_t offset_header = stream.get_offset();
        stream.out_uint16_le(0); // placeholder for length after type minus 7
        uint16_t cbr2_flags = (((  CBR2_NO_BITMAP_COMPRESSION_HDR
                                 | (this->persistent   ? CBR2_PERSISTENT_KEY_PRESENT : 0)
                                 | (this->do_not_cache ? CBR2_DO_NOT_CACHE           : 0)) << 7) & 0xFF80);
        uint16_t cbr2_bpp = (((Bpp + 2) << 3) & 0x78);
        stream.out_uint16_le(cbr2_flags | cbr2_bpp | (this->id & 7));
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED_REV2); // type


        if (this->persistent) {
            union {
                uint8_t  sig_8[20];
                uint32_t sig_32[2];
            } sig;
            this->bmp.compute_sha1(sig.sig_8);
            uint32_t * Key1 = sig.sig_32;
            uint32_t * Key2 = Key1 + 1;
            stream.out_uint32_le(*Key1);
            stream.out_uint32_le(*Key2);
            //if (this->verbose & 512) {
            //    LOG(LOG_INFO, "id=%u Key1=%08X Key2=%08X", this->id, *Key1, *Key2);
            //    LOG(LOG_INFO, "Persistent key");
            //    hexdump_d(sig.sig_8, 8);
            //}
        }

        stream.out_2BUE(this->bmp.cx());
        stream.out_2BUE(this->bmp.cy());
        uint32_t offset_bitmapLength = stream.get_offset();
        // TODO define out_4BUE in stream and find a way to predict compressed bitmap size (the problem is to write to it afterward). May be we can keep a compressed version of the bitmap instead of recompressing every time. The first time we would use a conservative encoding for length based on cx and cy.
        stream.out_uint16_be(0);
        stream.out_2BUE(this->do_not_cache ? BITMAPCACHE_WAITING_LIST_INDEX : this->idx);
        uint32_t offset_startBitmap = stream.get_offset();
        this->bmp.compress(session_color_depth, stream);

        stream.set_out_uint16_be((stream.get_offset() - offset_startBitmap) | 0x4000, offset_bitmapLength); // set the actual size
        stream.set_out_uint16_le(stream.get_offset() - (offset_header+12), offset_header); // length after type minus 7
    }

    void emit_raw_v2(OutStream & stream) const
    {
        using namespace RDP;
        // TODO this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);

        uint32_t offset_header = stream.get_offset();
        stream.out_uint16_le(0); // placeholder for length after type minus 7

        int bitsPerPixelId = nb_bytes_per_pixel(this->bmp.bpp())+2;

        // TODO some optimisations are possible here if we manage flags  but what will we do with persistant bitmaps ? We definitely do not want to save them on disk from here. There must be some kind of persistant structure where to save them and check if they exist.
        uint16_t flags = ((this->persistent   ? CBR2_PERSISTENT_KEY_PRESENT : 0) |
                          (this->do_not_cache ? CBR2_DO_NOT_CACHE           : 0));

        // header::extraFlags : (flags:9, bitsPerPixelId:3, cacheId:3)
        stream.out_uint16_le((flags << 7)
            |((bitsPerPixelId << 3)& 0x78)
            | (this->id & 7));

        // header::orderType
        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED_REV2);

        if (this->persistent) {
            union {
                uint8_t  sig_8[20];
                uint32_t sig_32[2];
            } sig;
            this->bmp.compute_sha1(sig.sig_8);
            uint32_t * Key1 = sig.sig_32;
            uint32_t * Key2 = Key1 + 1;
            stream.out_uint32_le(*Key1);
            stream.out_uint32_le(*Key2);
            //if (this->verbose & 512) {
            //    LOG(LOG_INFO, "Key1=%08X Key2=%08X", *Key1, *Key2);
            //    LOG(LOG_INFO, "Persistent key");
            //    hexdump_d(sig.sig_8, 8);
            //}
        }

        // key1 and key1 are not here because flags is not set
        // to CBR2_PERSISTENT_KEY_PRESENT
        // ---------------------------------------------------
        // key1 (4 bytes): A 32-bit, unsigned integer. The low 32 bits of the 64-bit
        //                 persistent bitmap cache key.

        // key2 (4 bytes): A 32-bit, unsigned integer. The high 32 bits of the
        //                 64-bit persistent bitmap cache key.

        // bitmapWidth (variable): A Two-Byte Unsigned Encoding (section
        //                         2.2.2.2.1.2.1.2) structure. The width of the
        //                         bitmap in pixels.
        stream.out_2BUE(this->bmp.cx());

        // bitmapHeight (variable): A Two-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.2) structure. The height of the
        //                          bitmap in pixels.
        stream.out_2BUE(this->bmp.cy());

        // bitmapLength (variable): A Four-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.4) structure. The size in bytes
        //                          of the data in the bitmapComprHdr and
        //                          bitmapDataStream fields.
        stream.out_uint16_be(this->bmp.bmp_size() | 0x4000);

        // cacheIndex (variable): A Two-Byte Unsigned Encoding (section
        //                        2.2.2.2.1.2.1.2) structure. An entry in the bitmap
        //                        cache (specified by the cacheId field) where the
        //                        bitmap MUST be stored. If the CBR2_DO_NOT_CACHE
        //                        flag is not set in the header field, the bitmap
        //                        cache index MUST be in the range negotiated by the
        //                        Bitmap Cache Capability Set (Revision 2) (see
        //                        [MS-RDPBCGR] section 2.2.7.1.4.2). Otherwise, if
        //                        the CBR2_DO_NOT_CACHE flag is set, the cacheIndex
        //                        MUST be set to BITMAPCACHE_WAITING_LIST_INDEX
        //                        (32767).
        stream.out_2BUE(this->do_not_cache ? BITMAPCACHE_WAITING_LIST_INDEX : this->idx);

        // No compression header in our case
        // ---------------------------------
        // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure (see
        //                           [MS-RDPBCGR] section 2.2.9.1.1.3.1.2.3)
        //                           describing the bitmap data in the
        //                           bitmapDataStream. This field MUST be present if
        //                           the TS_CACHE_BITMAP_COMPRESSED_REV2 (0x05) flag
        //                           is present in the header field, but the
        //                           CBR2_NO_BITMAP_COMPRESSION_HDR (0x08) flag is
        //                           not.

        // bitmapDataStream (variable): A variable-length byte array containing
        //                              bitmap data (the format of this data is
        //                              defined in [MS-RDPBCGR] section
        //                              2.2.9.1.1.3.1.2.2).

        // Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        // for uncompressed bitmaps the format is quite simple
        stream.out_copy_bytes(this->bmp.data(), this->bmp.bmp_size());

        stream.set_out_uint16_le(stream.get_offset() - (offset_header + 12), offset_header);
    }

    void receive(InStream & stream, const RDPSecondaryOrderHeader & header, const BGRPalette & palette, BitsPerPixel session_color_depth)
    {
        switch (header.type){
        case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
            this->receive_raw_v1(stream, header, palette, session_color_depth);
        break;
        case RDP::TS_CACHE_BITMAP_COMPRESSED:
            this->receive_compressed_v1(stream, header, palette, session_color_depth);
        break;
        case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
            this->receive_raw_v2(stream, header, palette, session_color_depth);
        break;
        case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
            this->receive_compressed_v2(stream, header, palette, session_color_depth);
        break;
        default:
            // can't happen, ensured by caller
            LOG(LOG_ERR, "Unexpected header type %u in rdp_orders_bmp_cache", header.type);
        }
    }

    void receive_raw_v2( InStream & stream, const RDPSecondaryOrderHeader & header
                       , const BGRPalette & palette, BitsPerPixel session_color_depth)
    {
        using namespace RDP;

        uint16_t extraFlags = header.flags;
        this->id            =   extraFlags & 0x0007;
        uint8_t cbr2_bpp    = ((extraFlags & 0x0078) >> 3);
        BitsPerPixel bpp    = cbr2_bpp_to_bpp(session_color_depth, cbr2_bpp);
        uint8_t cbr2_flags  = ((extraFlags & 0xFF80) >> 7);
        //LOG(LOG_INFO, "RDPBmpCache::receive_raw_v2: cbr2_bpp=%u cbr2_flags=0x%X", cbr2_bpp, cbr2_flags);

        if (cbr2_flags & CBR2_PERSISTENT_KEY_PRESENT) {
            this->key1 = stream.in_uint32_le();
            this->key2 = stream.in_uint32_le();
        }
        else {
            this->key1 = 0;
            this->key2 = 0;
        }

        uint16_t bitmapWidth  = stream.in_2BUE();
        uint16_t bitmapHeight;
        if (cbr2_flags & CBR2_HEIGHT_SAME_AS_WIDTH) {
            bitmapHeight = bitmapWidth;
        }
        else {
            bitmapHeight = stream.in_2BUE();
        }

        uint32_t bitmapLength = stream.in_4BUE();
        //LOG( LOG_INFO, "RDPBmpCache::receive_raw_v2: bitmapWidth=%u bitmapHeight=%u bitmapLength=%u"
        //   , bitmapWidth, bitmapHeight, bitmapLength);

        this->idx = stream.in_2BUE();
        //LOG(LOG_INFO, "RDPBmpCache::receive_raw_v2: cache_id=%u cacheIndex=%u", this->id, this->idx);

        const uint8_t * bitmapDataStream = stream.in_uint8p(bitmapLength);
        //if (this->verbose & 0x8000) {
        //    LOG(LOG_INFO,
        //        "RDPBmpCache::receive_raw_v2: session_bpp=%u bpp=%u width=%u height=%u size=%u",
        //        session_color_depth, bpp, bitmapWidth, bitmapHeight, bitmapLength);
        //    LOG(LOG_INFO, "Palette");
        //    hexdump_d(palette.data(), palette.data_size());
        //    LOG(LOG_INFO, "Bitmap");
        //    hexdump_d(bitmapDataStream, bitmapLength);
        //}
        this->bmp = Bitmap(session_color_depth, bpp, &palette, bitmapWidth, bitmapHeight,
            bitmapDataStream, bitmapLength, false);

        if (bitmapLength != this->bmp.bmp_size()){
            LOG( LOG_WARNING
               , "RDPBmpCache::receive_raw_v2: "
                 "broadcasted bufsize should be the same as bmp size computed from cx, cy, bpp and alignment rules");
        }
    }

    void receive_raw_v1(InStream & stream, const RDPSecondaryOrderHeader &/* header*/
                       , const BGRPalette & palette, BitsPerPixel session_color_depth)
    {
//        LOG(LOG_INFO, "receive raw v1");
        using namespace RDP;

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

        this->id = stream.in_uint8();

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.

        stream.in_skip_bytes(1);

        // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the
        // bitmap in pixels.

        uint8_t width = stream.in_uint8();

        // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
        //  bitmap in pixels.

        uint8_t height = stream.in_uint8();

        // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color
        //  depth of the bitmap data in bits-per-pixel. This field MUST be one
        //  of the following values.
        //  0x08 8-bit color depth.
        //  0x10 16-bit color depth.
        //  0x18 24-bit color depth.
        //  0x20 32-bit color depth.

        BitsPerPixel bpp{stream.in_uint8()};

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.

        uint16_t bufsize = stream.in_uint16_le();

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).

        this->idx = stream.in_uint16_le();

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        // TODO some error may occur inside bitmap (memory allocation  file load  decompression) we should catch thrown exception and emit some explicit log if that occurs (anyway that will lead to end of connection  as we can't do much to repair such problems).
        const uint8_t * buf = stream.in_uint8p(bufsize);
        //if (this->verbose & 0x8000) {
        //    LOG(LOG_INFO,
        //        "Uncompressed bitmap v1: session_bpp=%u bpp=%u width=%u height=%u size=%u",
        //        session_color_depth, bpp, width, height, bufsize);
        //    LOG(LOG_INFO, "Palette");
        //    hexdump_d(palette.data(), palette.data_size());
        //    LOG(LOG_INFO, "Bitmap");
        //    hexdump_d(buf, bufsize);
        //}
        this->bmp = Bitmap(session_color_depth, bpp, &palette, width, height, buf, bufsize);

        if (bufsize != this->bmp.bmp_size()){
            LOG(LOG_WARNING, "broadcasted bufsize should be the same as bmp size computed from cx, cy, bpp and alignment rules");
        }
    }

    void receive_compressed_v2( InStream & stream, const RDPSecondaryOrderHeader & header
                              , const BGRPalette & palette, BitsPerPixel session_color_depth)
    {
        using namespace RDP;

        uint16_t extraFlags = header.flags;
        this->id            =   extraFlags & 0x0007;
        uint8_t cbr2_bpp    = ((extraFlags & 0x0078) >> 3);
        BitsPerPixel bpp    = cbr2_bpp_to_bpp(session_color_depth, cbr2_bpp);
        uint8_t cbr2_flags  = ((extraFlags & 0xFF80) >> 7);
        //LOG(LOG_INFO, "RDPBmpCache::receive_compressed_v2: cbr2_bpp=%u cbr2_flags=0x%X", cbr2_bpp, cbr2_flags);

        if (cbr2_flags & CBR2_PERSISTENT_KEY_PRESENT) {
            this->key1 = stream.in_uint32_le();
            this->key2 = stream.in_uint32_le();
        }
        else {
            this->key1 = 0;
            this->key2 = 0;
        }

        uint16_t bitmapWidth  = stream.in_2BUE();
        uint16_t bitmapHeight;
        if (cbr2_flags & CBR2_HEIGHT_SAME_AS_WIDTH) {
            bitmapHeight = bitmapWidth;
        }
        else {
            bitmapHeight = stream.in_2BUE();
        }

        uint32_t bitmapLength = stream.in_4BUE();
        //LOG( LOG_INFO, "RDPBmpCache::receive_compressed_v2: bitmapWidth=%u bitmapHeight=%u bitmapLength=%u"
        //   , bitmapWidth, bitmapHeight, bitmapLength);

        this->idx = stream.in_2BUE();
        //LOG(LOG_INFO, "RDPBmpCache::receive_compressed_v2: cache_id=%u cacheIndex=%u", this->id, this->idx);

        if (cbr2_flags & CBR2_NO_BITMAP_COMPRESSION_HDR) {
            const uint8_t * bitmapDataStream = stream.in_uint8p(bitmapLength);
            //if (this->verbose & 0x8000) {
            //    LOG(LOG_INFO,
            //        "CRDPBmpCache::receive_compressed_v2: session_bpp=%u bpp=%u width=%u height=%u size=%u",
            //        session_color_depth, bpp, bitmapWidth, bitmapHeight, bitmapLength);
            //    LOG(LOG_INFO, "Palette");
            //    hexdump_d(palette.data(), palette.data_size());
            //    LOG(LOG_INFO, "Bitmap");
            //    hexdump_d(bitmapDataStream, bitmapLength);
            //}
            this->bmp = Bitmap(session_color_depth, bpp, &palette, bitmapWidth, bitmapHeight,
                bitmapDataStream, bitmapLength, true);
        }
        else {
            // Compressed Data Header (TS_CD_HEADER).
            stream.in_skip_bytes(2);    /* cbCompFirstRowSize(2) */
            uint16_t cbCompMainBodySize = stream.in_uint16_le();
            uint16_t cbScanWidth        = stream.in_uint16_le();
            uint16_t cbUncompressedSize = stream.in_uint16_le();

            const uint8_t * bitmapDataStream = stream.in_uint8p(cbCompMainBodySize);

            //if (this->verbose & 0x8000) {
            //    LOG(LOG_INFO,
            //        "RDPBmpCache::receive_compressed_v2: session_bpp=%u bpp=%u width=%u height=%u size=%u",
            //        session_color_depth, bpp, bitmapWidth, bitmapWidth, cbCompMainBodySize);
            //    LOG(LOG_INFO, "Palette");
            //    hexdump_d(palette.data(), palette.data_size());
            //    LOG(LOG_INFO, "Bitmap");
            //    hexdump_d(bitmapDataStream, cbCompMainBodySize);
            //}
            this->bmp = Bitmap(session_color_depth, bpp, &palette, bitmapWidth, bitmapHeight,
                bitmapDataStream, cbCompMainBodySize, true);
            if (cbScanWidth != (this->bmp.bmp_size() / this->bmp.cy())){
                LOG( LOG_WARNING
                   , "RDPBmpCache::receive_compressed_v2: "
                     "broadcasted row_size should be the same as line size computed from cx, bpp and alignment rules");
            }
            if (cbUncompressedSize != this->bmp.bmp_size()){
                LOG( LOG_WARNING
                   , "RDPBmpCache::receive_compressed_v2: "
                     "broadcasted final_size should be the same as bmp size computed from cx, cy, bpp and alignment rules");
            }
        }
    }

    void receive_compressed_v1( InStream & stream, const RDPSecondaryOrderHeader & header
                              , const BGRPalette & palette, BitsPerPixel session_color_depth)
    {
        int flags = header.flags;
        this->id = stream.in_uint8();
        stream.in_uint8(); // skip pad1
        uint8_t width = stream.in_uint8();
        uint8_t height = stream.in_uint8();
        BitsPerPixel bpp{stream.in_uint8()};
        uint16_t bufsize = stream.in_uint16_le();
        this->idx = stream.in_uint16_le();

        if (flags & NO_BITMAP_COMPRESSION_HDR) {
            const uint8_t* data = stream.in_uint8p(bufsize);
            //if (this->verbose & 0x8000) {
            //    LOG(LOG_INFO,
            //        "Compressed bitmap: session_bpp=%u bpp=%u width=%u height=%u size=%u",
            //        session_color_depth, bpp, width, height, bufsize);
            //    LOG(LOG_INFO, "Palette");
            //    hexdump_d(palette.data(), palette.data_size());
            //    LOG(LOG_INFO, "Bitmap");
            //    hexdump_d(data, bufsize);
            //}
            this->bmp = Bitmap(session_color_depth, bpp, &palette, width, height, data, bufsize, true);
        }
        else {
            stream.in_uint16_le(); // skip padding
            uint16_t size = stream.in_uint16_le();       // actual size of compressed buffer
            uint16_t row_size = stream.in_uint16_le();   // size of a row
            uint16_t final_size = stream.in_uint16_le(); // size of bitmap after decompression
            const uint8_t* data = stream.in_uint8p(size);

            //if (this->verbose & 0x8000) {
            //    LOG(LOG_INFO,
            //        "Compressed bitmap v1: session_bpp=%u bpp=%u width=%u height=%u size=%u",
            //        session_color_depth, bpp, width, height, size);
            //    LOG(LOG_INFO, "Palette");
            //    hexdump_d(palette.data(), palette.data_size());
            //    LOG(LOG_INFO, "Bitmap");
            //    hexdump_d(data, size);
            //}
            this->bmp = Bitmap(session_color_depth, bpp, &palette, width, height, data, size, true);
            if (row_size != (this->bmp.bmp_size() / this->bmp.cy())){
                LOG(LOG_WARNING, "broadcasted row_size should be the same as line size computed from cx, bpp and alignment rules");
            }
            if (final_size != this->bmp.bmp_size()){
                LOG(LOG_WARNING, "broadcasted final_size should be the same as bmp size computed from cx, cy, bpp and alignment rules");
            }
        }
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg = snprintf(buffer, sz,
            "RDPBmpCache(id=%d idx=%d bpp=%u cx=%u cy=%u)",
            this->id, this->idx, unsigned(this->bmp.bpp()),
            unsigned(this->bmp.cx()), unsigned(this->bmp.cy()));
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level) const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        LOG(level, "%s", buffer);
    }

private:
    static BitsPerPixel cbr2_bpp_to_bpp(BitsPerPixel session_color_depth, uint8_t cbr2_bpp)
    {
        switch (cbr2_bpp) {
            case CBR2_8BPP:
                return BitsPerPixel{8};
            case CBR2_16BPP:
                // Support of 16-bit bitmaps in 15-bit RDP session.
                return (session_color_depth == BitsPerPixel{15})
                    ? BitsPerPixel{15} : BitsPerPixel{16};
            break;
            case CBR2_24BPP:
                return BitsPerPixel{24};
            break;
            case CBR2_32BPP:
                return BitsPerPixel{32};
            break;
            default:
                LOG(LOG_ERR, "RDPBmpCache::receive_compressed_v2: Unsupported bitsPerPixelId(0x%X)", cbr2_bpp);
                throw Error(ERR_RDP_PROTOCOL);
        }
    }
};
