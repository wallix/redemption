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

#ifndef _REDEMPTION_CORE_RDP_NLA_NTLM_NTLMMESSAGEAUTHENTICATE_HPP_
#define _REDEMPTION_CORE_RDP_NLA_NTLM_NTLMMESSAGEAUTHENTICATE_HPP_

#include "RDP/nla/ntlm/ntlm_message.hpp"
#include "RDP/nla/ntlm/ntlm_avpair.hpp"

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
//     in bytes, of DomainName in Payload, not including a NULL terminator.
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
//     in bytes, of UserName in Payload, not including a NULL terminator.
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
//     in bytes, of Workstation in Payload, not including a NULL terminator.
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


struct NTLMAuthenticateMessage : public NTLMMessage {

    NtlmField LmChallengeResponse;        /* 8 Bytes */
    NtlmField NtChallengeResponse;        /* 8 Bytes */
    NtlmField DomainName;                 /* 8 Bytes */
    NtlmField UserName;                   /* 8 Bytes */
    NtlmField Workstation;                /* 8 Bytes */
    NtlmField EncryptedRandomSessionKey;  /* 8 Bytes */
    NtlmNegotiateFlags negoFlags;         /* 4 Bytes */
    NtlmVersion version;                  /* 8 Bytes */
    uint8_t MIC[16];                      /* 16 Bytes */
    bool ignore_mic;
    uint32_t PayloadOffset;

    NTLMAuthenticateMessage()
        : NTLMMessage(NtlmAuthenticate)
        , ignore_mic(false)
        , PayloadOffset(12+8+8+8+8+8+8+4+8 + 16)
    {
        memset(this->MIC, 0x00, 16);
    }

    virtual ~NTLMAuthenticateMessage() {}

    void emit(Stream & stream) {
        uint32_t currentOffset = this->PayloadOffset;
        // currentOffset += 16;
        NTLMMessage::emit(stream);
        currentOffset += this->LmChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->NtChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->DomainName.emit(stream, currentOffset);
        currentOffset += this->UserName.emit(stream, currentOffset);
        currentOffset += this->Workstation.emit(stream, currentOffset);
        currentOffset += this->EncryptedRandomSessionKey.emit(stream, currentOffset);
        this->negoFlags.emit(stream);
        this->version.emit(stream);

        if (this->ignore_mic) {
            stream.out_clear_bytes(16);
        }
        else {
            stream.out_copy_bytes(this->MIC, 16);
        }

        // PAYLOAD
        this->LmChallengeResponse.write_payload(stream);
        this->NtChallengeResponse.write_payload(stream);
        this->DomainName.write_payload(stream);
        this->UserName.write_payload(stream);
        this->Workstation.write_payload(stream);
        this->EncryptedRandomSessionKey.write_payload(stream);
        stream.mark_end();
    }

    void emit2(Stream & stream) {
        uint32_t currentOffset = this->PayloadOffset;
        // currentOffset += 16;
        NTLMMessage::emit(stream);
        currentOffset += this->LmChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->NtChallengeResponse.emit(stream, currentOffset);
        currentOffset += this->DomainName.emit(stream, currentOffset);
        currentOffset += this->UserName.emit(stream, currentOffset);
        currentOffset += this->Workstation.emit(stream, currentOffset);
        currentOffset += this->EncryptedRandomSessionKey.emit(stream, currentOffset);
        this->negoFlags.emit(stream);
        this->version.emit(stream);

        if (this->ignore_mic) {
            stream.out_clear_bytes(16);
        }
        else {
            stream.out_copy_bytes(this->MIC, 16);
        }

        // // PAYLOAD
        this->LmChallengeResponse.write_payload(stream);
        this->NtChallengeResponse.write_payload(stream);
        this->DomainName.write_payload(stream);
        this->UserName.write_payload(stream);
        this->Workstation.write_payload(stream);
        this->EncryptedRandomSessionKey.write_payload(stream);
        stream.mark_end();
    }



