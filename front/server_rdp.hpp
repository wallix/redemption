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

   header file. rdp layer at core module

*/

#if !defined(__RDP_HPP__)
#define __RDP_HPP__

#include "server_sec.hpp"
#include "client_info.hpp"
#include "config.hpp"
#include "error.hpp"
#include "callback.hpp"
#include "colors.hpp"
#include "altoco.hpp"
#include "RDP/x224.hpp"

/* rdp */
struct server_rdp {
    int up_and_running;

    int share_id;
    int mcs_channel;
    struct ClientInfo client_info;
    struct server_sec sec_layer;
    uint32_t packet_number;

    server_rdp(Transport * trans, Inifile * ini)
        :
        up_and_running(0),
        share_id(65538),
        mcs_channel(0),
        client_info(ini),
        sec_layer(&client_info, trans),
        packet_number(1)
    {
    }

    ~server_rdp()
    {
    }


    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
        CHANNEL_FLAG_FIRST = 0x01,
        CHANNEL_FLAG_LAST = 0x02,
        CHANNEL_FLAG_SHOW_PROTOCOL = 0x10,
    };


    void server_send_to_channel(int channel_id, uint8_t *data, int data_len,
                               int total_data_len, int flags) throw (Error)
    {
        Stream stream(data_len + 1024); /* this should be big enough */
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.channel_hdr = stream.p;
        stream.p += 8;

        stream.out_copy_bytes(data, data_len);
        stream.mark_end();

        int index = channel_id - MCS_GLOBAL_CHANNEL - 1;
        int count = (int) this->sec_layer.channel_list.size();
        if (index < 0 || index >= count) {
            throw Error(ERR_MCS_CHANNEL_NOT_FOUND);
        }
        mcs_channel_item* channel = this->sec_layer.channel_list[index];

        stream.p = stream.channel_hdr;
        stream.out_uint32_le(total_data_len);
        if (channel->flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        assert(channel->chanid == channel_id);

//        LOG(LOG_INFO, "1) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, channel_id);

        stream.p = stream.end;
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

    // Global palette cf [MS-RDPCGR] 2.2.9.1.1.3.1.1.1 Palette Update Data
    // -------------------------------------------------------------------

    // updateType (2 bytes): A 16-bit, unsigned integer. The graphics update type.
    // This field MUST be set to UPDATETYPE_PALETTE (0x0002).

    // pad2Octets (2 bytes): A 16-bit, unsigned integer. Padding.
    // Values in this field are ignored.

    // numberColors (4 bytes): A 32-bit, unsigned integer.
    // The number of RGB triplets in the paletteData field.
    // This field MUST be set to NUM_8BPP_PAL_ENTRIES (256).

    void send_global_palette(const BGRPalette & palette) throw (Error)
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_uint16_le(RDP_UPDATE_PALETTE);

        stream.out_uint16_le(0);

        stream.out_uint32_le(256); /* # of colors */

        for (int i = 0; i < 256; i++) {
            int color = palette[i];
            uint8_t r = color >> 16;
            uint8_t g = color >> 8;
            uint8_t b = color;
            stream.out_uint8(b);
            stream.out_uint8(g);
            stream.out_uint8(r);
        }
        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_UPDATE, stream.rdp_hdr - stream.data);

//        LOG(LOG_INFO, "2) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

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


    void send_rdp_packet(Stream & stream, uint8_t pdu_type1, uint8_t pdu_type2, size_t offset_hdr)
    {
        int len = stream.end - &(stream.data[offset_hdr]);
        // ShareControlHeader
        stream.set_out_uint16_le(len,                 offset_hdr);
        stream.set_out_uint16_le(0x10 | pdu_type1,    offset_hdr+2);
        stream.set_out_uint16_le(this->mcs_channel,   offset_hdr+4);

        stream.set_out_uint32_le(this->share_id,      offset_hdr+6);
        stream.set_out_uint8(0,                       offset_hdr+10);
        stream.set_out_uint8(1,                       offset_hdr+11);
        stream.set_out_uint16_le(len - 14,            offset_hdr+12);
        stream.set_out_uint8(pdu_type2,               offset_hdr+14);
        stream.set_out_uint8(0,                       offset_hdr+15);
        stream.set_out_uint16_le(0,                   offset_hdr+16);

//        LOG(LOG_INFO, "RDP Packet #%u (type=%u type2=%u)", this->packet_number++, PDUTYPE_DATAPDU, pdu_type2);
    }

//    2.2.9.1.1.4     Server Pointer Update PDU (TS_POINTER_PDU)
//    ----------------------------------------------------------
//    The Pointer Update PDU is sent from server to client and is used to convey
//    pointer information, including pointers' bitmap images, use of system or
//    hidden pointers, use of cached cursors and position updates.

//    tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//    x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//      section 13.7.

//    mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//      encapsulates an MCS Send Data Indication structure, as specified in
//      [T125] (the ASN.1 structure definitions are given in [T125] section 7,
//      parts 7 and 10). The userData field of the MCS Send Data Indication
//      contains a Security Header and the Pointer Update PDU data.

//    securityHeader (variable): Optional security header. If the Encryption
//      Level (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater
//      than ENCRYPTION_LEVEL_NONE (0) and the Encryption Method
//      (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//      ENCRYPTION_METHOD_NONE (0) then this field will contain one of the
//      following headers:

//      - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_LOW (1).

//      - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//        Level selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2) or ENCRYPTION_LEVEL_HIGH (3).

//      - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//        selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//        ENCRYPTION_LEVEL_FIPS (4).

//      If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the
//      server is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections
//      5.3.2 and 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE
//      (0), then this header is not included in the PDU.

//    shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//      containing information about the packet. The type subfield of the
//      pduType field of the Share Control Header (section 2.2.8.1.1.1.1) MUST
//      be set to PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data
//      Header MUST be set to PDUTYPE2_POINTER (27).

//    messageType (2 bytes): A 16-bit, unsigned integer. Type of pointer update.

//    +--------------------------------+---------------------------------------+
//    | 0x0001 TS_PTRMSGTYPE_SYSTEM    | Indicates a System Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.3).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0003 TS_PTRMSGTYPE_POSITION  | Indicates a Pointer Position Update   |
//    |                                | (section 2.2.9.1.1.4.2).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0006 TS_PTRMSGTYPE_COLOR     | Indicates a Color Pointer Update      |
//    |                                | (section 2.2.9.1.1.4.4).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0007 TS_PTRMSGTYPE_CACHED    | Indicates a Cached Pointer Update     |
//    |                                | (section 2.2.9.1.1.4.6).              |
//    +--------------------------------+---------------------------------------+
//    | 0x0008 TS_PTRMSGTYPE_POINTER   | Indicates a New Pointer Update        |
//    |                                | (section 2.2.9.1.1.4.5).              |
//    +--------------------------------+---------------------------------------+


//    2.2.9.1.1.4.2     Pointer Position Update (TS_POINTERPOSATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_POINTERPOSATTRIBUTE structure is used to indicate that the client
//    pointer should be moved to the specified position relative to the top-left
//    corner of the server's desktop (see [T128] section 8.14.4).

//    position (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing
//     the new x-coordinates and y-coordinates of the pointer.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.



//    2.2.9.1.1.4.3     System Pointer Update (TS_SYSTEMPOINTERATTRIBUTE)
//    -------------------------------------------------------------------
//    The TS_SYSTEMPOINTERATTRIBUTE structure is used to hide the pointer or to
//    set its shape to that of the operating system default (see [T128] section
//    8.14.1).

//    systemPointerType (4 bytes): A 32-bit, unsigned integer.
//    The type of system pointer.
//    +---------------------------|------------------------------+
//    | 0x00000000 SYSPTR_NULL    | The hidden pointer.          |
//    +---------------------------|------------------------------+
//    | 0x00007F00 SYSPTR_DEFAULT | The default system pointer.  |
//    +---------------------------|------------------------------+


//    2.2.9.1.1.4.4     Color Pointer Update (TS_COLORPOINTERATTRIBUTE)
//    -----------------------------------------------------------------
//    The TS_COLORPOINTERATTRIBUTE structure represents a regular T.128 24 bpp
//    color pointer, as specified in [T128] section 8.14.3. This pointer update
//    is used for both monochrome and color pointers in RDP.

    void server_rdp_send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(RDP_POINTER_COLOR);
        stream.out_uint16_le(0); /* pad */

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. The zero-based cache
//      entry in the pointer cache in which to store the pointer image. The
//      number of cache entries is negotiated using the Pointer Capability Set
//      (section 2.2.7.1.5).

        stream.out_uint16_le(cache_idx);

//    hotSpot (4 bytes): Point (section 2.2.9.1.1.4.1) structure containing the
//      x-coordinates and y-coordinates of the pointer hotspot.
//            2.2.9.1.1.4.1  Point (TS_POINT16)
//            ---------------------------------
//            The TS_POINT16 structure specifies a point relative to the
//            top-left corner of the server's desktop.

//            xPos (2 bytes): A 16-bit, unsigned integer. The x-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(x);

//            yPos (2 bytes): A 16-bit, unsigned integer. The y-coordinate
//              relative to the top-left corner of the server's desktop.

        stream.out_uint16_le(y);

//    width (2 bytes): A 16-bit, unsigned integer. The width of the pointer in
//      pixels (the maximum allowed pointer width is 32 pixels).

        stream.out_uint16_le(32);

//    height (2 bytes): A 16-bit, unsigned integer. The height of the pointer
//      in pixels (the maximum allowed pointer height is 32 pixels).

        stream.out_uint16_le(32);

//    lengthAndMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the andMaskData field.

        stream.out_uint16_le(128);

//    lengthXorMask (2 bytes): A 16-bit, unsigned integer. The size in bytes of
//      the xorMaskData field.

        stream.out_uint16_le(3072);

//    xorMaskData (variable): Variable number of bytes: Contains the 24-bpp,
//      bottom-up XOR mask scan-line data. The XOR mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 3x3 pixel cursor
//      is being sent, then each scan-line will consume 10 bytes (3 pixels per
//      scan-line multiplied by 3 bpp, rounded up to the next even number of
//      bytes).

        #warning a memcopy (or equivalent build in stream) would be much more efficient
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                stream.out_uint8(*data++);
                stream.out_uint8(*data++);
                stream.out_uint8(*data++);
            }
        }

