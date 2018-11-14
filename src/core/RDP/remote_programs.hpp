/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "core/error.hpp"
#include "core/RDP/non_null_terminated_utf16_from_utf8.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/cast.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"

// [MS-RDPERP] - 2.2.2.1 Common Header (TS_RAIL_PDU_HEADER)
// ========================================================

// The TS_RAIL_PDU_HEADER packet contains information common to every RAIL
//  Virtual Channel PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           orderType           |          orderLength          |
// +-------------------------------+-------------------------------+

// orderType (2 bytes): An unsigned 16-bit integer. The type of the Virtual
//  Channel message; MUST be set to one of the following values.

//  +-------------------------------+------------------------------------------+
//  | Value                         | Meaning                                  |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_EXEC            | Indicates a Client Execute PDU from      |
//  | 0x0001                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_ACTIVATE        | Indicates a Client Activate PDU from     |
//  | 0x0002                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_SYSPARAM        | Indicates a Client System Parameters     |
//  | 0x0003                        | Update PDU from client to server or a    |
//  |                               | Server System Parameters Update PDU from |
//  |                               | server to client.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_SYSCOMMAND      | Indicates a Client System Command PDU    |
//  | 0x0004                        | from client to server.                   |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_HANDSHAKE       | Indicates a bi-directional Handshake     |
//  | 0x0005                        | PDU.                                     |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_NOTIFY_EVENT    | Indicates a Client Notify Event PDU from |
//  | 0x0006                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_WINDOWMOVE      | Indicates a Client Window Move PDU from  |
//  | 0x0008                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_LOCALMOVESIZE   | Indicates a Server Move/Size Start PDU   |
//  | 0x0009                        | and a Server Move/Size End PDU from      |
//  |                               | server to client.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_MINMAXINFO      | Indicates a Server Min Max Info PDU from |
//  | 0x000a                        | server to client.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_CLIENTSTATUS    | Indicates a Client Information PDU from  |
//  | 0x000b                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_SYSMENU         | Indicates a Client System Menu PDU from  |
//  | 0x000c                        | client to server.                        |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_LANGBARINFO     | Indicates a Server Language Bar          |
//  | 0x000d                        | Information PDU from server to client,   |
//  |                               | or a Client Language Bar Information PDU |
//  |                               | from client to server.                   |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_EXEC_RESULT     | Indicates a Server Execute Result PDU    |
//  | 0x0080                        | from server to client.                   |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_GET_APPID_REQ   | Indicates a Client Get Application ID    |
//  | 0x000E                        | PDU from client to server.               |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_GET_APPID_RESP  | Indicates a Server Get Application ID    |
//  | 0x000F                        | Response PDU from server to client.      |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_LANGUAGEIMEINFO | Indicates a Language Profile Information |
//  | 0x0011                        | PDU from client to server.               |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_COMPARTMENTINFO | Indicates a bi-directional Compartment   |
//  | 0x0012                        | Status Information PDU.                  |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_HANDSHAKE_EX    | Indicates a bi-directional HandshakeEx   |
//  | 0x0013                        | PDU.                                     |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_ZORDER_SYNC     | Indicates a Server Z-Order Sync          |
//  | 0x0014                        | Information PDU from server to client.   |
//  +-------------------------------+------------------------------------------+
//  | TS_RAIL_ORDER_CLOAK           | Indicates a Window Cloak State Change    |
//  | 0x0015                        | PDU from client to server.               |
//  +-------------------------------+------------------------------------------+

enum {
      TS_RAIL_ORDER_EXEC            = 0x0001
    , TS_RAIL_ORDER_ACTIVATE        = 0x0002
    , TS_RAIL_ORDER_SYSPARAM        = 0x0003
    , TS_RAIL_ORDER_SYSCOMMAND      = 0x0004
    , TS_RAIL_ORDER_HANDSHAKE       = 0x0005
    , TS_RAIL_ORDER_NOTIFY_EVENT    = 0x0006
    , TS_RAIL_ORDER_WINDOWMOVE      = 0x0008
    , TS_RAIL_ORDER_LOCALMOVESIZE   = 0x0009
    , TS_RAIL_ORDER_MINMAXINFO      = 0x000a
    , TS_RAIL_ORDER_CLIENTSTATUS    = 0x000b
    , TS_RAIL_ORDER_SYSMENU         = 0x000c
    , TS_RAIL_ORDER_LANGBARINFO     = 0x000d
    , TS_RAIL_ORDER_EXEC_RESULT     = 0x0080
    , TS_RAIL_ORDER_GET_APPID_REQ   = 0x000E
    , TS_RAIL_ORDER_GET_APPID_RESP  = 0x000F
    , TS_RAIL_ORDER_LANGUAGEIMEINFO = 0x0011
    , TS_RAIL_ORDER_COMPARTMENTINFO = 0x0012
    , TS_RAIL_ORDER_HANDSHAKE_EX    = 0x0013
    , TS_RAIL_ORDER_ZORDER_SYNC     = 0x0014
    , TS_RAIL_ORDER_CLOAK           = 0x0015
};

static inline
const char * get_RAIL_orderType_name(uint16_t orderType) {
    switch (orderType) {
        case TS_RAIL_ORDER_EXEC:            return "TS_RAIL_ORDER_EXEC";
        case TS_RAIL_ORDER_ACTIVATE:        return "TS_RAIL_ORDER_ACTIVATE";
        case TS_RAIL_ORDER_SYSPARAM:        return "TS_RAIL_ORDER_SYSPARAM";
        case TS_RAIL_ORDER_SYSCOMMAND:      return "TS_RAIL_ORDER_SYSCOMMAND";
        case TS_RAIL_ORDER_HANDSHAKE:       return "TS_RAIL_ORDER_HANDSHAKE";
        case TS_RAIL_ORDER_NOTIFY_EVENT:    return "TS_RAIL_ORDER_NOTIFY_EVENT";
        case TS_RAIL_ORDER_WINDOWMOVE:      return "TS_RAIL_ORDER_WINDOWMOVE";
        case TS_RAIL_ORDER_LOCALMOVESIZE:   return "TS_RAIL_ORDER_LOCALMOVESIZE";
        case TS_RAIL_ORDER_MINMAXINFO:      return "TS_RAIL_ORDER_MINMAXINFO";
        case TS_RAIL_ORDER_CLIENTSTATUS:    return "TS_RAIL_ORDER_CLIENTSTATUS";
        case TS_RAIL_ORDER_SYSMENU:         return "TS_RAIL_ORDER_SYSMENU";
        case TS_RAIL_ORDER_LANGBARINFO:     return "TS_RAIL_ORDER_LANGBARINFO";
        case TS_RAIL_ORDER_EXEC_RESULT:     return "TS_RAIL_ORDER_EXEC_RESULT";
        case TS_RAIL_ORDER_GET_APPID_REQ:   return "TS_RAIL_ORDER_GET_APPID_REQ";
        case TS_RAIL_ORDER_GET_APPID_RESP:  return "TS_RAIL_ORDER_GET_APPID_RESP";
        case TS_RAIL_ORDER_LANGUAGEIMEINFO: return "TS_RAIL_ORDER_LANGUAGEIMEINFO";
        case TS_RAIL_ORDER_COMPARTMENTINFO: return "TS_RAIL_ORDER_COMPARTMENTINFO";
        case TS_RAIL_ORDER_HANDSHAKE_EX:    return "TS_RAIL_ORDER_HANDSHAKE_EX";
        case TS_RAIL_ORDER_ZORDER_SYNC:     return "TS_RAIL_ORDER_ZORDER_SYNC";
        case TS_RAIL_ORDER_CLOAK:           return "TS_RAIL_ORDER_CLOAK";
        default:                            return "<unknown>";
    }
}

// orderLength (2 bytes): An unsigned 16-bit integer. The length of the
//  Virtual Channel PDU, in bytes.

class RAILPDUHeader {
            uint16_t orderType_   = 0;
    mutable uint16_t orderLength_ = 0;

    mutable uint32_t   offset_of_orderLength = 0;
    mutable OutStream* output_stream = nullptr;

public:
    void emit_begin(OutStream & stream, uint16_t orderType) const {
        assert(this->output_stream == nullptr);

        this->output_stream = &stream;

        this->output_stream->out_uint16_le(orderType);

        this->offset_of_orderLength = this->output_stream->get_offset();
        this->output_stream->out_clear_bytes(2);
    }

    void emit_end() const {
        this->output_stream->set_out_uint16_le(
            this->output_stream->get_offset() - this->offset_of_orderLength +
                2 /* orderType(2) */,
            this->offset_of_orderLength);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;    // orderType(2) + orderLength(2)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Truncated RAIL PDU header: expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        this->orderType_   = stream.in_uint16_le();
        this->orderLength_ = stream.in_uint16_le();
    }

    uint16_t orderType() const { return this->orderType_; }

    uint16_t orderLength() const { return this->orderLength_; }

    static size_t size() {
        return 4;   // orderType(2) + orderLength(2)
    }

    size_t str(char * buffer, size_t size) const {
        const size_t length =
            ::snprintf(buffer, size,
                       "RAILPDUHeader=(orderType_=%s(%u) orderLength=%u)",
                       ::get_RAIL_orderType_name(this->orderType_), this->orderType_,
                       this->orderLength_);
        return ((length < size) ? length : size - 1);
    }
};

// [MS-RDPERP] - 2.2.2.2.1 Handshake PDU (TS_RAIL_ORDER_HANDSHAKE)
// ===============================================================

// The Handshake PDU is exchanged between the server and the client to
//  establish that both endpoints are ready to begin RAIL mode. The server
//  sends the Handshake PDU and the client responds with the Handshake PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +---------------------------------------------------------------+
// |                          buildNumber                          |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  the header MUST be set to 0x0005 (TS_RAIL_ORDER_HANDSHAKE).

// buildNumber (4 bytes): An unsigned 32-bit integer. The build or version of
//  the sending party.

