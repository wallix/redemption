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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common sec layer at core module

*/

#if !defined(__SEC_HPP__)
#define __SEC_HPP__

#include <assert.h>
#include <stdint.h>

#include <iostream>

#include "RDP/x224.hpp"
#include "RDP/rdp.hpp"
#include "client_info.hpp"
#include "rsa_keys.hpp"
#include "constants.hpp"


#warning ssl calls introduce some dependency on ssl system library, injecting it in the sec object would be better.
#include "ssl_calls.hpp"

// 2.2.1.1.1   RDP Negotiation Request (RDP_NEG_REQ)
// =================================================
//  The RDP Negotiation Request structure is used by a client to advertise the
//  security protocols which it supports.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x01 (TYPE_RDP_NEG_REQ) to indicate that the packet
//   is a Negotiation Request.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size.
//   This field MUST be set to 0x0008 (8 bytes).

// requestedProtocols (4 bytes): A 32-bit, unsigned integer. Flags indicating
//   the supported security protocols.

// +---------------------------------+-----------------------------------------+
// | 0x00000000 PROTOCOL_RDP_FLAG    |  Legacy RDP encryption.                 |
// +---------------------------------+-----------------------------------------+
// | 0x00000001 PROTOCOL_SSL_FLAG    | TLS 1.0 (section 5.4.5.1).              |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID_FLAG | Credential Security Support Provider    |
// |                                 | protocol (CredSSP) (section 5.4.5.2).   |
// |                                 | If this flag is set, then the           |
// |                                 | PROTOCOL_SSL_FLAG (0x00000001) SHOULD   |
// |                                 | also be set because Transport Layer     |
// |                                 | Security (TLS) is a subset of CredSSP.  |
// +---------------------------------+-----------------------------------------+

// 2.2.1.2.1   RDP Negotiation Response (RDP_NEG_RSP)
// ==================================================

//  The RDP Negotiation Response structure is used by a server to inform the
//  client of the security protocol which it has selected to use for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This field MUST be set to
//   0x02 (TYPE_RDP_NEG_RSP) to indicate that the packet is a Negotiation Response.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags.

// +--------------------------------+------------------------------------------+
// | EXTENDED_CLIENT_DATA_SUPPORTED | The server supports extended client data |
// | 0x00000001                     | blocks in the GCC Conference Create      |
// |                                | Request user data (section 2.2.1.3).     |
// +--------------------------------+------------------------------------------+

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes)

// selectedProtocol (4 bytes): A 32-bit, unsigned integer. Field indicating the
//   selected security protocol.

// +---------------------------------------------------------------------------+
// | 0x00000000 PROTOCOL_RDP    | Legacy RDP encryption                        |
// +---------------------------------------------------------------------------+
// | 0x00000001 PROTOCOL_SSL    | TLS 1.0 (section 5.4.5.1)                    |
// +---------------------------------------------------------------------------+
// | 0x00000002 PROTOCOL_HYBRID | CredSSP (section 5.4.5.2)                    |
// +---------------------------------------------------------------------------+

// 2.2.1.2.2   RDP Negotiation Failure (RDP_NEG_FAILURE)
// =====================================================

//  The RDP Negotiation Failure structure is used by a server to inform the
//  client of a failure that has occurred while preparing security for the
//  connection.

// type (1 byte): An 8-bit, unsigned integer. Negotiation packet type. This
//   field MUST be set to 0x03 (TYPE_RDP_NEG_FAILURE) to indicate that the
//   packet is a Negotiation Failure.

// flags (1 byte): An 8-bit, unsigned integer. Negotiation packet flags. There
//   are currently no defined flags so the field MUST be set to 0x00.

// length (2 bytes): A 16-bit, unsigned integer. Indicates the packet size. This
//   field MUST be set to 0x0008 (8 bytes).

// failureCode (4 bytes): A 32-bit, unsigned integer. Field containing the
//   failure code.

