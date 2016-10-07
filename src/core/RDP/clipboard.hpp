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

#include <iostream>
#include <cinttypes>

#include "utils/stream.hpp"
#include "core/error.hpp"
#include "utils/sugar/cast.hpp"

namespace RDPECLIP {

// Predefined Clipboard Formats (WinUser.h)
enum {
      CF_TEXT            = 1
    , CF_BITMAP          = 2
    , CF_METAFILEPICT    = 3
    , CF_SYLK            = 4
    , CF_DIF             = 5
    , CF_TIFF            = 6
    , CF_OEMTEXT         = 7
    , CF_DIB             = 8
    , CF_PALETTE         = 9
    , CF_PENDATA         = 10
    , CF_RIFF            = 11
    , CF_WAVE            = 12
    , CF_UNICODETEXT     = 13
    , CF_ENHMETAFILE     = 14
    , CF_HDROP           = 15
    , CF_LOCALE          = 16
    , CF_DIBV5           = 17
    , CF_OWNERDISPLAY    = 128
    , CF_DSPTEXT         = 129
    , CF_DSPBITMAP       = 130
    , CF_DSPMETAFILEPICT = 131
    , CF_DSPENHMETAFILE  = 142
    , CF_PRIVATEFIRST    = 512
    , CF_PRIVATELAST     = 767
    , CF_GDIOBJFIRST     = 768
    , CF_GDIOBJLAST      = 1023
};

inline static const char * get_Format_name(uint32_t FormatId) {
    switch (FormatId) {
        case CF_TEXT:            return "CF_TEXT";
        case CF_BITMAP:          return "CF_BITMAP";
        case CF_METAFILEPICT:    return "CF_METAFILEPICT";
        case CF_SYLK:            return "CF_SYLK";
        case CF_DIF:             return "CF_DIF";
        case CF_TIFF:            return "CF_TIFF";
        case CF_OEMTEXT:         return "CF_OEMTEXT";
        case CF_DIB:             return "CF_DIB";
        case CF_PALETTE:         return "CF_PALETTE";
        case CF_PENDATA:         return "CF_PENDATA";
        case CF_RIFF:            return "CF_RIFF";
        case CF_WAVE:            return "CF_WAVE";
        case CF_UNICODETEXT:     return "CF_UNICODETEXT";
        case CF_ENHMETAFILE:     return "CF_ENHMETAFILE";
        case CF_HDROP:           return "CF_HDROP";
        case CF_LOCALE:          return "CF_LOCALE";
        case CF_DIBV5:           return "CF_DIBV5";
        case CF_OWNERDISPLAY:    return "CF_OWNERDISPLAY";
        case CF_DSPTEXT:         return "CF_DSPTEXT";
        case CF_DSPBITMAP:       return "CF_DSPBITMAP";
        case CF_DSPMETAFILEPICT: return "CF_DSPMETAFILEPICT";
        case CF_DSPENHMETAFILE:  return "CF_DSPENHMETAFILE";
        case CF_PRIVATEFIRST:    return "CF_PRIVATEFIRST";
        case CF_PRIVATELAST:     return "CF_PRIVATELAST";
        case CF_GDIOBJFIRST:     return "CF_GDIOBJFIRST";
        case CF_GDIOBJLAST:      return "CF_GDIOBJLAST";
    }

    return "<unknown>";
}

// [MS-RDPECLIP] 2.2.1 Clipboard PDU Header (CLIPRDR_HEADER)
// =========================================================

// The CLIPRDR_HEADER structure is present in all clipboard PDUs. It is used
//  to identify the PDU type, specify the length of the PDU, and convey
//  message flags.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |            msgType            |            msgFlags           |
// +-------------------------------+-------------------------------+
// |                            dataLen                            |
// +---------------------------------------------------------------+

// msgType (2 bytes): An unsigned, 16-bit integer that specifies the type of
// the clipboard PDU that follows the dataLen field.

// +---------------------------------+-----------------------------------------+
// | Value                           | Meaning                                 |
// +---------------------------------+-----------------------------------------+
// | 0x0001 CB_MONITOR_READY         | Monitor Ready PDU                       |
// +---------------------------------+-----------------------------------------+
// | 0x0002 CB_FORMAT_LIST           | Format List PDU                         |
// +---------------------------------+-----------------------------------------+
// | 0x0003 CB_FORMAT_LIST_RESPONSE  | Format List Response PDU                |
// +---------------------------------+-----------------------------------------+
// | 0x0004 CB_FORMAT_DATA_REQUEST   | Format Data Request PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x0005 CB_FORMAT_DATA_RESPONSE  | Format Data Response PDU                |
// +---------------------------------+-----------------------------------------+
// | 0x0006 CB_TEMP_DIRECTORY        | Temporary Directory PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x0007 CB_CLIP_CAPS             | Clipboard Capabilities PDU              |
// +---------------------------------+-----------------------------------------+
// | 0x0008 CB_FILECONTENTS_REQUEST  | File Contents Request PDU               |
// +---------------------------------+-----------------------------------------+
// | 0x0009 CB_FILECONTENTS_RESPONSE | File Contents Response PDU              |
// +---------------------------------+-----------------------------------------+
// | 0x000A CB_LOCK_CLIPDATA         | Lock Clipboard Data PDU                 |
// +---------------------------------+-----------------------------------------+
// | 0x000B CB_UNLOCK_CLIPDATA       | Unlock Clipboard Data PDU               |
// +---------------------------------+-----------------------------------------+

enum {
      CB_MONITOR_READY         = 0x0001
    , CB_FORMAT_LIST           = 0x0002
    , CB_FORMAT_LIST_RESPONSE  = 0x0003
    , CB_FORMAT_DATA_REQUEST   = 0x0004
    , CB_FORMAT_DATA_RESPONSE  = 0x0005
    , CB_TEMP_DIRECTORY        = 0x0006
    , CB_CLIP_CAPS             = 0x0007
    , CB_FILECONTENTS_REQUEST  = 0x0008
    , CB_FILECONTENTS_RESPONSE = 0x0009
    , CB_LOCK_CLIPDATA         = 0x000A
    , CB_UNLOCK_CLIPDATA       = 0x000B
};

enum {
      CB_CHUNKED_FORMAT_DATA_RESPONSE = 0xFFFF
};

inline static const char * get_msgType_name(uint16_t msgType) {
    switch (msgType) {
        case CB_MONITOR_READY:         return "CB_MONITOR_READY";
        case CB_FORMAT_LIST:           return "CB_FORMAT_LIST";
        case CB_FORMAT_LIST_RESPONSE:  return "CB_FORMAT_LIST_RESPONSE";
        case CB_FORMAT_DATA_REQUEST:   return "CB_FORMAT_DATA_REQUEST";
        case CB_FORMAT_DATA_RESPONSE:  return "CB_FORMAT_DATA_RESPONSE";
        case CB_TEMP_DIRECTORY:        return "CB_TEMP_DIRECTORY";
        case CB_CLIP_CAPS:             return "CB_CLIP_CAPS";
        case CB_FILECONTENTS_REQUEST:  return "CB_FILECONTENTS_REQUEST";
        case CB_FILECONTENTS_RESPONSE: return "CB_FILECONTENTS_RESPONSE";
        case CB_LOCK_CLIPDATA:         return "CB_LOCK_CLIPDATA";
        case CB_UNLOCK_CLIPDATA:       return "CB_UNLOCK_CLIPDATA";

        case CB_CHUNKED_FORMAT_DATA_RESPONSE: return "CB_CHUNKED_FORMAT_DATA_RESPONSE";
    }

    return "<unknown>";
}

// msgFlags (2 bytes): An unsigned, 16-bit integer that indicates message
//  flags.

// +------------------+--------------------------------------------------------+
// | Value            | Meaning                                                |
// +------------------+--------------------------------------------------------+
// | CB_RESPONSE_OK   | Used by the Format List Response PDU, Format Data      |
// | 0x0001           | Response PDU, and File Contents Response PDU to        |
// |                  | indicate that the associated request Format List PDU,  |
// |                  | Format Data Request PDU, and File Contents Request PDU |
// |                  | were processed successfully.                           |
// +------------------+--------------------------------------------------------+
// | CB_RESPONSE_FAIL | Used by the Format List Response PDU, Format Data      |
// | 0x0002           | Response PDU, and File Contents Response PDU to        |
// |                  | indicate that the associated Format List PDU, Format   |
// |                  | Data Request PDU, and File Contents Request PDU were   |
// |                  | not processed successfully.                            |
// +------------------+--------------------------------------------------------+
// | CB_ASCII_NAMES   | Used by the Short Format Name variant of the Format    |
// | 0x0004           | List Response PDU to indicate the format names are in  |
// |                  | ASCII 8.                                               |
// +------------------+--------------------------------------------------------+


// Short Format Name
enum {
    SF_TEXT_HTML = 1
};

inline static const char * get_format_short_name(uint16_t formatID) {
    switch (formatID) {
        case SF_TEXT_HTML:         return "text/html";
    }

    return "<unknown>";
}

enum {
      CB_RESPONSE_OK   = 0x0001
    , CB_RESPONSE_FAIL = 0x0002
    , CB_ASCII_NAMES   = 0x0004
};

// dataLen (4 bytes): An unsigned, 32-bit integer that specifies the size, in
//  bytes, of the data which follows the Clipboard PDU Header.

struct RecvFactory {
    uint16_t msgType;