class HandshakePDU {
    uint32_t buildNumber_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->buildNumber_);
    }

    void receive(InStream & stream) {
        const unsigned expected = 4;    // buildNumber(4)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Handshake PDU: expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        this->buildNumber_ = stream.in_uint32_le();
    }

    uint32_t buildNumber() const { return this->buildNumber_; }

    void buildNumber(uint32_t buildNumber_) {
        this->buildNumber_ = buildNumber_;
    }

    static size_t size() {
        return 4;   // buildNumber(4)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "HandshakePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "buildNumber=%u", this->buildNumber_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.2.2 Client Information PDU
//  (TS_RAIL_ORDER_CLIENTSTATUS)
// ==============================================

// The Client Information PDU is sent from client to server and contains
//  information about RAIL client state and features supported by the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +---------------------------------------------------------------+
// |                             Flags                             |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  header MUST be set to 0x000b (TS_RAIL_ORDER_CLIENTSTATUS).

// Flags (4 bytes): An unsigned 32-bit integer. RAIL features that are
//  supported by the client; MUST be set to one of the following.

//  +-----------------------------------------------------+--------------------+
//  | Value                                               |   Meaning          |
//  +-----------------------------------------------------+--------------------+
//  | TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE             | Indicates that the |
//  | 0x00000001                                          | client supports    |
//  |                                                     | the local          |
//  |                                                     | move/size RAIL     |
//  |                                                     | feature.           |
//  +-----------------------------------------------------+--------------------+
//  | TS_RAIL_CLIENTSTATUS_AUTORECONNECT                  | Indicates that the |
//  | 0x00000002                                          | client is          |
//  |                                                     | auto-reconnecting  |
//  |                                                     | to the server      |
//  |                                                     | after an           |
//  |                                                     | unexpected         |
//  |                                                     | disconnect of the  |
//  |                                                     | session.           |
//  +-----------------------------------------------------+--------------------+
//  | TS_RAIL_CLIENTSTATUS_ZORDER_SYNC                    | Indicates that the |
//  | 0x00000004                                          | client supports    |
//  |                                                     | Z-order sync using |
//  |                                                     | the Z-Order Sync   |
//  |                                                     | Information PDU    |
//  |                                                     | (section           |
//  |                                                     | 2.2.2.11.1).       |
//  +-----------------------------------------------------+--------------------+
//  | TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED | Indicates that the |
//  | 0x00000010                                          | client supports    |
//  |                                                     | resize margins     |
//  |                                                     | using the Window   |
//  |                                                     | Information PDU    |
//  |                                                     | (section           |
//  |                                                     |  2.2.1.3.1).       |
//  +-----------------------------------------------------+--------------------+
//  | TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED      | Indicates that the |
//  | 0x00000040                                          | client supports    |
//  |                                                     | application        |
//  |                                                     | desktop toolbar    |
//  |                                                     | remoting using the |
//  |                                                     | Window Information |
//  |                                                     | PDU (section       |
//  |                                                     | 2.2.1.3.1).        |
//  +-----------------------------------------------------+--------------------+

enum {
      TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE             = 0x00000001
    , TS_RAIL_CLIENTSTATUS_AUTORECONNECT                  = 0x00000002
    , TS_RAIL_CLIENTSTATUS_ZORDER_SYNC                    = 0x00000004
    , TS_RAIL_CLIENTSTATUS_WINDOW_RESIZE_MARGIN_SUPPORTED = 0x00000010
    , TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED      = 0x00000040
};

class ClientInformationPDU {
    uint32_t Flags_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Flags_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // Flags(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Information PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_ = stream.in_uint32_le();
    }

    uint32_t Flags() const { return this->Flags_; }

    void Flags(uint32_t Flags_) {
        this->Flags_ = Flags_;
    }

    static size_t size() {
        return 4;   // Flags(4)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientInformationPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "Flags=0x%X", this->Flags_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.2.3 HandshakeEx PDU (TS_RAIL_ORDER_HANDSHAKE_EX)
// ====================================================================

// The HandshakeEx PDU is sent from the server to the client to signal that
//  it is ready to begin Enhanced RemoteApp mode. The server sends the
//  HandshakeEx PDU, and the client responds with the Handshake PDU (section
//  2.2.2.2.1).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +---------------------------------------------------------------+
// |                          buildNumber                          |
// +---------------------------------------------------------------+
// |                       railHandshakeFlags                      |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  the header MUST be set to 0x0013 (TS_RAIL_ORDER_HANDSHAKE_EX).

// buildNumber (4 bytes): An unsigned 32-bit integer. The build or version of
//  the sending party.

// railHandshakeFlags (4 bytes): An unsigned 32-bit integer. Flags for
//  setting up RAIL session parameters.

//  +---------------------------------------+----------------------------------+
//  | Flag                                  | Meaning                          |
//  +---------------------------------------+----------------------------------+
//  | TS_RAIL_ORDER_HANDSHAKEEX_FLAGS_HIDEF | Indicates that Enhanced          |
//  | 0x00000001                            |  RemoteApp (section 1.3.3) is    |
//  |                                       | supported. This implies support  |
//  |                                       | for the Remote Desktop Protocol: |
//  |                                       | Graphics Pipeline Extension      |
//  |                                       | ([MS-RDPEGFX] section 1.5),      |
//  |                                       | specifically the                 |
//  |                                       | RDPGFX_MAP_SURFACE_TO_WINDOW_PDU |
//  |                                       | ([MS-RDPEGFX] section 2.2.2.20)  |
//  |                                       | message.                         |
//  +---------------------------------------+----------------------------------+

enum {
      TS_RAIL_ORDER_HANDSHAKEEX_FLAGS_HIDEF = 0x00000001
};

class HandshakeExPDU {
    uint32_t buildNumber_        = 0;
    uint32_t railHandshakeFlags_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->buildNumber_);
        stream.out_uint32_le(this->railHandshakeFlags_);
    }

    void receive(InStream & stream) {
        const unsigned expected = 8;    // buildNumber(4) + railHandshakeFlags(4)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "HandshakeEx PDU: expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        this->buildNumber_        = stream.in_uint32_le();
        this->railHandshakeFlags_ = stream.in_uint32_le();
    }

    uint32_t buildNumber() const { return this->buildNumber_; }

    void buildNumber(uint32_t buildNumber_) {
        this->buildNumber_ = buildNumber_;
    }

    uint32_t railHandshakeFlags() const { return this->railHandshakeFlags_; }

    void railHandshakeFlags(uint32_t railHandshakeFlags_) {
        this->railHandshakeFlags_ = railHandshakeFlags_;
    }

    static size_t size() {
        return 8;   // buildNumber(4) + railHandshakeFlags(4)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "HandshakeExPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "buildNumber=%u railHandshakeFlags_=0x%X",
            this->buildNumber_, this->railHandshakeFlags_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.3.1 Client Execute PDU (TS_RAIL_ORDER_EXEC)
// ===============================================================

// The Client Execute PDU is sent from a client to a server to request that a
//  remote application launch on the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +-------------------------------+-------------------------------+
// |             Flags             |        ExeOrFileLength        |
// +-------------------------------+-------------------------------+
// |        WorkingDirLength       |          ArgumentsLen         |
// +-------------------------------+-------------------------------+
// |                      ExeOrFile (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                     WorkingDir (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      Arguments (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  the header MUST be set to 0x0001 (TS_RAIL_ORDER_EXEC).

// Flags (2 bytes): An unsigned 16-bit integer. Specifies a bitfield of flags
//  that indicate modifications to the Client Execute PDU fields.

//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | | | | | | | | | | |1| | | | | |
//  |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |A|B|C|D|E|0|0|0|0|0|0|0|0|0|0|0|
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//  Where the bits are defined as:

//  +-------------------------------------------+------------------------------+
//  | Value                                     | Description                  |
//  +-------------------------------------------+------------------------------+
//  | A                                         | The environment variables in |
//  | TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY | the WorkingDir field MUST be |
//  |                                           | expanded on the server.      |
//  +-------------------------------------------+------------------------------+
//  | B                                         | The drive letters in the     |
//  | TS_RAIL_EXEC_FLAG_TRANSLATE_FILES         | file path MUST be converted  |
//  |                                           | to corresponding mapped      |
//  |                                           | drives on the server. This   |
//  |                                           | flag MUST NOT be set if the  |
//  |                                           | TS_RAIL_EXEC_FLAG_FILE       |
//  |                                           | (0x0004) flag is not set.    |
//  +-------------------------------------------+------------------------------+
//  | C                                         | If this flag is set, the     |
//  | TS_RAIL_EXEC_FLAG_FILE                    | ExeOrFile field refers to a  |
//  |                                           | file path. If it is not set, |
//  |                                           | the ExeOrFile field refers   |
//  |                                           | to an executable.            |
//  +-------------------------------------------+------------------------------+
//  | D                                         | The environment variables in |
//  | TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS        | the Arguments field MUST be  |
//  |                                           | expanded on the server.      |
//  +-------------------------------------------+------------------------------+
//  | E                                         | If this flag is set, the     |
//  | TS_RAIL_EXEC_FLAG_APP_USER_MODEL_ID       | ExeOrFile field refers to an |
//  |                                           | application user model ID.   |
//  |                                           | If it is not set, the        |
//  |                                           | ExeOrFile field refers to a  |
//  |                                           | file path. This flag MUST be |
//  |                                           | ignored if the               |
//  |                                           | TS_RAIL_EXEC_FLAG_FILE       |
//  |                                           | (0x0004) flag is set. An     |
//  |                                           | application user model ID is |
//  |                                           | a string that uniquely       |
//  |                                           | identifies an application,   |
//  |                                           | regardless of where the      |
//  |                                           | application is installed on  |
//  |                                           | the operating system. The    |
//  |                                           | string can be used to        |
//  |                                           | identify Windows Store       |
//  |                                           | applications as well as      |
//  |                                           | desktop applications.        |
//  +-------------------------------------------+------------------------------+

enum {
      TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY = 0x0001
    , TS_RAIL_EXEC_FLAG_TRANSLATE_FILES         = 0x0002
    , TS_RAIL_EXEC_FLAG_FILE                    = 0x0004
    , TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS        = 0x0008
    , TS_RAIL_EXEC_FLAG_APP_USER_MODEL_ID       = 0x0010
};

// ExeOrFileLength (2 bytes): An unsigned 16-bit integer. Specifies the
//  length of the ExeOrFile field in bytes. The length MUST be nonzero. The
//  maximum length is 520 bytes.

// WorkingDirLength (2 bytes): An unsigned 16-bit integer. Specifies the
//  length of the WorkingDir field, in bytes. The minimum length is 0, and
//  the maximum length is 520 bytes.

// ArgumentsLen (2 bytes): An unsigned 16-bit integer. Specifies the length
//  of the Arguments field, in bytes. The minimum length is 0, and the
//  maximum length is 16,000 bytes.<10>

// ExeOrFile (variable): A non-null-terminated string of Unicode characters.
//  Variable length. Specifies the executable or file path to be launched on
//  the server. This field MUST be present. The maximum length of this field,
//  including file path translations (see TS_RAIL_EXEC_FLAG_TRANSLATE_FILES
//  mask of Flags field), is 520 bytes.

// WorkingDir (variable): Optional non-null-terminated string of Unicode
//  characters. Variable length. Specifies the working directory of the
//  launched ExeOrFile field. If the WorkingDirLength field is 0, this field
//  MUST NOT be present; otherwise, it MUST be present. The maximum length of
//  this field, including expanded environment variables (see
//  TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY mask of Flags field), is 520
//  bytes.

// Arguments (variable): Optional non-null-terminated string of Unicode
//  characters. Variable length. Specifies the arguments to the ExeOrFile
//  field. If the ArgumentsLength field is 0, this field MUST NOT be present;
//  otherwise, it MUST be present. The maximum length of this field,
//  including expanded environment variables (see
//  TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS mask of Flags field), is 16,000 bytes.

class ClientExecutePDU {

    uint16_t Flags_ = 0;

    std::string exe_or_file;
    std::string working_dir;
    std::string arguments;

public:
    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->Flags_);

        const uint32_t offset_of_ExeOrFile  = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_WorkingDir = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_Arguments  = stream.get_offset();
        stream.out_clear_bytes(2);

        const size_t maximum_length_of_ExeOrFile_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, this->exe_or_file, maximum_length_of_ExeOrFile_in_bytes,
            offset_of_ExeOrFile);

        const size_t maximum_length_of_WorkingDir_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, this->working_dir, maximum_length_of_WorkingDir_in_bytes,
            offset_of_WorkingDir);

        const size_t maximum_length_of_Arguments_in_bytes = 16000;
        put_non_null_terminated_utf16_from_utf8(
            stream, this->arguments, maximum_length_of_Arguments_in_bytes,
            offset_of_Arguments);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected =
                8;  // Flags(2) + ExeOrFileLength(2) + WorkingDirLength(2) + ArgumentsLen(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Execute PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_ = stream.in_uint16_le();

        uint16_t ExeOrFileLength  = stream.in_uint16_le();
        uint16_t WorkingDirLength = stream.in_uint16_le();
        uint16_t ArgumentsLen     = stream.in_uint16_le();

        get_non_null_terminated_utf16_from_utf8(
            this->exe_or_file, stream, ExeOrFileLength, "Client Execute PDU");
        get_non_null_terminated_utf16_from_utf8(
            this->working_dir, stream, WorkingDirLength, "Client Execute PDU");
        get_non_null_terminated_utf16_from_utf8(
            this->arguments, stream, ArgumentsLen, "Client Execute PDU");
    }

    uint16_t Flags() const { return this->Flags_; }

    void Flags(uint16_t Flags_) { this->Flags_ = Flags_; }

    const char * ExeOrFile() const { return this->exe_or_file.c_str(); }

    void ExeOrFile(const char * ExeOrFile_) { this->exe_or_file = ExeOrFile_; }

    const char * WorkingDir() const { return this->working_dir.c_str(); }

    void WorkingDir(const char * WorkingDir_) { this->working_dir = WorkingDir_; }

    const char * Arguments() const { return this->arguments.c_str(); }

    void Arguments(const char * Arguments_) { this->arguments = Arguments_; }

    size_t size() const {
        size_t count = 12;  // Flags(2) + ExeOrFileLength(2) + WorkingDirLength(2) + ArgumentsLen(2)

        {
            StaticOutStream<65536> out_stream;

            auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
                out_stream, this->exe_or_file, this->exe_or_file.length() * 2);

            count += 2 /* CbString(2) */ + size_of_unicode_data;
        }

        {
            StaticOutStream<65536> out_stream;

            auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
                out_stream, this->working_dir, this->working_dir.length() * 2);

            count += 2 /* CbString(2) */ + size_of_unicode_data;
        }

        {
            StaticOutStream<65536> out_stream;

            auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
                out_stream, this->arguments, this->arguments.length() * 2);

            count += 2 /* CbString(2) */ + size_of_unicode_data;
        }

        return count;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientExecutePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "Flags=0x%X ExeOrFile=\"%s\" WorkingDir=\"%s\" Arguments=\"%s\"",
            this->Flags_, this->exe_or_file.c_str(), this->working_dir.c_str(),
            this->arguments.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class ClientExecutePDU

// [MS-RDPERP] - 2.2.2.3.2 Server Execute Result PDU
// (TS_RAIL_ORDER_EXEC_RESULT)
// =================================================

// The Server Execute Result PDU is sent from server to client in response to
//  a Client Execute PDU request, and contains the result of the server's
//  attempt to launch the requested executable.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +-------------------------------+-------------------------------+
// |             Flags             |           ExecResult          |
// +-------------------------------+-------------------------------+
// |                           RawResult                           |
// +-------------------------------+-------------------------------+
// |            Padding            |        ExeOrFileLength        |
// +-------------------------------+-------------------------------+
// |                      ExeOrFile (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  the header MUST be set to TS_RAIL_ORDER_EXEC_RESULT (0x0080).

// Flags (2 bytes): An unsigned 16-bit integer. Identical to the Flags field
//  of the Client Execute PDU. The server sets this field to enable the
//  client to match the Client Execute PDU with the Server Execute Result
//  PDU.

// ExecResult (2 bytes): An unsigned 16-bit integer. The result of the Client
//  Execute PDU. This field MUST be set to one of the following values.

//  +------------------------------+-------------------------------------------+
//  | Value                        | Meaning                                   |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_S_OK               | The Client Execute request was successful |
//  | 0x0000                       | and the requested application or file has |
//  |                              | been launched.                            |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_HOOK_NOT_LOADED  | The Client Execute request could not be   |
//  | 0x0001                       | satisfied because the server is not       |
//  |                              | monitoring the current input desktop.     |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_DECODE_FAILED    | The Execute request could not be          |
//  | 0x0002                       | satisfied because the request PDU was     |
//  |                              | malformed.                                |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_NOT_IN_ALLOWLIST | The Client Execute request could not be   |
//  | 0x0003                       | satisfied because the requested           |
//  |                              | application was blocked by policy from    |
//  |                              | being launched on the server.             |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_FILE_NOT_FOUND   | The Client Execute request could not be   |
//  | 0x0005                       | satisfied because the application or file |
//  |                              | path could not be found.                  |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_FAIL             | The Client Execute request could not be   |
//  | 0x0006                       | satisfied because an unspecified error    |
//  |                              | occurred on the server.                   |
//  +------------------------------+-------------------------------------------+
//  | RAIL_EXEC_E_SESSION_LOCKED   | The Client Execute request could not be   |
//  | 0x0007                       | satisfied because the remote session is   |
//  |                              | locked.                                   |
//  +------------------------------+-------------------------------------------+

// RawResult (4 bytes): An unsigned 32-bit integer. Contains an operating
//  system-specific return code for the result of the Client Execute
//  request.<11>

// Padding (2 bytes): An unsigned 16-bit integer. Not used.

// ExeOrFileLength (2 bytes): An unsigned 16-bit integer. Specifies the
//  length of the ExeOrFile field in bytes. The length MUST be nonzero. The
//  maximum length is 520 bytes.

// ExeOrFile (variable): The executable or file that was attempted to be
//  launched. This field is copied from the ExeOrFile field of the Client
//  Execute PDU. The server sets this field to enable the client to match the
//  Client Execute PDU with the Server Execute Result PDU.

enum {
      RAIL_EXEC_S_OK               = 0x0000
    , RAIL_EXEC_E_HOOK_NOT_LOADED  = 0x0001
    , RAIL_EXEC_E_DECODE_FAILED    = 0x0002
    , RAIL_EXEC_E_NOT_IN_ALLOWLIST = 0x0003
    , RAIL_EXEC_E_FILE_NOT_FOUND   = 0x0005
    , RAIL_EXEC_E_FAIL             = 0x0006
    , RAIL_EXEC_E_SESSION_LOCKED   = 0x0007
};

static inline
const char* get_RAIL_ExecResult_name(uint16_t ExecResult) {
    switch (ExecResult) {
        case RAIL_EXEC_S_OK:               return "RAIL_EXEC_S_OK";
        case RAIL_EXEC_E_HOOK_NOT_LOADED:  return "RAIL_EXEC_E_HOOK_NOT_LOADED";
        case RAIL_EXEC_E_DECODE_FAILED:    return "RAIL_EXEC_E_DECODE_FAILED";
        case RAIL_EXEC_E_NOT_IN_ALLOWLIST: return "RAIL_EXEC_E_NOT_IN_ALLOWLIST";
        case RAIL_EXEC_E_FILE_NOT_FOUND:   return "RAIL_EXEC_E_FILE_NOT_FOUND";
        case RAIL_EXEC_E_SESSION_LOCKED:   return "RAIL_EXEC_E_SESSION_LOCKED";
        default:                           return "<unknown>";
    }
}

class ServerExecuteResultPDU {
    uint16_t Flags_          = 0;
    uint16_t ExecResult_     = 0;
    uint32_t RawResult_      = 0;
    uint16_t ExeOrFileLength = 0;

