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

#if !defined(__CORE_RDP_GCC_HPP__)
#define __CORE_RDP_GCC_HPP__

#include "stream.hpp"

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

#include "gcc_conference_user_data/cs_core.hpp"
#include "gcc_conference_user_data/cs_cluster.hpp"
#include "gcc_conference_user_data/cs_monitor.hpp"
#include "gcc_conference_user_data/cs_net.hpp"
#include "gcc_conference_user_data/cs_sec.hpp"
#include "gcc_conference_user_data/sc_core.hpp"
#include "gcc_conference_user_data/sc_net.hpp"
#include "gcc_conference_user_data/sc_sec1.hpp"

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
                LOG(LOG_INFO, "GCC Conference Create Request User data Length mismatch with header+data length %u %u", length, length_with_header);
                throw Error(ERR_GCC);
            }

            if (length != stream.size() - stream.get_offset()){
                LOG(LOG_INFO, "GCC Conference Create Request User data Length mismatch with header %u %u", length, stream.size() - stream.get_offset());
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
    // includes GCC-specific failure information sourced directly from
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



};

#endif