// +---------------------------+-----------------------------------------------+
// | SSL_REQUIRED_BY_SERVER    | The server requires that the client support   |
// | 0x00000001                | Enhanced RDP Security (section 5.4) with      |
// |                           | either TLS 1.0 (section 5.4.5.1) or CredSSP   |
// |                           | (section 5.4.5.2). If only CredSSP was        |
// |                           | requested then the server only supports TLS.  |
// +---------------------------+-----------------------------------------------+
// | SSL_NOT_ALLOWED_BY_SERVER | The server is configured to only use Standard |
// | 0x00000002                | RDP Security mechanisms (section 5.3) and     |
// |                           | does not support any External                 |
// |                           | Security Protocols (section 5.4.5).           |
// +---------------------------+-----------------------------------------------+
// | SSL_CERT_NOT_ON_SERVER    | The server does not possess a valid server    |
// | 0x00000003                | authentication certificate and cannot         |
// |                           | initialize the External Security Protocol     |
// |                           | Provider (section 5.4.5).                     |
// +---------------------------+-----------------------------------------------+
// | INCONSISTENT_FLAGS        | The list of requested security protocols is   |
// | 0x00000004                | not consistent with the current security      |
// |                           | protocol in effect. This error is only        |
// |                           | possible when the Direct Approach (see        |
// |                           | sections 5.4.2.2 and 1.3.1.2) is used and an  |
// |                           | External Security Protocol (section 5.4.5) is |
// |                           | already being used.                           |
// +---------------------------+-----------------------------------------------+
// | HYBRID_REQUIRED_BY_SERVER | The server requires that the client support   |
// | 0x00000005                | Enhanced RDP Security (section 5.4) with      |
// |                           | CredSSP (section 5.4.5.2).                    |
// +---------------------------+-----------------------------------------------+


class SecOut
{
    Stream & stream;
    uint16_t offhdr;
    uint8_t crypt_level;
    CryptContext & crypt;
    public:
    SecOut(Stream & stream, uint8_t crypt_level, uint32_t flags, CryptContext & crypt)
        : stream(stream), offhdr(stream.p - stream.data), crypt_level(crypt_level), crypt(crypt)
    {
        if (crypt_level > 1){
            this->stream.out_uint32_le(flags);
            this->stream.skip_uint8(8);
        }
        else {
            this->stream.out_uint32_le(0);
        }
    }

    void end(){
        if (crypt_level > 1){
            uint8_t * data = this->stream.data + this->offhdr + 12;
            int datalen = this->stream.p - data;
            this->crypt.sign(this->stream.data + this->offhdr + 4, 8, data, datalen);
            this->crypt.encrypt(data, datalen);
        }
    }
};


class SecIn
{
    public:
    uint32_t flags;
    SecIn(Stream & stream, CryptContext & crypt)
    {
        this->flags = stream.in_uint32_le();
        if ((this->flags & SEC_ENCRYPT)  || (this->flags & 0x0400)){
            #warning shouldn't we check signature ?
            stream.skip_uint8(8); /* signature */
            // decrypting to the end of tpdu
            crypt.decrypt(stream.p, stream.end - stream.p);
        }
    }

    void end(){
        #warning put some assertion here to ensure all data has been consumed
    }

};


struct Sec
{

// only in server_sec : need cleanup