    std::string exe_or_file;

public:
    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->Flags_);
        stream.out_uint16_le(this->ExecResult_);
        stream.out_uint32_le(this->RawResult_);

        stream.out_clear_bytes(2);  // Padding(2)

        const uint32_t offset_of_ExeOrFile  = stream.get_offset();
        stream.out_clear_bytes(2);

        const size_t maximum_length_of_ExeOrFile_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, this->exe_or_file, maximum_length_of_ExeOrFile_in_bytes,
            offset_of_ExeOrFile);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected =
                12;  // Flags(2) + ExecResult(2) + RawResult(4) + Padding(2) + ExeOrFileLength(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server Execute Result PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_            = stream.in_uint16_le();
        this->ExecResult_       = stream.in_uint16_le();
        this->RawResult_        = stream.in_uint32_le();

        stream.in_skip_bytes(2);    // Padding(2)

        this->ExeOrFileLength   = stream.in_uint16_le();

        get_non_null_terminated_utf16_from_utf8(
            this->exe_or_file, stream, this->ExeOrFileLength, "Server Execute Result PDU");
    }

    uint16_t Flags() const { return this->Flags_; }

    void Flags(uint16_t Flags_) { this->Flags_ = Flags_; }

    uint16_t ExecResult() const { return this->ExecResult_; }

    void ExecResult(uint16_t ExecResult_) { this->ExecResult_ = ExecResult_; }

    uint32_t RawResult() const { return this->RawResult_; }

    void RawResult(uint32_t RawResult_) { this->RawResult_ = RawResult_; }

    const char * ExeOrFile() const { return this->exe_or_file.c_str(); }

    void ExeOrFile(const char * ExeOrFile_) { this->exe_or_file = ExeOrFile_; }

    size_t size() const {
        size_t count = 12;  // Flags(2) + ExecResult(2) + RawResult(4) + Padding(2) + ExeOrFileLength(2)

        StaticOutStream<65536> out_stream;

        auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
            out_stream, this->exe_or_file, this->exe_or_file.length() * 2);

        count += 2 /* CbString(2) */ + size_of_unicode_data;

        return count;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerExecuteResultPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "Flags=0x%X ExecResult=%s(%u) RawResult=0x%08X ExeOrFile=\"%s\"",
            this->Flags_, ::get_RAIL_ExecResult_name(this->ExecResult_),
            this->ExecResult_, this->RawResult_, this->exe_or_file.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class ServerExecuteResultPDU

// [MS-RDPERP] - 2.2.2.4.2 High Contrast System Information Structure
//  (TS_HIGHCONTRAST)
// ==================================================================

// The TS_HIGHCONTRAST packet defines parameters for the high-contrast
//  accessibility feature.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Flags                             |
// +---------------------------------------------------------------+
// |                       ColorSchemeLength                       |
// +---------------------------------------------------------------+
// |                     ColorScheme (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Flags (4 bytes): An unsigned 32-bit integer. This field is opaque to RAIL.
//  It is transmitted from the client to the server and used by the server to
//  set the High Contrast parameters.<12>

// ColorSchemeLength (4 bytes): An unsigned 32-bit integer. The length, in
//  bytes, of the ColorScheme field.

// ColorScheme (variable): UNICODE_STRING. Variable length. The
//  Windows-specific name of the High Contrast Color Scheme, specified as a
//  null-terminated UNICODE_STRING.<13>

class HighContrastSystemInformationStructure {
    uint32_t Flags_ = 0;

    std::string color_scheme;

public:
    HighContrastSystemInformationStructure() = default;

    HighContrastSystemInformationStructure(uint32_t Flags_, const char* ColorScheme_)
    : Flags_(Flags_)
    , color_scheme(ColorScheme_) {}

    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->Flags_);

        const size_t offset_of_data_length = stream.get_offset();
        stream.out_skip_bytes(4);

        uint8_t * const unicode_data = stream.get_current();
        const size_t size_of_unicode_data = ::UTF8toUTF16(
            this->color_scheme, unicode_data, this->color_scheme.length() * 2);
        stream.out_skip_bytes(size_of_unicode_data);
        stream.out_clear_bytes(2);  // null-terminator

        stream.set_out_uint32_le(size_of_unicode_data + 2 /*null-terminator*/, offset_of_data_length);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 8;    // Flags(4) + ColorSchemeLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated High Contrast System Information Structure: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_ = stream.in_uint32_le();

        uint32_t const ColorSchemeLength = stream.in_uint32_le();

        if (!stream.in_check_rem(ColorSchemeLength)) {
            LOG(LOG_ERR,
                "Truncated High Contrast System Information Structure: expected=%u remains=%zu (1)",
                ColorSchemeLength, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

//        assert(ColorSchemeLength >= 2);

        get_non_null_terminated_utf16_from_utf8(
            this->color_scheme, stream, ColorSchemeLength/*stream.in_uint16_le()*/,
            "High Contrast System Information Structure");
    }

    uint32_t Flags() const { return this->Flags_; }

    const char * ColorScheme() const { return this->color_scheme.c_str(); }

    size_t size() const {
        size_t count = 8;   // // Flags(4) + ColorSchemeLength(4)

        StaticOutStream<65536> out_stream;

        auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
            out_stream, this->color_scheme, this->color_scheme.length() * 2);

        count += 2 /* CbString(2) */ + size_of_unicode_data;

        return count;
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "HighContrastSystemInformation=(");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "Flags=0x%X ColorScheme=\"%s\"",
            this->Flags_, this->color_scheme.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length, ")");
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.4.1 Client System Parameters Update PDU
//  (TS_RAIL_ORDER_SYSPARAM)
// ===========================================================

// The Client System Parameters Update PDU is sent from the client to the
//  server to synchronize system parameters on the server with those on the
//  client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             header                            |
// +---------------------------------------------------------------+
// |                          SystemParam                          |
// +---------------------------------------------------------------+
// |                        Body (variable)                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  header MUST be set to TS_RAIL_ORDER_SYSPARAM(0x0003).

// SystemParam (4 bytes): An unsigned 32-bit integer. The type of system
//  parameter being transmitted. The field MUST be set to one of the
//  following values.

//  +------------------------+-------------------------------------------------+
//  | Value                  | Meaning                                         |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETDRAGFULLWINDOWS | The system parameter for full-window drag.      |
//  | 0x00000025             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETKEYBOARDCUES    | The system parameter to determine whether menu  |
//  | 0x0000100B             | access keys are always underlined.              |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETKEYBOARDPREF    | The system parameter specifying a preference    |
//  | 0x00000045             | for the keyboard instead of the mouse.          |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETWORKAREA        | The system parameter to set the size of the     |
//  | 0x0000002F             | work area. The work area is the portion of the  |
//  |                        | screen not obscured by the system taskbar or by |
//  |                        | application desktop toolbars.                   |
//  +------------------------+-------------------------------------------------+
//  | RAIL_SPI_DISPLAYCHANGE | The system parameter for display resolution.    |
//  | 0x0000F001             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETMOUSEBUTTONSWAP | The system parameter to swap or restore the     |
//  | 0x00000021             | meaning of the left and right mouse buttons.    |
//  +------------------------+-------------------------------------------------+
//  | RAIL_SPI_TASKBARPOS    | The system parameter to indicate the size of    |
//  | 0x0000F000             | the client taskbar.                             |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETHIGHCONTRAST    | The system parameter to set the parameters of   |
//  | 0x00000043             | the HighContrast accessibility feature.         |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETCARETWIDTH      | The system parameter to set the caret width     |
//  | 0x00002007             | used for text entry controls. This flag SHOULD  |
//  |                        | NOT be sent if the server did not advertise the |
//  |                        | EXTENDED_SPI_SUPPORTED flag in the HandshakeEx  |
//  |                        | PDU (section 2.2.2.2.3).                        |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETSTICKYKEYS      | The system parameter to set the parameters of   |
//  | 0x0000003B             | the Sticky Keys accessibility feature. This     |
//  |                        | flag SHOULD NOT be sent if the server did not   |
//  |                        | advertise the EXTENDED_SPI_SUPPORTED flag in    |
//  |                        | the HandshakeEx PDU.                            |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETTOGGLEKEYS      | The system parameter to set the parameters of   |
//  | 0x00000035             | the Toggle Keys accessibility feature. This     |
//  |                        | flag SHOULD NOT be sent if the server did not   |
//  |                        | advertise the EXTENDED_SPI_SUPPORTED flag in    |
//  |                        | the HandshakeEx PDU.                            |
//  +------------------------+-------------------------------------------------+
//  | SPI_SETFILTERKEYS      | The system parameter to set the parameters of   |
//  | 0x00000033             | the Filter Keys accessibility feature. This     |
//  |                        | flag SHOULD NOT be sent if the server did not   |
//  |                        | advertise the EXTENDED_SPI_SUPPORTED flag in    |
//  |                        | the HandshakeEx PDU.                            |
//  +------------------------+-------------------------------------------------+

enum {
      SPI_SETDRAGFULLWINDOWS = 0x00000025
    , SPI_SETKEYBOARDCUES    = 0x0000100B
    , SPI_SETKEYBOARDPREF    = 0x00000045
    , SPI_SETWORKAREA        = 0x0000002F
    , RAIL_SPI_DISPLAYCHANGE = 0x0000F001
    , SPI_SETMOUSEBUTTONSWAP = 0x00000021
    , RAIL_SPI_TASKBARPOS    = 0x0000F000
    , SPI_SETHIGHCONTRAST    = 0x00000043
    , SPI_SETCARETWIDTH      = 0x00002007
    , SPI_SETSTICKYKEYS      = 0x0000003B
    , SPI_SETTOGGLEKEYS      = 0x00000035
    , SPI_SETFILTERKEYS      = 0x00000033
};

// Body (variable): The contents of this field depend on the SystemParameter
//  field. The following table outlines the valid values of the
//  SystemParameter field (Value column) and corresponding values of the Body
//  field (Meaning column).

//  +------------------------+------------------------------------------------+
//  | Value                  | Meaning                                        |
//  +------------------------+------------------------------------------------+
//  | SPI_SETDRAGFULLWINDOWS | Size of Body field: 1 byte.                    |
//  | 0x0025                 | 0 (FALSE): Full Window Drag is disabled.       |
//  |                        | Nonzero (TRUE): Full Window Drag is enabled.   |
//  +------------------------+------------------------------------------------+
//  | SPI_SETKEYBOARDCUES    | Size of Body field: 1 byte.                    |
//  | 0x100B                 | 0 (FALSE): Menu Access Keys are underlined     |
//  |                        | only when the menu is activated by the         |
//  |                        | keyboard. Nonzero (TRUE): Menu Access Keys are |
//  |                        | always underlined.                             |
//  +------------------------+------------------------------------------------+
//  | SPI_SETKEYBOARDPREF    | Size of Body field: 1 byte.                    |
//  | 0x0045                 | 0 (FALSE): The user does not prefer the        |
//  |                        | keyboard over mouse. Nonzero (TRUE): The user  |
//  |                        | prefers the keyboard over mouse. This causes   |
//  |                        | applications to display keyboard interfaces    |
//  |                        | that would otherwise be hidden.                |
//  +------------------------+------------------------------------------------+
//  | SPI_SETMOUSEBUTTONSWAP | Size of Body field: 1 byte.                    |
//  | 0x0021                 | 0 (FALSE): Restores the meaning of the left    |
//  |                        | and right mouse buttons to their original      |
//  |                        | meanings. Nonzero (TRUE): Swaps the meaning of |
//  |                        | the left and right mouse buttons.              |
//  +------------------------+------------------------------------------------+
//  | SPI_SETWORKAREA        | Size of Body field: 8 bytes.                   |
//  | 0x002F                 | The body is a TS_RECTANGLE_16 structure that   |
//  |                        | defines the work area in virtual screen        |
//  |                        | coordinates. In a system with multiple display |
//  |                        | monitors, the work area is that of the monitor |
//  |                        | that contains the specified rectangle. For     |
//  |                        | more information about virtual screen          |
//  |                        | coordinates, see [MSDN-VIRTUALSCR].            |
//  +------------------------+------------------------------------------------+
//  | RAIL_SPI_DISPLAYCHANGE | Size of Body field: 8 bytes.                   |
//  | 0xF001                 | The body is a TS_RECTANGLE_16 structure that   |
//  |                        | indicates the new display resolution in        |
//  |                        | virtual screen coordinates. For more           |
//  |                        | information about virtual screen coordinates,  |
//  |                        | see [MSDN-VIRTUALSCR].                         |
//  +------------------------+------------------------------------------------+
//  | RAIL_SPI_TASKBARPOS    | Size of Body field: 8 bytes.                   |
//  | 0xF000                 | The body is a TS_RECTANGLE_16 structure that   |
//  |                        | indicates the size of the client taskbar.      |
//  +------------------------+------------------------------------------------+
//  | SPI_SETHIGHCONTRAST    | Size of Body field: Variable number of bytes.  |
//  | 0x0043                 | The body is a TS_HIGHCONTRAST structure.       |
//  +------------------------+------------------------------------------------+
//  | SPI_SETCARETWIDTH      | Size of Body field: 4 bytes.                   |
//  | 0x00002007             | The body is an unsigned 32-bit integer that    |
//  |                        | indicates the width of the caret. The minimum  |
//  |                        | allowed value is 0x00000001.                   |
//  +------------------------+------------------------------------------------+
//  | SPI_SETSTICKYKEYS      | Size of Body field: 4 bytes.                   |
//  | 0x0000003B             | The body is a TS_STICKYKEYS structure.         |
//  +------------------------+------------------------------------------------+
//  | SPI_SETTOGGLEKEYS      | Size of Body field: 4 bytes.                   |
//  | 0x00000035             | The body is a TS_TOGGLEKEYS structure.         |
//  +------------------------+------------------------------------------------+
//  | SPI_SETFILTERKEYS      | Size of Body field: 20 bytes.                  |
//  | 0x00000033             | The body is a TS_FILTERKEYS structure.         |
//  +------------------------+------------------------------------------------+

static inline
const char* get_RAIL_ClientSystemParam_name(uint32_t SystemParam) {
    switch (SystemParam) {
        case SPI_SETDRAGFULLWINDOWS: return "SPI_SETDRAGFULLWINDOWS";
        case SPI_SETKEYBOARDCUES:    return "SPI_SETKEYBOARDCUES";
        case SPI_SETKEYBOARDPREF:    return "SPI_SETKEYBOARDPREF";
        case SPI_SETWORKAREA:        return "SPI_SETWORKAREA";
        case RAIL_SPI_DISPLAYCHANGE: return "RAIL_SPI_DISPLAYCHANGE";
        case SPI_SETMOUSEBUTTONSWAP: return "SPI_SETMOUSEBUTTONSWAP";
        case RAIL_SPI_TASKBARPOS:    return "RAIL_SPI_TASKBARPOS";
        case SPI_SETHIGHCONTRAST:    return "SPI_SETHIGHCONTRAST";
        case SPI_SETCARETWIDTH:      return "SPI_SETCARETWIDTH";
        case SPI_SETSTICKYKEYS:      return "SPI_SETSTICKYKEYS";
        case SPI_SETTOGGLEKEYS:      return "SPI_SETTOGGLEKEYS";
        case SPI_SETFILTERKEYS:      return "SPI_SETFILTERKEYS";

        default:                     return "<unknown>";
    }
}

class ClientSystemParametersUpdatePDU {
    uint32_t                                SystemParam_ = 0;

    uint8_t                                 body_b_ = 0;

