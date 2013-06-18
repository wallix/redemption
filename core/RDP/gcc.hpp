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

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#ifndef _REDEMPTION_CORE_RDP_GCC_HPP_
#define _REDEMPTION_CORE_RDP_GCC_HPP_

#include "stream.hpp"
#include "ssl_calls.hpp"

enum DATA_BLOCK_TYPE {
    //  The data block that follows contains Client Core Data (section 2.2.1.3.2).
    CS_CORE = 0xC001,
    // The data block that follows contains Client Security Data (section 2.2.1.3.3).
    CS_SECURITY = 0xC002,
    // The data block that follows contains Client Network Data (section 2.2.1.3.4).
    CS_NET = 0xC003,
    // The data block that follows contains Client Cluster Data (section 2.2.1.3.5).
    CS_CLUSTER = 0xC004,
    // The data block that follows contains Client Monitor Data (section 2.2.1.3.6).
    CS_MONITOR = 0xC005,
    // The data block that follows contains Server Core Data (section 2.2.1.4.2).
    SC_CORE = 0x0C01,
    // The data block that follows contains Server Security Data (section 2.2.1.4.3).
    SC_SECURITY = 0x0C02,
    // The data block that follows contains Server Network Data (section 2.2.1.4.4).
    SC_NET = 0x0C03
};

namespace GCC
{
// ConferenceName ::= SEQUENCE
// {
//    numeric     SimpleNumericString,
//    text        SimpleTextString OPTIONAL,
//    ...,
//    unicodeText TextString OPTIONAL
// }

// ConferenceCreateRequest ::= SEQUENCE
// { -- MCS-Connect-Provider request user data
//    conferenceName         ConferenceName,
//    convenerPassword       Password OPTIONAL,
//    password               Password OPTIONAL,
//    lockedConference       BOOLEAN,
//    listedConference       BOOLEAN,
//    conductibleConference  BOOLEAN,
//    terminationMethod      TerminationMethod,
//    conductorPrivileges    SET OF Privilege OPTIONAL,
//    conductedPrivileges    SET OF Privilege OPTIONAL,
//    nonConductedPrivileges SET OF Privilege OPTIONAL,
//    conferenceDescription  TextString OPTIONAL,
//    callerIdentifier       TextString OPTIONAL,
//    userData               UserData OPTIONAL,
//    ...,
//    conferencePriority     ConferencePriority OPTIONAL,
//    conferenceMode         ConferenceMode OPTIONAL
// }

//    PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
//    00 05 00 14 7c 00 01 81 2a 00 08 00 10 00 01 c0
//    00 44 75 63 61 81 1c

//    0 - CHOICE: From Key select object (0) of type OBJECT IDENTIFIER
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    05 -> object length = 5 bytes

//    00 14 7c 00 01 -> object
//    The first byte gives the first two values in the sextuple (m and n), as it is encoded as
//    40m + n. Hence, decoding the remaining data yields the correct results:

//    OID = { 0 0 20 124 0 1 } = {itu-t(0) recommendation(0) t(20) t124(124) version(0) 1}
//    Description = v.1 of ITU-T Recommendation T.124 (Feb 1998): "Generic Conference Control"

//    81 2a -> ConnectData::connectPDU length = 298 bytes
//    Since the most significant bit of the first byte (0x81) is set to 1 and the following bit is
//    set to 0, the length is given by the low six bits of the first byte and the second byte.
//    Hence, the value is 0x12a, which is 298 bytes.

//    PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Request PDU:
//    00 08 00 10 00 01 c0 00 44 75 63 61 81 1c

//    0x00:
//    0 - extension bit (ConnectGCCPDU)
//    0 - --\.
//    0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateRequest (0)
//    0 - --/ of type ConferenceCreateRequest
//    0 - extension bit (ConferenceCreateRequest)
//    0 - ConferenceCreateRequest::convenerPassword present
//    0 - ConferenceCreateRequest::password present
//    0 - ConferenceCreateRequest::conductorPrivileges present


//    0x08:
//    0 - ConferenceCreateRequest::conductedPrivileges present
//    0 - ConferenceCreateRequest::nonConductedPrivileges present
//    0 - ConferenceCreateRequest::conferenceDescription present
//    0 - ConferenceCreateRequest::callerIdentifier present
//    1 - ConferenceCreateRequest::userData present
//    0 - extension bit (ConferenceName)
//    0 - ConferenceName::text present
//    0 - padding

//    0x00
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | ConferenceName::numeric length = 0 + 1 = 1 character
//    0 -   | (minimum for SimpleNumericString is 1)
//    0 -   |
//    0 -   |
//    0 - --/

//    0x10:
//    0 - --\.
//    0 -   | ConferenceName::numeric = "1"
//    0 -   |
//    1 - --/
//    0 - ConferenceCreateRequest::lockedConference
//    0 - ConferenceCreateRequest::listedConference
//    0 - ConferenceCreateRequest::conducibleConference
//    0 - extension bit (TerminationMethod)

//    0x00:
//    0 - TerminationMethod::automatic
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    0x01:
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | number of UserData sets = 1
//    0 -   |
//    0 -   |
//    0 -   |
//    1 - --/

//    0xc0:
//    1 - UserData::value present
//    1 - CHOICE: From Key select h221NonStandard (1) of type H221NonStandardIdentifier
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding
//    0 - padding

//    0x00:
//    0 - --\.
//    0 -   |
//    0 -   |
//    0 -   | h221NonStandard length = 0 + 4 = 4 octets
//    0 -   | (minimum for H221NonStandardIdentifier is 4)
//    0 -   |
//    0 -   |
//    0 - --/

//    44 75 63 61 -> h221NonStandard (client-to-server H.221 key) = "Duca"
//    81 1c -> UserData::value length = 284 bytes
//    Since the most significant bit of the first byte (0x81) is set to 1 and the following bit is
//    set to 0, the length is given by the low six bits of the first byte and the second byte.
//    Hence, the value is 0x11c, which is 284 bytes.

    enum {
        MCS_GLOBAL_CHANNEL   = 1003,
        MCS_USERCHANNEL_BASE = 1001,
    };

    class Create_Request_Send {
        public:
        Create_Request_Send(Stream & stream, size_t payload_size) {
            // ConnectData
            stream.out_per_choice(0); // From Key select object (0) of type OBJECT_IDENTIFIER
            const uint8_t t124_02_98_oid[6] = { 0, 0, 20, 124, 0, 1 };
            stream.out_per_object_identifier(t124_02_98_oid); // ITU-T T.124 (02/98) OBJECT_IDENTIFIER

            //  ConnectData::connectPDU (OCTET_STRING)
            uint16_t offset_len = stream.get_offset();
            stream.out_per_length(256); // connectPDU length (reserve 16 bits)

            //  ConnectGCCPDU
            stream.out_per_choice(0); // From ConnectGCCPDU select conferenceCreateRequest (0) of type ConferenceCreateRequest
            stream.out_per_selection(0x08); // select optional userData from ConferenceCreateRequest

            //  ConferenceCreateRequest::conferenceName
            //	stream.out_per_numeric_string(s, (uint8*)"1", 1, 1); /* ConferenceName::numeric */
            stream.out_uint16_be(16);
            stream.out_per_padding(1); /* padding */

            //  UserData (SET OF SEQUENCE)
            stream.out_per_number_of_sets(1); // one set of UserData
            stream.out_per_choice(0xC0); // UserData::value present + select h221NonStandard (1)

            //  h221NonStandard
            const uint8_t h221_cs_key[4] = {'D', 'u', 'c', 'a'};
            stream.out_per_octet_string(h221_cs_key, 4, 4); // h221NonStandard, client-to-server H.221 key, "Duca"

            stream.out_per_length(payload_size); // user data length
            stream.mark_end();

            stream.set_out_per_length(payload_size + stream.get_offset() - 9, offset_len); // length including header
        }
    };

    class Create_Request_Recv {
        public:
        size_t payload_size;

        SubStream payload;

        Create_Request_Recv(Stream & stream) {
            // Key select object (0) of type OBJECT_IDENTIFIER
            // ITU-T T.124 (02/98) OBJECT_IDENTIFIER
            stream.in_skip_bytes(7);
            uint16_t length_with_header = stream.in_per_length();

            // ConnectGCCPDU
            // From ConnectGCCPDU select conferenceCreateRequest (0) of type ConferenceCreateRequest
            // select optional userData from ConferenceCreateRequest
            // ConferenceCreateRequest::conferenceName
            // UserData (SET OF SEQUENCE), one set of UserData
            // UserData::value present + select h221NonStandard (1)
            // h221NonStandard, client-to-server H.221 key, "Duca"

            stream.in_skip_bytes(12);
            uint16_t length = stream.in_per_length();

            if (length_with_header != length + 14){
                LOG(LOG_WARNING, "GCC Conference Create Request User data Length mismatch with header+data length %u %u", length, length_with_header);
                throw Error(ERR_GCC);
            }

            if (length != stream.size() - stream.get_offset()){
                LOG(LOG_WARNING, "GCC Conference Create Request User data Length mismatch with header %u %u", length, stream.size() - stream.get_offset());
                throw Error(ERR_GCC);
            }
            this->payload.resize(stream, stream.size() - stream.get_offset());
        }
    };

    // GCC Conference Create Response
    // ------------------------------

    // ConferenceCreateResponse Parameters
    // -----------------------------------

    // Generic definitions used in parameter descriptions:

    // simpleTextFirstCharacter UniversalString ::= {0, 0, 0, 0}

    // simpleTextLastCharacter UniversalString ::= {0, 0, 0, 255}

    // SimpleTextString ::=  BMPString (SIZE (0..255)) (FROM (simpleTextFirstCharacter..simpleTextLastCharacter))

    // TextString ::= BMPString (SIZE (0..255)) -- Basic Multilingual Plane of ISO/IEC 10646-1 (Unicode)

    // SimpleNumericString ::= NumericString (SIZE (1..255)) (FROM ("0123456789"))

    // DynamicChannelID ::= INTEGER (1001..65535) -- Those created and deleted by MCS

    // UserID ::= DynamicChannelID

    // H221NonStandardIdentifier ::= OCTET STRING (SIZE (4..255))
    //      -- First four octets shall be country code and
    //      -- Manufacturer code, assigned as specified in
    //      -- Annex A/H.221 for NS-cap and NS-comm

    // Key ::= CHOICE   -- Identifier of a standard or non-standard object
    // {
    //      object              OBJECT IDENTIFIER,
    //      h221NonStandard     H221NonStandardIdentifier
    // }

    // UserData ::= SET OF SEQUENCE
    // {
    //      key     Key,
    //      value   OCTET STRING OPTIONAL
    // }

    // ConferenceCreateResponse ::= SEQUENCE
    // {    -- MCS-Connect-Provider response user data
    //      nodeID              UserID, -- Node ID of the sending node
    //      tag                 INTEGER,
    //      result              ENUMERATED
    //      {
    //          success                         (0),
    //          userRejected                    (1),
    //          resourcesNotAvailable           (2),
    //          rejectedForSymmetryBreaking     (3),
    //          lockedConferenceNotSupported    (4),
    //          ...
    //      },
    //      userData            UserData OPTIONAL,
    //      ...
    //}


    // User Data                 : Optional
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // User Data: Optional user data which may be used for functions outside
    // the scope of this Recommendation such as authentication, billing,
    // etc.

    // Result                    : Mandatory
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // An indication of whether the request was accepted or rejected, and if
    // rejected, the reason why. It contains one of a list of possible
    // results: successful, user rejected, resources not available, rejected
    // for symmetry-breaking, locked conference not supported, Conference
    // Name and Conference Name Modifier already exist, domain parameters
    // unacceptable, domain not hierarchical, lower-layer initiated
    // disconnect, unspecified failure to connect. A negative result in the
    // GCC-Conference-Create confirm does not imply that the physical
    // connection to the node to which the connection was being attempted
    // is disconnected.

    // The ConferenceCreateResponse PDU is shown in Table 8-4. The Node ID
    // parameter, which is the User ID assigned by MCS in response to the
    // MCS-Attach-User request issued by the GCC Provider, shall be supplied
    // by the GCC Provider sourcing this PDU. The Tag parameter is assigned
    // by the source GCC Provider to be locally unique. It is used to
    // identify the returned UserIDIndication PDU. The Result parameter
    // include " GCC-specific failure information sourced directly from
    // the Result parameter in the GCC-Conference-Create response primitive.
    // If the Result parameter is anything except successful, the Result
    // parameter in the MCS-Connect-Provider response is set to
    // user-rejected.

    //            Table 8-4 – ConferenceCreateResponse GCCPDU
    // +------------------+------------------+--------------------------+
    // | Content          |     Source       |         Sink             |
    // +==================+==================+==========================+
    // | Node ID          | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Tag              | Top GCC Provider | Destination GCC Provider |
    // +------------------+------------------+--------------------------+
    // | Result           | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+
    // | User Data (opt.) | Response         | Confirm                  |
    // +------------------+------------------+--------------------------+

