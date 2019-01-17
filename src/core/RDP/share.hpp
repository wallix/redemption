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


#pragma once

#include <cstdlib>

#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

#include "core/RDP/mppc.hpp"


/* RDP PDU codes */
//  confirmActivePDU(3), dataPDU(7), deactivateAllPDU(6), deactivateOtherPDU(4),
//  deactivateSelfPDU(5), demandActivePDU(1), requestActivePDU(2)}(0..15)

enum {
    PDUTYPE_DEMANDACTIVEPDU        = 1, // demandActivePDU
    PDUTYPE_REQUESTACTIVEPDU       = 2, // requestActivePDU
    PDUTYPE_CONFIRMACTIVEPDU       = 3, // confirmActivePDU
    PDUTYPE_DEACTIVTEOTHERPDU      = 4, // deactivateOtherPDU ?
    PDUTYPE_DEACTIVATESELFPDU      = 5, // deactivateSelfPDU
    PDUTYPE_DEACTIVATEALLPDU       = 6, // deactivateAllPDU
    PDUTYPE_DATAPDU                = 7, // dataPDU
    PDUTYPE_SERVER_REDIR_PKT       = 10,
    FLOWPDU                        = 11 // FlowPDU from T.128
};

// PDUTypeFlow ::= INTEGER {flowResponsePDU(66), flowStopPDU(67), flowTestPDU(65)
// }(0..255)
enum {
    FLOW_RESPONSE_PDU = 66,
    FLOW_STOP_PDU     = 67,
    FLOW_TEST_PDU     = 65
};

enum {
    RDP_CTL_REQUEST_CONTROL        = 1,
    RDP_CTL_GRANT_CONTROL          = 2,
    RDP_CTL_DETACH                 = 3,
    RDP_CTL_COOPERATE              = 4
};

enum {
    RDP_UPDATE_ORDERS              = 0,
    RDP_UPDATE_BITMAP              = 1,
    RDP_UPDATE_PALETTE             = 2,
    RDP_UPDATE_SYNCHRONIZE         = 3
};

enum {
    RDP_NULL_POINTER               = 0,
    RDP_DEFAULT_POINTER            = 0x7F00
};


enum {
    RDP_POINTER_SYSTEM             = 1,
    RDP_POINTER_MOVE               = 3,
    RDP_POINTER_COLOR              = 6,
    RDP_POINTER_CACHED             = 7,
    RDP_POINTER_NEW                = 8
};

// Return True if flowMarker is detected
// In these case we have FlowTestPDU, FlowResponsePDU or FlowStopPDU
// and not ShareControl header.
inline bool peekFlowPDU(const InStream & stream) {
    if (!stream.in_check_rem(2)) {
        throw Error(ERR_SEC);
    }
    return (stream.get_data()[stream.get_offset()] == 0)
    && (stream.get_data()[stream.get_offset()+1] == 0x80);
}


//##############################################################################

// FlowPDU ::= SEQUENCE {
// flowMarker Integer16(32768), -- ('8000'H),
// -- distinguishes FlowPDUs from ASPDUs
// -- containing ShareControlHeaders
// pad8bits Integer8(0),
// pduTypeFlow PDUTypeFlow(flowResponsePDU | flowStopPDU | flowTestPDU),
// flowIdentifier Integer8(0..127),
// flowNumber Integer8,
// -- shall be zero for PDUType FlowStopPDU
// pduSource
// UserID
// -- MCS User ID of sending ASCE
// }