    explicit RecvFactory(InStream & stream) {
        const unsigned expected = 2;    /* msgType(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::RecvFactory truncated msgType, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->msgType = stream.in_uint16_le();
    }   // RecvFactory(InStream & stream)
};

struct CliprdrHeader {
    uint16_t msgType()  const { return this->msgType_; }
    uint16_t msgFlags() const { return this->msgFlags_; }
    uint32_t dataLen()  const { return this->dataLen_; }

protected:
    uint16_t msgType_;
    uint16_t msgFlags_;
    uint32_t dataLen_;


    CliprdrHeader()
        : msgType_(0)
        , msgFlags_(0)
        , dataLen_(0) {
    }   // CliprdrHeader()

    CliprdrHeader(uint16_t msgType, uint16_t msgFlags, uint32_t dataLen)
        : msgType_(msgType)
        , msgFlags_(msgFlags)
        , dataLen_(dataLen) {
    }   // CliprdrHeader(uint16_t msgType, uint16_t msgFlags, uint32_t dataLen)

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->msgType_);
        stream.out_uint16_le(this->msgFlags_);
        stream.out_uint32_le(this->dataLen_);
    }   // void emit(OutStream & stream)

    void recv(InStream & stream, const RecvFactory & recv_factory) {
        const unsigned expected = 6;    /* msgFlags_(2) + dataLen_(4) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::recv truncated data, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->msgType_  = recv_factory.msgType;
        this->msgFlags_ = stream.in_uint16_le();
        this->dataLen_  = stream.in_uint32_le();
    }

private:
    CliprdrHeader(CliprdrHeader const &);
    CliprdrHeader& operator=(CliprdrHeader const &);
};  // struct CliprdrHeader

// [MS-RDPECLIP] - 2.2.2.1 Clipboard Capabilities PDU (CLIPRDR_CAPS)
// =================================================================

// The Clipboard Capabilities PDU is an optional PDU used to exchange
//  capability information. It is first sent from the server to the client.
//  Upon receipt of the Monitor Ready PDU, the client sends the Clipboard
//  Capabilities PDU to the server.

// If this PDU is not sent by a Remote Desktop Protocol: Clipboard Virtual
//  Channel Extension endpoint, it is assumed that the endpoint is using the
//  default values for each capability field.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |       cCapabilitiesSets       |              pad1             |
// +-------------------------------+-------------------------------+
// |                   capabilitySets (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_CLIP_CAPS (0x0007), while the
//  msgFlags field MUST be set to 0x0000.

// cCapabilitiesSets (2 bytes): An unsigned, 16-bit integer that specifies
//  the number of CLIPRDR_CAPS_SETs, present in the capabilitySets field.

// pad1 (2 bytes): An unsigned, 16-bit integer used for padding. Values in
//  this field are ignored.

// capabilitySets (variable): A variable-sized array of capability sets, each
//  conforming in structure to the CLIPRDR_CAPS_SET.

struct ClipboardCapabilitiesPDU : public CliprdrHeader {
private:
    uint16_t cCapabilitiesSets = 0;

public:
    ClipboardCapabilitiesPDU() = default;

    ClipboardCapabilitiesPDU(uint16_t cCapabilitiesSets, uint32_t length_capabilities) :
        CliprdrHeader(CB_CLIP_CAPS,
                      0,
                      4 +   // cCapabilitiesSets(2) + pad1(2)
                          length_capabilities
                     ),
        cCapabilitiesSets(cCapabilitiesSets)
    {}

    void emit(OutStream & stream) {
        CliprdrHeader::emit(stream);

        stream.out_uint16_le(cCapabilitiesSets);
        stream.out_clear_bytes(2);  // pad1(2)
    }   // void emit(OutStream & stream)

    void recv(InStream & stream, const RecvFactory & recv_factory) {
        CliprdrHeader::recv(stream, recv_factory);

        const unsigned expected = 4;    // cCapabilitiesSets(2) + pad1(2)
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO,
                "RDPECLIP::ClipboardCapabilitiesPDU:recv(): recv truncated data, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->cCapabilitiesSets = stream.in_uint16_le();

        stream.in_skip_bytes(2);    // pad1(2)
    }
};  // struct ClipboardCapabilitiesPDU

// [MS-RDPECLIP] - 2.2.2.1.1 Capability Set (CLIPRDR_CAPS_SET)
// ===========================================================

// The CLIPRDR_CAPS_SET structure is used to wrap capability set data and to
//  specify the type and size of this data exchanged between the client and
//  the server. All capability sets conform to this basic structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       capabilitySetType       |        lengthCapability       |
// +-------------------------------+-------------------------------+
// |                   capabilityData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// capabilitySetType (2 bytes): An unsigned, 16-bit integer used as a type
//  identifier of the capability set.

//  +---------------------+------------------------+
//  | Value               | Meaning                |
//  +---------------------+------------------------+
//  | CB_CAPSTYPE_GENERAL | General Capability Set |
//  | 0x0001              |                        |
//  +---------------------+------------------------+

enum {
    CB_CAPSTYPE_GENERAL = 0x0001
};

// lengthCapability (2 bytes): An unsigned, 16-bit integer that specifies the
//  combined length, in bytes, of the capabilitySetType, capabilityData and
//  lengthCapability fields.

// capabilityData (variable): Capability set data specified by the type given
//  in the capabilitySetType field. This field is a variable number of bytes.

struct CapabilitySetRecvFactory {
    uint16_t capabilitySetType;

    explicit CapabilitySetRecvFactory(InStream & stream) {
        const unsigned expected = 2;    /* capabilitySetType(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO
               , "RDPECLIP::CapabilitySetRecvFactory truncated capabilitySetType, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->capabilitySetType = stream.in_uint16_le();
    }   // CapabilitySetRecvFactory(InStream & stream)

    static const char * get_capabilitySetType_name(uint16_t capabilitySetType) {
        switch (capabilitySetType) {
            case CB_CAPSTYPE_GENERAL:  return "CB_CAPSTYPE_GENERAL";
        }

        return "<unknown>";
    }
};

// [MS-RDPECLIP] - 2.2.2.1.1.1 General Capability Set
//  (CLIPRDR_GENERAL_CAPABILITY)
// ==================================================

// The CLIPRDR_GENERAL_CAPABILITY structure is used to advertise general
//  clipboard settings.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       capabilitySetType       |        lengthCapability       |
// +-------------------------------+-------------------------------+
// |                            version                            |
// +---------------------------------------------------------------+
// |                          generalFlags                         |
// +---------------------------------------------------------------+

// capabilitySetType (2 bytes): An unsigned 16-bit integer that specifies the
//  type of the capability set. This field MUST be set to CB_CAPSTYPE_GENERAL
//  (0x0001).

// lengthCapability (2 bytes): An unsigned, 16-bit integer that specifies the
//  length, in bytes, of the capabilitySetType, capability data and
//  lengthCapability fields.

// version (4 bytes): An unsigned, 32-bit integer that specifies the Remote
//  Desktop Protocol: Clipboard Virtual Channel Extension version number.
//  This field is for informational purposes and MUST NOT be used to make
//  protocol capability decisions. The actual features supported are
//  specified in the generalFlags field.

//  +-------------------+-----------+
//  | Value             | Meaning   |
//  +-------------------+-----------+
//  | CB_CAPS_VERSION_1 | Version 1 |
//  | 0x00000001        |           |
//  +-------------------+-----------+
//  | CB_CAPS_VERSION_2 | Version 2 |
//  | 0x00000002        |           |
//  +-------------------+-----------+

enum {
    CB_CAPS_VERSION_1 = 0x00000001,
    CB_CAPS_VERSION_2 = 0x00000002
};

// generalFlags (4 bytes): An unsigned, 32-bit integer that specifies the
//  general capability flags.

//  +----------------------------+--------------------------------------------+
//  | Value                      | Meaning                                    |
//  +----------------------------+--------------------------------------------+
//  | CB_USE_LONG_FORMAT_NAMES   | The Long Format Name variant of the Format |
//  | 0x00000002                 | List PDU is supported for exchanging       |
//  |                            | updated format names. If this flag is not  |
//  |                            | set, the Short Format Name variant MUST be |
//  |                            | used. If this flag is set by both protocol |
//  |                            | endpoints, then the Long Format Name       |
//  |                            | variant MUST be used.                      |
//  +----------------------------+--------------------------------------------+
//  | CB_STREAM_FILECLIP_ENABLED | File copy and paste using stream-based     |
//  | 0x00000004                 | operations are supported using the File    |
//  |                            | Contents Request PDU and File Contents     |
//  |                            | Response PDU.                              |
//  +----------------------------+--------------------------------------------+
//  | CB_FILECLIP_NO_FILE_PATHS  | Indicates that any description of files to |
//  | 0x00000008                 | copy and paste MUST NOT include the source |
//  |                            | path of the files.                         |
//  +----------------------------+--------------------------------------------+
//  | CB_CAN_LOCK_CLIPDATA       | Locking and unlocking of File Stream data  |
//  | 0x00000010                 | on the clipboard is supported using the    |
//  |                            | Lock Clipboard Data PDU and Unlock         |
//  |                            | Clipboard Data PDU.                        |
//  +----------------------------+--------------------------------------------+

enum {
    CB_USE_LONG_FORMAT_NAMES   = 0x00000002,
    CB_STREAM_FILECLIP_ENABLED = 0x00000004,
    CB_FILECLIP_NO_FILE_PATHS  = 0x00000008,
    CB_CAN_LOCK_CLIPDATA       = 0x00000010,

    CB_ALL_GENERAL_CAPABILITY_FLAGS = (CB_USE_LONG_FORMAT_NAMES |
                                       CB_STREAM_FILECLIP_ENABLED |
                                       CB_FILECLIP_NO_FILE_PATHS |
                                       CB_CAN_LOCK_CLIPDATA
                                      )
};

// If the General Capability Set is not present in the Clipboard Capabilities
//  PDU, then the default set of general capabilities MUST be assumed. By
//  definition the default set does not specify any flags in the generalFlags
//  field, that is the generalFlags field is set to 0x00000000.

class GeneralCapabilitySet {
    uint16_t capabilitySetType = CB_CAPSTYPE_GENERAL;
    uint16_t lengthCapability  = size();
    uint32_t version           = CB_CAPS_VERSION_1;
    uint32_t generalFlags_     = 0;

public:
    GeneralCapabilitySet() = default;

    GeneralCapabilitySet(uint32_t version, uint32_t generalFlags) {
        this->version       = version;
        this->generalFlags_ = generalFlags;
    }

    void emit(OutStream & stream) {
        stream.out_uint16_le(this->capabilitySetType);
        stream.out_uint16_le(this->lengthCapability);
        stream.out_uint32_le(this->version);
        stream.out_uint32_le(this->generalFlags_);
    }

    void recv(InStream & stream, const CapabilitySetRecvFactory & recv_factory) {
        REDASSERT(recv_factory.capabilitySetType == CB_CAPSTYPE_GENERAL);

        const unsigned expected = 10;   // lengthCapability(2) + version(4) +
                                        //     generalFlags(4)
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO
               , "RDPECLIP::GeneralCapabilitySet::recv truncated data, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->capabilitySetType = recv_factory.capabilitySetType;
        this->lengthCapability  = stream.in_uint16_le();

        REDASSERT(this->lengthCapability == size());

        this->version       = stream.in_uint32_le();
        this->generalFlags_ = stream.in_uint32_le();
    }

    uint32_t generalFlags() const { return this->generalFlags_; }

    static size_t size() {
        return 12;  // capabilitySetType(2) + lengthCapability(2) + version(4) +
                    //     generalFlags(4)
    }

private:
    static const char * get_version_name(uint32_t version) {
        switch (version) {
            case CB_CAPS_VERSION_1: return "CB_CAPS_VERSION_1";
            case CB_CAPS_VERSION_2: return "CB_CAPS_VERSION_2";
        }

        return "<unknown>";
    }

    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "RDPECLIP::GeneralCapabilitySet: "
                "capabilitySetType=%s(%d) lengthCapability=%u version=%s(0x%08X) generalFlags=0x%08X",
            CapabilitySetRecvFactory::get_capabilitySetType_name(this->capabilitySetType),
            this->capabilitySetType,
            unsigned(this->lengthCapability),
            this->get_version_name(this->version),
            this->version,
            this->generalFlags_);
        return ((length < size) ? length : size - 1);
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // GeneralCapabilitySet

// [MS-RDPECLIP] 2.2.2.2 Server Monitor Ready PDU (CLIPRDR_MONITOR_READY)
// ======================================================================

// The Monitor Ready PDU is sent from the server to the client to indicate
//  that the server is initialized and ready. This PDU is transmitted by the
//  server after it has sent the Clipboard Capabilities PDU to the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_MONITOR_READY (0x0001), while the
//  msgFlags field MUST be set to 0x0000.

struct ServerMonitorReadyPDU : public CliprdrHeader {
    ServerMonitorReadyPDU() : CliprdrHeader(CB_MONITOR_READY, 0, 0) {
    }   // ServerMonitorReadyPDU(bool response_ok)

    using CliprdrHeader::emit;
    using CliprdrHeader::recv;

};  // struct ServerMonitorReadyPDU

// [MS-RDPECLIP] 2.2.3.1 Format List PDU (CLIPRDR_FORMAT_LIST)
// ===========================================================

// The Format List PDU is sent by either the client or the server when its
//  local system clipboard is updated with new clipboard data. This PDU
//  contains the Clipboard Format ID and name pairs of the new Clipboard
//  Formats on the clipboard.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   formatListData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_LIST (0x0002), while the
//  msgFlags field MUST be set to 0x0000 or CB_ASCII_NAMES (0x0004) depending
//  on the type of data present in the formatListData field.

// formatListData (variable): An array consisting solely of either Short
//  Format Names or Long Format Names. The type of structure used in the
//  array is determined by the presence of the CB_USE_LONG_FORMAT_NAMES
//  (0x00000002) flag in the generalFlags field of the General Capability Set
//  (section 2.2.2.1.1.1). Each array holds a list of the Clipboard Format ID
//  and name pairs available on the local system clipboard of the sender. If
//  Short Format Names are being used, and the embedded Clipboard Format
//  names are in ASCII 8 format, then the msgFlags field of the clipHeader
//  must contain the CB_ASCII_NAMES (0x0004) flag.

enum : size_t {
    FORMAT_LIST_MAX_SIZE = 32
  , SHORT_NAME_MAX_SIZE  = 32
};

struct FormatListPDU : public CliprdrHeader {
    bool contians_data_in_text_format        = false;
    bool contians_data_in_unicodetext_format = false;

    FormatListPDU()
        : CliprdrHeader(CB_FORMAT_LIST, 0, 0)
        , contians_data_in_text_format(false)
        , contians_data_in_unicodetext_format(false) {}

    void emit(OutStream & stream) {
        this->dataLen_ = 36;    /* formatId(4) + formatName(32) */
        CliprdrHeader::emit(stream);

        // 1 CLIPRDR_SHORT_FORMAT_NAMES structures.
        stream.out_uint32_le(CF_TEXT);
        stream.out_clear_bytes(SHORT_NAME_MAX_SIZE); // formatName(32)
    }

