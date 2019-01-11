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

   MCS Channel Management, complies with T.125
*/


#pragma once

#include <cinttypes>

#include "core/error.hpp"
#include "utils/stream.hpp"
#include "core/RDP/out_per_bstream.hpp"


namespace MCS
{
    enum {
        BER_ENCODING,
        PER_ENCODING
    };

    // Reason ::= ENUMERATED   -- in DisconnectProviderUltimatum, DetachUserRequest, DetachUserIndication
    enum t_reasons {
        RN_DOMAIN_DISCONNECTED = 0,
        RN_PROVIDER_INITIATED  = 1,
        RN_TOKEN_PURGED        = 2,
        RN_USER_REQUESTED      = 3,
        RN_CHANNEL_PURGED      = 4
    };

    inline const char * get_reason(t_reasons reason)
    {
        const char * reasons[5] = {
            "RN_DOMAIN_DISCONNECTED",
            "RN_PROVIDER_INITIATED",
            "RN_TOKEN_PURGED",
            "RN_USER_REQUESTED",
            "RN_CHANNEL_PURGED"
        };
        return (reason > 4)?"???":reasons[reason];
    }

    // Result ::= ENUMERATED   -- in Connect, response, confirm
    enum {
        RT_SUCCESSFUL               = 0,
        RT_DOMAIN_MERGING           = 1,
        RT_DOMAIN_NOT_HIERARCHICAL  = 2,
        RT_NO_SUCH_CHANNEL          = 3,
        RT_NO_SUCH_DOMAIN           = 4,
        RT_NO_SUCH_USER             = 5,
        RT_NOT_ADMITTED             = 6,
        RT_OTHER_USER_ID            = 7,
        RT_PARAMETERS_UNACCEPTABLE  = 8,
        RT_TOKEN_NOT_AVAILABLE      = 9,
        RT_TOKEN_NOT_POSSESSED      = 10,
        RT_TOO_MANY_CHANNELS        = 11,
        RT_TOO_MANY_TOKENS          = 12,
        RT_TOO_MANY_USERS           = 13,
        RT_UNSPECIFIED_FAILURE      = 14,
        RT_USER_REJECTED            = 15
    };

    enum DomainMCSPDU
    {
        MCSPDU_PlumbDomainIndication       = 0x00,
        MCSPDU_ErectDomainRequest          = 0x01,
        MCSPDU_MergeChannelsRequest        = 0x02,
        MCSPDU_MergeChannelsConfirm        = 0x03,
        MCSPDU_PurgeChannelsIndication     = 0x04,
        MCSPDU_MergeTokensRequest          = 0x05,
        MCSPDU_MergeTokensConfirm          = 0x06,
        MCSPDU_PurgeTokensIndication       = 0x07,
        MCSPDU_DisconnectProviderUltimatum = 0x08,
        MCSPDU_RejectMCSPDUUltimatum       = 0x09,
        MCSPDU_AttachUserRequest           = 0x0A,
        MCSPDU_AttachUserConfirm           = 0x0B,
        MCSPDU_DetachUserRequest           = 0x0C,
        MCSPDU_DetachUserIndication        = 0x0D,
        MCSPDU_ChannelJoinRequest          = 0x0E,
        MCSPDU_ChannelJoinConfirm          = 0x0F,
        MCSPDU_ChannelLeaveRequest         = 0x10,
        MCSPDU_ChannelConveneRequest       = 0x11,
        MCSPDU_ChannelConveneConfirm       = 0x12,
        MCSPDU_ChannelDisbandRequest       = 0x13,
        MCSPDU_ChannelDisbandIndication    = 0x14,
        MCSPDU_ChannelAdmitRequest         = 0x15,
        MCSPDU_ChannelAdmitIndication      = 0x16,
        MCSPDU_ChannelExpelRequest         = 0x17,
        MCSPDU_ChannelExpelIndication      = 0x18,
        MCSPDU_SendDataRequest             = 0x19,
        MCSPDU_SendDataIndication          = 0x1A,
        MCSPDU_UniformSendDataRequest      = 0x1B,
        MCSPDU_UniformSendDataIndication   = 0x1C,
        MCSPDU_TokenGrabRequest            = 0x1D,
        MCSPDU_TokenGrabConfirm            = 0x1E,
        MCSPDU_TokenInhibitRequest         = 0x1F,
        MCSPDU_TokenInhibitConfirm         = 0x20,
        MCSPDU_TokenGiveRequest            = 0x21,
        MCSPDU_TokenGiveIndication         = 0x22,
        MCSPDU_TokenGiveResponse           = 0x23,
        MCSPDU_TokenGiveConfirm            = 0x24,
        MCSPDU_TokenPleaseRequest          = 0x25,
        MCSPDU_TokenPleaseIndication       = 0x26,
        MCSPDU_TokenReleaseRequest         = 0x27,
        MCSPDU_TokenReleaseConfirm         = 0x28,
        MCSPDU_TokenTestRequest            = 0x29,
        MCSPDU_TokenTestConfirm            = 0x2A
    };

    enum ConnectMCSPDU {
        MCSPDU_CONNECT_INITIAL             = 101,
        MCSPDU_CONNECT_RESPONSE            = 102,
        MCSPDU_CONNECT_ADDITIONAL          = 103,
        MCSPDU_CONNECT_RESULT              = 104
    };

    static const char* const RT_RESULT[] = {
        "rt-successful",
        "rt-domain-merging",
        "rt-domain-not-hierarchical",
        "rt-no-such-channel",
        "rt-no-such-domain",
        "rt-no-such-user",
        "rt-not-admitted",
        "rt-other-user-id",
        "rt-parameters-unacceptable",
        "rt-token-not-available",
        "rt-token-not-possessed",
        "rt-too-many-channels",
        "rt-too-many-tokens",
        "rt-too-many-users",
        "rt-unspecified-failure",
        "rt-user-rejected"
    };

    inline int peekPerEncodedMCSType(const InStream & stream) {
        if (!stream.in_check_rem(1)){
            throw Error(ERR_MCS);
        }
        return *stream.get_data() >> 2;
    }

    struct InBerStream
    {
        InStream & stream;

        explicit InBerStream(InStream & stream)
        : stream(stream)
        {
        }

        // =========================================================================
        // BER encoding rules support methods
        // =========================================================================

        enum {
            BER_TAG_BOOLEAN      =    1,
            BER_TAG_INTEGER      =    2,
            BER_TAG_OCTET_STRING =    4,
            BER_TAG_RESULT       =   10
        };


        // return string length or -1 on error
        int in_ber_octet_string_with_check(uint8_t * target, uint16_t target_len)
        {
            bool in_result;
            uint8_t tag = this->in_uint8_with_check(in_result);
            if (!in_result) {
                LOG(LOG_ERR, "Truncated BER octet string (need=1, remain=0)");
                return -1;
            }
            if (tag != BER_TAG_OCTET_STRING){
                LOG(LOG_ERR, "Octet string BER tag (%u) expected, got %u", BER_TAG_OCTET_STRING, tag);
                return -1;
            }
            size_t len = this->in_ber_len_with_check(in_result);
            if (!in_result){
                return -1;
            }
            if (!this->in_check_rem(len)){
                LOG(LOG_ERR, "Truncated BER octet string (need=%zu, remain=%zu)",
                    len, this->stream.in_remain());
                return -1;
            }
            if (len > target_len){
                LOG(LOG_ERR, "target string too large (max=%" PRIu16 ", got=%zu)", target_len, len);
                return -1;
            }
            this->stream.in_copy_bytes(target, len);
            return len;
        }

        // return 0 if false, 1 if true, -1 on error
        int in_ber_boolean_with_check()
        {
            bool in_result;
            uint8_t tag = this->in_uint8_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Truncated BER boolean tag (need=1, remain=0)");
                return -1;
            }
            if (tag != BER_TAG_BOOLEAN){
                LOG(LOG_ERR, "Boolean BER tag (%u) expected, got %u", BER_TAG_BOOLEAN, tag);
                return -1;
            }
            size_t len = this->in_ber_len_with_check(in_result);
            if (!in_result){
                return -1;
            }
            if (len != 1){
                LOG(LOG_ERR, "Boolean BER should be one byte");
                return -1;
            }
            if (!this->stream.in_check_rem(1)){
                LOG(LOG_ERR, "Truncated BER boolean value (need=1, remain=0)");
                return -1;
            }
            return this->stream.in_uint8();
        }

        int in_ber_int_with_check(bool & result){
            int v = 0;

            result = true;

            unsigned expected = 2; /* tag(1) + len(1) */
            if (this->stream.in_check_rem(expected)){
               uint8_t tag = this->stream.in_uint8();
               if (tag == BER_TAG_INTEGER){
                   uint8_t len = this->stream.in_uint8();
                   if (this->stream.in_check_rem(len)){
                       v = this->stream.in_bytes_be(len);
                   }
                   else {
                       LOG(LOG_ERR, "Truncated BER integer data (need=%u, remain=%zu)",
                           len, this->stream.in_remain());
                       result = false;
                   }
               }
               else {
                   LOG(LOG_ERR, "Integer BER tag (%u) expected, got %u", BER_TAG_INTEGER, tag);
                   result = false;
               }
            }
            else {
                LOG(LOG_ERR, "Truncated BER integer (need=%u, remain=%zu)",
                    expected, this->stream.in_remain());
                result = false;
            }
            return v;
        }


        //unsigned int in_ber_len() {
        //    uint8_t l = this->stream.in_uint8();
        //    if (l & 0x80) {
        //        const uint8_t nbbytes = (uint8_t)(l & 0x7F);
        //        unsigned int len = 0;
        //        for (uint8_t i = 0 ; i < nbbytes ; i++) {
        //            len = (len << 8) | this->stream.in_uint8();
        //        }
        //        return len;
        //    }
        //    return l;
        //}

        unsigned int in_ber_len_with_check(bool & result) {
            uint8_t l = 0;

            result = true;

            if (this->stream.in_check_rem(1))
            {
                l = this->stream.in_uint8();
                if (l & 0x80) {
                    const uint8_t nbbytes = static_cast<uint8_t>(l & 0x7F);

                    if (this->stream.in_check_rem(nbbytes)){
                        unsigned int len = 0;
                        for (uint8_t i = 0 ; i < nbbytes ; i++) {
                            len = (len << 8) | this->stream.in_uint8();
                        }

                        return len;
                    }

                    LOG(LOG_ERR, "Truncated PER length (need=%u, remain=%zu)",
                        nbbytes, this->stream.in_remain());
                    l = 0;
                    result = false;
                }
            }
            else {
                result = false;
                LOG(LOG_ERR, "Truncated BER length (need=1, remain=0)");
            }
            return l;
        }