//    andMaskData (variable): Variable number of bytes: Contains the 1-bpp,
//      bottom-up AND mask scan-line data. The AND mask is padded to a 2-byte
//      boundary for each encoded scan-line. For example, if a 7x7 pixel cursor
//      is being sent, then each scan-line will consume 2 bytes (7 pixels per
//      scan-line multiplied by 1 bpp, rounded up to the next even number of
//      bytes).


        stream.out_copy_bytes(mask, 128); /* mask */

//    colorPointerData (1 byte): Single byte representing unused padding.
//      The contents of this byte should be ignored.

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_POINTER, stream.rdp_hdr - stream.data);
//        LOG(LOG_INFO, "3) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }


//    2.2.9.1.1.4.5    New Pointer Update (TS_POINTERATTRIBUTE)
//    ---------------------------------------------------------
//    The TS_POINTERATTRIBUTE structure is used to send pointer data at an
//    arbitrary color depth. Support for the New Pointer Update is advertised
//    in the Pointer Capability Set (section 2.2.7.1.5).

//    xorBpp (2 bytes): A 16-bit, unsigned integer. The color depth in
//      bits-per-pixel of the XOR mask contained in the colorPtrAttr field.

//    colorPtrAttr (variable): Encapsulated Color Pointer Update (section
//      2.2.9.1.1.4.4) structure which contains information about the pointer.
//      The Color Pointer Update fields are all used, as specified in section
//      2.2.9.1.1.4.4; however, the XOR mask data alignment packing is slightly
//      different. For monochrome (1 bpp) pointers the XOR data is always padded
//      to a 4-byte boundary per scan line, while color pointer XOR data is
//      still packed on a 2-byte boundary. Color XOR data is presented in the
///     color depth described in the xorBpp field (for 8 bpp, each byte contains
//      one palette index; for 4 bpp, there are two palette indices per byte).

//    2.2.9.1.1.4.6    Cached Pointer Update (TS_CACHEDPOINTERATTRIBUTE)
//    ------------------------------------------------------------------
//    The TS_CACHEDPOINTERATTRIBUTE structure is used to instruct the client to
//    change the current pointer shape to one already present in the pointer
//    cache.