    void emit_short(OutStream & stream, uint32_t const * formatListData, std::string const * formatListDataShortName, std::size_t formatListData_size) {

        // 1 CLIPRDR_SHORT_FORMAT_NAMES structures.
        if (formatListData_size > FORMAT_LIST_MAX_SIZE) {
            formatListData_size = FORMAT_LIST_MAX_SIZE;
        }

        this->dataLen_ = formatListData_size * (4 + SHORT_NAME_MAX_SIZE);    /* formatId(4) + formatName(32) */
        CliprdrHeader::emit(stream);


        for (std::size_t i = 0; i < formatListData_size; i++) {
            stream.out_uint32_le(formatListData[i]);
            std::string const & currentStr = formatListDataShortName[i];
            REDASSERT(currentStr.size() <= SHORT_NAME_MAX_SIZE);
            stream.out_copy_bytes(currentStr.data(), currentStr.size());
            stream.out_clear_bytes(SHORT_NAME_MAX_SIZE - currentStr.size()); // formatName(32)
        }
    }

    void emit_long(OutStream & stream, uint32_t const * formatListData, std::string const * formatListDatalongName, std::size_t formatListData_size) {
        if (formatListData_size > FORMAT_LIST_MAX_SIZE) {
            formatListData_size = FORMAT_LIST_MAX_SIZE;
        }

        for (std::size_t i = 0; i < formatListData_size; i++) {
            this->dataLen_ += formatListDatalongName[i].size() + 4;    /* formatId(4) + formatName(variable) */
        }
        REDASSERT(this->dataLen_ <= 1024);
        CliprdrHeader::emit(stream);

        for (std::size_t i = 0; i < formatListData_size; i++) {
            stream.out_uint32_le(formatListData[i]);
            std::string const & currentStr = formatListDatalongName[i];
            stream.out_copy_bytes(currentStr.data(), currentStr.size());

        }
    }

    void emit_ex(OutStream & stream, bool unicodetext) {
        this->dataLen_ = 36;    /* formatId(4) + formatName(32) */
        CliprdrHeader::emit(stream);

        // 1 CLIPRDR_SHORT_FORMAT_NAMES structures.
        stream.out_uint32_le(unicodetext ? CF_UNICODETEXT : CF_TEXT);
        stream.out_clear_bytes(32); // formatName(32)
    }

    void emit_long(OutStream & stream, bool unicodetext) {
        this->dataLen_ = 6; /* formatId(4) + formatName(2) */
        CliprdrHeader::emit(stream);

        // 1 CLIPRDR_LONG_FORMAT_NAMES structures.
        stream.out_uint32_le(unicodetext ? CF_UNICODETEXT : CF_TEXT);
        stream.out_clear_bytes(2); // formatName(2) - a single Unicode null character.
    }

    void emit_2(OutStream & stream, bool unicodetext, bool use_long_format_names) {
        if (use_long_format_names) {
            this->emit_long(stream, unicodetext);
        }
        else {
            this->emit_ex(stream, unicodetext);
        }
    }

    void emit_empty(OutStream & stream) {
        this->dataLen_ = 0;
        CliprdrHeader::emit(stream);
    }

