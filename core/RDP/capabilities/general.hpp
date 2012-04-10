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

   RDP Capabilities : General Capabilities

*/

#if !defined(__RDP_CAPABILITIES_GENERAL_HPP__)
#define __RDP_CAPABILITIES_GENERAL_HPP__

#include "constants.hpp"

// 2.2.7.1.1 General Capability Set (TS_GENERAL_CAPABILITYSET)
// ===========================================================

// The TS_GENERAL_CAPABILITYSET structure is used to advertise general
// characteristics and is based on the capability set specified in [T128]
// section 8.2.3. This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//  capability set. This field MUST be set to CAPSTYPE_GENERAL (1).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//  of the capability data, including the size of the capabilitySetType and
//  lengthCapability fields.

// osMajorType (2 bytes): A 16-bit, unsigned integer. The type of platform.

// +--------------------------------+----------------------+
// | 0x0000 OSMAJORTYPE_UNSPECIFIED | Unspecified platform |
// +--------------------------------+----------------------+
// | 0x0001 OSMAJORTYPE_WINDOWS     | Windows platform     |
// +--------------------------------+----------------------+
// | 0x0002 OSMAJORTYPE_OS2         | OS/2 platform        |
// +--------------------------------+----------------------+
// | 0x0003 OSMAJORTYPE_MACINTOSH   | Macintosh platform   |
// +--------------------------------+----------------------+
// | 0x0004 OSMAJORTYPE_UNIX        | UNIX platform        |
// +--------------------------------+----------------------+

// osMinorType (2 bytes): A 16-bit, unsigned integer. The version of the
// platform specified in the osMajorType field.

// +--------------------------------------+----------------------+
// | 0x0000 OSMINORTYPE_UNSPECIFIED       | Unspecified version  |
// +--------------------------------------+----------------------+
// | 0x0001 OSMINORTYPE_WINDOWS_31X       | Windows 3.1x         |
// +--------------------------------------+----------------------+
// | 0x0002 TS_OSMINORTYPE_WINDOWS_95     | Windows 95           |
// +--------------------------------------+----------------------+
// | 0x0003 TS_OSMINORTYPE_WINDOWS_NT     | Windows NT           |
// +--------------------------------------+----------------------+
// | 0x0004 TS_OSMINORTYPE_OS2_V21        | OS/2 2.1             |
// +--------------------------------------+----------------------+
// | 0x0005 TS_OSMINORTYPE_POWER_PC       | PowerPC              |
// +--------------------------------------+----------------------+
// | 0x0006 TS_OSMINORTYPE_MACINTOSH      | Macintosh            |
// +--------------------------------------+----------------------+
// | 0x0007 TS_OSMINORTYPE_NATIVE_XSERVER | Native X Server      |
// +--------------------------------------+----------------------+
// | 0x0008 TS_OSMINORTYPE_PSEUDO_XSERVER | Pseudo X Server      |
// +--------------------------------------+----------------------+

// protocolVersion (2 bytes): A 16-bit, unsigned integer. The protocol version.
// This field MUST be set to TS_CAPS_PROTOCOLVERSION (0x0200).

// pad2octetsA (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
// field MUST be ignored.

// generalCompressionTypes (2 bytes): A 16-bit, unsigned integer. General
// compression types. This field MUST be set to 0.

// extraFlags (2 bytes): A 16-bit, unsigned integer. General capability
// information. Supported flags depends on RDP version.

// +----------------------------------+-------------------------------+------+
// | 0x0001 FASTPATH_OUTPUT_SUPPORTED | Advertiser supports fast-path | 5.0+ |
// |                                  | output.                       |      |
// +----------------------------------+-------------------------------+------+
// |                                  | Advertiser supports excluding |      |
// |                                  | the 8-byte Compressed Data    |      |
// |        0x0400                    | Header                        |      |
// |                                  | (section 2.2.9.1.1.3.1.2.3)   |      |
// |  NO_BITMAP_COMPRESSION_HDR       | from the Bitmap Data          | 5.0+ |
// |                                  | (section 2.2.9.1.1.3.1.2.2)   |      |
// |                                  | structure or the Cache Bitmap |      |
// |                                  | (Revision 2) Secondary Drawing|      |
// |                                  | Order ([MS-RDPEGDI] section   |      |
// |                                  | 2.2.2.2.1.2.3).               |      |
// +----------------------------------+-------------------------------+------+
// | 0x0004 LONG_CREDENTIALS_SUPPORTED| Advertiser supports           |      |
// |                                  | long-length credentials for   |      |
// |                                  | the user name, password, or   | 5.1+ |
// |                                  | domain name in the Save       |      |
// |                                  | Session Info PDU              |      |
// |                                  | (section 2.2.10.1).           |      |
// +----------------------------------+-------------------------------+------+
// | 0x0008 AUTORECONNECT_SUPPORTED   | Advertiser supports           |      |
// |                                  | auto-reconnection             | 5.2+ |
// |                                  | (section 5.5).                |      |
// +----------------------------------+-------------------------------+------+
// | 0x0010 ENC_SALTED_CHECKSUM       | Advertiser supports salted    |      |
// |                                  | MAC generation (see           | 5.2+ |
// |                                  | section 5.3.6.1.1).           |      |
// +----------------------------------+-------------------------------+------+