    //PER encoded (ALIGNED variant of BASIC-PER) GCC Connection Data (ConnectData):
    // 00 05 00
    // 14 7c 00 01
    // 2a
    // 14 76 0a 01 01 00 01 c0 00 4d 63 44 6e
    // 81 08

    class Create_Response_Send {
        public:
        Create_Response_Send(Stream & stream, size_t payload_size) {
            // ConnectData
            // 00 05 -> Key::object length = 5 bytes
            // 00 14 7c 00 01 -> Key::object = { 0 0 20 124 0 1 }
            stream.out_uint16_be(5);
            stream.out_copy_bytes("\x00\x14\x7c\x00\x01", 5);

            // 2a -> ConnectData::connectPDU length = 42 bytes
            // This length MUST be ignored by the client.
            stream.out_uint8(0x2a);

            // PER encoded (ALIGNED variant of BASIC-PER) GCC Conference Create Response
            // PDU:
            // 14 76 0a 01 01 00 01 c0 00 00 4d 63 44 6e 81 08

            // 0x14:
            // 0 - extension bit (ConnectGCCPDU)
            // 0 - --\ ...
            // 0 -   | CHOICE: From ConnectGCCPDU select conferenceCreateResponse (1)
            // 1 - --/ of type ConferenceCreateResponse
            // 0 - extension bit (ConferenceCreateResponse)
            // 1 - ConferenceCreateResponse::userData present
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0x10 | 4);

            // ConferenceCreateResponse::nodeID
            //  = 0x760a + 1001 = 30218 + 1001 = 31219
            //  (minimum for UserID is 1001)
            stream.out_uint16_le(0x760a);

            // ConferenceCreateResponse::tag length = 1 byte
            stream.out_uint8(1);

            // ConferenceCreateResponse::tag = 1
            stream.out_uint8(1);

            // 0x00:
            // 0 - extension bit (Result)
            // 0 - --\ ...
            // 0 -   | ConferenceCreateResponse::result = success (0)
            // 0 - --/
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0);

            // number of UserData sets = 1
            stream.out_uint8(1);

            // 0xc0:
            // 1 - UserData::value present
            // 1 - CHOICE: From Key select h221NonStandard (1)
            //               of type H221NonStandardIdentifier
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            // 0 - padding
            stream.out_uint8(0xc0);

            // h221NonStandard length = 0 + 4 = 4 octets
            //   (minimum for H221NonStandardIdentifier is 4)
            stream.out_uint8(0);

            // h221NonStandard (server-to-client H.221 key) = "McDn"
            stream.out_copy_bytes("McDn", 4);