    void recv(InStream & stream, const RecvFactory & recv_factory) {
        CliprdrHeader::recv(stream, recv_factory);

        // [MS-RDPECLIP] 2.2.3.1.1.1 Short Format Name (CLIPRDR_SHORT_FORMAT_NAME)
        // =======================================================================

        // The CLIPRDR_SHORT_FORMAT_NAME structure holds a Clipboard Format ID
        //  and Clipboard Format name pair.

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                            formatId                           |
        // +---------------------------------------------------------------+
        // |                      formatName (32 bytes)                    |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // formatId (4 bytes): An unsigned, 32-bit integer specifying the
        //  Clipboard Format ID.

        // formatName (32 bytes): A 32-byte block containing the
        //  null-terminated name assigned to the Clipboard Format (32 ASCII 8
        //  characters or 16 Unicode characters). If the name does not fit, it
        //  MUST be truncated. Not all Clipboard Formats have a name, and in
        //  that case the formatName field MUST contain only zeros.
        const uint32_t short_format_name_structure_size =
            36; /* formatId(4) + formatName(32) */

        // Parse PDU to find if clipboard data is available in a TEXT format.
        for (uint32_t i = 0; i < (dataLen_ / short_format_name_structure_size); i++) {
            if (!stream.in_check_rem(short_format_name_structure_size)) {
                LOG( LOG_INFO
                   , "RDPECLIP::FormatListPDU truncated CLIPRDR_SHORT_FORMAT_NAME structure, need=%u remains=%zu"
                   , short_format_name_structure_size, stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            uint32_t formatId = stream.in_uint32_le();
            //LOG(LOG_INFO, "RDPECLIP::FormatListPDU formatId=%u", formatId);

            if (formatId == CF_TEXT) {
                this->contians_data_in_text_format = true;
            }
            else if (formatId == CF_UNICODETEXT) {
                this->contians_data_in_unicodetext_format = true;
            }

            stream.in_skip_bytes(32);   // formatName(32)
        }
    }   // void recv(InStream & stream, const RecvFactory & recv_factory)
};  // struct FormatListPDU

// [MS-RDPECLIP] 2.2.3.2 Format List Response PDU (FORMAT_LIST_RESPONSE)
// =====================================================================
// The Format List Response PDU is sent as a reply to the Format List PDU. It
//  is used to indicate whether processing of the Format List PDU was
//  successful.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_LIST_RESPONSE (0x0003). The
//  CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set in
//  the msgFlags field of the Clipboard PDU Header.

struct FormatListResponsePDU : public CliprdrHeader {
    explicit FormatListResponsePDU(bool response_ok)
        : CliprdrHeader( CB_FORMAT_LIST_RESPONSE
                       , (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL)
                       , 0) {
    }   // FormatListResponsePDU(bool response_ok)

    using CliprdrHeader::emit;
    using CliprdrHeader::recv;
};  // struct FormatListResponsePDU

// [MS-RDPECLIP] 2.2.5.1 Format Data Request PDU (CLIPRDR_FORMAT_DATA_REQUEST)
// ===========================================================================

// The Format Data Request PDU is sent by the recipient of the Format List
//  PDU. It is used to request tFormatDataResponsePDUhe data for one of the formats that was listed
//  in the Format List PDU.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       requestedFormatId                       |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_DATA_REQUEST (0x0004), while
//  the msgFlags field MUST be set to 0x0000.

// requestedFormatId (4 bytes): An unsigned, 32-bit integer that specifies the
//  Clipboard Format ID of the clipboard data. The Clipboard Format ID MUST be
//  one listed previously in the Format List PDU.

struct FormatDataRequestPDU : public CliprdrHeader {
    uint32_t requestedFormatId;

    FormatDataRequestPDU()
            : CliprdrHeader(CB_FORMAT_DATA_REQUEST, 0, 4)
            , requestedFormatId(0) {
    }   // FormatDataRequestPDU()

    explicit FormatDataRequestPDU(uint32_t requestedFormatId)
            : CliprdrHeader(CB_FORMAT_DATA_REQUEST, 0, 4)
            , requestedFormatId(requestedFormatId) {
    }   // FormatDataRequestPDU(uint32_t requestedFormatId)

    void emit(OutStream & stream) {
        CliprdrHeader::emit(stream);

        stream.out_uint32_le(this->requestedFormatId);
    }   // void emit(OutStream & stream)

    void recv(InStream & stream, const RecvFactory & recv_factory) {
        CliprdrHeader::recv(stream, recv_factory);

        this->requestedFormatId = stream.in_uint32_le();
    }
};  // struct FormatDataRequestPDU






//2.2.5.3 File Contents Request PDU (CLIPRDR_FILECONTENTS_REQUEST)

//The File Contents Request PDU is sent by the recipient of the Format List PDU. It is used to request either the size of a remote file copied to the clipboard or a portion of the data in the file.


// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            streamId                           |
// +---------------------------------------------------------------+
// |                             lindex                            |
// +---------------------------------------------------------------+
// |                             dwFlags                           |
// +---------------------------------------------------------------+
// |                          nPositionLow                         |
// +---------------------------------------------------------------+
// |                          nPositionHigh                        |
// +---------------------------------------------------------------+
// |                           cbRequested                         |
// +---------------------------------------------------------------+
// |                       clipDataId (optional)                   |
// +---------------------------------------------------------------+


// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_FILECONTENTS_REQUEST (0x0008), while the msgFlags field MUST be set to 0x0000.

// streamId (4 bytes): An unsigned, 32-bit format ID used to associate the File Contents Request PDU with the corresponding File Contents Response PDU. The File Contents Response PDU is sent as a reply and contains an identical value in the streamId field.

// lindex (4 bytes): A signed, 32-bit integer that specifies the numeric ID of the remote file that is the target of the File Contents Request PDU. This field is used as an index that identifies a particular file in a File List. This File List SHOULD have been obtained as clipboard data in a prior Format Data Request PDU and Format Data Response PDU exchange.

// dwFlags (4 bytes): An unsigned, 32-bit integer that specifies the type of operation to be performed by the recipient.

//  +----------------------------+--------------------------------------------+
//  | Value                      | Meaning                                    |
//  +----------------------------+--------------------------------------------+
//  | FILECONTENTS_SIZE          | A request for the size of the file         |
//  | 0x00000001                 | identified by the lindex field. The size   |
//  |                            | MUST be returned as a 64-bit, unsigned     |
//  |                            | integer. The cbRequested field MUST be set |
//  |                            | to 0x00000008 and both the nPositionLow and|
//  |                            | nPositionHigh fields MUST be set to        |
//  |                            | 0x00000000.                                |
//  +----------------------------+--------------------------------------------+
//  | FILECONTENTS_RANGE         | A request for the data present in the file |
//  | 0x00000002                 | identified by the lindex field. The data to|
//  |                            | be retrieved is extracted starting from the|
//  |                            | offset given by the nPositionLow and       |
//  |                            | nPositionHigh fields. The maximum number   |
//  |                            | of bytes to extract is specified by the    |
//  |                            | cbRequested field.                         |
//  +----------------------------+--------------------------------------------+

//    The FILECONTENTS_SIZE and FILECONTENTS_RANGE flags MUST NOT be set at the same time.

// nPositionLow (4 bytes): An unsigned, 32-bit integer that specifies the low bytes of the offset into the remote file, identified by the lindex field, from where the data needs to be extracted to satisfy a FILECONTENTS_RANGE operation.

// nPositionHigh (4 bytes): An unsigned, 32-bit integer that specifies the high bytes of the offset into the remote file, identified by the lindex field, from where the data needs to be extracted to satisfy a FILECONTENTS_RANGE operation. This field is currently not used because offsets greater than 4,294,967,295 bytes are not supported, and MUST be set to zero.

// cbRequested (4 bytes): An unsigned, 32-bit integer that specifies the size, in bytes, of the data to retrieve. For a FILECONTENTS_SIZE operation, this field MUST be set to 0x00000008. In the case of a FILECONTENTS_RANGE operation, this field contains the maximum number of bytes to read from the remote file.

// clipDataId (4 bytes): An optional unsigned, 32-bit integer that identifies File Stream data which was tagged in a prior Lock Clipboard Data PDU (section 2.2.4.1).

enum : int {
    FILECONTENTS_SIZE              = 0x00000001
  , FILECONTENTS_RANGE             = 0x00000002
  , FILECONTENTS_SIZE_CB_REQUESTED = 0x00000008
};

struct FileContentsRequestPDU : CliprdrHeader {
    explicit FileContentsRequestPDU(bool response_ok = false)
    : CliprdrHeader( CB_FILECONTENTS_REQUEST, (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL), 24)
    {}

    void emit(OutStream & stream, int streamID, int flag, int lindex, uint64_t sizeRequested) {
        stream.out_uint16_le(this->msgType_);
        stream.out_uint16_le(this->msgFlags_);
        stream.out_uint32_le(this->dataLen_);
        stream.out_uint32_le(streamID);
        stream.out_uint32_le(lindex);
        stream.out_uint32_le(flag);
        stream.out_uint32_le(sizeRequested >> 32);
        stream.out_uint32_le(sizeRequested);
        stream.out_uint32_le(sizeRequested);
    }
};



    // 2.2.5.4 File Contents Response PDU (CLIPRDR_FILECONTENTS_RESPONSE)

    // The File Contents Response PDU is sent as a reply to the File Contents Request PDU. It is used to indicate whether processing of the File Contents Request PDU was successful. If the processing was successful, the File Contents Response PDU includes either a file size or extracted file data, based on the operation requested in the corresponding File Contents Request PDU.

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                          clipHeader                           |
    // +---------------------------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+
    // |                           streamId                            |
    // +---------------------------------------------------------------+
    // |              requestedFileContentsData (variable)             |
    // +---------------------------------------------------------------+
    // |                             ...                               |
    // +---------------------------------------------------------------+

    // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_FILECONTENTS_RESPONSE (0x0009). The CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set in the msgFlags field of the Clipboard PDU Header.

    // streamId (4 bytes): An unsigned, 32-bit numeric ID used to associate the File Contents Response PDU with the corresponding File Contents Request PDU. The File Contents Request PDU that triggered the response MUST contain an identical value in the streamId field.

    // requestedFileContentsData (variable): This field contains a variable number of bytes. If the response is to a FILECONTENTS_SIZE (0x00000001) operation, the requestedFileContentsData field holds a 64-bit, unsigned integer containing the size of the file. In the case of a FILECONTENTS_RANGE (0x00000002) operation, the requestedFileContentsData field contains a byte-stream of data extracted from the file.

struct FileContentsResponse : CliprdrHeader {
    explicit FileContentsResponse(bool response_ok = false)
    : CliprdrHeader( CB_FILECONTENTS_RESPONSE, (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL), 4)
    {}

    void emit(OutStream & stream) {
        CliprdrHeader::emit(stream);
    }

    void emit_size(OutStream & stream, const uint32_t streamID, const uint64_t size) {
        this->dataLen_ += 12;
        CliprdrHeader::emit(stream);
        stream.out_uint32_le(streamID);
        stream.out_uint64_le(size);
        stream.out_uint32_le(0);
    }

    void emit_range(OutStream & stream, const uint32_t streamID, const uint64_t size) {
        this->dataLen_ += size;                          //  requestedFileContentsData range
        CliprdrHeader::emit(stream);
        stream.out_uint32_le(streamID);
    }
};




struct PacketFileList {
    uint32_t cItems;
    /*variable fileDescriptorArray*/
};

// [MS-RDPECLIP] - 2.2.5.2.3.1 File Descriptor (CLIPRDR_FILEDESCRIPTOR)
// ====================================================================

// The CLIPRDR_FILEDESCRIPTOR structure describes the properties of a file.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             flags                             |
// +---------------------------------------------------------------+
// |                           reserved1                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         fileAttributes                        |
// +---------------------------------------------------------------+
// |                           reserved2                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         lastWriteTime                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                          fileSizeHigh                         |
// +---------------------------------------------------------------+
// |                          fileSizeLow                          |
// +---------------------------------------------------------------+
// |                            fileName                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 (fileName cont'd for 122 rows)                |
// +---------------------------------------------------------------+

// flags (4 bytes): An unsigned 32-bit integer that specifies which fields
//  contain valid data and the usage of progress UI during a copy operation.

//  +-------------------+---------------------------------------------------+
//  | Value             | Meaning                                           |
//  +-------------------+---------------------------------------------------+
//  | FD_ATTRIBUTES     | The fileAttributes field contains valid data.     |
//  | 0x00000004        |                                                   |
//  +-------------------+---------------------------------------------------+
//  | FD_FILESIZE       | The fileSizeHigh and fileSizeLow fields contain   |
//  | 0x00000040        | valid data.                                       |
//  +-------------------+---------------------------------------------------+
//  | FD_WRITESTIME     | The lastWriteTime field contains valid data.      |
//  | 0x00000020        |                                                   |
//  +-------------------+---------------------------------------------------+
//  | FD_SHOWPROGRESSUI | A progress indicator SHOULD be shown when copying |
//  | 0x00004000        | the file.                                         |
//  +-------------------+---------------------------------------------------+

enum {
    FD_ATTRIBUTES     = 0x0004,
    FD_FILESIZE       = 0x0040,
    FD_WRITESTIME     = 0x0020,
    FD_SHOWPROGRESSUI = 0x4000
};

// reserved1 (32 bytes): An array of 32 bytes. This field MUST be initialized
//  with zeros when sent and MUST be ignored on receipt.

// fileAttributes (4 bytes): An unsigned 32-bit integer that specifies file
//  attribute flags.

//  +--------------------------+-----------------------------------------------+
//  | Value                    | Meaning                                       |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_READONLY  | A file that is read-only. Applications can    |
//  | 0x00000001               | read the file, but cannot write to it or      |
//  |                          | delete it.                                    |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_HIDDEN    | The file or directory is hidden. It is not    |
//  | 0x00000002               | included in an ordinary directory listing.    |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_SYSTEM    | A file or directory that the operating system |
//  | 0x00000004               | uses a part of, or uses exclusively.          |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_DIRECTORY | Identifies a directory.                       |
//  | 0x00000010               |                                               |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_ARCHIVE   | A file or directory that is an archive file   |
//  | 0x00000020               | or directory. Applications typically use this |
//  |                          | attribute to mark files for backup or         |
//  |                          | removal.                                      |
//  +--------------------------+-----------------------------------------------+
//  | FILE_ATTRIBUTE_NORMAL    | A file that does not have other attributes    |
//  | 0x00000080               | set. This attribute is valid only when used   |
//  |                          | alone.                                        |
//  +--------------------------+-----------------------------------------------+

enum FileAttributes : uint32_t {
    FILE_ATTRIBUTES_READONLY  = 0x0001,
    FILE_ATTRIBUTES_HIDDEN    = 0x0002,
    FILE_ATTRIBUTES_SYSTEM    = 0x0004,
    FILE_ATTRIBUTES_DIRECTORY = 0x0010,
    FILE_ATTRIBUTES_ARCHIVE   = 0x0020,
    FILE_ATTRIBUTES_NORMAL    = 0x0080
};

// reserved2 (16 bytes): An array of 16 bytes. This field MUST be initialized
//  with zeros when sent and MUST be ignored on receipt.

// lastWriteTime (8 bytes): An unsigned 64-bit integer that specifies the
//  number of 100-nanoseconds intervals that have elapsed since 1 January
//  1601 to the time of the last write operation on the file.

// fileSizeHigh (4 bytes): An unsigned 32-bit integer that contains the most
//  significant 4 bytes of the file size.

// fileSizeLow (4 bytes): An unsigned 32-bit integer that contains the least
//  significant 4 bytes of the file size.

// fileName (520 bytes): A null-terminated 260 character Unicode string that
//  contains the name of the file.

class FileDescriptor {

public:
    uint32_t flags;
    uint32_t fileAttributes;
    uint64_t lastWriteTime;
    uint32_t fileSizeHigh;
    uint32_t fileSizeLow;
    std::string file_name;

public:
    void emit(OutStream & stream) const {
        stream.out_uint32_le(this->flags);

        stream.out_clear_bytes(32); // reserved1(32)

        stream.out_uint32_le(this->fileAttributes);

        stream.out_clear_bytes(16); // reserved2(16)

        stream.out_uint64_le(this->lastWriteTime);
        stream.out_uint32_le(this->fileSizeHigh);
        stream.out_uint32_le(this->fileSizeLow);

        // The null-terminator is included.
        uint8_t fileName_unicode_data[520]; // fileName(520)

        size_t size_of_fileName_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->file_name.c_str()),
            fileName_unicode_data, sizeof(fileName_unicode_data));

        stream.out_copy_bytes(fileName_unicode_data,
            size_of_fileName_unicode_data);

        stream.out_clear_bytes(520 /* fileName(520) */ -
            size_of_fileName_unicode_data);
    }

