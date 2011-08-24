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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp module main header file

*/

#if !defined(CORE_RDP_RDP_HPP__)
#define CORE_RDP_RDP_HPP__

// [MS-RDPBCGR] 2.2.8.1.1.1.1 Share Control Header (TS_SHARECONTROLHEADER)
// =======================================================================

// The TS_SHARECONTROLHEADER header is a T.128 legacy mode header (see [T128]
// section 8.3) present in slow-path I/O packets.

// totalLength (2 bytes): A 16-bit, unsigned integer. The total length of the
//   packet in bytes (the length includes the size of the Share Control Header).

// pduType (2 bytes): A 16-bit, unsigned integer. It contains the PDU type and
//   protocol version information. The format of the pduType word is described
//   by the following bitmask diagram:

// type (4 bits): Least significant 4 bits of the least significant byte.

// +-----------------------------+---------------------------------------------+
// | 1 PDUTYPE_DEMANDACTIVEPDU   | Demand Active PDU (section 2.2.1.13.1).     |
// +-----------------------------+---------------------------------------------+
// | 3 PDUTYPE_CONFIRMACTIVEPDU  | Confirm Active PDU (section 2.2.1.13.2).    |
// +-----------------------------+---------------------------------------------+
// | 6 PDUTYPE_DEACTIVATEALLPDU  | Deactivate All PDU (section 2.2.3.1).       |
// +-----------------------------+---------------------------------------------+
// | 7 PDUTYPE_DATAPDU           | Data PDU (actual type is revealed by the    |
// |                             | pduType2 field in the Share Data Header     |
// |                             | (section 2.2.8.1.1.1.2)  structure).        |
// +-----------------------------+---------------------------------------------+
// | 10 PDUTYPE_SERVER_REDIR_PKT | Enhanced Security Server Redirection PDU    |
// |                             | (see [MS-RDPEGDI] section 2.2.3.3.1).       |
// +-----------------------------+---------------------------------------------+

// versionLow (4 bits): Most significant 4 bits of the least significant byte.
//   This field MUST be set to TS_PROTOCOL_VERSION (0x1).

// versionHigh (1 byte): Most significant byte. This field MUST be set to 0x00.

// PDUSource (2 bytes): A 16-bit, unsigned integer. The channel ID which is the
//   transmission source of the PDU.

// [MS-RDPBCGR] 2.2.8.1.1.1.2 Share Data Header (TS_SHAREDATAHEADER)
// =================================================================
// The TS_SHAREDATAHEADER header is a T.128 legacy mode header (see [T128]
// section 8.3) present in slow-path I/O packets.

// shareControlHeader (6 bytes): Share Control Header (section 2.2.8.1.1.1.1)
//   containing information about the packet.

// shareId (4 bytes): A 32-bit, unsigned integer. Share identifier for the
//   packet (see [T128] section 8.4.2 for more information about share IDs).

// pad1 (1 byte): An 8-bit, unsigned integer. Padding. Values in this field are
//   ignored.

// streamId (1 byte): An 8-bit, unsigned integer. The stream identifier for the
//   packet.
// +------------------------+--------------------------------------------------+
// | 0x00 STREAM_UNDEFINED  | Undefined stream priority. This value might be   |
// |                        | used in the Server Synchronize PDU (see section  |
// |                        | 2.2.1.19) due to a server-side RDP bug. It MUST  |
// |                        | not be used in conjunction with any other PDUs.  |
// |                        |                                                  |
// +------------------------+--------------------------------------------------+
// | 0x01 STREAM_LOW        | Low-priority stream.                             |
// +------------------------+--------------------------------------------------+
// | 0x02 STREAM_MED        | Medium-priority stream.                          |
// +------------------------+--------------------------------------------------+
// | 0x04 STREAM_HI         | High-priority stream.                            |
// +------------------------+--------------------------------------------------+

// uncompressedLength (2 bytes): A 16-bit, unsigned integer. The uncompressed
//   length of the packet in bytes.

// pduType2 (1 byte): An 8-bit, unsigned integer. The type of data PDU.