    RDP::RAIL::Rectangle                    body_r_;
    HighContrastSystemInformationStructure  body_hcsis;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->SystemParam_);

        switch (this->SystemParam_) {
            case SPI_SETDRAGFULLWINDOWS:
            case SPI_SETKEYBOARDCUES:
            case SPI_SETKEYBOARDPREF:
            case SPI_SETMOUSEBUTTONSWAP:
                stream.out_uint8(this->body_b_);
                break;

            case SPI_SETWORKAREA:
            case RAIL_SPI_DISPLAYCHANGE:
            case RAIL_SPI_TASKBARPOS:
                this->body_r_.emit(stream);
                break;

            case SPI_SETHIGHCONTRAST:
                this->body_hcsis.emit(stream);
                break;
        }
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // SystemParam(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client System Parameters Update PDU (1): expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->SystemParam_ = stream.in_uint32_le();

        switch (this->SystemParam_) {
            case SPI_SETDRAGFULLWINDOWS:
            case SPI_SETKEYBOARDCUES:
            case SPI_SETKEYBOARDPREF:
            case SPI_SETMOUSEBUTTONSWAP:
                {
                    const unsigned expected = 1;    // Body(variable)

                    if (!stream.in_check_rem(expected)) {
                        LOG(LOG_ERR,
                            "Truncated Client System Parameters Update PDU (2): expected=%u remains=%zu",
                            expected, stream.in_remain());
                        throw Error(ERR_RAIL_PDU_TRUNCATED);
                    }
                }

                this->body_b_ = stream.in_uint8();
                break;

            case SPI_SETWORKAREA:
            case RAIL_SPI_DISPLAYCHANGE:
            case RAIL_SPI_TASKBARPOS:
                this->body_r_.receive(stream);
                break;

            case SPI_SETHIGHCONTRAST:
                this->body_hcsis.receive(stream);
                break;
        }
    }

    uint32_t SystemParam() const { return this->SystemParam_; }

    RDP::RAIL::Rectangle const & body_r() const {
        return this->body_r_;
    }

    uint8_t body_b() const { return this->body_b_; }

    size_t size() const {
        size_t count = 4;   // SystemParam(4)

        switch (this->SystemParam_) {
            case SPI_SETDRAGFULLWINDOWS:
            case SPI_SETKEYBOARDCUES:
            case SPI_SETKEYBOARDPREF:
            case SPI_SETMOUSEBUTTONSWAP:
                count += 1; // Body(variable)
                break;

            case SPI_SETWORKAREA:
            case RAIL_SPI_DISPLAYCHANGE:
            case RAIL_SPI_TASKBARPOS:
                count += this->body_r_.size();   // Body(variable)
                break;

            case SPI_SETHIGHCONTRAST:
                count += this->body_hcsis.size();   // Body(variable)
                break;
        }

        return count;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientSystemParametersUpdatePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "SystemParam=%s(0x%X) ",
            ::get_RAIL_ClientSystemParam_name(this->SystemParam_),
            this->SystemParam_);
        length += ((result < size - length) ? result : (size - length - 1));

        switch (this->SystemParam_) {
            case SPI_SETDRAGFULLWINDOWS:
            case SPI_SETKEYBOARDCUES:
            case SPI_SETKEYBOARDPREF:
            case SPI_SETMOUSEBUTTONSWAP:
                result = ::snprintf(buffer + length, size - length,
                    "Body=%s(%u) ",
                    (this->body_b_ ? "TRUE" : "FALSE"), this->body_b_);
                length += ((result < size - length) ? result : (size - length - 1));
                break;

            case SPI_SETWORKAREA:
            case RAIL_SPI_DISPLAYCHANGE:
            case RAIL_SPI_TASKBARPOS:
                result = this->body_r_.str(buffer + length, size - length);
                length += ((result < size - length) ? result : (size - length - 1));
                break;

            case SPI_SETHIGHCONTRAST:
                result = this->body_hcsis.str(buffer + length, size - length);
                length += ((result < size - length) ? result : (size - length - 1));
                break;
        }

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.5.1 Server System Parameters Update PDU
//  (TS_RAIL_ORDER_SYSPARAM)
// ===========================================================

// The Server System Parameters Update PDU is sent from the server to client
//  to synchronize system parameters on the client with those on the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Header                            |
// +---------------------------------------------------------------+
// |                        SystemParameter                        |
// +---------------+-----------------------------------------------+
// |      Body     |
// +---------------+

// Header (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of
//  header MUST be set to 0x03 (TS_RAIL_ORDER_SYSPARAM).

// SystemParameter (4 bytes): An unsigned 32-bit integer. The type of system
//  parameter being transmitted. This field MUST be set to one of the
//  following values.

//  +-------------------------+------------------------------------------------+
//  | Value                   | Meaning                                        |
//  +-------------------------+------------------------------------------------+
//  | SPI_SETSCREENSAVEACTIVE | The system parameter indicating whether the    |
//  | 0x00000011              | screen saver is enabled.                       |
//  +-------------------------+------------------------------------------------+
//  | SPI_SETSCREENSAVESECURE | The system parameter indicating whether the    |
//  | 0x00000077              | desktop is to be locked after switching out of |
//  |                         | screen saver mode (that is, after the screen   |
//  |                         | saver starts due to inactivity, then stops due |
//  |                         | to activity).<14>                              |
//  +-------------------------+------------------------------------------------+

enum {
      SPI_SETSCREENSAVEACTIVE = 0x00000011
    , SPI_SETSCREENSAVESECURE = 0x00000077
};

// Body (1 byte): The content of this field depends on the SystemParameter
//  field. The following table outlines the valid values of the
//  SystemParameter field (Value column) and corresponding values of the Body
//  field (Meaning column).

//  +-------------------------+------------------------------------------------+
//  | Value                   | Meaning                                        |
//  +-------------------------+------------------------------------------------+
//  | SPI_SETSCREENSAVEACTIVE | Size of Body field: 1 byte.                    |
//  | 0x00000011              | 0 (FALSE): Screen saver is not enabled.        |
//  |                         | Nonzero (TRUE): Screen Saver is enabled.       |
//  +-------------------------+------------------------------------------------+
//  | SPI_SETSCREENSAVESECURE | Size of Body field: 1 byte.                    |
//  | 0x00000077              | 0 (FALSE): Do not lock the desktop when        |
//  |                         | switching out of screen saver mode. Nonzero    |
//  |                         | (TRUE): Lock the desktop when switching out of |
//  |                         | screen saver mode.                             |
//  +-------------------------+------------------------------------------------+

static inline
const char* get_RAIL_ServerSystemParam_name(uint32_t SystemParam) {
    switch (SystemParam) {
        case SPI_SETSCREENSAVEACTIVE: return "SPI_SETSCREENSAVEACTIVE";
        case SPI_SETSCREENSAVESECURE: return "SPI_SETSCREENSAVESECURE";
        default:                      return "<unknown>";
    }
}