    void receive(InStream & stream) {
        {
            const unsigned expected = 592;  // flags(4) + reserved1(32) +
                                            //     fileAttributes(4) +
                                            //     reserved2(16) +
                                            //     lastWriteTime(8) +
                                            //     fileSizeHigh(4) +
                                            //     fileSizeLow(4) +
                                            //     fileName(520)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileDescriptor: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        this->flags = stream.in_uint32_le();

        stream.in_skip_bytes(32);   // reserved1(32)

        this->fileAttributes = stream.in_uint32_le();

        stream.in_skip_bytes(16);   // reserved2(16)

        this->lastWriteTime = stream.in_uint64_le();
        this->fileSizeHigh  = stream.in_uint32_le();
        this->fileSizeLow   = stream.in_uint32_le();

        uint8_t const * const fileName_unicode_data = stream.get_current();
        uint8_t fileName_utf8_string[520 /* fileName(520) */ / sizeof(uint16_t) * maximum_length_of_utf8_character_in_bytes];
        ::UTF16toUTF8(fileName_unicode_data, 520 /* fileName(520) */ / 2,
            fileName_utf8_string, sizeof(fileName_utf8_string));
        // The null-terminator is included.
        this->file_name = ::char_ptr_cast(fileName_utf8_string);

        stream.in_skip_bytes(520);       // fileName(520)

    }

    const char * fileName() const { return this->file_name.c_str(); }

    uint64_t file_size() const {
        return (static_cast<uint64_t>(this->fileSizeLow)) |
            (static_cast<uint64_t>(this->fileSizeHigh) << 32);
    }

    static constexpr size_t size() {
        return 592; // flags(4) + reserved1(32) + fileAttributes(4) +
                    //     reserved2(16) + lastWriteTime(8) +
                    //     fileSizeHigh(4) + fileSizeLow(4) +
                    //     fileName(520)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileDescriptor: flags=0x%X fileAttributes=0x%X lastWriteTime=%" PRIu64 " "
                "fileSizeHigh=0x%X fileSizeLow=0x%X "
                "fileName=\"%s\"",
            this->flags, this->fileAttributes, this->lastWriteTime,
            this->fileSizeHigh, this->fileSizeLow, this->file_name.c_str());
        return ((length < size) ? length : size - 1);
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class FileDescriptor



// 2.1.1.18 MetafileType Enumeration

// The MetafileType Enumeration specifies where the metafile is stored.

enum {
    MEMORYMETAFILE = 0x0001,
    DISKMETAFILE   = 0x0002
};

// MEMORYMETAFILE:  Metafile is stored in memory.

// DISKMETAFILE:  Metafile is stored on disk.



// 2.1.1.19 MetafileVersion Enumeration

// The MetafileVersion Enumeration defines values that specify support for device-independent bitmaps (DIBs) in metafiles.

enum {
    METAVERSION100 = 0x0100,
    METAVERSION300 = 0x0300
};

// METAVERSION100:  DIBs are not supported.

// METAVERSION300:  DIBs are supported.



// 2.2.5.2.1 Packed Metafile Payload (CLIPRDR_MFPICT)

// The CLIPRDR_MFPICT structure is used to transfer a Windows metafile. The Windows metafile format is specified in [MS-WMF] section 2.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          mappingMode                          |
// +---------------------------------------------------------------+
// |                             xExt                              |
// +---------------------------------------------------------------+
// |                             yExt                              |
// +---------------------------------------------------------------+
// |                     metaFileData (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// mappingMode (4 bytes): An unsigned, 32-bit integer specifying the mapping mode in which the picture is drawn.

enum {
    MM_TEXT        = 0x00000001, // Each logical unit is mapped to one device pixel. Positive x is to the right; positive y is down.
    MM_LOMETRIC    = 0x00000002, // Each logical unit is mapped to 0.1 millimeter. Positive x is to the right; positive y is up.
    MM_HIMETRIC    = 0x00000003, // Each logical unit is mapped to 0.01 millimeter. Positive x is to the right; positive y is up.
    MM_LOENGLISH   = 0x00000004, // Each logical unit is mapped to 0.01 inch. Positive x is to the right; positive y is up.
    MM_HIENGLISH   = 0x00000005, // Each logical unit is mapped to 0.001 inch. Positive x is to the right; positive y is up.
    MM_TWIPS       = 0x00000006, // Each logical unit is mapped to 1/20 of a printer's point (1/1440 of an inch), also called a twip. Positive x is to the right; positive y is up.
    MM_ISOTROPIC   = 0x00000007, // ogical units are mapped to arbitrary units with equally scaled axes; one unit along the x-axis is equal to one unit along the y-axis.
    MM_ANISOTROPIC = 0x00000008  // Logical units are mapped to arbitrary units with arbitrarily scaled axes.
};

//    For MM_ISOTROPIC and MM_ANISOTROPIC modes, which can be scaled, the xExt and yExt fields contain an optional suggested size in MM_HIMETRIC units. For MM_ANISOTROPIC pictures, xExt and yExt SHOULD be zero when no suggested size is given. For MM_ISOTROPIC pictures, an aspect ratio MUST be supplied even when no suggested size is given. If a suggested size is given, the aspect ratio is implied by the size. To give an aspect ratio without implying a suggested size, the xExt and yExt fields are set to negative values whose ratio is the appropriate aspect ratio. The magnitude of the negative xExt and yExt values is ignored; only the ratio is used.

//xExt (4 bytes): An unsigned, 32-bit integer that specifies the width of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

//yExt (4 bytes): An unsigned, 32-bit integer that specifies the height of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

//metaFileData (variable): The variable sized contents of the metafile as specified in [MS-WMF] section 2

// 2.1.1.1 RecordType Enumeration

// The RecordType Enumeration defines the types of records that can be used in WMF metafiles.


enum {
   META_EOF                   = 0x0000,
   META_REALIZEPALETTE        = 0x0035,
   META_SETPALENTRIES         = 0x0037,
   META_SETBKMODE             = 0x0102,
   META_SETMAPMODE            = 0x0103,
   META_SETROP2               = 0x0104,
   META_SETRELABS             = 0x0105,
   META_SETPOLYFILLMODE       = 0x0106,
   META_SETSTRETCHBLTMODE     = 0x0107,
   META_SETTEXTCHAREXTRA      = 0x0108,
   META_RESTOREDC             = 0x0127,
   META_RESIZEPALETTE         = 0x0139,
   META_DIBCREATEPATTERNBRUSH = 0x0142,
   META_SETLAYOUT             = 0x0149,
   META_SETBKCOLOR            = 0x0201,
   META_SETTEXTCOLOR          = 0x0209,
   META_OFFSETVIEWPORTORG     = 0x0211,
   META_LINETO                = 0x0213,
   META_MOVETO                = 0x0214,
   META_OFFSETCLIPRGN         = 0x0220,
   META_FILLREGION            = 0x0228,
   META_SETMAPPERFLAGS        = 0x0231,
   META_SELECTPALETTE         = 0x0234,
   META_POLYGON               = 0x0324,
   META_POLYLINE              = 0x0325,
   META_SETTEXTJUSTIFICATION  = 0x020A,
   META_SETWINDOWORG          = 0x020B,
   META_SETWINDOWEXT          = 0x020C,
   META_SETVIEWPORTORG        = 0x020D,
   META_SETVIEWPORTEXT        = 0x020E,
   META_OFFSETWINDOWORG       = 0x020F,
   META_SCALEWINDOWEXT        = 0x0410,
   META_SCALEVIEWPORTEXT      = 0x0412,
   META_EXCLUDECLIPRECT       = 0x0415,
   META_INTERSECTCLIPRECT     = 0x0416,
   META_ELLIPSE               = 0x0418,
   META_FLOODFILL             = 0x0419,
   META_FRAMEREGION           = 0x0429,
   META_ANIMATEPALETTE        = 0x0436,
   META_TEXTOUT               = 0x0521,
   META_POLYPOLYGON           = 0x0538,
   META_EXTFLOODFILL          = 0x0548,
   META_RECTANGLE             = 0x041B,
   META_SETPIXEL              = 0x041F,
   META_ROUNDRECT             = 0x061C,
   META_PATBLT                = 0x061D,
   META_SAVEDC                = 0x001E,
   META_PIE                   = 0x081A,
   META_STRETCHBLT            = 0x0B23,
   META_ESCAPE                = 0x0626,
   META_INVERTREGION          = 0x012A,
   META_PAINTREGION           = 0x012B,
   META_SELECTCLIPREGION      = 0x012C,
   META_SELECTOBJECT          = 0x012D,
   META_SETTEXTALIGN          = 0x012E,
   META_ARC                   = 0x0817,
   META_CHORD                 = 0x0830,
   META_BITBLT                = 0x0922,
   META_EXTTEXTOUT            = 0x0a32,
   META_SETDIBTODEV           = 0x0d33,
   META_DIBBITBLT             = 0x0940,
   META_DIBSTRETCHBLT         = 0x0b41,
   META_STRETCHDIB            = 0x0f43,
   META_DELETEOBJECT          = 0x01f0,
   META_CREATEPALETTE         = 0x00f7,
   META_CREATEPATTERNBRUSH    = 0x01F9,
   META_CREATEPENINDIRECT     = 0x02FA,
   META_CREATEFONTINDIRECT    = 0x02FB,
   META_CREATEBRUSHINDIRECT   = 0x02FC,
   META_CREATEREGION          = 0x06FF
 };

// META_EOF:  This record specifies the end of the file, the last record in the metafile.

// META_REALIZEPALETTE:  This record maps entries from the logical palette that is defined in the playback device context to the system palette.

// META_SETPALENTRIES:  This record defines red green blue (RGB) color values in a range of entries in the logical palette that is defined in the playback device context.

// META_SETBKMODE:  This record defines the background raster operation mix mode in the playback device context. The background mix mode is the mode for combining pens, text, hatched brushes, and interiors of filled objects with background colors on the output surface.

// META_SETMAPMODE:  This record defines the mapping mode in the playback device context. The mapping mode defines the unit of measure used to transform page-space coordinates into coordinates of the output device, and also defines the orientation of the device's x and y axes.

// META_SETROP2:  This record defines the foreground raster operation mix mode in the playback device context. The foreground mix mode is the mode for combining pens and interiors of filled objects with foreground colors on the output surface.

// META_SETRELABS:  This record is undefined and MUST be ignored.

// META_SETPOLYFILLMODE:  This record defines polygon fill mode in the playback device context for graphics operations that fill polygons.

// META_SETSTRETCHBLTMODE:  This record defines the bitmap stretching mode in the playback device context.

// META_SETTEXTCHAREXTRA:  This record defines inter-character spacing for text justification in the playback device context. Spacing is added to the white space between each character, including break characters, when a line of justified text is output.

// META_RESTOREDC:  This record restores the playback device context from a previously saved device context.

// META_RESIZEPALETTE:  This record redefines the size of the logical palette that is defined in the playback device context.

// META_DIBCREATEPATTERNBRUSH:  This record defines a brush with a pattern specified by a device-independent bitmap (DIB).

// META_SETLAYOUT:  This record defines the layout orientation in the playback device context.<2>

// META_SETBKCOLOR:  This record sets the background color in the playback device context to a specified color, or to the nearest physical color if the device cannot represent the specified color.

// META_SETTEXTCOLOR:  This record defines the text color in the playback device context.

// META_OFFSETVIEWPORTORG:  This record moves the viewport origin in the playback device context by using specified horizontal and vertical offsets.

// META_LINETO:  This record draws a line from the output position that is defined in the playback device context up to, but not including, a specified point.

// META_MOVETO:  This record sets the output position in the playback device context to a specified point.

// META_OFFSETCLIPRGN:  This record moves the clipping region that is defined in the playback device context by specified offsets.

// META_FILLREGION:  This record fills a region by using a specified brush.

// META_SETMAPPERFLAGS:  This record defines the algorithm that the font mapper uses when it maps logical fonts to physical fonts.

// META_SELECTPALETTE:  This record specifies the logical palette in the playback device context.

// META_POLYGON:  This record paints a polygon consisting of two or more vertices connected by straight lines. The polygon is outlined by using the pen and filled by using the brush and polygon fill mode; these are defined in the playback device context.

// META_POLYLINE:  This record draws a series of line segments by connecting the points in a specified array.

// META_SETTEXTJUSTIFICATION:  This record defines the amount of space to add to break characters in a string of justified text.

// META_SETWINDOWORG:  This record defines the output window origin in the playback device context.

// META_SETWINDOWEXT:  This record defines the horizontal and vertical extents of the output window in the playback device context.

// META_SETVIEWPORTORG:  This record defines the viewport origin in the playback device context.

// META_SETVIEWPORTEXT:  This record defines the horizontal and vertical extents of the viewport in the playback device context.

// META_OFFSETWINDOWORG:  This record moves the output window origin in the playback device context by using specified horizontal and vertical offsets.

// META_SCALEWINDOWEXT:  This record scales the horizontal and vertical extents of the output window that is defined in the playback device context by using the ratios formed by specified multiplicands and divisors.

// META_SCALEVIEWPORTEXT:  This record scales the horizontal and vertical extents of the viewport that is defined in the playback device context by using the ratios formed by specified multiplicands and divisors.

// META_EXCLUDECLIPRECT:  This record sets the clipping region that is defined in the playback device context to the existing clipping region minus a specified rectangle.

// META_INTERSECTCLIPRECT:  This record sets the clipping region that is defined in the playback device context to the intersection of the existing clipping region and a specified rectangle.

// META_ELLIPSE:  This record defines an ellipse. The center of the ellipse is the center of a specified bounding rectangle. The ellipse is outlined by using the pen and is filled by using the brush; these are defined in the playback device context.

// META_FLOODFILL:  This record fills an area of the display surface with the brush that is defined in the playback device context.

// META_FRAMEREGION:  This record defines a border around a specified region by using a specified brush.

// META_ANIMATEPALETTE:  This record redefines entries in the logical palette that is defined in the playback device context.

// META_TEXTOUT:  This record outputs a character string at a specified location using the font, background color, and text color; these are defined in the playback device context.

// META_POLYPOLYGON:  This record paints a series of closed polygons. Each polygon is outlined by using the pen and filled by using the brush and polygon fill mode; these are defined in the playback device context. The polygons drawn in this operation can overlap.

// META_EXTFLOODFILL:  This record fills an area with the brush that is defined in the playback device context.

// META_RECTANGLE:  This record paints a rectangle. The rectangle is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_SETPIXEL:  This record sets the pixel at specified coordinates to a specified color.

// META_ROUNDRECT:  This record draws a rectangle with rounded corners. The rectangle is outlined by using the current pen and filled by using the current brush.

// META_PATBLT:  This record paints the specified rectangle by using the brush that is currently selected into the playback device context. The brush color and the surface color or colors are combined using the specified raster operation.

// META_SAVEDC:  This record saves the playback device context for later retrieval.

// META_PIE:  This record draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials. The pie is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_STRETCHBLT:  This record specifies the transfer of a block of pixels according to a raster operation, with possible expansion or contraction.

// META_ESCAPE:  This record makes it possible to access capabilities of a particular printing device that are not directly available through other WMF records.

// META_INVERTREGION:  This record inverts the colors in a specified region.

// META_PAINTREGION:  This record paints a specified region by using the brush that is defined in the playback device context.

// META_SELECTCLIPREGION:  This record specifies the clipping region in the playback device context.

// META_SELECTOBJECT:  This record specifies a graphics object in the playback device context. The new object replaces the previous object of the same type, if one is defined.

// META_SETTEXTALIGN:  This record defines the text-alignment values in the playback device context.

// META_ARC:  This record draws an elliptical arc.

// META_CHORD:  This record draws a chord, which is a region bounded by the intersection of an ellipse and a line segment. The chord is outlined by using the pen and filled by using the brush; these are defined in the playback device context.

// META_BITBLT:  This record specifies the transfer of a block of pixels according to a raster operation.

// META_EXTTEXTOUT:  This record outputs a character string by using the font, background color, and text color; these are defined in the playback device context. Optionally, dimensions can be provided for clipping, opaquing, or both.

// META_SETDIBTODEV:  This record sets a block of pixels using device-independent color data.

// META_DIBBITBLT:  This record specifies the transfer of a block of pixels in device-independent format according to a raster operation.

// META_DIBSTRETCHBLT:  This record specifies the transfer of a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.

// META_STRETCHDIB:  This record specifies the transfer of color data from a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.

// META_DELETEOBJECT:  This record deletes a graphics object, which can be a pen, brush, font, region, or palette.

// META_CREATEPALETTE:  This record defines a logical palette.

// META_CREATEPATTERNBRUSH:  This record defines a brush with a pattern specified by a DIB.

// META_CREATEPENINDIRECT:  This record defines a pen with specified style, width, and color.

// META_CREATEFONTINDIRECT:  This record defines a font with specified characteristics.

// META_CREATEBRUSHINDIRECT:  This record defines a brush with specified style, color, and pattern.

// META_CREATEREGION:  This record defines a region.

// The high-order byte of the WMF record type values SHOULD be ignored for all record types except the following.<3>

//     META_BITBLT

//     META_DIBBITBLT

//     META_DIBSTRETCHBLT

//     META_POLYGON

//     META_POLYLINE

//     META_SETPALENTRIES

//     META_STRETCHBLT

// The meanings of the high-order bytes of these record type fields are specified in the respective sections that define them.

// A record type is not defined for the WMF Header record, because only one can be present as the first record in the metafile.


// [MS-RDPECLIP] 2.2.5.2 Format Data Response PDU (CLIPRDR_FORMAT_DATA_RESPONSE)
// =============================================================================

// The Format Data Response PDU is sent as a reply to the Format Data Request
//  PDU. It is used to indicate whether processing of the Format Data Request
//  PDU was successful. If the processing was successful, the Format Data
//  Response PDU includes the contents of the requested clipboard data.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           clipHeader                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                 requestedFormatData (variable)                |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the
//  Clipboard PDU Header MUST be set to CB_FORMAT_DATA_RESPONSE (0x0005). The
//  CB_RESPONSE_OK (0x0001) or CB_RESPONSE_FAIL (0x0002) flag MUST be set in
//  the msgFlags field of the Clipboard PDU Header structure.

// requestedFormatData (variable): Variable length clipboard format data. The
//  contents of this field MUST be one of the following types: generic, Packed
//  Metafile Payload, or Packed Palette Payload.

enum : int {
      METAFILE_HEADERS_SIZE          = 130
    , META_DIBSTRETCHBLT_HEADER_SIZE = 66
    , METAFILE_WORDS_HEADER_SIZE     = 118
};

enum : uint32_t {
    FILE_ATTRIBUTE_READONLY  = 0x0001,
    FILE_ATTRIBUTE_HIDDEN    = 0x0002,
    FILE_ATTRIBUTE_SYSTEM    = 0x0004,
    FILE_ATTRIBUTE_DIRECTORY = 0x0010,
    FILE_ATTRIBUTE_ARCHIVE   = 0x0020,
    FILE_ATTRIBUTE_NORMAL    = 0x0080
};

enum : uint64_t {
    TIME64_FILE_LIST = 0x01d1e2a0379fb504
};

struct FormatDataResponsePDU : public CliprdrHeader {

    const double ARBITRARY_SCALE = 40;

    FormatDataResponsePDU()
        : CliprdrHeader( CB_FORMAT_DATA_RESPONSE
                       , CB_RESPONSE_FAIL
                       , 0) {
    }

    explicit FormatDataResponsePDU(bool response_ok)
        : CliprdrHeader( CB_FORMAT_DATA_RESPONSE
                       , (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL)
                       , 0) {
    }

    void emit(OutStream & stream, const uint8_t * data, size_t data_length) const {
        stream.out_uint16_le(this->msgType_);
        stream.out_uint16_le(this->msgFlags_);

        if (this->msgFlags_ == CB_RESPONSE_OK) {
            stream.out_uint32_le(data_length);  // dataLen(4)

            if (data_length) {
                stream.out_copy_bytes(data, data_length);
            }
        }
        else {
            stream.out_uint32_le(0);    // dataLen(4)
        }
    }

    void emit_ex(OutStream & stream, size_t data_length) const {
        stream.out_uint16_le(this->msgType_);
        stream.out_uint16_le(this->msgFlags_);

        stream.out_uint32_le(                           // dataLen(4)
                (this->msgFlags_ == CB_RESPONSE_OK) ?
                data_length :
                0
            );
    }




    // 2.2.5.2.3 Packed File List (CLIPRDR_FILELIST)

    //  The CLIPRDR_FILELIST structure is used to describe a list of files, each file in the list being represented by a File Descriptor (section 2.2.5.2.3.1).

    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
    // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
    // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    // |                            cItems                             |
    // +---------------------------------------------------------------+
    // |                 fileDescriptorArray (variable)                |
    // +---------------------------------------------------------------+
    // |                              ...                              |
    // +---------------------------------------------------------------+

    // cItems (4 bytes): An unsigned 32-bit integer that specifies the number of entries in the fileDescriptorArray field.

    // fileDescriptorArray (variable): An array of File Descriptors (section 2.2.5.2.3.1). The number of elements in the array is specified by the cItems field.

    void emit_fileList(OutStream & stream, const int cItems, const std::string name, const uint64_t size) {
        this->dataLen_ = (FileDescriptor::size() * cItems) + 4;
        CliprdrHeader::emit(stream);

        stream.out_uint32_le(cItems);

        stream.out_uint32_le(FD_SHOWPROGRESSUI |
                             FD_FILESIZE       |
                             FD_WRITESTIME     |
                             FD_ATTRIBUTES
                            );
        stream.out_clear_bytes(32);
        stream.out_uint32_le(FILE_ATTRIBUTE_ARCHIVE);
        stream.out_clear_bytes(16);
        stream.out_uint64_le(TIME64_FILE_LIST);
        stream.out_uint32_le(size >> 32);
        stream.out_uint32_le(size);
        size_t sizeName(name.size());
        if (sizeName > 520) {
            sizeName = 520;
        }
        stream.out_copy_bytes(name.data(), sizeName);;
        stream.out_clear_bytes(520 - sizeName);
    }

    void emit_text(OutStream & stream, uint32_t length) {
        this->dataLen_ = length;
        CliprdrHeader::emit(stream);
    }

    void emit_metaFilePic(OutStream & stream, const uint32_t data_length, const uint16_t width, const uint16_t height, const uint16_t depth, const double ARBITRARY_SCALE) {
        this->dataLen_ = data_length + METAFILE_HEADERS_SIZE;
        CliprdrHeader::emit(stream);

        const int largeRecordWordsSize((data_length + META_DIBSTRETCHBLT_HEADER_SIZE)/2);


        // 2.2.5.2.1 Packed Metafile Payload
        // 12 bytes

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                          mappingMode                          |
        // +---------------------------------------------------------------+
        // |                             xExt                              |
        // +---------------------------------------------------------------+
        // |                             yExt                              |
        // +---------------------------------------------------------------+
        // |                    metaFileData (variable)                    |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // mappingMode (4 bytes): An unsigned, 32-bit integer specifying the mapping mode in which the picture is drawn.

        //  +----------------------------+--------------------------------------------+
        //  | Value                      | Meaning                                    |
        //  +----------------------------+--------------------------------------------+
        //  | MM_TEXT                    | Each logical unit is mapped to one device  |
        //  |                            | pixel. Positive x is to the right; positive|
        //  | 0x00000001                 | y is down.                                 |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_LOMETRIC                | Each logical unit is mapped to 0.1         |
        //  |                            | millimeter. Positive x is to the right;    |
        //  | 0x00000002                 | positive y is up.                          |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_HIMETRIC                | Each logical unit is mapped to 0.01        |
        //  |                            | millimeter. Positive x is to the right;    |
        //  | 0x00000003                 | positive y is up.                          |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_LOENGLISH               | Each logical unit is mapped to 0.01        |
        //  |                            | inch. Positive x is to the right;          |
        //  | 0x00000004                 | positive y is up.                          |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_HIENGLISH               | Each logical unit is mapped to 0.001       |
        //  |                            | inch. Positive x is to the right;          |
        //  | 0x00000005                 | positive y is up.                          |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_TWIPS                   | Each logical unit is mapped to 1/20 of a   |
        //  |                            | printer's point (1/1440 of an inch), also  |
        //  | 0x00000006                 | called a twip. Positive x is to the right; |
        //  |                            | positive y is up.                          |                                                   //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_ISOTROPIC               | Logical units are mapped to arbitrary units|
        //  |                            | with equally scaled axes; one unit along   |
        //  | 0x00000007                 | the x-axis is equal to one unit along the  |
        //  |                            | y-axis.                                    |                                                   //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+
        //  | MM_ANISOTROPIC             | Logical units are mapped to arbitrary units|
        //  |                            | with arbitrarily scaled axes.              |
        //  | 0x00000008                 |                                            |
        //  |                            |                                            |
        //  +----------------------------+--------------------------------------------+

        // For MM_ISOTROPIC and MM_ANISOTROPIC modes, which can be scaled, the xExt and yExt fields contain an optional suggested size in MM_HIMETRIC units. For MM_ANISOTROPIC pictures, xExt and yExt SHOULD be zero when no suggested size is given. For MM_ISOTROPIC pictures, an aspect ratio MUST be supplied even when no suggested size is given. If a suggested size is given, the aspect ratio is implied by the size. To give an aspect ratio without implying a suggested size, the xExt and yExt fields are set to negative values whose ratio is the appropriate aspect ratio. The magnitude of the negative xExt and yExt values is ignored; only the ratio is used.

        // xExt (4 bytes): An unsigned, 32-bit integer that specifies the width of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

        // yExt (4 bytes): An unsigned, 32-bit integer that specifies the height of the rectangle within which the picture is drawn, except in the MM_ISOTROPIC and MM_ANISOTROPIC modes. The coordinates are in units that correspond to the mapping mode.

        // metaFileData (variable): The variable sized contents of the metafile as specified in [MS-WMF] section 2.

        stream.out_uint32_le(MM_ANISOTROPIC);
        stream.out_uint32_le(int(double(width)  * ARBITRARY_SCALE));
        stream.out_uint32_le(int(double(height) * ARBITRARY_SCALE));


        // 3.2.1 META_HEADER Example

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |             Type              |          HeaderSize           |
        // +-------------------------------+-------------------------------+
        // |           Version             |             Size              |
        // +-------------------------------+-------------------------------+
        // |             ...               |        NumberOfObjects        |
        // +-------------------------------+-------------------------------+
        // |                     metaFileData (variable)                   |
        // +-------------------------------+-------------------------------+
        // |       NumberOfMembers         |                               |
        // +-------------------------------+-------------------------------+

        // Type: 0x0001 specifies the type of metafile from the MetafileType Enumeration
        // (section 2.1.1.18) to be a metafile stored in memory.

        // HeaderSize: 0x0009 specifies the number of WORDs in this record, which is equivalent
        // to 18 (0x0012) bytes.

        // Version: 0x0300 specifies the metafile version from the MetafileVersion Enumeration
        // (section 2.1.1.19) to be a WMF metafile that supports DIBs.

        // Size: 0x00000036 specifies the number of WORDs in the entire metafile, which is
        // equivalent to 108 (0x0000006C) bytes.

        // NumberOfObjects: 0x0002 specifies the number of graphics objects that are defined in the metafile.

        // MaxRecord: 0x0000000C specifies the size in WORDs of the largest record in the
        // metafile, which is equivalent to 24 (0x00000018) bytes.

        // NumberOfMembers: 0x0000 is not used.

        // Note Based on the value of the NumberOfObjects field, a WMF Object Table (section 3.1.4.1)
        // can be created that is large enough for 2 objects.

        // 18 bytes
        stream.out_uint16_le(MEMORYMETAFILE);
        stream.out_uint16_le(9);
        stream.out_uint16_le(METAVERSION300);
        stream.out_uint32_le((data_length + METAFILE_WORDS_HEADER_SIZE)/2);
        stream.out_uint16_le(0);
        stream.out_uint32_le(largeRecordWordsSize);
        stream.out_uint16_le(0);


        // 2.3 WMF Records

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                           RecordSize                          |
        // +-------------------------------+-------------------------------+
        // |        RecordFunction         |           rdParam             |
        // +-------------------------------+-------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // RecordSize (4 bytes): A 32-bit unsigned integer that defines the number of 16-bit WORDs
        // in the record.

        // RecordFunction (2 bytes): A 16-bit unsigned integer that defines the type of this record.
        // The low-order byte MUST match the low-order byte of one of the values in the RecordType Enumeration.

        // rdParam (variable): An optional place holder that is provided for record-specific fields.

        //      META_SETMAPMODE (8 bytes)
        stream.out_uint32_le(4);
        stream.out_uint16_le(META_SETMAPMODE);
        stream.out_uint16_le(MM_ANISOTROPIC);


        //      META_SETWINDOWEXT (10 bytes)
        stream.out_uint32_le(5);
        stream.out_uint16_le(META_SETWINDOWEXT);
        stream.out_uint16_le( - height);
        stream.out_uint16_le(width);


        //      META_SETWINDOWORG (10 bytes)
        stream.out_uint32_le(5);
        stream.out_uint16_le(META_SETWINDOWORG);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0);


        // 2.3.1.3.1 META_DIBSTRETCHBLT With Bitmap

        // This section specifies the structure of the META_DIBSTRETCHBLT record when it contains an
        // embedded device-independent bitmap (DIB).

        // Fields not specified in this section are specified in the preceding META_DIBSTRETCHBLT section.

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                           RecordSize                          |
        // +-------------------------------+-------------------------------+
        // |        RecordFunction         |           rdParam             |
        // +-------------------------------+-------------------------------+
        // |             ...               |          SrcHeight            |
        // +-------------------------------+-------------------------------+
        // |           SrcWidth            |             YSrc              |
        // +-------------------------------+-------------------------------+
        // |             XSrc              |         DestHeight            |
        // +-------------------------------+-------------------------------+
        // |          DestWidth            |             YDest             |
        // +-------------------------------+-------------------------------+
        // |            XDest              |      Target (variable)        |
        // +-------------------------------+-------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // RecordFunction (2 bytes): A 16-bit unsigned integer that defines this WMF record type.
        // The low-order byte MUST match the low-order byte of the RecordType enumeration (section 2.1.1.1)
        // value META_DIBSTRETCHBLT. The high-order byte MUST contain a value equal to the number of 16-bit
        // WORDs in the record minus the number of WORDs in the RecordSize and Target fields. That is:

        //      RecordSize - (2 + (sizeof(Target)/2))

        // Target (variable): A variable-sized DeviceIndependentBitmap Object (section 2.2.2.9) that defines
        // image content. This object MUST be specified, even if the raster operation does not require a source.

        // 26 bytes
        stream.out_uint32_le(largeRecordWordsSize);
        stream.out_uint16_le(META_DIBSTRETCHBLT);
        stream.out_uint32_le(0x00CC0020); // SRCCOPY
        stream.out_uint16_le(height);
        stream.out_uint16_le(width);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0);
        stream.out_uint16_le(- height);
        stream.out_uint16_le(width);
        stream.out_uint16_le(0);
        stream.out_uint16_le(0);


        // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                     DIBHeaderInfo (variable)                  |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                        Colors (variable)                      |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                    BitmapBuffer (variable)                    |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // DIBHeaderInfo (variable): Either a BitmapCoreHeader Object (section 2.2.2.2) or a BitmapInfoHeader
        // Object (section 2.2.2.3) that specifies information about the image.

        // The first 32 bits of this field is the HeaderSize value. If it is 0x0000000C, then this is a
        // BitmapCoreHeader; otherwise, this is a BitmapInfoHeader.

        // Colors (variable): An optional array of either RGBQuad Objects (section 2.2.2.20) or 16-bit unsigned
        // integers that define a color table.

        // The size and contents of this field SHOULD be determined from the metafile record or object that
        // contains this DeviceIndependentBitmap and from information in the DIBHeaderInfo field. See ColorUsage
        // Enumeration (section 2.1.1.6) and BitCount Enumeration (section 2.1.1.3) for additional details.


        // BitmapBuffer (variable): A buffer containing the image, which is not required to be contiguous with the
        // DIB header, unless this is a packed bitmap.

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                    UndefinedSpace (variable)                  |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+
        // |                        aData (variable)                       |
        // +---------------------------------------------------------------+
        // |                              ...                              |
        // +---------------------------------------------------------------+

        // UndefinedSpace (variable): An optional field that MUST be ignored. If this DIB is a packed bitmap,
        // this field MUST NOT be present.

        // aData (variable): An array of bytes that define the image.

        //      The size and format of this data is determined by information in the DIBHeaderInfo field.
        // If it is a BitmapCoreHeader, the size in bytes MUST be calculated as follows:

        //              (((Width * Planes * BitCount + 31) & ~31) / 8) * abs(Height)

        //      This formula SHOULD also be used to calculate the size of aData when DIBHeaderInfo is a BitmapInfoHeader
        // Object, using values from that object, but only if its Compression value is BI_RGB, BI_BITFIELDS, or BI_CMYK.

        //      Otherwise, the size of aData MUST be the BitmapInfoHeader Object value ImageSize.


        // 2.2.2.3 BitmapInfoHeader Object

        // The BitmapInfoHeader Object contains information about the dimensions and color format of a device-independent
        // bitmap (DIB).

        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
        // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                           HeaderSize                          |
        // +---------------------------------------------------------------+
        // |                             Width                             |
        // +---------------------------------------------------------------+
        // |                            Height                             |
        // +-------------------------------+-------------------------------+
        // |          Planes               |           BitCount            |
        // +-------------------------------+-------------------------------+
        // |                         Compression                           |
        // +---------------------------------------------------------------+
        // |                          ImageSize                            |
        // +---------------------------------------------------------------+
        // |                         XPelsPerMeter                         |
        // +---------------------------------------------------------------+
        // |                         YPelsPerMeter                         |
        // +---------------------------------------------------------------+
        // |                          ColorUsed                            |
        // +---------------------------------------------------------------+
        // |                        ColorImportant                         |
        // +---------------------------------------------------------------+

        // HeaderSize (4 bytes): A 32-bit unsigned integer that defines the size of this object, in bytes.

        // Width (4 bytes): A 32-bit signed integer that defines the width of the DIB, in pixels. This value
        // MUST be positive.

        //          This field SHOULD specify the width of the decompressed image file, if the
        //          Compression value specifies JPEG or PNG format.<44>

        // Height (4 bytes): A 32-bit signed integer that defines the height of the DIB, in pixels. This value MUST NOT
        // be zero.

        // Planes (2 bytes): A 16-bit unsigned integer that defines the number of planes for the target device.
        // This value MUST be 0x0001.

        // BitCount (2 bytes): A 16-bit unsigned integer that defines the number of bits that define each pixel
        // and the maximum number of colors in the DIB. This value MUST be in the BitCount Enumeration (section 2.1.1.3).

        // Compression (4 bytes): A 32-bit unsigned integer that defines the compression mode of the DIB. This value
        // MUST be in the Compression Enumeration (section 2.1.1.7).

        //          This value MUST NOT specify a compressed format if the DIB is a top-down bitmap, as indicated by
        //          the Height value.

        // ImageSize (4 bytes): A 32-bit unsigned integer that defines the size, in bytes, of the image.

        //          If the Compression value is BI_RGB, this value SHOULD be zero and MUST be ignored.<45>

        //          If the Compression value is BI_JPEG or BI_PNG, this value MUST specify the size of the JPEG or PNG
        //          image buffer, respectively.

        // XPelsPerMeter (4 bytes): A 32-bit signed integer that defines the horizontal resolution, in pixels-per-meter,
        // of the target device for the DIB.

        // YPelsPerMeter (4 bytes): A 32-bit signed integer that defines the vertical resolution, in pixels-per-meter,
        // of the target device for the DIB.

        // ColorUsed (4 bytes): A 32-bit unsigned integer that specifies the number of indexes in the color table
        // used by the DIB, as follows:

        //           If this value is zero, the DIB uses the maximum number of colors that correspond to the BitCount value.

        //           If this value is nonzero and the BitCount value is less than 16, this value specifies the number
        //           of colors used by the DIB.

        //           If this value is nonzero and the BitCount value is 16 or greater, this value specifies the size
        //           of the color table used to optimize performance of the system palette.

        //           Note If this value is nonzero and greater than the maximum possible size of the color table
        //          based on the BitCount value, the maximum color table size SHOULD be assumed.

        // ColorImportant (4 bytes): A 32-bit unsigned integer that defines the number of color indexes that are
        // required for displaying the DIB. If this value is zero, all color indexes are required.

        //           A DIB is specified by a DeviceIndependentBitmap Object (section 2.2.2.9).

        //           When the array of pixels in the DIB immediately follows the BitmapInfoHeader, the DIB is a packed
        //           bitmap. In a packed bitmap, the ColorUsed value MUST be either 0x00000000 or the actual size
        //           of the color table.

        // 40 bytes
        stream.out_uint32_le(40);
        stream.out_uint32_le(width);
        stream.out_uint32_le( - height);
        stream.out_uint16_le(1);
        stream.out_uint16_le(depth);
        stream.out_uint32_le(0);  // BI_RGB
        stream.out_uint32_le(data_length);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0);
        stream.out_uint32_le(0);
    }

