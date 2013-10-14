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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rdp module main header file
*/

#ifndef _REDEMPTION_CORE_RDP_SHARE_HPP_
#define _REDEMPTION_CORE_RDP_SHARE_HPP_

#include <stdlib.h>

#include "log.hpp"
#include "stream.hpp"

#include "RDP/mppc.hpp"


/* RDP PDU codes */
enum {
    PDUTYPE_DEMANDACTIVEPDU        = 1,
    PDUTYPE_CONFIRMACTIVEPDU       = 3,
    RDP_PDU_REDIRECT               = 4, // This one is not documented...
    PDUTYPE_DEACTIVATEALLPDU       = 6,
    PDUTYPE_DATAPDU                = 7,
    PDUTYPE_SERVER_REDIR_PKT       = 10,
};

enum {
    RDP_CTL_REQUEST_CONTROL        = 1,
    RDP_CTL_GRANT_CONTROL          = 2,
    RDP_CTL_DETACH                 = 3,
    RDP_CTL_COOPERATE              = 4,
};

enum {
    RDP_UPDATE_ORDERS              = 0,
    RDP_UPDATE_BITMAP              = 1,
    RDP_UPDATE_PALETTE             = 2,
    RDP_UPDATE_SYNCHRONIZE         = 3,
};

enum {
    RDP_NULL_POINTER               = 0,
    RDP_DEFAULT_POINTER            = 0x7F00,
};

// [MS-RDPBCGR] 2.2.8.1.1.1.1 Share Control Header (TS_SHARECONTROLHEADER)
// =======================================================================

// The TS_SHARECONTROLHEADER header is a T.128 legacy mode header (see [T128]
// section 8.3) present in slow-path I/O packets.

// totalLength (2 bytes): A 16-bit, unsigned integer. The total length of the
//   packet in bytes (the length include " the size of the Share Control Header).

// pduType (2 bytes): A 16-bit, unsigned integer. It contains the PDU type and
//   protocol version information. The format of the pduType word is described
//   by the following bitmask diagram:

// pduType::type (4 bits): Least significant 4 bits of the least significant byte.

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

// pduType::versionLow (4 bits): Most significant 4 bits of the least significant byte.
//   This field MUST be set to TS_PROTOCOL_VERSION (0x1).

// pduType::versionHigh (1 byte): Most significant byte. This field MUST be set to 0x00.

// PDUSource (2 bytes): A 16-bit, unsigned integer. The channel ID which is the
//   transmission source of the PDU.

enum {
    RDP_POINTER_SYSTEM             = 1,
    RDP_POINTER_MOVE               = 3,
    RDP_POINTER_COLOR              = 6,
    RDP_POINTER_CACHED             = 7,
    RDP_POINTER_NEW                = 8,
};