    void recv(Stream & stream) {
        uint8_t * pBegin = stream.p;
        bool res;
        res = NTLMMessage::recv(stream);
        if (!res) {
            LOG(LOG_ERR, "INVALID MSG RECEIVED type: %u", this->msgType);
        }
        this->LmChallengeResponse.recv(stream);
        this->NtChallengeResponse.recv(stream);
        this->DomainName.recv(stream);
        this->UserName.recv(stream);
        this->Workstation.recv(stream);
        this->EncryptedRandomSessionKey.recv(stream);
        this->negoFlags.recv(stream);
        this->version.recv(stream);

        // uint32_t min_offset = this->LmChallengeResponse.bufferOffset;
        // if (this->NtChallengeResponse.bufferOffset < min_offset)
        //     min_offset = this->NtChallengeResponse.bufferOffset;
        // if (this->DomainName.bufferOffset < min_offset)
        //     min_offset = this->DomainName.bufferOffset;
        // if (this->UserName.bufferOffset < min_offset)
        //     min_offset = this->UserName.bufferOffset;
        // if (this->Workstation.bufferOffset < min_offset)
        //     min_offset = this->Workstation.bufferOffset;
        // if (this->EncryptedRandomSessionKey.bufferOffset < min_offset)
        //     min_offset = this->EncryptedRandomSessionKey.bufferOffset;

        stream.in_copy_bytes(this->MIC, 16);

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
    uint8_t Response[16];
    uint8_t ClientChallenge[8];

    void emit(Stream & stream) {
        stream.out_copy_bytes(this->Response, 16);
        stream.out_copy_bytes(this->ClientChallenge, 8);
    }

    void recv(Stream & stream) {
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
    uint8_t  Timestamp[8];          // Current system time
    uint8_t  ClientChallenge[8];    // Client Challenge
    uint32_t Reserved3;             // MUST BE 0x00
    NtlmAvPairList AvPairList;
    uint32_t Reserved4;             // MUST BE 0x00

    void emit(Stream & stream) {
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


    void recv(Stream & stream) {
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
    uint8_t Response[16];
    NTLMv2_Client_Challenge Challenge;

    void emit(Stream & stream) {
        stream.out_copy_bytes(this->Response, 16);
        this->Challenge.emit(stream);
    }

    void recv(Stream & stream) {
        stream.in_copy_bytes(this->Response, 16);
        this->Challenge.recv(stream);
    }

};

#endif



//     SECURITY_STATUS ntlm_read_AuthenticateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	UINT32 flags;
// 	UINT32 MicOffset;
// 	NTLM_AV_PAIR* AvFlags;
// 	NTLMv2_RESPONSE response;
// 	UINT32 PayloadBufferOffset;
// 	NTLM_AUTHENTICATE_MESSAGE* message;

// 	flags = 0;
// 	MicOffset = 0;
// 	AvFlags = NULL;

// 	message = &context->AUTHENTICATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_AUTHENTICATE_MESSAGE));
// 	ZeroMemory(&response, sizeof(NTLMv2_RESPONSE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	ntlm_read_message_header(s, (NTLM_MESSAGE_HEADER*) message);

// 	if (!ntlm_validate_message_header(s, (NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_AUTHENTICATE))
//             {
// 		Stream_Free(s, FALSE);
// 		return SEC_E_INVALID_TOKEN;
//             }

// 	ntlm_read_message_fields(s, &(message->LmChallengeResponse)); /* LmChallengeResponseFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->NtChallengeResponse)); /* NtChallengeResponseFields (8 bytes) */

// 	/* only set if NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED is set */

// 	ntlm_read_message_fields(s, &(message->DomainName)); /* DomainNameFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->UserName)); /* UserNameFields (8 bytes) */

// 	/* only set if NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED is set */

// 	ntlm_read_message_fields(s, &(message->Workstation)); /* WorkstationFields (8 bytes) */

// 	ntlm_read_message_fields(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKeyFields (8 bytes) */

// 	Stream_Read_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_read_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	PayloadBufferOffset = Stream_GetPosition(s);

// 	ntlm_read_message_fields_buffer(s, &(message->DomainName)); /* DomainName */

// 	ntlm_read_message_fields_buffer(s, &(message->UserName)); /* UserName */

// 	ntlm_read_message_fields_buffer(s, &(message->Workstation)); /* Workstation */

// 	ntlm_read_message_fields_buffer(s, &(message->LmChallengeResponse)); /* LmChallengeResponse */

// 	ntlm_read_message_fields_buffer(s, &(message->NtChallengeResponse)); /* NtChallengeResponse */

// 	if (message->NtChallengeResponse.Len > 0)
//             {
// 		wStream* s = Stream_New(message->NtChallengeResponse.Buffer, message->NtChallengeResponse.Len);
// 		ntlm_read_ntlm_v2_response(s, &response);
// 		Stream_Free(s, FALSE);

// 		context->NtChallengeResponse.pvBuffer = message->NtChallengeResponse.Buffer;
// 		context->NtChallengeResponse.cbBuffer = message->NtChallengeResponse.Len;

// 		context->ChallengeTargetInfo.pvBuffer = (void*) response.Challenge.AvPairs;
// 		context->ChallengeTargetInfo.cbBuffer = message->NtChallengeResponse.Len - (28 + 16);

// 		CopyMemory(context->ClientChallenge, response.Challenge.ClientChallenge, 8);

// 		AvFlags = ntlm_av_pair_get(response.Challenge.AvPairs, MsvAvFlags);

// 		if (AvFlags != NULL)
//                     flags = *((UINT32*) ntlm_av_pair_get_value_pointer(AvFlags));
//             }

// 	/* EncryptedRandomSessionKey */
// 	ntlm_read_message_fields_buffer(s, &(message->EncryptedRandomSessionKey));
// 	CopyMemory(context->EncryptedRandomSessionKey, message->EncryptedRandomSessionKey.Buffer, 16);

// 	length = Stream_GetPosition(s);
// 	sspi_SecBufferAlloc(&context->AuthenticateMessage, length);
// 	CopyMemory(context->AuthenticateMessage.pvBuffer, Stream_Buffer(s), length);
// 	buffer->cbBuffer = length;

// 	Stream_SetPosition(s, PayloadBufferOffset);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		MicOffset = Stream_GetPosition(s);
// 		Stream_Read(s, message->MessageIntegrityCheck, 16);
// 		PayloadBufferOffset += 16;
//             }

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "AUTHENTICATE_MESSAGE (length = %d)\n", (int) context->AuthenticateMessage.cbBuffer);
// 	winpr_HexDump(context->AuthenticateMessage.pvBuffer, context->AuthenticateMessage.cbBuffer);
// 	fprintf(stderr, "\n");

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	ntlm_print_message_fields(&(message->DomainName), "DomainName");
// 	ntlm_print_message_fields(&(message->UserName), "UserName");
// 	ntlm_print_message_fields(&(message->Workstation), "Workstation");
// 	ntlm_print_message_fields(&(message->LmChallengeResponse), "LmChallengeResponse");
// 	ntlm_print_message_fields(&(message->NtChallengeResponse), "NtChallengeResponse");
// 	ntlm_print_message_fields(&(message->EncryptedRandomSessionKey), "EncryptedRandomSessionKey");

// 	ntlm_print_av_pair_list(response.Challenge.AvPairs);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		fprintf(stderr, "MessageIntegrityCheck:\n");
// 		winpr_HexDump(message->MessageIntegrityCheck, 16);
//             }
// #endif

// 	if (message->UserName.Len > 0)
//             {
// 		context->identity.User = (UINT16*) malloc(message->UserName.Len);
// 		CopyMemory(context->identity.User, message->UserName.Buffer, message->UserName.Len);
// 		context->identity.UserLength = message->UserName.Len / 2;
//             }

// 	if (message->DomainName.Len > 0)
//             {
// 		context->identity.Domain = (UINT16*) malloc(message->DomainName.Len);
// 		CopyMemory(context->identity.Domain, message->DomainName.Buffer, message->DomainName.Len);
// 		context->identity.DomainLength = message->DomainName.Len / 2;
//             }

// 	/* LmChallengeResponse */
// 	ntlm_compute_lm_v2_response(context);

// 	/* NtChallengeResponse */
// 	ntlm_compute_ntlm_v2_response(context);

// 	/* KeyExchangeKey */
// 	ntlm_generate_key_exchange_key(context);

// 	/* EncryptedRandomSessionKey */
// 	ntlm_decrypt_random_session_key(context);

// 	/* ExportedSessionKey */
// 	ntlm_generate_exported_session_key(context);

// 	if (flags & MSV_AV_FLAGS_MESSAGE_INTEGRITY_CHECK)
//             {
// 		ZeroMemory(&((PBYTE) context->AuthenticateMessage.pvBuffer)[MicOffset], 16);
// 		ntlm_compute_message_integrity_check(context);
// 		CopyMemory(&((PBYTE) context->AuthenticateMessage.pvBuffer)[MicOffset], message->MessageIntegrityCheck, 16);

// 		if (memcmp(context->MessageIntegrityCheck, message->MessageIntegrityCheck, 16) != 0)
//                     {
// 			fprintf(stderr, "Message Integrity Check (MIC) verification failed!\n");

// 			fprintf(stderr, "Expected MIC:\n");
// 			winpr_HexDump(context->MessageIntegrityCheck, 16);
// 			fprintf(stderr, "Actual MIC:\n");
// 			winpr_HexDump(message->MessageIntegrityCheck, 16);
// 			Stream_Free(s, FALSE);

// 			return SEC_E_MESSAGE_ALTERED;
//                     }
//             }

// 	/* Generate signing keys */
// 	ntlm_generate_client_signing_key(context);
// 	ntlm_generate_server_signing_key(context);

// 	/* Generate sealing keys */
// 	ntlm_generate_client_sealing_key(context);
// 	ntlm_generate_server_sealing_key(context);

// 	/* Initialize RC4 seal state */
// 	ntlm_init_rc4_seal_states(context);

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "ClientChallenge\n");
// 	winpr_HexDump(context->ClientChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerChallenge\n");
// 	winpr_HexDump(context->ServerChallenge, 8);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "SessionBaseKey\n");
// 	winpr_HexDump(context->SessionBaseKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "KeyExchangeKey\n");
// 	winpr_HexDump(context->KeyExchangeKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ExportedSessionKey\n");
// 	winpr_HexDump(context->ExportedSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "RandomSessionKey\n");
// 	winpr_HexDump(context->RandomSessionKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSigningKey\n");
// 	winpr_HexDump(context->ClientSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ClientSealingKey\n");
// 	winpr_HexDump(context->ClientSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSigningKey\n");
// 	winpr_HexDump(context->ServerSigningKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "ServerSealingKey\n");
// 	winpr_HexDump(context->ServerSealingKey, 16);
// 	fprintf(stderr, "\n");

// 	fprintf(stderr, "Timestamp\n");
// 	winpr_HexDump(context->Timestamp, 8);
// 	fprintf(stderr, "\n");
// #endif

// 	context->state = NTLM_STATE_FINAL;

// 	Stream_Free(s, FALSE);

// 	ntlm_free_message_fields_buffer(&(message->DomainName));
// 	ntlm_free_message_fields_buffer(&(message->UserName));
// 	ntlm_free_message_fields_buffer(&(message->Workstation));
// 	ntlm_free_message_fields_buffer(&(message->LmChallengeResponse));
// 	ntlm_free_message_fields_buffer(&(message->NtChallengeResponse));
// 	ntlm_free_message_fields_buffer(&(message->EncryptedRandomSessionKey));

// 	return SEC_I_COMPLETE_NEEDED;
//     }

//     /**
//      * Send NTLMSSP AUTHENTICATE_MESSAGE.\n
//      * AUTHENTICATE_MESSAGE @msdn{cc236643}
//      * @param NTLM context
//      * @param buffer
//      */


// =====================================================
// =====================================================
// =====================================================
// =====================================================




//     SECURITY_STATUS ntlm_write_AuthenticateMessage(PSecBuffer buffer)
//     {
// 	wStream* s;
// 	int length;
// 	UINT32 MicOffset = 0;
// 	UINT32 PayloadBufferOffset;
// 	NTLM_AUTHENTICATE_MESSAGE* message;

// 	message = &context->AUTHENTICATE_MESSAGE;
// 	ZeroMemory(message, sizeof(NTLM_AUTHENTICATE_MESSAGE));

// 	s = Stream_New(buffer->pvBuffer, buffer->cbBuffer);

// 	if (context->NTLMv2)
//             {
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_56;

// 		if (context->SendVersionInfo)
//                     message->NegotiateFlags |= NTLMSSP_NEGOTIATE_VERSION;
//             }

// 	if (context->UseMIC)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_TARGET_INFO;

// 	if (context->SendWorkstationName)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED;

// 	if (context->confidentiality)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SEAL;

// 	if (context->CHALLENGE_MESSAGE.NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             message->NegotiateFlags |= NTLMSSP_NEGOTIATE_KEY_EXCH;

// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_128;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_EXTENDED_SESSION_SECURITY;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_NTLM;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_SIGN;
// 	message->NegotiateFlags |= NTLMSSP_REQUEST_TARGET;
// 	message->NegotiateFlags |= NTLMSSP_NEGOTIATE_UNICODE;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_get_version_info(&(message->Version));

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED)
//             {
// 		message->Workstation.Len = context->Workstation.Length;
// 		message->Workstation.Buffer = (BYTE*) context->Workstation.Buffer;
//             }

// 	if (context->identity.DomainLength > 0)
//             {
// 		message->NegotiateFlags |= NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED;
// 		message->DomainName.Len = (UINT16) context->identity.DomainLength * 2;
// 		message->DomainName.Buffer = (BYTE*) context->identity.Domain;
//             }

// 	message->UserName.Len = (UINT16) context->identity.UserLength * 2;
// 	message->UserName.Buffer = (BYTE*) context->identity.User;

// 	message->LmChallengeResponse.Len = (UINT16) context->LmChallengeResponse.cbBuffer;
// 	message->LmChallengeResponse.Buffer = (BYTE*) context->LmChallengeResponse.pvBuffer;

// 	//if (context->NTLMv2)
// 	//	ZeroMemory(message->LmChallengeResponse.Buffer, message->LmChallengeResponse.Len);

// 	message->NtChallengeResponse.Len = (UINT16) context->NtChallengeResponse.cbBuffer;
// 	message->NtChallengeResponse.Buffer = (BYTE*) context->NtChallengeResponse.pvBuffer;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             {
// 		message->EncryptedRandomSessionKey.Len = 16;
// 		message->EncryptedRandomSessionKey.Buffer = context->EncryptedRandomSessionKey;
//             }

// 	PayloadBufferOffset = 64;

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             PayloadBufferOffset += 8; /* Version (8 bytes) */

// 	if (context->UseMIC)
//             PayloadBufferOffset += 16; /* Message Integrity Check (16 bytes) */

// 	message->DomainName.BufferOffset = PayloadBufferOffset;
// 	message->UserName.BufferOffset = message->DomainName.BufferOffset + message->DomainName.Len;
// 	message->Workstation.BufferOffset = message->UserName.BufferOffset + message->UserName.Len;
// 	message->LmChallengeResponse.BufferOffset = message->Workstation.BufferOffset + message->Workstation.Len;
// 	message->NtChallengeResponse.BufferOffset = message->LmChallengeResponse.BufferOffset + message->LmChallengeResponse.Len;
// 	message->EncryptedRandomSessionKey.BufferOffset = message->NtChallengeResponse.BufferOffset + message->NtChallengeResponse.Len;

// 	ntlm_populate_message_header((NTLM_MESSAGE_HEADER*) message, MESSAGE_TYPE_AUTHENTICATE);

// 	ntlm_write_message_header(s, (NTLM_MESSAGE_HEADER*) message); /* Message Header (12 bytes) */

// 	ntlm_write_message_fields(s, &(message->LmChallengeResponse)); /* LmChallengeResponseFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->NtChallengeResponse)); /* NtChallengeResponseFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->DomainName)); /* DomainNameFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->UserName)); /* UserNameFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->Workstation)); /* WorkstationFields (8 bytes) */

// 	ntlm_write_message_fields(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKeyFields (8 bytes) */

// 	Stream_Write_UINT32(s, message->NegotiateFlags); /* NegotiateFlags (4 bytes) */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_write_version_info(s, &(message->Version)); /* Version (8 bytes) */

// 	if (context->UseMIC)
//             {
// 		MicOffset = Stream_GetPosition(s);
// 		Stream_Zero(s, 16); /* Message Integrity Check (16 bytes) */
//             }

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_DOMAIN_SUPPLIED)
//             ntlm_write_message_fields_buffer(s, &(message->DomainName)); /* DomainName */