struct ShareFlow_Recv
//##############################################################################
{
    uint16_t flowMarker;
    uint8_t pad;
    uint8_t pduTypeFlow;
    uint8_t flowIdentifier;
    uint8_t flowNumber;
    uint16_t mcs_channel;

    explicit ShareFlow_Recv(InStream & stream)
    : flowMarker([&stream]{
        if (!stream.in_check_rem(2+1+1+1+1+2)){
            LOG(LOG_ERR,
                "Truncated "
                "[2: ShareFlow PDU packet]"
                "[1: ShareFlow pad]"
                "[1: ShareFlow PDU type]"
                "[1: flow Identifier]"
                "[1: flow number]"
                "[2: ShareFlow PDU packet] , remains=%zu", stream.in_remain());
            throw Error(ERR_SEC);
        }
        return stream.in_uint16_le();
    }())
    , pad(stream.in_uint8())
    , pduTypeFlow(stream.in_uint8())
    , flowIdentifier(stream.in_uint8())
    , flowNumber(stream.in_uint8())
    , mcs_channel(stream.in_uint16_le())
    {
        //LOG(LOG_INFO, "Flow control packet %.4x (offset=%zu)", this->flowMarker, stream.get_offset());
        if (this->flowMarker != 0x8000) {
            LOG(LOG_ERR, "Expected flow control packet, got %.4x", this->flowMarker);
            throw Error(ERR_SEC);
        }

        if ((FLOW_TEST_PDU == this->pad) || (FLOW_RESPONSE_PDU == this->pad) || (FLOW_STOP_PDU == this->pad)) {
            // Bug Microsoft?! The fields pad8bits and pduTypeFlow are reversed from their positions described in T.128 documentation.
            uint8_t tmp = this->pduTypeFlow;
            this->pduTypeFlow = this->pad;
            this->pad         = tmp;
        }

        //LOG(LOG_INFO, "PDUTypeFlow=%u", this->pduTypeFlow);
        if (stream.in_remain()) {
            LOG(LOG_INFO, "trailing bytes in FlowPDU, remains %zu bytes", stream.in_remain());
        }
    }
}; // END CLASS ShareFlow_Recv


struct ShareFlow_Send
{
    ShareFlow_Send(OutStream & stream, uint8_t pduTypeFlow, uint8_t flowIdentifier, uint8_t flowNumber, uint16_t pduSource) {
        stream.out_uint16_le(0x8000);   // flowMarker(2)

        // Bug Microsoft?! The fields pad8bits and pduTypeFlow are reversed from their positions described in T.128 documentation.
        stream.out_uint8(pduTypeFlow);
        stream.out_clear_bytes(1);      // pad8bits(1)

        stream.out_uint8(flowIdentifier);
        stream.out_uint8(flowNumber);
        stream.out_uint16_le(pduSource);
    }
};


// [MS-RDPBCGR] 2.2.8.1.1.1.1 Share Control Header (TS_SHARECONTROLHEADER)
// =======================================================================

// The TS_SHARECONTROLHEADER header is a T.128 legacy mode header (see [T128]
// section 8.3) present in slow-path I/O packets.

// T128 8.3 : ASPDU formats
// ~~~~~~~~~~~~~~~~~~~~~~~~
// In the legacy mode of the AS protocol, all ASPDUs, other than the following flow control
// ASPDUs, contain a ShareControl header. Table 8-22 describes the ShareControl header for the
// legacy mode of the AS protocol. In the base mode of the AS protocol, ASPDUs do not contain
// ShareControl headers.

// Not containing ShareControl Header : FlowTestPDU, FlowResponsePDU, FlowStopPDU (See clause 8.5)

// The format below is for ShareControlHeader:

// totalLength (2 bytes): A 16-bit, unsigned integer. The total length of the
//  packet in bytes (the length include the size of the Share Control Header).
// If the totalLength field equals 0x8000, then the Share Control Header and
// any data that follows MAY be interpreted as a T.128 FlowPDU as described in
// [T128] section 8.5 (the ASN.1 structure definition is detailed in [T128]
// section 9.1) and MUST be ignored.

// From T128:
// ~~~~~~~~~~
// This is the total length in octets of the ASPDU within which this header is
// included. This parameter is required as MCS implementations may
// segment ASPDUs in transmission and are not required to reassemble on
// delivery. This parameter allows receiving ASCEs to efficiently perform
// reassembly where MCS segmentation is present.



// pduType (2 bytes): A 16-bit, unsigned integer. It contains the PDU type and
//   protocol version information. The format of the pduType word is described
//   by the following bitmask diagram:

// pduType::type (2 bits): Least significant 4 bits of the least significant byte.

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
// |                             | (see [MS-RDPEGDI] section 2.2.13.3.1).      |
// +-----------------------------+---------------------------------------------+

// pduType::versionLow (4 bits): Most significant 4 bits of the least significant byte.
//   This field MUST be set to TS_PROTOCOL_VERSION (0x1).

// pduType::versionHigh (1 byte): Most significant byte. This field MUST be set to 0x00.