    uint8_t server_random[32];
    uint8_t client_random[64];

// only in rdp_sec : need cleanup
    uint32_t server_public_key_len;

// shared

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];

    CryptContext encrypt, decrypt;
    uint8_t crypt_level;
    #warning seems rc4_key_size is redundant with crypt level ?

    Sec(uint8_t crypt_level) :
      crypt_level(crypt_level)
    {
        // from server_sec
        // CGR: see if init has influence for the 3 following fields
        memset(this->server_random, 0, 32);
        memset(this->client_random, 0, 64);

        // from rdp_sec
        memset(this->client_crypt_random, 0, 512);
        this->server_public_key_len = 0;

        // shared
        memset(this->decrypt.key, 0, 16);
        memset(this->encrypt.key, 0, 16);
        memset(this->decrypt.update_key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        switch (crypt_level) {
        case 1:
        case 2:
            this->decrypt.rc4_key_size = 1; /* 40 bits */
            this->encrypt.rc4_key_size = 1; /* 40 bits */
            this->decrypt.rc4_key_len = 8; /* 8 = 40 bit */
            this->encrypt.rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->decrypt.rc4_key_size = 2; /* 128 bits */
            this->encrypt.rc4_key_size = 2; /* 128 bits */
            this->decrypt.rc4_key_len = 16; /* 16 = 128 bit */
            this->encrypt.rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }

    }

    ~Sec() {}



    /*****************************************************************************/
    void server_sec_disconnect(Transport * trans)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);

        tpdu.end();
        tpdu.send(trans);
    }



    void send_mcs_connect_response_pdu_with_gcc_conference_create_response(
                                            Transport * trans,
                                            ClientInfo * client_info,
                                            const ChannelList & channel_list,
                                            int rc4_key_size,
                                            uint8_t (&pub_mod)[512],
                                            uint8_t (&pri_exp)[512]
                                        ) throw(Error)
    {
        Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);
        memset(this->server_random, 0x44, 32);
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
        }
        if (fd != -1) {
            if (read(fd, this->server_random, 32) != 32) {
            }
            close(fd);
        }

        uint8_t pub_sig[512];

        memcpy(pub_mod, rsa_keys.pub_mod, 64);
        memcpy(pub_sig, rsa_keys.pub_sig, 64);
        memcpy(pri_exp, rsa_keys.pri_exp, 64);

        Stream stream(8192);

        // TPKT Header (length = 337 bytes)
        // X.224 Data TPDU
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        uint32_t offset_len_mcs_connect_response = stream.p - stream.data;
        // BER: Type Length
        stream.out_ber_len_uint16(0); // filled later, 3 bytes

        // Connect-Response::result = rt-successful (0)
        // The first byte (0x0a) is the ASN.1 BER encoded Enumerated type. The
        // length of the value is given by the second byte (1 byte), and the
        // actual value is 0 (rt-successful).
        stream.out_uint8(BER_TAG_RESULT);
        stream.out_ber_len_uint7(1);
        stream.out_uint8(0);

        // Connect-Response::calledConnectId = 0
        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_ber_len_uint7(1);
        stream.out_uint8(0);

        // Connect-Response::domainParameters (26 bytes)
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len_uint7(26);
        // DomainParameters::maxChannelIds = 34
        stream.out_ber_int8(22);
        // DomainParameters::maxUserIds = 3
        stream.out_ber_int8(3);
        // DomainParameters::maximumTokenIds = 0
        stream.out_ber_int8(0);
        // DomainParameters::numPriorities = 1
        stream.out_ber_int8(1);
        // DomainParameters::minThroughput = 0
        stream.out_ber_int8(0);
        // DomainParameters::maxHeight = 1
        stream.out_ber_int8(1);
        // DomainParameters::maxMCSPDUsize = 65528
        stream.out_ber_int24(0xfff8);
        // DomainParameters::protocolVersion = 2
        stream.out_ber_int8(2);

        // Connect-Response::userData (287 bytes)
        stream.out_uint8(BER_TAG_OCTET_STRING);
        uint32_t offset_len_mcs_data = stream.p - stream.data;
        stream.out_ber_len_uint16(0); // filled later, 3 bytes


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
        // 0 - --\
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
        // 0 - --\
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

        uint16_t padding = channel_list.size() & 1;
        uint16_t srv_channel_size = 8 + (channel_list.size() + padding) * 2;
        stream.out_2BUE(8 + srv_channel_size + 236); // len

        stream.out_uint16_le(SC_CORE);
        // length, including tag and length fields
        stream.out_uint16_le(8); /* len */
        stream.out_uint8(4); /* 4 = rdp5 1 = rdp4 */
        stream.out_uint8(0);
        stream.out_uint8(8);
        stream.out_uint8(0);

        uint16_t num_channels = channel_list.size();
        uint16_t padchan = num_channels & 1;

//01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12
//bytes

//04 00 08 00 -> TS_UD_SC_CORE::version = 0x0008004
//00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

//03 0c 10 00 -> TS_UD_HEADER::type = SC_NET (0x0c03), length = 16 bytes

//eb 03 -> TS_UD_SC_NET::MCSChannelID = 0x3eb = 1003 (I/O channel)
//03 00 -> TS_UD_SC_NET::channelCount = 3
//ec 03 -> channel0 = 0x3ec = 1004 (rdpdr)
//ed 03 -> channel1 = 0x3ed = 1005 (cliprdr)
//ee 03 -> channel2 = 0x3ee = 1006 (rdpsnd)
//00 00 -> padding

//02 0c ec 00 -> TS_UD_HEADER::type = SC_SECURITY, length = 236