// updateCapabilityFlag (2 bytes): A 16-bit, unsigned integer. Support for
//  update capability. This field MUST be set to 0.

// remoteUnshareFlag (2 bytes): A 16-bit, unsigned integer. Support for remote
//  unsharing. This field MUST be set to 0.

// generalCompressionLevel (2 bytes): A 16-bit, unsigned integer. General
// compression level. This field MUST be set to 0.

// refreshRectSupport (1 byte): An 8-bit, unsigned integer. Server-only flag
// that indicates whether the Refresh Rect PDU (section 2.2.11.2) is supported.

// +------------+------------------------------------------+
// | 0x00 FALSE | Server does not support Refresh Rect PDU.|
// +------------+------------------------------------------+
// | 0x01 TRUE  | Server supports Refresh Rect PDU.        |
// +------------+------------------------------------------+

// suppressOutputSupport (1 byte): An 8-bit, unsigned integer. Server-only flag
// that indicates whether the Suppress Output PDU (section 2.2.11.3) is
// supported.

// +------------+----------------------------------------------+
// | 0x00 FALSE | Server does not support Suppress Output PDU. |
// +------------+----------------------------------------------+
// | 0x01 TRUE  | Server supports Suppress Output PDU.         |
// +------------+----------------------------------------------+


static inline void cs_out_general_caps(Stream & stream, int use_rdp5)
{
    LOG(LOG_INFO, "Sending General caps to server");

    stream.out_uint16_le(RDP_CAPSET_GENERAL);
    const uint16_t offset_len = stream.get_offset(0);
    stream.out_uint16_le(0);

    stream.out_uint16_le(1); /* OS major type */
    stream.out_uint16_le(3); /* OS minor type */
    stream.out_uint16_le(0x200); /* Protocol version */
    stream.out_uint16_le(0); /* Pad */
    stream.out_uint16_le(0); /* Compression types */
    // 0x040D
    // ------
    // 0x0400 NO_BITMAP_COMPRESSION_HDR
    // 0x0008 AUTORECONNECT_SUPPORTED
    // 0x0004 LONG_CREDENTIALS_SUPPORTED
    // 0x0001 FASTPATH_OUTPUT_SUPPORTED
    stream.out_uint16_le(use_rdp5?0x40C:0); // 0 for RDP4

    stream.out_uint16_le(0); /* Update capability */
    stream.out_uint16_le(0); /* Remote unshare capability */
    stream.out_uint16_le(0); /* Compression level */
    stream.out_uint16_le(0); /* Pad */
    stream.set_out_uint16_le(RDP_CAPLEN_GENERAL, offset_len);
}

    /*****************************************************************************/