// PDUSource (2 bytes): A 16-bit, unsigned integer. The channel ID which is the
//   transmission source of the PDU.

struct ShareControl_Recv
{
    public:
    uint16_t totalLength;
    uint16_t pduType;
    uint16_t PDUSource;
    InStream payload;

    explicit ShareControl_Recv(InStream & stream)
    : totalLength([&stream]() {
        if (!stream.in_check_rem(2+2)){
            LOG(LOG_ERR,
                "Truncated [4: ShareControl packet] , remains=%zu", stream.in_remain());
            throw Error(ERR_SEC);
        }
        return stream.in_uint16_le();
    }())
    , pduType(stream.in_uint16_le() & 0xF)
    , PDUSource([&stream, this]() {
        if (this->pduType == PDUTYPE_DEACTIVATEALLPDU && this->totalLength == 4) {
            // should not happen
            // but DEACTIVATEALLPDU seems to be broken on windows 2000
            return static_cast<uint16_t>(0);
        }
        return stream.in_uint16_le();
    }())
    , payload([&stream, this]() {
        if (this->pduType == PDUTYPE_DEACTIVATEALLPDU && this->totalLength == 4) {
            // should not happen
            // but DEACTIVATEALLPDU seems to be broken on windows 2000
            return InStream(stream.get_current(), 0);
        }

        if (this->totalLength < 6) {
            LOG(LOG_ERR, "ShareControl packet too short totalLength=%u pduType=%u mcs_channel=%u",
                this->totalLength, this->pduType, this->PDUSource);
            throw Error(ERR_SEC);
        }

        if (!stream.in_check_rem(this->totalLength - 6)) {
            LOG(LOG_ERR, "Truncated ShareControl packet, need=%u remains=%zu",
                this->totalLength - 6u,
                stream.in_remain());
            throw Error(ERR_SEC);
        }
        return InStream(stream.get_current(), this->totalLength - 6);
    }())
    // body of constructor
    {
        if (this->totalLength == 0x8000) {
            LOG(LOG_ERR, "Expected ShareControl header, got flowMarker");
            throw Error(ERR_SEC);
        }
        stream.in_skip_bytes(this->payload.get_capacity());
    }

    inline void log() const {
        LOG(LOG_INFO, "ShareControl_Recv: \n"
                      "     * totalLength = %d (2 bytes)\n"
                      "     * pduType = %d (2 bytes)\n"
                      "     * PDUSource = %d (2 bytes)\n"
                      "     * payload (%zu byte(s))\n",
                      int(totalLength), int(pduType), int(PDUSource), this->payload.in_remain());
    }

}; // END CLASS ShareControl_Recv

//##############################################################################
struct ShareControl_Send
//##############################################################################
{
    ShareControl_Send(OutStream & stream, uint8_t pduType, uint16_t PDUSource, uint16_t payload_len)
    {
        enum {
            versionLow = 0x10,
            versionHigh = 0
        };
        stream.out_uint16_le(payload_len + 6);
        stream.out_uint16_le(versionHigh | versionLow | pduType);
        stream.out_uint16_le(PDUSource);
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
} // namespace RDP

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
// | 54 PDUTYPE2_STATUS_INFO_PDU       | Status Info PDU (section 2.2.5.2)     |
// +-----------------------------------+---------------------------------------+
// | 55 PDUTYPE2_MONITOR_LAYOUT_PDU    | Monitor Layout PDU (section 2.2.12.1) |
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
    PDUTYPE2_STATUS_INFO_PDU             = 54, // Status Info PDU (section 2.2.5.2)
    PDUTYPE2_MONITOR_LAYOUT_PDU          = 55  // Monitor Layout PDU (section 2.2.12.1)
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


// Inheritance is only used to check if we have enough data available
struct CheckShareData_Recv {
    explicit CheckShareData_Recv(const InStream & stream) {
        // share_id(4)
        // + ignored(1)
        // + streamid(1)
        // + len(2)
        // + pdutype2(1)
        // + compressedType(1)
        // + compressedLen(2)
        const unsigned expected = 12;
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "sdata packet len too short: need %u, remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_SEC);
        }
    }
};