//    cacheIndex (2 bytes): A 16-bit, unsigned integer. A zero-based cache entry
//      containing the cache index of the cached pointer to which the client's
//      pointer should be changed. The pointer data should have already been
//      cached using either the Color Pointer Update (section 2.2.9.1.1.4.4) or
//      New Pointer Update (section 2.2.9.1.1.4.5).

    void server_rdp_set_pointer(int cache_idx) throw (Error)
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(RDP_POINTER_CACHED);
        stream.out_uint16_le(0); /* pad */
        stream.out_uint16_le(cache_idx);

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_POINTER, stream.rdp_hdr - stream.data);
//        LOG(LOG_INFO, "4) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

    void activate_and_process_data(Callback & cb)
    {
        #warning this code needs (yet and again) much clarification

        Stream input_stream(65535);

        do {
            input_stream.init(65535);
            X224In tpdu(this->sec_layer.trans, input_stream);
            int opcode = input_stream.in_uint8();
            int appid = opcode >> 2;

//            LOG(LOG_INFO, "appid = %u", appid);
            /* Channel Join ReQuest datagram */
            #warning this loop should move to sec layer
            while(appid == MCS_CJRQ) {
                /* this is channels getting added from the client */
                int userid = input_stream.in_uint16_be();
                int chanid = input_stream.in_uint16_be();

                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);

                stream.out_uint8((MCS_CJCF << 2) | 2);
                stream.out_uint8(0);
                stream.out_uint16_be(userid);
                stream.out_uint16_be(chanid);
                stream.out_uint16_be(chanid);

                tpdu.end();
                tpdu.send(this->sec_layer.trans);

                input_stream.init(65535);
                X224In(this->sec_layer.trans, input_stream);
                appid = input_stream.in_uint8() >> 2;

            }
            /* Disconnect Provider Ultimatum datagram */
            if (appid == MCS_DPUM) {
                throw Error(ERR_MCS_APPID_IS_MCS_DPUM);
            }
            /* SenD ReQuest datagram */
            if (appid != MCS_SDRQ) {
                throw Error(ERR_MCS_APPID_NOT_MCS_SDRQ);
            }

            input_stream.skip_uint8(2);
            int chan = input_stream.in_uint16_be();
            input_stream.skip_uint8(1);
            int len = input_stream.in_uint8();
            if (len & 0x80) {
                input_stream.skip_uint8(1);
            }

            const uint32_t flags = input_stream.in_uint32_le();
            if (flags & SEC_ENCRYPT) { /* 0x08 */
                input_stream.skip_uint8(8); /* signature */
                this->sec_layer.sec_decrypt(input_stream.p, (int)(input_stream.end - input_stream.p));
            }

            if (flags & SEC_CLIENT_RANDOM) { /* 0x01 */
                input_stream.in_uint32_le(); // len

                memcpy(this->sec_layer.client_crypt_random, input_stream.in_uint8p(64), 64);

                this->sec_layer.server_sec_rsa_op();
                {
                    ssllib ssl;

                    uint8_t pre_master_secret[48];
                    uint8_t master_secret[48];
                    uint8_t key_block[48];

                    /* Construct pre-master secret (session key) */
                    memcpy(key_block, this->sec_layer.client_random, 24);
                    memcpy(key_block + 24, this->sec_layer.server_random, 24);

                    /* Generate master secret and then key material */
                    this->sec_layer.sec_hash_48(master_secret, key_block, this->sec_layer.client_random, this->sec_layer.server_random, 65);
                    this->sec_layer.sec_hash_48(pre_master_secret, master_secret, this->sec_layer.client_random, this->sec_layer.server_random, 88);

                    /* First 16 bytes of key material is MAC secret */
                    memcpy(this->sec_layer.sign_key, pre_master_secret, 16);

                    /* Generate export keys from next two blocks of 16 bytes */
                    this->sec_layer.sec_hash_16(this->sec_layer.encrypt_key, &pre_master_secret[16], this->sec_layer.client_random, this->sec_layer.server_random);
                    this->sec_layer.sec_hash_16(this->sec_layer.decrypt_key, &pre_master_secret[32], this->sec_layer.client_random, this->sec_layer.server_random);

                    if (this->sec_layer.rc4_key_size == 1) {
                        this->sec_layer.sec_make_40bit(this->sec_layer.sign_key);
                        this->sec_layer.sec_make_40bit(this->sec_layer.encrypt_key);
                        this->sec_layer.sec_make_40bit(this->sec_layer.decrypt_key);
                        this->sec_layer.rc4_key_len = 8;
                    } else {
                        this->sec_layer.rc4_key_len = 16;
                    }
                    memcpy(this->sec_layer.decrypt_update_key, this->sec_layer.decrypt_key, 16);
                    memcpy(this->sec_layer.encrypt_update_key, this->sec_layer.encrypt_key, 16);
                    ssl.rc4_set_key(this->sec_layer.decrypt_rc4_info, this->sec_layer.decrypt_key, this->sec_layer.rc4_key_len);
                    ssl.rc4_set_key(this->sec_layer.encrypt_rc4_info, this->sec_layer.encrypt_key, this->sec_layer.rc4_key_len);
                }
                if (!this->up_and_running){
//                    input_stream.next_packet = input_stream.end;
                    continue;
                }
            }
            else if (flags & SEC_LOGON_INFO) { /* 0x40 */
                this->sec_layer.server_sec_process_logon_info(input_stream);
                if (this->sec_layer.client_info->is_mce) {
//                    LOG(LOG_INFO, "server_sec_send media_lic_response");
                    this->sec_layer.server_sec_send_media_lic_response();
                    this->server_rdp_send_demand_active();
                    if (!this->up_and_running){
//                        input_stream.next_packet = input_stream.end;
                        continue;
                    }
                }
                else {
//                    LOG(LOG_INFO, "server_sec_send lic_initial");
                    this->sec_layer.server_sec_send_lic_initial();
                    if (!this->up_and_running){
//                        input_stream.next_packet = input_stream.end;
                        continue;
                    }
                }
            }
            else if (flags & SEC_LICENCE_NEG) { /* 0x80 */
//                LOG(LOG_INFO, "server_sec_send lic_response");
                this->sec_layer.server_sec_send_lic_response();
                this->server_rdp_send_demand_active();
                if (!this->up_and_running){
//                    input_stream.next_packet = input_stream.end;
                    continue;
                }
            }
            else if (chan > MCS_GLOBAL_CHANNEL) {
            #warning is it possible to get channel data when we are not up and running ?
                /*****************************************************************************/
                /* This is called from the secure layer to process an incoming non global
                   channel packet.
                   'chan' passed in here is the mcs channel id so it is
                   MCS_GLOBAL_CHANNEL plus something. */

                /* this assumes that the channels are in order of chanid(mcs channel id)
                   but they should be, see server_sec_process_mcs_data_channels
                   the first channel should be MCS_GLOBAL_CHANNEL + 1, second
                   one should be MCS_GLOBAL_CHANNEL + 2, and so on */
                int channel_id = (chan - MCS_GLOBAL_CHANNEL) - 1;

                struct mcs_channel_item* channel = this->sec_layer.channel_list[channel_id];

                if (channel == 0) {
                    throw Error(ERR_CHANNEL_UNKNOWN_CHANNEL);
                }
                int length = input_stream.in_uint32_le();
                int flags = input_stream.in_uint32_le();

                int size = (int)(input_stream.end - input_stream.p);
                #warning check the long parameter is OK for p here. At start it is a pointer, converting to long is dangerous. See why this should be necessary in callback.
                cb.callback(WM_CHANNELDATA,
                                  ((flags & 0xffff) << 16) | (channel_id & 0xffff),
                                  size, (long)(input_stream.p), length);
//                if (!this->up_and_running){ continue; }
                // We consume all the data of the packet
                input_stream.p = input_stream.end;
            }
//            LOG(LOG_INFO, "PDUTYPE DATA");

            input_stream.next_packet = input_stream.p;

            if (input_stream.next_packet < input_stream.end){
                int length = input_stream.in_uint16_le();
                if (length == 0x8000) {
                    input_stream.next_packet += 8;
                }
                else {
                    int pdu_code = input_stream.in_uint16_le();
                    input_stream.skip_uint8(2); /* mcs user id */
                    input_stream.next_packet += length;
                    switch (pdu_code & 0xf) {

                    case 0:
//                        LOG(LOG_INFO, "PDUTYPE_ZERO");
                        break;
                    case PDUTYPE_DEMANDACTIVEPDU: /* 1 */
//                        LOG(LOG_INFO, "PDUTYPE_DEMANDACTIVEPDU");
                        break;
                    case PDUTYPE_CONFIRMACTIVEPDU:
//                        LOG(LOG_INFO, "PDUTYPE_CONFIRMACTIVEPDU");
                        this->process_confirm_active(input_stream);
                        break;
                    case PDUTYPE_DATAPDU: /* 7 */
//                        LOG(LOG_INFO, "PDUTYPE_DATAPDU");
                        // this is rdp_process_data that will set up_and_running to 1
                        // when fonts have been received
                        // we will not exit this loop until we are in this state.
                        #warning see what happen if we never receive up_and_running due to some error in client code ?
                        this->process_data(input_stream, cb);
//                        LOG(LOG_INFO, "PROCESS_DATA_DONE");
                        break;
                    case PDUTYPE_DEACTIVATEALLPDU:
                        LOG(LOG_WARNING, "unsupported PDU DEACTIVATEALLPDU in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    case PDUTYPE_SERVER_REDIR_PKT:
                        LOG(LOG_WARNING, "unsupported PDU SERVER_REDIR_PKT in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    default:
                        LOG(LOG_WARNING, "unknown PDU type in session_data (%d)\n", pdu_code & 0xf);
                        break;
                    }
                }
            }
//            LOG(LOG_INFO, "READY TO LOOP IN activate and process data");
        } while ((input_stream.next_packet < input_stream.end) || !this->up_and_running);

        #warning the postcondition could be changed to signify we want to get hand back immediately, because we still have data to process.
//        LOG(LOG_INFO, "out of activate and process data");
    }

    /*****************************************************************************/
    void server_rdp_send_data_update_sync() throw (Error)
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(RDP_UPDATE_SYNCHRONIZE);
        stream.out_clear_bytes(2);

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_UPDATE, stream.rdp_hdr - stream.data);
//        LOG(LOG_INFO, "5) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

    void server_rdp_incoming() throw (Error)
    {
        Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);

        memset(this->sec_layer.server_random, 0x44, 32);
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, this->sec_layer.server_random, 32) != 32) {
            }
            close(fd);
        }

        memcpy(this->sec_layer.pub_exp, rsa_keys.pub_exp, 4);
        memcpy(this->sec_layer.pub_mod, rsa_keys.pub_mod, 64);
        memcpy(this->sec_layer.pub_sig, rsa_keys.pub_sig, 64);
        memcpy(this->sec_layer.pri_exp, rsa_keys.pri_exp, 64);

        Stream in(8192);
        X224In crtpdu(this->sec_layer.trans, in);
        if (crtpdu.tpdu_hdr.code != ISO_PDU_CR) {
            throw Error(ERR_ISO_INCOMING_CODE_NOT_PDU_CR);
        }

        Stream out(11);
        X224Out cctpdu(X224Packet::CC_TPDU, out);
        cctpdu.end();
        cctpdu.send(this->sec_layer.trans);

        this->sec_layer.recv_connection_initial(this->sec_layer.client_mcs_data);
        #warning we should fully decode Client MCS Connect Initial PDU with GCC Conference Create Request instead of just calling the function below to extract the fields, that is quite dirty
        this->sec_layer.server_sec_process_mcs_data(this->sec_layer.client_mcs_data);
        this->sec_layer.server_sec_out_mcs_data(this->sec_layer.data);
        this->sec_layer.send_connect_response(this->sec_layer.data, this->sec_layer.trans);

        //   2.2.1.5 Client MCS Erect Domain Request PDU
        //   -------------------------------------------
        //   The MCS Erect Domain Request PDU is an RDP Connection Sequence PDU sent
        //   from client to server during the Channel Connection phase (see section
        //   1.3.1.1). It is sent after receiving the MCS Connect Response PDU (section
        //   2.2.1.4).

        //   tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

        //   x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
        //      section 13.7.

        // See description of tpktHeader and x224 Data TPDU in cheat sheet

        //   mcsEDrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an MCS
        //      Erect Domain Request structure, as specified in [T125] (the ASN.1
        //      structure definitions are given in [T125] section 7, parts 3 and 10).

        {
            Stream stream(8192);
            X224In(this->sec_layer.trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_EDRQ) {
                throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
            }
            stream.skip_uint8(2);
            stream.skip_uint8(2);
            if (opcode & 2) {
                this->sec_layer.userid = stream.in_uint16_be();
            }
            if (!stream.check_end()) {
                throw Error(ERR_MCS_RECV_EDQR_TRUNCATED);
            }
        }


        // 2.2.1.6 Client MCS Attach User Request PDU
        // ------------------------------------------
        // The MCS Attach User Request PDU is an RDP Connection Sequence PDU
        // sent from client to server during the Channel Connection phase (see
        // section 1.3.1.1) to request a user channel ID. It is sent after
        // transmitting the MCS Erect Domain Request PDU (section 2.2.1.5).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //   [X224] section 13.7.

        // See description of tpktHeader and x224 Data TPDU in cheat sheet

        // mcsAUrq (1 byte): PER-encoded MCS Domain PDU which encapsulates an
        //  MCS Attach User Request structure, as specified in [T125] (the ASN.1
        //  structure definitions are given in [T125] section 7, parts 5 and 10).

        {
            Stream stream(8192);
            X224In(this->sec_layer.trans, stream);
            uint8_t opcode = stream.in_uint8();
            if ((opcode >> 2) != MCS_AURQ) {
                throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
            }
            if (opcode & 2) {
                this->sec_layer.userid = stream.in_uint16_be();
            }
            if (!stream.check_end()) {
                throw Error(ERR_MCS_RECV_AURQ_TRUNCATED);
            }
        }

        // 2.2.1.7 Server MCS Attach User Confirm PDU
        // ------------------------------------------
        // The MCS Attach User Confirm PDU is an RDP Connection Sequence
        // PDU sent from server to client during the Channel Connection
        // phase (see section 1.3.1.1). It is sent as a response to the MCS
        // Attach User Request PDU (section 2.2.1.6).

        // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
        //   section 8.

        // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
        //   section [X224] 13.7.

        // mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
        //   an MCS Attach User Confirm structure, as specified in [T125]
        //   (the ASN.1 structure definitions are given in [T125] section 7,
        // parts 5 and 10).

        {
            Stream stream(8192);
            X224Out tpdu(X224Packet::DT_TPDU, stream);
            stream.out_uint8(((MCS_AUCF << 2) | 2));
            stream.out_uint8(0);
            stream.out_uint16_be(this->sec_layer.userid);
            tpdu.end();
            tpdu.send(this->sec_layer.trans);
        }

        {
            {
                Stream stream(8192);
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                X224In(this->sec_layer.trans, stream);

                int opcode = stream.in_uint8();
                if ((opcode >> 2) != MCS_CJRQ) {
                    throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
                }
                stream.skip_uint8(4);
                if (opcode & 2) {
                    stream.skip_uint8(2);
                }
            }

            // 2.2.1.9 Server MCS Channel Join Confirm PDU
            // -------------------------------------------
            // The MCS Channel Join Confirm PDU is an RDP Connection Sequence
            // PDU sent from server to client during the Channel Connection
            // phase (see section 1.3.1.1). It is sent as a response to the MCS
            // Channel Join Request PDU (section 2.2.1.8).

            // tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
            //   section 8.

            // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
            //  [X224] section 13.7.

            // mcsCJcf (8 bytes): PER-encoded MCS Domain PDU which encapsulates
            //  an MCS Channel Join Confirm PDU structure, as specified in
            //  [T125] (the ASN.1 structure definitions are given in [T125]
            //  section 7, parts 6 and 10).

            {
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                stream.out_uint8((MCS_CJCF << 2) | 2);
                stream.out_uint8(0);
                stream.out_uint16_be(this->sec_layer.userid);
                stream.out_uint16_be(this->sec_layer.userid + MCS_USERCHANNEL_BASE);
                stream.out_uint16_be(this->sec_layer.userid + MCS_USERCHANNEL_BASE);
                tpdu.end();
                tpdu.send(this->sec_layer.trans);
            }
        }

        {
            {
                Stream stream(8192);
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)
                X224In(this->sec_layer.trans, stream);

                int opcode = stream.in_uint8();
                if ((opcode >> 2) != MCS_CJRQ) {
                    throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
                }
                stream.skip_uint8(4);
                if (opcode & 2) {
                    stream.skip_uint8(2);
                }
            }

            {
                Stream stream(8192);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                stream.out_uint8((MCS_CJCF << 2) | 2);
                stream.out_uint8(0);
                stream.out_uint16_be(this->sec_layer.userid);
                stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
                stream.out_uint16_be(MCS_GLOBAL_CHANNEL);
                tpdu.end();
                tpdu.send(this->sec_layer.trans);
            }
        }

        this->mcs_channel = this->sec_layer.userid + MCS_USERCHANNEL_BASE;
    }

    /*****************************************************************************/
    void server_rdp_send_demand_active() throw (Error)
    {

//        LOG(LOG_INFO, "server_rdp_send_demand_active()");
        int caps_count;
        int caps_size;
        uint8_t* caps_count_ptr;
        uint8_t* caps_size_ptr;
        uint8_t* caps_ptr;

        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 6;

        caps_count = 0;
        stream.out_uint32_le(this->share_id);
        stream.out_uint16_le(4); /* 4 chars for RDP\0 */


        /* 2 bytes size after num caps, set later */
        caps_size_ptr = stream.p;
        stream.out_clear_bytes(2);
        stream.out_copy_bytes("RDP", 4);

        /* 4 byte num caps, set later */
        caps_count_ptr = stream.p;
        stream.out_clear_bytes(4);
        caps_ptr = stream.p;

        /* Output share capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_SHARE);
        stream.out_uint16_le(RDP_CAPLEN_SHARE);
        stream.out_uint16_le(this->mcs_channel);
        stream.out_uint16_be(0xb5e2); /* 0x73e1 */

        /* Output general capability set */
        caps_count++;
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

        /* Output bitmap capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_BITMAP); /* 2 */
        stream.out_uint16_le(RDP_CAPLEN_BITMAP); /* 28(0x1c) */
        stream.out_uint16_le(this->client_info.bpp); /* Preferred BPP */
        stream.out_uint16_le(1); /* Receive 1 BPP */
        stream.out_uint16_le(1); /* Receive 4 BPP */
        stream.out_uint16_le(1); /* Receive 8 BPP */
        stream.out_uint16_le(this->client_info.width); /* width */
        stream.out_uint16_le(this->client_info.height); /* height */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Allow resize */
        stream.out_uint16_le(1); /* bitmap compression */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* unknown */
        stream.out_uint16_le(0); /* pad */

        /* Output font capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_FONT); /* 14 */
        stream.out_uint16_le(RDP_CAPLEN_FONT); /* 4 */

        /* Output order capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_ORDER); /* 3 */
        stream.out_uint16_le(RDP_CAPLEN_ORDER); /* 88(0x58) */
        stream.out_clear_bytes(16);
        stream.out_uint32_be(0x40420f00);
        stream.out_uint16_le(1); /* Cache X granularity */
        stream.out_uint16_le(20); /* Cache Y granularity */
        stream.out_uint16_le(0); /* Pad */
        stream.out_uint16_le(1); /* Max order level */
        stream.out_uint16_le(0x2f); /* Number of fonts */
        stream.out_uint16_le(0x22); /* Capability flags */
        /* caps */
        stream.out_uint8(1); /* dest blt */
        stream.out_uint8(1); /* pat blt */
        stream.out_uint8(1); /* screen blt */
        stream.out_uint8(1); /* mem blt */
        stream.out_uint8(0); /* tri blt */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* nine grid */
        stream.out_uint8(1); /* line to */
        stream.out_uint8(0); /* multi nine grid */
        stream.out_uint8(1); /* rect */
        stream.out_uint8(0); /* desk save */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* multi dest blt */
        stream.out_uint8(0); /* multi pat blt */
        stream.out_uint8(0); /* multi screen blt */
        stream.out_uint8(0); /* multi rect */
        stream.out_uint8(0); /* fast index */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polygon */
        stream.out_uint8(0); /* polyline */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* fast glyph */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ellipse */
        stream.out_uint8(0); /* ? */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint8(0); /* unused */
        stream.out_uint16_le(0x6a1);
        stream.out_clear_bytes(2); /* ? */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(0x0f4240); /* desk save */
        stream.out_uint32_le(1); /* ? */
        stream.out_uint32_le(0); /* ? */

        /* Output color cache capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_COLCACHE);
        stream.out_uint16_le(RDP_CAPLEN_COLCACHE);
        stream.out_uint16_le(6); /* cache size */
        stream.out_uint16_le(0); /* pad */

        /* Output pointer capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_POINTER);
        stream.out_uint16_le(RDP_CAPLEN_POINTER);
        stream.out_uint16_le(1); /* Colour pointer */
        stream.out_uint16_le(0x19); /* Cache size */
        stream.out_uint16_le(0x19); /* Cache size */

        /* Output input capability set */
        caps_count++;
        stream.out_uint16_le(RDP_CAPSET_INPUT); /* 13(0xd) */
        stream.out_uint16_le(RDP_CAPLEN_INPUT); /* 88(0x58) */
        stream.out_uint8(1);
        stream.out_clear_bytes(83);

        stream.out_clear_bytes(4); /* pad */

        stream.mark_end();

        caps_size = (int)(stream.end - caps_ptr);
        caps_size_ptr[0] = caps_size;
        caps_size_ptr[1] = caps_size >> 8;

        caps_count_ptr[0] = caps_count;
        caps_count_ptr[1] = caps_count >> 8;
        caps_count_ptr[2] = caps_count >> 16;
        caps_count_ptr[3] = caps_count >> 24;

        stream.p = stream.rdp_hdr;
        int len = stream.end - stream.p;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | PDUTYPE_DEMANDACTIVEPDU);
        stream.out_uint16_le(this->mcs_channel);

