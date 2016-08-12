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

#include "utils/sugar/cast.hpp"
#include "core/error.hpp"
#include "utils/stream.hpp"
#include "core/RDP/non_null_terminated_utf16_from_utf8.hpp"

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
};

static inline
char const* get_RAIL_orderType_name(uint16_t orderType) {
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
        default:                            return "<unknown>";
    }
}

// orderLength (2 bytes): An unsigned 16-bit integer. The length of the
//  Virtual Channel PDU, in bytes.

class RAILPDUHeader_Recv {
    uint16_t orderType_;
    uint16_t orderLength_;

public:
    explicit RAILPDUHeader_Recv(InStream & stream) {
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
};

class RAILPDUHeader_Send {
    OutStream & stream;
    uint32_t    offset_of_orderLength;

public:
    explicit RAILPDUHeader_Send(OutStream & stream)
    : stream(stream), offset_of_orderLength(0) {}

    void emit_begin(uint16_t orderType) {
        this->stream.out_uint16_le(orderType);

        this->offset_of_orderLength = this->stream.get_offset();
        this->stream.out_clear_bytes(2);
    }

    void emit_end() {
        this->stream.set_out_uint16_le(
            this->stream.get_offset() - this->offset_of_orderLength +
                2 /* orderType(2) */,
            this->offset_of_orderLength);
    }

    //static size_t header_length() { return 4; /* orderType(4) */ }
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

class HandshakePDU_Recv {
    uint32_t buildNumber_;

public:
    explicit HandshakePDU_Recv(InStream & stream) {
        const unsigned expected = 4;    // buildNumber(4)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Handshake PDU: expected=%u remains=%zu",
                expected, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        this->buildNumber_ = stream.in_uint32_le();
    }