//##############################################################################
struct ShareControl_Recv
//##############################################################################
{
    SubStream payload;
    public:
    uint16_t totalLength;
    TODO("rename to pduType");
    uint8_t pdu_type1;
    TODO("Rename to PDUSource");
    uint16_t mcs_channel;

    ShareControl_Recv(Stream & stream)
    : payload(stream, 0)
    , totalLength(0)
    , pdu_type1(0)
    , mcs_channel(0)
    {
        const unsigned expected = 4; /* totalLength(2) + pdu_type1(2) */
        if (!stream.in_check_rem(expected)){
            LOG(LOG_ERR, "Truncated ShareControl packet, need=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_SEC);
        }

        this->totalLength = stream.in_uint16_le();

        this->pdu_type1 = stream.in_uint16_le() & 0xF;
        if (this->pdu_type1 == PDUTYPE_DEACTIVATEALLPDU && this->totalLength == 4){
            // should not happen
            // but DEACTIVATEALLPDU seems to be broken on windows 2000
            this->payload.resize(stream, 0);
            return;
        }

        if (!stream.in_check_rem(2)){
            LOG(LOG_ERR, "Truncated ShareControl packet mcs_channel, need=2 remains=%u",
                stream.in_remain());
            throw Error(ERR_SEC);
        }

        this->mcs_channel = stream.in_uint16_le();
        if (this->totalLength < 6){
            LOG(LOG_ERR, "ShareControl packet too short totalLength=%u", this->totalLength);
            throw Error(ERR_SEC);
        }

        size_t new_size = this->totalLength - 6;

        if (!stream.in_check_rem(new_size)){
            LOG(LOG_ERR, "Truncated ShareControl packet mcs_channel, need=2 remains=%u",
                stream.in_remain());
            throw Error(ERR_SEC);
        }

        this->payload.resize(stream, new_size);
    }
}; // END CLASS ShareControl_Recv

//##############################################################################
struct ShareControl_Send
//##############################################################################
{
    ShareControl_Send(Stream & stream, uint8_t pdu_type1, uint16_t mcs_channel, uint16_t payload_len)
    {
        enum {
            versionLow = 0x10,
            versionHigh = 0
        };
        stream.out_uint16_le(payload_len + 6);
        stream.out_uint16_le(versionHigh | versionLow | pdu_type1);
        stream.out_uint16_le(mcs_channel);
        stream.mark_end();
    }
}; // END CLASS ShareControl_Send

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

namespace RDP {
    enum {
        STREAM_UNDEFINED = 0,
        STREAM_LOW = 1,
        STREAM_MED = 2,
        STREAM_HI = 4
    };
};

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

enum {
    PDUTYPE2_UPDATE                      = 2,  // Update PDU (section 2.2.9.1.1.3)
    PDUTYPE2_CONTROL                     = 20, // Control PDU (section 2.2.1.15.1)
    PDUTYPE2_POINTER                     = 27, // Pointer Update PDU (section 2.2.9.1.1.4)
    PDUTYPE2_INPUT                       = 28, // Input PDU (section 2.2.8.1.1.3)
    PDUTYPE2_SYNCHRONIZE                 = 31, // Synchronize PDU (section 2.2.1.14.1)
    PDUTYPE2_REFRESH_RECT                = 33, // Refresh Rect PDU (section 2.2.11.2.1)
    PDUTYPE2_PLAY_SOUND                  = 34, // Play Sound PDU (section 2.2.9.1.1.5.1)
    PDUTYPE2_SUPPRESS_OUTPUT             = 35, // Suppress Output PDU (section 2.2.11.3.1)
    PDUTYPE2_SHUTDOWN_REQUEST            = 36, // Shutdown Request PDU (section 2.2.2.2.1)
    PDUTYPE2_SHUTDOWN_DENIED             = 37, // Shutdown Request Denied PDU
                                               // (section 2.2.2.3.1)
    PDUTYPE2_SAVE_SESSION_INFO           = 38, // Save Session Info PDU
                                               // (section 2.2.10.1.1)
    PDUTYPE2_FONTLIST                    = 39, // Font List PDU (section 2.2.1.18.1)
    PDUTYPE2_FONTMAP                     = 40, // Font Map PDU (section 2.2.1.22.1)
    PDUTYPE2_SET_KEYBOARD_INDICATORS     = 41, // Set Keyboard Indicators PDU
                                               //   (section 2.2.8.2.1.1)
    PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST = 43, // Persistent Key List PDU
                                               // (section 2.2.1.17.1)
    PDUTYPE2_BITMAPCACHE_ERROR_PDU       = 44, // Bitmap Cache Error PDU (see
                                               // [MS-RDPEGDI] section 2.2.2.3.1)
    PDUTYPE2_SET_KEYBOARD_IME_STATUS     = 45, // Set Keyboard IME Status PDU
                                               // (section 2.2.8.2.2.1)
    PDUTYPE2_OFFSCRCACHE_ERROR_PDU       = 46, // Offscreen Bitmap Cache Error PDU
                                               // (see [MS-RDPEGDI] section 2.2.2.3.2)
    PDUTYPE2_SET_ERROR_INFO_PDU          = 47, // Set Error Info PDU (section 2.2.5.1.1)
    PDUTYPE2_DRAWNINEGRID_ERROR_PDU      = 48, // DrawNineGrid Cache Error PDU
                                               // (see [MS-RDPEGDI] section 2.2.2.3.3)
    PDUTYPE2_DRAWGDIPLUS_ERROR_PDU       = 49, // GDI+ Error PDU
                                               // (see [MS-RDPEGDI] section 2.2.2.3.4)
    PDUTYPE2_ARC_STATUS_PDU              = 50, // Auto-Reconnect Status PDU
                                               // (section 2.2.4.1.1)
};

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

//##############################################################################
struct ShareData
//##############################################################################
{
    Stream & stream;
    SubStream payload;

    public:
    uint32_t share_id;
    uint8_t streamid;
    uint16_t len;
    uint8_t pdutype2;
    uint16_t uncompressedLen;
    uint8_t compressedType;
    uint16_t compressedLen;

    // CONSTRUCTOR
    //==============================================================================
    ShareData(Stream & stream)
    //==============================================================================
    : stream(stream)
    , payload(this->stream, 0)
    , share_id(0)
    , streamid(0)
    , len(0)
    , pdutype2(0)
    , uncompressedLen(0)
    , compressedType(0)
    , compressedLen(0)
    {
    } // END CONSTRUCTOR

    //==============================================================================
    void emit_begin( uint8_t pdu_type2
                   , uint32_t share_id
                   , uint8_t streamid
                   , uint8_t _uncompressedLen = 0
                   , uint8_t compressedType = 0
                   , uint16_t compressedLen = 0
                   )
    //==============================================================================
    {
        stream.out_uint32_le(share_id);
        stream.out_uint8(0); // pad1
        stream.out_uint8(streamid); // streamid
        this->uncompressedLen = _uncompressedLen;
        if (!_uncompressedLen) {
            stream.out_clear_bytes(2); // skip len
        }
        else {
            stream.out_uint16_le(_uncompressedLen);
        }
        stream.out_uint8(pdu_type2); // pdutype2
        stream.out_uint8(compressedType); // compressedType
        stream.out_uint16_le(compressedLen); // compressedLen
    } // END METHOD emit_begin

    //==============================================================================
    void emit_end()
    //==============================================================================
    {
        if (!this->uncompressedLen) {
            stream.set_out_uint16_le(stream.get_offset() - 8, 6);
        }
        stream.mark_end();
    } // END METHOD emit_end

    //==============================================================================
    void recv_begin(rdp_mppc_dec * dec = 0)
    //==============================================================================
    {
        /* share_id(4) + ignored(1) + streamid(1) + len(2) + pdutype2(1) + compressedType(1) + compressedLen(2) */
        const unsigned expected = 12;
        if (!this->stream.in_check_rem(expected)){
            LOG(LOG_ERR, "sdata packet len too short: need %u, remains=%u",
                expected, this->stream.in_remain());
            throw Error(ERR_SEC);
        }

        this->share_id = stream.in_uint32_le();
        stream.in_uint8();
        this->streamid = stream.in_uint8();
        this->len = stream.in_uint16_le();
        this->pdutype2 = stream.in_uint8();
        this->compressedType = stream.in_uint8();
        this->compressedLen = stream.in_uint16_le();

        this->payload.resize(this->stream, this->stream.in_remain());

        if (this->compressedType & PACKET_COMPRESSED) {
            if (!dec) {
                LOG(LOG_INFO, "ShareData::recv_begin: got unexpected compressed share data");
            }

/*
            uint32_t roff = 0;
            uint32_t rlen = 0;
            dec->decompress_rdp(this->payload.get_data(), this->payload.size(),
                this->compressedType, &roff, &rlen);

            this->payload.resize(StaticStream(dec->history_buf + roff, rlen), rlen);
*/
            const uint8_t * rdata;
            uint32_t        rlen;

            dec->decompress(this->payload.get_data(), this->payload.size(),
                this->compressedType, rdata, rlen);

            this->payload.resize(StaticStream(rdata, rlen), rlen);
        }
    } // END METHOD recv_begin

    //==============================================================================
    void recv_end() {
    //==============================================================================
        if (!this->payload.check_end()) {
            LOG(LOG_INFO, "ShareData : some payload data were not consumed len=%u compressedLen=%u remains1=%u remains=%u",
                this->len, this->compressedLen, payload.in_remain(), stream.in_remain());
                throw Error(ERR_SEC);
        }
    } // END METHOD recv_end
}; // END CLASS ShareData

#endif