struct ShareData_Recv : private CheckShareData_Recv
{
    public:
    uint32_t share_id;
    uint8_t pad1;
    uint8_t streamid;
    uint16_t len;
    uint8_t pdutype2;
    uint8_t compressedType;
    uint16_t compressedLen;
    InStream payload;


    explicit ShareData_Recv(InStream & stream, rdp_mppc_dec * dec = nullptr)
    //==============================================================================
    : CheckShareData_Recv(stream)
    , share_id(stream.in_uint32_le())
    , pad1(stream.in_uint8())
    , streamid(stream.in_uint8())
    , len(stream.in_uint16_le())
    , pdutype2(stream.in_uint8())
    , compressedType(stream.in_uint8())
    , compressedLen(stream.in_uint16_le())
    , payload([&stream, dec, this]() {
        if (this->compressedType & PACKET_COMPRESSED) {
            if (!dec) {
                LOG(LOG_INFO, "ShareData_Recv: got unexpected compressed share data");
                throw Error(ERR_SEC);
            }

            const uint8_t * rdata;
            uint32_t        rlen;

            dec->decompress(stream.get_current(), stream.in_remain(),
                this->compressedType, rdata, rlen);

            return InStream(rdata, rlen);
        }
        return InStream(stream.get_current(), stream.in_remain());
      }())
    // BEGIN CONSTRUCTOR
    {
        //LOG( LOG_INFO, "ShareData_Recv: pdutype2=%u len=%u compressedLen=%u payload_size=%u"
        //   , this->pdutype2, this->len, this->compressedLen, this->payload.size());
        stream.in_skip_bytes(stream.in_remain());
    } // END CONSTRUCTOR

    inline void log() const {
        LOG(LOG_INFO, "ShareData_Recv: \n"
                      "     * shareId = %d (4 bytes)\n"
                      "     * pad1 (1 byte) \n"
                      "     * streamId = %d (1 byte)\n"
                      "     * uncompressedLength = %d (2 bytes)\n"
                      "     * pduType2 = %d (1 byte)\n"
                      "     * compressedType = %d (1 byte)\n"
                      "     * compressedLength = %d (2 bytes)\n"
                      "     * payload (%zu byte(s))\n",
                      int(share_id), int(streamid), int(len), int(pdutype2), int(compressedType), int(compressedLen)
                      , payload.in_remain());
    }

    ~ShareData_Recv() noexcept(false) {
        if (!this->payload.check_end()) {
            LOG( LOG_INFO
               , "~ShareData_Recv: some payload data were not consumed len=%u compressedLen=%u remains=%zu"
               , this->len, this->compressedLen, payload.in_remain());
            throw Error(ERR_SEC);
        }
    }
}; // END CLASS ShareData_Recv

//##############################################################################
struct ShareData
//##############################################################################
{
    OutStream & stream;

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
    explicit ShareData(OutStream & stream)
    //==============================================================================
    : stream(stream)
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
                   , uint16_t _uncompressedLen = 0
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
            assert(compressedType == 0);
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
            stream.set_out_uint16_le(  stream.get_offset()
                                     + 6,                   // TS_SHARECONTROLHEADER(6)
                                     6);
        }
    } // END METHOD emit_end
}; // END CLASS ShareData

//##############################################################################
struct FlowPDU_Send
//##############################################################################
{
    FlowPDU_Send(OutStream & stream, uint8_t flow_pdu_type, uint8_t flow_identifier,
                 uint8_t flow_number, uint16_t pdu_source)
    {
        stream.out_uint16_le(0x8000);
        stream.out_uint8(flow_pdu_type);
        stream.out_uint8(0);
        stream.out_uint8(flow_identifier);
        stream.out_uint8(flow_number);
        stream.out_uint16_le(pdu_source);
    }
}; // END CLASS FlowPDU_Send
// FlowPDU ::= SEQUENCE {
// flowMarker
// Integer16(32768), -- ('8000'H),
// -- distinguishes FlowPDUs from ASPDUs
// -- containing ShareControlHeaders
// pad8bits  Integer8(0),
// pduTypeFlow  PDUTypeFlow(flowResponsePDU | flowStopPDU | flowTestPDU),
// flowIdentifier Integer8(0..127),
// flowNumber Integer8,
// -- shall be zero for PDUType FlowStopPDU
// pduSource UserID
// -- MCS User ID of sending ASCE
// }