//        LOG(LOG_INFO, "XX RDP Packet #%u (type=%u)", this->packet_number, PDUTYPE_DEMANDACTIVEPDU);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

        stream.p = stream.end;
        tpdu.end();
        tpdu.send(this->sec_layer.trans);


    }

    /*****************************************************************************/
    void capset_general(Stream & stream, int len)
    {
//        LOG(LOG_INFO, "capset_general");
        stream.skip_uint8(10);
        /* use_compact_packets is pretty much 'use rdp5' */
        this->client_info.use_compact_packets = stream.in_uint16_le();
        if (this->client_info.use_compact_packets){
            LOG(LOG_INFO, "Use compact packets");
        }
        /* op2 is a boolean to use compact bitmap headers in bitmap cache */
        /* set it to same as 'use rdp5' boolean */
        this->client_info.op2 = this->client_info.use_compact_packets;
        if (this->client_info.op2){
            LOG(LOG_INFO, "Use compact headers for cache");
        }
    }

    /*****************************************************************************/
    void capset_order(Stream & stream, int len)
    {

        stream.skip_uint8(20); /* Terminal desc, pad */
        stream.skip_uint8(2); /* Cache X granularity */
        stream.skip_uint8(2); /* Cache Y granularity */
        stream.skip_uint8(2); /* Pad */
        stream.skip_uint8(2); /* Max order level */
        stream.skip_uint8(2); /* Number of fonts */
        stream.skip_uint8(2); /* Capability flags */
        char order_caps[32];
        memcpy(order_caps, stream.in_uint8p(32), 32); /* Orders supported */
        LOG(LOG_INFO, "dest blt-0 %d\n", order_caps[0]);
        LOG(LOG_INFO, "pat blt-1 %d\n", order_caps[1]);
        LOG(LOG_INFO, "screen blt-2 %d\n", order_caps[2]);
        LOG(LOG_INFO, "memblt-3-13 %d %d\n", order_caps[3], order_caps[13]);
        LOG(LOG_INFO, "triblt-4-14 %d %d\n", order_caps[4], order_caps[14]);
        LOG(LOG_INFO, "line-8 %d\n", order_caps[8]);
        LOG(LOG_INFO, "line-9 %d\n", order_caps[9]);
        LOG(LOG_INFO, "rect-10 %d\n", order_caps[10]);
        LOG(LOG_INFO, "desksave-11 %d\n", order_caps[11]);
        LOG(LOG_INFO, "polygon-20 %d\n", order_caps[20]);
        LOG(LOG_INFO, "polygon2-21 %d\n", order_caps[21]);
        LOG(LOG_INFO, "polyline-22 %d\n", order_caps[22]);
        LOG(LOG_INFO, "ellipse-25 %d\n", order_caps[25]);
        LOG(LOG_INFO, "ellipse2-26 %d\n", order_caps[26]);
        LOG(LOG_INFO, "text2-27 %d\n", order_caps[27]);
        LOG(LOG_INFO, "order_caps dump\n");
        stream.skip_uint8(2); /* Text capability flags */
        stream.skip_uint8(6); /* Pad */
        /* desktop cache size, usually 0x38400 */
        this->client_info.desktop_cache = stream.in_uint32_le();;
        LOG(LOG_INFO, "desktop cache size %d\n", this->client_info.desktop_cache);
        stream.skip_uint8(4); /* Unknown */
        stream.skip_uint8(4); /* Unknown */
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache");
        stream.skip_uint8(24);
        this->client_info.cache1_entries = stream.in_uint16_le();
        this->client_info.cache1_size = stream.in_uint16_le();
        this->client_info.cache2_entries = stream.in_uint16_le();
        this->client_info.cache2_size = stream.in_uint16_le();
        this->client_info.cache3_entries = stream.in_uint16_le();
        this->client_info.cache3_size = stream.in_uint16_le();
        LOG(LOG_INFO, "cache1_entries=%d cache1_size=%d "
                      "cache2_entries=%d cache2_size=%d "
                      "cache3_entries=%d cache3_size=%d\n",
            this->client_info.cache1_entries, this->client_info.cache1_size,
            this->client_info.cache2_entries, this->client_info.cache2_size,
            this->client_info.cache3_entries, this->client_info.cache3_size);
    }

    /* store the bitmap cache size in client_info */
    void capset_bmpcache2(Stream & stream, int len)
    {
        LOG(LOG_INFO, "capset_bmpcache2");
        this->client_info.bitmap_cache_version = 2;
        int Bpp = nbbytes(this->client_info.bpp);
        this->client_info.bitmap_cache_persist_enable = stream.in_uint16_le();
        stream.skip_uint8(2); /* number of caches in set, 3 */
        this->client_info.cache1_entries = stream.in_uint32_le();
        this->client_info.cache1_size = 256 * Bpp;
        this->client_info.cache2_entries = stream.in_uint32_le();
        this->client_info.cache2_size = 1024 * Bpp;
        this->client_info.cache3_entries = (stream.in_uint32_le() & 0x7fffffff);
        this->client_info.cache3_size = 4096 * Bpp;
    }

    /* store the number of client cursor cache in client_info */
    void capset_pointercache(Stream & stream, int len)
    {
    }


    void process_confirm_active(Stream & stream)
    {
        stream.skip_uint8(4); /* rdp_shareid */
        stream.skip_uint8(2); /* userid */
        int source_len = stream.in_uint16_le(); /* sizeof RDP_SOURCE */
        // int cap_len = stream.in_uint16_le();
        stream.skip_uint8(2); // skip cap_len
        stream.skip_uint8(source_len);
        int num_caps = stream.in_uint16_le();
        stream.skip_uint8(2); /* pad */

        for (int index = 0; index < num_caps; index++) {
            uint8_t *p = stream.p;
            int type = stream.in_uint16_le();
            int len = stream.in_uint16_le();

            switch (type) {
            case RDP_CAPSET_GENERAL: /* 1 */
                this->capset_general(stream, len);
                break;
            case RDP_CAPSET_BITMAP: /* 2 */
                break;
            case RDP_CAPSET_ORDER: /* 3 */
                this->capset_order(stream, len);
                break;
            case RDP_CAPSET_BMPCACHE: /* 4 */
                this->capset_bmpcache(stream, len);
                break;
            case RDP_CAPSET_CONTROL: /* 5 */
                break;
            case RDP_CAPSET_ACTIVATE: /* 7 */
                break;
            case RDP_CAPSET_POINTER: /* 8 */
                {
                    stream.skip_uint8(2); /* color pointer */
                    int i = stream.in_uint16_le();
                    this->client_info.pointer_cache_entries = std::min(i, 32);
                }
                break;
            case RDP_CAPSET_SHARE: /* 9 */
                break;
            case RDP_CAPSET_COLCACHE: /* 10 */
                break;
            case 12: /* 12 */
                break;
            case 13: /* 13 */
                break;
            case 14: /* 14 */
                break;
            case RDP_CAPSET_BRUSHCACHE: /* 15 */
                this->client_info.brush_cache_code = stream.in_uint32_le();
                break;
            case 16: /* 16 */
                break;
            case 17: /* 17 */
                break;
            case RDP_CAPSET_BMPCACHE2: /* 19 */
                this->capset_bmpcache2(stream, len);
                break;
            case 20: /* 20 */
                break;
            case 21: /* 21 */
                break;
            case 22: /* 22 */
                break;
            case 26: /* 26 */
                break;
            default:
                break;
            }
            stream.p = p + len;
        }
    }

