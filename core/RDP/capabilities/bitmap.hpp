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

#if !defined(__RDP_CAPABILITIES_BITMAP_HPP__)
#define __RDP_CAPABILITIES_BITMAP_HPP__

#include "log.hpp"

#include "constants.hpp"

// 2.2.7.1.2    Bitmap Capability Set (TS_BITMAP_CAPABILITYSET)
// ============================================================

//  The TS_BITMAP_CAPABILITYSET structure is used to advertise bitmap-oriented
//    characteristics and is based on the capability set specified in [T128]
// section 8.2.4. This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//   capability set. This field MUST be set to CAPSTYPE_BITMAP (2).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//   of the capability data, including the size of the capabilitySetType and
//   lengthCapability fields.

// preferredBitsPerPixel (2 bytes): A 16-bit, unsigned integer. Color depth of
//   the remote session. In RDP 4.0 and 5.0, this field MUST be set to 8 (even
//   for a 16-color session).

// receive1BitPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 1 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive4BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   the client can receive 4 bpp. This field is ignored and SHOULD be set to
//   TRUE (0x0001).

// receive8BitsPerPixel (2 bytes): A 16-bit, unsigned integer. Indicates whether
//    the client can receive 8 bpp. This field is ignored and SHOULD be set to
//    TRUE (0x0001).

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The width of the desktop
//   in the remote session.

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The height of the
//   desktop in the remote session.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field are ignored.

// desktopResizeFlag (2 bytes): A 16-bit, unsigned integer. Indicates whether
//   desktop resizing is supported.
//   0x0000 FALSE  Desktop resizing is not supported.
//   0x0001 TRUE   Desktop resizing is supported.
//   If a desktop resize occurs, the server will deactivate the session (see
//   section 1.3.1.3), and on session reactivation will specify the new desktop
//   size in the desktopWidth and desktopHeight fields in the Bitmap Capability
//   Set, along with a value of TRUE for the desktopResizeFlag field. The client
//   should check these sizes and, if different from the previous desktop size,
//   resize any windows to support this size.

// bitmapCompressionFlag (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports bitmap compression. RDP requires bitmap
//   compression and hence this field MUST be set to TRUE (0x0001). If it is not
//   set to TRUE, the server MUST NOT continue with the connection.

// highColorFlags (1 byte): An 8-bit, unsigned integer. Client support for
//   16 bpp color modes. This field is ignored and SHOULD be set to 0.

// drawingFlags (1 byte): An 8-bit, unsigned integer. Flags describing support
//   for 32 bpp bitmaps.

// +----------------------------------------+----------------------------------+
// | 0x02 DRAW_ALLOW_DYNAMIC_COLOR_FIDELITY | Indicates support for lossy      |
// |                                        | compression of 32 bpp bitmaps by |
// |                                        | reducing color-fidelity on a     |
// |                                        | per-pixel basis.                 |
// +----------------------------------------+----------------------------------+
// | 0x04 DRAW_ALLOW_COLOR_SUBSAMPLING      | Indicates support for chroma     |
// |                                        | subsampling when compressing     |
// |                                        | 32 bpp bitmaps.                  |
// +----------------------------------------+----------------------------------+
// | 0x08 DRAW_ALLOW_SKIP_ALPHA             | Indicates that the client        |
// |                                        | supports the removal of the      |
// |                                        | alpha-channel when compressing   |
// |                                        | 32 bpp bitmaps. In this case the |
// |                                        | alpha is assumed to be 0xFF,     |
// |                                        | meaning the bitmap is opaque.    |
// |                                        | Compression of 32 bpp bitmaps is |
// |                                        | specified in [MS-RDPEGDI]        |
// |                                        | section 3.1.9.                   |
// +----------------------------------------+----------------------------------+

// multipleRectangleSupport (2 bytes): A 16-bit, unsigned integer. Indicates
//   whether the client supports the use of multiple bitmap rectangles. RDP
//   requires the use of multiple bitmap rectangles and hence this field MUST be
//   set to TRUE (0x0001). If it is not set to TRUE, the server MUST NOT
//   continue with the connection.

// pad2octetsB (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//   field are ignored.

static inline void out_bitmap_caps(Stream & stream, uint16_t bpp, uint16_t bitmap_compression)
{
    LOG(LOG_INFO, "Sending bitmap caps to remote server\n");
    stream.out_uint16_le(RDP_CAPSET_BITMAP);
    stream.out_uint16_le(RDP_CAPLEN_BITMAP);
    stream.out_uint16_le(bpp); /* Preferred bpp */
    stream.out_uint16_le(1); /* Receive 1 BPP */
    stream.out_uint16_le(1); /* Receive 4 BPP */
    stream.out_uint16_le(1); /* Receive 8 BPP */
    stream.out_uint16_le(800); /* Desktop width */
    stream.out_uint16_le(600); /* Desktop height */
    stream.out_uint16_le(0); /* Pad */
    stream.out_uint16_le(1); /* Allow resize */
    stream.out_uint16_le(bitmap_compression); /* Support compression */
    stream.out_uint16_le(0); /* Unknown */
    stream.out_uint16_le(1); /* Unknown */
    stream.out_uint16_le(0); /* Pad */
}

/* Process a bitmap capability set */
static inline void process_bitmap_caps(Stream & stream, uint16_t & bpp)
{
    bpp = stream.in_uint16_le();
    stream.skip_uint8(6);
    uint16_t width = stream.in_uint16_le();
    uint16_t height = stream.in_uint16_le();
    /* todo, call reset if needed and use width and height */
    LOG(LOG_INFO, "Server bitmap caps (%dx%dx%d) [bpp=%d] ok\n", width, height, bpp, bpp);
}

static inline void front_out_bitmap_caps(Stream & stream, uint16_t bpp, uint16_t width, uint16_t height)
{
    stream.out_uint16_le(RDP_CAPSET_BITMAP); /* 2 */
    stream.out_uint16_le(RDP_CAPLEN_BITMAP); /* 28(0x1c) */
    stream.out_uint16_le(bpp); /* Preferred BPP */
    stream.out_uint16_le(1); /* Receive 1 BPP */
    stream.out_uint16_le(1); /* Receive 4 BPP */
    stream.out_uint16_le(1); /* Receive 8 BPP */
    stream.out_uint16_le(width); /* width */
    stream.out_uint16_le(height); /* height */
    stream.out_uint16_le(0); /* Pad */
    stream.out_uint16_le(1); /* Allow resize */
    stream.out_uint16_le(1); /* bitmap compression */
    stream.out_uint16_le(0); /* unknown */
    stream.out_uint16_le(0); /* unknown */
    stream.out_uint16_le(0); /* pad */
}

#endif