static inline void cs_in_general_caps(Stream & stream, int len, bool & use_compact_packets)
{
    LOG(LOG_INFO, "Received General caps from client");

    uint16_t os_major = stream.in_uint16_le(); /* OS major type */
    LOG(LOG_INFO, "General caps::major %u", os_major);
    uint16_t os_minor = stream.in_uint16_le(); /* OS minor type */
    LOG(LOG_INFO, "General caps::minor %u", os_minor);
    uint16_t protocolVersion = stream.in_uint16_le(); /* Protocol version */
    LOG(LOG_INFO, "General caps::protocol %u", protocolVersion);
    (void)stream.in_uint16_le(); /* Pad */
    uint16_t compressionType = stream.in_uint16_le(); /* Compression types */
    LOG(LOG_INFO, "General caps::compression types %x", compressionType);

    /* Receiving rdp_5 extra flags supported for RDP 5.0 and later versions*/
    uint16_t extraflags = stream.in_uint16_le();
    LOG(LOG_INFO, "General caps::extra flags %x", extraflags);

    enum {
        FASTPATH_OUTPUT_SUPPORTED = 0x0001,
        LONG_CREDENTIALS_SUPPORTED = 0x0004,
        AUTORECONNECT_SUPPORTED = 0x0008,
        ENC_SALTED_CHECKSUM = 0x0010,
        NO_BITMAP_COMPRESSION_HDR = 0x0400,
    };

    if (extraflags & FASTPATH_OUTPUT_SUPPORTED){
        LOG(LOG_INFO, "FASTPATH_OUTPUT_SUPPORTED");
    }

    if (extraflags & LONG_CREDENTIALS_SUPPORTED){
        LOG(LOG_INFO, "LONG_CREDENTIALS_SUPPORTED");
    }

    if (extraflags & AUTORECONNECT_SUPPORTED){
        LOG(LOG_INFO, "AUTORECONNECT_SUPPORTED");
    }

    if (extraflags & ENC_SALTED_CHECKSUM){
        LOG(LOG_INFO, "ENC_SALTED_CHECKSUM");
    }

    if (extraflags & NO_BITMAP_COMPRESSION_HDR){
        use_compact_packets = true;
        LOG(LOG_INFO, "NO_BITMAP_COMPRESSION_HDR");
    }

    
    uint16_t updateCapability = stream.in_uint16_le(); /* Update capability */
    LOG(LOG_INFO, "General caps::updateCapability %x", updateCapability);

    uint16_t remoteUnshare = stream.in_uint16_le(); /* Remote unshare capability */
    LOG(LOG_INFO, "General caps::remoteUnshare %x", remoteUnshare);

    uint16_t compressionLevel = stream.in_uint16_le(); /* Compression level */
    LOG(LOG_INFO, "General caps::compressionLevel %x", compressionLevel);

    (void)stream.in_uint16_le(); /* Pad */
}


static inline void sc_out_general_caps(Stream & stream)
{
    LOG(LOG_INFO, "Sending General caps to client");

    stream.out_uint16_le(RDP_CAPSET_GENERAL); /* 1 */
    stream.out_uint16_le(RDP_CAPLEN_GENERAL); /* 24(0x18) */
    stream.out_uint16_le(1); /* OS major type */
    stream.out_uint16_le(3); /* OS minor type */
    stream.out_uint16_le(0x200); /* Protocol version */
    stream.out_uint16_le(0); /* pad */
    stream.out_uint16_le(0); /* Compression types */
    stream.out_uint16_le(0); /* pad use 0x40d for rdp packets, 0 for not */
    stream.out_uint16_le(0); /* Update capability */
    stream.out_uint16_le(0); /* Remote unshare capability */
    stream.out_uint16_le(0); /* Compression level */
    stream.out_uint16_le(0); /* Pad */
}

static inline void sc_in_general_caps(Stream & stream)
{
    LOG(LOG_INFO, "Received General caps from server");

    uint16_t os_major = stream.in_uint16_le(); /* OS major type */
    LOG(LOG_INFO, "General caps::major %u", os_major);
    uint16_t os_minor = stream.in_uint16_le(); /* OS minor type */
    LOG(LOG_INFO, "General caps::minor %u", os_minor);
    uint16_t protocolVersion = stream.in_uint16_le(); /* Protocol version */
    LOG(LOG_INFO, "General caps::protocol %u", protocolVersion);
    (void)stream.in_uint16_le(); /* Pad */
    uint16_t compressionType = stream.in_uint16_le(); /* Compression types */
    LOG(LOG_INFO, "General caps::compression types %x", compressionType);
    /* Receiving rdp_5 extra flags supported for RDP 5.0 and later versions*/
    uint16_t extraflags = stream.in_uint16_le();
    LOG(LOG_INFO, "General caps::extra flags %x", extraflags);

    uint16_t updateCapability = stream.in_uint16_le(); /* Update capability */
    LOG(LOG_INFO, "General caps::updateCapability %x", updateCapability);
    uint16_t remoteUnshare = stream.in_uint16_le(); /* Remote unshare capability */
    LOG(LOG_INFO, "General caps::remoteUnshare %x", remoteUnshare);
    uint16_t compressionLevel = stream.in_uint16_le(); /* Compression level */
    LOG(LOG_INFO, "General caps::compressionLevel %x", compressionLevel);
    (void)stream.in_uint16_le(); /* Pad */

}

#endif