        uint16_t in_uint16_be_with_check(bool & result) {
            if (this->in_check_rem(2)){
                result = true;
                return this->stream.in_uint16_be();
            }

            result = false;
            return 0;
        }

        unsigned char in_uint8_with_check(bool & result) {
            if (this->stream.in_check_rem(1)){
                result = true;
                return this->stream.in_uint8();
            }

            result = false;
            return 0;
        }

        unsigned in_bytes_le(const uint8_t nb){
            return this->stream.in_bytes_le(nb);
        }


        bool in_check_rem(const unsigned n) const {
            // returns true if there is enough data available to read n bytes
            return this->stream.in_check_rem(n);
        }

        size_t in_remain() const {
            return this->stream.in_remain();
        }

        uint32_t get_offset() const {
            return this->stream.get_offset();
        }

    };

    struct OutBerStream
    {
        // =========================================================================
        // BER encoding rules support methods
        // =========================================================================

        enum {
            BER_TAG_BOOLEAN      =    1,
            BER_TAG_INTEGER      =    2,
            BER_TAG_OCTET_STRING =    4,
            BER_TAG_RESULT       =   10,
            BER_TAG_MCS_DOMAIN_PARAMS = 0x30
        };

        OutStream & stream;

        explicit OutBerStream(OutStream & stream)
        : stream(stream)
        {
        }

        void out_ber_len(unsigned int v){
            if (v < 0x80){
                this->stream.out_uint8(static_cast<uint8_t>(v));
            }
            else if (v < 0x100) {
                this->stream.out_uint8(0x81);
                this->stream.out_uint8(v);
            }
            else {
                this->stream.out_uint8(0x82);
                this->stream.out_uint16_be(v);
            }
        }

        void out_ber_integer(unsigned int v){
            this->stream.out_uint8(BER_TAG_INTEGER);
            if (v < 0x80) {
                this->stream.out_uint8(1);
                this->stream.out_uint8(static_cast<uint8_t>(v));
            }
            else if (v < 0xfff8) { // Actually ffff should also work, but it would break old code
                this->stream.out_uint8(2);
                this->stream.out_uint8(static_cast<uint8_t>(v >> 8));
                this->stream.out_uint8(static_cast<uint8_t>(v));
            }
            else {
                this->stream.out_uint8(3);
                this->stream.out_uint8(static_cast<uint8_t>(v >> 16));
                this->stream.out_uint8(static_cast<uint8_t>(v >> 8));
                this->stream.out_uint8(static_cast<uint8_t>(v));
            }
        }

        void set_out_ber_len_uint7(unsigned int v, size_t offset){
            if (v >= 0x80) {
                LOG(LOG_INFO, "Value too large for out_ber_len_uint7");
                throw Error(ERR_STREAM_VALUE_TOO_LARGE_FOR_OUT_BER_LEN_UINT7);
            }
            this->stream.set_out_uint8(static_cast<uint8_t>(v), offset+0);
        }

        void out_ber_len_uint7(unsigned int v){
            if (v >= 0x80) {
                LOG(LOG_INFO, "Value too large for out_ber_len_uint7");
                throw Error(ERR_STREAM_VALUE_TOO_LARGE_FOR_OUT_BER_LEN_UINT7);
            }
            this->stream.out_uint8(static_cast<uint8_t>(v));
        }

        void set_out_ber_len_uint16(unsigned int v, size_t offset){
            this->stream.set_out_uint8(0x82, offset+0);
            this->stream.set_out_uint16_be(v, offset+1);
        }

        void out_ber_len_uint16(unsigned int v){
            this->stream.out_uint8(0x82);
            this->stream.out_uint16_be(v);
        }

        uint32_t get_offset() const {
            return this->stream.get_offset();
        }

        void out_uint16_be(unsigned int v) {
            return this->stream.out_uint16_be(v);
        }

        void out_uint8(unsigned char v) {
            return this->stream.out_uint8(v);
        }
    };

    struct CONNECT_INITIAL_Send
    {
        OutBerStream ber_stream;

        CONNECT_INITIAL_Send(OutStream & stream, size_t payload_length, int encoding)
        : ber_stream(stream)
        {
            if (encoding != BER_ENCODING) {
                LOG(LOG_ERR, "Connect Initial::BER_ENCODING mandatory for Connect PDUs");
                throw Error(ERR_MCS);
            }
            this->ber_stream.out_uint16_be(0x7F00|MCSPDU_CONNECT_INITIAL);
            this->ber_stream.out_ber_len_uint16(0); // filled later, 3 bytes

            this->ber_stream.out_uint8(OutBerStream::BER_TAG_OCTET_STRING);
            this->ber_stream.out_ber_len(1); /* calling domain */
            this->ber_stream.out_uint8(1);
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_OCTET_STRING);
            this->ber_stream.out_ber_len(1); /* called domain */
            this->ber_stream.out_uint8(1);
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_BOOLEAN);
            this->ber_stream.out_ber_len(1);
            this->ber_stream.out_uint8(0xff); /* upward flag */

            // target params
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_MCS_DOMAIN_PARAMS);
            this->ber_stream.out_ber_len(26);       // 26 bytes
            this->ber_stream.out_ber_integer(34);     // 3 bytes : max_channels
            this->ber_stream.out_ber_integer(2);      // 3 bytes : max_users
            this->ber_stream.out_ber_integer(0);      // 3 bytes : max_tokens
            this->ber_stream.out_ber_integer(1);      // 3 bytes :
            this->ber_stream.out_ber_integer(0);      // 3 bytes :
            this->ber_stream.out_ber_integer(1);      // 3 bytes :
            this->ber_stream.out_ber_integer(0xffff); // 5 bytes : max_pdu_size
            this->ber_stream.out_ber_integer(2);      // 3 bytes :

            // min params
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_MCS_DOMAIN_PARAMS);
            this->ber_stream.out_ber_len(25);     // 25 bytes
            this->ber_stream.out_ber_integer(1);     // 3 bytes : max_channels
            this->ber_stream.out_ber_integer(1);     // 3 bytes : max_users
            this->ber_stream.out_ber_integer(1);     // 3 bytes : max_tokens
            this->ber_stream.out_ber_integer(1);     // 3 bytes :
            this->ber_stream.out_ber_integer(0);     // 3 bytes :
            this->ber_stream.out_ber_integer(1);     // 3 bytes :
            this->ber_stream.out_ber_integer(0x420); // 4 bytes : max_pdu_size
            this->ber_stream.out_ber_integer(2);     // 3 bytes :

            // max params
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_MCS_DOMAIN_PARAMS);
            this->ber_stream.out_ber_len(31);      // 31 bytes
            this->ber_stream.out_ber_integer(0xffff); // 5 bytes : max_channels
            this->ber_stream.out_ber_integer(0xfc17); // 4 bytes : max_users
            this->ber_stream.out_ber_integer(0xffff); // 5 bytes : max_tokens
            this->ber_stream.out_ber_integer(1);      // 3 bytes :
            this->ber_stream.out_ber_integer(0);      // 3 bytes :
            this->ber_stream.out_ber_integer(1);      // 3 bytes :
            this->ber_stream.out_ber_integer(0xffff); // 5 bytes : max_pdu_size
            this->ber_stream.out_ber_integer(2);      // 3 bytes :

            this->ber_stream.out_uint8(OutBerStream::BER_TAG_OCTET_STRING);
            // We assume here that payload_length is encoded in 2 bytes
            // (length > 0xFFFF will probably bug whereas ber should allow theses values)
            this->ber_stream.out_ber_len_uint16(payload_length); // 3 bytes
            // now we know full MCS Initial header length (without initial tag and len)
            // fill 3 bytes of length at the begining of this function
            this->ber_stream.set_out_ber_len_uint16(payload_length + stream.get_offset() - 5, 2);
        }
    };



// 2.2.1.3 Client MCS Connect Initial PDU with GCC Conference Create Request
// =========================================================================

// The MCS Connect Initial PDU is an RDP Connection Sequence PDU sent from
// client to server during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent after receiving the X.224 Connection Confirm PDU
// (section 2.2.1.2). The MCS Connect Initial PDU encapsulates a GCC Conference
// Create Request, which encapsulates concatenated blocks of settings data. A
// basic high-level overview of the nested structure for the Client MCS Connect
// Initial PDU is illustrated in section 1.3.1.1, in the figure specifying MCS
// Connect Initial PDU. Note that the order of the settings data blocks is
// allowed to vary from that shown in the previously mentioned figure and the
// message syntax layout that follows. This is possible because each data block
// is identified by a User Data Header structure (section 2.2.1.3.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//   section 13.7.

// mcsCi (variable): Variable-length BER-encoded MCS Connect Initial structure
//   (using definite-length encoding) as described in [T125] (the ASN.1
//   structure definition is detailed in [T125] section 7, part 2). The userData
//   field of the MCS Connect Initial encapsulates the GCC Conference Create
//   Request data (contained in the gccCCrq and subsequent fields). The maximum
//   allowed size of this user data is 1024 bytes, which implies that the
//   combined size of the gccCCrq and subsequent fields MUST be less than 1024
//   bytes.

//    Connect-Initial ::= [APPLICATION 101] IMPLICIT SEQUENCE
//    {
//        callingDomainSelector   OCTET STRING,
//        calledDomainSelector    OCTET STRING,
//        upwardFlag              BOOLEAN,
//                                -- TRUE if called provider is higher
//        targetParameters        DomainParameters,
//        minimumParameters       DomainParameters,
//        maximumParameters       DomainParameters,
//        userData                OCTET STRING
//    }

//    DomainParameters ::= SEQUENCE
//    {
//        maxChannelIds   INTEGER (0..MAX),
//                        -- a limit on channel ids in use,
//                        -- static + user id + private + assigned

//        maxUserIds      INTEGER (0..MAX),
//                        -- a sublimit on user id channels alone
//        maxTokenIds     INTEGER (0..MAX),
//                        -- a limit on token ids in use
//                        -- grabbed + inhibited + giving + ungivable + given
//        numPriorities   INTEGER (0..MAX),
//                        -- the number of TCs in an MCS connection
//        minThroughput   INTEGER (0..MAX),
//                        -- the enforced number of octets per second
//        maxHeight       INTEGER (0..MAX),
//                        -- a limit on the height of a provider
//        maxMCSPDUsize   INTEGER (0..MAX),
//                        -- an octet limit on domain MCSPDUs
//        protocolVersion INTEGER (0..MAX)
//    }