            // set user_data_len (TWO_BYTE_UNSIGNED_ENCODING)
            stream.out_uint16_be(0x8000 | payload_size);
            stream.mark_end();
        }
    };

    class Create_Response_Recv {
        public:
        size_t payload_size;

        SubStream payload;

        Create_Response_Recv(Stream & stream) {
            stream.in_skip_bytes(21); /* header (T.124 ConferenceCreateResponse) */
            size_t length = stream.in_per_length();
            if (length != stream.size() - stream.get_offset()){
                LOG(LOG_WARNING, "GCC Conference Create Response User data Length mismatch with header %u %u",
                    length, stream.size() - stream.get_offset());
                throw Error(ERR_GCC);
            }

            this->payload.resize(stream, stream.size() - stream.get_offset());
        }
    };


    // 2.2.1.3.1 User Data Header (TS_UD_HEADER)
    // =========================================

    // type (2 bytes): A 16-bit, unsigned integer. The type of the data
    //                 block that this header precedes.

    // +-------------------+-------------------------------------------------------+
    // | CS_CORE 0xC001    | The data block that follows contains Client Core      |
    // |                   | Data (section 2.2.1.3.2).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_SECURITY 0xC002| The data block that follows contains Client           |
    // |                   | Security Data (section 2.2.1.3.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | CS_NET 0xC003     | The data block that follows contains Client Network   |
    // |                   | Data (section 2.2.1.3.4).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_CLUSTER 0xC004 | The data block that follows contains Client Cluster   |
    // |                   | Data (section 2.2.1.3.5).                             |
    // +-------------------+-------------------------------------------------------+
    // | CS_MONITOR 0xC005 | The data block that follows contains Client           |
    // |                   | Monitor Data (section 2.2.1.3.6).                     |
    // +-------------------+-------------------------------------------------------+
    // | SC_CORE 0x0C01    | The data block that follows contains Server Core      |
    // |                   | Data (section 2.2.1.4.2)                              |
    // +-------------------+-------------------------------------------------------+
    // | SC_SECURITY 0x0C02| The data block that follows contains Server           |
    // |                   | Security Data (section 2.2.1.4.3).                    |
    // +-------------------+-------------------------------------------------------+
    // | SC_NET 0x0C03     | The data block that follows contains Server Network   |
    // |                   | Data (section 2.2.1.4.4)                              |
    // +-------------------+-------------------------------------------------------+

    // length (2 bytes): A 16-bit, unsigned integer. The size in bytes of the data
    //   block, including this header.

    namespace UserData
    {
        struct RecvFactory
        {
            uint16_t tag;
            uint16_t length;
            SubStream payload;

            RecvFactory(Stream & stream) : payload(stream, stream.get_offset())
            {
                if (!stream.in_check_rem(4)){
                    LOG(LOG_WARNING, "Incomplete GCC::UserData data block header");
                    throw Error(ERR_GCC);
                }
                this->tag = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                LOG(LOG_INFO, "GCC::UserData tag=%0.4x length=%u", tag, length);
                if (!stream.in_check_rem(length - 4)){
                    LOG(LOG_WARNING, "Incomplete GCC::UserData data block"
                                     " tag=%u length=%u available_length=%u",
                                     tag, length, stream.size() - 4);
                    throw Error(ERR_GCC);
                }
                stream.in_skip_bytes(length - 4);
                this->payload.resize(this->payload, length);
            }
        };


        // 2.2.1.4.2 Server Core Data (TS_UD_SC_CORE)
        // ==========================================

        // The TS_UD_SC_CORE data block contains core server connection-related
        // information.

        // header (4 bytes): GCC user data block header, as specified in User Data
        //  Header (section 2.2.1.3.1). The User Data Header type field MUST be set to
        //  SC_CORE (0x0C01).

        // version (4 bytes): A 32-bit, unsigned integer. The server version number for
        //  the RDP. The major version number is stored in the high two bytes, while the
        //  minor version number is stored in the low two bytes.

        // 0x00080001 RDP 4.0 servers
        // 0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers

        // If the server advertises a version number greater than or equal to 0x00080004,
        // it MUST support a maximum length of 512 bytes for the UserName field in the
        // Info Packet (section 2.2.1.11.1.1).

        // clientRequestedProtocols (4 bytes): A 32-bit, unsigned integer that contains
        // the flags sent by the client in the requestedProtocols field of the RDP
        // Negotiation Request (section 2.2.1.1.1). In the event that an RDP Negotiation
        // Request was not received from the client, this field MUST be initialized to
        // PROTOCOL_RDP (0). If this field is not present, all of the subsequent
        // fields MUST NOT be present.

        // earlyCapabilityFlags (4 bytes): A 32-bit, unsigned integer that specifies
        // capabilities early in the connection sequence. If this field is present, all
        // of the preceding fields MUST also be present.

        // +----------------------------------+-----------------------------------------+
        // |             0x00000001           | Indicates that the following key        |
        // | RNS_UD_SC_EDGE_ACTIONS_SUPPORTED | combinations are reserved by the server |
        // |                                  | operating system:                       |
        // |                                  |             - WIN + Z                   |
        // |                                  |             - WIN + CTRL + TAB          |
        // |                                  |             - WIN + C                   |
        // |                                  |             - WIN + .                   |
        // |                                  |             - WIN + SHIFT + .           |
        // |                                  | In addition, the monitor boundaries of  |
        // |                                  | the remote session are employed by the  |
        // |                                  | server operating system to trigger user |
        // |                                  | interface elements via touch or mouse   |
        // |                                  | gestures.                               |
        // +----------------------------------+-----------------------------------------+
        // |             0x00000002           | Indicates that the server supports      |
        // | RNS_UD_SC_DYNAMIC_DST_SUPPORTED  | Dynamic DST. Dynamic DST information is |
        // |                                  | provided by the client in the           |
        // |                                  | cbDynamicDSTTimeZoneKeyName,            |
        // |                                  | dynamicDSTTimeZoneKeyName and           |
        // |                                  | dynamicDaylightTimeDisabled fields of   |
        // |                                  | the Extended Info Packet (section       |
        // |                                  | 2.2.1.11.1.1.1).                        |
        // +----------------------------------+-----------------------------------------+

        // Exemple:
        //01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12 bytes
        //04 00 08 00 -> TS_UD_SC_CORE::version = 0x0080004
        //00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

        struct SCCore {
            uint16_t userDataType;
            uint16_t length;
            uint32_t version;
            uint32_t clientRequestedProtocols;
            uint32_t earlyCapabilityFlags;

            SCCore()
            : userDataType(SC_CORE)
            , length(8)
            , version(0x00080001)
            , clientRequestedProtocols(0)
            , earlyCapabilityFlags(0)
            {
            }

            void emit(Stream & stream)
            {
                if (this->length != 8
                && this->length != 12
                && this->length != 16) {
                    LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                    throw Error(ERR_GCC);
                };

                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->version);

                if (this->length >= 12){
                    stream.out_uint32_le(this->clientRequestedProtocols);
                }
                if (this->length >= 16){
                    stream.out_uint32_le(this->earlyCapabilityFlags);
                }
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->version = stream.in_uint32_le();
                if (this->length < 12) {
                    if (this->length != 8) {
                        LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                        throw Error(ERR_GCC);
                    }
                    return;
                }
                this->clientRequestedProtocols = stream.in_uint32_le();
                if (this->length < 16) {
                    if (this->length != 12) {
                        LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                        throw Error(ERR_GCC);
                    }
                    return;
                }
                this->earlyCapabilityFlags = stream.in_uint32_le();
                if (this->length != 16) {
                    LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                    throw Error(ERR_GCC);
                }
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data SC_CORE (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "sc_core::version [%04x] %s", this->version,
                      (this->version==0x00080001) ? "RDP 4 client"
                     :(this->version==0x00080004) ? "RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 servers)"
                                                  : "Unknown client");
                if (this->length < 12) {
                    if (this->length != 8) {
                        LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                        throw Error(ERR_GCC);
                    }
                    return;
                }
                LOG(LOG_INFO, "sc_core::clientRequestedProtocols  = %u", this->clientRequestedProtocols);
                if (this->length < 16) {
                    if (this->length != 12) {
                        LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                        throw Error(ERR_GCC);
                    }
                    return;
                }
                LOG(LOG_INFO, "sc_core::earlyCapabilityFlags  = %u", this->earlyCapabilityFlags);
                if (this->length != 16) {
                    LOG(LOG_ERR, "SC_CORE invalid length (%u)", this->length);
                    throw Error(ERR_GCC);
                }
            }
        };

        // 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
        // -------------------------------------
        // Below relevant quotes from MS-RDPBCGR v20100601 (2.2.1.3.2)

        // header (4 bytes): GCC user data block header, as specified in section
        //                   2.2.1.3.1. The User Data Header type field MUST be
        //                   set to CS_CORE (0xC001).

        // version (4 bytes): A 32-bit, unsigned integer. Client version number
        //                    for the RDP. The major version number is stored in
        //                    the high 2 bytes, while the minor version number
        //                    is stored in the low 2 bytes.
        //
        //         Value Meaning
        //         0x00080001 RDP 4.0 clients
        //         0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 clients

        // desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested
        //                         desktop width in pixels (up to a maximum
        //                         value of 4096 pixels).

        // desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested
        //                         desktop height in pixels (up to a maximum
        //                         value of 2048 pixels).

        // colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
        //                       depth. Values in this field MUST be ignored if
        //                       the postBeta2ColorDepth field is present.
        //          Value Meaning
        //          RNS_UD_COLOR_4BPP 0xCA00 4 bits-per-pixel (bpp)
        //          RNS_UD_COLOR_8BPP 0xCA01 8 bpp

        // SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access
        //                        sequence. This field SHOULD be set to
        //                        RNS_UD_SAS_DEL (0xAA03).

        // keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout
        //                           (active input locale identifier). For a
        //                           list of possible input locales, see
        //                           [MSDN-MUI].

        // clientBuild (4 bytes): A 32-bit, unsigned integer. The build number
        // of the client.

        // clientName (32 bytes): Name of the client computer. This field
        //                        contains up to 15 Unicode characters plus a
        //                        null terminator.

        // keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
        //              Value Meaning
        //              0x00000001 IBM PC/XT or compatible (83-key) keyboard
        //              0x00000002 Olivetti "ICO" (102-key) keyboard
        //              0x00000003 IBM PC/AT (84-key) and similar keyboards
        //              0x00000004 IBM enhanced (101-key or 102-key) keyboard
        //              0x00000005 Nokia 1050 and similar keyboards
        //              0x00000006 Nokia 9140 and similar keyboards
        //              0x00000007 Japanese keyboard

        // keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard
        //                        subtype (an original equipment manufacturer-
        //                        -dependent value).

        // keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number
        //                        of function keys on the keyboard.

        // If the Layout Manager entry points for LayoutMgrGetKeyboardType and
        // LayoutMgrGetKeyboardLayoutName do not exist, the values in certain registry
        // keys are queried and their values are returned instead. The following
        // registry key example shows the registry keys to configure to support RDP.

        // [HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\KEYBD]
        //    "Keyboard Type"=dword:<type>
        //    "Keyboard SubType"=dword:<subtype>
        //    "Keyboard Function Keys"=dword:<function keys>
        //    "Keyboard Layout"="<layout>"

        // To set these values for the desired locale, set the variable DEFINE_KEYBOARD_TYPE
        // in Platform.reg before including Keybd.reg. The following code sample shows
        // how to set the DEFINE_KEYBOARD_TYPE in Platform.reg before including Keybd.reg.

        //    #define DEFINE_KEYBOARD_TYPE
        //    #include "$(DRIVERS_DIR)\keybd\keybd.reg"
        //    This will bring in the proper values for the current LOCALE, if it is
        //    supported. Logic in Keybd.reg sets these values. The following registry
        //    example shows this logic.
        //    ; Define this variable in platform.reg if your keyboard driver does not
        //    ; report its type information.
        //    #if defined DEFINE_KEYBOARD_TYPE

        //    #if $(LOCALE)==0411

        //    ; Japanese keyboard layout
        //        "Keyboard Type"=dword:7
        //        "Keyboard SubType"=dword:2
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000411"

        //    #elif $(LOCALE)==0412

        //    ; Korean keyboard layout
        //        "Keyboard Type"=dword:8
        //        "Keyboard SubType"=dword:3
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000412"

        //    #else

        //    ; Default to US keyboard layout
        //        "Keyboard Type"=dword:4
        //        "Keyboard SubType"=dword:0
        //        "Keyboard Function Keys"=dword:c
        //        "Keyboard Layout"="00000409"

        //    #endif

        //    #endif ; DEFINE_KEYBOARD_TYPE


        // imeFileName (64 bytes): A 64-byte field. The Input Method Editor
        //                        (IME) file name associated with the input
        //                        locale. This field contains up to 31 Unicode
        //                        characters plus a null terminator.

        // --> Note By CGR How do we know that the following fields are
        //     present of Not ? The only rational method I see is to look
        //     at the length field in the preceding User Data Header
        //     120 bytes without optional data
        //     216 bytes with optional data present

        // postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The
        //                        requested color depth. Values in this field
        //                        MUST be ignored if the highColorDepth field
        //                        is present.
        //       Value Meaning
        //       RNS_UD_COLOR_4BPP 0xCA00        : 4 bits-per-pixel (bpp)
        //       RNS_UD_COLOR_8BPP 0xCA01        : 8 bpp
        //       RNS_UD_COLOR_16BPP_555 0xCA02   : 15-bit 555 RGB mask
        //                                         (5 bits for red, 5 bits for
        //                                         green, and 5 bits for blue)
        //       RNS_UD_COLOR_16BPP_565 0xCA03   : 16-bit 565 RGB mask
        //                                         (5 bits for red, 6 bits for
        //                                         green, and 5 bits for blue)
        //       RNS_UD_COLOR_24BPP 0xCA04       : 24-bit RGB mask
        //                                         (8 bits for red, 8 bits for
        //                                         green, and 8 bits for blue)
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // clientProductId (2 bytes): A 16-bit, unsigned integer. The client
        //                          product ID. This field SHOULD be initialized
        //                          to 1. If this field is present, all of the
        //                          preceding fields MUST also be present. If
        //                          this field is not present, all of the
        //                          subsequent fields MUST NOT be present.

        // serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number.
        //                         This field SHOULD be initialized to 0. If
        //                         this field is present, all of the preceding
        //                         fields MUST also be present. If this field
        //                         is not present, all of the subsequent fields
        //                         MUST NOT be present.

        // highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
        //                         color depth.
        //          Value Meaning
        // HIGH_COLOR_4BPP  0x0004             : 4 bpp
        // HIGH_COLOR_8BPP  0x0008             : 8 bpp
        // HIGH_COLOR_15BPP 0x000F             : 15-bit 555 RGB mask
        //                                       (5 bits for red, 5 bits for
        //                                       green, and 5 bits for blue)
        // HIGH_COLOR_16BPP 0x0010             : 16-bit 565 RGB mask
        //                                       (5 bits for red, 6 bits for
        //                                       green, and 5 bits for blue)
        // HIGH_COLOR_24BPP 0x0018             : 24-bit RGB mask
        //                                       (8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        //
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies
        //                                 the high color depths that the client
        //                                 is capable of supporting.
        //
        //         Flag Meaning
        //   RNS_UD_24BPP_SUPPORT 0x0001       : 24-bit RGB mask
        //                                       (8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        //   RNS_UD_16BPP_SUPPORT 0x0002       : 16-bit 565 RGB mask
        //                                       (5 bits for red, 6 bits for
        //                                       green, and 5 bits for blue)
        //   RNS_UD_15BPP_SUPPORT 0x0004       : 15-bit 555 RGB mask
        //                                       (5 bits for red, 5 bits for
        //                                       green, and 5 bits for blue)
        //   RNS_UD_32BPP_SUPPORT 0x0008       : 32-bit RGB mask
        //                                       (8 bits for the alpha channel,
        //                                       8 bits for red, 8 bits for
        //                                       green, and 8 bits for blue)
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // earlyCapabilityFlags (2 bytes)      : A 16-bit, unsigned integer. It
        //                                       specifies capabilities early in
        //                                       the connection sequence.
        //        Flag                        Meaning
        //  RNS_UD_CS_SUPPORT_ERRINFO_PDU Indicates that the client supports
        //    0x0001                        the Set Error Info PDU
        //                                 (section 2.2.5.1).
        //
        //  RNS_UD_CS_WANT_32BPP_SESSION Indicates that the client is requesting
        //    0x0002                     a session color depth of 32 bpp. This
        //                               flag is necessary because the
        //                               highColorDepth field does not support a
        //                               value of 32. If this flag is set, the
        //                               highColorDepth field SHOULD be set to
        //                               24 to provide an acceptable fallback
        //                               for the scenario where the server does
        //                               not support 32 bpp color.
        //
        //  RNS_UD_CS_SUPPORT_STATUSINFO_PDU  Indicates that the client supports
        //    0x0004                          the Server Status Info PDU
        //                                    (section 2.2.5.2).
        //
        //  RNS_UD_CS_STRONG_ASYMMETRIC_KEYS  Indicates that the client supports
        //    0x0008                          asymmetric keys larger than
        //                                    512 bits for use with the Server
        //                                    Certificate (section 2.2.1.4.3.1)
        //                                    sent in the Server Security Data
        //                                    block (section 2.2.1.4.3).
        //
        //  RNS_UD_CS_VALID_CONNECTION_TYPE Indicates that the connectionType
        //     0x0020                       field contains valid data.
        //
        //  RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU Indicates that the client
        //     0x0040                            supports the Monitor Layout PDU
        //                                       (section 2.2.12.1).
        //
        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // clientDigProductId (64 bytes): Contains a value that uniquely
        //                                identifies the client. If this field
        //                                is present, all of the preceding
        //                                fields MUST also be present. If this
        //                                field is not present, all of the
        //                                subsequent fields MUST NOT be present.

        // connectionType (1 byte): An 8-bit unsigned integer. Hints at the type
        //                      of network connection being used by the client.
        //                      This field only contains valid data if the
        //                      RNS_UD_CS_VALID_CONNECTION_TYPE (0x0020) flag
        //                      is present in the earlyCapabilityFlags field.
        //
        //    Value                          Meaning
        //  CONNECTION_TYPE_MODEM 0x01 : Modem (56 Kbps)
        //  CONNECTION_TYPE_BROADBAND_LOW 0x02 : Low-speed broadband
        //                                 (256 Kbps - 2 Mbps)
        //  CONNECTION_TYPE_SATELLITE 0x03 : Satellite
        //                                 (2 Mbps - 16 Mbps with high latency)
        //  CONNECTION_TYPE_BROADBAND_HIGH 0x04 : High-speed broadband
        //                                 (2 Mbps - 10 Mbps)
        //  CONNECTION_TYPE_WAN 0x05 : WAN (10 Mbps or higher with high latency)
        //  CONNECTION_TYPE_LAN 0x06 : LAN (10 Mbps or higher)

        // If this field is present, all of the preceding fields MUST also be
        // present. If this field is not present, all of the subsequent fields
        // MUST NOT be present.

        // pad1octet (1 byte): An 8-bit, unsigned integer. Padding to align the
        //   serverSelectedProtocol field on the correct byte boundary. If this
        //   field is present, all of the preceding fields MUST also be present.
        //   If this field is not present, all of the subsequent fields MUST NOT
        //   be present.

        // serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer that
        //   contains the value returned by the server in the selectedProtocol
        //   field of the RDP Negotiation Response (section 2.2.1.2.1). In the
        //   event that an RDP Negotiation Response was not received from the
        //   server, this field MUST be initialized to PROTOCOL_RDP (0). This
        //   field MUST be present if an RDP Negotiation Request (section
        //   2.2.1.1.1) was sent to the server. If this field is present,
        //   then all of the preceding fields MUST also be present.


        struct CSCore {
            // header
            uint16_t userDataType;
            uint16_t length;
            uint32_t version;
            uint16_t desktopWidth;
            uint16_t desktopHeight;
            uint16_t colorDepth;
            uint16_t SASSequence;
            uint32_t keyboardLayout;
            uint32_t clientBuild;
            uint16_t clientName[16];
            uint32_t keyboardType;
            uint32_t keyboardSubType;
            uint32_t keyboardFunctionKey;
            uint16_t imeFileName[32];
            // optional payload
            uint16_t postBeta2ColorDepth;
            uint16_t clientProductId;
            uint32_t serialNumber;
            uint16_t highColorDepth;
            uint16_t supportedColorDepths;
            uint16_t earlyCapabilityFlags;
            uint8_t  clientDigProductId[64];
            uint8_t  connectionType;
            uint8_t  pad1octet;
            uint32_t serverSelectedProtocol;

            // we do not provide parameters in constructor,
            // because setting them one field at a time is more explicit and maintainable
            // (drawback: danger is different, not swapping parameters, but we may forget to define some...)
            CSCore()
            : userDataType(CS_CORE)
            , length(216) // default: everything including serverSelectedProtocol
            , version(0x00080001)  // RDP version. 1 == RDP4, 4 == RDP5.
            , colorDepth(0xca01)
            , SASSequence(0xaa03)
            , keyboardLayout(0x040c) // default to French
            , clientBuild(2600)
            // clientName = ""
            , keyboardType(4)
            , keyboardSubType(0)
            , keyboardFunctionKey(12)
            // imeFileName = ""
            , postBeta2ColorDepth(0xca01)
            , clientProductId(1)
            , serialNumber(0)
            , highColorDepth(0)
            , supportedColorDepths(7)
            , earlyCapabilityFlags(1)
            // clientDigProductId = ""
            , connectionType(0)
            , pad1octet(0)
            , serverSelectedProtocol(0)
            {
                bzero(this->clientName, 32);
                bzero(this->imeFileName, 64);
                bzero(this->clientDigProductId, 64);
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->version = stream.in_uint32_le();
                this->desktopWidth = stream.in_uint16_le();
                this->desktopHeight = stream.in_uint16_le();
                this->colorDepth = stream.in_uint16_le();
                this->SASSequence = stream.in_uint16_le();
                this->keyboardLayout =stream.in_uint32_le();
                this->clientBuild = stream.in_uint32_le();
                // utf16 hostname fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.in_utf16(this->clientName, 16);
                this->keyboardType = stream.in_uint32_le();
                this->keyboardSubType = stream.in_uint32_le();
                this->keyboardFunctionKey = stream.in_uint32_le();
                // utf16 fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.in_utf16(this->imeFileName, 32);
                // --------------------- Optional Fields ---------------------------------------
                if (this->length < 134) { return; }
                this->postBeta2ColorDepth = stream.in_uint16_le();
                if (this->length < 136) { return; }
                this->clientProductId = stream.in_uint16_le();
                if (this->length < 140) { return; }
                this->serialNumber = stream.in_uint32_le();
                if (this->length < 142) { return; }
                this->highColorDepth = stream.in_uint16_le();
                if (this->length < 144) { return; }
                this->supportedColorDepths = stream.in_uint16_le();
                if (this->length < 146) { return; }
                this->earlyCapabilityFlags = stream.in_uint16_le();
                if (this->length < 210) { return; }
                stream.in_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
                if (this->length < 211) { return; }
                this->connectionType = stream.in_uint8();
                if (this->length < 212) { return; }
                this->pad1octet = stream.in_uint8();
                if (this->length < 216) { return; }
                this->serverSelectedProtocol = stream.in_uint32_le();
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->version);
                stream.out_uint16_le(this->desktopWidth);
                stream.out_uint16_le(this->desktopHeight);
                stream.out_uint16_le(this->colorDepth);
                stream.out_uint16_le(this->SASSequence);
                stream.out_uint32_le(this->keyboardLayout);
                stream.out_uint32_le(this->clientBuild);
                // utf16 hostname fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.out_utf16(this->clientName, 16);
                stream.out_uint32_le(this->keyboardType);
                stream.out_uint32_le(this->keyboardSubType);
                stream.out_uint32_le(this->keyboardFunctionKey);
                // utf16 fixed length,
                // including mandatory terminal 0
                // length is a number of utf16 characters
                stream.out_utf16(this->imeFileName, 32);

                // --------------------- Optional Fields ---------------------------------------
                this->emit_optional(stream);
                stream.mark_end();
            }

            private:
            void emit_optional(Stream & stream)
            {
                if (this->length < 134) { return; }
                stream.out_uint16_le(this->postBeta2ColorDepth);
                if (this->length < 136) { return; }
                stream.out_uint16_le(this->clientProductId);
                if (this->length < 140) { return; }
                stream.out_uint32_le(this->serialNumber);
                if (this->length < 142) { return; }
                stream.out_uint16_le(this->highColorDepth);
                if (this->length < 144) { return; }
                stream.out_uint16_le(this->supportedColorDepths);
                if (this->length < 146) { return; }
                stream.out_uint16_le(this->earlyCapabilityFlags);
                if (this->length < 210) { return; }
                stream.out_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
                if (this->length < 211) { return; }
                stream.out_uint8(this->connectionType);
                if (this->length < 212) { return; }
                stream.out_uint8(this->pad1octet);
                if (this->length < 216) { return; }
                stream.out_uint32_le(this->serverSelectedProtocol);
            }

            public:
            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data CS_CORE (%u bytes)", msg, this->length);

                if (this->length < 132){
                    LOG(LOG_INFO, "GCC User Data CS_CORE truncated");
                    return;
                }

                LOG(LOG_INFO, "cs_core::version [%04x] %s", this->version,
                      (this->version==0x00080001) ? "RDP 4 client"
                     :(this->version==0x00080004) ? "RDP 5.0, 5.1, 5.2, and 6.0 clients)"
                                                  : "Unknown client");
                LOG(LOG_INFO, "cs_core::desktopWidth  = %u",  this->desktopWidth);
                LOG(LOG_INFO, "cs_core::desktopHeight = %u", this->desktopHeight);
                LOG(LOG_INFO, "cs_core::colorDepth    = [%04x] [%s] superseded by postBeta2ColorDepth", this->colorDepth,
                    (this->colorDepth == 0xCA00) ? "RNS_UD_COLOR_4BPP"
                  : (this->colorDepth == 0xCA01) ? "RNS_UD_COLOR_8BPP"
                                                 : "Unknown");
                LOG(LOG_INFO, "cs_core::SASSequence   = [%04x] [%s]", this->SASSequence,
                    (this->SASSequence == 0xCA00) ? "RNS_UD_SAS_DEL"
                                                  : "Unknown");
                LOG(LOG_INFO, "cs_core::keyboardLayout= %04x",  this->keyboardLayout);
                LOG(LOG_INFO, "cs_core::clientBuild   = %u",  this->clientBuild);
                char hostname[16];
                for (size_t i = 0; i < 16 ; i++) {
                    hostname[i] = (uint8_t)this->clientName[i];
                }
                LOG(LOG_INFO, "cs_core::clientName    = %s",  hostname);
                LOG(LOG_INFO, "cs_core::keyboardType  = [%04x] %s",  this->keyboardType,
                      (this->keyboardType == 0x00000001) ? "IBM PC/XT or compatible (83-key) keyboard"
                    : (this->keyboardType == 0x00000002) ? "Olivetti \"ICO\" (102-key) keyboard"
                    : (this->keyboardType == 0x00000003) ? "IBM PC/AT (84-key) and similar keyboards"
                    : (this->keyboardType == 0x00000004) ? "IBM enhanced (101-key or 102-key) keyboard"
                    : (this->keyboardType == 0x00000005) ? "Nokia 1050 and similar keyboards"
                    : (this->keyboardType == 0x00000006) ? "Nokia 9140 and similar keyboards"
                    : (this->keyboardType == 0x00000007) ? "Japanese keyboard"
                                                         : "Unknown");
                LOG(LOG_INFO, "cs_core::keyboardSubType      = [%04x] OEM code",  this->keyboardSubType);
                LOG(LOG_INFO, "cs_core::keyboardFunctionKey  = %u function keys",  this->keyboardFunctionKey);
                char imename[32];
                for (size_t i = 0; i < 32 ; i++){
                    imename[i] = (uint8_t)this->imeFileName[i];
                }
                LOG(LOG_INFO, "cs_core::imeFileName    = %s",  imename);

                // --------------------- Optional Fields ---------------------------------------
                if (this->length < 134) { return; }
                LOG(LOG_INFO, "cs_core::postBeta2ColorDepth  = [%04x] [%s]", this->postBeta2ColorDepth,
                    (this->postBeta2ColorDepth == 0xCA00) ? "4 bpp"
                  : (this->postBeta2ColorDepth == 0xCA01) ? "8 bpp"
                  : (this->postBeta2ColorDepth == 0xCA02) ? "15-bit 555 RGB mask"
                  : (this->postBeta2ColorDepth == 0xCA03) ? "16-bit 565 RGB mask"
                  : (this->postBeta2ColorDepth == 0xCA04) ? "24-bit RGB mask"
                                                          : "Unknown");
                if (this->length < 136) { return; }
                LOG(LOG_INFO, "cs_core::clientProductId = %u", this->clientProductId);
                if (this->length < 140) { return; }
                LOG(LOG_INFO, "cs_core::serialNumber = %u", this->serialNumber);
                if (this->length < 142) { return; }
                LOG(LOG_INFO, "cs_core::highColorDepth  = [%04x] [%s]", this->highColorDepth,
                    (this->highColorDepth == 4)  ? "4 bpp"
                  : (this->highColorDepth == 8)  ? "8 bpp"
                  : (this->highColorDepth == 15) ? "15-bit 555 RGB mask"
                  : (this->highColorDepth == 16) ? "16-bit 565 RGB mask"
                  : (this->highColorDepth == 24) ? "24-bit RGB mask"
                                                 : "Unknown");
                if (this->length < 144) { return; }
                LOG(LOG_INFO, "cs_core::supportedColorDepths  = [%04x] [%s/%s/%s/%s]", this->supportedColorDepths,
                    (this->supportedColorDepths & 1) ? "24":"",
                    (this->supportedColorDepths & 2) ? "16":"",
                    (this->supportedColorDepths & 4) ? "15":"",
                    (this->supportedColorDepths & 8) ? "32":"");
                if (this->length < 146) { return; }
                LOG(LOG_INFO, "cs_core::earlyCapabilityFlags  = [%04x]", this->earlyCapabilityFlags);
                if (this->earlyCapabilityFlags & 0x0001){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU");
                }
                if (this->earlyCapabilityFlags & 0x0002){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_WANT_32BPP_SESSION");
                }
                if (this->earlyCapabilityFlags & 0x0004){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU");
                }
                if (this->earlyCapabilityFlags & 0x0008){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS");
                }
                if (this->earlyCapabilityFlags & 0x00020){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE");
                }
                if (this->earlyCapabilityFlags & 0x00040){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU");
                }
                if (this->earlyCapabilityFlags & 0xFF10){
                    LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:Unknown early capability flag");
                }
                if (this->length < 210) { return; }
                const uint8_t (& cdpid)[64] = this->clientDigProductId;
                LOG(LOG_INFO, "cs_core::clientDigProductId=["
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                cdpid[0x00], cdpid[0x01], cdpid[0x02], cdpid[0x03],
                cdpid[0x04], cdpid[0x05], cdpid[0x06], cdpid[0x07],
                cdpid[0x08], cdpid[0x09], cdpid[0x0A], cdpid[0x0B],
                cdpid[0x0C], cdpid[0x0D], cdpid[0x0E], cdpid[0x0F],

                cdpid[0x10], cdpid[0x11], cdpid[0x12], cdpid[0x13],
                cdpid[0x14], cdpid[0x15], cdpid[0x16], cdpid[0x17],
                cdpid[0x18], cdpid[0x19], cdpid[0x1A], cdpid[0x1B],
                cdpid[0x1C], cdpid[0x1D], cdpid[0x1E], cdpid[0x1F],

                cdpid[0x20], cdpid[0x21], cdpid[0x22], cdpid[0x23],
                cdpid[0x24], cdpid[0x25], cdpid[0x26], cdpid[0x27],
                cdpid[0x28], cdpid[0x29], cdpid[0x2A], cdpid[0x2B],
                cdpid[0x2C], cdpid[0x2D], cdpid[0x2E], cdpid[0x2F],

                cdpid[0x30], cdpid[0x31], cdpid[0x32], cdpid[0x33],
                cdpid[0x34], cdpid[0x35], cdpid[0x36], cdpid[0x37],
                cdpid[0x38], cdpid[0x39], cdpid[0x3A], cdpid[0x3B],
                cdpid[0x3C], cdpid[0x3D], cdpid[0x3E], cdpid[0x3F]
                );
                if (this->length < 211) { return; }
                LOG(LOG_INFO, "cs_core::connectionType  = %u", this->connectionType);
                if (this->length < 212) { return; }
                LOG(LOG_INFO, "cs_core::pad1octet  = %u", this->pad1octet);
                if (this->length < 216) { return; }
                LOG(LOG_INFO, "cs_core::serverSelectedProtocol = %u", this->serverSelectedProtocol);
            }
        };


        // 2.2.1.3.5 Client Cluster Data (TS_UD_CS_CLUSTER)
        // ------------------------------------------------
        // The TS_UD_CS_CLUSTER data block is sent by the client to the server either to advertise that it can
        // support the Server Redirection PDUs (sections 2.2.13.2 and 2.2.13.3) or to request a connection to
        // a given session identifier.

        // header (4 bytes): GCC user data block header, as specified in User Data
        //                   Header (section 2.2.1.3.1). The User Data Header type
        //                   field MUST be set to CS_CLUSTER (0xC004).

        // Flags (4 bytes): A 32-bit, unsigned integer. Cluster information flags.

        //           Flag                            Meaning
        // -------------------------------------------------------------------------
        // REDIRECTION_SUPPORTED               The client can receive server session
        //       0x00000001                    redirection packets. If this flag is
        //                                     set, the
        //                                     ServerSessionRedirectionVersionMask
        //                                     MUST contain the server session
        //                                     redirection version that the client
        //                                     supports.
        // -------------------------------------------------------------------------
        // ServerSessionRedirectionVersionMask The server session redirection
        //       0x0000003C                    version that the client supports.
        //                                     See the discussion which follows
        //                                     this table for more information.
        // -------------------------------------------------------------------------
        // REDIRECTED_SESSIONID_FIELD_VALID    The RedirectedSessionID field
        //       0x00000002                    contains an ID that identifies a
        //                                     session on the server to associate
        //                                     with the connection.
        // -------------------------------------------------------------------------
        // REDIRECTED_SMARTCARD                The client logged on with a smart
        //       0x00000040                    card.
        // -------------------------------------------------------------------------

        // The ServerSessionRedirectionVersionMask is a 4-bit enumerated value
        // containing the server session redirection version supported by the
        // client. The following are possible version values.

        //          Value                              Meaning
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION1                If REDIRECTION_SUPPORTED is set,
        //          0x00                       server session redirection version 1
        //    (Flag = 0x00)                    is supported by the client.
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION2                If REDIRECTION_SUPPORTED is set,
        //          0x01                       server session redirection version 2
        //    (Flag = 0x04)                    is supported by the client.
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION3                If REDIRECTION_SUPPORTED is set,
        //          0x02                       server session redirection version 3
        //    (Flag = 0x08)                    is supported by the client.
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION4                If REDIRECTION_SUPPORTED is set,
        //          0x03                       server session redirection version 4
        //    (Flag = 0x0C)                    is supported by the client.
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION5                If REDIRECTION_SUPPORTED is set,
        //          0x04                       server session redirection version 5
        //    (Flag = 0x10)                    is supported by the client.
        // -------------------------------------------------------------------------
        // REDIRECTION_VERSION6                If REDIRECTION_SUPPORTED is set,
        //          0x05                       server session redirection version 6
        //    (Flag = 0x20)                    is supported by the client.
        // -------------------------------------------------------------------------

        // The version values cannot be combined; only one value MUST be specified
        // if the REDIRECTED_SESSIONID_FIELD_VALID (0x00000002) flag is present in
        // the Flags field.

        // RedirectedSessionID (4 bytes): A 32-bit unsigned integer. If the
        //                                REDIRECTED_SESSIONID_FIELD_VALID flag is
        //                                set in the Flags field, then the
        //                                RedirectedSessionID field contains a valid
        //                                session identifier to which the client
        //                                requests to connect.


        struct CSCluster {
            uint16_t userDataType;
            uint16_t length;

            enum {
                REDIRECTION_SUPPORTED            = 0x00000001,
                REDIRECTED_SESSIONID_FIELD_VALID = 0x00000002,
                REDIRECTED_SMARTCARD             = 0x00000040,
                ServerSessionRedirectionVersionMask = 0x0000003C,
            };
            uint32_t flags;
            uint32_t redirectedSessionID;

            CSCluster()
            : userDataType(CS_CLUSTER)
            , length(12)
            , flags(0)
            , redirectedSessionID(0)
            {
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->flags);
                stream.out_uint32_le(this->redirectedSessionID);
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->flags = stream.in_uint32_le();
                this->redirectedSessionID = stream.in_uint32_le();
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data CS_CLUSTER (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "cs_cluster::flags [%04x]", this->flags);
                if (this->flags == REDIRECTION_SUPPORTED){
                    LOG(LOG_INFO, "cs_cluster::flags::REDIRECTION_SUPPORTED");
                    LOG(LOG_INFO, "cs_cluster::flags::redirectionVersion = %u",
                        (this->flags & ServerSessionRedirectionVersionMask) >> 2);
                }
                if (this->flags == REDIRECTED_SESSIONID_FIELD_VALID){
                    LOG(LOG_INFO, "cs_cluster::flags::REDIRECTED_SESSIONID_FIELD_VALID");
                }
                if (this->flags == REDIRECTED_SMARTCARD){
                    LOG(LOG_INFO, "cs_cluster::flags::REDIRECTED_SMARTCARD");
                }
                if (this->length < 12) { return; }
                LOG(LOG_INFO, "cs_cluster::redirectedSessionID = %u", this->redirectedSessionID);
            }
        };

        // 2.2.1.3.6 Client Monitor Data (TS_UD_CS_MONITOR)
        // ------------------------------------------------
        // The TS_UD_CS_MONITOR packet describes the client-side display monitor
        // layout. This packet is an Extended Client Data Block and MUST NOT be sent
        // to a server which does not advertise support for Extended Client Data
        // Blocks by using the EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as
        // described in section 2.2.1.2.1.

        // header (4 bytes): GCC user data block header, as specified in User Data
        //                   Header (section 2.2.1.3.1). The User Data Header type
        //                   field MUST be set to CS_MONITOR (0xC005).

        // flags (4 bytes): A 32-bit, unsigned integer. This field is unused and
        //                  reserved for future use.

        // monitorCount (4 bytes): A 32-bit, unsigned integer. The number of display
        //                         monitor definitions in the monitorDefArray field
        //                        (the maximum allowed is 16).

        // monitorDefArray (variable): A variable-length array containing a series
        //                             of TS_MONITOR_DEF structures (section
        //                             2.2.1.3.6.1) which describe the display
        //                             monitor layout of the client. The number of
        //                             TS_MONITOR_DEF structures is given by the
        //                             monitorCount field.


        // 2.2.1.3.6.1 Monitor Definition (TS_MONITOR_DEF)
        // -----------------------------------------------
        // The TS_MONITOR_DEF packet describes the configuration of a client-side
        // display monitor. The x and y coordinates used to describe the monitor
        // position MUST be relative to the upper-left corner of the monitor
        // designated as the "primary display monitor" (the upper-left corner of the
        // primary monitor is always (0, 0)).

        // left (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate of
        //                 the upper-left corner of the display monitor.

        // top (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate of
        //                the upper-left corner of the display monitor.

        // right (4 bytes): A 32-bit, unsigned integer. Specifies the x-coordinate
        //                  of the lower-right corner of the display monitor.

        // bottom (4 bytes): A 32-bit, unsigned integer. Specifies the y-coordinate
        //                   of the lower-right corner of the display monitor.

        // flags (4 bytes): A 32-bit, unsigned integer. Monitor configuration flags.

        //        Value                          Meaning
        // -------------------------------------------------------------------------
        // TS_MONITOR_PRIMARY            The top, left, right and bottom fields
        //      0x00000001               describe the position of the primary
        //                               monitor.
        // -------------------------------------------------------------------------

        struct CSMonitor {
            uint16_t userDataType;
            uint16_t length;
            uint32_t left;
            uint32_t top;
            uint32_t right;
            uint32_t bottom;
            enum {
                TS_MONITOR_PRIMARY = 0x00000001,
            };

            uint32_t flags;

            CSMonitor()
            : userDataType(CS_MONITOR)
            , length(12) // default: everything except serverSelectedProtocol
            , left(0)
            , top(0)
            , right(0)
            , bottom(0)
            , flags(0)
            {
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->left);
                stream.out_uint32_le(this->top);
                stream.out_uint32_le(this->right);
                stream.out_uint32_le(this->bottom);
                stream.out_uint32_le(this->flags);
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->left   = stream.in_uint32_le();
                this->top    = stream.in_uint32_le();
                this->right  = stream.in_uint32_le();
                this->bottom = stream.in_uint32_le();
                this->flags  = stream.in_uint32_le();
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data CS_MONITOR (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "cs_monitor::left   = %u", this->left);
                LOG(LOG_INFO, "cs_monitor::top    = %u", this->top);
                LOG(LOG_INFO, "cs_monitor::right  = %u", this->right);
                LOG(LOG_INFO, "cs_monitor::bottom = %u", this->bottom);
                LOG(LOG_INFO, "cs_monitor::flags [%04X]", this->flags);
                if (this->flags & TS_MONITOR_PRIMARY){
                    LOG(LOG_INFO, "cs_monitor::flags::TS_MONITOR_PRIMARY");
                }
            }
        };

        // 2.2.1.3.4 Client Network Data (TS_UD_CS_NET)
        // --------------------------------------------
        // The TS_UD_CS_NET packet contains a list of requested virtual channels.

        // header (4 bytes): A 32-bit, unsigned integer. GCC user data block header,
        //                   as specified in User Data Header (section 2.2.1.3.1).
        //                   The User Data Header type field MUST be set to CS_NET
        //                   (0xC003).

        // channelCount (4 bytes): A 32-bit, unsigned integer. The number of
        //                         requested static virtual channels (the maximum
        //                         allowed is 31).

        // channelDefArray (variable): A variable-length array containing the
        //                             information for requested static virtual
        //                             channels encapsulated in CHANNEL_DEF
        //                             structures (section 2.2.1.3.4.1). The number
        //                             of CHANNEL_DEF structures which follows is
        //                             given by the channelCount field.

        // 2.2.1.3.4.1 Channel Definition Structure (CHANNEL_DEF)
        // ------------------------------------------------------
        // The CHANNEL_DEF packet contains information for a particular static
        // virtual channel.

        // name (8 bytes): An 8-byte array containing a null-terminated collection
        //                 of seven ANSI characters that uniquely identify the
        //                 channel.

        // options (4 bytes): A 32-bit, unsigned integer. Channel option flags.
        //
        //           Flag                             Meaning
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_INITIALIZED   Absence of this flag indicates that this
        //        0x80000000            channel is a placeholder and that the
        //                              server MUST NOT set it up.
        // ------------------------------------------------------------------------
        // CHANNEL_OPTION_ENCRYPT_RDP   This flag is unused and its value MUST be
        //        0x40000000            ignored by the server.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_ENCRYPT_SC    This flag is unused and its value MUST be
        //        0x20000000            ignored by the server.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_ENCRYPT_CS    This flag is unused and its value MUST be
        //        0x10000000            ignored by the server.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_PRI_HIGH      Channel data MUST be sent with high MCS
        //        0x08000000            priority.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_PRI_MED       Channel data MUST be sent with medium
        //        0x04000000            MCS priority.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_PRI_LOW       Channel data MUST be sent with low MCS
        //        0x02000000            priority.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_COMPRESS_RDP  Virtual channel data MUST be compressed
        //        0x00800000            if RDP data is being compressed.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_COMPRESS      Virtual channel data MUST be compressed,
        //        0x00400000            regardless of RDP compression settings.
        // -------------------------------------------------------------------------
        // CHANNEL_OPTION_SHOW_PROTOCOL The value of this flag MUST be ignored by
        //        0x00200000            the server. The visibility of the Channel
        //                              PDU Header (section 2.2.6.1.1) is
        //                              determined by the CHANNEL_FLAG_SHOW_PROTOCOL
        //                              (0x00000010) flag as defined in the flags
        //                              field (section 2.2.6.1.1).
        // -------------------------------------------------------------------------
        //REMOTE_CONTROL_PERSISTENT     Channel MUST be persistent across remote
        //       0x00100000             control transactions.

        struct CSNet {
            uint16_t userDataType;
            uint16_t length;
            uint32_t channelCount;

            enum {
                CHANNEL_OPTION_INITIALIZED   = 0x80000000,
                CHANNEL_OPTION_ENCRYPT_RDP   = 0x40000000,
                CHANNEL_OPTION_ENCRYPT_SC    = 0x20000000,
                CHANNEL_OPTION_ENCRYPT_CS    = 0x10000000,
                CHANNEL_OPTION_PRI_HIGH      = 0x08000000,
                CHANNEL_OPTION_PRI_MED       = 0x04000000,
                CHANNEL_OPTION_PRI_LOW       = 0x02000000,
                CHANNEL_OPTION_COMPRESS_RDP  = 0x00800000,
                CHANNEL_OPTION_COMPRESS      = 0x00400000,
                CHANNEL_OPTION_SHOW_PROTOCOL = 0x00200000,
                REMOTE_CONTROL_PERSISTENT    = 0x00100000,
            };

            struct {
                char name[8];
                uint32_t options;
            } channelDefArray[32];

            CSNet()
            : userDataType(CS_NET)
            , length(12)
            , channelCount(0)
            {
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                this->length = this->channelCount * 12 + 8;
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->channelCount);
                for (size_t i = 0; i < this->channelCount ; i++){
                    stream.out_copy_bytes(this->channelDefArray[i].name, 8);
                    stream.out_uint32_le(this->channelDefArray[i].options);
                }
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->channelCount = stream.in_uint32_le();
                for (size_t i = 0; i < this->channelCount ; i++){
                    stream.in_copy_bytes(this->channelDefArray[i].name, 8);
                    this->channelDefArray[i].options = stream.in_uint32_le();
                }
            }

            void log(const char * msg)
            {
                LOG(LOG_INFO, "%s GCC User Data CS_NET (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "cs_net::channelCount   = %u", this->channelCount);

                for (size_t i = 0; i < this->channelCount ; i++){
                    uint32_t options = channelDefArray[i].options;
                    LOG(LOG_INFO, "cs_net::channel '%*s' [%u]%s%s%s%s%s%s%s%s"
                        , 8
                        , channelDefArray[i].name, GCC::MCS_GLOBAL_CHANNEL + i + 1
                        , (options & CHANNEL_OPTION_INITIALIZED)?" INITIALIZED":""
                        , (options & CHANNEL_OPTION_PRI_HIGH)?" PRI_HIGH":""
                        , (options & CHANNEL_OPTION_PRI_MED)?" PRI_MED":""
                        , (options & CHANNEL_OPTION_PRI_LOW)?" PRI_LOW":""
                        , (options & CHANNEL_OPTION_COMPRESS_RDP)?" COMPRESS_RDP":""
                        , (options & CHANNEL_OPTION_COMPRESS)?" COMPRESS":""
                        , (options & REMOTE_CONTROL_PERSISTENT)?" PERSISTENT":""
                        , (options & CHANNEL_OPTION_SHOW_PROTOCOL)?" SHOW_PROTOCOL":""
                        );
                }
            }
        };

        // 2.2.1.4.4 Server Network Data (TS_UD_SC_NET)
        // ============================================
        // The TS_UD_SC_NET data block is a reply to the static virtual channel list
        // presented in the Client Network Data structure (section 2.2.1.3.4).

        // header (4 bytes): A GCC user data block header, as specified in section User
        //  Data Header (section 2.2.1.3.1). The User Data Header type field MUST be set
        //  to SC_NET (0x0C03).

        // MCSChannelId (2 bytes): A 16-bit, unsigned integer. The MCS channel
        //  identifier which the client MUST join to receive display data and send
        //  client input (I/O channel).

        // channelCount (2 bytes): A 16-bit, unsigned integer. The number of 16-bit,
        //  unsigned integer MCS channel IDs in the channelIdArray field.

        // channelIdArray (variable): A variable-length array of MCS channel IDs (each
        //  channel ID is a 16-bit, unsigned integer) which have been allocated (the
        //  number is given by the channelCount field). Each MCS channel ID corresponds
        //  in position to the channels requested in the Client Network Data structure.
        //  A channel value of 0 indicates that the channel was not allocated.

        // Pad (2 bytes): A 16-bit, unsigned integer. Optional padding. Values in this
        //  field MUST be ignored. The size in bytes of the Server Network Data
        //  structure MUST be a multiple of 4. If the channelCount field contains an odd
        //  value, then the size of the channelIdArray (and by implication the entire
        //  Server Network Data structure) will not be a multiple of 4. In this
        //  scenario, the Pad field MUST be present and it is used to add an additional
        //  2 bytes to the size of the Server Network Data structure. If the
        //  channelCount field contains an even value, then the Pad field is not
        //  required and MUST NOT be present.

        //03 0c 10 00 -> TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes

        //eb 03 -> TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
        //03 00 -> TS_UD_SC_NET::channelCount = 3
        //ec 03 -> channel0 = 0x3ec = 1004 (rdpdr)
        //ed 03 -> channel1 = 0x3ed = 1005 (cliprdr)
        //ee 03 -> channel2 = 0x3ee = 1006 (rdpsnd)
        //00 00 -> padding

        struct SCNet {
            uint16_t userDataType;
            uint16_t length;
            uint16_t MCSChannelId;
            uint16_t channelCount;
            struct {
                uint16_t id;
            } channelDefArray[32];

            SCNet()
            : userDataType(SC_NET)
            , length(12)
            , MCSChannelId(GCC::MCS_GLOBAL_CHANNEL)
            , channelCount(0)
            {
            }

            void emit(Stream & stream)
            {
                this->length = 8 + 4 * ((this->channelCount+1) >> 1);
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint16_le(this->MCSChannelId);
                stream.out_uint16_le(this->channelCount);
                for (size_t i = 0; i < this->channelCount ; i++){
                    stream.out_uint16_le(this->channelDefArray[i].id);
                }
                if (this->channelCount & 1){
                    stream.out_uint16_le(0);
                }
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->MCSChannelId = stream.in_uint16_le();
                this->channelCount = stream.in_uint16_le();
                for (size_t i = 0; i < this->channelCount ; i++){
                    this->channelDefArray[i].id = stream.in_uint16_le();
                }
                if (this->channelCount & 1){
                    stream.in_skip_bytes(2);
                }
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                this->length = 8 + 4 * ((this->channelCount+1) >> 1);
                LOG(LOG_INFO, "%s GCC User Data SC_NET (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "sc_net::MCSChannelId   = %u", this->MCSChannelId);
                LOG(LOG_INFO, "sc_net::channelCount   = %u", this->channelCount);

                for (size_t i = 0; i < this->channelCount ; i++){
                    LOG(LOG_INFO, "sc_net::channel[%u]::id = %u"
                        , GCC::MCS_GLOBAL_CHANNEL + i + 1
                        , this->channelDefArray[i].id
                        );
                }
                if (this->channelCount & 1){
                    LOG(LOG_INFO, "sc_net::padding 2 bytes 0000");
                }
            }
        };

        // 2.2.1.4.3 Server Security Data (TS_UD_SC_SEC1)
        // ==============================================

        // The TS_UD_SC_SEC1 data block returns negotiated security-related information
        // to the client. See section 5.3.2 for a detailed discussion of how this
        // information is used.

        // header (4 bytes): GCC user data block header, as specified in User Data
        // Header (section 2.2.1.3.1). The User Data Header type field MUST be set to
        // SC_SECURITY (0x0C02).

        // encryptionMethod (4 bytes): A 32-bit, unsigned integer. The selected
        // cryptographic method to use for the session. When Enhanced RDP Security
        // (section 5.4) is being used, this field MUST be set to ENCRYPTION_METHOD_NONE
        // (0).

        // +-------------------------------------+-------------------------------------+
        // | 0x00000000 ENCRYPTION_METHOD_NONE   | No encryption or Message            |
        // |                                     | Authentication Codes (MACs) will be |
        // |                                     | used.                               |
        // +-------------------------------------+-------------------------------------+
        // | 0x00000001 ENCRYPTION_METHOD_40BIT  | 40-bit session keys will be used to |
        // |                                     | encrypt data (with RC4) and generate|
        // |                                     | MACs.                               |
        // +-------------------------------------+-------------------------------------+
        // | 0x00000002 ENCRYPTION_METHOD_128BIT | 128-bit session keys will be used   |
        // |                                     | to encrypt data (with RC4) and      |
        // |                                     | generate MACs.                      |
        // +-------------------------------------+-------------------------------------+
        // | 0x00000008 ENCRYPTION_METHOD_56BIT  | 56-bit session keys will be used to |
        // |                                     | encrypt data (with RC4) and generate|
        // |                                     | MACs.                               |
        // +-------------------------------------+-------------------------------------+
        // | 0x00000010 ENCRYPTION_METHOD_FIPS   | All encryption and Message          |
        // |                                     | Authentication Code                 |
        // |                                     | generation routines will            |
        // |                                     | be FIPS 140-1 compliant.            |
        // +-------------------------------------+-------------------------------------+

        // encryptionLevel (4 bytes): A 32-bit unsigned integer. It describes the
        //  encryption behavior to use for the session. When Enhanced RDP Security
        //  (section 5.4) is being used, this field MUST be set to ENCRYPTION_LEVEL_NONE
        //  (0).

        // +------------------------------------+------------+
        // | ENCRYPTION_LEVEL_NONE              | 0x00000000 |
        // +------------------------------------+------------+
        // | ENCRYPTION_LEVEL_LOW               | 0x00000001 |
        // +------------------------------------+------------+
        // | ENCRYPTION_LEVEL_CLIENT_COMPATIBLE | 0x00000002 |
        // +------------------------------------+------------+
        // | ENCRYPTION_LEVEL_HIGH              | 0x00000003 |
        // +------------------------------------+------------+
        // | ENCRYPTION_LEVEL_FIPS              | 0x00000004 |
        // +------------------------------------+------------+

        // See section 5.3.1 for a description of each of the low, client-compatible,
        // high, and FIPS encryption levels.

        // serverRandomLen (4 bytes): An optional 32-bit, unsigned integer that specifies
        // the size in bytes of the serverRandom field. If the encryptionMethod and
        // encryptionLevel fields are both set to zero, then this field MUST NOT be present
        // and the length of the serverRandom field MUST be zero. If either the
        // encryptionMethod or encryptionLevel field is non-zero, this field MUST be set
        // to 0x00000020 (32 bytes serverRandom).

        // serverCertLen (4 bytes): An optional 32-bit, unsigned integer that specifies
        // the size in bytes of the serverCertificate field. If the encryptionMethod and
        // encryptionLevel fields are both set to zero, then this field MUST NOT be present
        // and the length of the serverCertificate field MUST be zero.

        // serverRandom (variable): The variable-length server random value used to
        // derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
        // given by the serverRandomLen field. If the encryptionMethod and
        // encryptionLevel fields are both set to 0 then this field MUST NOT be present.

        // serverCertificate (variable): The variable-length certificate containing the
        //  server's public key information. The length in bytes is given by the
        // serverCertLen field. If the encryptionMethod and encryptionLevel fields are
        // both set to 0 then this field MUST NOT be present.

        // 2.2.1.4.3.1 Server Certificate (SERVER_CERTIFICATE)
        // ===================================================

        // The SERVER_CERTIFICATE structure describes the generic server certificate
        // structure to which all server certificates present in the Server Security
        // Data (section 2.2.1.4.3) conform.

        // dwVersion (4 bytes): A 32-bit, unsigned integer.
        // dwVersion::certChainVersion (31 bits): A 31-bit field. The certificate version.

        // +---------------------------------+-----------------------------------------+
        // | 0x00000001 CERT_CHAIN_VERSION_1 | The certificate contained in the        |
        // |                                 | certData field is a Server Proprietary  |
        // |                                 | Certificate (section 2.2.1.4.3.1.1).    |
        // +---------------------------------+-----------------------------------------+
        // | 0x00000002 CERT_CHAIN_VERSION_2 | The certificate contained in the        |
        // |                                 | certData field is an X.509 Certificate  |
        // |                                 | (see section 5.3.3.2).                  |
        // +---------------------------------+-----------------------------------------+

        // dwVersion::t (1 bit): A 1-bit field. Indicates whether the certificate contained in the
        //  certData field has been permanently or temporarily issued to the server.

        // +---+----------------------------------------------------------------------+
        // | 0 | The certificate has been permanently issued to the server.           |
        // +---+----------------------------+-----------------------------------------+
        // | 1 | The certificate has been temporarily issued to the server.           |
        // +---+----------------------------+-----------------------------------------+

        // certData (variable): Certificate data. The format of this certificate data is
        //  determined by the dwVersion field.

        // 2.2.1.4.3.1.1 Server Proprietary Certificate (PROPRIETARYSERVERCERTIFICATE)
        // ===========================================================================

        // The PROPRIETARYSERVERCERTIFICATE structure describes a signed certificate
        // containing the server's public key and conforming to the structure of a
        // Server Certificate (section 2.2.1.4.3.1). For a detailed description of
        // Proprietary Certificates, see section 5.3.3.1.

        // dwVersion (4 bytes): A 32-bit, unsigned integer. The certificate version
        //  number. This field MUST be set to CERT_CHAIN_VERSION_1 (0x00000001).

        // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
        //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).

        // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
        //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).

        // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
        //  (0x0006).

        // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the PublicKeyBlob field.

        // PublicKeyBlob (variable): Variable-length server public key bytes, formatted
        //  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
        //  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
        //  field.

        // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
        //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
        //  (0x0008).

        // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
        //  of the SignatureKeyBlob field.

        // SignatureBlob (variable): Variable-length signature of the certificate
        // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
        // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.

        // 2.2.1.4.3.1.1.1 RSA Public Key (RSA_PUBLIC_KEY)
        // ===============================================
        // The structure used to describe a public key in a Proprietary Certificate
        // (section 2.2.1.4.3.1.1).

        // magic (4 bytes): A 32-bit, unsigned integer. The sentinel value. This field
        //  MUST be set to 0x31415352.

        // keylen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
        //  modulus field. This value is directly related to the bitlen field and MUST
        //  be ((bitlen / 8) + 8) bytes.

        // bitlen (4 bytes): A 32-bit, unsigned integer. The number of bits in the
        //  public key modulus.

        // datalen (4 bytes): A 32-bit, unsigned integer. The maximum number of bytes
        //  that can be encoded using the public key.

        // pubExp (4 bytes): A 32-bit, unsigned integer. The public exponent of the
        //  public key.

        // modulus (variable): A variable-length array of bytes containing the public
        //  key modulus. The length in bytes of this field is given by the keylen field.
        //  The modulus field contains all (bitlen / 8) bytes of the public key modulus
        //  and 8 bytes of zero padding (which MUST follow after the modulus bytes).

        // 5.3.3.1.1 Terminal Services Signing Key
        // =======================================
        // The modulus, private exponent, and public exponent of the 512-bit Terminal Services asymmetric
        // key used for signing Proprietary Certificates with the RSA algorithm are detailed as follows.

        // 64 byte Modulus (n)

        //        0x3d, 0x3a, 0x5e, 0xbd, 0x72, 0x43, 0x3e, 0xc9,
        //        0x4d, 0xbb, 0xc1, 0x1e, 0x4a, 0xba, 0x5f, 0xcb,
        //        0x3e, 0x88, 0x20, 0x87, 0xef, 0xf5, 0xc1, 0xe2,
        //        0xd7, 0xb7, 0x6b, 0x9a, 0xf2, 0x52, 0x45, 0x95,
        //        0xce, 0x63, 0x65, 0x6b, 0x58, 0x3a, 0xfe, 0xef,
        //        0x7c, 0xe7, 0xbf, 0xfe, 0x3d, 0xf6, 0x5c, 0x7d,
        //        0x6c, 0x5e, 0x06, 0x09, 0x1a, 0xf5, 0x61, 0xbb,
        //        0x20, 0x93, 0x09, 0x5f, 0x05, 0x6d, 0xea, 0x87

        // 64-byte Private Exponent (d) :

        //        0x87, 0xa7, 0x19, 0x32, 0xda, 0x11, 0x87, 0x55,
        //        0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xf8,
        //        0x24, 0x3e, 0xe6, 0xfa, 0xe9, 0x67, 0x49, 0x94,
        //        0xcf, 0x92, 0xcc, 0x33, 0x99, 0xe8, 0x08, 0x60,
        //        0x17, 0x9a, 0x12, 0x9f, 0x24, 0xdd, 0xb1, 0x24,
        //        0x99, 0xc7, 0x3a, 0xb8, 0x0a, 0x7b, 0x0d, 0xdd,
        //        0x35, 0x07, 0x79, 0x17, 0x0b, 0x51, 0x9b, 0xb3,
        //        0xc7, 0x10, 0x01, 0x13, 0xe7, 0x3f, 0xf3, 0x5f

        //  4-byte Public Exponent (e):

        //        0x5b, 0x7b, 0x88, 0xc0

        /// The enumerated integers are in little-endian byte order. The public
        // key is the pair (e, n), while the private key is the pair (d, n)

        // 5.3.3.1.2 Signing a Proprietary Certificate
        // ========================================

        // The Proprietary Certificate is signed by using RSA to encrypt the hash
        // of the first six fields with the Terminal Services private signing key
        // (specified in section 5.3.3.1.1) and then appending the result to the end
        // of the certificate. Mathematically the signing operation is formulated as follows:

        //    s = m^d mod n

        // Where

        //    s = signature
        //    m = hash of first six fields of certificate
        //    d = private exponent
        //    n = modulus

        // The structure of the Proprietary Certificate is detailed in section 2.2.1.4.3.1.1.
        // The structure of the public key embedded in the certificate is described in 2.2.1.4.3.1.1.1.
        // An example of public key bytes (in little-endian format) follows.

        // 0x52 0x53 0x41 0x31: magic (0x31415352)
        // 0x48 0x00 0x00 0x00: keylen (72 bytes)
        // 0x00 0x02 0x00 0x00: bitlen (512 bits)
        // 0x3f 0x00 0x00 0x00: datalen (63 bytes)
        // 0x01 0x00 0x01 0x00: pubExp (0x00010001)

        // 0xaf 0xfe 0x36 0xf2 0xc5 0xa1 0x44 0x2e
        // 0x47 0xc1 0x31 0xa7 0xdb 0xc6 0x67 0x02
        // 0x64 0x71 0x5c 0x00 0xc9 0xb6 0xb3 0x04
        // 0xd0 0x89 0x9f 0xe7 0x6b 0x24 0xe8 0xe8
        // 0xe5 0x2d 0x0b 0x13 0xa9 0x0c 0x6d 0x4d
        // 0x91 0x5e 0xe8 0xf6 0xb3 0x17 0x17 0xe3
        // 0x9f 0xc5 0x4d 0x4a 0xba 0xfa 0xb9 0x2a
        // 0x1b 0xfb 0x10 0xdd 0x91 0x8c 0x60 0xb7: modulus

        // A 128-bit MD5 hash over the first six fields of the proprietary certificate
        // (which are all in little-endian format) appears as follows.

        //      PublicKeyBlob = wBlobType + wBlobLen + PublicKeyBytes
        //      hash = MD5(dwVersion + dwSigAlgID + dwKeyAlgID + PublicKeyBlob)

        // Because the Terminal Services private signing key has a 64-byte modulus, the
        // maximum number of bytes that can be encoded by using the key is 63 (the size
        // of the modulus, in bytes, minus 1). An array of 63 bytes is created and
        //initialized as follows.

        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0x00 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0x01

        // The 128-bit MD5 hash is copied into the first 16 bytes of the array.
        // For example, assume that the generated hash is as follows.

        // 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
        // 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f

        // The byte array will appear as follows after copying in the 16 bytes of the MD5 hash.

        // 0xf5 0xcc 0x18 0xee 0x45 0xe9 0x4d 0xa6
        // 0x79 0x02 0xca 0x76 0x51 0x33 0xe1 0x7f
        // 0x00 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff
        // 0xff 0xff 0xff 0xff 0xff 0xff 0x01

        // The 63-byte array is then treated as an unsigned little-endian integer and signed
        // with the Terminal Services private key by using RSA. The resultant signature must
        // be in little-endian format before appending it to the Proprietary Certificate
        // structure. The final structure of the certificate must conform to the specification
        // in section 2.2.1.4.3.1.1. This means that fields 7 through to 9 will be the signature
        // BLOB type, the number of bytes in the signature and the actual signature bytes respectively.
        // The BLOB type and number of bytes must be in little-endian format.

        // Example Java source code that shows how to use a private 64-byte asymmetric key to sign an
        // array of 63 bytes using RSA is presented in section 4.9. The code also shows how to use the
        // associated public key to verify the signature.

        struct SCSecurity {
            uint16_t userDataType;
            uint16_t length;

            enum {
                ENCRYPTION_METHOD_NONE   = 0x00000000,
                ENCRYPTION_METHOD_40BIT  = 0x00000001,
                ENCRYPTION_METHOD_128BIT = 0x00000002,
                ENCRYPTION_METHOD_56BIT  = 0x00000008,
                ENCRYPTION_METHOD_FIPS   = 0x00000010,
            };
            uint32_t encryptionMethod;

            enum {
                ENCRYPTION_LEVEL_NONE              = 0x00000000,
                ENCRYPTION_LEVEL_LOW               = 0x00000001,
                ENCRYPTION_LEVEL_CLIENT_COMPATIBLE = 0x00000002,
                ENCRYPTION_LEVEL_HIGH              = 0x00000003,
                ENCRYPTION_LEVEL_FIPS              = 0x00000004,
            };
            uint32_t encryptionLevel;
            uint32_t serverRandomLen;
            uint8_t serverRandom[32];

            uint32_t serverCertLen;

            uint8_t pri_exp[64];
            uint8_t pub_sig[64];

            enum { CERT_CHAIN_VERSION_1 = 0x00000001
                 , CERT_CHAIN_VERSION_2 = 0x00000002
                 ,
            };

            enum { SIGNATURE_ALG_RSA = 1
                 , KEY_EXCHANGE_ALG_RSA = 1
                 , BB_RSA_KEY_BLOB = 0x0006
                 , BB_RSA_SIGNATURE_BLOB = 0x0008
                 , RSA_MAGIC     = 0x31415352, /* RSA1 */

            };

            // really proprietaryCertificate and X509Certificate should be some union (sum) controlled by dwVersion
            // anyway, it's not really usefull here to bother about small lost space
            // (real alternative would be to dynamically allocate memory, buth memory allocation also has it's costs)

            uint32_t dwVersion;
            bool temporary;

            struct ServerProprietaryCertificate {
                // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
                //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
                uint32_t dwSigAlgId;

                // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
                //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
                uint32_t dwKeyAlgId;

                // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
                //  (0x0006).
                uint16_t wPublicKeyBlobType;

                // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                //  of the PublicKeyBlob field.
                uint16_t wPublicKeyBlobLen;

                // PublicKeyBlob (variable): Variable-length server public key bytes, formatted
                //  using the Rivest-Shamir-Adleman (RSA) Public Key structure (section
                //  2.2.1.4.3.1.1.1). The length in bytes is given by the wPublicKeyBlobLen
                //  field.
                struct PublicKeyBlob {
                    // 2.2.1.4.3.1.1.1 RSA Public Key (RSA_PUBLIC_KEY)
                    // ===============================================
                    // The structure used to describe a public key in a Proprietary Certificate
                    // (section 2.2.1.4.3.1.1).

                    // magic (4 bytes): A 32-bit, unsigned integer. The sentinel value. This field
                    //  MUST be set to 0x31415352.
                    uint32_t magic;

                    // keylen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
                    //  modulus field. This value is directly related to the bitlen field and MUST
                    //  be ((bitlen / 8) + 8) bytes.
                    uint32_t keylen;

                    // bitlen (4 bytes): A 32-bit, unsigned integer. The number of bits in the
                    //  public key modulus.
                    uint32_t bitlen;

                    // datalen (4 bytes): A 32-bit, unsigned integer. The maximum number of bytes
                    //  that can be encoded using the public key.
                    // This value is directly related to the bitlen field and MUST be ((bitlen / 8) - 1) bytes.
                    uint32_t datalen;

                    // pubExp (4 bytes): A 32-bit, unsigned integer. The public exponent of the
                    //  public key.
                    uint8_t pubExp[4];

                    // modulus (variable): A variable-length array of bytes containing the public
                    //  key modulus. The length in bytes of this field is given by the keylen field.
                    //  The modulus field contains all (bitlen / 8) bytes of the public key modulus
                    //  and 8 bytes of zero padding (which MUST follow after the modulus bytes).
                    uint8_t modulus[72];

                    PublicKeyBlob()
                    : magic(RSA_MAGIC)
                    , keylen(72)
                    , bitlen(512)
                    , datalen(63)
                    {
                    }
                } RSAPK;

                // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
                //  (0x0008).
                uint16_t wSignatureBlobType;

                // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                //  of the SignatureKeyBlob field.
                uint16_t wSignatureBlobLen;

                // SignatureBlob (variable): Variable-length signature of the certificate
                // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
                // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
                uint8_t wSignatureBlob[72];

                ServerProprietaryCertificate()
                : dwSigAlgId(SIGNATURE_ALG_RSA)
                , dwKeyAlgId(KEY_EXCHANGE_ALG_RSA)
                , wPublicKeyBlobType(BB_RSA_KEY_BLOB)
                , wPublicKeyBlobLen(92)
                , wSignatureBlobType(BB_RSA_SIGNATURE_BLOB)
                , wSignatureBlobLen(72)
                {
                }
            } proprietaryCertificate;

            struct X509CertificateChain {
                uint32_t certCount;
                struct X509CertificateWithLen {
                    uint32_t len;
                    X509 * cert;
                } cert[32]; // a chain of at most 32 certificates, should be enough
            } x509;

            SCSecurity()
            : userDataType(SC_SECURITY)
            , length(236)
            , encryptionMethod(0)
            , encryptionLevel(0) // crypt level 0 = none, 1 = low 2 = medium, 3 = high
            , serverRandomLen(32)
            , serverCertLen(184)
            , dwVersion(CERT_CHAIN_VERSION_1)
            , temporary(false)
            {
                for (size_t i = 0 ; i < sizeof(this->x509.cert) / sizeof(this->x509.cert[0]) ; i++){
                    this->x509.cert[i].cert = NULL;
                }
            }

            ~SCSecurity(){
                for (size_t i = 0 ; i < sizeof(this->x509.cert) / sizeof(this->x509.cert[0]) ; i++){
                    if (this->x509.cert[i].cert){
                        X509_free(this->x509.cert[i].cert);
                        this->x509.cert[i].cert = NULL;
                    }
                }
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(SC_SECURITY);

                if ((this->encryptionMethod == 0) && (this->encryptionLevel == 0)){
                    this->length = 12;
                    this->serverRandomLen = 0;
                    this->encryptionLevel = 0;
                    stream.out_uint16_le(this->length); // length, including tag and length fields
                    stream.out_uint32_le(this->encryptionMethod); // encryptionMethod
                    stream.out_uint32_le(this->encryptionLevel); // encryptionLevel
                }
                else {
                    stream.out_uint16_le(this->length); // length, including tag and length fields
                    stream.out_uint32_le(this->encryptionMethod); // key len 1 = 40 bit 2 = 128 bit
                    stream.out_uint32_le(this->encryptionLevel);
                    stream.out_uint32_le(this->serverRandomLen);  // random len
                    stream.out_uint32_le(this->serverCertLen); // len of rsa info(certificate)
                    stream.out_copy_bytes(this->serverRandom, this->serverRandomLen);

                    // --------------------------------------------------------------
                    /* here to end is certificate */
                    /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\TermService\Parameters\Certificate */
                    stream.out_uint32_le(this->dwVersion|(this->temporary << 31));

                    if ((this->dwVersion & 0x7FFFFFFF) == CERT_CHAIN_VERSION_1){
                        stream.out_uint32_le(this->proprietaryCertificate.dwSigAlgId);
                        stream.out_uint32_le(this->proprietaryCertificate.dwKeyAlgId);

                        stream.out_uint16_le(this->proprietaryCertificate.wPublicKeyBlobType);
                        stream.out_uint16_le(this->proprietaryCertificate.wPublicKeyBlobLen);
                        stream.out_uint32_le(this->proprietaryCertificate.RSAPK.magic);
                        stream.out_uint32_le(this->proprietaryCertificate.RSAPK.keylen);
                        stream.out_uint32_le(this->proprietaryCertificate.RSAPK.bitlen);
                        stream.out_uint32_le(this->proprietaryCertificate.RSAPK.datalen);
                        stream.out_copy_bytes(this->proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                        stream.out_copy_bytes(this->proprietaryCertificate.RSAPK.modulus, SEC_MODULUS_SIZE);
                        stream.out_clear_bytes(SEC_PADDING_SIZE);

                        stream.out_uint16_le(this->proprietaryCertificate.wSignatureBlobType);
                        stream.out_uint16_le(this->proprietaryCertificate.wSignatureBlobLen); /* len */
                        stream.out_copy_bytes(this->proprietaryCertificate.wSignatureBlob, 64); /* pub sig */
                        stream.out_clear_bytes(8); /* pad */
                    }
                    else {
                        // send chain of certificates
                    }
                    /* end certificate */
                    // --------------------------------------------------------------
                }
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                TODO("check we are not reading outside stream. Create substream based on len to ensure that")

                this->userDataType = stream.in_uint16_le();
                this->length = stream.in_uint16_le();
                this->encryptionMethod = stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
                this->encryptionLevel = stream.in_uint32_le();  /* 1 = low, 2 = medium, 3 = high */

                if ((this->encryptionMethod == 0) && (this->encryptionLevel == 0)){
                    this->serverRandomLen = 0;
                    this->encryptionLevel = 0;
                }
                TODO("add sanity check if crypto is NONE and length not 12")
                if (this->length == 12) {
                    if ((this->encryptionLevel != 0)||(this->encryptionMethod != 0)){
                        LOG(LOG_ERR, "SC_SECURITY short header with encription method=%u level=%u",
                            this->encryptionMethod, this->encryptionLevel);
                        throw Error(ERR_GCC);
                    }
                    return;
                }

                if ((this->encryptionLevel == 0) || (encryptionMethod == 0)){
                    LOG(LOG_ERR, "SC_SECURITY encryption header but no encryption setted : method=%u level=%u",
                        this->encryptionMethod, this->encryptionLevel);
                    throw Error(ERR_GCC);
                }

                // serverRandomLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of
                // the serverRandom field. If the encryptionMethod and encryptionLevel fields
                // are both set to 0 then the contents of this field MUST be ignored and the
                // serverRandom field MUST NOT be present. Otherwise, this field MUST be set to
                // 32 bytes.
                this->serverRandomLen = stream.in_uint32_le();

                // serverCertLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
                //  serverCertificate field. If the encryptionMethod and encryptionLevel fields
                //  are both set to 0 then the contents of this field MUST be ignored and the
                // serverCertificate field MUST NOT be present.

                this->serverCertLen = stream.in_uint32_le();

                if (this->serverRandomLen != SEC_RANDOM_SIZE) {
                    LOG(LOG_ERR, "SCSecutity recv: serverRandomLen %d, expected %d",
                         this->serverRandomLen, SEC_RANDOM_SIZE);
                    throw Error(ERR_GCC);
                }

                if (!stream.in_check_rem(this->serverCertLen)) {
                    LOG(LOG_ERR, "SCSecutity recv: serverCertLen %d, not enough data available (%u)",
                         this->serverCertLen, stream.size() - stream.get_offset());
                    throw Error(ERR_GCC);
                }
                // serverRandom (variable): The variable-length server random value used to
                // derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
                // given by the serverRandomLen field. If the encryptionMethod and
                // encryptionLevel fields are both set to 0 then this field MUST NOT be present.

                stream.in_copy_bytes(this->serverRandom, this->serverRandomLen);
                uint32_t certType = stream.in_uint32_le();
                this->dwVersion = certType & 0x7FFFFFFF;
                this->temporary = 0 != (certType & 0x80000000);
                if (this->dwVersion == CERT_CHAIN_VERSION_1){
                    // dwSigAlgId (4 bytes): A 32-bit, unsigned integer. The signature algorithm
                    //  identifier. This field MUST be set to SIGNATURE_ALG_RSA (0x00000001).
                    this->proprietaryCertificate.dwSigAlgId = stream.in_uint32_le();

                    // dwKeyAlgId (4 bytes): A 32-bit, unsigned integer. The key algorithm
                    //  identifier. This field MUST be set to KEY_EXCHANGE_ALG_RSA (0x00000001).
                    this->proprietaryCertificate.dwKeyAlgId = stream.in_uint32_le();

                    // wPublicKeyBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                    //  in the PublicKeyBlob field. This field MUST be set to BB_RSA_KEY_BLOB
                    //  (0x0006).
                    this->proprietaryCertificate.wPublicKeyBlobType = stream.in_uint16_le();

                    // wPublicKeyBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                    //  of the PublicKeyBlob field.
                    this->proprietaryCertificate.wPublicKeyBlobLen = stream.in_uint16_le();

                    if (this->proprietaryCertificate.wPublicKeyBlobLen != 92){
                        LOG(LOG_ERR, "RSA Key blob len too large in certificate %u (expected 92)",
                            this->proprietaryCertificate.wPublicKeyBlobLen);
                        throw Error(ERR_GCC);
                    }

                    this->proprietaryCertificate.RSAPK.magic = stream.in_uint32_le();
                    if (this->proprietaryCertificate.RSAPK.magic != RSA_MAGIC) {
                            LOG(LOG_ERR, "Bad RSA magic 0x%x", this->proprietaryCertificate.RSAPK.magic);
                            throw Error(ERR_GCC);
                    }
                    this->proprietaryCertificate.RSAPK.keylen = stream.in_uint32_le();
                    if (this->proprietaryCertificate.RSAPK.keylen != 72){
                        LOG(LOG_WARNING, "Bad server public key len (%u bytes)", this->proprietaryCertificate.RSAPK.keylen);
                        throw Error(ERR_GCC);
                    }
                    this->proprietaryCertificate.RSAPK.bitlen = stream.in_uint32_le();
                    this->proprietaryCertificate.RSAPK.datalen = stream.in_uint32_le();
                    stream.in_copy_bytes(this->proprietaryCertificate.RSAPK.pubExp, SEC_EXPONENT_SIZE);
                    stream.in_copy_bytes(this->proprietaryCertificate.RSAPK.modulus, SEC_MODULUS_SIZE + SEC_PADDING_SIZE);


                    // wSignatureBlobType (2 bytes): A 16-bit, unsigned integer. The type of data
                    //  in the SignatureKeyBlob field. This field is set to BB_RSA_SIGNATURE_BLOB
                    //  (0x0008).
                    this->proprietaryCertificate.wSignatureBlobType = stream.in_uint16_le();
                    if (this->proprietaryCertificate.wSignatureBlobType != BB_RSA_SIGNATURE_BLOB){
                        LOG(LOG_ERR, "RSA Signature blob expected, got %x",
                            this->proprietaryCertificate.wSignatureBlobType);
                        throw Error(ERR_GCC);
                    }

                    // wSignatureBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
                    //  of the SignatureKeyBlob field.
                    this->proprietaryCertificate.wSignatureBlobLen = stream.in_uint16_le();

                    // SignatureBlob (variable): Variable-length signature of the certificate
                    // created with the Terminal Services Signing Key (see sections 5.3.3.1.1 and
                    // 5.3.3.1.2). The length in bytes is given by the wSignatureBlobLen field.
                    if (this->proprietaryCertificate.wSignatureBlobLen != 72){
                        LOG(LOG_ERR, "RSA Signature blob len too large in certificate %u (expected 72)",
                            this->proprietaryCertificate.wSignatureBlobLen);
                        throw Error(ERR_GCC);
                    }
                    stream.in_copy_bytes(this->proprietaryCertificate.wSignatureBlob, 64 + SEC_PADDING_SIZE);
                }
                else {
                    this->x509.certCount = stream.in_uint32_le();
                    for (size_t i = 0; i < this->x509.certCount ; i++){
                        this->x509.cert[i].len = stream.in_uint32_le();
                        this->x509.cert[i].cert = d2i_X509(NULL, const_cast<const uint8_t **>(&stream.p), this->x509.cert[i].len);
                    }
                    stream.in_skip_bytes(16); /* Padding */
                }
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data SC_SECURITY (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "sc_security::encryptionMethod = %u", this->encryptionMethod);
                LOG(LOG_INFO, "sc_security::encryptionLevel  = %u", this->encryptionLevel);
                if (this->length == 12) { return; }
                LOG(LOG_INFO, "sc_security::serverRandomLen  = %u", this->serverRandomLen);
                LOG(LOG_INFO, "sc_security::serverCertLen    = %u", this->serverCertLen);
                LOG(LOG_INFO, "sc_security::dwVersion = %x", this->dwVersion);
                LOG(LOG_INFO, "sc_security::temporary = %s", this->temporary?"true":"false");
                if (this->dwVersion == GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
                    LOG(LOG_INFO, "sc_security::RDP4-style encryption");
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::dwSigAlgId = %u", this->proprietaryCertificate.dwSigAlgId);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::dwKeyAlgId = %u", this->proprietaryCertificate.dwKeyAlgId);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::wPublicKeyBlobType = %u",
                         this->proprietaryCertificate.wPublicKeyBlobType);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::wPublicKeyBlobLen = %u",
                        this->proprietaryCertificate.wPublicKeyBlobLen);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::magic = %u",
                        this->proprietaryCertificate.RSAPK.magic);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::keylen = %u",
                        this->proprietaryCertificate.RSAPK.keylen);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::bitlen = %u",
                        this->proprietaryCertificate.RSAPK.bitlen);
                    LOG(LOG_INFO, "sc_security::proprietaryCertificate::RSAPK::datalen = %u",
                        this->proprietaryCertificate.RSAPK.datalen);
                }
                else {
                    LOG(LOG_INFO, "sc_security::RDP5-style encryption");
                }
            }
        };


        // 2.2.1.3.3 Client Security Data (TS_UD_CS_SEC)
        // ---------------------------------------------
        // The TS_UD_CS_SEC data block contains security-related information used to
        // advertise client cryptographic support. This information is only relevant
        // when Standard RDP Security mechanisms (section 5.3) will be used. See
        // sections 3 and 5.3.2 for a detailed discussion of how this information is
        // used.

        // header (4 bytes): GCC user data block header as described in User Data
        //                   Header (section 2.2.1.3.1). The User Data Header type
        //                   field MUST be set to CS_SECURITY (0xC002).

        // encryptionMethods (4 bytes): A 32-bit, unsigned integer. Cryptographic
        //                              encryption methods supported by the client
        //                              and used in conjunction with Standard RDP
        //                              Security The server MUST select one of these
        //                              methods. Section 5.3.2 describes how the
        //                              client and server negotiate the security
        //                              parameters for a given connection.
        //
        //           Value                           Meaning
        // -------------------------------------------------------------------------
        //    40BIT_ENCRYPTION_FLAG   40-bit session keys MUST be used to encrypt
        //       0x00000001           data (with RC4) and generate Message
        //                            Authentication Codes (MAC).
        // -------------------------------------------------------------------------
        //    128BIT_ENCRYPTION_FLAG  128-bit session keys MUST be used to encrypt
        //       0x00000002           data (with RC4) and generate MACs.
        // -------------------------------------------------------------------------
        //    56BIT_ENCRYPTION_FLAG   56-bit session keys MUST be used to encrypt
        //       0x00000008           data (with RC4) and generate MACs.
        // -------------------------------------------------------------------------
        //   FIPS_ENCRYPTION_FLAG All encryption and Message Authentication Code
        //                            generation routines MUST be Federal
        //       0x00000010           Information Processing Standard (FIPS) 140-1
        //                            compliant.

        // extEncryptionMethods (4 bytes): A 32-bit, unsigned integer. This field is
        //                               used exclusively for the French locale.
        //                               In French locale clients, encryptionMethods
        //                               MUST be set to 0 and extEncryptionMethods
        //                               MUST be set to the value to which
        //                               encryptionMethods would have been set.
        //                               For non-French locale clients, this field
        //                               MUST be set to 0

        struct CSSecurity {
            enum {
                  _40BIT_ENCRYPTION_FLAG = 0x01
                , _128BIT_ENCRYPTION_FLAG = 0x02
                , _56BIT_ENCRYPTION_FLAG = 0x08
                , FIPS_ENCRYPTION_FLAG = 0x10
            };

            uint16_t userDataType;
            uint16_t length;
            uint32_t encryptionMethods;
            uint32_t extEncryptionMethods;

            CSSecurity()
            : userDataType(CS_SECURITY)
            , length(12)
            , encryptionMethods(_40BIT_ENCRYPTION_FLAG | _128BIT_ENCRYPTION_FLAG)
            , extEncryptionMethods(0)
            {
            }

            void emit(Stream & stream)
            {
                stream.out_uint16_le(this->userDataType);
                stream.out_uint16_le(this->length);
                stream.out_uint32_le(this->encryptionMethods);
                stream.out_uint32_le(this->extEncryptionMethods);
                stream.mark_end();
            }

            void recv(Stream & stream)
            {
                this->userDataType         = stream.in_uint16_le();
                this->length               = stream.in_uint16_le();
                this->encryptionMethods    = stream.in_uint32_le();
                this->extEncryptionMethods = stream.in_uint32_le();
            }

            void log(const char * msg)
            {
                // --------------------- Base Fields ---------------------------------------
                LOG(LOG_INFO, "%s GCC User Data CS_SECURITY (%u bytes)", msg, this->length);
                LOG(LOG_INFO, "CSSecGccUserData::encryptionMethods %u", this->encryptionMethods);
                LOG(LOG_INFO, "CSSecGccUserData::extEncryptionMethods %u", this->extEncryptionMethods);

            }
        };


    }; /* namespace UserData */
}; /* namespace GCC */

#endif