// +------------------------------+--------------------------------------------+
// | 2 PDUTYPE2_UPDATE            | Update PDU (section 2.2.9.1.1.3)           |
// +------------------------------+--------------------------------------------+
// | 20 PDUTYPE2_CONTROL          | Control PDU (section 2.2.1.15.1)           |
// +------------------------------+--------------------------------------------+
// | 27 PDUTYPE2_POINTER          | Pointer Update PDU (section 2.2.9.1.1.4)   |
// +------------------------------+--------------------------------------------+
// | 28 PDUTYPE2_INPUT            | Input PDU (section 2.2.8.1.1.3)            |
// +------------------------------+--------------------------------------------+
// | 31 PDUTYPE2_SYNCHRONIZE      | Synchronize PDU (section 2.2.1.14.1)       |
// +------------------------------+--------------------------------------------+
// | 33 PDUTYPE2_REFRESH_RECT     | Refresh Rect PDU (section 2.2.11.2.1)      |
// +------------------------------+--------------------------------------------+
// | 34 PDUTYPE2_PLAY_SOUND       | Play Sound PDU (section 2.2.9.1.1.5.1)     |
// +------------------------------+--------------------------------------------+
// | 35 PDUTYPE2_SUPPRESS_OUTPUT  | Suppress Output PDU (section 2.2.11.3.1)   |
// +------------------------------+--------------------------------------------+
// | 36 PDUTYPE2_SHUTDOWN_REQUEST | Shutdown Request PDU (section 2.2.2.2.1)   |
// +------------------------------+------+-------------------------------------+
// | 37 PDUTYPE2_SHUTDOWN_DENIED         | Shutdown Request Denied PDU         |
// |                                     |         (section 2.2.2.3.1)         |
// +-------------------------------------+-------------------------------------+
// | 38 PDUTYPE2_SAVE_SESSION_INFO       | Save Session Info PDU               |
// |                                     |         (section 2.2.10.1.1)        |
// +-------------------------------------+-------------------------------------+
// | 39 PDUTYPE2_FONTLIST                | Font List PDU (section 2.2.1.18.1)  |
// +-------------------------------------+-------------------------------------+
// | 40 PDUTYPE2_FONTMAP                 | Font Map PDU (section 2.2.1.22.1)   |
// +-------------------------------------+-------------------------------------+
// | 41 PDUTYPE2_SET_KEYBOARD_INDICATORS | Set Keyboard Indicators PDU         |
// |                                     |       (section 2.2.8.2.1.1)         |
// +-------------------------------------+---+---------------------------------+
// | 43 PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST | Persistent Key List PDU         |
// |                                         | (section 2.2.1.17.1)            |
// +-------------------------------------+---+---------------------------------+
// | 44 PDUTYPE2_BITMAPCACHE_ERROR_PDU   | Bitmap Cache Error PDU (see         |
// |                                     |     [MS-RDPEGDI] section 2.2.2.3.1) |
// +-------------------------------------+-------------------------------------+
// | 45 PDUTYPE2_SET_KEYBOARD_IME_STATUS | Set Keyboard IME Status PDU         |
// |                                     |       (section 2.2.8.2.2.1)         |
// +-----------------------------------+-+-------------------------------------+
// | 46 PDUTYPE2_OFFSCRCACHE_ERROR_PDU | Offscreen Bitmap Cache Error PDU      |
// |                                   | (see [MS-RDPEGDI] section 2.2.2.3.2)  |
// +-----------------------------------+---------------------------------------+
// | 47 PDUTYPE2_SET_ERROR_INFO_PDU    | Set Error Info PDU (section 2.2.5.1.1)|
// +-----------------------------------+---------------------------------------+
// | 48 PDUTYPE2_DRAWNINEGRID_ERROR_PDU| DrawNineGrid Cache Error PDU          |
// |                                   | (see [MS-RDPEGDI] section 2.2.2.3.3)  |
// +-----------------------------------+---------------------------------------+
// | 49 PDUTYPE2_DRAWGDIPLUS_ERROR_PDU | GDI+ Error PDU (see [MS-RDPEGDI]      |
// |                                   |  section 2.2.2.3.4)                   |
// +-----------------------------------+---------------------------------------+
// | 50 PDUTYPE2_ARC_STATUS_PDU        | Auto-Reconnect Status PDU             |
// |                                   |    (section 2.2.4.1.1)                |
// +-----------------------------------+---------------------------------------+

// compressedType (1 byte): An 8-bit, unsigned integer. The compression type
//   and flags specifying the data following the Share Data Header (section
//   2.2.8.1.1.1.2).