class ServerSystemParametersUpdatePDU {
    uint32_t SystemParam_ = 0;
    uint8_t  Body_        = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->SystemParam_);

        stream.out_uint8(this->Body_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 5;    // SystemParam(4) + Body(1)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server System Parameters Update PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->SystemParam_ = stream.in_uint32_le();
        this->Body_        = stream.in_uint8();
    }

    uint32_t SystemParam() const { return this->SystemParam_; }

    void SystemParam(uint32_t SystemParam_) { this->SystemParam_ = SystemParam_; }

    uint8_t Body() const { return this->Body_; }

    void Body(uint8_t Body_) { this->Body_ = Body_; }

    static size_t size() {
        return 5;   // SystemParam(4) + Body(1)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerSystemParametersUpdatePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "SystemParam=%s(0x%X) Body=%s(\\x%02x)",
            ::get_RAIL_ServerSystemParam_name(this->SystemParam_),
            this->SystemParam_, (this->Body_ ? "TRUE" : "FALSE"),
            this->Body_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.6.1 Client Activate PDU (TS_RAIL_ORDER_ACTIVATE)
// ====================================================================

// The Client Activate PDU is sent from client to server when a local RAIL
//  window on the client is activated or deactivated.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +---------------+-----------------------------------------------+
// |    Enabled    |
// +---------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_ACTIVATE (0x0002).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the associated
//  window on the server that is to be activated or deactivated.

// Enabled (1 byte): An unsigned 8-bit integer. Indicates whether the window
//  is to be activated (value = nonzero) or deactivated (value = 0).

class ClientActivatePDU {
    mutable uint32_t WindowId_ = 0;
            uint8_t  Enabled_  = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint8(this->Enabled_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 5;    // WindowId(4) + Enabled(1)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Get Activate PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
        this->Enabled_  = stream.in_uint8();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_ACTIVATE; }

    uint32_t WindowId() const { return this->WindowId_; }

    uint8_t Enabled() const { return this->Enabled_; }

    static size_t size() {
        return 5;   // WindowId(4) + Enabled(1)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientActivatePDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientActivatePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X Enabled=%s",
            this->WindowId_, (this->Enabled_ ? "Yes" : "No"));
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.6.2 Client System Menu PDU (TS_RAIL_ORDER_SYSMENU)
// ======================================================================

// The Client System Menu PDU packet is sent from the client to the server
//  when a local RAIL window on the client receives a command to display its
//  System menu. This command is forwarded to the server via the System menu
//  PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |              Left             |              Top              |
// +-------------------------------+-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_SYSMENU (0x000C).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server that SHOULD display its System menu.

// Left (2 bytes): A 16-bit signed integer. The x-coordinate of the top-left
//  corner at which the System menu SHOULD be displayed. Specified in screen
//  coordinates.

// Top (2 bytes): A 16-bit signed integer. The y-coordinate of the top-left
//  corner at which the System menu SHOULD be displayed. Specified in screen
//  coordinates.

class ClientSystemMenuPDU {
    mutable uint32_t WindowId_ = 0;
            int16_t  Left_     = 0;
            int16_t  Top_      = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_sint16_le(this->Left_);
        stream.out_sint16_le(this->Top_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 8;    // WindowId(4) + Left(2) + Top(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client System Menu PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
        this->Left_     = stream.in_sint16_le();
        this->Top_      = stream.in_sint16_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_SYSMENU; }

    uint32_t WindowId() const { return this->WindowId_; }

    int16_t Left() const { return this->Left_; }

    int16_t Top() const { return this->Top_; }

    static size_t size() {
        return 8;   // WindowId(4) + Left(2) + Top(2)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientSystemMenuPDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientSystemMenuPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X Left=%d Top=%d",
            this->WindowId_, this->Left_, this->Top_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.6.3 Client System Command PDU
//  (TS_RAIL_ORDER_SYSCOMMAND)
// =================================================

// The Client System Command PDU packet is sent from the client to the server
//  when a local RAIL window on the client receives a command to perform an
//  action on the window, such as minimize or maximize. This command is
//  forwarded to the server via the System Command PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |            Command            |
// +-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_SYSCOMMAND (0x0004).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server to activate or deactivate.

// Command (2 bytes): An unsigned 16-bit integer. Specifies the type of
//  command. The field MUST be one of the following values.

//  +-------------+----------------------------------------------------------+
//  | Value       | Meaning                                                  |
//  +-------------+----------------------------------------------------------+
//  | SC_SIZE     | Resize the window.                                       |
//  | 0xF000      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_MOVE     | Move the window.                                         |
//  | 0xF010      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_MINIMIZE | Minimize the window.                                     |
//  | 0xF020      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_MAXIMIZE | Maximize the window.                                     |
//  | 0xF030      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_CLOSE    | Close the window.                                        |
//  | 0xF060      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_KEYMENU  | The ALT + SPACE key combination was pressed; display the |
//  | 0xF100      | window's system menu.                                    |
//  +-------------+----------------------------------------------------------+
//  | SC_RESTORE  | Restore the window to its original shape and size.       |
//  | 0xF120      |                                                          |
//  +-------------+----------------------------------------------------------+
//  | SC_DEFAULT  | Perform the default action of the window's system menu.  |
//  | 0xF160      |                                                          |
//  +-------------+----------------------------------------------------------+

enum {
      SC_SIZE     = 0xF000
    , SC_MOVE     = 0xF010
    , SC_MINIMIZE = 0xF020
    , SC_MAXIMIZE = 0xF030
    , SC_CLOSE    = 0xF060
    , SC_KEYMENU  = 0xF100
    , SC_RESTORE  = 0xF120
    , SC_DEFAULT  = 0xF160
};

static inline
const char* get_RAIL_Command_name(uint16_t Command) {
    switch (Command) {
        case SC_SIZE:     return "SC_SIZE";
        case SC_MOVE:     return "SC_MOVE";
        case SC_MINIMIZE: return "SC_MINIMIZE";
        case SC_MAXIMIZE: return "SC_MAXIMIZE";
        case SC_CLOSE:    return "SC_CLOSE";
        case SC_KEYMENU:  return "SC_KEYMENU";
        case SC_RESTORE:  return "SC_RESTORE";
        case SC_DEFAULT:  return "SC_DEFAULT";
        default:          return "<unknown>";
    }
}

class ClientSystemCommandPDU {
    mutable uint32_t WindowId_ = 0;
            uint16_t Command_  = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint16_le(this->Command_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 6;    // WindowId(4) + Command(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client System Command PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
        this->Command_  = stream.in_uint16_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_SYSCOMMAND; }

    uint32_t WindowId() const { return this->WindowId_; }

    void WindowId(uint32_t WindowId_) { this->WindowId_ = WindowId_; }

    uint16_t Command() const { return this->Command_; }

    void Command(uint16_t Command_) { this->Command_ = Command_; }

    static size_t size() {
        return 6;   // WindowId(4) + Command(2)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientSystemCommandPDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientSystemCommandPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X Command=%s(0x%04X)",
            this->WindowId_, ::get_RAIL_Command_name(this->Command_),
            this->Command_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.6.4 Client Notify Event PDU
//  (TS_RAIL_ORDER_NOTIFY_EVENT)
// ===============================================

// The Client Notify Event PDU packet is sent from a client to a server when
//  a local RAIL Notification Icon on the client receives a keyboard or mouse
//  message from the user. This notification is forwarded to the server via
//  the Notify Event PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +---------------------------------------------------------------+
// |                          NotifyIconId                         |
// +---------------------------------------------------------------+
// |                            Message                            |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_NOTIFY_EVENT (0x0006).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the associated
//  window on the server that owns the notification icon being specified in
//  the PDU.

// NotifyIconId (4 bytes): An unsigned 32-bit integer. The ID of the
//  associated notification icon on the server that SHOULD receive the
//  keyboard or mouse interaction.

// Message (4 bytes): An unsigned 32-bit integer. The message being sent to
//  the notification icon on the server.

//  +----------------------+---------------------------------------------------+
//  | Value                | Meaning                                           |
//  +----------------------+---------------------------------------------------+
//  | WM_LBUTTONDOWN       | The user pressed the left mouse button in the     |
//  | 0x00000201           | client area of the notification icon.             |
//  +----------------------+---------------------------------------------------+
//  | WM_LBUTTONUP         | The user released the left mouse button while the |
//  | 0x00000202           | cursor was in the client area of the notification |
//  |                      | icon.                                             |
//  +----------------------+---------------------------------------------------+
//  | WM_RBUTTONDOWN       | The user pressed the right mouse button in the    |
//  | 0x00000204           | client area of the notification icon.             |
//  +----------------------+---------------------------------------------------+
//  | WM_RBUTTONUP         | The user released the right mouse button while    |
//  | 0x00000205           | the cursor was in the client area of the          |
//  |                      | notification icon.                                |
//  +----------------------+---------------------------------------------------+
//  | WM_CONTEXTMENU       | The user selected a notification icon’s shortcut  |
//  | 0x0000007B           | menu with the keyboard. This message is sent only |
//  |                      | for notification icons that follow Windows 2000   |
//  |                      | behavior (see Version field in section            |
//  |                      | 2.2.1.3.2.2.1).                                   |
//  +----------------------+---------------------------------------------------+
//  | WM_LBUTTONDBLCLK     | The user double-clicked the left mouse button in  |
//  | 0x00000203           | the client area of the notification icon.         |
//  +----------------------+---------------------------------------------------+
//  | WM_RBUTTONDBLCLK     | The user double-clicked the right mouse button in |
//  | 0x00000206           | the client area of the notification icon.         |
//  +----------------------+---------------------------------------------------+
//  | NIN_SELECT           | The user selected a notification icon with the    |
//  | 0x00000400           | mouse and activated it with the ENTER key. This   |
//  |                      | message is sent only for notification icons that  |
//  |                      | follow Windows 2000 behavior (see Version field   |
//  |                      | in section 2.2.1.3.2.2.1).                        |
//  +----------------------+---------------------------------------------------+
//  | NIN_KEYSELECT        | The user selected a notification icon with the    |
//  | 0x00000401           | keyboard and activated it with the SPACEBAR or    |
//  |                      | ENTER key. This message is sent only for          |
//  |                      | notification icons that follow Windows 2000       |
//  |                      | behavior (see Version field in section            |
//  |                      | 2.2.1.3.2.2.1).                                   |
//  +----------------------+---------------------------------------------------+
//  | NIN_BALLOONSHOW      | The user passed the mouse pointer over an icon    |
//  | 0x00000402           | with which a balloon tooltip is associated (see   |
//  |                      | InfoTip field in section 2.2.1.3.2.2.1), and the  |
//  |                      | balloon tooltip was shown. This message is sent   |
//  |                      | only for notification icons that follow Windows   |
//  |                      | 2000 behavior (see Version field in section       |
//  |                      | 2.2.1.3.2.2.1).                                   |
//  +----------------------+---------------------------------------------------+
//  | NIN_BALLOONHIDE      | The icon's balloon tooltip disappeared because,   |
//  | 0x00000403           | for example, the icon was deleted. This message   |
//  |                      | is not sent if the balloon is dismissed because   |
//  |                      | of a timeout or mouse click by the user. This     |
//  |                      | message is sent only for notification icons that  |
//  |                      | follow Windows 2000 behavior (see Version field   |
//  |                      | in section 2.2.1.3.2.2.1).                        |
//  +----------------------+---------------------------------------------------+
//  | NIN_BALLOONTIMEOUT   | The icon's balloon tooltip was dismissed because  |
//  | 0x00000404           | of a timeout. This message is sent only for       |
//  |                      | notification icons that follow Windows 2000       |
//  |                      | behavior (see Version field in section            |
//  |                      | 2.2.1.3.2.2.1).                                   |
//  +----------------------+---------------------------------------------------+
//  | NIN_BALLOONUSERCLICK | User dismissed the balloon by clicking the mouse. |
//  | 0x00000405           | This message is sent only for notification icons  |
//  |                      | that follow Windows 2000 behavior (see Version    |
//  |                      | field in section 2.2.1.3.2.2.1).                  |
//  +----------------------+---------------------------------------------------+

enum {
      WM_LBUTTONDOWN       = 0x00000201
    , WM_LBUTTONUP         = 0x00000202
    , WM_RBUTTONDOWN       = 0x00000204
    , WM_RBUTTONUP         = 0x00000205
    , WM_CONTEXTMENU       = 0x0000007B
    , WM_LBUTTONDBLCLK     = 0x00000203
    , WM_RBUTTONDBLCLK     = 0x00000206
    , NIN_SELECT           = 0x00000400
    , NIN_KEYSELECT        = 0x00000401
    , NIN_BALLOONSHOW      = 0x00000402
    , NIN_BALLOONHIDE      = 0x00000403
    , NIN_BALLOONTIMEOUT   = 0x00000404
    , NIN_BALLOONUSERCLICK = 0x00000405
};

static inline
const char* get_RAIL_Message_name(uint32_t Message) {
    switch (Message) {
        case WM_LBUTTONDOWN:       return "WM_LBUTTONDOWN";
        case WM_LBUTTONUP:         return "WM_LBUTTONUP";
        case WM_RBUTTONDOWN:       return "WM_RBUTTONDOWN";
        case WM_RBUTTONUP:         return "WM_RBUTTONUP";
        case WM_CONTEXTMENU:       return "WM_CONTEXTMENU";
        case WM_LBUTTONDBLCLK:     return "WM_LBUTTONDBLCLK";
        case WM_RBUTTONDBLCLK:     return "WM_RBUTTONDBLCLK";
        case NIN_SELECT:           return "NIN_SELECT";
        case NIN_KEYSELECT:        return "NIN_KEYSELECT";
        case NIN_BALLOONSHOW:      return "NIN_BALLOONSHOW";
        case NIN_BALLOONHIDE:      return "NIN_BALLOONHIDE";
        case NIN_BALLOONTIMEOUT:   return "NIN_BALLOONTIMEOUT";
        case NIN_BALLOONUSERCLICK: return "NIN_BALLOONUSERCLICK";
        default:                   return "<unknown>";
    }
}

class ClientNotifyEventPDU {
    mutable uint32_t WindowId_     = 0;
            uint32_t NotifyIconId_ = 0;
            uint32_t Message_      = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint16_le(this->NotifyIconId_);
        stream.out_uint16_le(this->Message_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;   // WindowId(4) + NotifyIconId(4) + Message(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Notify Event PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_     = stream.in_uint32_le();
        this->NotifyIconId_ = stream.in_uint32_le();
        this->Message_      = stream.in_uint32_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_NOTIFY_EVENT; }

    uint32_t WindowId() const { return this->WindowId_; }

    uint32_t NotifyIconId() const { return this->NotifyIconId_; }

    uint32_t Message() const { return this->Message_; }

    static size_t size() {
        return 12;  // WindowId(4) + NotifyIconId(4) + Message(4)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientNotifyEventPDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "NonMonitoredDesktop: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X NotifyIconId=%u Message=%s(%u)",
            this->WindowId_, this->NotifyIconId_, ::get_RAIL_Message_name(this->Message_),
            this->Message_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.6.5 Client Get Application ID PDU
//  (TS_RAIL_ORDER_GET_APPID_REQ)
// =====================================================

// The Client Get Application ID PDU is sent from a client to a server. This
//  PDU requests information from the server about the Application ID that
//  the window SHOULD<15> have on the client.

// The server MAY ignore this PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_GET_APPID_REQ (0x000E).

// WindowId (4 bytes): An unsigned 32-bit integer specifying the ID of the
//  associated window on the server that requires needs an Application ID.

class ClientGetApplicationIDPDU {
    mutable uint32_t WindowId_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // WindowId(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Get Application ID PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_GET_APPID_REQ; }

    static size_t size() {
        return 4;   // WindowId(4)
    }

    uint32_t WindowId() const { return this->WindowId_; }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientGetApplicationIDPDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientGetApplicationIDPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X", this->WindowId_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.7.1 Server Min Max Info PDU (TS_RAIL_ORDER_MINMAXINFO)
// ==========================================================================

// The Server Min Max Info PDU is sent from a server to a client when a
//  window move or resize on the server is being initiated. This PDU contains
//  information about the minimum and maximum extents to which the window can
//  be moved or sized.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |            MaxWidth           |           MaxHeight           |
// +-------------------------------+-------------------------------+
// |            MaxPosX            |            MaxPosY            |
// +-------------------------------+-------------------------------+
// |         MinTrackWidth         |         MinTrackHeight        |
// +-------------------------------+-------------------------------+
// |         MaxTrackWidth         |         MaxTrackHeight        |
// +-------------------------------+-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_MINMAXINFO (0x000A).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server that is being moved or resized.

// MaxWidth (2 bytes): An unsigned 16-bit integer. The width of the maximized
//  window.

// MaxHeight (2 bytes): An unsigned 16-bit integer. The height of the
//  maximized window.

// MaxPosX (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  top-left corner of the maximized window.

// MaxPosY (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  top-left corner of the maximized window.

// MinTrackWidth (2 bytes): An unsigned 16-bit integer. The minimum width to
//  which the window can be resized.

// MinTrackHeight (2 bytes): An unsigned 16-bit integer. The minimum height
//  to which the window can be resized.

// MaxTrackWidth (2 bytes): An unsigned 16-bit integer. The maximum width to
//  which the window can be resized.

// MaxTrackHeight (2 bytes): An unsigned 16-bit integer. The maximum height
//  to which the window can be resized.

class ServerMinMaxInfoPDU {
    mutable uint32_t WindowId_       = 0;
            uint16_t MaxWidth_       = 0;
            uint16_t MaxHeight_      = 0;
            uint16_t MaxPosX_        = 0;
            uint16_t MaxPosY_        = 0;
            uint16_t MinTrackWidth_  = 0;
            uint16_t MinTrackHeight_ = 0;
            uint16_t MaxTrackWidth_  = 0;
            uint16_t MaxTrackHeight_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint16_le(this->MaxWidth_);
        stream.out_uint16_le(this->MaxHeight_);
        stream.out_uint16_le(this->MaxPosX_);
        stream.out_uint16_le(this->MaxPosY_);
        stream.out_uint16_le(this->MinTrackWidth_);
        stream.out_uint16_le(this->MinTrackHeight_);
        stream.out_uint16_le(this->MaxTrackWidth_);
        stream.out_uint16_le(this->MaxTrackHeight_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 20;   // WindowId(4) + MaxWidth(2) +
                                            //  MaxHeight(2) + MaxPosX(2) +
                                            //  MaxPosY(2) + MinTrackWidth(2) +
                                            //  MinTrackHeight(2) + MaxTrackWidth(2) +
                                            //  MaxTrackHeight(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server Min Max Info PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_       = stream.in_uint32_le();
        this->MaxWidth_       = stream.in_uint16_le();
        this->MaxHeight_      = stream.in_uint16_le();
        this->MaxPosX_        = stream.in_uint16_le();
        this->MaxPosY_        = stream.in_uint16_le();
        this->MinTrackWidth_  = stream.in_uint16_le();
        this->MinTrackHeight_ = stream.in_uint16_le();
        this->MaxTrackWidth_  = stream.in_uint16_le();
        this->MaxTrackHeight_ = stream.in_uint16_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_MINMAXINFO; }

    static size_t size() {
        return 20;  // WindowId(4) + MaxWidth(2) + MaxHeight(2) +
                    //  MaxPosX(2) + MaxPosY(2) + MinTrackWidth(2) +
                    //  MinTrackHeight(2) + MaxTrackWidth(2) +
                    //  MaxTrackHeight(2)
    }

    uint32_t WindowId() const { return this->WindowId_; }

    void WindowId(uint32_t WindowId_) { this->WindowId_ = WindowId_; }

    uint16_t MaxWidth() const { return this->MaxWidth_; }

    void MaxWidth(uint16_t MaxWidth_) { this->MaxWidth_ = MaxWidth_; }

    uint16_t MaxHeight() const { return this->MaxHeight_; }

    void MaxHeight(uint16_t MaxHeight_) { this->MaxHeight_ = MaxHeight_; }

    uint16_t MaxPosX() const { return this->MaxPosX_; }

    void MaxPosX(uint16_t MaxPosX_) { this->MaxPosX_ = MaxPosX_; }

    uint16_t MaxPosY() const { return this->MaxPosY_; }

    void MaxPosY(uint16_t MaxPosY_) { this->MaxPosY_ = MaxPosY_; }

    uint16_t MinTrackWidth() const { return this->MinTrackWidth_; }

    void MinTrackWidth(uint16_t MinTrackWidth_) { this->MinTrackWidth_ = MinTrackWidth_; }

    uint16_t MinTrackHeight() const { return this->MinTrackHeight_; }

    void MinTrackHeight(uint16_t MinTrackHeight_) { this->MinTrackHeight_ = MinTrackHeight_; }

    uint16_t MaxTrackWidth() const { return this->MaxTrackWidth_; }

    void MaxTrackWidth(uint16_t MaxTrackWidth_) { this->MaxTrackWidth_ = MaxTrackWidth_; }

    uint16_t MaxTrackHeight() const { return this->MaxTrackHeight_; }

    void MaxTrackHeight(uint16_t MaxTrackHeight_) { this->MaxTrackHeight_ = MaxTrackHeight_; }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t client_window_id = rail_window_id_manager.get_client_window_id_ex(this->WindowId_);

        assert(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != client_window_id);

        if (client_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = client_window_id;

        return true;
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerMinMaxInfoPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));


        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X MaxWidth=%u MaxHeight=%u MaxPosX=%u MaxPosY=%u "
                "MinTrackWidth=%u MinTrackHeight=%u MaxTrackWidth=%u "
                "MaxTrackHeight=%u",
            this->WindowId_, this->MaxWidth_, this->MaxHeight_, this->MaxPosX_,
            this->MaxPosY_, this->MinTrackWidth_, this->MinTrackHeight_,
            this->MaxTrackWidth_, this->MaxTrackHeight_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.7.2 Server Move/Size Start PDU
//  (TS_RAIL_ORDER_LOCALMOVESIZE)
// ==================================================

// The Server Move/Size Start PDU packet is sent by the server when a window
//  on the server is beginning a move or resize. The client uses this
//  information to initiate a local move or resize of the corresponding local
//  window.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |        IsMoveSizeStart        |          MoveSizeType         |
// +-------------------------------+-------------------------------+
// |              PosX             |              PosY             |
// +-------------------------------+-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_LOCALMOVESIZE (0x0009).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server that is being moved or resized.

// IsMoveSizeStart (2 bytes): An unsigned 16-bit integer. Indicates that the
//  move/size is beginning; MUST be set to a nonzero value.

// MoveSizeType (2 bytes): An unsigned 16-bit integer. Indicates the type of
//  the move/size. This value determines the meaning of the fields PosX and
//  PosY.

//  +-----------------------+--------------------------------------------------+
//  | Value                 | Meaning
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_LEFT        | The left edge of the window is being sized.      |
//  | 0x0001                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_RIGHT       | The right edge of the window is being sized.     |
//  | 0x0002                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOP         | The top edge of the window is being sized.       |
//  | 0x0003                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOPLEFT     | The top-left corner of the window is being       |
//  | 0x0004                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOPRIGHT    | The top-right corner of the window is being      |
//  | 0x0005                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOM      | The bottom edge of the window is being sized.    |
//  | 0x0006                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMLEFT  | The bottom-left corner of the window is being    |
//  | 0x0007                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMRIGHT | The bottom-right corner of the window is being   |
//  | 0x0008                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_MOVE        | The window is being moved by using the mouse.    |
//  | 0x0009                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_KEYMOVE     | The window is being moved by using the keyboard. |
//  | 0x000A                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_KEYSIZE     | The window is being resized by using the         |
//  | 0x000B                | keyboard.                                        |
//  +-----------------------+--------------------------------------------------+

enum {
      RAIL_WMSZ_LEFT        = 0x0001
    , RAIL_WMSZ_RIGHT       = 0x0002
    , RAIL_WMSZ_TOP         = 0x0003
    , RAIL_WMSZ_TOPLEFT     = 0x0004
    , RAIL_WMSZ_TOPRIGHT    = 0x0005
    , RAIL_WMSZ_BOTTOM      = 0x0006
    , RAIL_WMSZ_BOTTOMLEFT  = 0x0007
    , RAIL_WMSZ_BOTTOMRIGHT = 0x0008
    , RAIL_WMSZ_MOVE        = 0x0009
    , RAIL_WMSZ_KEYMOVE     = 0x000A
    , RAIL_WMSZ_KEYSIZE     = 0x000B
};

// PosX (2 bytes): An unsigned 16-bit integer. The meaning of this field
//  depends upon the value of the MoveSizeType field.

//  +-----------------------+-------------------------------------------------+
//  | Value                 | Meaning                                         |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_LEFT        | The x-coordinate of the last mouse button-down. |
//  | 0x0001                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_RIGHT       | The x-coordinate of the last mouse button-down. |
//  | 0x0002                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOP         | The x-coordinate of the last mouse button-down. |
//  | 0x0003                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOPLEFT     | The x-coordinate of the last mouse button-down. |
//  | 0x0004                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOPRIGHT    | The x-coordinate of the last mouse button-down. |
//  | 0x0005                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOM      | The x-coordinate of the last mouse button-down. |
//  | 0x0006                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMLEFT  | The x-coordinate of the last mouse button-down. |
//  | 0x0007                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMRIGHT | The x-coordinate of the last mouse button-down. |
//  | 0x0008                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_MOVE        | The horizontal offset between the window's      |
//  | 0x0009                | top-left edge and the current mouse position.   |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_KEYMOVE     | The x-coordinate of the last mouse button-down. |
//  | 0x000A                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_KEYSIZE     | The x-coordinate of the last mouse button-down. |
//  | 0x000B                |                                                 |
//  +-----------------------+-------------------------------------------------+

// PosY (2 bytes): An unsigned 16-bit integer. The meaning of this field
//  depends on the value of the MoveSizeType field.

//  +-----------------------+-------------------------------------------------+
//  | Value                 | Meaning                                         |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_LEFT        | The y-coordinate of the last mouse button-down. |
//  | 0x0001                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_RIGHT       | The y-coordinate of the last mouse button-down. |
//  | 0x0002                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOP         | The y-coordinate of the last mouse button-down. |
//  | 0x0003                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOPLEFT     | The y-coordinate of the last mouse button-down. |
//  | 0x0004                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_TOPRIGHT    | The y-coordinate of the last mouse button-down. |
//  | 0x0005                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOM      | The y-coordinate of the last mouse button-down. |
//  | 0x0006                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMLEFT  | The y-coordinate of the last mouse button-down. |
//  | 0x0007                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMRIGHT | The y-coordinate of the last mouse button-down. |
//  | 0x0008                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_MOVE        | The vertical offset between the window's        |
//  | 0x0009                | top-left edge and the current mouse position.   |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_KEYMOVE     | The y-coordinate of the last mouse button-down. |
//  | 0x000A                |                                                 |
//  +-----------------------+-------------------------------------------------+
//  | RAIL_WMSZ_KEYSIZE     | The y-coordinate of the last mouse button-down. |
//  | 0x000B                |                                                 |
//  +-----------------------+-------------------------------------------------+

static inline
const char* get_RAIL_MoveSizeType_name(uint16_t MoveSizeType) {
    switch (MoveSizeType) {
        case RAIL_WMSZ_LEFT:        return "RAIL_WMSZ_LEFT";
        case RAIL_WMSZ_RIGHT:       return "RAIL_WMSZ_RIGHT";
        case RAIL_WMSZ_TOP:         return "RAIL_WMSZ_TOP";
        case RAIL_WMSZ_TOPLEFT:     return "RAIL_WMSZ_TOPLEFT";
        case RAIL_WMSZ_TOPRIGHT:    return "RAIL_WMSZ_TOPRIGHT";
        case RAIL_WMSZ_BOTTOM:      return "RAIL_WMSZ_BOTTOM";
        case RAIL_WMSZ_BOTTOMLEFT:  return "RAIL_WMSZ_BOTTOMLEFT";
        case RAIL_WMSZ_BOTTOMRIGHT: return "RAIL_WMSZ_BOTTOMRIGHT";
        case RAIL_WMSZ_MOVE:        return "RAIL_WMSZ_MOVE";
        case RAIL_WMSZ_KEYMOVE:     return "RAIL_WMSZ_KEYMOVE";
        case RAIL_WMSZ_KEYSIZE:     return "RAIL_WMSZ_KEYSIZE";
        default:                    return "<unknown>";
    }
}

// [MS-RDPERP] - 2.2.2.7.3 Server Move/Size End PDU
//  (TS_RAIL_ORDER_LOCALMOVESIZE)
// ================================================

// The Server Move/Size End PDU is sent by the server when a window on the
//  server is completing a move or resize. The client uses this information
//  to end a local move/resize of the corresponding local window.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |        IsMoveSizeStart        |          MoveSizeType         |
// +-------------------------------+-------------------------------+
// |            TopLeftX           |            TopLeftY           |
// +-------------------------------+-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_LOCALMOVESIZE (0x0009).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server that is being moved or resized.

// IsMoveSizeStart (2 bytes): An unsigned 16-bit integer. Indicates the move
//  or resize is ending. This field MUST be set to 0.

// MoveSizeType (2 bytes): An unsigned 16-bit integer. Indicates the type of
//  the move/size.

//  +-----------------------+--------------------------------------------------+
//  | Value                 | Meaning                                          |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_LEFT        | The left edge of the window is being sized.      |
//  | 0x0001                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_RIGHT       | The right edge of the window is being sized.     |
//  | 0x0002                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOP         | The top edge of the window is being sized.       |
//  | 0x0003                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOPLEFT     | The top-left corner of the window is being       |
//  | 0x0004                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_TOPRIGHT    | The top-right corner of the window is being      |
//  | 0x0005                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOM      | The bottom edge of the window is being sized.    |
//  | 0x0006                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMLEFT  | The bottom-left corner of the window is being    |
//  | 0x0007                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_BOTTOMRIGHT | The bottom-right corner of the window is being   |
//  | 0x0008                | sized.                                           |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_MOVE        | The window is being moved by using the mouse.    |
//  | 0x0009                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_KEYMOVE     | The window is being moved by using the keyboard. |
//  | 0x000A                |                                                  |
//  +-----------------------+--------------------------------------------------+
//  | RAIL_WMSZ_KEYSIZE     | The window is being resized by using the         |
//  | 0x000B                | keyboard.                                        |
//  +-----------------------+--------------------------------------------------+

// TopLeftX (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  moved or resized window's top-left corner.

// TopLeftY (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  moved or resized window's top-left corner.

class ServerMoveSizeStartOrEndPDU {
    mutable uint32_t WindowId_        = 0;
            uint16_t IsMoveSizeStart_ = 0;
            uint16_t MoveSizeType_    = 0;
            uint16_t PosXOrTopLeftX_  = 0;
            uint16_t PosYOrTopLeftY_  = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint16_le(this->IsMoveSizeStart_);
        stream.out_uint16_le(this->MoveSizeType_);
        stream.out_uint16_le(this->PosXOrTopLeftX_);
        stream.out_uint16_le(this->PosYOrTopLeftY_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;   // WindowId(4) +
                                            //  IsMoveSizeStart(2) +
                                            //  MoveSizeType(2) + PosX/TopLeftX(2) +
                                            //  PosY/TopLeftY(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server Move/Size Start/End PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_        = stream.in_uint32_le();
        this->IsMoveSizeStart_ = stream.in_uint16_le();
        this->MoveSizeType_    = stream.in_uint16_le();
        this->PosXOrTopLeftX_  = stream.in_uint16_le();
        this->PosYOrTopLeftY_  = stream.in_uint16_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_LOCALMOVESIZE; }

    static size_t size() {
        return 12;  // WindowId(4) + IsMoveSizeStart(2) + MoveSizeType(2) +
                    //  PosX/TopLeftX(2) + PosY/TopLeftY(2)
    }

    uint32_t WindowId() const { return this->WindowId_; }

    void WindowId(uint32_t WindowId_) { this->WindowId_ = WindowId_; }

    uint16_t IsMoveSizeStart() const { return this->IsMoveSizeStart_; }

    void IsMoveSizeStart(uint16_t IsMoveSizeStart_) { this->IsMoveSizeStart_ = IsMoveSizeStart_; }

    uint16_t MoveSizeType() const { return this->MoveSizeType_; }

    void MoveSizeType(uint16_t MoveSizeType_) { this->MoveSizeType_ = MoveSizeType_; }

    uint16_t PosXOrTopLeftX() const { return this->PosXOrTopLeftX_; }

    void PosXOrTopLeftX(uint16_t PosXOrTopLeftX_) { this->PosXOrTopLeftX_ = PosXOrTopLeftX_; }

    uint16_t PosYOrTopLeftY() const { return this->PosYOrTopLeftY_; }

    void PosYOrTopLeftY(uint16_t PosYOrTopLeftY_) { this->PosYOrTopLeftY_ = PosYOrTopLeftY_; }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t client_window_id = rail_window_id_manager.get_client_window_id_ex(this->WindowId_);

        assert(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != client_window_id);

        if (client_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = client_window_id;

        return true;
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerMoveSizeStartOrEndPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));


        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X IsMoveSizeStart=%s(%u) MoveSizeType=%s(%u) PosX/TopLeftX=%u PosY/TopLeftY=%u",
            this->WindowId_, (this->IsMoveSizeStart_ ? "Yes" : "No"), this->IsMoveSizeStart_,
            ::get_RAIL_MoveSizeType_name(this->MoveSizeType_),
            this->MoveSizeType_, this->PosXOrTopLeftX_, this->PosYOrTopLeftY_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.7.4 Client Window Move PDU (TS_RAIL_ORDER_WINDOWMOVE)
// =========================================================================

// The Client Window Move PDU packet is sent from the client to the server
//  when a local window is ending a move or resize. The client communicates
//  the locally moved or resized window's position to the server by using
//  this packet. The server uses this information to reposition its window.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +-------------------------------+-------------------------------+
// |              Left             |              Top              |
// +-------------------------------+-------------------------------+
// |             Right             |             Bottom            |
// +-------------------------------+-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_WINDOWMOVE (0x0008).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the window on
//  the server corresponding to the local window that was moved or resized.

// Left (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  top-left corner of the window's new position.

// Top (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  top-left corner of the window's new position.

// Right (2 bytes): An unsigned 16-bit integer. The x-coordinate of the
//  bottom-right corner of the window's new position.

// Bottom (2 bytes): An unsigned 16-bit integer. The y-coordinate of the
//  bottom-right corner of the window's new position.

class ClientWindowMovePDU {
    mutable uint32_t WindowId_ = 0;
            uint16_t Left_     = 0;
            uint16_t Top_      = 0;
            uint16_t Right_    = 0;
            uint16_t Bottom_   = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint16_le(this->Left_);
        stream.out_uint16_le(this->Top_);
        stream.out_uint16_le(this->Right_);
        stream.out_uint16_le(this->Bottom_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 12;   // WindowId(4) + Left(2) +
                                            //  Top(2) + Right(2) + Bottom(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Window Move PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
        this->Left_     = stream.in_uint16_le();
        this->Top_      = stream.in_uint16_le();
        this->Right_    = stream.in_uint16_le();
        this->Bottom_   = stream.in_uint16_le();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_WINDOWMOVE; }

    uint32_t WindowId() const { return this->WindowId_; }

    void WindowId(uint32_t WindowId_) { this->WindowId_ = WindowId_; }

    uint16_t Left() const { return this->Left_; }

    void Left(uint32_t Left_) { this->Left_ = Left_; }

    uint16_t Top() const { return this->Top_; }

    void Top(uint32_t Top_) { this->Top_ = Top_; }

    uint16_t Right() const { return this->Right_; }

    void Right(uint32_t Right_) { this->Right_ = Right_; }

    uint16_t Bottom() const { return this->Bottom_; }

    void Bottom(uint32_t Bottom_) { this->Bottom_ = Bottom_; }

    static size_t size() {
        return 12;  // WindowId(4) + Left(2) + Top(2) +
                    //  Right(2) + Bottom(2)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "ClientWindowMovePDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

private:
    inline size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ClientWindowMovePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));


        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X Left=%u Top=%u Right=%u Bottom=%u",
            this->WindowId_, this->Left_, this->Top_, this->Right_, this->Bottom_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.8.1 Server Get Application ID Response PDU
//  (TS_RAIL_ORDER_GET_APPID_RESP)
// ==============================================================

// The Server Get Application ID Response PDU is sent from a server to a
//  client. This PDU MAY be sent to the client as a response to a Client Get
//  Application ID PDU.

// This PDU specifies the Application ID that the specified window SHOULD<16>
//  have on the client. The client MAY ignore this PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +---------------------------------------------------------------+
// |                   ApplicationId (512 bytes)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_GET_APPID_RESP (0x000F).

// WindowId (4 bytes): An unsigned 32-bit integer specifying the ID of the
//  associated window on the server whose Application ID is being sent to the
//  client.

// ApplicationId (512 bytes): A null-terminated string of Unicode characters
//  specifying the Application ID that the Client SHOULD associate with its
//  window, if it supports using the Application ID for identifying and
//  grouping windows.

class ServerGetApplicationIDResponsePDU {
    mutable uint32_t WindowId_ = 0;

    std::string application_id;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);

        uint8_t ApplicationId_unicode_data[512];
        ::memset(ApplicationId_unicode_data, 0, sizeof(ApplicationId_unicode_data));
        /*const size_t size_of_ApplicationId_unicode_data = */::UTF8toUTF16(
            this->application_id,
            ApplicationId_unicode_data, sizeof(ApplicationId_unicode_data) - 2 /* null-terminator */);

        stream.out_copy_bytes(ApplicationId_unicode_data, sizeof(ApplicationId_unicode_data));
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 516;    // WindowId(4) + ApplicationId(512)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server Get Application ID Response PDU: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();

        uint8_t ApplicationId_utf8_string[512 / 2 * 4 /* Max UTF-8 character length */]; // ApplicationId(512)
        const size_t length_of_ApplicationId_utf8_string = ::UTF16toUTF8(
            stream.get_current(),
            512 / 2 * 4 /* Max UTF-8 character length */,    // ApplicationId(512)
            ApplicationId_utf8_string,
            sizeof(ApplicationId_utf8_string));
        this->application_id.assign(::char_ptr_cast(ApplicationId_utf8_string),
            length_of_ApplicationId_utf8_string);
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_GET_APPID_RESP; }

    uint32_t WindowId() const { return this->WindowId_; }

    void WindowId(uint32_t WindowId_) { this->WindowId_ = WindowId_; }

    const char* ApplicationId() const { return this->application_id.c_str(); }

    void ApplicationId(const char * ApplicationId_) { this->application_id = ApplicationId_; }

    static size_t size() {
        return 516; // WindowId(4) + ApplicationId(512)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t client_window_id = rail_window_id_manager.get_client_window_id_ex(this->WindowId_);

        assert(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != client_window_id);

        if (client_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = client_window_id;

        return true;
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerGetApplicationIDResponsePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X ApplicationId=\"%s\"",
            this->WindowId_, this->application_id.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.9.1 Language Bar Information PDU
//  (TS_RAIL_ORDER_LANGBARINFO)
// ====================================================

// The Language Bar Information PDU is used to set the language bar status.
//  It is sent from a client to a server or a server to a client, but only
//  when both support the Language Bar docking capability
//  (TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED). This PDU contains information
//  about the language bar status.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                       LanguageBarStatus                       |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER (section 2.2.2.1) header. The
//  orderType field of the header MUST be set to TS_RAIL_ORDER_LANGBARINFO
//  (0x000D).

// LanguageBarStatus (4 bytes): An unsigned 32-bit integer. The possible
//  values are indicated in the table below. The server sends the
//  LanguageBarStatus it retrieves from the local language bar:

//  +--------------------------------+----------------------------------------+
//  | Value                          | Meaning                                |
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_SHOWNORMAL              | Display the language bar as a floating |
//  | 0x00000001                     | window. This constant cannot be        |
//  |                                | combined with the TF_SFT_DOCK,         |
//  |                                | TF_SFT_MINIMIZED, TF_SFT_HIDDEN, or    |
//  |                                | TF_SFT_DESKBAND constants.             |
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_DOCK                    | Dock the language bar in its own task  |
//  | 0x00000002                     | pane. This constant cannot be combined |
//  |                                | with the TF_SFT_SHOWNORMAL,            |
//  |                                | TF_SFT_MINIMIZED, TF_SFT_HIDDEN, or    |
//  |                                | TF_SFT_DESKBAND constants.<17>         |
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_MINIMIZED               | Display the language bar as a single
//  | 0x00000004                     | icon in the system tray. This constant
//  |                                | cannot be combined with the
//  |                                | TF_SFT_SHOWNORMAL, TF_SFT_DOCK,
//  |                                | TF_SFT_HIDDEN, or TF_SFT_DESKBAND
//  |                                | constants.
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_HIDDEN                  | Hide the language bar. This constant
//  | 0x00000008                     | cannot be combined with the
//  |                                | TF_SFT_SHOWNORMAL, TF_SFT_DOCK,
//  |                                | TF_SFT_MINIMIZED, or TF_SFT_DESKBAND
//  |                                | constants.
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_NOTRANSPARENCY          | Make the language bar opaque.
//  | 0x00000010                     |
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_LOWTRANSPARENCY         | Make the language bar partially
//  | 0x00000020                     | transparent.<18>
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_HIGHTRANSPARENCY        | Make the language bar highly
//  | 0x00000040                     | transparent.<19>
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_LABELS                  | Display text labels next to language
//  | 0x00000080                     | bar icons.
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_NOLABELS                | Hide language bar icon text labels.
//  | 0x00000100                     |
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_EXTRAICONSONMINIMIZED   | Display text service icons on the
//  | 0x00000200                     | taskbar when the language bar is
//  |                                | minimized.
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_NOEXTRAICONSONMINIMIZED | Hide text service icons on the taskbar
//  | 0x00000400                     | when the language bar is minimized.
//  +--------------------------------+----------------------------------------+
//  | TF_SFT_DESKBAND                | Dock the language bar in the system
//  | 0x00000800                     | task bar. This constant cannot be
//  |                                | combined with the TF_SFT_SHOWNORMAL,
//  |                                | TF_SFT_DOCK, TF_SFT_MINIMIZED, or
//  |                                | TF_SFT_HIDDEN constants.<20>
//  +--------------------------------+----------------------------------------+

enum {
      TF_SFT_SHOWNORMAL              = 0x00000001
    , TF_SFT_DOCK                    = 0x00000002
    , TF_SFT_MINIMIZED               = 0x00000004
    , TF_SFT_HIDDEN                  = 0x00000008
    , TF_SFT_NOTRANSPARENCY          = 0x00000010
    , TF_SFT_LOWTRANSPARENCY         = 0x00000020
    , TF_SFT_HIGHTRANSPARENCY        = 0x00000040
    , TF_SFT_LABELS                  = 0x00000080
    , TF_SFT_NOLABELS                = 0x00000100
    , TF_SFT_EXTRAICONSONMINIMIZED   = 0x00000200
    , TF_SFT_NOEXTRAICONSONMINIMIZED = 0x00000400
    , TF_SFT_DESKBAND                = 0x00000800
};

class LanguageBarInformationPDU {
    uint32_t LanguageBarStatus = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->LanguageBarStatus);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // LanguageBarStatus(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Language Bar Information PDU: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->LanguageBarStatus = stream.in_uint32_le();
    }

    static size_t size() {
        return 4;   // LanguageBarStatus(4)
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "LanguageBarInformationPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "LanguageBarStatus=0x%X", this->LanguageBarStatus);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.10.1.1 Globally Unique Identifier (GUID)
// ============================================================

// The GUID structure contains 128 bits that represent a globally unique
//  identifier that can be used to provide a distinctive reference number, as
//  defined in [MS-DTYP] section 2.3.4.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           codecGUID1                          |
// +-------------------------------+-------------------------------+
// |           codecGUID2          |           codecGUID3          |
// +---------------+---------------+---------------+---------------+
// |   codecGUID4  |   codecGUID5  |   codecGUID6  |   codecGUID7  |
// +---------------+---------------+---------------+---------------+
// |   codecGUID8  |   codecGUID9  |  codecGUID10  |  codecGUID11  |
// +---------------+---------------+---------------+---------------+

// codecGUID1 (4 bytes): A 32-bit, unsigned integer. The first GUID
//  component.

// codecGUID2 (2 bytes): A 16-bit, unsigned integer. The second GUID
//  component.

// codecGUID3 (2 bytes): A 16-bit, unsigned integer. The third GUID
//  component.

// codecGUID4 (1 byte): An 8-bit, unsigned integer. The fourth GUID
//  component.

// codecGUID5 (1 byte): An 8-bit, unsigned integer. The fifth GUID component.

// codecGUID6 (1 byte): An 8-bit, unsigned integer. The sixth GUID component.

// codecGUID7 (1 byte): An 8-bit, unsigned integer. The seventh GUID
//  component.

// codecGUID8 (1 byte): An 8-bit, unsigned integer. The eighth GUID
//  component.

// codecGUID9 (1 byte): An 8-bit, unsigned integer. The ninth GUID component.

// codecGUID10 (1 byte): An 8-bit, unsigned integer. The tenth GUID
//  component.

// codecGUID11 (1 byte): An 8-bit, unsigned integer. The eleventh GUID
//  component.

class GloballyUniqueIdentifier {
    uint32_t codecGUID1  = 0;
    uint16_t codecGUID2  = 0;
    uint16_t codecGUID3  = 0;
    uint8_t  codecGUID4  = 0;
    uint8_t  codecGUID5  = 0;
    uint8_t  codecGUID6  = 0;
    uint8_t  codecGUID7  = 0;
    uint8_t  codecGUID8  = 0;
    uint8_t  codecGUID9  = 0;
    uint8_t  codecGUID10 = 0;
    uint8_t  codecGUID11 = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->codecGUID1);
        stream.out_uint16_le(this->codecGUID2);
        stream.out_uint16_le(this->codecGUID3);
        stream.out_uint8(this->codecGUID4);
        stream.out_uint8(this->codecGUID5);
        stream.out_uint8(this->codecGUID6);
        stream.out_uint8(this->codecGUID7);
        stream.out_uint8(this->codecGUID8);
        stream.out_uint8(this->codecGUID9);
        stream.out_uint8(this->codecGUID10);
        stream.out_uint8(this->codecGUID11);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 16;   // codecGUID1(4) + codecGUID2(2) +
                                            //  codecGUID3(2) + codecGUID4(1) +
                                            //  codecGUID5(1) + codecGUID6(1) +
                                            //  codecGUID7(1) + codecGUID8(1) +
                                            //  codecGUID9(1) + codecGUID10(1) +
                                            //  codecGUID11(1)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Globally Unique Identifier: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->codecGUID1  = stream.in_uint32_le();
        this->codecGUID2  = stream.in_uint16_le();
        this->codecGUID3  = stream.in_uint16_le();
        this->codecGUID4  = stream.in_uint8();
        this->codecGUID5  = stream.in_uint8();
        this->codecGUID6  = stream.in_uint8();
        this->codecGUID7  = stream.in_uint8();
        this->codecGUID8  = stream.in_uint8();
        this->codecGUID9  = stream.in_uint8();
        this->codecGUID10 = stream.in_uint8();
        this->codecGUID11 = stream.in_uint8();
    }

    static size_t size() {
        return 16;  // codecGUID1(4) + codecGUID2(2) + codecGUID3(2) +
                    //  codecGUID4(1) + codecGUID5(1) + codecGUID6(1) +
                    //  codecGUID7(1) + codecGUID8(1) + codecGUID9(1) +
                    //  codecGUID10(1) + codecGUID11(1)
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "GloballyUniqueIdentifier=(");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "codecGUID1=0x%X codecGUID2=0x%X codecGUID3=0x%X "
                "codecGUID4=0x%X codecGUID5=0x%X codecGUID6=0x%X "
                "codecGUID7=0x%X codecGUID8=0x%X codecGUID9=0x%X "
                "codecGUID10=0x%X codecGUID11=0x%X",
            this->codecGUID1, this->codecGUID2, this->codecGUID3,
            this->codecGUID4, this->codecGUID5, this->codecGUID6,
            this->codecGUID7, this->codecGUID8, this->codecGUID9,
            this->codecGUID10, this->codecGUID11);
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length, ")");
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.10.1 Language Profile Information PDU
//  (TS_RAIL_ORDER_LANGUAGEIMEINFO)
// =========================================================

// The Language Profile Information PDU is used to send the current active
//  language profile of the client to the server. It is only sent when both
//  client and server support this capability
//  (TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED). This PDU contains
//  information about the current active language profile.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +-------------------------------+-------------------------------+
// |                          ProfileType                          |
// +-------------------------------+-------------------------------+
// |           LanguageID          |LanguageProfileCLSID (16 bytes)|
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |     ProfileGUID (16 bytes)    |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |         KeyboardLayout        |
// +-------------------------------+-------------------------------+
// |              ...              |
// +-------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_LANGUAGEIMEINFO (0x0011).

// ProfileType (4 bytes): An unsigned 4-byte integer that identifies the
//  profile type of the language. The value SHOULD be either
//  TF_PROFILETYPE_INPUTPROCESSOR (0x0001) or TF_PROFILETYPE_KEYBOARDLAYOUT
/// (0x0002).

//  +-------------------------------+---------------------------------------+
//  | Value                         | Meaning                               |
//  +-------------------------------+---------------------------------------+
//  | TF_PROFILETYPE_INPUTPROCESSOR | Indicates that the profile type is an |
//  | 0x00000001                    | input processor.                      |
//  +-------------------------------+---------------------------------------+
//  | TF_PROFILETYPE_KEYBOARDLAYOUT | Indicates that the profile type is a  |
//  | 0x00000002                    | keyboard layout.                      |
//  +-------------------------------+---------------------------------------+

enum {
      TF_PROFILETYPE_INPUTPROCESSOR = 0x00000001
    , TF_PROFILETYPE_KEYBOARDLAYOUT = 0x00000002
};

// LanguageID (2 bytes): An unsigned 2-byte integer. This is the language
//  identifier that identifies both the language and the country/region. For
//  a list of language identifiers, see [MSDN-MUI].

// LanguageProfileCLSID (16 bytes): A globally unique identifier (section
//  2.2.2.10.1.1) that uniquely identifies the text service of the client.
//  This field MUST be set to GUID_NULL if the ProfileType field is set to
//  TF_PROFILETYPE_KEYBOARDLAYOUT (0x0002).

//  +------------------------------------------------------------+---------------------------------------+
//  | Value                                                      | Meaning                               |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_NULL                                                  | Indicates that there is no input      |
//  | {0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, | processor.                            |
//  | 0x00, 0x00, 0x00}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_MSIME_JPN                                             | Indicates that the input processor is |
//  | {0x03B5835F, 0xF03C, 0x411B, 0x9C, 0xE2, 0xAA, 0x23, 0xE1, | Japanese.                             |
//  | 0x17, 0x1E, 0x36}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_MSIME_KOR                                             | Indicates that the input processor is |
//  | {0xA028AE76, 0x01B1, 0x46C2, 0x99, 0xC4, 0xAC, 0xD9, 0x85, | Korean.                               |
//  | 0x8A, 0xE0, 0x2}                                           |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_CHSIME                                                | Indicates that the input processor is |
//  | {0x81D4E9C9, 0x1D3B, 0x41BC, 0x9E, 0x6C, 0x4B, 0x40, 0xBF, | Chinese Simplified.                   |
//  | 0x79, 0xE3, 0x5E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_CHTIME                                                | Indicates that the input processor is |
//  | {0x531FDEBF, 0x9B4C, 0x4A43, 0xA2, 0xAA, 0x96, 0x0E, 0x8F, | Chinese (Taiwanese).                  |
//  | 0xCD, 0xC7, 0x32}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+

// ProfileGUID (16 bytes): A globally unique identifier (section
//  2.2.2.10.1.1) that uniquely identifies the language profile of the
//  client. This field MUST be set to GUID_NULL if the ProfileType field is
//  set to TF_PROFILETYPE_KEYBOARDLAYOUT (0x0002).

//  +------------------------------------------------------------+---------------------------------------+
//  | Value                                                      | Meaning                               |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_NULL                                                  | Indicates that there is no profile.   |
//  | {0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, |                                       |
//  | 0x00, 0x00, 0x00}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_NEWPHONETIC                                   | Indicates that the profile is new     |
//  | {0xB2F9C502, 0x1742, 0x11D4, 0x97, 0x90, 0x00, 0x80, 0xC8, | phonetic.                             |
//  | 0x82, 0x68, 0x7E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_CHANGJIE                                      | Indicates that the profile is         |
//  | {0x4BDF9F03, 0xC7D3, 0x11D4, 0xB2, 0xAB, 0x00, 0x80, 0xC8, | ChangJie.                             |
//  | 0x82, 0x68, 0x7E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_QUICK                                         | Indicates that the profile is Quick   |
//  | {0x6024B45F, 0x5C54, 0x11D4, 0xB9, 0x21, 0x00, 0x80, 0xC8, | type.                                 |
//  | 0x82, 0x68, 0x7E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_CANTONESE                                     | Indicates that the profile is         |
//  | {0x0AEC109C, 0x7E96, 0x11D4, 0xB2, 0xEF, 0x00, 0x80, 0xC8, | Cantonese.                            |
//  | 0x82, 0x68, 0x7E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_PINYIN                                        | Indicates that the profile is PinYin. |
//  | {0xF3BA9077, 0x6C7E, 0x11D4, 0x97, 0xFA, 0x00, 0x80, 0xC8, |                                       |
//  | 0x82, 0x68, 0x7E}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_SIMPLEFAST                                    | Indicates that the profile is         |
//  | {0xFA550B04, 0x5AD7, 0x411F, 0xA5, 0xAC, 0xCA, 0x03, 0x8E, | SimpleFast.                           |
//  | 0xC5, 0x15, 0xD7}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_GUID_PROFILE_MSIME_JPN                                | Indicates that the profile is         |
//  | {0xA76C93D9, 0x5523, 0x4E90, 0xAA, 0xFA, 0x4D, 0xB1, 0x12, | Microsoft Japanese IME.               |
//  | 0xF9, 0xAC, 0x76}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+
//  | GUID_PROFILE_MSIME_KOR                                     | Indicates that the profile is         |
//  | {0xB5FE1F02, 0xD5F2, 0x4445, 0x9C, 0x03, 0xC5, 0x68, 0xF2, | Microsoft Korean IME.                 |
//  | 0x3C, 0x99, 0xA1}                                          |                                       |
//  +------------------------------------------------------------+---------------------------------------+

// KeyboardLayout (4 bytes): An unsigned 4-byte integer. The active input
//  locale identifier, also known as the "HKL" (for example, 0x00010409
//  identifies a "United States-Dvorak" keyboard layout, while 0x00020418 is
//  a "Romanian (Programmers)" keyboard layout). For a list of input locale
//  identifiers, see [MSFT-DIL].

class LanguageProfileInformationPDU {
    uint32_t ProfileType = 0;

    uint16_t LanguageID = 0;

    GloballyUniqueIdentifier LanguageProfileCLSID;
    GloballyUniqueIdentifier ProfileGUID;

    uint32_t KeyboardLayout = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->ProfileType);
        stream.out_uint16_le(this->LanguageID);

        this->LanguageProfileCLSID.emit(stream);
        this->ProfileGUID.emit(stream);

        stream.out_uint32_le(this->KeyboardLayout);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 6;    // ProfileType(4) + LanguageID(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Language Profile Information PDU (1): expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->ProfileType = stream.in_uint32_le();
        this->LanguageID  = stream.in_uint16_le();

        this->LanguageProfileCLSID.receive(stream);
        this->ProfileGUID.receive(stream);

        {
            const unsigned expected = 4;    // KeyboardLayout(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Language Profile Information PDU (2): expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->KeyboardLayout = stream.in_uint32_le();
    }

    static size_t size() {
        return 6 +  // ProfileType(4) +
               2 +  // LanguageID(2) +
               GloballyUniqueIdentifier::size() +   // LanguageProfileCLSID
               GloballyUniqueIdentifier::size() +   // ProfileGUID
               4;   // KeyboardLayout(1)
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "GloballyUniqueIdentifier: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "ProfileType=%u LanguageID=0x%X ",
            this->ProfileType, this->LanguageID);
        length += ((result < size - length) ? result : (size - length - 1));

        result = this->LanguageProfileCLSID.str(buffer + length, size - length);
        length += ((result < size - length) ? result : (size - length - 1));
        result = this->ProfileGUID.str(buffer + length, size - length);
        length += ((result < size - length) ? result : (size - length - 1));
        result = ::snprintf(buffer + length, size - length,
            " KeyboardLayout=0x%X",
            this->KeyboardLayout);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.10.2 Compartment Status Information PDU
//  (TS_RAIL_ORDER_COMPARTMENTINFO_BODY)
// ===========================================================

// The Compartment Status Information PDU is used to send the current input
//  method editor (IME) status information. It is sent from a client to the
//  server, or from a server to the client, but only when client and server
//  both support this capability (TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED).
//  This PDU is used to send the current compartment values of the client or
//  server and is sent only if the current language profile type is
//  TF_PROFILETYPE_INPUTPROCESSOR (0x0001).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            ImeState                           |
// +---------------------------------------------------------------+
// |                          ImeConvMode                          |
// +---------------------------------------------------------------+
// |                        ImeSentenceMode                        |
// +---------------------------------------------------------------+
// |                            KANAMode                           |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER header. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_COMPARTMENTINFO (0x0012).

// ImeState (4 bytes): A 32-bit, unsigned integer. Indicates the open or
//  closed state of the IME.

//  +------------------+--------------------------+
//  | Value            | Meaning                  |
//  +------------------+--------------------------+
//  | IME_STATE_CLOSED | The IME state is closed. |
//  | 0x00000000       |                          |
//  +------------------+--------------------------+
//  | IME_STATE_OPEN   | The IME state is open.   |
//  | 0x00000001       |                          |
//  +------------------+--------------------------+

enum {
      IME_STATE_CLOSED = 0x00000000
    , IME_STATE_OPEN   = 0x00000001
};

// ImeConvMode (4 bytes): A 32-bit, unsigned integer. Indicates the IME
//  conversion mode.

//  +------------------------+-------------------------------------------------+
//  | Value                  | Meaning                                         |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_NATIVE       | The input mode is native. If not set, the input |
//  | 0x00000001             | mode is alphanumeric.                           |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_KATAKANA     | The input mode is Katakana. If not set, the     |
//  | 0x00000002             | input mode is Hiragana.                         |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_FULLSHAPE    | The input mode is full-width. If not set, the   |
//  | 0x00000008             | input mode is half-width.                       |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_ROMAN        | The input mode is Roman.                        |
//  | 0x00000010             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_CHARCODE     | Character-code input is in effect.              |
//  | 0x00000020             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_HANJACONVERT | Hanja conversion mode is in effect.             |
//  | 0x00000040             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_SOFTKBD      | A soft (on-screen) keyboard is being used.      |
//  | 0x00000080             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_NOCONVERSION | IME conversion is inactive (that is, the IME is |
//  | 0x00000100             | closed).                                        |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_EUDC         | End-User Defined Character (EUDC) conversion    |
//  | 0x00000200             | mode is in effect.                              |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_SYMBOL       | Symbol conversion mode is in effect.            |
//  | 0x00000400             |                                                 |
//  +------------------------+-------------------------------------------------+
//  | IME_CMODE_FIXED        | Fixed conversion mode is in effect.             |
//  | 0x00000800             |                                                 |
//  +------------------------+-------------------------------------------------+

enum {
      IME_CMODE_NATIVE       = 0x00000001
    , IME_CMODE_KATAKANA     = 0x00000002
    , IME_CMODE_FULLSHAPE    = 0x00000008
    , IME_CMODE_ROMAN        = 0x00000010
    , IME_CMODE_CHARCODE     = 0x00000020
    , IME_CMODE_HANJACONVERT = 0x00000040
    , IME_CMODE_SOFTKBD      = 0x00000080
    , IME_CMODE_NOCONVERSION = 0x00000100
    , IME_CMODE_EUDC         = 0x00000200
    , IME_CMODE_SYMBOL       = 0x00000400
    , IME_CMODE_FIXED        = 0x00000800
};

// ImeSentenceMode (4 bytes): An unsigned 4-byte integer that identifies the
//  sentence mode of the IME.

//  +-------------------------+------------------------------------------------+
//  | Flag                    | Meaning                                        |
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_NONE          | Indicates that the IME uses no information for |
//  | 0x00000000              | sentence.
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_PLURALCLAUSE  | Indicates that the IME uses plural clause      |
//  | 0x00000001              | information to carry out conversion            |
//  |                         | processing.                                    |
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_SINGLECONVERT | Indicates that the IME carries out conversion  |
//  | 0x00000002              | processing in single-character mode.           |
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_AUTOMATIC     | Indicates that the IME carries conversion      |
//  | 0x00000004              | processing in automatic mode.                  |
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_PHRASEPREDICT | Indicates that the IME uses phrase information |
//  | 0x00000008              | to predict the next character.                 |
//  +-------------------------+------------------------------------------------+
//  | IME_SMODE_CONVERSATION  | Indicates that the IME uses conversation mode. |
//  | 0x00000010              | This is useful for chat applications.          |
//  +-------------------------+------------------------------------------------+

enum {
      IME_SMODE_NONE          = 0x00000000
    , IME_SMODE_PLURALCLAUSE  = 0x00000001
    , IME_SMODE_SINGLECONVERT = 0x00000002
    , IME_SMODE_AUTOMATIC     = 0x00000004
    , IME_SMODE_PHRASEPREDICT = 0x00000008
    , IME_SMODE_CONVERSATION  = 0x00000010
};

// KANAMode (4 bytes): An unsigned 4-byte integer that identifies whether the
//  input mode is Romaji or KANA for Japanese text processors. The value is
//  0x0000 for all non-Japanese text processors.

//  +---------------+--------------------------------------------------+
//  | Value         | Meaning                                          |
//  +---------------+--------------------------------------------------+
//  | KANA_MODE_OFF | Indicates that the KANA input mode is off.       |
//  | 0x00000000    |                                                  |
//  +---------------+--------------------------------------------------+
//  | KANA_MODE_ON  | Indicates that the KANA input mode is activated. |
//  | 0x00000001    |                                                  |
//  +---------------+--------------------------------------------------+

enum {
      KANA_MODE_OFF = 0x00000000
    , KANA_MODE_ON  = 0x00000001
};

class CompartmentStatusInformationPDU {
    uint32_t ImeState        = 0;
    uint32_t ImeConvMode     = 0;
    uint32_t ImeSentenceMode = 0;
    uint32_t KANAMode        = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->ImeState);
        stream.out_uint32_le(this->ImeConvMode);
        stream.out_uint32_le(this->ImeSentenceMode);
        stream.out_uint32_le(this->KANAMode);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 16;   // ImeState(4) + ImeConvMode(4) +
                                            //  ImeSentenceMode(4) + KANAMode(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Compartment Status Information PDU: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->ImeState        = stream.in_uint32_le();
        this->ImeConvMode     = stream.in_uint32_le();
        this->ImeSentenceMode = stream.in_uint32_le();
        this->KANAMode        = stream.in_uint32_le();
    }

    static size_t size() {
        return 16;  // ImeState(4) + ImeConvMode(4) + ImeSentenceMode(4) +
                    //  KANAMode(4)
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "CompartmentStatusInformationPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "ImeState=0x%X ImeConvMode=0x%X ImeSentenceMode=0x%X KANAMode=0x%X",
            this->ImeState, this->ImeConvMode, this->ImeSentenceMode, this->KANAMode);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.11.1 Server Z-Order Sync Information PDU
//  (TS_RAIL_ORDER_ZORDER_SYNC)
// ============================================================

// The Z-Order Sync Information PDU is sent from the server to the client if
//  the client has advertised support for Z-order sync in the Client
//  Information PDU (section 2.2.2.2.2).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                         WindowIdMarker                        |
// +---------------------------------------------------------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_ZORDER_SYNC (0x0014).

// WindowIdMarker (4 bytes): An unsigned 32-bit integer. Indicates the ID of
//  the marker window (section 3.3.1.3), which is used to manage the
//  activation of RAIL windows as specified in section 3.2.5.2.9.2.

class ServerZOrderSyncInformationPDU {
    mutable uint32_t WindowIdMarker_ = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowIdMarker_);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 4;    // WindowIdMarker(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server Z-Order Sync Information PDU: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowIdMarker_ = stream.in_uint32_le();
    }

    uint32_t WindowId() const { return this->WindowIdMarker_; }

    uint32_t WindowIdMarker() const { return this->WindowIdMarker_; }

    static uint16_t orderType() { return TS_RAIL_ORDER_ZORDER_SYNC; }

    static size_t size() {
        return 4;   // WindowIdMarker(4)
    }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t client_window_id = rail_window_id_manager.get_client_window_id_ex(this->WindowIdMarker_);

        assert(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != client_window_id);

        if (client_window_id == this->WindowIdMarker_) {
            return false;
        }

        this->WindowIdMarker_ = client_window_id;

        return true;
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "ServerZOrderSyncInformationPDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowIdMarker=0x%X", this->WindowIdMarker_);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};

// [MS-RDPERP] - 2.2.2.12.1 Window Cloak State Change PDU
//  (TS_RAIL_ORDER_CLOAK)
// ======================================================

// The Window Cloak State Change PDU is sent from the client to the server
//  when a RAIL window has been cloaked or uncloaked on the client. It is
//  only sent when both the client and server support syncing per-window
//  cloak state (indicated by the TS_RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED
//  flag in the Remote Programs Capability Set (section 2.2.1.1.1). The
//  server uses this information to sync the cloak state to the associated
//  window on the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Hdr                              |
// +---------------------------------------------------------------+
// |                            WindowId                           |
// +---------------+-----------------------------------------------+
// |    Cloaked    |
// +---------------+

// Hdr (4 bytes): A TS_RAIL_PDU_HEADER structure. The orderType field of the
//  header MUST be set to TS_RAIL_ORDER_CLOAK (0x0015).

// WindowId (4 bytes): An unsigned 32-bit integer. The ID of the associated
//  window on the server that is to be cloaked or uncloaked.

// Cloaked (1 byte): An unsigned 8-bit integer that indicates whether the
//  window SHOULD be cloaked or uncloaked.

//  +-------+---------------------------------+
//  | Value | Meaning                         |
//  +-------+---------------------------------+
//  | 0x00  | The window SHOULD be uncloaked. |
//  +-------+---------------------------------+
//  | 0x01  | The window SHOULD be cloaked.   |
//  +-------+---------------------------------+

class WindowCloakStateChangePDU {
    mutable uint32_t WindowId_ = 0;
    uint8_t  Cloaked   = 0;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->WindowId_);
        stream.out_uint8(this->Cloaked);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 5;    // WindowId(4) + Cloaked(1)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Window Cloak State Change PDU: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->WindowId_ = stream.in_uint32_le();
        this->Cloaked   = stream.in_uint8();
    }

    static uint16_t orderType() { return TS_RAIL_ORDER_CLOAK; }

    static size_t size() {
        return 5;   // WindowId(4) + Cloaked(1)
    }

    uint32_t WindowId() const { return this->WindowId_; }

    bool map_window_id(RemoteProgramsWindowIdManager const & rail_window_id_manager) const {
        const uint32_t server_window_id = rail_window_id_manager.get_server_window_id(this->WindowId_);

        if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == server_window_id) {
            LOG(LOG_ERR, "WindowCloakStateChangePDU::map_window_id: Failed to map window id. ClientWindowId=0x%X", this->WindowId_);
            throw Error(ERR_UNEXPECTED);
        }

        if (server_window_id == this->WindowId_) {
            return false;
        }

        this->WindowId_ = server_window_id;

        return true;
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length, "WindowCloakStateChangePDU: ");
        length += ((result < size - length) ? result : (size - length - 1));

        result = ::snprintf(buffer + length, size - length,
            "WindowId=0x%X Cloaked=0x%X", this->WindowId_, this->Cloaked);
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};