// gccCCrq (variable): Variable-length Packed Encoding Rule encoded
//   (PER-encoded) GCC Connect Data structure, which encapsulates a Connect GCC
//   PDU that contains a GCC Conference Create Request structure as described in
//   [T124] (the ASN.1 structure definitions are detailed in [T124] section 8.7)
//   appended as user data to the MCS Connect Initial (using the format
//   described in [T124] sections 9.5 and 9.6). The userData field of the GCC
//   Conference Create Request contains one user data set consisting of
//   concatenated client data blocks.

// clientCoreData (216 bytes): Client Core Data structure (section 2.2.1.3.2).

// clientSecurityData (12 bytes): Client Security Data structure (section
//   2.2.1.3.3).

// clientNetworkData (variable): Optional and variable-length Client Network
//   Data structure (section 2.2.1.3.4).

// clientClusterData (12 bytes): Optional Client Cluster Data structure (section
//   2.2.1.3.5).

// clientMonitorData (variable): Optional Client Monitor Data structure (section
//   2.2.1.3.6). This field MUST NOT be include " if the server does not
//   advertise support for extended client data blocks by using the
//   EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section
//   2.2.1.2.1.

    struct DomainParameters
    {
        int maxChannelIds;
        int maxUserIds;
        int maxTokenIds;
        int numPriorities;
        int minThroughput;
        int maxHeight;
        int maxMCSPDUsize;
        int protocolVersion;

        int recv(InBerStream & ber_stream){
            enum {
                BER_TAG_MCS_DOMAIN_PARAMS = 0x30
            };

            bool in_result;

            uint8_t tag = ber_stream.in_uint8_with_check(in_result);
            if (!in_result){
               LOG(LOG_ERR, "Domain Parameters BER tag expected");
            }
            if (BER_TAG_MCS_DOMAIN_PARAMS != tag){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS (%u) expected, got %u",
                    BER_TAG_MCS_DOMAIN_PARAMS, tag);
                return -1;
            }
            size_t len = ber_stream.in_ber_len_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::len error");
                return -1;
            }
            size_t start_offset = ber_stream.get_offset();
            this->maxChannelIds = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxChannelIds tag error");
                return -1;
            }
            this->maxUserIds = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxUserIds tag error");
                return -1;
            }
            this->maxTokenIds = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxTokenIds tag error");
                return -1;
            }
            this->numPriorities = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::numPriorities tag error");
                return -1;
            }
            this->minThroughput = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::minThroughput tag error");
                return -1;
            }
            this->maxHeight = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxHeight tag error");
                return -1;
            }
            this->maxMCSPDUsize = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxMCSPDUsize tag error");
                return -1;
            }
            this->protocolVersion = ber_stream.in_ber_int_with_check(in_result);
            if (!in_result){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::protocolVersion tag error");
                return -1;
            }
            if (ber_stream.get_offset() != start_offset + len){
                LOG(LOG_ERR, "Connect Initial, bad length in BER_TAG_MCS_DOMAIN_PARAMS. Total subfield length mismatch %zu %zu", ber_stream.get_offset() - start_offset, len);
            }
            return 0;
        }
    };

    struct CONNECT_INITIAL_PDU_Recv
    {
        InBerStream ber_stream;

        size_t _header_size;

        uint16_t tag;
        size_t tag_len;

        uint8_t callingDomainSelector[256];
        int len_callingDomainSelector;

        uint8_t calledDomainSelector[256];
        int len_calledDomainSelector;

        bool upwardFlag;

        DomainParameters targetParameters;
        DomainParameters minimumParameters;
        DomainParameters maximumParameters;

        InStream payload;

        CONNECT_INITIAL_PDU_Recv(InStream & stream, int encoding)
            : ber_stream(stream)
            , tag([this, encoding](){
                // TODO simplify this there is no real use for ber_stream
                if (encoding != BER_ENCODING){
                    LOG(LOG_ERR, "Connect Initial::BER_ENCODING mandatory for Connect PDUs");
                    throw Error(ERR_MCS);
                }

                bool in_result;
                uint16_t tag = this->ber_stream.in_uint16_be_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Initial PDU tag: expected=2, remains=%zu",
                        this->ber_stream.in_remain());
                    throw Error(ERR_MCS);
                }
                if ((0x7F00|MCSPDU_CONNECT_INITIAL) != tag){
                    LOG(LOG_ERR, "Connect Initial::CONNECT_INITIAL tag (0x%04x) expected, got 0x%04x",
                        (0x7F00u|MCSPDU_CONNECT_INITIAL), unsigned(tag));
                    throw Error(ERR_MCS);
                }
                return MCSPDU_CONNECT_INITIAL;

            }())
            , tag_len([this](){
                bool in_result;
                size_t tag_len = this->ber_stream.in_ber_len_with_check(in_result);
                if (!in_result) {
                    LOG(LOG_ERR, "Truncated Connect Initial PDU tag length");
                    throw Error(ERR_MCS);
                }
                return tag_len;
            }())
            , callingDomainSelector{}
            , len_callingDomainSelector([this](){
                int len_callingDomainSelector =
                    this->ber_stream.in_ber_octet_string_with_check(this->callingDomainSelector, sizeof(this->callingDomainSelector));
                if (-1 == len_callingDomainSelector){
                    LOG(LOG_ERR, "Connect Initial::bad callingDomainSelector");
                    throw Error(ERR_MCS);
                }
                return len_callingDomainSelector;
            }())
            , calledDomainSelector{}
            , len_calledDomainSelector([this](){
                len_calledDomainSelector =
                    this->ber_stream.in_ber_octet_string_with_check(this->calledDomainSelector, sizeof(this->calledDomainSelector));
                if (-1 == len_calledDomainSelector){
                    LOG(LOG_ERR, "Connect Initial::bad calledDomainSelector");
                    throw Error(ERR_MCS);
                }
                return len_calledDomainSelector;
            }())
            , upwardFlag([this](){
                // upwardFlag BOOLEAN, -- TRUE if called provider is higher
                int upward = this->ber_stream.in_ber_boolean_with_check();
                if (-1 == upward){
                    LOG(LOG_ERR, "Connect Initial::bad upwardFlag");
                    throw Error(ERR_MCS);
                }
                return upward;
            }())
            , targetParameters([this](){
                DomainParameters targetParameters;
                if (-1 == targetParameters.recv(this->ber_stream)){
                    LOG(LOG_ERR, "Connect Initial::bad targetParameters");
                    throw Error(ERR_MCS);
                }
                return targetParameters;
            }())
            , minimumParameters([this](){
                DomainParameters minimumParameters;
                if (-1 == minimumParameters.recv(this->ber_stream)){
                    LOG(LOG_ERR, "Connect Initial::bad minimumParameters");
                    throw Error(ERR_MCS);
                }
                return minimumParameters;
            }())
            , maximumParameters([this](){
                DomainParameters maximumParameters;
                if (-1 == maximumParameters.recv(this->ber_stream)){
                    LOG(LOG_ERR, "Connect Initial::bad maximumParameters");
                    throw Error(ERR_MCS);
                }
                return maximumParameters;
            }())
            // This is GCC Conference User Data
            , payload([&stream, this]{
                // userData OCTET STRING
                bool in_result;
                uint8_t tag = this->ber_stream.in_uint8_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Initial PDU payload tag: expected=1, remains=0");
                    throw Error(ERR_MCS);
                }
                if (InBerStream::BER_TAG_OCTET_STRING != tag){
                    LOG(LOG_ERR, "ConnectInitial::BER payload tag mismatch, expected BER_TAG_OCTET_STRING(%u), got %u",
                        InBerStream::BER_TAG_OCTET_STRING, tag);
                    throw Error(ERR_MCS);
                }

                size_t payload_size = this->ber_stream.in_ber_len_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Initial PDU payload size");
                    throw Error(ERR_MCS);
                }
                if (payload_size != this->ber_stream.in_remain()){
                    LOG(LOG_ERR, "ConnectInitial::BER payload size (%zu) does not match available data size (%zu)",
                        payload_size, this->ber_stream.in_remain());
                    throw Error(ERR_MCS);
                }
                return InStream(stream.get_current(), payload_size);
            }())
        {
// The payload is the USER_DATA block
            this->_header_size  = this->ber_stream.get_offset();
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };

// 2.2.1.4  Server MCS Connect Response PDU with GCC Conference Create Response
// ----------------------------------------------------------------------------

// From [MSRDPCGR]

// The MCS Connect Response PDU is an RDP Connection Sequence PDU sent from
// server to client during the Basic Settings Exchange phase (see section
// 1.3.1.1). It is sent as a response to the MCS Connect Initial PDU (section
// 2.2.1.3). The MCS Connect Response PDU encapsulates a GCC Conference Create
// Response, which encapsulates concatenated blocks of settings data.

// A basic high-level overview of the nested structure for the Server MCS
// Connect Response PDU is illustrated in section 1.3.1.1, in the figure
// specifying MCS Connect Response PDU. Note that the order of the settings
// data blocks is allowed to vary from that shown in the previously mentioned
// figure and the message syntax layout that follows. This is possible because
// each data block is identified by a User Data Header structure (section
// 2.2.1.4.1).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
// section 13.7.

// mcsCrsp (variable): Variable-length BER-encoded MCS Connect Response
//   structure (using definite-length encoding) as described in [T125]
//   (the ASN.1 structure definition is detailed in [T125] section 7, part 2).
//   The userData field of the MCS Connect Response encapsulates the GCC
//   Conference Create Response data (contained in the gccCCrsp and subsequent
//   fields).

// Result ::= ENUMERATED   -- in Connect, response, confirm
// {
//    rt-successful               (0),
//    rt-domain-merging           (1),
//    rt-domain-not-hierarchical  (2),
//    rt-no-such-channel          (3),
//    rt-no-such-domain           (4),
//    rt-no-such-user             (5),
//    rt-not-admitted             (6),
//    rt-other-user-id            (7),
//    rt-parameters-unacceptable  (8),
//    rt-token-not-available      (9),
//    rt-token-not-possessed      (10),
//    rt-too-many-channels        (11),
//    rt-too-many-tokens          (12),
//    rt-too-many-users           (13),
//    rt-unspecified-failure      (14),
//    rt-user-rejected            (15)
// }