// 2.2.1.19 Server Synchronize PDU
// ===============================

// The Server Synchronize PDU is an RDP Connection Sequence PDU sent from server
// to client during the Connection Finalization phase (see section 1.3.1.1). It
// is sent after receiving the Confirm Active PDU (section 2.2.1.13.2).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU which
//   encapsulates an MCS Send Data Indication structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in section 7, parts 7 and 10 of
//   [T125]). The userData field of the MCS Send Data Indication contains a
//   Security Header and the Synchronize PDU Data (section 2.2.1.14.1).

// securityHeader (variable): Optional security header. If the Encryption Level
//   (sections 5.3.2 and 2.2.1.4.3) selected by the server is greater than
//   ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
//   2.2.1.4.3) selected by the server is greater than ENCRYPTION_METHOD_NONE
//   (0) then this field will contain one of the following headers:

//   - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
//     selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//     ENCRYPTION_LEVEL_LOW (1).

//  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), or ENCRYPTION_LEVEL_HIGH (3).

//  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Level
//    selected by the server (see sections 5.3.2 and 2.2.1.4.3) is
//    ENCRYPTION_LEVEL_FIPS (4).

// If the Encryption Level (sections 5.3.2 and 2.2.1.4.3) selected by the server
// is ENCRYPTION_LEVEL_NONE (0) and the Encryption Method (sections 5.3.2 and
// 2.2.1.4.3) selected by the server is ENCRYPTION_METHOD_NONE (0), then this
// header is not included in the PDU.

