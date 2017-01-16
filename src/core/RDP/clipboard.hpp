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
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/


#pragma once

#include <iostream>
#include <cinttypes>

#include "utils/stream.hpp"
#include "core/error.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "utils/sugar/cast.hpp"
#include "core/WMF/MetaFileFormat.hpp"


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

enum {
      CB_RESPONSE_OK   = 0x0001
    , CB_RESPONSE_FAIL = 0x0002
    , CB_ASCII_NAMES   = 0x0004
};

inline static const char * get_msgFlag_name(uint16_t msgFlag) {
    switch (msgFlag) {
        case CB_RESPONSE_OK:   return "CB_RESPONSE_OK";
        case CB_RESPONSE_FAIL: return "CB_RESPONSE_FAIL";
    }

    return "<unknown>";
}

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


struct RecvPredictor {
    uint16_t msgType;

    explicit RecvPredictor(const InStream & stream) {
        InStream s = stream.clone();
        const unsigned expected = 2;    /* msgType(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::RecvPredictor truncated msgType, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }
        this->msgType = s.in_uint16_le();
    }   // RecvFactory(InStream & stream)
};


struct CliprdrHeader {
    uint16_t msgType()  const { return this->msgType_; }
    uint16_t msgFlags() const { return this->msgFlags_; }
    uint32_t dataLen()  const { return this->dataLen_; }

public:
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

    void recv(InStream & stream) {
        const unsigned expected = 6;    /* msgFlags_(2) + dataLen_(4) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::recv truncated data, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->msgType_  = stream.in_uint16_le();
        this->msgFlags_ = stream.in_uint16_le();
        this->dataLen_  = stream.in_uint32_le();
    }

    void log() {
        LOG(LOG_INFO, "     CliprdrHeader:");
        LOG(LOG_INFO, "          * MsgType  = 0x%x (%s)", this->msgType_, get_msgType_name(this->msgType_));
        LOG(LOG_INFO, "          * MsgFlags = 0x%x (%s)", this->msgFlags_, get_msgFlag_name(this->msgFlags_));
        LOG(LOG_INFO, "          * DataLen  = %d Byte(s)", this->dataLen_);
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

struct ClipboardCapabilitiesPDU
{
    CliprdrHeader header;

    uint16_t cCapabilitiesSets = 0;

public:
    ClipboardCapabilitiesPDU() = default;

    ClipboardCapabilitiesPDU(uint16_t cCapabilitiesSets, uint32_t length_capabilities)
        : header(CB_CLIP_CAPS,
                      0,
                      4 +   // cCapabilitiesSets(2) + pad1(2)
                          length_capabilities
                     )
        , cCapabilitiesSets(cCapabilitiesSets)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);

        stream.out_uint16_le(cCapabilitiesSets);
        stream.out_clear_bytes(2);  // pad1(2)
    }   // void emit(OutStream & stream)

    void recv(InStream & stream) {
        this->header.recv(stream);

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

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Clipboard Capabilities PDU:");
        LOG(LOG_INFO, "          * cCapabilitiesSets = %d (2 bytes)", this->cCapabilitiesSets);
        LOG(LOG_INFO, "          * Padding - (2 byte) NOT USED");
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

    void log() {
        LOG(LOG_INFO, "     General Capability Set:");
        LOG(LOG_INFO, "          * capabilitySetType = 0x%04x (2 bytes): CB_CAPSTYPE_GENERAL", this->capabilitySetType);
        LOG(LOG_INFO, "          * lengthCapability  = 0x%04x (2 bytes)", this->lengthCapability);
        LOG(LOG_INFO, "          * version           = 0x%08x (4 bytes)", this->version);
        LOG(LOG_INFO, "          * generalFlags      = 0x%08x (4 bytes)", this->generalFlags_);
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

struct ServerMonitorReadyPDU
{
    CliprdrHeader header;

    ServerMonitorReadyPDU() : header(CB_MONITOR_READY, 0, 0) {
    }   // ServerMonitorReadyPDU(bool response_ok)

    void emit(OutStream & stream)
    {
        this->header.emit(stream);
    }

    void recv(InStream & stream)
    {
        this->header.recv(stream);
    }

    void log() {
        this->header.log();
    }

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

struct FormatListPDU
{
    CliprdrHeader header;
    bool contains_data_in_text_format        = false;
    bool contains_data_in_unicodetext_format = false;

    uint32_t const    * formatListDataIDs;
    std::string const * formatListDataName;
    std::size_t         formatListDataSize;


    FormatListPDU( uint32_t const * formatListDataIDs
                 , std::string const * formatListDataName
                 , std::size_t formatListDataSize)
        : header(CB_FORMAT_LIST, 0, 0)
        , formatListDataIDs(formatListDataIDs)
        , formatListDataName(formatListDataName)
        , formatListDataSize(formatListDataSize)
        {}

    FormatListPDU()
        : header(CB_FORMAT_LIST, 0, 0)
        , contains_data_in_text_format(false)
        , contains_data_in_unicodetext_format(false)
        {}

    void emit(OutStream & stream) {
        this->header.dataLen_ = 36;    /* formatId(4) + formatName(32) */
        this->header.emit(stream);

        // 1 CLIPRDR_SHORT_FORMAT_NAMES structures.
        stream.out_uint32_le(CF_TEXT);
        stream.out_clear_bytes(SHORT_NAME_MAX_SIZE); // formatName(32)
    }

