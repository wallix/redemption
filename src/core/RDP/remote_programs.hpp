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

#ifndef _REDEMPTION_CORE_RDP_REMOTE_PROGRAMS_HPP_
#define _REDEMPTION_CORE_RDP_REMOTE_PROGRAMS_HPP_

#include "cast.hpp"
#include "stream.hpp"

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

// orderLength (2 bytes): An unsigned 16-bit integer. The length of the
//  Virtual Channel PDU, in bytes.

class RAILPDUHeader_Recv {
    uint16_t orderType_;
    uint16_t orderLength_;

public:
    explicit RAILPDUHeader_Recv(Stream & stream) {
        const unsigned expected = 4;    // orderType(2) + orderLength(2)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Truncated RAIL PDU header: expected=%u remains=%u",
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
    Stream   & stream;
    uint32_t   offset_of_orderLength;

public:
    explicit RAILPDUHeader_Send(Stream & stream)
    : stream(stream), offset_of_orderLength(0) {}

    void emit_begin(uint16_t orderType) {
        this->stream.out_uint16_le(orderType);

        this->offset_of_orderLength = this->stream.get_offset();
        this->stream.out_clear_bytes(2);

        this->stream.mark_end();
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
    explicit HandshakePDU_Recv(Stream & stream) {
        const unsigned expected = 4;    // buildNumber(4)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "Handshake PDU: expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        this->buildNumber_ = stream.in_uint32_le();
    }

    uint32_t buildNumber() const { return this->buildNumber_; }
};

class HandshakePDU_Send {
public:
    HandshakePDU_Send(Stream & stream, uint32_t buildNumber) {
        stream.out_uint32_le(buildNumber);

        stream.mark_end();
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
    explicit ClientInformationPDU_Recv(Stream & stream) {
        {
            const unsigned expected = 4;    // Flags(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Information PDU: expected=%u remains=%u",
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
    ClientInformationPDU_Send(Stream & stream, uint32_t Flags) {
        stream.out_uint32_le(Flags);

        stream.mark_end();
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
    explicit ClientExecutePDU_Recv(Stream & stream) {
        {
            const unsigned expected =
                8;  // Flags(2) + ExeOrFileLength(2) + WorkingDirLength(2) + ArgumentsLen(2)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client Execute PDU: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_            = stream.in_uint16_le();
        this->ExeOrFileLength   = stream.in_uint16_le();
        this->WorkingDirLength  = stream.in_uint16_le();
        this->ArgumentsLen      = stream.in_uint16_le();

        auto get_non_null_terminated_utf16_string =
            [&stream] (std::string & out,
                       size_t length_of_utf16_data_in_bytes) {
                if (!stream.in_check_rem(length_of_utf16_data_in_bytes)) {
                    LOG(LOG_ERR,
                        "Truncated Client Execute PDU: expected=%u remains=%u",
                        length_of_utf16_data_in_bytes, stream.in_remain());
                    throw Error(ERR_RAIL_PDU_TRUNCATED);
                }
                uint8_t * const utf16_data = static_cast<uint8_t *>(
                    ::alloca(length_of_utf16_data_in_bytes));
                stream.in_copy_bytes(utf16_data,
                    length_of_utf16_data_in_bytes);

                const size_t size_of_utf8_string =
                    length_of_utf16_data_in_bytes / 2 *
                    maximum_length_of_utf8_character_in_bytes + 1;
                uint8_t * const utf8_string = static_cast<uint8_t *>(
                    ::alloca(size_of_utf8_string));
                const size_t length_of_utf8_string = ::UTF16toUTF8(
                    utf16_data, length_of_utf16_data_in_bytes / 2,
                    utf8_string, size_of_utf8_string);
                out.assign(::char_ptr_cast(utf8_string),
                    length_of_utf8_string);
            };

        get_non_null_terminated_utf16_string(this->exe_or_file_,
            this->ExeOrFileLength);
        get_non_null_terminated_utf16_string(this->working_dir_,
            this->WorkingDirLength);
        get_non_null_terminated_utf16_string(this->arguments_,
            this->ArgumentsLen);
    }

    uint16_t Flags() const { return this->Flags_; }

    const char * exe_or_file() const { return this->exe_or_file_.c_str(); }

    const char * working_dir() const { return this->working_dir_.c_str(); }

    const char * arguments() const { return this->arguments_.c_str(); }
};  // class ClientExecutePDU_Recv

class ClientExecutePDU_Send {
public:
    ClientExecutePDU_Send(Stream & stream, uint16_t Flags,
                          const char * exe_or_file, const char * working_dir,
                          const char * arguments) {
        stream.out_uint16_le(Flags);

        const uint32_t offset_of_ExeOrFile  = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_WorkingDir = stream.get_offset();
        stream.out_clear_bytes(2);
        const uint32_t offset_of_Arguments  = stream.get_offset();
        stream.out_clear_bytes(2);

        auto put_non_null_terminate_utf16_string =
            [&stream] (const char * in,
                       size_t maximum_length_of_utf16_data_in_bytes,
                       uint32_t offset_of_data_length) {
                uint8_t * const utf16_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_utf16_data_in_bytes));
                const size_t size_of_utf16_data = ::UTF8toUTF16(
                    reinterpret_cast<const uint8_t *>(in), utf16_data,
                    maximum_length_of_utf16_data_in_bytes);

                stream.out_copy_bytes(utf16_data, size_of_utf16_data);

                stream.set_out_uint16_le(size_of_utf16_data,
                    offset_of_data_length);
            };

        const size_t maximum_length_of_ExeOrFile_in_bytes = 520;
        put_non_null_terminate_utf16_string(
            exe_or_file, maximum_length_of_ExeOrFile_in_bytes,
            offset_of_ExeOrFile);

        const size_t maximum_length_of_WorkingDir_in_bytes = 520;
        put_non_null_terminate_utf16_string(
            working_dir, maximum_length_of_WorkingDir_in_bytes,
            offset_of_WorkingDir);

        const size_t maximum_length_of_Arguments_in_bytes = 16000;
        put_non_null_terminate_utf16_string(
            arguments, maximum_length_of_Arguments_in_bytes,
            offset_of_Arguments);

        stream.mark_end();
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

class ClientSystemParametersUpdatePDU_Recv {
    uint32_t SystemParam_;

public:
    explicit ClientSystemParametersUpdatePDU_Recv(Stream & stream) {
        {
            const unsigned expected = 4;    // SystemParam(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated Client System Parameters Update PDU: expected=%u remains=%u",
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
    ClientSystemParametersUpdatePDU_Send(Stream & stream, uint32_t SystemParam) {
        stream.out_uint32_le(SystemParam);

        stream.mark_end();
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
    explicit HighContrastSystemInformationStructure_Recv(Stream & stream) {
        {
            const unsigned expected = 8;    // Flags(4) + ColorSchemeLength(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated High Contrast System Information Structure: expected=%u remains=%u (0)",
                    expected, stream.in_remain());
                throw Error(ERR_RAIL_PDU_TRUNCATED);
            }
        }

        this->Flags_            = stream.in_uint32_le();
        this->ColorSchemeLength = stream.in_uint32_le();

        if (!stream.in_check_rem(this->ColorSchemeLength)) {
            LOG(LOG_ERR,
                "Truncated High Contrast System Information Structure: expected=%u remains=%u (1)",
                this->ColorSchemeLength, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        REDASSERT(this->ColorSchemeLength >= 2 /* CbString(2) */);

        const uint16_t CbString = stream.in_uint16_le();

        if (!stream.in_check_rem(CbString)) {
            LOG(LOG_ERR,
                "Truncated High Contrast System Information Structure: expected=%u remains=%u (2)",
                this->ColorSchemeLength, stream.in_remain());
            throw Error(ERR_RAIL_PDU_TRUNCATED);
        }

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(CbString));

        stream.in_copy_bytes(unicode_data, CbString);

        const size_t size_of_utf8_string =
                    CbString / 2 * maximum_length_of_utf8_character_in_bytes + 1;
        uint8_t * const utf8_string = static_cast<uint8_t *>(
            ::alloca(size_of_utf8_string));
        const size_t length_of_utf8_string = ::UTF16toUTF8(
            unicode_data, CbString / 2, utf8_string, size_of_utf8_string);
        this->ColorScheme_.assign(::char_ptr_cast(utf8_string),
            length_of_utf8_string);
    }

    uint32_t Flags() const { return this->Flags_; }

    const char * ColorScheme() const { return this->ColorScheme_.c_str(); }
};

class HighContrastSystemInformationStructure_Send {
public:
    HighContrastSystemInformationStructure_Send(Stream & stream, uint32_t Flags,
                                                const char * color_scheme) {
        stream.out_uint32_le(Flags);

        const size_t maximum_length_of_ColorScheme_in_bytes = strlen(color_scheme) * 2;

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_ColorScheme_in_bytes));
        const size_t size_of_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(color_scheme), unicode_data,
            maximum_length_of_ColorScheme_in_bytes);

        stream.out_uint32_le(2 /* CbString(2) */ + size_of_unicode_data);

        stream.out_uint16_le(size_of_unicode_data);

        stream.out_copy_bytes(unicode_data, size_of_unicode_data);

        stream.mark_end();
    }
};

#endif  // #ifndef _REDEMPTION_CORE_RDP_REMOTE_PROGRAMS_HPP_