// synchronizePduData (22 bytes): The contents of the Synchronize PDU as
// described in section 2.2.1.14.1.

// 2.2.1.14.1 Synchronize PDU Data (TS_SYNCHRONIZE_PDU)
// ====================================================
// The TS_SYNCHRONIZE_PDU structure is a standard T.128 Synchronize PDU (see
// [T128] section 8.6.1).

// shareDataHeader (18 bytes): Share Control Header (section 2.2.8.1.1.1.1)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header MUST be set to PDUTYPE_DATAPDU (7). The
//   pduType2 field of the Share Data Header MUST be set to PDUTYPE2_SYNCHRONIZE
//   (31).

// messageType (2 bytes): A 16-bit, unsigned integer. The message type. This
//   field MUST be set to SYNCMSGTYPE_SYNC (1).

// targetUser (2 bytes): A 16-bit, unsigned integer. The MCS channel ID of the
//   target user.

    void server_rdp_send_synchronize()
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(1); /* messageType */
        stream.out_uint16_le(1002); /* control id */

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_SYNCHRONIZE, stream.rdp_hdr - stream.data);
//        LOG(LOG_INFO, "6) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

// 2.2.1.15.1 Control PDU Data (TS_CONTROL_PDU)
// ============================================

// The TS_CONTROL_PDU structure is a standard T.128 Synchronize PDU (see [T128]
// section 8.12).

// shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//   containing information about the packet. The type subfield of the pduType
//   field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be set to
//   PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST be set
//   to PDUTYPE2_CONTROL (20).

// action (2 bytes): A 16-bit, unsigned integer. The action code.
// 0x0001 CTRLACTION_REQUEST_CONTROL Request control
// 0x0002 CTRLACTION_GRANTED_CONTROL Granted control
// 0x0003 CTRLACTION_DETACH Detach
// 0x0004 CTRLACTION_COOPERATE Cooperate

// grantId (2 bytes): A 16-bit, unsigned integer. The grant identifier.

// controlId (4 bytes): A 32-bit, unsigned integer. The control identifier.

    void server_rdp_send_control(int action)
    {
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;

        stream.out_uint16_le(action);
        stream.out_uint16_le(0); /* userid */
        stream.out_uint32_le(1002); /* control id */

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_CONTROL, stream.rdp_hdr - stream.data);
//        LOG(LOG_INFO, "7) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }



    /*****************************************************************************/
    void server_rdp_send_fontmap() throw (Error)
    {
    static uint8_t g_fontmap[172] = { 0xff, 0x02, 0xb6, 0x00, 0x28, 0x00, 0x00, 0x00,
                                0x27, 0x00, 0x27, 0x00, 0x03, 0x00, 0x04, 0x00,
                                0x00, 0x00, 0x26, 0x00, 0x01, 0x00, 0x1e, 0x00,
                                0x02, 0x00, 0x1f, 0x00, 0x03, 0x00, 0x1d, 0x00,
                                0x04, 0x00, 0x27, 0x00, 0x05, 0x00, 0x0b, 0x00,
                                0x06, 0x00, 0x28, 0x00, 0x08, 0x00, 0x21, 0x00,
                                0x09, 0x00, 0x20, 0x00, 0x0a, 0x00, 0x22, 0x00,
                                0x0b, 0x00, 0x25, 0x00, 0x0c, 0x00, 0x24, 0x00,
                                0x0d, 0x00, 0x23, 0x00, 0x0e, 0x00, 0x19, 0x00,
                                0x0f, 0x00, 0x16, 0x00, 0x10, 0x00, 0x15, 0x00,
                                0x11, 0x00, 0x1c, 0x00, 0x12, 0x00, 0x1b, 0x00,
                                0x13, 0x00, 0x1a, 0x00, 0x14, 0x00, 0x17, 0x00,
                                0x15, 0x00, 0x18, 0x00, 0x16, 0x00, 0x0e, 0x00,
                                0x18, 0x00, 0x0c, 0x00, 0x19, 0x00, 0x0d, 0x00,
                                0x1a, 0x00, 0x12, 0x00, 0x1b, 0x00, 0x14, 0x00,
                                0x1f, 0x00, 0x13, 0x00, 0x20, 0x00, 0x00, 0x00,
                                0x21, 0x00, 0x0a, 0x00, 0x22, 0x00, 0x06, 0x00,
                                0x23, 0x00, 0x07, 0x00, 0x24, 0x00, 0x08, 0x00,
                                0x25, 0x00, 0x09, 0x00, 0x26, 0x00, 0x04, 0x00,
                                0x27, 0x00, 0x03, 0x00, 0x28, 0x00, 0x02, 0x00,
                                0x29, 0x00, 0x01, 0x00, 0x2a, 0x00, 0x05, 0x00,
                                0x2b, 0x00, 0x2a, 0x00
                              };

        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 18;
        stream.out_copy_bytes((char*)g_fontmap, 172);

        stream.mark_end();
        this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_FONTMAP, stream.rdp_hdr - stream.data);