    void log() {
        this->header.log();

    }

    void emit_ex(OutStream & stream, bool unicodetext) {
        this->header.dataLen_ = 36;    /* formatId(4) + formatName(32) */
        this->header.emit(stream);

        // 1 CLIPRDR_SHORT_FORMAT_NAMES structures.
        stream.out_uint32_le(unicodetext ? CF_UNICODETEXT : CF_TEXT);
        stream.out_clear_bytes(32); // formatName(32)
    }

    void emit_long(OutStream & stream, bool unicodetext) {
        this->header.dataLen_ = 6; /* formatId(4) + formatName(2) */
        this->header.emit(stream);

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
        this->header.dataLen_ = 0;
        this->header.emit(stream);
    }

    void recv(InStream & stream) {
//        LOG(LOG_INFO, "RDPECLIP::FormatListPDU::recv");
        this->header.recv(stream);

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
        const uint32_t short_format_name_structure_size = 36; /* formatId(4) + formatName(32) */

        // Parse PDU to find if clipboard data is available in a TEXT format.
        for (uint32_t i = 0; i < (this->header.dataLen() / short_format_name_structure_size); i++) {
            if (!stream.in_check_rem(short_format_name_structure_size)) {
                LOG( LOG_INFO
                   , "RDPECLIP::FormatListPDU truncated CLIPRDR_SHORT_FORMAT_NAME structure, need=%u remains=%zu"
                   , short_format_name_structure_size, stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            uint32_t formatId = stream.in_uint32_le();
            //LOG(LOG_INFO, "RDPECLIP::FormatListPDU formatId=%u", formatId);

            if (formatId == CF_TEXT) {
                this->contains_data_in_text_format = true;
            }
            else if (formatId == CF_UNICODETEXT) {
                this->contains_data_in_unicodetext_format = true;
            }

            stream.in_skip_bytes(32);   // formatName(32)
        }
    }   // void recv(InStream & stream)

    void recv_long(InStream & stream) {
        // 2.2.3.1.2 Long Format Names (CLIPRDR_LONG_FORMAT_NAMES)
        // =======================================================

        // The CLIPRDR_LONG_FORMAT_NAMES structure holds a collection of
        // CLIPRDR_LONG_FORMAT_NAME structures.
        // longFormatNames (variable): An array of CLIPRDR_LONG_FORMAT_NAME structures.

        // 2.2.3.1.2.1 Long Format Name (CLIPRDR_LONG_FORMAT_NAME)
        // =======================================================

        // The CLIPRDR_LONG_FORMAT_NAME structure holds a Clipboard Format ID and a Clipboard Format name pair.

        // formatId (4 bytes): An unsigned, 32-bit integer that specifies the Clipboard Format ID.

        // wszFormatName (variable): A variable length null-terminated Unicode
        // string name that contains the Clipboard Format name. Not all
        // Clipboard Formats have a name; in such cases, the formatName field
        // MUST consist of a single Unicode null character.

// length: 0x24, 0x00, 0x00, 0x00,
// flags:  0x13, 0x00, 0x00, 0x00,

        this->header.recv(stream);
// msgType:  2 = CB_FORMAT_LIST | 0x02, 0x00,
// msgFlags: 0                  | 0x00, 0x00,
// datalen: 24 (after headers)| 0x18, 0x00, 0x00, 0x00,

        const size_t max_length_of_format_name = 256;
// FormatId[]: CF_UNICODETEXT 0x0d, 0x00, 0x00, 0x00,
// 0x00, 0x00,
// FormatId[]: CF_LOCALE 0x10, 0x00, 0x00, 0x00,
// 0x00, 0x00,
// FormatId[]: CF_TEXT 0x01, 0x00, 0x00, 0x00,
// 0x00, 0x00,
// FormatId[]: CF_OEMTEXT 0x07, 0x00, 0x00, 0x00,
// 0x00, 0x00,

// Padding ? : 0x00, 0x00, 0x00, 0x00,

        InStream fns(stream.get_current(), this->header.dataLen());
        while (fns.in_remain()) {
            if (fns.in_remain() < 6){
                // Minimal Possible remaining data: FormatId + 0 + 0
                // TODO: this should probably raise an error, as it is
                // an incorrent clipboard format is we don't have exactly
                // the expected datas
                stream.in_skip_bytes(fns.in_remain());
                return;
            }
            const uint32_t formatId = fns.in_uint32_le();
            switch (formatId){
            case CF_UNICODETEXT:
                this->contains_data_in_unicodetext_format = true;
            break;
            case CF_TEXT:
                this->contains_data_in_text_format = true;
            break;
            default:
             ;   // other formats unsupported
            }
            uint16_t buffer[max_length_of_format_name];
            (void)fns.in_utf16_sz(buffer,
                    std::min(fns.in_remain(),max_length_of_format_name-2)/2);
        }
    }   // void recv_long(InStream & stream)
};  // struct FormatListPDU

struct FormatListPDU_LongName : public FormatListPDU {

    explicit FormatListPDU_LongName( uint32_t const * formatListDataIDs
                                   , std::string const * formatListDataName
                                   , std::size_t formatListDataSize)
        : FormatListPDU(formatListDataIDs, formatListDataName, formatListDataSize)
    {}

    void emit(OutStream & stream) {
        if (this->formatListDataSize > FORMAT_LIST_MAX_SIZE) {
            this->formatListDataSize = FORMAT_LIST_MAX_SIZE;
        }

        for (std::size_t i = 0; i < this->formatListDataSize; i++) {
            /* formatId(4) + formatName(variable) */
            this->header.dataLen_ = this->header.dataLen() + formatListDataName[i].size() + 4;
        }
        REDASSERT(this->header.dataLen() <= 1024);
        this->header.emit(stream);

        for (std::size_t i = 0; i < this->formatListDataSize; i++) {
            stream.out_uint32_le(this->formatListDataIDs[i]);
            std::string const & currentStr = this->formatListDataName[i];
            stream.out_copy_bytes(currentStr.data(), currentStr.size());
        }
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Format List PDU Long Name:");
        LOG(LOG_INFO, "          * formatListDataSize = %d (4 bytes)", int(this->formatListDataSize));
        for (size_t i = 0; i < this->formatListDataSize; i++) {
        LOG(LOG_INFO, "          * formatListDataName = %s", this->formatListDataName[i]);
        LOG(LOG_INFO, "          * formatListDataIDs  = 0x%08x (4 bytes)", this->formatListDataIDs[i]);
        }
    }

};

struct FormatListPDU_ShortName : public FormatListPDU {

    explicit FormatListPDU_ShortName( uint32_t const * formatListDataIDs
                                    , std::string const * formatListDataName
                                    , std::size_t formatListDataSize)
        : FormatListPDU(formatListDataIDs, formatListDataName, formatListDataSize)
    {}

    void emit(OutStream & stream) {
        if (this->formatListDataSize > FORMAT_LIST_MAX_SIZE) {
            this->formatListDataSize = FORMAT_LIST_MAX_SIZE;
        }

        this->header.dataLen_ = this->formatListDataSize * (4 + SHORT_NAME_MAX_SIZE);    /* formatId(4) + formatName(32) */
        this->header.emit(stream);

        for (std::size_t i = 0; i < this->formatListDataSize; i++) {
            stream.out_uint32_le(this->formatListDataIDs[i]);
            std::string const & currentStr = this->formatListDataName[i];
            REDASSERT(currentStr.size() <= SHORT_NAME_MAX_SIZE);
            stream.out_copy_bytes(currentStr.data(), currentStr.size());
            stream.out_clear_bytes(SHORT_NAME_MAX_SIZE - currentStr.size()); // formatName(32)
        }
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Format List PDU Short Name:");
        LOG(LOG_INFO, "          * formatListDataSize = %d (4 bytes)", int(this->formatListDataSize));
        for (size_t i = 0; i < this->formatListDataSize; i++) {
        LOG(LOG_INFO, "          * formatListDataName = %s (8 bytes)", this->formatListDataName[i]);
        LOG(LOG_INFO, "          * formatListDataIDs  = 0x%08x (4 bytes)", this->formatListDataIDs[i]);
        }
    }
};



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

struct FormatListResponsePDU
{
    CliprdrHeader header;
    explicit FormatListResponsePDU(bool response_ok)
        : header( CB_FORMAT_LIST_RESPONSE
                       , (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL)
                       , 0) {
    }   // FormatListResponsePDU(bool response_ok)

    void emit(OutStream & stream) {
        this->header.emit(stream);
    }

    void recv(InStream & stream)
    {
        this->header.recv(stream);
    }

    void log() {
        this->header.log();

    }

};  // struct FormatListResponsePDU

// [MS-RDPECLIP] 2.2.5.1 Format Data Request PDU (CLIPRDR_FORMAT_DATA_REQUEST)
// ===========================================================================

// The Format Data Request PDU is sent by the recipient of the Format List
//  PDU. It is used to request the data for one of the formats that was listed
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

struct FormatDataRequestPDU
{
    CliprdrHeader header;
    uint32_t requestedFormatId;

    FormatDataRequestPDU()
            : header(CB_FORMAT_DATA_REQUEST, 0, 4)
            , requestedFormatId(0) {
    }   // FormatDataRequestPDU()

    explicit FormatDataRequestPDU(uint32_t requestedFormatId)
            : header(CB_FORMAT_DATA_REQUEST, 0, 4)
            , requestedFormatId(requestedFormatId) {
    }   // FormatDataRequestPDU(uint32_t requestedFormatId)

    void emit(OutStream & stream) {
        this->header.emit(stream);

        stream.out_uint32_le(this->requestedFormatId);
    }   // void emit(OutStream & stream)

    void recv(InStream & stream) {
        this->header.recv(stream);
        const unsigned expected = 4;   // requestedFormatId
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_INFO
               , "FormatDataRequestPDU::recv truncated requestedFormatId, need=%u remains=%zu"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->requestedFormatId = stream.in_uint32_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Format Data Request PDU:");
        LOG(LOG_INFO, "          * requestedFormatId = 0x%08x (4 bytes)", this->requestedFormatId);
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

struct FileContentsRequestPDU     // Resquest RANGE
{
    CliprdrHeader header;
    int streamID;
    int flag;
    int lindex;
    uint64_t sizeRequested;

    explicit FileContentsRequestPDU( int streamID
                                   , int flag
                                   , int lindex
                                   , uint64_t sizeRequested)
    : header( CB_FILECONTENTS_REQUEST, CB_RESPONSE_OK, 24)
    , streamID(streamID)
    , flag(flag)
    , lindex(lindex)
    , sizeRequested(sizeRequested)
    {}

    explicit FileContentsRequestPDU(bool response_ok = false)
    : header( CB_FILECONTENTS_REQUEST, (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL), 24)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(this->streamID);
        stream.out_uint32_le(this->lindex);
        stream.out_uint32_le(this->flag);
        stream.out_uint32_le(this->sizeRequested >> 32);
        stream.out_uint32_le(this->sizeRequested);
        if (flag & FILECONTENTS_SIZE) {
            stream.out_uint32_le(FILECONTENTS_SIZE_CB_REQUESTED);
        } else {
            stream.out_uint32_le(this->sizeRequested);
        }
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        this->streamID = stream.in_uint32_le();
        this->lindex = stream.in_uint32_le();
        this->flag = stream.in_uint32_le();
        this->sizeRequested = stream.in_uint32_le();
        this->sizeRequested = this->sizeRequested << 32;
        this->sizeRequested += stream.in_uint32_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     File Contents Request PDU:");
        LOG(LOG_INFO, "          * streamID      = %08x (4 bytes)", this->streamID);
        LOG(LOG_INFO, "          * flag          = %08x (4 bytes)", this->flag);
        LOG(LOG_INFO, "          * lindex        = %08x (4 bytes)", this->lindex);
        LOG(LOG_INFO, "          * sizeRequested = %" PRIu64 " (8 bytes)", this->sizeRequested);
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

struct FileContentsResponse
{
    CliprdrHeader header;

    uint32_t streamID;
    uint64_t size;


    explicit FileContentsResponse(const uint32_t streamID, const uint64_t size, uint32_t data_size)
    : header( CB_FILECONTENTS_RESPONSE, CB_RESPONSE_OK, data_size)
    , streamID(streamID)
    , size(size)
    {}

    explicit FileContentsResponse(bool response_ok = false)
    : header( CB_FILECONTENTS_RESPONSE, (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL), 4)
    , streamID(0)
    , size(0)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
    }

};

struct FileContentsResponse_Size : FileContentsResponse {

    explicit FileContentsResponse_Size()
    : FileContentsResponse()
    {}

    explicit FileContentsResponse_Size(const uint32_t streamID, const uint64_t size)
    : FileContentsResponse(streamID, size, 16)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(this->streamID);
        stream.out_uint64_le(this->size);
        stream.out_uint32_le(0);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        this->streamID = stream.in_uint32_le();
        this->size = stream.in_uint64_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     File Contents Response Size:");
        LOG(LOG_INFO, "          * size     = %" PRIu64 " (8 bytes)", this->size);
        LOG(LOG_INFO, "          * streamID = 0X%08x (4 bytes)", this->streamID);
        LOG(LOG_INFO, "          * Padding - (4 byte) NOT USED");
    }


};

struct FileContentsResponse_Range : FileContentsResponse {

    explicit FileContentsResponse_Range()
    : FileContentsResponse()
    {}

    explicit FileContentsResponse_Range(const uint32_t streamID, const uint64_t size)
    : FileContentsResponse(streamID, size, size+4)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(this->streamID);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        this->streamID = stream.in_uint32_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     File Contents Response Range:");
        LOG(LOG_INFO, "          * streamID = 0X%08x (4 bytes)", this->streamID);
    }

};


struct PacketFileList {
    uint32_t cItems;
    /*variable fileDescriptorArray*/
};

enum : uint64_t {
    TIME64_FILE_LIST = 0x01d1e2a0379fb504
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
// |                      reserved1 (32 bytes)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         fileAttributes                        |
// +---------------------------------------------------------------+
// |                      reserved2 (16 bytes)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         lastWriteTime                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         fileSizeHigh                          |
// +---------------------------------------------------------------+
// |                         fileSizeLow                           |
// +---------------------------------------------------------------+
// |                     fileName (520 bytes)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
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
    uint32_t flags = 0;
    uint32_t fileAttributes = 0;
    uint64_t lastWriteTime = 0;
    uint32_t fileSizeHigh = 0;
    uint32_t fileSizeLow = 0;
    std::string file_name;

    FileDescriptor() = default;

    explicit FileDescriptor(std::string name, const uint64_t size, const uint32_t attribute)
      : flags(FD_SHOWPROGRESSUI |FD_FILESIZE | FD_WRITESTIME | FD_ATTRIBUTES)
      , fileAttributes(attribute)
      , lastWriteTime(TIME64_FILE_LIST)
      , fileSizeHigh(size >> 32)
      , fileSizeLow(size)
      , file_name(std::move(name))
    {}

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



enum : uint32_t {
    SRCCOPY = 0x00CC0020
};

enum : uint16_t {
    PLANES_NUMBER = 0x00001
};

enum : int {
      METAFILE_HEADERS_SIZE          = 130
    , META_DIBSTRETCHBLT_HEADER_SIZE = 66
    , METAFILE_WORDS_HEADER_SIZE     = (METAFILE_HEADERS_SIZE/2) -6
    , META_HEADER_SIZE               = 9

    , META_SETMAPMODE_WORDS_SIZE   = 4
    , META_SETWINDOWEXT_WORDS_SIZE = 5
    , META_SETWINDOWORG_WORDS_SIZE = 5
    , BITMAPINFO_HEADER_SIZE       = 40

};

struct FormatDataResponsePDU
{
    CliprdrHeader header;


    FormatDataResponsePDU()
        : header( CB_FORMAT_DATA_RESPONSE
                , CB_RESPONSE_FAIL
                , 0)
    {
    }

    explicit FormatDataResponsePDU(std::size_t data_len)
        : header( CB_FORMAT_DATA_RESPONSE
                , CB_RESPONSE_OK
                , data_len)
    {
    }

    explicit FormatDataResponsePDU(bool response_ok)
        : header( CB_FORMAT_DATA_RESPONSE
                , (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL)
                , 0)
    {
    }

    void emit(OutStream & stream, const uint8_t * data, size_t data_length) const {
        stream.out_uint16_le(this->header.msgType());
        stream.out_uint16_le(this->header.msgFlags());

        if (this->header.msgFlags() == CB_RESPONSE_OK) {
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
        stream.out_uint16_le(this->header.msgType());
        stream.out_uint16_le(this->header.msgFlags());

        stream.out_uint32_le(                           // dataLen(4)
                (this->header.msgFlags() == CB_RESPONSE_OK) ?
                data_length :
                0
            );
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
    }

    void log() {
        this->header.log();

    }


}; // struct FormatDataResponsePDU


struct FormatDataResponsePDU_MetaFilePic : FormatDataResponsePDU {

    // 2.2.5.2.1 Packed Metafile Payload

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

    uint32_t mappingMode;
    uint32_t xExt;
    uint32_t yExt;

    MFF::MetaHeader metaHeader;
    MFF::MetaSetMapMod metaSetMapMod;
    MFF::MetaSetWindowExt metaSetWindowExt;
    MFF::MetaSetWindowOrg metaSetWindowOrg;
    MFF::DibStretchBLT dibStretchBLT;


    struct Ender {
        enum : uint32_t {
            SIZE = 6
          , FLAG = 0x03
        };

        void emit(uint8_t * chunk, const size_t data_len) {
            chunk[data_len + 1] = FLAG;
            chunk[data_len + 2] = 0x00;
            chunk[data_len + 3] = 0x00;
            chunk[data_len + 4] = 0x00;
            chunk[data_len + 5] = 0x00;
            chunk[data_len + 6] = 0x00;
        }
    };

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Packed Metafile Payload:");
        LOG(LOG_INFO, "          * mappingMode = 0x%08x (4 bytes)", this->mappingMode);
        LOG(LOG_INFO, "          * xExt        = %d (4 bytes)", int(this->xExt));
        LOG(LOG_INFO, "          * yExt        = %d (4 bytes)", int(this->yExt));
        this->metaHeader.log();
        this->metaSetMapMod.log();
        this->metaSetWindowExt.log();
        this->metaSetWindowOrg.log();
        this->dibStretchBLT.log();
    }

    explicit FormatDataResponsePDU_MetaFilePic()
      : FormatDataResponsePDU()
      , mappingMode(0)
      , xExt(0)
      , yExt(0)
      , metaHeader(0, 0, 0)
      , metaSetMapMod(0)
      , metaSetWindowExt(0, 0)
      , metaSetWindowOrg(0, 0)
      , dibStretchBLT(0, 0, 0, 0, SRCCOPY)
    {}

    explicit FormatDataResponsePDU_MetaFilePic( const std::size_t data_length
                                              , const uint16_t width
                                              , const uint16_t height
                                              , const uint16_t depth
                                              , const double ARBITRARY_SCALE)
      : FormatDataResponsePDU(data_length + METAFILE_HEADERS_SIZE)
      , mappingMode(MM_ANISOTROPIC)
      , xExt(int(double(width ) * ARBITRARY_SCALE))
      , yExt(int(double(height) * ARBITRARY_SCALE))
      , metaHeader(MFF::MEMORYMETAFILE, MFF::METAVERSION300, data_length)
      , metaSetMapMod(MM_ANISOTROPIC)
      , metaSetWindowExt(height, width)
      , metaSetWindowOrg(0, 0)
      , dibStretchBLT(data_length, height, width, depth, SRCCOPY)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);

        // 2.2.5.2.1 Packed Metafile Payload
        // 12 bytes
        stream.out_uint32_le(this->mappingMode);
        stream.out_uint32_le(this->xExt);
        stream.out_uint32_le(this->yExt);

        this->metaHeader.emit(stream);

        this->metaSetMapMod.emit(stream);

        this->metaSetWindowExt.emit(stream);

        this->metaSetWindowOrg.emit(stream);

        this->dibStretchBLT.emit(stream);
    }


    void recv(InStream & stream) {
        this->header.recv(stream);

        this->mappingMode = stream.in_uint32_le();
        this->xExt = stream.in_uint32_le();
        this->yExt = stream.in_uint32_le();

        this->metaHeader.recv(stream);

        InStream stream_header = stream.clone();
        bool notEOF(true);
        while(notEOF) {

            int recordSize = stream_header.in_uint32_le();
            int type = stream_header.in_uint16_le();

            switch (type) {

                case MFF::META_SETWINDOWEXT:
                    stream_header.in_skip_bytes(4);
                    this->metaSetWindowExt.recv(stream);
                    break;

                case MFF::META_SETWINDOWORG:
                    stream_header.in_skip_bytes(4);
                    this->metaSetWindowOrg.recv(stream);
                    break;

                case MFF::META_SETMAPMODE:
                    this->metaSetMapMod.recv(stream);
                    stream_header.in_skip_bytes(2);
                    REDASSERT(this->mappingMode == this->metaSetMapMod.mappingMode);
                    break;

                case MFF::META_DIBSTRETCHBLT:
                    {
                        notEOF = false;

                        this->dibStretchBLT.recv(stream);

                        REDASSERT(this->metaHeader.maxRecord == this->dibStretchBLT.recordSize);
                        REDASSERT(this->metaSetWindowExt.height == this->dibStretchBLT.destHeight);
                        REDASSERT(this->metaSetWindowExt.width == this->dibStretchBLT.destWidth);
                    }
                    break;

                default:
                    LOG(LOG_WARNING, "DEFAULT: unknow record type=%x size=%d octets", type, recordSize*2);
                    stream.in_skip_bytes((recordSize*2) - 6);
                    break;
            }
        }
    }


};

struct FormatDataResponsePDU_Text : FormatDataResponsePDU {