// 	ntlm_write_message_fields_buffer(s, &(message->UserName)); /* UserName */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_WORKSTATION_SUPPLIED)
//             ntlm_write_message_fields_buffer(s, &(message->Workstation)); /* Workstation */

// 	ntlm_write_message_fields_buffer(s, &(message->LmChallengeResponse)); /* LmChallengeResponse */

// 	ntlm_write_message_fields_buffer(s, &(message->NtChallengeResponse)); /* NtChallengeResponse */

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_KEY_EXCH)
//             ntlm_write_message_fields_buffer(s, &(message->EncryptedRandomSessionKey)); /* EncryptedRandomSessionKey */

// 	length = Stream_GetPosition(s);
// 	sspi_SecBufferAlloc(&context->AuthenticateMessage, length);
// 	CopyMemory(context->AuthenticateMessage.pvBuffer, Stream_Buffer(s), length);
// 	buffer->cbBuffer = length;

// 	if (context->UseMIC)
//             {
// 		/* Message Integrity Check */
// 		ntlm_compute_message_integrity_check(context);

// 		Stream_SetPosition(s, MicOffset);
// 		Stream_Write(s, context->MessageIntegrityCheck, 16);
// 		Stream_SetPosition(s, length);
//             }

// #ifdef WITH_DEBUG_NTLM
// 	fprintf(stderr, "AUTHENTICATE_MESSAGE (length = %d)\n", length);
// 	winpr_HexDump(Stream_Buffer(s), length);
// 	fprintf(stderr, "\n");

