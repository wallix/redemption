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

#if !defined(__RDPORDERSSECONDARYBMPCACHE_HPP__)
#define __RDPORDERSSECONDARYBMPCACHE_HPP__


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

    // bitmapDataStream (variable): A variable-length byte array containing
    //  bitmap data (the format of this data is defined in [MS-RDPBCGR] section
    //  2.2.9.1.1.3.1.2.2).

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
    // ----------------------------------------------------------
    //  The TS_BITMAP_DATA structure wraps the bitmap data bytestream for a
    //  screen area rectangle containing a clipping taken from the server-side
    //  screen frame buffer.

    // destLeft (2 bytes): A 16-bit, unsigned integer. Left bound of the
    //  rectangle.

    // destTop (2 bytes): A 16-bit, unsigned integer. Top bound of the
    //  rectangle.

    // destRight (2 bytes): A 16-bit, unsigned integer. Right bound of the
    //  rectangle.

    // destBottom (2 bytes): A 16-bit, unsigned integer. Bottom bound of the
    //  rectangle.

    // width (2 bytes): A 16-bit, unsigned integer. The width of the rectangle.

    // height (2 bytes): A 16-bit, unsigned integer. The height of the
    //  rectangle.

    // bitsPerPixel (2 bytes): A 16-bit, unsigned integer. The color depth of
    //  the rectangle data in bits-per-pixel.

    // Flags (2 bytes): A 16-bit, unsigned integer. The flags describing the
    //  format of the bitmap data in the bitmapDataStream field.

    // +----------------------------------+------------------------------------+
    // | 0x0001 BITMAP_COMPRESSION        | Indicates that the bitmap data is  |
    // |                                  | compressed.This implies that the   |
    // |                                  | bitmapComprHdr field is present if |
    // |                                  | the NO_BITMAP_COMPRESSION_HDR      |
    // |                                  | (0x0400) flag is not set.          |
    // +----------------------------------+------------------------------------+
    // | 0x0400 NO_BITMAP_COMPRESSION_HDR | Indicates that the bitmapComprHdr  |
    // |                                  | field is not present (removed for  |
    // |                                  | bandwidth efficiency to save 8     |
    // |                                  | bytes).                            |
    // +----------------------------------+------------------------------------+

    // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of
    //  the data in the bitmapComprHdr and bitmapDataStream fields.

    // bitmapComprHdr (8 bytes): Optional Compressed Data Header structure
    //  (see Compressed Data Header (TS_CD_HEADER) (section 2.2.9.1.1.3.1.2.3))
    //  specifying the bitmap data in the bitmapDataStream. This field MUST be
    //  present if the BITMAP_COMPRESSION (0x0001) flag is present in the Flags
    //  field, but the NO_BITMAP_COMPRESSION_HDR (0x0400) flag is not.

    // bitmapDataStream (variable): A variable-sized array of bytes.
    //  Uncompressed bitmap data represents a bitmap as a bottom-up,
    //  left-to-right series of pixels. Each pixel is a whole
    //  number of bytes. Each row contains a multiple of four bytes
    // (including up to three bytes of padding, as necessary).

    // Compressed bitmaps not in 32 bpp format are compressed using Interleaved
    // RLE and encapsulated in an RLE Compresssed Bitmap Stream structure (see
    // section 2.2.9.1.1.3.1.2.4) while compressed bitmap data at a color depth
    // of 32 bpp is compressed using RDP 6.0 Bitmap Compression and stored
    // inside an RDP 6.0 Bitmap Compressed Stream structure (see section
    // 2.2.2.5.1 in [MS-RDPEGDI]).

    // MS-RDPBCGR: 2.2.9.1.1.3.1.2.3 Compressed Data Header (TS_CD_HEADER)
    // -------------------------------------------------------------------

    // The TS_CD_HEADER structure is used to describe compressed bitmap data.

    // cbCompFirstRowSize (2 bytes): A 16-bit, unsigned integer. The field
    //  MUST be set to 0x0000.

    // cbCompMainBodySize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the compressed bitmap data (which follows this header).

    // cbScanWidth (2 bytes): A 16-bit, unsigned integer. The width of the
    //  bitmap (which follows this header) in pixels (this value MUST be
    //  divisible by 4).

    // cbUncompressedSize (2 bytes): A 16-bit, unsigned integer. The size in
    //  bytes of the bitmap data (which follows this header) after it has been
    //  decompressed.

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
    int cache_id;
    Bitmap * bmp;
    int bpp;
    int cache_idx;
    const ClientInfo * client_info;

    RDPBmpCache(int bpp, Bitmap * bmp, int cache_id, int cache_idx, const ClientInfo * client_info) :
                    cache_id(cache_id),
                    bmp(bmp),
                    bpp(bpp),
                    cache_idx(cache_idx),
                    client_info(client_info)
    {
    }

    RDPBmpCache(int bpp) : bpp(bpp)
    {
    }

    ~RDPBmpCache()
    {
    }

    void emit(Stream & stream) const
    {
        using namespace RDP;
        if (0 == this->client_info->bitmap_cache_version){
            if (this->client_info->use_bitmap_comp){
//                LOG(LOG_INFO, "/* BMP Cache compressed V1 */");
                this->emit_v1_compressed(stream);
            }
            else {
//                LOG(LOG_INFO, "/* BMP Cache raw V1 */");
                this->emit_raw_v1(stream);
            }
        }
        else {
            if (this->client_info->use_bitmap_comp){
//                LOG(LOG_INFO, "/* BMP Cache compressed V2 */");
                this->emit_v2_compressed(stream);
            }
            else {
//                LOG(LOG_INFO, "/* BMP Cache raw V2 */");
                this->emit_raw_v2(stream);
            }
        }
    }

    void emit_v1_compressed(Stream & stream) const
    {
        using namespace RDP;

        bool small_headers = this->client_info->op2;
        Stream tmp(16384);
        this->bmp->compress(this->bpp, tmp);
        size_t bufsize = tmp.p - tmp.data;

//        LOG(LOG_INFO, "bufsize=%u [%u]", bufsize, tmp.data[0]);

        int order_flags = STANDARD | SECONDARY;
        stream.out_uint8(order_flags);
        /* length after type minus 7 */
        stream.out_uint16_le(bufsize + (small_headers?2:10));
        stream.out_uint16_le(small_headers?1024:8); /* flags */
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED); /* type */

        stream.out_uint8(cache_id);
        stream.out_clear_bytes(1); /* pad */

        stream.out_uint8(align4(this->bmp->cx));
        stream.out_uint8(this->bmp->cy);
        stream.out_uint8(this->bpp);
        stream.out_uint16_le(bufsize/* + 8*/);
        stream.out_uint16_le(this->cache_idx);

        if (!small_headers){
            stream.out_clear_bytes(2); /* pad */
            stream.out_uint16_le(bufsize);
            stream.out_uint16_le(this->bmp->line_size(this->bpp));
            stream.out_uint16_le(this->bmp->bmp_size(this->bpp)); /* final size */
        }

        stream.out_copy_bytes(tmp.data, bufsize);

    }

    void emit_v2_compressed(Stream & stream) const
    {
        using namespace RDP;

        Stream tmp(16384);
        this->bmp->compress(bpp, tmp);
        size_t bufsize = tmp.p - tmp.data;

        int Bpp = nbbytes(this->bpp);

        stream.out_uint8(STANDARD | SECONDARY);

        stream.out_uint16_le(bufsize - 1); /* length after type minus 7 */
        stream.out_uint16_le(0x400 | (((Bpp + 2) << 3) & 0x38) | (cache_id & 7)); /* flags */
        stream.out_uint8(TS_CACHE_BITMAP_COMPRESSED_REV2); /* type */
        stream.out_uint8(align4(this->bmp->cx));
        stream.out_uint8(this->bmp->cy);
        stream.out_uint16_be(bufsize | 0x4000);
        stream.out_uint8(((this->cache_idx >> 8) & 0xff) | 0x80);
        stream.out_uint8(this->cache_idx);
        stream.out_copy_bytes(tmp.data, bufsize);
    }

    void emit_raw_v1(Stream & stream) const
    {
//        LOG(LOG_INFO, "emit_raw_v1(cache_id=%d, cache_idx=%d)\n",
//                this->cache_id, this->cache_idx);

        using namespace RDP;
        unsigned int row_size = align4(this->bmp->cx * nbbytes(this->bpp));

        #warning this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);
//        LOG(LOG_INFO, "out_uint8::Standard and secondary");

        stream.out_uint16_le(9 + this->bmp->cy * row_size  - 7); // length after orderType - 7
//        LOG(LOG_INFO, "out_uint16_le::len %d\n", 9 + this->height * row_size - 7);

        stream.out_uint16_le(8);        // extraFlags
//        LOG(LOG_INFO, "out_uint16_le::flags %d\n", 8);

        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED); // type