    using CliprdrHeader::recv;
}; // struct FormatDataResponsePDU


class MetaFilePicDescriptor
{

public:
    int recordSize;
    int type;
    int height;
    int width;
    int bpp;
    int imageSize;


    MetaFilePicDescriptor() :
      recordSize(0)
    , type(0)
    , height(0)
    , width(0)
    , bpp(0)
    , imageSize(0)
    {}

    void receive(InStream & chunk) {
        // 2.2.5.2.1 Packed Metafile Payload (cliboard.hpp)
        chunk.in_skip_bytes(12);

        // 3.2.1 META_HEADER Example
        chunk.in_skip_bytes(18);

        bool notEOF(true);
        while(notEOF) {

            // 2.3 WMF Records
            this->recordSize = chunk.in_uint32_le();
            this->type = chunk.in_uint16_le();

            switch (type) {

                case META_SETWINDOWEXT:
                    chunk.in_skip_bytes(recordSize*2 - 6);
                break;

                case META_SETWINDOWORG:
                    chunk.in_skip_bytes(recordSize*2 - 6);
                break;

                case META_SETMAPMODE:
                    chunk.in_skip_bytes(recordSize*2 - 6);
                break;

                case META_DIBSTRETCHBLT:
                {
                    notEOF = false;

                    // 2.3.1.3.1 META_DIBSTRETCHBLT With Bitmap
                    chunk.in_skip_bytes(4);

                    this->height = chunk.in_uint16_le();
                    this->width  = chunk.in_uint16_le();
                    chunk.in_skip_bytes(12);

                    // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object

                    // 2.2.2.3 BitmapInfoHeader Object
                    chunk.in_skip_bytes(14);

                    this->bpp     = chunk.in_uint16_le();
                    chunk.in_skip_bytes(4);

                    this->imageSize   = chunk.in_uint32_le();
                    chunk.in_skip_bytes(8);

                    int skip(0);
                    if (chunk.in_uint32_le() == 0) { // if colorUsed == 0
                        skip = 0;
                    }
                    chunk.in_skip_bytes(4);

                        // Colors (variable)
                    chunk.in_skip_bytes(skip);

                        // BitmapBuffer (variable)
                    chunk.in_skip_bytes(0);
                }
                break;

                default:
                break;
            }
        }
    }

};  // class MetaFilePicDescriptor