//        LOG(LOG_INFO, "8) RDP Packet #%u", this->packet_number);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);

        tpdu.end();
        tpdu.send(this->sec_layer.trans);

    }

    /* PDUTYPE_DATAPDU */
    void process_data(Stream & stream, Callback & cb) throw (Error)
    {
        stream.skip_uint8(6);
        stream.in_uint16_le(); // len
        int data_type = stream.in_uint8();
        stream.in_uint8(); // ctype
        stream.in_uint16_le(); // clen
        switch (data_type) {
        case PDUTYPE2_POINTER: /* 27(0x1b) */
//            LOG(LOG_INFO, "PDUTYPE2_POINTER");
            break;
        case PDUTYPE2_INPUT: /* 28(0x1c) */
//            LOG(LOG_INFO, "PDUTYPE2_INPUT");
            {
                int num_events = stream.in_uint16_le();
                stream.skip_uint8(2); /* pad */
                for (int index = 0; index < num_events; index++) {
                    int time = stream.in_uint32_le();
                    uint16_t msg_type = stream.in_uint16_le();
                    uint16_t device_flags = stream.in_uint16_le();
                    int16_t param1 = stream.in_sint16_le();
                    int16_t param2 = stream.in_sint16_le();
                    /* msg_type can be
                       RDP_INPUT_SYNCHRONIZE = 0
                       RDP_INPUT_SCANCODE = 4
                       RDP_INPUT_MOUSE = 0x8001 */
                    if (msg_type == 4){
//                        LOG(LOG_INFO, "receive input: time=%u device_flags = %u param1=%u param2=%u\n", time, device_flags, param1, param2);
                    }
                    cb.callback(msg_type, param1, param2, device_flags, time);
                }
            }
            break;
        case PDUTYPE2_CONTROL: /* 20(0x14) */
//            LOG(LOG_INFO, "PDUTYPE2_CONTROL");
            {
                int action = stream.in_uint16_le();
                stream.skip_uint8(2); /* user id */
                stream.skip_uint8(4); /* control id */
                switch (action){
                    case RDP_CTL_REQUEST_CONTROL:
                        this->server_rdp_send_control(RDP_CTL_GRANT_CONTROL);
                    break;
                    case RDP_CTL_COOPERATE:
                        this->server_rdp_send_control(RDP_CTL_COOPERATE);
                    break;
                    default:
                        LOG(LOG_WARNING, "process DATA_PDU_CONTROL unknown action (%d)\n", action);
                }
            }
            break;
        case PDUTYPE2_SYNCHRONIZE:
//            LOG(LOG_INFO, "PDUTYPE2_SYNCHRONIZE");
            this->server_rdp_send_synchronize();
            break;
        case PDUTYPE2_REFRESH_RECT:
//            LOG(LOG_INFO, "PDUTYPE2_REFRESH_RECT");
            {
                /* int op = */ stream.in_uint32_le();
                int left = stream.in_uint16_le();
                int top = stream.in_uint16_le();
                int right = stream.in_uint16_le();
                int bottom = stream.in_uint16_le();
                int cx = (right - left) + 1;
                int cy = (bottom - top) + 1;
                cb.callback(0x4444, left, top, cx, cy);
            }
            break;
        case PDUTYPE2_SUPPRESS_OUTPUT:
//            LOG(LOG_INFO, "PDUTYPE2_SUPPRESS_OUTPUT");
            // PDUTYPE2_SUPPRESS_OUTPUT comes when minimizing a full screen
            // mstsc.exe 2600. I think this is saying the client no longer wants
            // screen updates and it will issue a PDUTYPE2_REFRESH_RECT above
            // to catch up so minimized apps don't take bandwidth
            break;
        case PDUTYPE2_SHUTDOWN_REQUEST:
//            LOG(LOG_INFO, "PDUTYPE2_SHUTDOWN_REQUEST");
            {
                // when this message comes, send a PDUTYPE2_SHUTDOWN_DENIED back
                // so the client is sure the connection is alive and it can ask
                // if user really wants to disconnect */
                Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
                X224Out tpdu(X224Packet::DT_TPDU, stream);
                stream.mcs_hdr = stream.p;
                stream.p += 8;
// -------------------------

                if (this->sec_layer.client_info->crypt_level > 1) {
                    stream.sec_hdr = stream.p;
                    stream.p += 4 + 8;
                }
                else {
                    stream.sec_hdr = stream.p;
                    stream.p += 4;
                }

                stream.rdp_hdr = stream.p;
                stream.p += 18;

                stream.mark_end();
                this->send_rdp_packet(stream, PDUTYPE_DATAPDU, PDUTYPE2_SHUTDOWN_DENIED, stream.rdp_hdr - stream.data);
//                LOG(LOG_INFO, "9) RDP Packet #%u", this->packet_number);
                this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
                tpdu.end();
                tpdu.send(this->sec_layer.trans);

            }
            break;
        case PDUTYPE2_FONTLIST: /* 39(0x27) */
//            LOG(LOG_INFO, "PDUTYPE2_FONTLIST");
            stream.skip_uint8(2); /* num of fonts */
            stream.skip_uint8(2); /* unknown */
            {
                int seq = stream.in_uint16_le();
                /* 419 client sends Seq 1, then 2 */
                /* 2600 clients sends only Seq 3 */
                /* after second font message, we are up and running */
                if (seq == 2 || seq == 3)
                {
                    this->server_rdp_send_fontmap();
                    this->up_and_running = 1;
                    this->server_rdp_send_data_update_sync();
                }
            }
            break;
        default:
            LOG(LOG_WARNING, "unsupported PDUTYPE in process_data %d\n", data_type);
            break;
        }
    }

    void server_rdp_disconnect() throw (Error)
    {
        this->sec_layer.server_sec_disconnect();
    }

    void server_rdp_send_deactive() throw (Error)
    {
        #warning we should create some RDPStream object created on init and sent before destruction
        Stream stream(8192);
// -------------------------
//        McsOut pdu(stream);
        X224Out tpdu(X224Packet::DT_TPDU, stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
// -------------------------

        if (this->sec_layer.client_info->crypt_level > 1) {
            stream.sec_hdr = stream.p;
            stream.p += 4 + 8;
        }
        else {
            stream.sec_hdr = stream.p;
            stream.p += 4;
        }

        stream.rdp_hdr = stream.p;
        stream.p += 6;
        stream.mark_end();

        int len = stream.p - stream.rdp_hdr;
        stream.p = stream.rdp_hdr;
        stream.out_uint16_le(len);
        stream.out_uint16_le(0x10 | PDUTYPE_DEACTIVATEALLPDU);
        stream.out_uint16_le(this->mcs_channel);

//        LOG(LOG_INFO, "RDP Packet #%u (type=%u (PDUTYPE_DEACTIVATEALLPDU))", this->packet_number++, PDUTYPE_DEACTIVATEALLPDU);
        this->sec_layer.server_sec_send(stream, MCS_GLOBAL_CHANNEL);
        tpdu.end();
        tpdu.send(this->sec_layer.trans);
    }
};

#endif
