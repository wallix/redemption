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

#ifndef _REDEMPTION_CORE_RDP_CAPABILITIES_GENERAL_HPP_
#define _REDEMPTION_CORE_RDP_CAPABILITIES_GENERAL_HPP_

// 2.2.7.1.1 General Capability Set (TS_GENERAL_CAPABILITYSET)
// ===========================================================

// The TS_GENERAL_CAPABILITYSET structure is used to advertise general
// characteristics and is based on the capability set specified in [T128]
// section 8.2.3. This capability is sent by both client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the
//    capability set. This field MUST be set to CAPSTYPE_GENERAL (1).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes
//    of the capability data, including the size of the capabilitySetType and
//    lengthCapability fields.

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

enum {
       OSMAJORTYPE_UNSPECIFIED
     , OSMAJORTYPE_WINDOWS
     , OSMAJORTYPE_OS2
     , OSMAJORTYPE_MACINTOSH
     , OSMAJORTYPE_UNIX
};


// osMinorType (2 bytes): A 16-bit, unsigned integer. The version of the
//    platform specified in the osMajorType field.

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

enum {
       OSMINORTYPE_UNSPECIFIED
     , OSMINORTYPE_WINDOWS_31X
     , TS_OSMINORTYPE_WINDOWS_95
     , TS_OSMINORTYPE_WINDOWS_NT
     , TS_OSMINORTYPE_OS2_V21
     , TS_OSMINORTYPE_POWER_PC
     , TS_OSMINORTYPE_MACINTOSH
     , TS_OSMINORTYPE_NATIVE_XSERVER
     , TS_OSMINORTYPE_PSEUDO_XSERVER
};

// protocolVersion (2 bytes): A 16-bit, unsigned integer. The protocol version.
//    This field MUST be set to TS_CAPS_PROTOCOLVERSION (0x0200).

// pad2octetsA (2 bytes): A 16-bit, unsigned integer. Padding. Values in this
//    field MUST be ignored.

// generalCompressionTypes (2 bytes): A 16-bit, unsigned integer. General
//    compression types. This field MUST be set to 0.

// extraFlags (2 bytes): A 16-bit, unsigned integer. General capability
//    information. Supported flags depends on RDP version.

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

enum {
    FASTPATH_OUTPUT_SUPPORTED = 0x0001,
    LONG_CREDENTIALS_SUPPORTED = 0x0004,
    AUTORECONNECT_SUPPORTED = 0x0008,
    ENC_SALTED_CHECKSUM = 0x0010,
    NO_BITMAP_COMPRESSION_HDR = 0x0400,
};

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

enum {
    CAPLEN_GENERAL = 24
};


struct GeneralCaps : public Capability {
    uint16_t os_major;
    uint16_t os_minor;
    uint16_t protocolVersion;
    uint16_t pad1;
    uint16_t compressionType;
    uint16_t extraflags;
    uint16_t updateCapability;
    uint16_t remoteUnshare;
    uint16_t compressionLevel;
    uint16_t pad2;
    GeneralCaps()
    : Capability(CAPSTYPE_GENERAL, CAPLEN_GENERAL)
    , os_major(OSMAJORTYPE_WINDOWS)
    , os_minor(TS_OSMINORTYPE_WINDOWS_NT)
    , protocolVersion(0x200)
    , pad1(0)
    , compressionType(0)
    , extraflags(0)
    , updateCapability(0)
    , remoteUnshare(0)
    , compressionLevel(0)
    , pad2(0)
    {
    }

    void emit(Stream & stream){
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->os_major);
        stream.out_uint16_le(this->os_minor);
        stream.out_uint16_le(this->protocolVersion);
        stream.out_uint16_le(this->pad1);
        stream.out_uint16_le(this->compressionType);
        stream.out_uint16_le(this->extraflags);
        stream.out_uint16_le(this->updateCapability);
        stream.out_uint16_le(this->remoteUnshare);
        stream.out_uint16_le(this->compressionLevel);
        stream.out_uint16_le(this->pad2);
        TODO("Add support for server only flags refreshRectSupport and suppressOutputSupport")
    }

    void recv(Stream & stream, uint16_t len){
        this->len = len;
        this->os_major = stream.in_uint16_le();
        this->os_minor = stream.in_uint16_le();
        this->protocolVersion = stream.in_uint16_le();
        this->pad1 = stream.in_uint16_le();
        this->compressionType = stream.in_uint16_le();
        this->extraflags = stream.in_uint16_le();
        this->updateCapability = stream.in_uint16_le();
        this->remoteUnshare = stream.in_uint16_le();
        this->compressionLevel = stream.in_uint16_le();
        this->pad2 = stream.in_uint16_le();
        TODO("Add support for server only flags refreshRectSupport and suppressOutputSupport")
    }

    void log(const char * msg){
        LOG(LOG_INFO, "%s General caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "General caps::major %u", this->os_major);
        LOG(LOG_INFO, "General caps::minor %u", this->os_minor);
        LOG(LOG_INFO, "General caps::protocol %u", this->protocolVersion);
        LOG(LOG_INFO, "General caps::compression type %x", this->compressionType);
        LOG(LOG_INFO, "General caps::extra flags %x", this->extraflags);
        LOG(LOG_INFO, "General caps::extraflags:FASTPATH_OUTPUT_SUPPORTED %s",
            (extraflags & FASTPATH_OUTPUT_SUPPORTED)?"yes":"no");
        LOG(LOG_INFO, "General caps::extraflags:LONG_CREDENTIALS_SUPPORTED %s",
            (this->extraflags & LONG_CREDENTIALS_SUPPORTED)?"yes":"no");
        LOG(LOG_INFO, "General caps::extraflags:AUTORECONNECT_SUPPORTED %s",
            (this->extraflags & AUTORECONNECT_SUPPORTED)?"yes":"no");
        LOG(LOG_INFO, "General caps::extraflags:ENC_SALTED_CHECKSUM %s",
            (this->extraflags & ENC_SALTED_CHECKSUM)?"yes":"no");
        LOG(LOG_INFO, "General caps::extraflags:NO_BITMAP_COMPRESSION_HDR %s",
            (this->extraflags & NO_BITMAP_COMPRESSION_HDR)?"yes":"no");
        LOG(LOG_INFO, "General caps::updateCapability %x", this->updateCapability);
        LOG(LOG_INFO, "General caps::remoteUnshare %x", this->remoteUnshare);
        LOG(LOG_INFO, "General caps::compressionLevel %x", this->compressionLevel);

        TODO("Add support for server only flags refreshRectSupport and suppressOutputSupport")
    }
};

#endif