//        LOG(LOG_INFO, "out_uint8::orderType %d\n", RAW_BMPCACHE);

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

        stream.out_uint8(this->cache_id);
//        LOG(LOG_INFO, "out_uint8::cache_id %d\n", cache_id);

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.

        stream.out_clear_bytes(1);
//        LOG(LOG_INFO, "out_uint8::pad\n");

        // bitmapWidth (1 byte): An 8-bit, unsigned integer. The width of the
        // bitmap in pixels.

        stream.out_uint8(this->bmp->cx);
//        LOG(LOG_INFO, "out_uint8::width=%d\n", width);

        // bitmapHeight (1 byte): An 8-bit, unsigned integer. The height of the
        //  bitmap in pixels.

        stream.out_uint8(this->bmp->cy);
//        LOG(LOG_INFO, "out_uint8::height=%d\n", height);

        // bitmapBitsPerPel (1 byte): An 8-bit, unsigned integer. The color
        //  depth of the bitmap data in bits-per-pixel. This field MUST be one
        //  of the following values.
        //  0x08 8-bit color depth.
        //  0x10 16-bit color depth.
        //  0x18 24-bit color depth.
        //  0x20 32-bit color depth.

        stream.out_uint8(this->bpp);
//        LOG(LOG_INFO, "out_uint8::bpp=%d\n", bpp);

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.