// 2.2.4.1 Lock Clipboard Data PDU (CLIPRDR_LOCK_CLIPDATA)

// The Lock Clipboard Data PDU can be sent at any point in time after the clipboard capabilities and temporary directory have been exchanged in the Clipboard Initialization Sequence (section 1.3.2.1) by a Local Clipboard Owner (section 1.3.2.2.1). The purpose of this PDU is to request that the Shared Clipboard Owner (section 1.3.2.2.1) retain all File Stream (section 1.3.1.1.5) data on the clipboard until the Unlock Clipboard Data PDU (section 2.2.4.2) is received. This ensures that File Stream data can be requested by the Local Owner in a subsequent File Contents Paste Sequence (section 1.3.2.2.3) by using the File Contents Request PDU (section 2.2.5.3) even when the Shared Owner clipboard has changed and the File Stream data is no longer available.

 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
 // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // |                          clipHeader                           |
 // +---------------------------------------------------------------+
 // |                              ...                              |
 // +---------------------------------------------------------------+
 // |                          clipDataId                           |
 // +-------------------------------+-------------------------------+

// clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_LOCK_CLIPDATA (0x000A), while the msgFlags field MUST be set to 0x0000.

// clipDataId (4 bytes): An unsigned, 32-bit integer that is used to tag File Stream data on the Shared Owner clipboard so that it can be requested in a subsequent File Contents Request PDU (section 2.2.5.3).