// +--------------------------+------------------------------------------------+
// | 0x0F CompressionTypeMask | Indicates the package which was used for       |
// |                          | compression. See the table which follows for a |
// |                          | list of compression packages.                  |
// +--------------------------+------------------------------------------------+
// | 0x20 PACKET_COMPRESSED   | The payload data is compressed. This value     |
// |                          | corresponds to MPPC bit C (see [RFC2118]       |
// |                          | section 3.1).                                  |
// +--------------------------+------------------------------------------------+
// | 0x40 PACKET_AT_FRONT     | The decompressed packet MUST be placed at the  |
// |                          | beginning of the history buffer. This value    |
// |                          | corresponds to MPPC bit B (see [RFC2118]       |
// |                          | section 3.1).                                  |
// +--------------------------+------------------------------------------------+
// | 0x80 PACKET_FLUSHED      | The history buffer MUST be reinitialized. This |
// |                          | value corresponds to MPPC bit A (see [RFC2118] |
// |                          | section 3.1).                                  |
// +--------------------------+------------------------------------------------+

// Possible compression package values:

// +---------------------------+-----------------------------------------------+
// | 0 PACKET_COMPR_TYPE_8K    | MPPC-8K compression (see section 3.1.8.4.1)   |
// +---------------------------+-----------------------------------------------+
// | 1 PACKET_COMPR_TYPE_64K   | MPPC-64K compression (see section 3.1.8.4.2)  |
// +---------------------------+-----------------------------------------------+
// | 2 PACKET_COMPR_TYPE_RDP6  | RDP 6.0 bulk compression (see [MS-RDPEGDI]    |
// |                           | section 3.1.8.1).                             |
// +---------------------------+-----------------------------------------------+
// | 2 PACKET_COMPR_TYPE_RDP61 | RDP 6.1 bulk compression (see [MS-RDPEGDI]    |
// |                           | section 3.1.8.2).                             |
// +---------------------------+-----------------------------------------------+

// Instructions specifying how to compress a data stream are listed in section
//   3.1.8.2, while decompression of a data stream is described in section
//   3.1.8.3.

// compressedLength (2 bytes): A 16-bit, unsigned integer. The compressed length
//   of the packet in bytes.


class ShareControlAndDataOut
{
    Stream & stream;
    uint8_t offlen1;
    uint8_t offlen2;
    public:
    ShareControlAndDataOut(Stream & stream, uint8_t pdu_type1, uint8_t pdu_type2, uint16_t mcs_channel, uint32_t share_id)
        : stream(stream), offlen1(stream.p - stream.data), offlen2(this->offlen1+12)
    {
        // share control
        stream.skip_uint8(2); // len
        stream.out_uint16_le(0x10 | pdu_type1);
        stream.out_uint16_le(mcs_channel);
        // share data
        stream.out_uint32_le(share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.skip_uint8(2); // len - 14
        stream.out_uint8(pdu_type2);
        stream.out_uint8(0);
        stream.out_uint16_le(0);
    }

    void end(){
        int len = stream.p - stream.data + this->offlen1;
        stream.set_out_uint16_le(len, this->offlen1);
        stream.set_out_uint16_le(len - 14, this->offlen2);
    }
};


class ShareControlOut
{
    Stream & stream;
    uint8_t offlen;
    public:
    ShareControlOut(Stream & stream, uint8_t pdu_type1, uint16_t mcs_channel)
        : stream(stream), offlen(stream.p - stream.data)
    {
        stream.skip_uint8(2); // len
        stream.out_uint16_le(0x10 | pdu_type1);
        stream.out_uint16_le(mcs_channel);
    }

    void end(){
        int len = stream.p - stream.data + this->offlen;
        stream.set_out_uint16_le(len, this->offlen);
    }
};

class ShareDataOut
{
    Stream & stream;
    uint8_t offlen;
    public:
    ShareDataOut(Stream & stream, uint8_t pdu_type2, uint32_t share_id)
        : stream(stream), offlen(stream.p - stream.data + 6)
    {
        stream.out_uint32_le(share_id);
        stream.out_uint8(0);
        stream.out_uint8(1);
        stream.skip_uint8(2); // len - 14
        stream.out_uint8(pdu_type2);
        stream.out_uint8(0);
        stream.out_uint16_le(0);
    }

    void end(){
        int len = stream.p - stream.data - 14;
        stream.set_out_uint16_le(len, this->offlen);
    }
};

#endif