    uint32_t buildNumber() const { return this->buildNumber_; }
};

class HandshakePDU_Send {
public:
    HandshakePDU_Send(OutStream & stream, uint32_t buildNumber) {
        stream.out_uint32_le(buildNumber);
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

//  +-----------------------------------------+------------------------------+
//  | Value                                   |   Meaning                    |
//  +-----------------------------------------+------------------------------+
//  | TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE | Indicates that the client    |
//  | 0x00000001                              | supports the local move/size |
//  |                                         | RAIL feature.                |
//  +-----------------------------------------+------------------------------+
//  | TS_RAIL_CLIENTSTATUS_AUTORECONNECT      | Indicates that the client is |
//  | 0x00000002                              | auto-reconnecting to the     |
//  |                                         | server after an unexpected   |
//  |                                         | disconnect of the session.   |
//  +-----------------------------------------+------------------------------+

enum {
      TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE = 0x00000001
    , TS_RAIL_CLIENTSTATUS_AUTORECONNECT      = 0x00000002
};

class ClientInformationPDU_Recv {
    uint32_t Flags_;

public:
    explicit ClientInformationPDU_Recv(InStream & stream) {
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
};

class ClientInformationPDU_Send {
public:
    ClientInformationPDU_Send(OutStream & stream, uint32_t Flags) {
        stream.out_uint32_le(Flags);
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


class HandshakeExPDU_Recv {
    uint32_t buildNumber_;
    uint32_t railHandshakeFlags_;

public:
    explicit HandshakeExPDU_Recv(InStream & stream) {
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

    uint32_t railHandshakeFlags() const { return this->railHandshakeFlags_; }
};

class HandshakeExPDU_Send {
public:
    HandshakeExPDU_Send(OutStream & stream, uint32_t buildNumber, uint32_t railHandshakeFlags) {
        stream.out_uint32_le(buildNumber);
        stream.out_uint32_le(railHandshakeFlags);
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
//  |A|B|C|D|0|0|0|0|0|0|0|0|0|0|0|0|
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

enum {
      TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY = 0x0001
    , TS_RAIL_EXEC_FLAG_TRANSLATE_FILES         = 0x0002
    , TS_RAIL_EXEC_FLAG_FILE                    = 0x0004
    , TS_RAIL_EXEC_FLAG_EXPAND_ARGUMENTS        = 0x0008
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

class ClientExecutePDU_Recv {
    uint16_t Flags_;
    uint16_t ExeOrFileLength;
    uint16_t WorkingDirLength;
    uint16_t ArgumentsLen;

    std::string exe_or_file_;
    std::string working_dir_;
    std::string arguments_;

public:
    explicit ClientExecutePDU_Recv(InStream & stream) {
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

        this->Flags_            = stream.in_uint16_le();
        this->ExeOrFileLength   = stream.in_uint16_le();
        this->WorkingDirLength  = stream.in_uint16_le();
        this->ArgumentsLen      = stream.in_uint16_le();

        get_non_null_terminated_utf16_from_utf8(
            this->exe_or_file_, stream, this->ExeOrFileLength, "Client Execute PDU");
        get_non_null_terminated_utf16_from_utf8(
            this->working_dir_, stream, this->WorkingDirLength, "Client Execute PDU");
        get_non_null_terminated_utf16_from_utf8(
            this->arguments_, stream, this->ArgumentsLen, "Client Execute PDU");
    }

    uint16_t Flags() const { return this->Flags_; }

    const char * exe_or_file() const { return this->exe_or_file_.c_str(); }

    const char * working_dir() const { return this->working_dir_.c_str(); }

    const char * arguments() const { return this->arguments_.c_str(); }
};  // class ClientExecutePDU_Recv

class ClientExecutePDU_Send {
public:
    ClientExecutePDU_Send(OutStream & stream, uint16_t Flags,
                          const char * exe_or_file, const char * working_dir,
                          const char * arguments) {
        stream.out_uint16_le(Flags);

        const uint32_t offset_of_ExeOrFile  = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_WorkingDir = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_Arguments  = stream.get_offset();
        stream.out_clear_bytes(2);

        const size_t maximum_length_of_ExeOrFile_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, exe_or_file, maximum_length_of_ExeOrFile_in_bytes,
            offset_of_ExeOrFile);

        const size_t maximum_length_of_WorkingDir_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, working_dir, maximum_length_of_WorkingDir_in_bytes,
            offset_of_WorkingDir);

        const size_t maximum_length_of_Arguments_in_bytes = 16000;
        put_non_null_terminated_utf16_from_utf8(
            stream, arguments, maximum_length_of_Arguments_in_bytes,
            offset_of_Arguments);
    }
};

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

class ServerExecuteResultPDU_Recv {
    uint16_t Flags_;
    uint16_t ExecResult_;
    uint32_t RawResult_;
    uint16_t ExeOrFileLength;

    std::string exe_or_file_;

public:
    explicit ServerExecuteResultPDU_Recv(InStream & stream) {
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
            this->exe_or_file_, stream, this->ExeOrFileLength, "Server Execute Result PDU");
    }

    uint16_t Flags() const { return this->Flags_; }

    uint16_t ExecResult() const { return this->ExecResult_; }

    uint16_t RawResult() const { return this->RawResult_; }

    const char * exe_or_file() const { return this->exe_or_file_.c_str(); }
};  // class ServerExecuteResultPDU_Recv

class ServerExecuteResultPDU_Send {
public:
    ServerExecuteResultPDU_Send(OutStream & stream, uint16_t Flags,
            uint16_t ExecResult, uint32_t RawResult,
            const char * exe_or_file) {
        stream.out_uint16_le(Flags);
        stream.out_uint16_le(ExecResult);
        stream.out_uint32_le(RawResult);

        stream.out_clear_bytes(2);  // Padding(2)

        const uint32_t offset_of_ExeOrFile  = stream.get_offset();
        stream.out_clear_bytes(2);

        const size_t maximum_length_of_ExeOrFile_in_bytes = 520;
        put_non_null_terminated_utf16_from_utf8(
            stream, exe_or_file, maximum_length_of_ExeOrFile_in_bytes,
            offset_of_ExeOrFile);
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

enum {
      SPI_SETDRAGFULLWINDOWS = 0x00000025
    , SPI_SETKEYBOARDCUES    = 0x0000100B
    , SPI_SETKEYBOARDPREF    = 0x00000045
    , SPI_SETWORKAREA        = 0x0000002F
    , RAIL_SPI_DISPLAYCHANGE = 0x0000F001
    , SPI_SETMOUSEBUTTONSWAP = 0x00000021
    , RAIL_SPI_TASKBARPOS    = 0x0000F000
    , SPI_SETHIGHCONTRAST    = 0x00000043
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

static inline
char const* get_RAIL_ClientSystemParam_name(uint32_t SystemParam) {
    switch (SystemParam) {
        case SPI_SETDRAGFULLWINDOWS: return "SPI_SETDRAGFULLWINDOWS";
        case SPI_SETKEYBOARDCUES:    return "SPI_SETKEYBOARDCUES";
        case SPI_SETKEYBOARDPREF:    return "SPI_SETKEYBOARDPREF";
        case SPI_SETWORKAREA:        return "SPI_SETWORKAREA";
        case RAIL_SPI_DISPLAYCHANGE: return "RAIL_SPI_DISPLAYCHANGE";
        case SPI_SETMOUSEBUTTONSWAP: return "SPI_SETMOUSEBUTTONSWAP";
        case RAIL_SPI_TASKBARPOS:    return "RAIL_SPI_TASKBARPOS";
        case SPI_SETHIGHCONTRAST:    return "SPI_SETHIGHCONTRAST";
        default:                     return "<unknown>";
    }
}

class ClientSystemParametersUpdatePDU_Recv {
    uint32_t SystemParam_;

public:
    explicit ClientSystemParametersUpdatePDU_Recv(InStream & stream) {
        {
            const unsigned expected = 4;    // SystemParam(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client System Parameters Update PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->SystemParam_ = stream.in_uint32_le();
    }

    uint32_t SystemParam() const { return this->SystemParam_; }
};

class ClientSystemParametersUpdatePDU_Send {
public:
    ClientSystemParametersUpdatePDU_Send(OutStream & stream, uint32_t SystemParam) {
        stream.out_uint32_le(SystemParam);
    }
};

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


class HighContrastSystemInformationStructure_Recv {
    uint32_t Flags_;
    uint32_t ColorSchemeLength;

    std::string ColorScheme_;

public:
    explicit HighContrastSystemInformationStructure_Recv(InStream & stream) {
        {
            const unsigned expected = 8;    // Flags(4) + ColorSchemeLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated High Contrast System Information Structure: expected=%u remains=%zu (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_            = stream.in_uint32_le();
        this->ColorSchemeLength = stream.in_uint32_le();

        if (!stream.in_check_rem(this->ColorSchemeLength)) {
            LOG(LOG_ERR,
                "Truncated High Contrast System Information Structure: expected=%u remains=%zu (1)",
                this->ColorSchemeLength, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        REDASSERT(this->ColorSchemeLength >= 2 /* CbString(2) */);

        get_non_null_terminated_utf16_from_utf8(
            this->ColorScheme_, stream, stream.in_uint16_le(),
            "High Contrast System Information Structure");
    }

    uint32_t Flags() const { return this->Flags_; }

    const char * ColorScheme() const { return this->ColorScheme_.c_str(); }
};

class HighContrastSystemInformationStructure_Send {
public:
    HighContrastSystemInformationStructure_Send(OutStream & stream, uint32_t Flags,
                                                const char * color_scheme) {
        stream.out_uint32_le(Flags);

        const size_t offset_of_data_length = stream.get_offset();
        stream.out_skip_bytes(4);

        auto size_of_unicode_data = put_non_null_terminated_utf16_from_utf8(
            stream, color_scheme, strlen(color_scheme) * 2);

        stream.set_out_uint32_le(2 /* CbString(2) */ + size_of_unicode_data, offset_of_data_length);
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
char const* get_RAIL_ServerSystemParam_name(uint32_t SystemParam) {
    switch (SystemParam) {
        case SPI_SETSCREENSAVEACTIVE: return "SPI_SETSCREENSAVEACTIVE";
        case SPI_SETSCREENSAVESECURE: return "SPI_SETSCREENSAVESECURE";
        default:                      return "<unknown>";
    }
}

class ServerSystemParametersUpdatePDU_Recv {
    uint32_t SystemParam_;

public:
    explicit ServerSystemParametersUpdatePDU_Recv(InStream & stream) {
        {
            const unsigned expected = 4;    // SystemParam(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Server System Parameters Update PDU: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->SystemParam_ = stream.in_uint32_le();
    }

    uint32_t SystemParam() const { return this->SystemParam_; }
};

class ServerSystemParametersUpdatePDU_Send {
public:
    ServerSystemParametersUpdatePDU_Send(OutStream & stream, uint32_t SystemParam) {
        stream.out_uint32_le(SystemParam);
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

class ClientActivatePDU_Recv {
    uint32_t WindowId_;
    uint8_t  Enabled_;

public:
    explicit ClientActivatePDU_Recv(InStream & stream) {
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

    uint32_t WindowId() const { return this->WindowId_; }

    uint8_t Enabled() const { return this->Enabled_; }
};

class ClientActivatePDU_Send {
public:
    ClientActivatePDU_Send(OutStream & stream, uint32_t WindowId, uint8_t Enabled) {
        stream.out_uint32_le(WindowId);
        stream.out_uint8(Enabled);
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

class ClientSystemMenuPDU_Recv {
    uint32_t WindowId_;
    int16_t  Left_;
    int16_t  Top_;

public:
    explicit ClientSystemMenuPDU_Recv(InStream & stream) {
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

    uint32_t WindowId() const { return this->WindowId_; }

    int16_t Left() const { return this->Left_; }

    int16_t Top() const { return this->Top_; }
};

class ClientSystemMenuPDU_Send {
public:
    ClientSystemMenuPDU_Send(OutStream & stream, uint32_t WindowId, int16_t Left, int16_t Top) {
        stream.out_uint32_le(WindowId);
        stream.out_sint16_le(Left);
        stream.out_sint16_le(Top);
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

class ClientSystemCommandPDU_Recv {
    uint32_t WindowId_;
    uint16_t Command_;

public:
    explicit ClientSystemCommandPDU_Recv(InStream & stream) {
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

    uint32_t WindowId() const { return this->WindowId_; }

    int16_t Command() const { return this->Command_; }
};

class ClientSystemCommandPDU_Send {
public:
    ClientSystemCommandPDU_Send(OutStream & stream, uint32_t WindowId, uint16_t Command) {
        stream.out_uint32_le(WindowId);
        stream.out_uint16_le(Command);
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


class ClientNotifyEventPDU_Recv {
    uint32_t WindowId_;
    uint32_t NotifyIconId_;
    uint32_t Message_;

public:
    explicit ClientNotifyEventPDU_Recv(InStream & stream) {
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

    uint32_t WindowId() const { return this->WindowId_; }

    uint32_t NotifyIconId() const { return this->NotifyIconId_; }

    uint32_t Message() const { return this->Message_; }
};

class ClientNotifyEventPDU_Send {
public:
    ClientNotifyEventPDU_Send(OutStream & stream, uint32_t WindowId, uint32_t NotifyIconId, uint32_t Message) {
        stream.out_uint32_le(WindowId);
        stream.out_uint16_le(NotifyIconId);
        stream.out_uint16_le(Message);
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

class ClientGetApplicationIDPDU_Recv {
    uint32_t WindowId_;

public:
    explicit ClientGetApplicationIDPDU_Recv(InStream & stream) {
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

    uint32_t SystemParam() const { return this->WindowId_; }
};

class ClientGetApplicationIDPDU_Send {
public:
    ClientGetApplicationIDPDU_Send(OutStream & stream, uint32_t WindowId) {
        stream.out_uint32_le(WindowId);
    }
};
