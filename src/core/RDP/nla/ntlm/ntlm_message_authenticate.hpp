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
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/


#pragma once

#include "core/RDP/nla/ntlm/ntlm_message.hpp"
#include "core/RDP/nla/ntlm/ntlm_avpair.hpp"

// [MS-NLMP]
// 2.2.1.3   AUTHENTICATE_MESSAGE
// ===================================================
// The AUTHENTICATE_MESSAGE defines an NTLM authenticate message that is sent from the client
//  to the server after the CHALLENGE_MESSAGE (section 2.2.1.2) is processed by the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Signature                           |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                          MessageType                          |
// +---------------+---------------+---------------+---------------+
// |                   LmChallengeResponseFields                   |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                   NtChallengeResponseFields                   |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        DomainNameFields                       |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                         UserNameFields                        |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                        WorkstationFields                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                EncryptedRandomSessionKeyFields                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                         NegotiateFlags                        |
// +---------------+---------------+---------------+---------------+
// |                            Version                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              MIC                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                       Payload (Variable)                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+

// Signature (8 bytes):  An 8-byte character array that MUST contain the
//  ASCII string ('N', 'T', 'L', 'M', 'S', 'S', 'P', '\0').

// MessageType (4 bytes):  A 32-bit unsigned integer that indicates the message
//  type. This field MUST be set to 0x00000003.

// LmChallengeResponseFields (8 bytes):  If the client chooses not to send an
//  LmChallengeResponse to the server:
//  - LmChallengeResponseLen and LmChallengeResponseMaxLen MUST be set to zero on
//     transmission.
//  - LmChallengeResponseBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the LmChallengeResponse would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    LmChallengeResponseLen     |   LmChallengeResponseMaxLen   |
// +---------------+---------------+---------------+---------------+
// |                LmChallengeResponseBufferOffset                |
// +---------------+---------------+---------------+---------------+
//    LmChallengeResponseLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of LmChallengeResponse in Payload.
//    LmChallengeResponseMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of LmChallengeResponseLen and MUST be ignored on receipt.
//    LmChallengeResponseBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     LmChallengeResponse in Payload.

// NtChallengeResponseFields (8 bytes):  If the client chooses not to send an
//  NtChallengeResponse to the server:
//  - NtChallengeResponseLen and NtChallengeResponseMaxLen MUST be set to zero on
//     transmission.
//  - NtChallengeResponseBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the NtChallengeResponse would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |    NtChallengeResponseLen     |   NtChallengeResponseMaxLen   |
// +---------------+---------------+---------------+---------------+
// |                NtChallengeResponseBufferOffset                |
// +---------------+---------------+---------------+---------------+
//    NtChallengeResponseLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of NtChallengeResponse in Payload.
//    NtChallengeResponseMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of NtChallengeResponseLen and MUST be ignored on receipt.
//    NtChallengeResponseBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     NtChallengeResponse in Payload.

// DomainNameFields (8 bytes):  If the client chooses not to send an
//  DomainName to the server:
//  - DomainNameLen and DomainNameMaxLen MUST be set to zero on transmission.
//  - DomainNameBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the DomainName would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         DomainNameLen         |        DomainNameMaxLen       |
// +---------------+---------------+---------------+---------------+
// |                    DomainNameBufferOffset                     |
// +---------------+---------------+---------------+---------------+
//    DomainNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of DomainName in Payload, not including a nullptr terminator.
//    DomainNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of DomainNameLen and MUST be ignored on receipt.
//    DomainNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     DomainName in Payload.
//     If DomainName is a Unicode string, the values of DomainNameBufferOffset
//     and DomainNameLen MUST be multiples of 2.

// UserNameFields (8 bytes):  If the client chooses not to send an
//  UserName to the server:
//  - UserNameLen and UserNameMaxLen MUST be set to zero on transmission.
//  - UserNameBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the UserName would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          UserNameLen          |         UserNameMaxLen        |
// +---------------+---------------+---------------+---------------+
// |                     UserNameBufferOffset                      |
// +---------------+---------------+---------------+---------------+
//    UserNameLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of UserName in Payload, not including a nullptr terminator.
//    UserNameMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of UserNameLen and MUST be ignored on receipt.
//    UserNameBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     UserName in Payload.
//     If UserName is a Unicode string, the values of UserNameBufferOffset
//     and UserNameLen MUST be multiples of 2.