    struct Ender {
        enum : uint32_t {
            SIZE = 2
        };

        void emit (uint8_t * chunk, size_t data_len) {
            chunk[data_len + 1] = 0;
            chunk[data_len + 2] = 0;
        }
    };

    explicit FormatDataResponsePDU_Text()
      : FormatDataResponsePDU()
    {}

    explicit FormatDataResponsePDU_Text(std::size_t length)
      : FormatDataResponsePDU(length)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Format Data Response Text PDU:");
    }

};



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

struct FormatDataResponsePDU_FileList : FormatDataResponsePDU {

    int cItems;

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Format Data Response File List PDU:");
        LOG(LOG_INFO, "          * cItems       = %d (4 bytes)", this->cItems);
    }

    explicit FormatDataResponsePDU_FileList()
      : FormatDataResponsePDU()
      , cItems(0)
    {}

    explicit FormatDataResponsePDU_FileList(const std::size_t cItems)
      : FormatDataResponsePDU((FileDescriptor::size() * cItems) + 4)
      , cItems(cItems)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(this->cItems);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        this->cItems = stream.in_uint32_le();
    }
};

class MetaFilePicDescriptor
{

public:
    int recordSize;
    int height;
    int width;
    int bpp;
    int imageSize;


    MetaFilePicDescriptor() :
      recordSize(0)
    , height(0)
    , width(0)
    , bpp(0)
    , imageSize(0)
    {}