//        LOG(LOG_INFO, "out_uint16::bufsize=%d\n", bufsize);
        stream.out_uint16_le(this->bmp->cy * row_size);

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).

        stream.out_uint16_le(this->cache_idx);

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        for (size_t y = 0 ; y < this->bmp->cy; y++) {
            stream.out_copy_bytes(this->bmp->data_co(this->bpp) + y * row_size, row_size);
        }
    }

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
    // | 0x08 CBR2_NO_BITMAP_COMPRESSION_HEADER | Indicates that the           |
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

    void emit_raw_v2(Stream & stream) const
    {
        using namespace RDP;
        unsigned int row_size = align4(this->bmp->cx * nbbytes(this->bpp));

        #warning this should become some kind of emit header
        uint8_t control = STANDARD | SECONDARY;
        stream.out_uint8(control);

        // here is length, it will be computed when packet will be complete
        uint8_t * length_ptr = stream.p;
        stream.skip_uint8(2);

        int bitsPerPixelId = nbbytes(this->bpp)+2;

        #warning some optimisations are possible here if we manage flags, but what will we do with persistant bitmaps ? We definitely do not want to save them on disk from here. There must be some kind of persistant structure where to save them and check if they exist.
        uint16_t flags = 0;

        // header::extraFlags : (flags:9, bitsPerPixelId:3, cacheId:3)
        stream.out_uint16_le((flags << 6)
            |((bitsPerPixelId << 3)& 0x38)
            | (this->cache_id & 7));

        // header::orderType
        stream.out_uint8(TS_CACHE_BITMAP_UNCOMPRESSED_REV2);

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
        stream.out_uint8(align4(this->bmp->cx));

        // bitmapHeight (variable): A Two-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.2) structure. The height of the
        //                          bitmap in pixels.
        stream.out_uint8(this->bmp->cy);

        // bitmapLength (variable): A Four-Byte Unsigned Encoding (section
        //                          2.2.2.2.1.2.1.4) structure. The size in bytes
        //                          of the data in the bitmapComprHdr and
        //                          bitmapDataStream fields.
        stream.out_uint16_be((this->bmp->cy * row_size) | 0x4000);

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
        stream.out_2BUE(this->cache_idx);

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

        // for uncompressed bitmaps the format is quite simple
        stream.out_copy_bytes(this->bmp->data_co(this->bpp), this->bmp->cy * row_size);
        stream.set_length(-12, length_ptr);
    }

    void receive(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
            this->receive_raw_v1(stream, control, header);
    }

    void receive_raw_v2(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
        using namespace RDP;
        #warning DO NOT USE : partial implementation, we do not know yet how to manage persistant bitmap storage
    }

    void receive_raw_v1(Stream & stream, const uint8_t control, const RDPSecondaryOrderHeader & header)
    {
//        LOG(LOG_INFO, "receive raw v1");
        using namespace RDP;

        // cacheId (1 byte): An 8-bit, unsigned integer. The bitmap cache into
        //  which to store the bitmap data. The bitmap cache ID MUST be in the
        // range negotiated by the Bitmap Cache Capability Set (Revision 1)
        //  (see [MS-RDPBCGR] section 2.2.7.1.4.1).

        this->cache_id = stream.in_uint8();

        // pad1Octet (1 byte): An 8-bit, unsigned integer. Padding. Values in
        // this field are arbitrary and MUST be ignored.

        stream.skip_uint8(1);

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

        uint8_t bpp = stream.in_uint8();

        // bitmapLength (2 bytes): A 16-bit, unsigned integer. The size in
        //  bytes of the data in the bitmapComprHdr and bitmapDataStream
        //  fields.

        uint16_t bufsize = stream.in_uint16_le();

        // cacheIndex (2 bytes): A 16-bit, unsigned integer. An entry in the
        // bitmap cache (specified by the cacheId field) where the bitmap MUST
        // be stored. The bitmap cache index MUST be in the range negotiated by
        // the Bitmap Cache Capability Set (Revision 1) (see [MS-RDPBCGR]
        // section 2.2.7.1.4.1).

        this->cache_idx = stream.in_uint16_le();

        // bitmapDataStream (variable): A variable-length byte array containing
        //  bitmap data (the format of this data is defined in [MS-RDPBCGR]
        // section 2.2.9.1.1.3.1.2.2).

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        this->bmp = new Bitmap(bpp, NULL, width, height, stream.in_uint8p(bufsize), bufsize);
        assert(bufsize == this->bmp->bmp_size(bpp));
    }

    bool operator==(const RDPBmpCache & other) const {
        return true;
    }

    size_t str(char * buffer, size_t sz) const
    {
        size_t lg  = snprintf(buffer, sz, "RDPBmpCache(cache_id=%u cache_idx=%u bpp=%u cache_version=%u compression=%u)",
            this->cache_id, this->cache_idx, this->bpp,
            this->client_info->bitmap_cache_version,
            this->client_info->use_bitmap_comp);
        if (lg >= sz){
            return sz;
        }
        return lg;
    }

    void log(int level) const
    {
        char buffer[1024];
        this->str(buffer, 1024);
        LOG(level, buffer);
    }

};


#endif