// WorkstationFields (8 bytes):  If the client chooses not to send an
//  Workstation to the server:
//  - WorkstationLen and WorkstationMaxLen MUST be set to zero on transmission.
//  - WorkstationBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the Workstation would
//     be in Payload if it was present.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        WorkstationLen         |        WorkstationMaxLen      |
// +---------------+---------------+---------------+---------------+
// |                    WorkstationBufferOffset                    |
// +---------------+---------------+---------------+---------------+
//    WorkstationLen (2 bytes):  A 16-bit unsigned integer that defines the size,
//     in bytes, of Workstation in Payload, not including a nullptr terminator.
//    WorkstationMaxLen (2 bytes):  A 16-bit unsigned integer that SHOULD be
//     set to the value of WorkstationLen and MUST be ignored on receipt.
//    WorkstationBufferOffset (4 bytes):  A 32-bit unsigned integer that defines
//     the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     Workstation in Payload.
//     If Workstation is a Unicode string, the values of WorkstationBufferOffset
//     and WorkstationLen MUST be multiples of 2.

// EncryptedRandomSessionKeyFields (8 bytes):  If the NTLMSSP_NEGOTIATE_KEY_EXCH flag
//  is not set in NegotiateFlags, indicating that no EncryptedRandomSessionKey is supplied:
//  - EncryptedRandomSessionKeyLen and EncryptedRandomSessionKeyMaxLen
//     SHOULD be set to zero on transmission.
//  - EncryptedRandomSessionKeyBufferOffset field SHOULD be set to the offset from the
//     beginning of the AUTHENTICATE_MESSAGE to where the EncryptedRandomSessionKey
//     would be in Payload if it was present.
//  - EncryptedRandomSessionKeyLen, EncryptedRandomSessionKeyMaxLen and
//     EncryptedRandomSessionKeyBufferOffset MUST be ignored on receipt.
//  Otherwise, these fields are defined as:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  EncryptedRandomSessionKeyLen |EncryptedRandomSessionKeyMaxLen|
// +---------------+---------------+---------------+---------------+
// |               EncryptedRandomSessionBufferOffset              |
// +---------------+---------------+---------------+---------------+
//    EncryptedRandomSessionKeyLen (2 bytes):  A 16-bit unsigned integer that defines
//     the size, in bytes, of EncryptedRandomSessionKey in Payload.
//    EncryptedRandomSessionKeyMaxLen (2 bytes):  A 16-bit unsigned integer that
//     SHOULD be set to the value of EncryptedRandomSessionKeyLen and MUST be
//     ignored on receipt.
//    EncryptedRandomSessionKeyBufferOffset (4 bytes):  A 32-bit unsigned integer that
//     defines the offset, in bytes, from the beginning of the AUTHENTICATE_MESSAGE to
//     EncryptedRandomSessionKey in Payload.

// NegotiateFlags (4 bytes):  In connectionless mode, a NEGOTIATE structure that contains
//  a set of bit flags (section 2.2.2.5) and represents the conclusion of negotiation.
//  the choices the client has made from the options the server offered in the
//  CHALLENGE_MESSAGE. In connection-oriented mode, a NEGOTIATE structure that contains
//  the set of bit flags (section 2.2.2.5) negotiated in the previous messages.

// Version (8 bytes):  A VERSION structure (section 2.2.2.10) that is present only when
//  the NTLMSSP_NEGOTIATE_VERSION flag is set in the NegotiateFlags field. This structure
//  is used for debugging purposes only. In normal protocol messages, it is ignored and
//  does not affect the NTLM message processing.

// MIC (16 bytes):  The message integrity for the NTLM NEGOTIATE_MESSAGE,
//  CHALLENGE_MESSAGE, and AUTHENTICATE_MESSAGE.