struct LockClipboardDataPDU : public CliprdrHeader {
    explicit LockClipboardDataPDU()
    : CliprdrHeader(CB_LOCK_CLIPDATA, 0, 4)
    {}

    void emit(OutStream & stream, uint32_t streamDataID) {
        CliprdrHeader::emit(stream);
        stream.out_uint32_le(streamDataID);
    }
};



 // 2.2.4.2 Unlock Clipboard Data PDU (CLIPRDR_UNLOCK_CLIPDATA)

 // The Unlock Clipboard Data PDU can be sent at any point in time after the Clipboard Initialization Sequence (section 1.3.2.1) by a Local Clipboard Owner (section 1.3.2.2.1). The purpose of this PDU is to notify the Shared Clipboard Owner (section 1.3.2.2.1) that File Stream data that was locked in response to the Lock Clipboard Data PDU (section 2.2.4.1) can be released.

 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
 // |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
 // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 // |                          clipHeader                           |
 // +---------------------------------------------------------------+
 // |                              ...                              |
 // +---------------------------------------------------------------+
 // |                          clipDataId                           |
 // +-------------------------------+-------------------------------+

 // clipHeader (8 bytes): A Clipboard PDU Header. The msgType field of the Clipboard PDU Header MUST be set to CB_UNLOCK_CLIPDATA (0x000B), while the msgFlags field MUST be set to 0x0000.

 // clipDataId (4 bytes): An unsigned, 32-bit integer that identifies the File Stream data that was locked by the Lock Clipboard Data PDU (section 2.2.4.1) and can now be released.

struct UnlockClipboardDataPDU : public CliprdrHeader {
    explicit UnlockClipboardDataPDU()
    : CliprdrHeader(CB_UNLOCK_CLIPDATA, 0, 4)
    {}

    void emit(OutStream & stream, uint32_t streamDataID) {
        CliprdrHeader::emit(stream);
        stream.out_uint32_le(streamDataID);
    }
};


}   // namespace RDPECLIP