// 	ntlm_print_negotiate_flags(message->NegotiateFlags);

// 	if (message->NegotiateFlags & NTLMSSP_NEGOTIATE_VERSION)
//             ntlm_print_version_info(&(message->Version));

// 	if (context->AuthenticateTargetInfo.cbBuffer > 0)
//             {
// 		fprintf(stderr, "AuthenticateTargetInfo (%d):\n", (int) context->AuthenticateTargetInfo.cbBuffer);
// 		ntlm_print_av_pair_list(context->AuthenticateTargetInfo.pvBuffer);
//             }

// 	ntlm_print_message_fields(&(message->DomainName), "DomainName");
// 	ntlm_print_message_fields(&(message->UserName), "UserName");
// 	ntlm_print_message_fields(&(message->Workstation), "Workstation");
// 	ntlm_print_message_fields(&(message->LmChallengeResponse), "LmChallengeResponse");
// 	ntlm_print_message_fields(&(message->NtChallengeResponse), "NtChallengeResponse");
// 	ntlm_print_message_fields(&(message->EncryptedRandomSessionKey), "EncryptedRandomSessionKey");

// 	if (context->UseMIC)
//             {
// 		fprintf(stderr, "MessageIntegrityCheck (length = 16)\n");
// 		winpr_HexDump(context->MessageIntegrityCheck, 16);
// 		fprintf(stderr, "\n");
//             }
// #endif

// 	context->state = NTLM_STATE_FINAL;

// 	Stream_Free(s, FALSE);

// 	return SEC_I_COMPLETE_NEEDED;
//     }
