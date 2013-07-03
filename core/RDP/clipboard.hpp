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

#ifndef _REDEMPTION_CORE_RDP_CLIPBOARD_HPP_
#define _REDEMPTION_CORE_RDP_CLIPBOARD_HPP_

#include "stream.hpp"

namespace RDPECLIP {

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
//  the clipboard PDU that follows the dataLen field.

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

// dataLen (4 bytes): An unsigned, 32-bit integer that specifies the size, in
//  bytes, of the data which follows the Clipboard PDU Header.

struct RecvFactory {
    uint16_t msgType;

    RecvFactory(Stream & stream) {
        const unsigned expected = 2;    /* msgType(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::RecvFactory truncated msgType, need=%u remains=%u"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->msgType = stream.in_uint16_le();
    }   // RecvFactory(Stream & stream)
};

struct CliprdrHeader {
    uint16_t msgType;
    uint16_t msgFlags;
    uint32_t dataLen;

    CliprdrHeader()
        : msgType(0)
        , msgFlags(0)
        , dataLen(0) {
    }   // CliprdrHeader()

    CliprdrHeader(uint16_t msgType, uint16_t msgFlags, uint32_t dataLen)
        : msgType(msgType)
        , msgFlags(msgFlags)
        , dataLen(dataLen) {
    }   // CliprdrHeader(uint16_t msgType, uint16_t msgFlags, uint32_t dataLen)

    virtual void emit(Stream & stream) {
        stream.out_uint16_le(this->msgType);
        stream.out_uint16_le(this->msgFlags);
        stream.out_uint32_le(this->dataLen);
        stream.mark_end();
    }   // void emit(Stream & stream)

    virtual void recv(Stream & stream, const RecvFactory & recv_factory) {
        const unsigned expected = 6;    /* msgFlags(2) + dataLen(4) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_INFO, "RDPECLIP::recv truncated data, need=%u remains=%u"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->msgType  = recv_factory.msgType;
        this->msgFlags = stream.in_uint16_le();
        this->dataLen  = stream.in_uint32_le();
    }
};  // struct CliprdrHeader

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
//  Format Names or Long Format Names. The type of structure used in the array
//  is determined by the presence of the CB_USE_LONG_FORMAT_NAMES (0x00000002)
//  flag in the generalFlags field of the General Capability Set (section
//  2.2.2.1.1.1). Each array holds a list of the Clipboard Format ID and name
//  pairs available on the local system clipboard of the sender. If Short
//  Format Names are being used, and the embedded Clipboard Format names are
//  in ASCII 8 format, then the msgFlags field of the clipHeader must contain
//  the CB_ASCII_NAMES (0x0004) flag.

struct FormatListPDU : public CliprdrHeader {
    bool contians_data_in_text_format;

    FormatListPDU() : CliprdrHeader(), contians_data_in_text_format(false) {}

    virtual void recv(Stream & stream, const RecvFactory & recv_factory) {
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
        // |                           formatName                          |
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
        for(uint32_t i = 0; i < (dataLen / short_format_name_structure_size); i++) {
            if (!stream.in_check_rem(short_format_name_structure_size)) {
                LOG( LOG_INFO
                   , "RDPECLIP::FormatListPDU truncated CLIPRDR_SHORT_FORMAT_NAME structure, need=%u remains=%u"
                   , short_format_name_structure_size, stream.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            uint32_t formatId = stream.in_uint32_le();

            if (   (formatId == CHANNELS::CF_UNICODETEXT)
                || (formatId == CHANNELS::CF_TEXT)
                ) {
                LOG(LOG_INFO, "RDPECLIP::FormatListPDU formatId=%u", formatId);
                this->contians_data_in_text_format = true;

                break;
            }

            stream.in_skip_bytes(32);   // formatName(32)
        }
    }   // void recv(Stream & stream, const RecvFactory & recv_factory)
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
    FormatListResponsePDU(bool response_ok)
        : CliprdrHeader( CB_FORMAT_LIST_RESPONSE
                       , (response_ok ? CB_RESPONSE_OK : CB_RESPONSE_FAIL)
                       , 0) {
    }   // FormatListResponsePDU(bool response_ok)
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

struct FormatDataRequestPDU : public CliprdrHeader {
    uint32_t requestedFormatId;

    FormatDataRequestPDU(uint32_t requestedFormatId)
            : CliprdrHeader(CB_FORMAT_DATA_REQUEST, 0, 4)
            , requestedFormatId(requestedFormatId) {
    }   // FormatDataRequestPDU(uint32_t requestedFormatId)

    virtual void emit(Stream & stream) {
        CliprdrHeader::emit(stream);

        stream.out_uint32_le(this->requestedFormatId);
        stream.mark_end();
    }   // void emit(Stream & stream)
};  // struct FormatDataRequestPDU

}   // namespace RDPECLIP

#endif  // #ifndef _REDEMPTION_CORE_RDP_CLIPBOARD_HPP_