//02 00 00 00 -> TS_UD_SC_SEC1::encryptionMethod = 128BIT_ENCRYPTION_FLAG
//02 00 00 00 -> TS_UD_SC_SEC1::encryptionLevel = TS_ENCRYPTION_LEVEL_CLIENT_COMPATIBLE
//20 00 00 00 -> TS_UD_SC_SEC1::serverRandomLen = 32 bytes
//b8 00 00 00 -> TS_UD_SC_SEC1::serverCertLen = 184 bytes


        stream.out_uint16_le(SC_NET);
        // length, including tag and length fields
        stream.out_uint16_le(8 + (num_channels + padchan) * 2);
        stream.out_uint16_le(MCS_GLOBAL_CHANNEL);
        stream.out_uint16_le(num_channels); /* number of other channels */

        for (int index = 0; index < num_channels; index++) {
                stream.out_uint16_le(MCS_GLOBAL_CHANNEL + (index + 1));
        }
        if (padchan){
            stream.out_uint16_le(0);
        }

        stream.out_uint16_le(SC_SECURITY);
        stream.out_uint16_le(236); // length, including tag and length fields
        stream.out_uint32_le(rc4_key_size); // key len 1 = 40 bit 2 = 128 bit
        stream.out_uint32_le(client_info->crypt_level); // crypt level 1 = low 2 = medium
        /* 3 = high */
        stream.out_uint32_le(32);  // random len
        stream.out_uint32_le(184); // len of rsa info(certificate)
        stream.out_copy_bytes(this->server_random, 32);
        /* here to end is certificate */
        /* HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ */
        /* TermService\Parameters\Certificate */
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);
        stream.out_uint32_le(1);

        // 96 bytes long of sec_tag pubkey
        send_sec_tag_pubkey(stream, rsa_keys.pub_exp, pub_mod);
        // 76 bytes long of sec_tag_pub_sig
        send_sec_tag_sig(stream, pub_sig);
        /* end certificate */

        assert(offset_len_mcs_connect_response - offset_len_mcs_data == 38);

        #warning create a function in stream that sets differed ber_len_offsets
        // set mcs_data len, BER_TAG_OCTET_STRING (some kind of BLOB)
        stream.set_out_ber_len_uint16(stream.p - stream.data - offset_len_mcs_data - 3, offset_len_mcs_data);
        // set BER_TAG_MCS_CONNECT_RESPONSE len
        stream.set_out_ber_len_uint16(stream.p - stream.data - offset_len_mcs_connect_response - 3, offset_len_mcs_connect_response);

        tpdu.end();
        tpdu.send(trans);
    }

    /*****************************************************************************/
    /* Process crypto information blob */

//    /*****************************************************************************/
//    static void rdp_sec_rsa_op(uint8_t* out, uint8_t* in, uint8_t* mod, uint8_t* exp)
//    {
//        ssl_mod_exp(out, SEC_MODULUS_SIZE, /* 64 */
//                    in, SEC_RANDOM_SIZE, /* 32 */
//                    mod, SEC_MODULUS_SIZE, /* 64 */
//                    exp, SEC_EXPONENT_SIZE); /* 4 */
//    }


    /******************************************************************************/

    /* TODO: this function is not working well because it is stopping copy / paste
       what is required is to stop data from server to client. What we need to do is
       to recover clip_flags, send it to rdp_process_redirect_pdu. After that, we
       need to pass this flags to session_send_to_channel and before doing the
       stream.out_uint8a(data, data_len), we need to do stream.out_uint16_le(clip_flags)*/

    int clipboard_check(char* name, bool clipboard)
    {
      if (!clipboard)
      {
        if (strcmp("cliprdr", name) == 0)
        {
          return 1;
        }
      }
      return 0;
    }

    void send_security_exchange_PDU(Transport * trans, int userid)
    {
        LOG(LOG_INFO, "Iso Layer : setting encryption\n");
        /* Send the client random to the server */
        //      if (this->encryption)
        Stream sdrq_stream(8192);
        X224Out sdrq_tpdu(X224Packet::DT_TPDU, sdrq_stream);
        McsOut sdrq_out(sdrq_stream, MCS_SDRQ, userid, MCS_GLOBAL_CHANNEL);

        sdrq_stream.out_uint32_le(SEC_CLIENT_RANDOM);
        sdrq_stream.out_uint32_le(this->server_public_key_len + SEC_PADDING_SIZE);
        LOG(LOG_INFO, "Server public key is %d bytes long", this->server_public_key_len);
        sdrq_stream.out_copy_bytes(this->client_crypt_random, this->server_public_key_len);
        sdrq_stream.out_clear_bytes(SEC_PADDING_SIZE);

        sdrq_out.end();
        sdrq_tpdu.end();
        sdrq_tpdu.send(trans);
    }

};



#endif