    void receive(InStream & chunk) {
        // Header
        chunk.in_skip_bytes(8);

        // 2.2.5.2.1 Packed Metafile Payload (cliboard.hpp)
        chunk.in_skip_bytes(12);

        // 3.2.1 META_HEADER Example
        chunk.in_skip_bytes(18);

        bool notEOF(true);
        while(notEOF) {

            // 2.3 WMF Records
            int size = chunk.in_uint32_le() * 2;
            int type = chunk.in_uint16_le();

            switch (type) {

                case MFF::META_SETWINDOWEXT:
                    chunk.in_skip_bytes(4);
                break;

                case MFF::META_SETWINDOWORG:
                    chunk.in_skip_bytes(4);
                break;

                case MFF::META_SETMAPMODE:
                    chunk.in_skip_bytes(2);
                break;

                case MFF::META_DIBSTRETCHBLT:
                {
                    notEOF = false;
                    this->recordSize = size;

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
                    //REDASSERT(this->imageSize == ((this->bpp/8) * this->height * this->width));
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

                default: LOG(LOG_INFO, "DEFAULT: unknow record type=%x size=%d octets", type, size);
                         chunk.in_skip_bytes(size - 6);

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

struct LockClipboardDataPDU
{
    CliprdrHeader header;

    uint32_t streamDataID;

    explicit LockClipboardDataPDU(): header(CB_UNLOCK_CLIPDATA, CB_RESPONSE_FAIL, 4)
    {}

    explicit LockClipboardDataPDU(uint32_t streamDataID)
    : header(CB_LOCK_CLIPDATA, 0, 4)
    , streamDataID(streamDataID)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(streamDataID);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        streamDataID = stream.in_uint32_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Lock Clipboard Data PDU:");
        LOG(LOG_INFO, "          * streamDataID = 0x%08x (4 bytes)", this->streamDataID);
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

struct UnlockClipboardDataPDU
{
    CliprdrHeader header;

    uint32_t streamDataID;

    explicit UnlockClipboardDataPDU(): header(CB_UNLOCK_CLIPDATA, CB_RESPONSE_FAIL, 4)
    {}

    explicit UnlockClipboardDataPDU(uint32_t streamDataID)
    : header(CB_UNLOCK_CLIPDATA, 0, 4)
    , streamDataID(streamDataID)
    {}

    void emit(OutStream & stream) {
        this->header.emit(stream);
        stream.out_uint32_le(streamDataID);
    }

    void recv(InStream & stream) {
        this->header.recv(stream);
        streamDataID = stream.in_uint32_le();
    }

    void log() {
        this->header.log();
        LOG(LOG_INFO, "     Unlock Clipboard Data PDU:");
        LOG(LOG_INFO, "          * streamDataID = 0x%08x (4 bytes)", this->streamDataID);
    }

};




}   // namespace RDPECLIP