//    Connect-Response ::= [APPLICATION 102] IMPLICIT SEQUENCE
//    {
//        result              Result,
//        calledConnectId     INTEGER (0..MAX),
//                            -- assigned by the called provider
//                            -- to identify additional TCs of
//                            -- the same MCS connection
//        domainParameters    DomainParameters,
//        userData            OCTET STRING
//    }

// gccCCrsp (variable): Variable-length PER-encoded GCC Connect Data structure
//   which encapsulates a Connect GCC PDU that contains a GCC Conference Create
//   Response structure as described in [T124] (the ASN.1 structure definitions
//   are specified in [T124] section 8.7) appended as user data to the MCS
//   Connect Response (using the format specified in [T124] sections 9.5 and
//   9.6). The userData field of the GCC Conference Create Response contains
//   one user data set consisting of concatenated server data blocks.

// serverCoreData (12 bytes): Server Core Data structure (section 2.2.1.4.2).

// serverSecurityData (variable): Variable-length Server Security Data structure
//   (section 2.2.1.4.3).

// serverNetworkData (variable): Variable-length Server Network Data structure
//   (section 2.2.1.4.4).

    struct CONNECT_RESPONSE_PDU_Recv
    {
        InBerStream ber_stream;

        uint16_t tag;
        size_t tag_len;

        uint8_t result;
        int connectId;

        struct DomainParameters domainParameters;

        size_t _header_size;
        InStream payload;

        CONNECT_RESPONSE_PDU_Recv(InStream & stream, int encoding)
            : ber_stream(stream)
            , tag([&encoding, this](){
                    if (encoding != BER_ENCODING){
                        LOG(LOG_ERR, "Connect Response::BER_ENCODING mandatory for Connect PDUs");
                        throw Error(ERR_MCS);
                    }

                    bool in_result;
                    uint16_t tag = this->ber_stream.in_uint16_be_with_check(in_result);
                    if (!in_result){
                        LOG(LOG_ERR, "Truncated Connect Response PDU tag: expected=2, remains=%zu",
                            this->ber_stream.in_remain());
                        throw Error(ERR_MCS);
                    }
                    if ((0x7F00|MCSPDU_CONNECT_RESPONSE) != tag){
                        LOG(LOG_ERR, "Connect Response::CONNECT_RESPONSE tag (0x%04x) expected, got 0x%04x",
                            (0x7F00u|MCSPDU_CONNECT_RESPONSE), tag);
                        throw Error(ERR_MCS);
                    }
                    return MCSPDU_CONNECT_RESPONSE;
                }())
            , tag_len([this](){

                bool in_result;
                size_t tag_len = this->ber_stream.in_ber_len_with_check(in_result);
                if (!in_result) {
                    LOG(LOG_ERR, "Truncated Connect Response PDU tag length");
                    throw Error(ERR_MCS);
                }
                return tag_len;
            }())
            , result([this](){

                bool in_result;
                uint8_t tag = this->ber_stream.in_uint8_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Response PDU tag result: expected=1, remains=0");
                    throw Error(ERR_MCS);
                }
                if (tag != InBerStream::BER_TAG_RESULT) {
                    LOG(LOG_ERR, "Connect Response result tag (%u) expected, got %u",
                        InBerStream::BER_TAG_RESULT, tag);
                    throw Error(ERR_MCS);
                }

                unsigned int len = this->ber_stream.in_ber_len_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Connect Response::bad result length");
                    throw Error(ERR_MCS);
                }
                if (1 != len){
                    LOG(LOG_ERR, "Connect Response::result length should be 1, got %u", len);
                    throw Error(ERR_MCS);
                }
                uint8_t result = this->ber_stream.in_uint8_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Response PDU result: expected=1, remains=0");
                    throw Error(ERR_MCS);
                }
                return result;
            }())
            , connectId([this](){

                bool in_result;
                uint8_t tag = this->ber_stream.in_uint8_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Truncated Connect Response PDU tag integer: expected=1, remains=0");
                    throw Error(ERR_MCS);
                }
                if (tag != InBerStream::BER_TAG_INTEGER) {
                    LOG(LOG_ERR, "Connect Response connectId type Integer (%u) expected, got %u",
                        InBerStream::BER_TAG_INTEGER, tag);
                    throw Error(ERR_MCS);
                }
                size_t len = this->ber_stream.in_ber_len_with_check(in_result);
                if (!in_result){
                    LOG(LOG_ERR, "Connect Response::bad connectId length");
                    throw Error(ERR_MCS);
                }

                if (!this->ber_stream.in_check_rem(len)) {
                    LOG(LOG_ERR, "Truncated Connect Response PDU connectId: expected=%zu, remains=%zu",
                        len, this->ber_stream.in_remain());
                    throw Error(ERR_MCS);
                }
                return this->ber_stream.in_bytes_le(len); /* connect id */
            }())
            , domainParameters([this](){

                    struct DomainParameters domainParameters;
                    if (-1 == domainParameters.recv(this->ber_stream)){
                        LOG(LOG_ERR, "Connect Response::bad domainParameters");
                        throw Error(ERR_MCS);
                    }
                    return domainParameters;
            }())
            , payload([&stream, this](){

                // userData OCTET STRING
                    // TODO Octets below are part of GCC Conference User Data
                    bool in_result;
                    uint8_t tag = this->ber_stream.in_uint8_with_check(in_result);
                    if (!in_result){
                        LOG(LOG_ERR, "Truncated Connect Response PDU payload tag: expected=1, remains=0");
                        throw Error(ERR_MCS);
                    }

                    if (InBerStream::BER_TAG_OCTET_STRING != tag){
                        LOG(LOG_ERR, "ConnectInitial::BER payload tag mismatch, expected BER_TAG_OCTET_STRING(%u), got %u",
                            InBerStream::BER_TAG_OCTET_STRING, tag);
                        throw Error(ERR_MCS);
                    }

                    size_t payload_size = this->ber_stream.in_ber_len_with_check(in_result);
                    if (!in_result){
                        LOG(LOG_ERR, "Connect Response::bad connectId length");
                        throw Error(ERR_MCS);
                    }

                    if (payload_size != this->ber_stream.in_remain()){
                        LOG(LOG_ERR, "ConnectResponse::BER payload size (%zu) does not match available data size (%zu)",
                            payload_size, this->ber_stream.in_remain());
                        throw Error(ERR_MCS);
                    }

                    return InStream(stream.get_current(), payload_size);
            }())
        {
            if (this->result){
                LOG(LOG_ERR, "Check Result Error (0x%02X): %s", this->result, RT_RESULT[this->result]);
                throw Error(ERR_MCS);
            }
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };

    struct CONNECT_RESPONSE_Send
    {
        OutBerStream ber_stream;

        CONNECT_RESPONSE_Send(OutStream & stream, size_t payload_length, int encoding)
        : ber_stream(stream)
        {
            if (encoding != BER_ENCODING){
                LOG(LOG_ERR, "Connect Response::BER_ENCODING mandatory for Connect PDUs");
                throw Error(ERR_MCS);
           }
            // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
            this->ber_stream.out_uint16_be(0x7F00|MCSPDU_CONNECT_RESPONSE);
            // BER: Type Length
            if (payload_length > 88){
                this->ber_stream.out_ber_len_uint16(0);
            }
            else {
                this->ber_stream.out_ber_len_uint7(0);
            }
            uint16_t start_offset = this->ber_stream.get_offset();

            // Connect-Response::result = rt-successful (0)
            // The first byte (0x0a) is the ASN.1 BER encoded Enumerated type. The
            // length of the value is given by the second byte (1 byte), and the
            // actual value is 0 (rt-successful).
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_RESULT);
            this->ber_stream.out_ber_len_uint7(1);
            this->ber_stream.out_uint8(0);

            // Connect-Response::calledConnectId = 0
            this->ber_stream.out_ber_integer(0); // 3 bytes

            // Connect-Response::domainParameters (26 bytes)
            this->ber_stream.out_uint8(OutBerStream::BER_TAG_MCS_DOMAIN_PARAMS);
            this->ber_stream.out_ber_len_uint7(26);
            this->ber_stream.out_ber_integer(34);          // DomainParameters::maxChannelIds = 34
            this->ber_stream.out_ber_integer(3);           // DomainParameters::maxUserIds = 3
            this->ber_stream.out_ber_integer(0);           // DomainParameters::maximumTokenIds = 0
            this->ber_stream.out_ber_integer(1);           // DomainParameters::numPriorities = 1
            this->ber_stream.out_ber_integer(0);           // DomainParameters::minThroughput = 0
            this->ber_stream.out_ber_integer(1);           // DomainParameters::maxHeight = 1
            this->ber_stream.out_ber_integer(0xfff8);      // DomainParameters::maxMCSPDUsize = 65528
            this->ber_stream.out_ber_integer(2);           // DomainParameters::protocolVersion = 2

            this->ber_stream.out_uint8(OutBerStream::BER_TAG_OCTET_STRING);
            this->ber_stream.out_ber_len(payload_length);

            // now we know full MCS Initial header length (without initial tag and len)
            if (payload_length > 88){
                this->ber_stream.set_out_ber_len_uint16(payload_length + this->ber_stream.get_offset() - start_offset, 2);
            }
            else {
                this->ber_stream.set_out_ber_len_uint7(payload_length + this->ber_stream.get_offset() - start_offset, 2);
            }
        }
    };