// Payload (variable):  A byte array that contains the data referred to by the
//  LmChallengeResponseBufferOffset, NtChallengeResponseBufferOffset,
//  DomainNameBufferOffset, UserNameBufferOffset, WorkstationBufferOffset, and
//  EncryptedRandomSessionKeyBufferOffset message fields. Payload data can be present
//  in any order within the Payload field, with variable-length padding before or after
//  the data. The data that can be present in the Payload field of this message,
//  in no particular order, are:
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 LmChallengeResponse (Variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                 NtChallengeResponse (Variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      DomainName (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                       UserName (Variable)                     |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                     Workstation (Variable)                    |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |              EncryptedRandomSessionKey (Variable)             |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  LmChallengeResponse (variable):  An LM_RESPONSE or LMv2_RESPONSE structure that
//   contains the computed LM response to the challenge. If NTLM v2 authentication is
//   configured, LmChallengeResponse MUST be an LMv2_RESPONSE structure (section
//   2.2.2.4). Otherwise, it MUST be an LM_RESPONSE structure (section 2.2.2.3).
//  NtChallengeResponse (variable):  An NTLM_RESPONSE or NTLMv2_RESPONSE
//   structure that contains the computed NT response to the challenge. If NTLM v2
//   authentication is configured, NtChallengeResponse MUST be an NTLMv2_RESPONSE
//   (section 2.2.2.8). Otherwise, it MUST be an NTLM_RESPONSE structure (section
//   2.2.2.6).
//  DomainName (variable):  The domain or computer name hosting the user account.
//   DomainName MUST be encoded in the negotiated character set.
//  UserName (variable):  The name of the user to be authenticated. UserName MUST be
//   encoded in the negotiated character set.
//  Workstation (variable):  The name of the computer to which the user is logged on.
//   Workstation MUST be encoded in the negotiated character set.
//  EncryptedRandomSessionKey (variable):  The client's encrypted random session key.
//   EncryptedRandomSessionKey and its usage are defined in sections 3.1.5 and 3.2.5.


struct NTLMAuthenticateMessage {
    NTLMMessage message;

    NtlmField LmChallengeResponse;        /* 8 Bytes */
    NtlmField NtChallengeResponse;        /* 8 Bytes */
    NtlmField DomainName;                 /* 8 Bytes */
    NtlmField UserName;                   /* 8 Bytes */
    NtlmField Workstation;                /* 8 Bytes */
    NtlmField EncryptedRandomSessionKey;  /* 8 Bytes */
    NtlmNegotiateFlags negoFlags;         /* 4 Bytes */
    NtlmVersion version;                  /* 8 Bytes */
    uint8_t MIC[16]{};                      /* 16 Bytes */
    bool ignore_mic{false};
    bool has_mic{true};
    uint32_t PayloadOffset;

    NTLMAuthenticateMessage()
        : message(NtlmAuthenticate)
        ,
         PayloadOffset(12+8+8+8+8+8+8+4+8)
    {
        memset(this->MIC, 0x00, 16);
    }

    void log() {
        this->LmChallengeResponse.log("LmChallengeResponse");
        this->NtChallengeResponse.log("NtChallengeResponse");
        this->DomainName.log("DomainName");
        this->UserName.log("UserName");
        this->Workstation.log("Workstation");
        this->EncryptedRandomSessionKey.log("EncryptedRandomSessionKey");
        this->negoFlags.log();
        this->version.log();
        LOG(LOG_DEBUG, "MIC");
        hexdump_d(this->MIC, 16);
    }

    void emit(OutStream & stream) /* TODO const*/ {
        uint32_t currentOffset = this->PayloadOffset;
        if (this->version.ignore_version) {
            currentOffset -= 8;
        }
        if (this->has_mic) {
            currentOffset += 16;
        }
        this->message.emit(stream);
        currentOffset += this->LmChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->NtChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->DomainName.emit(stream, currentOffset);
        currentOffset += this->UserName.emit(stream, currentOffset);
        currentOffset += this->Workstation.emit(stream, currentOffset);
        currentOffset += this->EncryptedRandomSessionKey.emit(stream, currentOffset);
        (void)currentOffset;
        this->negoFlags.emit(stream);
        this->version.emit(stream);

        if (this->has_mic) {
            if (this->ignore_mic) {
                stream.out_clear_bytes(16);
            }
            else {
                stream.out_copy_bytes(this->MIC, 16);
            }
        }

        // PAYLOAD
        this->LmChallengeResponse.write_payload(stream);
        this->NtChallengeResponse.write_payload(stream);
        this->DomainName.write_payload(stream);
        this->UserName.write_payload(stream);
        this->Workstation.write_payload(stream);
        this->EncryptedRandomSessionKey.write_payload(stream);
    }

    void recv(InStream & stream) {
        uint8_t const * pBegin = stream.get_current();
        bool res;
        res = this->message.recv(stream);
        if (!res) {
            LOG(LOG_ERR, "INVALID MSG RECEIVED type: %u", this->message.msgType);
        }
        this->LmChallengeResponse.recv(stream);
        this->NtChallengeResponse.recv(stream);
        this->DomainName.recv(stream);
        this->UserName.recv(stream);
        this->Workstation.recv(stream);
        this->EncryptedRandomSessionKey.recv(stream);
        this->negoFlags.recv(stream);
        if (this->negoFlags.flags & NTLMSSP_NEGOTIATE_VERSION) {
            this->version.recv(stream);
        }
        uint32_t min_offset = this->LmChallengeResponse.bufferOffset;
        if (this->NtChallengeResponse.bufferOffset < min_offset)
            min_offset = this->NtChallengeResponse.bufferOffset;
        if (this->DomainName.bufferOffset < min_offset)
            min_offset = this->DomainName.bufferOffset;
        if (this->UserName.bufferOffset < min_offset)
            min_offset = this->UserName.bufferOffset;
        if (this->Workstation.bufferOffset < min_offset)
            min_offset = this->Workstation.bufferOffset;
        if (this->EncryptedRandomSessionKey.bufferOffset < min_offset)
            min_offset = this->EncryptedRandomSessionKey.bufferOffset;
        if (min_offset + pBegin > stream.get_current()) {
            this->has_mic = true;
            stream.in_copy_bytes(this->MIC, 16);
        }
        else {
            this->has_mic = false;
        }

        // PAYLOAD
        this->LmChallengeResponse.read_payload(stream, pBegin);
        this->NtChallengeResponse.read_payload(stream, pBegin);
        this->DomainName.read_payload(stream, pBegin);
        this->UserName.read_payload(stream, pBegin);
        this->Workstation.read_payload(stream, pBegin);
        this->EncryptedRandomSessionKey.read_payload(stream, pBegin);
    }
};

// 2.2.2.3   LM_RESPONSE
// ====================================
// The LM_RESPONSE structure defines the NTLM v1 authentication LmChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLM v1 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (24 bytes):  A 24-byte array of unsigned char that contains the client's
//   LmChallengeResponse as defined in section 3.3.1.

struct LM_Response {
    uint8_t response[24];
};


// 2.2.2.4   LMv2_RESPONSE
// =================================================
// The LMv2_RESPONSE structure defines the NTLM v2 authentication LmChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLM v2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      ChallengeFromClient                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (16 bytes):  A 16-byte array of unsigned char that contains the client's LM
//   challenge-response. This is the portion of the LmChallengeResponse field to which the
//   HMAC_MD5 algorithm has been applied, as defined in section 3.3.2. Specifically, Response
//   corresponds to the result of applying the HMAC_MD5 algorithm, using the key
//   ResponseKeyLM, to a message consisting of the concatenation of the ResponseKeyLM,
//   ServerChallenge and ClientChallenge.
//  ChallengeFromClient (8 bytes):  An 8-byte array of unsigned char that contains the client's
//   ClientChallenge, as defined in section 3.1.5.1.2.

struct LMv2_Response {
    uint8_t Response[16]{};
    uint8_t ClientChallenge[8]{};

    LMv2_Response()
    = default;

    void emit(OutStream & stream) const {
        stream.out_copy_bytes(this->Response, 16);
        stream.out_copy_bytes(this->ClientChallenge, 8);
    }

    void recv(InStream & stream) {
        stream.in_copy_bytes(this->Response, 16);
        stream.in_copy_bytes(this->ClientChallenge, 8);
    }

};

// 2.2.2.6   NTLM v1 Response: NTLM_RESPONSE
// ===============================================
// The NTLM_RESPONSE structure defines the NTLM v1 authentication NtChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is only used when NTLM v1 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (24 bytes):  A 24-byte array of unsigned char that contains the client's
//   NtChallengeResponse (section 3.3.1).
struct NTLM_Response {
    uint8_t response[32];
};


// 2.2.2.7   NTLM v2: NTLMv2_CLIENT_CHALLENGE
// ===============================================
// The NTLMv2_CLIENT_CHALLENGE structure defines the client challenge in the
// AUTHENTICATE_MESSAGE. This structure is used only when NTLM v2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   RespType    |   HiRespType  |          Reserved1            |
// +---------------+---------------+---------------+---------------+
// |                           Reserved2                           |
// +---------------+---------------+---------------+---------------+
// |                           TimeStamp                           |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                      ChallengeFromClient                      |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                           Reserved3                           |
// +---------------+---------------+---------------+---------------+
// |                       AvPairs (variable)                      |
// +---------------+---------------+---------------+---------------+
//  RespType (1 byte):  An 8-bit unsigned char that contains the current version of the challenge
//   response type. This field MUST be 0x01.
//  HiRespType (1 byte):  An 8-bit unsigned char that contains the maximum supported version of
//   the challenge response type. This field MUST be 0x01.
//  Reserved1 (2 bytes):  A 16-bit unsigned integer that SHOULD be 0x0000 and MUST be ignored
//   on receipt.
//  Reserved2 (4 bytes):  A 32-bit unsigned integer that SHOULD be 0x00000000 and MUST be
//   ignored on receipt.
//  TimeStamp (8 bytes):  A 64-bit unsigned integer that contains the current system time,
//   represented as the number of 100 nanosecond ticks elapsed since midnight of January 1,
//   1601 (UTC).
//  ChallengeFromClient (8 bytes):  An 8-byte array of unsigned char that contains the client's
//   ClientChallenge (section 3.1.5.1.2).
//  Reserved3 (4 bytes):  A 32-bit unsigned integer that SHOULD be 0x00000000 and MUST be
//   ignored on receipt.
//  AvPairs (variable):  A byte array that contains a sequence of AV_PAIR structures (section
//   2.2.2.1). The sequence contains the server-naming context and is terminated by an AV_PAIR
//   structure with an AvId field of MsvAvEOL.

struct NTLMv2_Client_Challenge {
    uint8_t  RespType;              // MUST BE 0x01
    uint8_t  HiRespType;            // MUST BE 0x01
    uint16_t Reserved1;             // MUST BE 0x00
    uint32_t Reserved2;             // MUST BE 0x00
    uint8_t  Timestamp[8]{};          // Current system time
    uint8_t  ClientChallenge[8]{};    // Client Challenge
    uint32_t Reserved3;             // MUST BE 0x00
    NtlmAvPairList AvPairList;
    uint32_t Reserved4;             // MUST BE 0x00

    NTLMv2_Client_Challenge()

    = default;

    void emit(OutStream & stream) /* TODO const*/ {
        // ULONG length;

        this->RespType = 0x01;
        this->HiRespType = 0x01;
        stream.out_uint8(this->RespType);
        stream.out_uint8(this->HiRespType);
        stream.out_clear_bytes(2);
        stream.out_clear_bytes(4);
        stream.out_copy_bytes(this->Timestamp, 8);
        stream.out_copy_bytes(this->ClientChallenge, 8);
        stream.out_clear_bytes(4);
        this->AvPairList.emit(stream);
        stream.out_clear_bytes(4);
    }

    void recv(InStream & stream) {
        // size_t size;
        this->RespType = stream.in_uint8();
        this->HiRespType = stream.in_uint8();
        stream.in_skip_bytes(2);
        stream.in_skip_bytes(4);
        stream.in_copy_bytes(this->Timestamp, 8);
        stream.in_copy_bytes(this->ClientChallenge, 8);
        stream.in_skip_bytes(4);
        this->AvPairList.recv(stream);
        stream.in_skip_bytes(4);
    }
};

// 2.2.2.8   NTLM2 V2 Response: NTLMv2_RESPONSE
// ==================================================
// The NTLMv2_RESPONSE structure defines the NTLMv2 authentication NtChallengeResponse in the
// AUTHENTICATE_MESSAGE. This response is used only when NTLMv2 authentication is configured.
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           Response                            |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
// |                 ChallengeFromClient (variable)                |
// +---------------+---------------+---------------+---------------+
// |                              ...                              |
// +---------------+---------------+---------------+---------------+
//  Response (16 bytes):  A 16-byte array of unsigned char that contains the client's NT
//  challenge-response as defined in section 3.3.2. Response corresponds to the NTProofStr
//   variable from section 3.3.2.
//  ChallengeFromClient (variable):  A variable-length byte array that contains the
//   ClientChallenge as defined in section 3.3.2. ChallengeFromClient corresponds to the
//   temp variable from section 3.3.2.

struct NTLMv2_Response {
    uint8_t Response[16]{};
    NTLMv2_Client_Challenge Challenge;

    NTLMv2_Response()

    = default;

    void emit(OutStream & stream) /* TODO const*/ {
        stream.out_copy_bytes(this->Response, 16);
        this->Challenge.emit(stream);
    }

    void recv(InStream & stream) {
        stream.in_copy_bytes(this->Response, 16);
        this->Challenge.recv(stream);
    }

};