//    PlumbDomainIndication ::= [APPLICATION 0] IMPLICIT SEQUENCE
//    {
//        heightLimit     INTEGER (0..MAX)
//                        -- a restriction on the MCSPDU receiver
//    }
    struct PlumbDomainIndication_Send
    {
        [[noreturn]] PlumbDomainIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct PlumbDomainIndication_Recv
    {
        [[noreturn]] PlumbDomainIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//   2.2.1.5 Client MCS Erect Domain Request PDU
//   -------------------------------------------
//   The MCS Erect Domain Request PDU is an RDP Connection Sequence PDU sent
//   from client to server during the Channel Connection phase (see section
//   1.3.1.1). It is sent after receiving the MCS Connect Response PDU (section
//   2.2.1.4).

//   tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

//   x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//      section 13.7.

// See description of tpktHeader and x224 Data TPDU in cheat sheet

//   mcsEDrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an MCS
//      Erect Domain Request structure, as specified in [T125] (the ASN.1
//      structure definitions are given in [T125] section 7, parts 3 and 10).

//    ErectDomainRequest ::= [APPLICATION 1] IMPLICIT SEQUENCE
//    {
//        subHeight   INTEGER (0..MAX),
//                    -- height in domain of the MCSPDU transmitter
//        subInterval INTEGER (0..MAX)
//                    -- its throughput enforcement interval in milliseconds
//    }

    struct ErectDomainRequest_Send
    {
        ErectDomainRequest_Send(OutStream & stream, uint32_t subheight, uint32_t subinterval, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ErectDomainRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8((MCSPDU_ErectDomainRequest << 2));
            out_per_integer(stream, subheight); /* subHeight (INTEGER) */
            out_per_integer(stream, subinterval); /* subInterval (INTEGER) */
        }
    };

    struct ErectDomainRequest_Recv
    {
        uint8_t type;
        uint32_t subHeight;
        uint32_t subInterval;

        ErectDomainRequest_Recv(InStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ErectDomainRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            if (!stream.in_check_rem(1)){
                LOG(LOG_ERR, "Truncated ErectDomainRequest need 1, got 0");
                throw Error(ERR_MCS);
            }
            uint8_t tag = stream.in_uint8();

            if ((MCS::MCSPDU_ErectDomainRequest << 2) != tag){
                LOG(LOG_ERR, "ErectDomainRequest tag (%d) expected, got %u",
                    (MCS::MCSPDU_ErectDomainRequest << 2), tag);
                throw Error(ERR_MCS);
            }
            this->type = MCS::MCSPDU_ErectDomainRequest;

            {
                if (!stream.in_check_rem(2)) {
                    LOG(LOG_ERR, "ErectDomainRequest not enough data for subHeight len : (need 2, available %zu)", stream.in_remain());
                    throw Error(ERR_MCS);
                }
                uint16_t len = stream.in_2BUE();
                // case len = 0 is theoretically forbidden but rdesktop send that (treat it as 1)
                if (len == 0) {
                    len = 1;
                }
                if (len > 4) {
                    LOG(LOG_ERR, "ErectDomainRequest bad subHeight");
                    throw Error(ERR_MCS);
                }
                if (!stream.in_check_rem(len)) {
                    LOG(LOG_ERR, "ErectDomainRequest bad subHeight");
                    throw Error(ERR_MCS);
                }
                this->subHeight = stream.in_bytes_be(len);
            }
            {
                if (!stream.in_check_rem(2)) {
                    LOG(LOG_ERR, "ErectDomainRequest not enough data for subInterval len : (need 2, available %zu)", stream.in_remain());
                    throw Error(ERR_MCS);
                }
                uint16_t len = stream.in_2BUE();
                // case len = 0 is theoretically forbidden but rdesktop send that (treat it as 1)
                if (len == 0) {
                    len = 1;
                }
                if (len > 4) {
                    LOG(LOG_ERR, "ErectDomainRequest bad subInterval");
                    throw Error(ERR_MCS);
                }
                if (!stream.in_check_rem(len)) {
                    LOG(LOG_ERR, "ErectDomainRequest bad subInterval");
                    throw Error(ERR_MCS);
                }
                this->subInterval = stream.in_bytes_be(len);
            }
        }
    };

//    ChannelAttributes ::= CHOICE
//    {
//        static [0] IMPLICIT SEQUENCE
//        {
//            channelId   StaticChannelId
//                        -- joined is implicitly TRUE
//        },

//        userId  [1] IMPLICIT SEQUENCE
//        {
//            joined      BOOLEAN,
//                        -- TRUE if user is joined to its user id
//            userId      UserId
//        },

//        private [2] IMPLICIT SEQUENCE
//        {
//            joined      BOOLEAN,
//                        -- TRUE if channel id is joined below
//            channelId   PrivateChannelId,
//            manager     UserId,
//            admitted    SET OF UserId
//                        -- may span multiple MergeChannelsRequest
//        },

//        assigned [3] IMPLICIT SEQUENCE
//        {
//            channelId   AssignedChannelId
//                        -- joined is implicitly TRUE
//        }
//    }

//    MergeChannelsRequest ::= [APPLICATION 2] IMPLICIT SEQUENCE
//    {
//        mergeChannels   SET OF ChannelAttributes,
//        purgeChannelIds SET OF ChannelId
//    }

    struct MergeChannelRequest_Send
    {
        [[noreturn]] MergeChannelRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct MergeChannelRequest_Recv
    {
        [[noreturn]] MergeChannelRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    MergeChannelsConfirm ::= [APPLICATION 3] IMPLICIT SEQUENCE
//    {
//        mergeChannels   SET OF ChannelAttributes,
//        purgeChannelIds SET OF ChannelId
//    }

    struct MergeChannelsConfirm_Send
    {
        [[noreturn]] MergeChannelsConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct MergeChannelsConfirm_Recv
    {
        [[noreturn]] MergeChannelsConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    PurgeChannelsIndication ::= [APPLICATION 4] IMPLICIT SEQUENCE
//    {
//        detachUserIds       SET OF UserId,
//                            -- purge user id channels
//        purgeChannelIds     SET OF ChannelId
//                            -- purge other channels
//    }
    struct PurgeChannelsIndication_Send
    {
        [[noreturn]] PurgeChannelsIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct PurgeChannelsIndication_Recv
    {
        [[noreturn]] PurgeChannelsIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenAttributes ::= CHOICE
//    {
//        grabbed [0] IMPLICIT SEQUENCE
//        {
//            tokenId     TokenId,
//            grabber     UserId
//        },

//        inhibited [1] IMPLICIT SEQUENCE
//        {
//            tokenId     TokenId,
//            inhibitors  SET OF UserId
//                        -- may span multiple MergeTokensRequest
//        },

//        giving [2] IMPLICIT SEQUENCE
//        {
//            tokenId     TokenId,
//            grabber     UserId,
//            recipient   UserId
//        },

//        ungivable [3] IMPLICIT SEQUENCE
//        {
//            tokenId     TokenId,
//            grabber     UserId
//                        -- recipient has since detached
//        },

//        given [4] IMPLICIT SEQUENCE
//        {
//            tokenId         TokenId,
//            recipient       UserId
//                            -- grabber released or detached
//        }
//    }

//    MergeTokensRequest ::= [APPLICATION 5] IMPLICIT SEQUENCE
//    {
//        mergeTokens     SET OF TokenAttributes,
//        purgeTokenIds   SET OF TokenId
//    }

    struct MergeTokensRequest_Send
    {
        [[noreturn]] MergeTokensRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct MergeTokensRequest_Recv
    {
        [[noreturn]] MergeTokensRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    MergeTokensConfirm ::= [APPLICATION 6] IMPLICIT SEQUENCE
//    {
//        mergeTokens     SET OF TokenAttributes,
//        purgeTokenIds   SET OF TokenId
//    }

    struct MergeTokensConfirm_Send
    {
        [[noreturn]] MergeTokensConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct MergeTokensConfirm_Recv
    {
        [[noreturn]] MergeTokensConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    PurgeTokensIndication ::= [APPLICATION 7] IMPLICIT SEQUENCE
//    {
//        purgeTokenIds   SET OF TokenId
//    }

    struct PurgeTokensIndication_Send
    {
        [[noreturn]] PurgeTokensIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct PurgeTokensIndication_Recv
    {
        [[noreturn]] PurgeTokensIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    DisconnectProviderUltimatum ::= [APPLICATION 8] IMPLICIT SEQUENCE
//    {
//        reason          Reason
//    }

    struct DisconnectProviderUltimatum_Send
    {
        DisconnectProviderUltimatum_Send(OutStream & stream, uint8_t reason, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "DisconnectProviderUltimatum PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            uint16_t data = ( (MCS::MCSPDU_DisconnectProviderUltimatum << 10)
                            | (reason << 7)
                            );
            stream.out_uint16_be(data);
        }
    };

    struct DisconnectProviderUltimatum_Recv
    {
        uint8_t type;
        t_reasons reason;

        DisconnectProviderUltimatum_Recv(InStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "DisconnectProviderUltimatum PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            const unsigned expected = 2; /* tag(1) + reason(1) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated DisconnectProviderUltimatum: expected=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS);
            }

            uint16_t tag = stream.in_uint16_be();
            if ((tag >> 10) != MCS::MCSPDU_DisconnectProviderUltimatum) {
                LOG(LOG_ERR, "DisconnectProviderUltimatum tag (%u) expected, got %d",
                   MCS::MCSPDU_DisconnectProviderUltimatum, (tag >> 10));
                throw Error(ERR_MCS);
            }
            this->type   = MCS::MCSPDU_DisconnectProviderUltimatum;
            this->reason = static_cast<t_reasons>((tag & 0x0380) >> 7);
        }
    };

//    RejectMCSPDUUltimatum ::= [APPLICATION 9] IMPLICIT SEQUENCE
//    {
//        diagnostic      Diagnostic,
//        initialOctets   OCTET STRING
//    }

    struct RejectMCSPDUUltimatum_Send
    {
        [[noreturn]] RejectMCSPDUUltimatum_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct RejectMCSPDUUltimatum_Recv
    {
        [[noreturn]] RejectMCSPDUUltimatum_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

// 2.2.1.6 Client MCS Attach User Request PDU
// ------------------------------------------
// The MCS Attach User Request PDU is an RDP Connection Sequence PDU
// sent from client to server during the Channel Connection phase (see
// section 1.3.1.1) to request a user channel ID. It is sent after
// transmitting the MCS Erect Domain Request PDU (section 2.2.1.5).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//   [X224] section 13.7.

// See description of tpktHeader and x224 Data TPDU in cheat sheet

// mcsAUrq (1 byte): PER-encoded MCS Domain PDU which encapsulates an
//  MCS Attach User Request structure, as specified in [T125] (the ASN.1
//  structure definitions are given in [T125] section 7, parts 5 and 10).

// 11.17 AttachUserRequest
// -----------------------

// AttachUserRequest is generated by an MCS-ATTACH-USER request. It rises to the
// top MCS provider, which returns an AttachUserConfirm reply. If the domain
// limit on number of user ids allows, a new user id is generated.

// AttachUserRequest contains no information other than its MCSPDU type. The
// domain to which the user attaches is determined by the MCS connection
// conveying the MCSPDU. The only initial characteristic of the user id
// generated is its uniqueness. An MCS provider shall make a record of each
// unanswered AttachUserRequest received and by which MCS connection it arrived,
// so that a replying AttachUserConfirm can be routed back to the same source.
// To distribute replies fairly, each provider should maintain a first-in,
// first-out queue for this purpose.

//    AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
//    {
//    }

    struct AttachUserRequest_Send
    {
        AttachUserRequest_Send(OutStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "AttachUserRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8(MCS::MCSPDU_AttachUserRequest << 2);
        }
    };

    struct AttachUserRequest_Recv
    {
        uint8_t type;

        AttachUserRequest_Recv(InStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "AttachUserRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            if (!stream.in_check_rem(1)){
                LOG(LOG_ERR, "Truncated AttachUserRequest: expected=1, remains=0");
                throw Error(ERR_MCS);
            }

            uint8_t tag = stream.in_uint8();
            if ((MCS::MCSPDU_AttachUserRequest << 2) != tag){
                LOG(LOG_ERR, "AttachUserRequest tag (%d) expected, got %u", MCS::MCSPDU_AttachUserRequest << 2, tag);
                throw Error(ERR_MCS);
            }
            this->type = MCS::MCSPDU_AttachUserRequest;
        }
    };

// 2.2.1.7 Server MCS Attach User Confirm PDU
// ------------------------------------------
// The MCS Attach User Confirm PDU is an RDP Connection Sequence
// PDU sent from server to client during the Channel Connection
// phase (see section 1.3.1.1). It is sent as a response to the MCS
// Attach User Request PDU (section 2.2.1.6).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
//   section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//   section [X224] 13.7.

// mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
//   an MCS Attach User Confirm structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in [T125] section 7,
// parts 5 and 10).

// AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
// {
//     result       Result,
//     initiator    UserId OPTIONAL
// }

// 11.18 AttachUserConfirm
// -----------------------

// AttachUserConfirm is generated at the top MCS provider upon receipt of
// AttachUserRequest. Routed back to the requesting provider, it generates an
//  MCS-ATTACH-USER confirm.

//      Table 11-18/T.125 – AttachUserConfirm MCSPDU
// +----------------------+-----------------+------------+
// |     Contents         |      Source     |    Sink    |
// +----------------------+-----------------+------------+
// | Result               | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+
// | Initiator (optional) | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+

// AttachUserConfirm contains a user id if and only if the result is successful.
// Providers that receive a successful AttachUserConfirm shall enter the user id
// into their information base. MCS providers shall route AttachUserConfirm to
// the source of an antecedent AttachUserRequest, using the knowledge that
// there is a one-to-one reply. A provider that transmits AttachUserConfirm
// shall note to which downward MCS connection the new user id is thereby
// assigned, so that it may validate the user id when it arises later in other
// requests.

//            stream.out_uint8(PER_DomainMCSPDU_CHOICE_AttachUserConfirm | 2);
//            stream.out_uint8(0); // result OK
//            stream.out_uint16_be(user_id);

// 2.2.1.7 Server MCS Attach User Confirm PDU
// ------------------------------------------
// The MCS Attach User Confirm PDU is an RDP Connection Sequence
// PDU sent from server to client during the Channel Connection
// phase (see section 1.3.1.1). It is sent as a response to the MCS
// Attach User Request PDU (section 2.2.1.6).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
//   section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//   section [X224] 13.7.

// mcsAUcf (4 bytes): PER-encoded MCS Domain PDU which encapsulates
//   an MCS Attach User Confirm structure, as specified in [T125]
//   (the ASN.1 structure definitions are given in [T125] section 7,
// parts 5 and 10).

// AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
// {
//     result       Result,
//     initiator    UserId OPTIONAL
// }

// 11.18 AttachUserConfirm
// -----------------------

// AttachUserConfirm is generated at the top MCS provider upon receipt of
// AttachUserRequest. Routed back to the requesting provider, it generates an
//  MCS-ATTACH-USER confirm.

//      Table 11-18/T.125 – AttachUserConfirm MCSPDU
// +----------------------+-----------------+------------+
// |     Contents         |      Source     |    Sink    |
// +----------------------+-----------------+------------+
// | Result               | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+
// | Initiator (optional) | Top provider    |  Confirm   |
// +----------------------+-----------------+------------+

// AttachUserConfirm contains a user id if and only if the result is successful.
// Providers that receive a successful AttachUserConfirm shall enter the user id
// into their information base. MCS providers shall route AttachUserConfirm to
// the source of an antecedent AttachUserRequest, using the knowledge that
// there is a one-to-one reply. A provider that transmits AttachUserConfirm
// shall note to which downward MCS connection the new user id is thereby
// assigned, so that it may validate the user id when it arises later in other
// requests.

//    AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
//    {
//        result          Result,
//        initiator       UserId OPTIONAL
//    }

    struct AttachUserConfirm_Send
    {
        AttachUserConfirm_Send(OutStream & stream, uint8_t result, bool initiator_flag, uint16_t initiator, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "AttachUserConfirm PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8((MCS::MCSPDU_AttachUserConfirm << 2) | initiator_flag * 2);
            stream.out_uint8(result);
            if (initiator_flag){
                stream.out_uint16_be(initiator);
            }
        }
    };

    struct AttachUserConfirm_Recv
    {
        uint8_t type;
        uint8_t result;
        bool initiator_flag;
        uint16_t initiator;

        AttachUserConfirm_Recv(InStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "AttachUserConfirm PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            const unsigned expected = 2; /* tag(1) + result(1) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated AttachUserConfirm: expected=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS);
            }

            uint8_t tag = stream.in_uint8();
            this->initiator_flag = (tag & 2) != 0;
            if ((tag & 0xFC) != MCS::MCSPDU_AttachUserConfirm << 2){
                LOG(LOG_ERR, "AttachUserConfirm tag (%d) expected, got %u",
                    MCS::MCSPDU_AttachUserConfirm << 2, (tag & 0xFCu));
                throw Error(ERR_MCS);
            }
            this->type = MCS::MCSPDU_AttachUserConfirm;
            this->result = stream.in_uint8();
            if (this->initiator_flag){
                if (!stream.in_check_rem(2)){
                   LOG(LOG_ERR, "Truncated AttachUserConfirm indicator: expected=2, remains=%zu",
                       stream.in_remain());
                   throw Error(ERR_MCS);
                }
                this->initiator = stream.in_uint16_be();
            }
        }
    };

//    DetachUserRequest ::= [APPLICATION 12] IMPLICIT SEQUENCE
//    {
//        reason          Reason,
//        userIds         SET OF UserId
//    }

    struct DetachUserRequest_Send
    {
        [[noreturn]] DetachUserRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct DetachUserRequest_Recv
    {
        [[noreturn]] DetachUserRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    DetachUserIndication ::= [APPLICATION 13] IMPLICIT SEQUENCE
//    {
//        reason          Reason,
//        userIds         SET OF UserId
//    }

    struct DetachUserIndication_Send
    {
        [[noreturn]] DetachUserIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct DetachUserIndication_Recv
    {
        [[noreturn]] DetachUserIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };



// 2.2.1.8 Client MCS Channel Join Request PDU
// -------------------------------------------
// The MCS Channel Join Request PDU is an RDP Connection Sequence PDU sent
// from client to server during the Channel Connection phase (see section
// 1.3.1.1). It is sent after receiving the MCS Attach User Confirm PDU
// (section 2.2.1.7). The client uses the MCS Channel Join Request PDU to
// join the user channel obtained from the Attach User Confirm PDU, the
// I/O channel and all of the static virtual channels obtained from the
// Server Network Data structure (section 2.2.1.4.4).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//                     section 13.7.

// mcsCJrq (5 bytes): PER-encoded MCS Domain PDU which encapsulates an
//                    MCS Channel Join Request structure as specified in
//                    [T125] sections 10.19 and I.3 (the ASN.1 structure
//                    definitions are given in [T125] section 7, parts 6
//                    and 10).

//    ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId
//                        -- may be zero
//    }


// 11.21 ChannelJoinRequest
// ------------------------

// ChannelJoinRequest is generated by an MCS-CHANNEL-JOIN request. If valid, it
// rises until it reaches an MCS provider with enough information to generate a
// ChannelJoinConfirm reply. This may be the top MCS provider.

// Table 11-21/T.125 – ChannelJoinRequest MCSPDU
// +-----------------+-------------------------------+------------------------+
// | Contents        |           Source              |           Sink         |
// +-----------------+-------------------------------+------------------------+
// | Initiator       |      Requesting Provider      |       Higher provider  |
// +-----------------+-------------------------------+------------------------+
// | Channel Id      |      Request                  |       Higher provider  |
// +-----------------+-------------------------------+------------------------+

// The user id of the initiating MCS attachment is supplied by the MCS provider
// that receives the primitive request. Providers that receive
// ChannelJoinRequest subsequently shall validate the user id to ensure that it
// is legitimately assigned to the subtree of origin. If the user id is invalid,
// the MCSPDU shall be ignored.

// NOTE – This allows for the possibility that ChannelJoinRequest may be racing
// upward against a purge of the initiating user id flowing down. A provider
// that receives PurgeChannelsIndication first might receive a
// ChannelJoinRequest soon thereafter that contains an invalid user id. This is
// a normal occurrence and is not cause for rejecting the MCSPDU.

// ChannelJoinRequest may rise to an MCS provider that has the requested channel
// id in its information base. Any such provider, being consistent with the top
// MCS provider, will agree whether the request should succeed. If the request
// should fail, the provider shall generate an unsuccessful ChannelJoinConfirm.
// If it should succeed and the provider is already joined to the same channel,
// the provider shall generate a successful ChannelJoinConfirm. In these two
// cases, MCS-CHANNEL-JOIN completes without necessarily visiting the top MCS
// provider. Otherwise, if the request should succeed but the channel is not yet
// joined, a provider shall forward ChannelJoinRequest upward.

// If ChannelJoinRequest rises to the top MCS provider, the channel id
// requested may be zero, which is in no information base because it is an
// invalid id. If the domain limit on the number of channels in use allows,
// a new assigned channel id shall be generated and returned in a successful
// ChannelJoinConfirm. If the channel id requested is in the static range and
// the domain limit on the number of channels in use allows, the channel id
// shall be entered into the information base and shall likewise be returned
// in a successful ChannelJoinConfirm.

// Otherwise, the request will succeed only if the channel id is already in the
// information base of the top MCS provider. A user id channel can only be
// joined by the same user. A private channel id can be joined only by users
// previously admitted by its manager. An assigned channel id can be joined
// by any user.

    struct ChannelJoinRequest_Send
    {
        ChannelJoinRequest_Send(OutStream & stream, uint16_t initiator, uint16_t channelId, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ChannelJoinRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8(MCS::MCSPDU_ChannelJoinRequest << 2);
            stream.out_uint16_be(initiator);
            stream.out_uint16_be(channelId);
        }
    };

    struct ChannelJoinRequest_Recv
    {
        uint8_t type;
        uint16_t initiator;
        uint16_t channelId;

        ChannelJoinRequest_Recv(InStream & stream, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ChannelJoinRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            const unsigned expected = 5; /* tag(1) + initiator(2) + channelId(2) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated ChannelJoinRequest: expected=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS);
            }

            uint8_t tag = stream.in_uint8();
            if (tag != (MCS::MCSPDU_ChannelJoinRequest << 2)){
                LOG(LOG_ERR, "ChannelJoinRequest tag (%d) expected, got %u", MCS::MCSPDU_ChannelJoinRequest << 2, tag);
                throw Error(ERR_MCS);
            }
            this->type = MCS::MCSPDU_ChannelJoinRequest;
            this->initiator = stream.in_uint16_be();
            this->channelId = stream.in_uint16_be();
        }
    };


// 2.2.1.9 Server MCS Channel Join Confirm PDU
// -------------------------------------------
// The MCS Channel Join Confirm PDU is an RDP Connection Sequence
// PDU sent from server to client during the Channel Connection
// phase (see section 1.3.1.1). It is sent as a response to the MCS
// Channel Join Request PDU (section 2.2.1.8).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123]
//   section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in
//  [X224] section 13.7.

// mcsCJcf (8 bytes): PER-encoded MCS Domain PDU which encapsulates
//  an MCS Channel Join Confirm PDU structure, as specified in
//  [T125] (the ASN.1 structure definitions are given in [T125]
//  section 7, parts 6 and 10).

//    ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
//    {
//        result          Result,
//        initiator       UserId,
//        requested       ChannelId,
//                        -- may be zero
//        channelId       ChannelId OPTIONAL
//    }

// 11.22 ChannelJoinConfirm
// ------------------------

// ChannelJoinConfirm is generated at a higher MCS provider upon receipt of
// ChannelJoinRequest. Routed back to the requesting provider, it generates an
// MCS-CHANNEL-JOIN confirm.

// Table 11-22/T.125 – ChannelJoinConfirm MCSPDU
// +-----------------------+------------------------+--------------------------+
// | Contents              |       Source           |         Sink             |
// +-----------------------+------------------------+--------------------------+
// | Result                |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+
// | Initiator             |   Higher provider      |        MCSPDU routing    |
// +-----------------------+------------------------+--------------------------+
// | Requested             |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+
// | Channel Id (optional) |   Higher provider      |        Confirm           |
// +-----------------------+------------------------+--------------------------+


// ChannelJoinConfirm contains a joined channel id if and only if the result is
// successful.

// The channel id requested is the same as in ChannelJoinRequest. This helps
// the initiating attachment relate MCS-CHANNEL-JOIN confirm to an antecedent
// request. Since ChannelJoinRequest need not rise to the top provider,
// confirms may occur out of order.

// If the result is successful, ChannelJoinConfirm joins the receiving MCS
// provider to the specified channel. Thereafter, higher providers shall route
// to it any data that users send over the channel. A provider shall remain
// joined to a channel as long as any of its attachments or subordinate
// providers does. To leave the channel, a provider shall generate
// ChannelLeaveRequest.

// Providers that receive a successful ChannelJoinConfirm shall enter the
// channel id into their information base. If not already there, the channel id
// shall be given type static or assigned, depending on its range.

// ChannelJoinConfirm shall be forwarded in the direction of the initiating user
// id. If the user id is unreachable because an MCS connection no longer exists,
// the provider shall decide whether it has reason to remain joined to the
// channel. If not, it shall generate ChannelLeaveRequest.


    struct ChannelJoinConfirm_Send
    {
        ChannelJoinConfirm_Send(OutStream & stream
                               , uint8_t result
                               , uint16_t initiator
                               , uint16_t requested
                               , bool channelId_flag
                               , uint16_t channelId
                               , int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ChannelJoinConfirm PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8((MCSPDU_ChannelJoinConfirm << 2) | 2 * channelId_flag);
            stream.out_uint8(result); // Result = rt_successfull
            stream.out_uint16_be(initiator);
            stream.out_uint16_be(requested);
            if (channelId_flag){
                stream.out_uint16_be(channelId);
            }
        }
    };

    struct ChannelJoinConfirm_Recv
    {
        uint8_t type;
        uint8_t result;
        uint16_t initiator;
        uint16_t requested; // -- may be zero
        bool channelId_flag;
        uint16_t channelId;

        ChannelJoinConfirm_Recv(InStream & stream, int encoding)
        : type(MCS::MCSPDU_ChannelJoinConfirm)
        , result(0)
        , initiator(0)
        , requested(0)
        , channelId_flag(false)
        , channelId(0)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "ChannelJoinConfirm PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }

            const unsigned expected = 6; /* tag(1) + result(1) + initiator(2) + requested(2) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated ChannelJoinConfirm: expected=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_MCS);
            }

            uint8_t tag = stream.in_uint8();
            if ((tag & 0xFC) != (MCS::MCSPDU_ChannelJoinConfirm << 2)){
                LOG(LOG_ERR, "ChannelJoinConfirm tag (%d) expected, got %u", MCS::MCSPDU_ChannelJoinConfirm << 2, (tag & 0xFCu));
                throw Error(ERR_MCS);
            }
            this->result    = stream.in_uint8();
            this->initiator = stream.in_uint16_be();
            this->requested = stream.in_uint16_be();
            this->channelId_flag = (tag & 2) != 0;
            if ((tag & 2) && this->result){
                LOG(LOG_ERR, "ChannelJoinConfirm provided a channel while result is negative (%u)", this->result);
                throw Error(ERR_MCS);
            }
            if (this->channelId_flag){
                if (!stream.in_check_rem(2)){
                   LOG(LOG_ERR, "Truncated ChannelJoinConfirm indicator: expected=2, remains=%zu",
                       stream.in_remain());
                   throw Error(ERR_MCS);
                }
                this->channelId = stream.in_uint16_be();
            }
        }
    };

//    ChannelLeaveRequest ::= [APPLICATION 16] IMPLICIT SEQUENCE
//    {
//        channelIds      SET OF ChannelId
//    }

    struct ChannelLeaveRequest_Send
    {
        [[noreturn]] ChannelLeaveRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelLeaveRequest_Recv
    {
        [[noreturn]] ChannelLeaveRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelConveneRequest ::= [APPLICATION 17] IMPLICIT SEQUENCE
//    {
//        initiator       UserId
//    }

    struct ChannelConveneRequest_Send
    {
        [[noreturn]] ChannelConveneRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelConveneRequest_Recv
    {
        [[noreturn]] ChannelConveneRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelConveneConfirm ::= [APPLICATION 18] IMPLICIT SEQUENCE
//    {
//        result          Result,
//        initiator       UserId,
//        channelId       PrivateChannelId OPTIONAL
//    }

    struct ChannelConveneConfirm_Send
    {
        [[noreturn]] ChannelConveneConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelConveneConfirm_Recv
    {
        [[noreturn]] ChannelConveneConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelDisbandRequest ::= [APPLICATION 19] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       PrivateChannelId
//    }

    struct ChannelDisbandRequest_Send
    {
        [[noreturn]] ChannelDisbandRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelDisbandRequest_Recv
    {
        [[noreturn]] ChannelDisbandRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelDisbandIndication ::= [APPLICATION 20] IMPLICIT SEQUENCE
//    {
//        channelId       PrivateChannelId
//    }

    struct ChannelDisbandIndication_Send
    {
        [[noreturn]] ChannelDisbandIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelDisbandIndication_Recv
    {
        [[noreturn]] ChannelDisbandIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelAdmitRequest ::= [APPLICATION 21] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       PrivateChannelId,
//        userIds         SET OF UserId
//    }

    struct ChannelAdmitRequest_Send
    {
        [[noreturn]] ChannelAdmitRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelAdmitRequest_Recv
    {
        [[noreturn]] ChannelAdmitRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };


//    ChannelAdmitIndication ::= [APPLICATION 22] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       PrivateChannelId,
//        userIds         SET OF UserId
//    }

    struct ChannelAdmitIndication_Send
    {
        [[noreturn]] ChannelAdmitIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelAdmitIndication_Recv
    {
        [[noreturn]] ChannelAdmitIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };


//    ChannelExpelRequest ::= [APPLICATION 23] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       PrivateChannelId,
//        userIds         SET OF UserId
//    }

    struct ChannelExpelRequest_Send
    {
        [[noreturn]] ChannelExpelRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelExpelRequest_Recv
    {
        [[noreturn]] ChannelExpelRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    ChannelExpelIndication ::= [APPLICATION 24] IMPLICIT SEQUENCE
//    {
//        channelId       PrivateChannelId,
//        userIds         SET OF UserId
//    }

    struct ChannelExpelIndication_Send
    {
        [[noreturn]] ChannelExpelIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct ChannelExpelIndication_Recv
    {
        [[noreturn]] ChannelExpelIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//        SendDataRequest ::= [APPLICATION 25] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority, ENUMERATED { top = 00, high = 01, medium = 10, low = 11 }
//            segmentation    Segmentation, BIT STRING { begin = 00, end = 11 }
//            userData        OCTET STRING
//        }

    struct SendDataRequest_Send
    {
        SendDataRequest_Send(OutStream & stream, uint16_t initiator, uint16_t channelId, uint8_t dataPriority, uint8_t segmentation, size_t payload_length, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "SendDataRequest PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8(MCS::MCSPDU_SendDataRequest << 2);
            stream.out_uint16_be(initiator);
            stream.out_uint16_be(channelId);
            stream.out_uint8((dataPriority << 6)|(segmentation << 4));
            out_per_length(stream, payload_length);
        }
    };

    struct SendDataRequest_Recv
    {
        uint8_t type;
        uint16_t initiator;
        uint16_t channelId;
        uint8_t magic;
        uint8_t dataPriority;
        uint8_t segmentation;

        InStream payload;

        SendDataRequest_Recv(InStream & stream, int encoding)
            : type([&stream, encoding](){
                if (encoding != PER_ENCODING){
                    LOG(LOG_ERR, "SendDataRequest PER_ENCODING mandatory");
                    throw Error(ERR_MCS);
                }

                if (!stream.in_check_rem(1)){ // tag
                    LOG(LOG_ERR, "SendDataRequest: truncated MCS PDU, expected=1 remains=%zu",
                        stream.in_remain());
                    throw Error(ERR_MCS);
                }

                uint8_t first_byte = stream.in_uint8();
                uint8_t tag = first_byte >> 2;
                if (tag != MCS::MCSPDU_SendDataRequest){
                    LOG(LOG_ERR, "SendDataRequest tag (%u) expected, got %u", MCS::MCSPDU_SendDataRequest, tag);
                    throw Error(ERR_MCS);
                }
                return MCS::MCSPDU_SendDataRequest;
            }())
            , initiator([&stream](){
                const unsigned expected =
                      6; // initiator(2) + channelId(2) + magic(1) + first byte of PER length(1)
                if (!stream.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated SendDataRequest data: expected=%u remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_MCS);
                }
                return stream.in_uint16_be();
            }())
            , channelId(stream.in_uint16_be())
            // low 4 bits of magic are padding
            , magic(stream.in_uint8())
            // dataPriority = high 2 bits,
            , dataPriority((magic >> 6) & 3)
            // segmentation = next 2 bits
            , segmentation((magic >> 4) & 3)

            , payload([&stream](){
                if (!stream.in_check_rem(2)){
                    LOG(LOG_ERR, "Truncated SendDataRequest data: payload length");
                    throw Error(ERR_MCS);
                }
                // length of payload, per_encoded
                uint16_t payload_size = stream.in_2BUE();

                if (stream.in_remain() != payload_size){
                    LOG(LOG_ERR, "Mismatching SendDataRequest data: expected=%u remains=%zu",
                        payload_size, stream.in_remain());
                    throw Error(ERR_MCS);
                }
                return InStream(stream.get_current(), stream.in_remain());
            }())
        {
//            stream.in_skip_bytes(this->payload.size());
        }
    };

//    SendDataIndication ::= [APPLICATION 26] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId,
//        dataPriority    DataPriority,
//        segmentation    Segmentation,
//        userData        OCTET STRING
//    }

    struct SendDataIndication_Send
    {
        SendDataIndication_Send(OutStream & stream, uint16_t initiator, uint16_t channelId, uint8_t dataPriority, uint8_t segmentation, size_t payload_length, int encoding)
        {
            if (encoding != PER_ENCODING){
                LOG(LOG_ERR, "SendDataIndication PER_ENCODING mandatory");
                throw Error(ERR_MCS);
            }
            stream.out_uint8(MCS::MCSPDU_SendDataIndication << 2);
            stream.out_uint16_be(initiator);
            stream.out_uint16_be(channelId);
            stream.out_uint8((dataPriority << 6)|(segmentation << 4));
            out_per_length(stream, payload_length);
        }
    };

    struct SendDataIndication_Recv
    {
        uint8_t type;
        uint16_t initiator;
        uint16_t channelId;
        uint8_t magic;
        uint8_t dataPriority;
        uint8_t segmentation;

        InStream payload;

        SendDataIndication_Recv(InStream & stream, int encoding)
            : type([&stream, encoding](){
                if (encoding != PER_ENCODING){
                    LOG(LOG_ERR, "SendDataIndication PER_ENCODING mandatory");
                    throw Error(ERR_MCS);
                }

                if (!stream.in_check_rem(1)){ // tag
                    LOG(LOG_ERR, "SendDataIndication: truncated MCS PDU, expected=1 remains=%zu",
                        stream.in_remain());
                    throw Error(ERR_MCS);
                }

                uint8_t first_byte = stream.in_uint8();
                uint8_t tag = first_byte >> 2;
                if (tag != MCS::MCSPDU_SendDataIndication){
                    LOG(LOG_ERR, "SendDataIndication tag (%u) expected, got %u", MCS::MCSPDU_SendDataIndication, tag);
                    throw Error(ERR_MCS);
                }
                return MCS::MCSPDU_SendDataIndication;
            }())
            , initiator([&stream](){
                const unsigned expected = 5; /* initiator(2) + channelId(2) + magic(1) */
                if (!stream.in_check_rem(expected)){
                    LOG(LOG_ERR, "Truncated SendDataIndication data: expected=%u, remains=%zu",
                        expected, stream.in_remain());
                    throw Error(ERR_MCS);
                }

                return stream.in_uint16_be();
            }())
            , channelId(stream.in_uint16_be())
            // low 4 bits of magic are padding
            , magic(stream.in_uint8())
            // dataPriority = high 2 bits,
            , dataPriority((magic >> 6) & 3)
            // segmentation = next 2 bits
            , segmentation((magic >> 4) & 3)
            , payload([&stream](){
                if (!stream.in_check_rem(2)){
                    LOG(LOG_ERR, "Truncated SendDataIndication data: payload length");
                    throw Error(ERR_MCS);
                }
                // length of payload, per_encoded
                size_t payload_size = stream.in_2BUE();

                if (!stream.in_check_rem(payload_size)){
                    LOG(LOG_ERR, "Truncated SendDataIndication payload data: expected=%zu remains=%zu",
                        payload_size, stream.in_remain());
                    throw Error(ERR_MCS);
                }
                return InStream(stream.get_current(), stream.in_remain());
            }())
        {
            stream.in_skip_bytes(this->payload.get_capacity());
        }
    };


//    UniformSendDataRequest ::= [APPLICATION 27] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId,
//        dataPriority    DataPriority,
//        segmentation    Segmentation,
//        userData        OCTET STRING
//    }

    struct UniformSendDataRequest_Send
    {
        [[noreturn]] UniformSendDataRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct UniformSendDataRequest_Recv
    {
        [[noreturn]] UniformSendDataRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    UniformSendDataIndication ::= [APPLICATION 28] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId,
//        dataPriority    DataPriority,
//        segmentation    Segmentation,
//        userData        OCTET STRING
//    }

    struct UniformSendDataIndication_Send
    {
        [[noreturn]] UniformSendDataIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct UniformSendDataIndication_Recv
    {
        [[noreturn]] UniformSendDataIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenGrabRequest ::= [APPLICATION 29] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenGrabRequest_Send
    {
        [[noreturn]] TokenGrabRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGrabRequest_Recv
    {
        [[noreturn]] TokenGrabRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenGrabConfirm ::= [APPLICATION 30] IMPLICIT SEQUENCE
//    {
//        result      Result,
//        initiator   UserId,
//        tokenId     TokenId,
//        tokenStatus TokenStatus
//    }

    struct TokenGrabConfirm_Send
    {
        [[noreturn]] TokenGrabConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGrabConfirm_Recv
    {
        [[noreturn]] TokenGrabConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenInhibitRequest ::= [APPLICATION 31] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenInhibitRequest_Send
    {
        [[noreturn]] TokenInhibitRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenInhibitRequest_Recv
    {
        [[noreturn]] TokenInhibitRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenInhibitConfirm ::= [APPLICATION 32] IMPLICIT SEQUENCE
//    {
//        result      Result,
//        initiator   UserId,
//        tokenId     TokenId,
//        tokenStatus TokenStatus
//    }


    struct TokenInhibitConfirm_Send
    {
        [[noreturn]] TokenInhibitConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenInhibitConfirm_Recv
    {
        [[noreturn]] TokenInhibitConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenGiveRequest ::= [APPLICATION 33] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId,
//        recipient   UserId
//    }

    struct TokenGiveRequest_Send
    {
        [[noreturn]] TokenGiveRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGiveRequest_Recv
    {
        [[noreturn]] TokenGiveRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };


//    TokenGiveIndication ::= [APPLICATION 34] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId,
//        recipient   UserId
//    }

    struct TokenGiveIndication_Send
    {
        [[noreturn]] TokenGiveIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGiveIndication_Recv
    {
        [[noreturn]] TokenGiveIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };


//    TokenGiveResponse ::= [APPLICATION 35] IMPLICIT SEQUENCE
//    {
//        result      Result,
//        recipient   UserId,
//        tokenId     TokenId
//    }

    struct TokenGiveResponse_Send
    {
        [[noreturn]] TokenGiveResponse_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGiveResponse_Recv
    {
        [[noreturn]] TokenGiveResponse_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenGiveConfirm ::= [APPLICATION 36] IMPLICIT SEQUENCE
//    {
//        result       Result,
//        initiator    UserId,
//        tokenId      TokenId,
//        tokenStatus  TokenStatus
//    }

    struct TokenGiveConfirm_Send
    {
        [[noreturn]] TokenGiveConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenGiveConfirm_Recv
    {
        [[noreturn]] TokenGiveConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenPleaseRequest ::= [APPLICATION 37] IMPLICIT SEQUENCE
//    {
//        initiator    UserId,
//        tokenId      TokenId
//    }

    struct TokenPleaseRequest_Send
    {
        [[noreturn]] TokenPleaseRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenPleaseRequest_Recv
    {
        [[noreturn]] TokenPleaseRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenPleaseIndication ::= [APPLICATION 38] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }


    struct TokenPleaseIndication_Send
    {
        [[noreturn]] TokenPleaseIndication_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenPleaseIndication_Recv
    {
        [[noreturn]] TokenPleaseIndication_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenReleaseRequest ::= [APPLICATION 39] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenReleaseRequest_Send
    {
        [[noreturn]] TokenReleaseRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenReleaseRequest_Recv
    {
        [[noreturn]] TokenReleaseRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };


//    TokenReleaseConfirm ::= [APPLICATION 40] IMPLICIT SEQUENCE
//    {
//        result      Result,
//        initiator   UserId,
//        tokenId     TokenId,
//        tokenStatus TokenStatus
//    }

    struct TokenReleaseConfirm_Send
    {
        [[noreturn]] TokenReleaseConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenReleaseConfirm_Recv
    {
        [[noreturn]] TokenReleaseConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenTestRequest ::= [APPLICATION 41] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenTestRequest_Send
    {
        [[noreturn]] TokenTestRequest_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenTestRequest_Recv
    {
        [[noreturn]] TokenTestRequest_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

//    TokenTestConfirm ::= [APPLICATION 42] IMPLICIT SEQUENCE
//    {
//        initiator    UserId,
//        tokenId      TokenId,
//        tokenStatus  TokenStatus
//    }

    struct TokenTestConfirm_Send
    {
        [[noreturn]] TokenTestConfirm_Send(OutStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

    struct TokenTestConfirm_Recv
    {
        [[noreturn]] TokenTestConfirm_Recv(InStream & stream, int encoding)
        {
            (void)stream; (void)encoding;
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
            throw Error(ERR_MCS);
        }
    };

} // namespace MCS
