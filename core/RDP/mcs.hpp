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

#if !defined(__CORE_RDP_MCS_HPP__)
#define __CORE_RDP_MCS_HPP__

#include <algorithm>
#include "client_info.hpp"
#include "RDP/x224.hpp"
#include "RDP/gcc.hpp"
#include "channel_list.hpp"
#include "genrandom.hpp"

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
    MCSPDU_TokenTestConfirm            = 0x2A,
};

enum ConnectMCSPDU {
    MCSPDU_CONNECT_INITIAL             = 101,
    MCSPDU_CONNECT_RESPONSE            = 102,
    MCSPDU_CONNECT_ADDITIONAL          = 103,
    MCSPDU_CONNECT_RESULT              = 104,
};

enum {
    PER_DomainMCSPDU_CHOICE_PlumbDomainIndication       =  0,
    PER_DomainMCSPDU_CHOICE_ErectDomainRequest          =  4,
    PER_DomainMCSPDU_CHOICE_MergeChannelsRequest        =  8,
    PER_DomainMCSPDU_CHOICE_MergeChannelsConfirm        = 12,
    PER_DomainMCSPDU_CHOICE_PurgeChannelsIndication     = 16,
    PER_DomainMCSPDU_CHOICE_MergeTokensRequest          = 20,
    PER_DomainMCSPDU_CHOICE_MergeTokensConfirm          = 24,
    PER_DomainMCSPDU_CHOICE_PurgeTokensIndication       = 28,
    PER_DomainMCSPDU_CHOICE_DisconnectProviderUltimatum = 32,
    PER_DomainMCSPDU_CHOICE_RejectMCSPDUUltimatum       = 36,
    PER_DomainMCSPDU_CHOICE_AttachUserRequest           = 40,
    PER_DomainMCSPDU_CHOICE_AttachUserConfirm           = 44,
    PER_DomainMCSPDU_CHOICE_DetachUserRequest           = 48,
    PER_DomainMCSPDU_CHOICE_DetachUserIndication        = 52,
    PER_DomainMCSPDU_CHOICE_ChannelJoinRequest          = 56,
    PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm          = 60,
    PER_DomainMCSPDU_CHOICE_ChannelLeaveRequest         = 64,
    PER_DomainMCSPDU_CHOICE_ChannelConveneRequest       = 68,
    PER_DomainMCSPDU_CHOICE_ChannelConveneConfirm       = 72,
    PER_DomainMCSPDU_CHOICE_ChannelDisbandRequest       = 76,
    PER_DomainMCSPDU_CHOICE_ChannelDisbandIndication    = 80,
    PER_DomainMCSPDU_CHOICE_ChannelAdmitRequest         = 84,
    PER_DomainMCSPDU_CHOICE_ChannelAdmitIndication      = 88,
    PER_DomainMCSPDU_CHOICE_ChannelExpelRequest         = 92,
    PER_DomainMCSPDU_CHOICE_ChannelExpelIndication      = 96,
    PER_DomainMCSPDU_CHOICE_SendDataRequest             = 100,
    PER_DomainMCSPDU_CHOICE_SendDataIndication          = 104,
    PER_DomainMCSPDU_CHOICE_UniformSendDataRequest      = 108,
    PER_DomainMCSPDU_CHOICE_UniformSendDataIndication   = 112,
    PER_DomainMCSPDU_CHOICE_TokenGrabRequest            = 116,
    PER_DomainMCSPDU_CHOICE_TokenGrabConfirm            = 120,
    PER_DomainMCSPDU_CHOICE_TokenInhibitRequest         = 124,
    PER_DomainMCSPDU_CHOICE_TokenInhibitConfirm         = 128,
    PER_DomainMCSPDU_CHOICE_TokenGiveRequest            = 132,
    PER_DomainMCSPDU_CHOICE_TokenGiveIndication         = 136,
    PER_DomainMCSPDU_CHOICE_TokenGiveResponse           = 140,
    PER_DomainMCSPDU_CHOICE_TokenGiveConfirm            = 144,
    PER_DomainMCSPDU_CHOICE_TokenPleaseRequest          = 148,
    PER_DomainMCSPDU_CHOICE_TokenPleaseIndication       = 152,
    PER_DomainMCSPDU_CHOICE_TokenReleaseRequest         = 156,
    PER_DomainMCSPDU_CHOICE_TokenReleaseConfirm         = 160,
    PER_DomainMCSPDU_CHOICE_TokenTestRequest            = 164,
    PER_DomainMCSPDU_CHOICE_TokenTestConfirm            = 168,
};

enum {
    BER_TAG_ConnectMCSPDU_CONNECT_INITIAL    = 0x7f65,
    BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE   = 0x7f66,
    BER_TAG_ConnectMCSPDU_CONNECT_ADDITIONAL = 0x7f67,
    BER_TAG_ConnectMCSPDU_CONNECT_RESULT     = 0x7f68,
};

enum {
    BER_TAG_MCS_DOMAIN_PARAMS = 0x30
};

// Reason ::= ENUMERATED   -- in DisconnectProviderUltimatum, DetachUserRequest, DetachUserIndication
enum {
    RN_DOMAIN_DISCONNECTED = 0,
    RN_PROVIDER_INITIATED  = 1,
    RN_TOKEN_PURGED        = 2,
    RN_USER_REQUESTED      = 3,
    RN_CHANNEL_PURGED      = 4
};

//##############################################################################
struct Mcs
//##############################################################################
{
    Stream & stream;
    SubStream payload;
    uint8_t offlen;
    public:
    uint8_t opcode;
    uint8_t result;
    uint16_t user_id;
    uint16_t chan_id;
    uint8_t magic_0x70; // some ber header ?
    uint16_t len;

    // CONSTRUCTOR
    //==============================================================================
    Mcs ( Stream & stream )
    //==============================================================================
    : stream(stream)
    , payload(this->stream, 0)
    , offlen(0)
    , opcode(0)
    , result(0)
    , user_id(0)
    , chan_id(0)
    , len(0)
    {
    } // END CONSTRUCTOR

    //==============================================================================
    void emit_begin( uint8_t command
                   , uint8_t user_id
                   , uint16_t chan_id
                   )
    //==============================================================================
    {
        this->opcode = command << 2;
        switch (this->opcode){
        case PER_DomainMCSPDU_CHOICE_PlumbDomainIndication:
        {
//        PlumbDomainIndication ::= [APPLICATION 0] IMPLICIT SEQUENCE
//        {
//            heightLimit     INTEGER (0..MAX)
//                            -- a restriction on the MCSPDU receiver
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PlumbDomainIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ErectDomainRequest:
        {
//        ErectDomainRequest ::= [APPLICATION 1] IMPLICIT SEQUENCE
//        {
//            subHeight   INTEGER (0..MAX),
//                        -- height in domain of the MCSPDU transmitter
//            subInterval INTEGER (0..MAX)
//                        -- its throughput enforcement interval in milliseconds
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ErectDomainRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsRequest:
        {
//        ChannelAttributes ::= CHOICE
//        {
//            static [0] IMPLICIT SEQUENCE
//            {
//                channelId   StaticChannelId
//                            -- joined is implicitly TRUE
//            },

//            userId  [1] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if user is joined to its user id
//                userId      UserId
//            },

//            private [2] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if channel id is joined below
//                channelId   PrivateChannelId,
//                manager     UserId,
//                admitted    SET OF UserId
//                            -- may span multiple MergeChannelsRequest
//            },

//            assigned [3] IMPLICIT SEQUENCE
//            {
//                channelId   AssignedChannelId
//                            -- joined is implicitly TRUE
//            }
//        }

//        MergeChannelsRequest ::= [APPLICATION 2] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsConfirm:
        {
//        MergeChannelsConfirm ::= [APPLICATION 3] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeChannelsIndication:
        {
//        PurgeChannelsIndication ::= [APPLICATION 4] IMPLICIT SEQUENCE
//        {
//            detachUserIds       SET OF UserId,
//                                -- purge user id channels
//            purgeChannelIds     SET OF ChannelId
//                                -- purge other channels
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeChannelsIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensRequest:
        {
//        TokenAttributes ::= CHOICE
//        {
//            grabbed [0] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//            },

//            inhibited [1] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                inhibitors  SET OF UserId
//                            -- may span multiple MergeTokensRequest
//            },

//            giving [2] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId,
//                recipient   UserId
//            },

//            ungivable [3] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//                            -- recipient has since detached
//            },

//            given [4] IMPLICIT SEQUENCE
//            {
//                tokenId         TokenId,
//                recipient       UserId
//                                -- grabber released or detached
//            }
//        }

//        MergeTokensRequest ::= [APPLICATION 5] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensConfirm:
        {
//        MergeTokensConfirm ::= [APPLICATION 6] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }

            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeTokensIndication:
        {
//        PurgeTokensIndication ::= [APPLICATION 7] IMPLICIT SEQUENCE
//        {
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeTokensIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DisconnectProviderUltimatum:
        {
//        -- Part 4: Disconnect provider

//        DisconnectProviderUltimatum ::= [APPLICATION 8] IMPLICIT SEQUENCE
//        {
//            reason          Reason
//        }
          stream.out_uint8(PER_DomainMCSPDU_CHOICE_DisconnectProviderUltimatum);
          stream.out_uint8(RN_DOMAIN_DISCONNECTED);
        }
        break;
        case PER_DomainMCSPDU_CHOICE_RejectMCSPDUUltimatum:
        {
//        RejectMCSPDUUltimatum ::= [APPLICATION 9] IMPLICIT SEQUENCE
//        {
//            diagnostic      Diagnostic,
//            initialOctets   OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU RejectMCSPDUUltimatum");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserRequest:
        {
//        -- Part 5: Attach/Detach user

//        AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
//        {
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU AttachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserConfirm:
        {
//        AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId OPTIONAL
//        }
            stream.out_uint8(PER_DomainMCSPDU_CHOICE_AttachUserConfirm | 2);
            stream.out_uint8(0); // result OK
            stream.out_uint16_be(user_id);
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserRequest:
        {
//        DetachUserRequest ::= [APPLICATION 12] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserIndication:
        {
//        DetachUserIndication ::= [APPLICATION 13] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinRequest:
        {
//        ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId
//                            -- may be zero
//        }
            stream.out_uint8(PER_DomainMCSPDU_CHOICE_ChannelJoinRequest);
            stream.out_uint16_be(user_id);
            stream.out_uint16_be(chan_id);
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm:
        {
//        ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            requested       ChannelId,
//                            -- may be zero
//            channelId       ChannelId OPTIONAL
//        }

            stream.out_uint8(PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm | 2);
            stream.out_uint8(0); // Result = rt_successfull
            stream.out_uint16_be(user_id);
            stream.out_uint16_be(chan_id);
            stream.out_uint16_be(chan_id);
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelLeaveRequest:
        {
//        ChannelLeaveRequest ::= [APPLICATION 16] IMPLICIT SEQUENCE
//        {
//            channelIds      SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelLeaveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneRequest:
        {
//        ChannelConveneRequest ::= [APPLICATION 17] IMPLICIT SEQUENCE
//        {
//            initiator       UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneConfirm:
        {
//        ChannelConveneConfirm ::= [APPLICATION 18] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            channelId       PrivateChannelId OPTIONAL
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandRequest:
        {
//        ChannelDisbandRequest ::= [APPLICATION 19] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandIndication:
        {
//        ChannelDisbandIndication ::= [APPLICATION 20] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitRequest:
        {
//        ChannelAdmitRequest ::= [APPLICATION 21] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitIndication:
        {
//        ChannelAdmitIndication ::= [APPLICATION 22] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelRequest:
        {
//        ChannelExpelRequest ::= [APPLICATION 23] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelIndication:
        {
//        ChannelExpelIndication ::= [APPLICATION 24] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataRequest:
        {

//        SendDataRequest ::= [APPLICATION 25] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority, ENUMERATED { top = 00, high = 01, medium = 10, low = 11 }
//            segmentation    Segmentation, BIT STRING { begin = 00, end = 11 }
//            userData        OCTET STRING
//        }

            stream.out_uint8(PER_DomainMCSPDU_CHOICE_SendDataRequest);
            stream.out_uint16_be(user_id);
            stream.out_uint16_be(chan_id);
            stream.out_uint8(0x70);  // dataPriority = high, segmentation = end
            this->offlen = stream.get_offset(0);
            stream.out_uint16_be(0); // skip len
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataIndication:
        {

//        SendDataIndication ::= [APPLICATION 26] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }

            stream.out_uint8(PER_DomainMCSPDU_CHOICE_SendDataIndication);
            stream.out_uint16_be(user_id);
            stream.out_uint16_be(chan_id);
            stream.out_uint8(0x70);  // dataPriority = high, segmentation = end
            this->offlen = stream.get_offset(0);
            stream.out_uint16_be(0); // skip len
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataRequest:
        {
//        UniformSendDataRequest ::= [APPLICATION 27] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataIndication:
        {
//        UniformSendDataIndication ::= [APPLICATION 28] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabRequest:
        {
//        -- Part 8: Token management

//        TokenGrabRequest ::= [APPLICATION 29] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabConfirm:
        {
//        TokenGrabConfirm ::= [APPLICATION 30] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitRequest:
        {
//        TokenInhibitRequest ::= [APPLICATION 31] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitConfirm:
        {
//        TokenInhibitConfirm ::= [APPLICATION 32] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveRequest:
        {
//        TokenGiveRequest ::= [APPLICATION 33] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveIndication:
        {
//        TokenGiveIndication ::= [APPLICATION 34] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveResponse:
        {
//        TokenGiveResponse ::= [APPLICATION 35] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            recipient   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveResponse");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveConfirm:
        {
//        TokenGiveConfirm ::= [APPLICATION 36] IMPLICIT SEQUENCE
//        {
//            result       Result,
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseRequest:
        {
//        TokenPleaseRequest ::= [APPLICATION 37] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseIndication:
        {
//        TokenPleaseIndication ::= [APPLICATION 38] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseRequest:
        {
//        TokenReleaseRequest ::= [APPLICATION 39] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseConfirm:
        {
//        TokenReleaseConfirm ::= [APPLICATION 40] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestRequest:
        {
//        TokenTestRequest ::= [APPLICATION 41] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestConfirm:
        {
//        TokenTestConfirm ::= [APPLICATION 42] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestConfirm");
        }
        break;
        default:
        {
            LOG(LOG_WARNING, "Unsupported DomainPDU %u", this->opcode);
        }
        break;
        }

    } // END METHOD emit_begin

    //==============================================================================
    void emit_end(){
    //==============================================================================
        switch (this->opcode){
        case PER_DomainMCSPDU_CHOICE_PlumbDomainIndication:
        {
//        PlumbDomainIndication ::= [APPLICATION 0] IMPLICIT SEQUENCE
//        {
//            heightLimit     INTEGER (0..MAX)
//                            -- a restriction on the MCSPDU receiver
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PlumbDomainIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ErectDomainRequest:
        {
//        ErectDomainRequest ::= [APPLICATION 1] IMPLICIT SEQUENCE
//        {
//            subHeight   INTEGER (0..MAX),
//                        -- height in domain of the MCSPDU transmitter
//            subInterval INTEGER (0..MAX)
//                        -- its throughput enforcement interval in milliseconds
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ErectDomainRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsRequest:
        {
//        ChannelAttributes ::= CHOICE
//        {
//            static [0] IMPLICIT SEQUENCE
//            {
//                channelId   StaticChannelId
//                            -- joined is implicitly TRUE
//            },

//            userId  [1] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if user is joined to its user id
//                userId      UserId
//            },

//            private [2] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if channel id is joined below
//                channelId   PrivateChannelId,
//                manager     UserId,
//                admitted    SET OF UserId
//                            -- may span multiple MergeChannelsRequest
//            },

//            assigned [3] IMPLICIT SEQUENCE
//            {
//                channelId   AssignedChannelId
//                            -- joined is implicitly TRUE
//            }
//        }

//        MergeChannelsRequest ::= [APPLICATION 2] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsConfirm:
        {
//        MergeChannelsConfirm ::= [APPLICATION 3] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeChannelsIndication:
        {
//        PurgeChannelsIndication ::= [APPLICATION 4] IMPLICIT SEQUENCE
//        {
//            detachUserIds       SET OF UserId,
//                                -- purge user id channels
//            purgeChannelIds     SET OF ChannelId
//                                -- purge other channels
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeChannelsIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensRequest:
        {
//        TokenAttributes ::= CHOICE
//        {
//            grabbed [0] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//            },

//            inhibited [1] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                inhibitors  SET OF UserId
//                            -- may span multiple MergeTokensRequest
//            },

//            giving [2] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId,
//                recipient   UserId
//            },

//            ungivable [3] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//                            -- recipient has since detached
//            },

//            given [4] IMPLICIT SEQUENCE
//            {
//                tokenId         TokenId,
//                recipient       UserId
//                                -- grabber released or detached
//            }
//        }

//        MergeTokensRequest ::= [APPLICATION 5] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensConfirm:
        {
//        MergeTokensConfirm ::= [APPLICATION 6] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }

            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeTokensIndication:
        {
//        PurgeTokensIndication ::= [APPLICATION 7] IMPLICIT SEQUENCE
//        {
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeTokensIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DisconnectProviderUltimatum:
        {
//        -- Part 4: Disconnect provider

//        DisconnectProviderUltimatum ::= [APPLICATION 8] IMPLICIT SEQUENCE
//        {
//            reason          Reason
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DisconnectProviderUltimatum");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_RejectMCSPDUUltimatum:
        {
//        RejectMCSPDUUltimatum ::= [APPLICATION 9] IMPLICIT SEQUENCE
//        {
//            diagnostic      Diagnostic,
//            initialOctets   OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU RejectMCSPDUUltimatum");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserRequest:
        {
//        -- Part 5: Attach/Detach user

//        AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
//        {
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU AttachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserConfirm:
        {
//        AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId OPTIONAL
//        }
            // Nothing to do
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserRequest:
        {
//        DetachUserRequest ::= [APPLICATION 12] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserIndication:
        {
//        DetachUserIndication ::= [APPLICATION 13] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinRequest:
        {
//        ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId
//                            -- may be zero
//        }
        }
        // Nothing to do
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm:
        {
//        ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            requested       ChannelId,
//                            -- may be zero
//            channelId       ChannelId OPTIONAL
//        }
        // Nothing to do
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelLeaveRequest:
        {
//        ChannelLeaveRequest ::= [APPLICATION 16] IMPLICIT SEQUENCE
//        {
//            channelIds      SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelLeaveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneRequest:
        {
//        ChannelConveneRequest ::= [APPLICATION 17] IMPLICIT SEQUENCE
//        {
//            initiator       UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneConfirm:
        {
//        ChannelConveneConfirm ::= [APPLICATION 18] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            channelId       PrivateChannelId OPTIONAL
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandRequest:
        {
//        ChannelDisbandRequest ::= [APPLICATION 19] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandIndication:
        {
//        ChannelDisbandIndication ::= [APPLICATION 20] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitRequest:
        {
//        ChannelAdmitRequest ::= [APPLICATION 21] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitIndication:
        {
//        ChannelAdmitIndication ::= [APPLICATION 22] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelRequest:
        {
//        ChannelExpelRequest ::= [APPLICATION 23] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelIndication:
        {
//        ChannelExpelIndication ::= [APPLICATION 24] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataRequest:
        {
//        SendDataRequest ::= [APPLICATION 25] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority, ENUMERATED { top = 00, high = 01, medium = 10, low = 11 }
//            segmentation    Segmentation, BIT STRING { begin = 00, end = 11 }
//            userData        OCTET STRING
//        }
            int len = stream.get_offset(this->offlen + 2);
            stream.set_out_uint16_be(0x8000|len, this->offlen); // userData header
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataIndication:
        {

//        SendDataIndication ::= [APPLICATION 26] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }

            int len = stream.get_offset(this->offlen + 2);
            stream.set_out_uint16_be(0x8000|len, this->offlen); // userData header
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataRequest:
        {
//        UniformSendDataRequest ::= [APPLICATION 27] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataIndication:
        {
//        UniformSendDataIndication ::= [APPLICATION 28] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabRequest:
        {
//        -- Part 8: Token management

//        TokenGrabRequest ::= [APPLICATION 29] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabConfirm:
        {
//        TokenGrabConfirm ::= [APPLICATION 30] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitRequest:
        {
//        TokenInhibitRequest ::= [APPLICATION 31] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitConfirm:
        {
//        TokenInhibitConfirm ::= [APPLICATION 32] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveRequest:
        {
//        TokenGiveRequest ::= [APPLICATION 33] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveIndication:
        {
//        TokenGiveIndication ::= [APPLICATION 34] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveResponse:
        {
//        TokenGiveResponse ::= [APPLICATION 35] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            recipient   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveResponse");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveConfirm:
        {
//        TokenGiveConfirm ::= [APPLICATION 36] IMPLICIT SEQUENCE
//        {
//            result       Result,
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseRequest:
        {
//        TokenPleaseRequest ::= [APPLICATION 37] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseIndication:
        {
//        TokenPleaseIndication ::= [APPLICATION 38] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseRequest:
        {
//        TokenReleaseRequest ::= [APPLICATION 39] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseConfirm:
        {
//        TokenReleaseConfirm ::= [APPLICATION 40] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestRequest:
        {
//        TokenTestRequest ::= [APPLICATION 41] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestConfirm:
        {
//        TokenTestConfirm ::= [APPLICATION 42] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestConfirm");
        }
        break;
        default:
        {
            LOG(LOG_WARNING, "Unsupported DomainPDU %u", this->opcode);
        }
        break;
        }
    } // END METHOD emit_end

    //==============================================================================
    void recv_begin() {
    //==============================================================================
        this->opcode = stream.in_uint8();
        switch (this->opcode & 0xFC){
        case PER_DomainMCSPDU_CHOICE_PlumbDomainIndication:
        {
//        PlumbDomainIndication ::= [APPLICATION 0] IMPLICIT SEQUENCE
//        {
//            heightLimit     INTEGER (0..MAX)
//                            -- a restriction on the MCSPDU receiver
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PlumbDomainIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ErectDomainRequest:
        {
//        ErectDomainRequest ::= [APPLICATION 1] IMPLICIT SEQUENCE
//        {
//            subHeight   INTEGER (0..MAX),
//                        -- height in domain of the MCSPDU transmitter
//            subInterval INTEGER (0..MAX)
//                        -- its throughput enforcement interval in milliseconds
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ErectDomainRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsRequest:
        {
//        ChannelAttributes ::= CHOICE
//        {
//            static [0] IMPLICIT SEQUENCE
//            {
//                channelId   StaticChannelId
//                            -- joined is implicitly TRUE
//            },

//            userId  [1] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if user is joined to its user id
//                userId      UserId
//            },

//            private [2] IMPLICIT SEQUENCE
//            {
//                joined      BOOLEAN,
//                            -- TRUE if channel id is joined below
//                channelId   PrivateChannelId,
//                manager     UserId,
//                admitted    SET OF UserId
//                            -- may span multiple MergeChannelsRequest
//            },

//            assigned [3] IMPLICIT SEQUENCE
//            {
//                channelId   AssignedChannelId
//                            -- joined is implicitly TRUE
//            }
//        }

//        MergeChannelsRequest ::= [APPLICATION 2] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeChannelsConfirm:
        {
//        MergeChannelsConfirm ::= [APPLICATION 3] IMPLICIT SEQUENCE
//        {
//            mergeChannels   SET OF ChannelAttributes,
//            purgeChannelIds SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeChannelsConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeChannelsIndication:
        {
//        PurgeChannelsIndication ::= [APPLICATION 4] IMPLICIT SEQUENCE
//        {
//            detachUserIds       SET OF UserId,
//                                -- purge user id channels
//            purgeChannelIds     SET OF ChannelId
//                                -- purge other channels
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeChannelsIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensRequest:
        {
//        TokenAttributes ::= CHOICE
//        {
//            grabbed [0] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//            },

//            inhibited [1] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                inhibitors  SET OF UserId
//                            -- may span multiple MergeTokensRequest
//            },

//            giving [2] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId,
//                recipient   UserId
//            },

//            ungivable [3] IMPLICIT SEQUENCE
//            {
//                tokenId     TokenId,
//                grabber     UserId
//                            -- recipient has since detached
//            },

//            given [4] IMPLICIT SEQUENCE
//            {
//                tokenId         TokenId,
//                recipient       UserId
//                                -- grabber released or detached
//            }
//        }

//        MergeTokensRequest ::= [APPLICATION 5] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_MergeTokensConfirm:
        {
//        MergeTokensConfirm ::= [APPLICATION 6] IMPLICIT SEQUENCE
//        {
//            mergeTokens     SET OF TokenAttributes,
//            purgeTokenIds   SET OF TokenId
//        }

            LOG(LOG_WARNING, "Unsupported DomainPDU MergeTokensConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_PurgeTokensIndication:
        {
//        PurgeTokensIndication ::= [APPLICATION 7] IMPLICIT SEQUENCE
//        {
//            purgeTokenIds   SET OF TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU PurgeTokensIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DisconnectProviderUltimatum:
        {
//        -- Part 4: Disconnect provider

//        DisconnectProviderUltimatum ::= [APPLICATION 8] IMPLICIT SEQUENCE
//        {
//            reason          Reason
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DisconnectProviderUltimatum");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_RejectMCSPDUUltimatum:
        {
//        RejectMCSPDUUltimatum ::= [APPLICATION 9] IMPLICIT SEQUENCE
//        {
//            diagnostic      Diagnostic,
//            initialOctets   OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU RejectMCSPDUUltimatum");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserRequest:
        {
//        -- Part 5: Attach/Detach user

//        AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
//        {
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU AttachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_AttachUserConfirm:
        {
//        AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId OPTIONAL
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU AttachUserConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserRequest:
        {
//        DetachUserRequest ::= [APPLICATION 12] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_DetachUserIndication:
        {
//        DetachUserIndication ::= [APPLICATION 13] IMPLICIT SEQUENCE
//        {
//            reason          Reason,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU DetachUserIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinRequest:
        {
//        ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId
//                            -- may be zero
//        }
            this->user_id = stream.in_uint16_be();
            this->chan_id = stream.in_uint16_be();
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm:
        {
//        ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            requested       ChannelId,
//                            -- may be zero
//            channelId       ChannelId OPTIONAL
//        }
            // TODO: the actual result should be checked
            this->result  = stream.in_uint8();
            this->user_id = stream.in_uint16_be();
            this->chan_id = stream.in_uint16_be();
            if (this->opcode & 2){
                this->chan_id = stream.in_uint16_be();
            }
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelLeaveRequest:
        {
//        ChannelLeaveRequest ::= [APPLICATION 16] IMPLICIT SEQUENCE
//        {
//            channelIds      SET OF ChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelLeaveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneRequest:
        {
//        ChannelConveneRequest ::= [APPLICATION 17] IMPLICIT SEQUENCE
//        {
//            initiator       UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelConveneConfirm:
        {
//        ChannelConveneConfirm ::= [APPLICATION 18] IMPLICIT SEQUENCE
//        {
//            result          Result,
//            initiator       UserId,
//            channelId       PrivateChannelId OPTIONAL
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelConveneConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandRequest:
        {
//        ChannelDisbandRequest ::= [APPLICATION 19] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelDisbandIndication:
        {
//        ChannelDisbandIndication ::= [APPLICATION 20] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelDisbandIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitRequest:
        {
//        ChannelAdmitRequest ::= [APPLICATION 21] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelAdmitIndication:
        {
//        ChannelAdmitIndication ::= [APPLICATION 22] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelAdmitIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelRequest:
        {
//        ChannelExpelRequest ::= [APPLICATION 23] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_ChannelExpelIndication:
        {
//        ChannelExpelIndication ::= [APPLICATION 24] IMPLICIT SEQUENCE
//        {
//            channelId       PrivateChannelId,
//            userIds         SET OF UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU ChannelExpelIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataRequest:
        {
//        SendDataRequest ::= [APPLICATION 25] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority, ENUMERATED { top = 00, high = 01, medium = 10, low = 11 }
//            segmentation    Segmentation, BIT STRING { begin = 00, end = 11 }
//            userData        OCTET STRING
//        }

            this->user_id = stream.in_uint16_be();
            this->chan_id = stream.in_uint16_be();
            this->magic_0x70 = stream.in_uint8(); // dataPriority = high 2 bits, segmentation = end 2 bits, padding 4 bits
            this->len = stream.in_uint8();
            if (this->len & 0x80){
                this->len = ((this->len & 0x7F) << 8) + stream.in_uint8();
            }
        }
        break;
        case PER_DomainMCSPDU_CHOICE_SendDataIndication:
        {

//        SendDataIndication ::= [APPLICATION 26] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }

            this->user_id = stream.in_uint16_be();
            this->chan_id = stream.in_uint16_be();
            this->magic_0x70 = stream.in_uint8(); // dataPriority = high 2 bits, segmentation = end 2 bits, padding 4 bits
            this->len = stream.in_uint8();
            if (this->len & 0x80){
                this->len = ((this->len & 0x7F) << 8) + stream.in_uint8();
            }
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataRequest:
        {
//        UniformSendDataRequest ::= [APPLICATION 27] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_UniformSendDataIndication:
        {
//        UniformSendDataIndication ::= [APPLICATION 28] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId,
//            dataPriority    DataPriority,
//            segmentation    Segmentation,
//            userData        OCTET STRING
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU UniformSendDataIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabRequest:
        {
//        -- Part 8: Token management

//        TokenGrabRequest ::= [APPLICATION 29] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGrabConfirm:
        {
//        TokenGrabConfirm ::= [APPLICATION 30] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGrabConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitRequest:
        {
//        TokenInhibitRequest ::= [APPLICATION 31] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenInhibitConfirm:
        {
//        TokenInhibitConfirm ::= [APPLICATION 32] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenInhibitConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveRequest:
        {
//        TokenGiveRequest ::= [APPLICATION 33] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveIndication:
        {
//        TokenGiveIndication ::= [APPLICATION 34] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId,
//            recipient   UserId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveResponse:
        {
//        TokenGiveResponse ::= [APPLICATION 35] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            recipient   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveResponse");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenGiveConfirm:
        {
//        TokenGiveConfirm ::= [APPLICATION 36] IMPLICIT SEQUENCE
//        {
//            result       Result,
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenGiveConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseRequest:
        {
//        TokenPleaseRequest ::= [APPLICATION 37] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenPleaseIndication:
        {
//        TokenPleaseIndication ::= [APPLICATION 38] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenPleaseIndication");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseRequest:
        {
//        TokenReleaseRequest ::= [APPLICATION 39] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenReleaseConfirm:
        {
//        TokenReleaseConfirm ::= [APPLICATION 40] IMPLICIT SEQUENCE
//        {
//            result      Result,
//            initiator   UserId,
//            tokenId     TokenId,
//            tokenStatus TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenReleaseConfirm");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestRequest:
        {
//        TokenTestRequest ::= [APPLICATION 41] IMPLICIT SEQUENCE
//        {
//            initiator   UserId,
//            tokenId     TokenId
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestRequest");
        }
        break;
        case PER_DomainMCSPDU_CHOICE_TokenTestConfirm:
        {
//        TokenTestConfirm ::= [APPLICATION 42] IMPLICIT SEQUENCE
//        {
//            initiator    UserId,
//            tokenId      TokenId,
//            tokenStatus  TokenStatus
//        }
            LOG(LOG_WARNING, "Unsupported DomainPDU TokenTestConfirm");
        }
        break;
        default:
        {
            LOG(LOG_WARNING, "Unsupported DomainPDU %u ", this->opcode);
        }
        break;
        }
        this->payload.reset(this->stream, this->stream.get_offset(0));
    } // END METHOD recv_begin

    //==============================================================================
    void recv_end(){
    //==============================================================================
        if (this->stream.p != this->stream.end
        && this->payload.p != this->payload.end){
            LOG(LOG_ERR, "MCS: all data should have been consumed : remains %d, opcode=%u", stream.end - stream.p, this->opcode);
            exit(0);
        }
    } // END METHOD recv_end


}; // END CLASS Mcs



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
//   2.2.1.3.6). This field MUST NOT be included if the server does not
//   advertise support for extended client data blocks by using the
//   EXTENDED_CLIENT_DATA_SUPPORTED flag (0x00000001) as described in section
//   2.2.1.2.1.




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

// AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
// {
// }

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



static inline void mcs_send_erect_domain_and_attach_user_request_pdu(Transport * trans)
{
    LOG(LOG_INFO, "mcs_send_erect_domain_and_attach_user_request_pdu");
    {
        BStream stream(65536);
        stream.out_uint8((MCSPDU_ErectDomainRequest << 2));
        stream.out_per_integer(0); /* subHeight (INTEGER) */
        stream.out_per_integer(0); /* subInterval (INTEGER) */
        stream.end = stream.p;

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }
    {
        BStream stream(65536);
        stream.out_uint8((MCSPDU_AttachUserRequest << 2));
        stream.end = stream.p;

        BStream x224_header(256);
        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

        trans->send(x224_header.data, x224_header.end - x224_header.data);
        trans->send(stream.data, stream.end - stream.data);
    }
}


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

// ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
// {
//     initiator UserId
//     channelId ChannelId
//               -- may be zero
// }


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

static inline void mcs_send_channel_join_request_pdu(Transport * trans, int userid, int chanid)
{
    BStream stream(65536);
    stream.out_uint8((MCSPDU_ChannelJoinRequest << 2));
    stream.out_uint16_be(userid);
    stream.out_uint16_be(chanid);
    stream.end = stream.p;

    BStream x224_header(256);
    X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

    trans->send(x224_header.data, x224_header.end - x224_header.data);
    trans->send(stream.data, stream.end - stream.data);
}


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

// ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
// {
//   result Result,
//   initiator UserId,
//   requested ChannelId, -- may be zero
//   channelId ChannelId OPTIONAL
// }

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

static inline void mcs_recv_channel_join_confirm_pdu(Transport * trans, uint16_t & mcs_userid, uint16_t & req_chanid, uint16_t & join_chanid)
{
    BStream stream(65536);
    X224::RecvFactory f(*trans, stream);
    X224::DT_TPDU_Recv x224(*trans, stream, f.length);
    SubStream payload(stream, x224.header_size);

    Mcs mcs(payload);
    mcs.recv_begin();

    if ((mcs.opcode >> 2) != MCSPDU_ChannelJoinConfirm) {
        LOG(LOG_ERR, "Recv channel join confirm pdu expected");
        throw Error(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF);
    }
    if (0 != mcs.result) {
        LOG(LOG_INFO, "mcs_recv_channel_join_confirm_pdu exception, expected 0, got %u", mcs.result);
        throw Error(ERR_MCS_RECV_CJCF_EMPTY);
    }
    mcs_userid = mcs.user_id;
    join_chanid = mcs.chan_id;
    mcs.recv_end();
}

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

static inline void mcs_recv_attach_user_confirm_pdu(Transport * trans, uint16_t & userid)
{
    BStream stream(65536);
    X224::RecvFactory f(*trans, stream);
    X224::DT_TPDU_Recv x224(*trans, stream, f.length);
    SubStream payload(stream, x224.header_size);

    int opcode = payload.in_uint8();
    if ((opcode >> 2) != MCSPDU_AttachUserConfirm) {
        LOG(LOG_ERR, "Attach user confirm pdu expected");
        throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
    }
    int res = payload.in_uint8();
    if (res != 0) {
        LOG(LOG_ERR, "Attach user confirm pdu OK expected");
        throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
    }
    if (opcode & 2) {
        userid = payload.in_uint16_be();
    }
}

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

// AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
// {
// }

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

static inline void mcs_recv_erect_domain_and_attach_user_request_pdu(Transport * trans, uint16_t & userid)
{
    TODO(" this code could lead to some problem if both MCS are combined in the same TPDU  we should manage this case")
    {
        BStream stream(65536);
        X224::RecvFactory f(*trans, stream);
        X224::DT_TPDU_Recv x224(*trans, stream, f.length);
        SubStream payload(stream, x224.header_size);

        uint8_t opcode = payload.in_uint8();
        if ((opcode >> 2) != MCSPDU_ErectDomainRequest) {
            LOG(LOG_ERR, "Erect Domain Request expected");
            throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
        }
        payload.in_skip_bytes(2);
        payload.in_skip_bytes(2);
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }
    }

    {
        BStream stream(65536);
        X224::RecvFactory f(*trans, stream);
        X224::DT_TPDU_Recv x224(*trans, stream, f.length);
        SubStream payload(stream, x224.header_size);

        uint8_t opcode = payload.in_uint8();
        if ((opcode >> 2) != MCSPDU_AttachUserRequest) {
            LOG(LOG_ERR, "Attach User Request expected");
            throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
        }
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }

    }
}


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

static inline void mcs_send_attach_user_confirm_pdu(Transport * trans, uint16_t userid)
{
    BStream stream(65536);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_AttachUserConfirm, userid, 0);
    mcs.emit_end();
    stream.end = stream.p;

    BStream x224_header(256);
    X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

    trans->send(x224_header.data, x224_header.end - x224_header.data);
    trans->send(stream.data, stream.end - stream.data);
}


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

// ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
// {
//   result Result,
//   initiator UserId,
//   requested ChannelId, -- may be zero
//   channelId ChannelId OPTIONAL
// }

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

static inline void mcs_send_channel_join_confirm_pdu(Transport * trans, uint16_t userid, uint16_t chanid)
{
    BStream stream(65536);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_ChannelJoinConfirm, userid, chanid);
    mcs.emit_end();
    stream.end = stream.p;

    BStream x224_header(256);
    X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

    trans->send(x224_header.data, x224_header.end - x224_header.data);
    trans->send(stream.data, stream.end - stream.data);
}


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

// ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
// {
//     initiator UserId
//     channelId ChannelId
//               -- may be zero
// }


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

static inline void mcs_recv_channel_join_request_pdu(Transport * trans, uint16_t & userid, uint16_t & chanid){
    // read tpktHeader (4 bytes = 3 0 len)
    // TPDU class 0    (3 bytes = LI F0 PDU_DT)
    BStream stream(65536);
    X224::RecvFactory f(*trans, stream);
    X224::DT_TPDU_Recv x224(*trans, stream, f.length);
    SubStream payload(stream, x224.header_size);

    Mcs mcs(payload);
    mcs.recv_begin();
    if ((mcs.opcode >> 2) != MCSPDU_ChannelJoinRequest) {
        LOG(LOG_INFO, "unexpected opcode = %u", mcs.opcode);
        throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
    }
    userid = mcs.user_id;
    chanid = mcs.chan_id;
    mcs.recv_end();
}

namespace MCS
{
    enum {
        BER_ENCODING,
        PER_ENCODING
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
        MCSPDU_TokenTestConfirm            = 0x2A,
    };

    enum ConnectMCSPDU {
        MCSPDU_CONNECT_INITIAL             = 101,
        MCSPDU_CONNECT_RESPONSE            = 102,
        MCSPDU_CONNECT_ADDITIONAL          = 103,
        MCSPDU_CONNECT_RESULT              = 104,
    };

    struct RecvFactory
    {
        int type;
        RecvFactory(Stream & stream, int encoding)
        {
            switch (encoding){
            case PER_ENCODING:
                TODO("Check we have at least 1 byte available")
                this->type = (stream.data[0] >> 2);
            break;
            default:
            case BER_ENCODING:
                TODO("Check we have at least 2 bytes available")
                TODO("getting to the type this way should works in our restricted use case,"
                     " but it would be nicer to perform actual BER TAG value decoding")
                this->type = stream.data[1];
            break;
            }
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
//   2.2.1.3.6). This field MUST NOT be included if the server does not
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

        int in_ber_int(Stream & stream, int & v){
            uint8_t tag = stream.in_uint8();
            if (Stream::BER_TAG_INTEGER != tag){
                LOG(LOG_ERR, "Connect Initial BER_TAG_INTEGER (%u) expected, got %u", Stream::BER_TAG_INTEGER, tag);
                return -1;
            }
            uint8_t len = stream.in_uint8();
            v = stream.in_bytes_be(len);
            return 0;
        }

        int recv(Stream & stream){
            if (BER_TAG_MCS_DOMAIN_PARAMS != stream.in_uint8()){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS expected");
                return -1;
            }
            size_t len = stream.in_ber_len();
            size_t start_offset = stream.get_offset(0);
            if ( -1 == this->in_ber_int(stream, this->maxChannelIds)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxChannelIds tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->maxUserIds)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxUserIds tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->maxTokenIds)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxTokenIds tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->numPriorities)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::numPriorities tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->minThroughput)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::minThroughput tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->maxHeight)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxHeight tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->maxMCSPDUsize)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::maxMCSPDUsize tag error");
                return -1;
            }
            if ( -1 == this->in_ber_int(stream, this->protocolVersion)){
                LOG(LOG_ERR, "Connect Initial BER_TAG_MCS_DOMAIN_PARAMS::protocolVersion tag error");
                return -1;
            }
            if (stream.get_offset(start_offset) != len){
                LOG(LOG_ERR, "Connect Initial, bad length in BER_TAG_MCS_DOMAIN_PARAMS. Total subfield length mismatch %u %u", stream.get_offset(start_offset), len);
            }
            return 0;
        }
    };

    struct CONNECT_INITIAL_PDU_Recv
    {
        struct DomainParameters targetParameters;
        struct DomainParameters minimumParameters;
        struct DomainParameters maximumParameters;

        size_t header_size;
        size_t payload_size;

        uint16_t tag;
        size_t tag_len;

        uint8_t callingDomainSelector[256];
        int len_callingDomainSelector;

        uint8_t calledDomainSelector[256];
        int len_calledDomainSelector;

        bool upwardFlag;

        CONNECT_INITIAL_PDU_Recv(Stream & stream, size_t available_length, int encoding)
        {
            this->tag = stream.in_uint16_be();
            if (BER_TAG_ConnectMCSPDU_CONNECT_INITIAL != this->tag){
                LOG(LOG_ERR, "Connect Initial::CONNECT_INITIAL tag expected, got %u", this->tag);
                throw Error(ERR_MCS);
            }
            this->tag = MCSPDU_CONNECT_INITIAL;
            this->tag_len = stream.in_ber_len();

            this->len_callingDomainSelector = stream.in_ber_octet_string(callingDomainSelector, sizeof(callingDomainSelector));
            if (-1 == this->len_callingDomainSelector){
                LOG(LOG_ERR, "Connect Initial::bad callingDomainSelector");
                throw Error(ERR_MCS);
            }

            this->len_calledDomainSelector = stream.in_ber_octet_string(calledDomainSelector, sizeof(calledDomainSelector));
            if (-1 == this->len_calledDomainSelector){
                LOG(LOG_ERR, "Connect Initial::bad calledDomainSelector");
                throw Error(ERR_MCS);
            }

//        upwardFlag              BOOLEAN, -- TRUE if called provider is higher
            int upward = stream.in_ber_boolean();
            if (-1 == upward){
                LOG(LOG_ERR, "Connect Initial::bad upwardFlag");
                throw Error(ERR_MCS);
            }
            this->upwardFlag = upward;

            if (-1 == this->targetParameters.recv(stream)){
                LOG(LOG_ERR, "Connect Initial::bad targetParameters");
                throw Error(ERR_MCS);
            }
            
            if (-1 == this->minimumParameters.recv(stream)){
                LOG(LOG_ERR, "Connect Initial::bad minimumParameters");
                throw Error(ERR_MCS);
            }

            if (-1 == this->maximumParameters.recv(stream)){
                LOG(LOG_ERR, "Connect Initial::bad maximumParameters");
                throw Error(ERR_MCS);
            }

//        userData                OCTET STRING
            if (Stream::BER_TAG_OCTET_STRING != stream.in_uint8()){
                LOG(LOG_ERR, "ConnectInitial::BER payload size, tag mismatch, expected (BER_TAG_OCTET_STRING)");
                throw Error(ERR_MCS);
            }
            this->payload_size = stream.in_ber_len();
            if (this->payload_size != (size_t)(stream.end - stream.p)){
                LOG(LOG_ERR, "ConnectInitial::BER payload size (%u) does not match available data size (%u)",
                    this->payload_size, stream.end - stream.p);
                throw Error(ERR_MCS);
            }
            TODO("Octets below are part of GCC Conference User Data")
//            stream.in_skip_bytes(23);

// The payload is the USER_DATA block
            this->header_size  = stream.p - stream.data;
        }
    };

    struct CONNECT_INITIAL_Send
    {
        CONNECT_INITIAL_Send(Stream & stream, size_t payload_length, int encoding)
        {
            stream.out_uint16_be(BER_TAG_ConnectMCSPDU_CONNECT_INITIAL);
            stream.out_ber_len_uint16(0); // filled later, 3 bytes

            stream.out_uint8(Stream::BER_TAG_OCTET_STRING);
            stream.out_ber_len(1); /* calling domain */
            stream.out_uint8(1);
            stream.out_uint8(Stream::BER_TAG_OCTET_STRING);
            stream.out_ber_len(1); /* called domain */
            stream.out_uint8(1);
            stream.out_uint8(Stream::BER_TAG_BOOLEAN);
            stream.out_ber_len(1);
            stream.out_uint8(0xff); /* upward flag */

            // target params
            stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
            stream.out_ber_len(26);      // 26 = 0x1a
            stream.out_ber_int8(34);     // max_channels
            stream.out_ber_int8(2);      // max_users
            stream.out_ber_int8(0);      // max_tokens
            stream.out_ber_int8(1);
            stream.out_ber_int8(0);
            stream.out_ber_int8(1);
            stream.out_ber_int24(0xffff); // max_pdu_size
            stream.out_ber_int8(2);

            // min params
            stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
            stream.out_ber_len(25);     // 25=0x19
            stream.out_ber_int8(1);     // max_channels
            stream.out_ber_int8(1);     // max_users
            stream.out_ber_int8(1);     // max_tokens
            stream.out_ber_int8(1);
            stream.out_ber_int8(0);
            stream.out_ber_int8(1);
            stream.out_ber_int16(0x420); // max_pdu_size
            stream.out_ber_int8(2);

            // max params
            stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
            stream.out_ber_len(31);
            stream.out_ber_int24(0xffff); // max_channels
            stream.out_ber_int16(0xfc17); // max_users
            stream.out_ber_int24(0xffff); // max_tokens
            stream.out_ber_int8(1);
            stream.out_ber_int8(0);
            stream.out_ber_int8(1);
            stream.out_ber_int24(0xffff); // max_pdu_size
            stream.out_ber_int8(2);

            stream.out_uint8(Stream::BER_TAG_OCTET_STRING);
            stream.out_ber_len_uint16(payload_length);
            // now we know full MCS Initial header length (without initial tag and len)
            stream.set_out_ber_len_uint16(payload_length + stream.get_offset(5), 2);
            stream.end = stream.p;
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
        uint16_t tag;
        size_t tag_len;

        uint8_t result;
        int connectId;

        struct DomainParameters domainParameters;

        size_t header_size;
        size_t payload_size;

        CONNECT_RESPONSE_PDU_Recv(Stream & stream, size_t available_length, int encoding)
        {
            this->tag = stream.in_uint16_be();
            if (BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE != this->tag){
                LOG(LOG_ERR, "recv connect response (%u) expected, got (%u)",
                    MCS::MCSPDU_CONNECT_RESPONSE, this->tag);
                throw Error(ERR_MCS);
            }
            this->tag = MCS::MCSPDU_CONNECT_RESPONSE;
            this->tag_len = stream.in_ber_len();

            if (stream.in_uint8() != Stream::BER_TAG_RESULT) {
                LOG(LOG_ERR, "recv connect response result expected");
                throw Error(ERR_MCS);
            }
            if (1 != stream.in_ber_len()){
                LOG(LOG_ERR, "result length should be 1");
                throw Error(ERR_MCS);
            }                
            this->result = stream.in_uint8();

            if (stream.in_uint8() != Stream::BER_TAG_INTEGER) {
                LOG(LOG_ERR, "recv connect response connectId type not Integer");
                throw Error(ERR_MCS);
            }
            {
                size_t len = stream.in_ber_len();
                this->connectId = stream.in_bytes_le(len); /* connect id */
            }

            if (-1 == this->domainParameters.recv(stream)){
                LOG(LOG_ERR, "Connect Initial::bad targetParameters");
                throw Error(ERR_MCS);
            }

//        userData                OCTET STRING
            stream.in_uint8();
            this->payload_size = stream.in_ber_len();
            if (this->payload_size != (size_t)(stream.end - stream.p)){
                LOG(LOG_ERR, "ConnectInitial::BER payload size (%u) does not match available data size (%u)",
                    this->payload_size, stream.end - stream.p);
                throw Error(ERR_MCS);
            }
            TODO("Octets below are part of GCC Conference User Data")
//            stream.in_skip_bytes(23);

// The payload is the USER_DATA block
            this->header_size  = stream.p - stream.data;
        }
    };

    struct CONNECT_RESPONSE_Send
    {
        CONNECT_RESPONSE_Send(Stream & stream, size_t payload_length, int encoding)
        {
            // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
            stream.out_uint16_be(BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE);
            // BER: Type Length
            if (payload_length > 88){
                stream.out_ber_len_uint16(0);
            }
            else {
                stream.out_ber_len_uint7(0);
            }
            uint16_t start_offset = stream.get_offset(0);

            // Connect-Response::result = rt-successful (0)
            // The first byte (0x0a) is the ASN.1 BER encoded Enumerated type. The
            // length of the value is given by the second byte (1 byte), and the
            // actual value is 0 (rt-successful).
            stream.out_uint8(Stream::BER_TAG_RESULT);
            stream.out_ber_len_uint7(1);
            stream.out_uint8(0);

            // Connect-Response::calledConnectId = 0
            stream.out_uint8(Stream::BER_TAG_INTEGER);
            stream.out_ber_len_uint7(1);
            stream.out_uint8(0);

            // Connect-Response::domainParameters (26 bytes)
            stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
            stream.out_ber_len_uint7(26);
            // DomainParameters::maxChannelIds = 34
            stream.out_ber_int8(34);
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

            stream.out_uint8(Stream::BER_TAG_OCTET_STRING);
            stream.out_ber_len(payload_length);

            // now we know full MCS Initial header length (without initial tag and len)
            if (payload_length > 88){
                stream.set_out_ber_len_uint16(payload_length + stream.get_offset(start_offset), 2);
            }
            else {
                stream.set_out_ber_len_uint7(payload_length + stream.get_offset(start_offset), 2);
            }
            stream.end = stream.p;
        }
    };

//    PlumbDomainIndication ::= [APPLICATION 0] IMPLICIT SEQUENCE
//    {
//        heightLimit     INTEGER (0..MAX)
//                        -- a restriction on the MCSPDU receiver
//    }
    struct PlumbDomainIndication_Send
    {
        PlumbDomainIndication_Send(Stream & stream, int encoding)
        {
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
        }
    };

    struct PlumbDomainIndication_Recv
    {
        PlumbDomainIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
            LOG(LOG_ERR, "Not Implemented, not used by RDP protocol");
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

//    {
//        BStream stream(65536);
//        stream.out_uint8((MCSPDU_ErectDomainRequest << 2));
//        stream.out_per_integer(0); /* subHeight (INTEGER) */
//        stream.out_per_integer(0); /* subInterval (INTEGER) */
//        stream.end = stream.p;

//        BStream x224_header(256);
//        X224::DT_TPDU_Send(x224_header, stream.end - stream.data);

//        trans->send(x224_header.data, x224_header.end - x224_header.data);
//        trans->send(stream.data, stream.end - stream.data);
//    }

    struct ErectDomainRequest_Send
    {
        ErectDomainRequest_Send(Stream & stream, uint32_t subheight, uint32_t subinterval, int encoding)
        {
            stream.out_uint8((MCSPDU_ErectDomainRequest << 2));
            stream.out_per_integer(subheight); /* subHeight (INTEGER) */
            stream.out_per_integer(subinterval); /* subInterval (INTEGER) */
            stream.end = stream.p;
        }
    };

    struct ErectDomainRequest_Recv
    {
        uint8_t type;
        uint32_t subHeight;
        uint32_t subInterval;

        ErectDomainRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
            uint8_t tag = stream.in_uint8();
            if ((MCS::MCSPDU_ErectDomainRequest << 2) != tag){
                LOG(LOG_ERR, "ErectDomainRequest expected, got %u", tag);
                throw Error(ERR_MCS);
            }
            this->type = MCS::MCSPDU_ErectDomainRequest;
            this->subHeight = stream.in_per_integer();
            this->subInterval = stream.in_per_integer();
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
        MergeChannelRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct MergeChannelRequest_Recv
    {
        MergeChannelRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    MergeChannelsConfirm ::= [APPLICATION 3] IMPLICIT SEQUENCE
//    {
//        mergeChannels   SET OF ChannelAttributes,
//        purgeChannelIds SET OF ChannelId
//    }

    struct MergeChannelsConfirm_Send
    {
        MergeChannelsConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct MergeChannelsConfirm_Recv
    {
        MergeChannelsConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        PurgeChannelsIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct PurgeChannelsIndication_Recv
    {
        PurgeChannelsIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        MergeTokensRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct MergeTokensRequest_Recv
    {
        MergeTokensRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    MergeTokensConfirm ::= [APPLICATION 6] IMPLICIT SEQUENCE
//    {
//        mergeTokens     SET OF TokenAttributes,
//        purgeTokenIds   SET OF TokenId
//    }

    struct MergeTokensConfirm_Send
    {
        MergeTokensConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct MergeTokensConfirm_Recv
    {
        MergeTokensConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    PurgeTokensIndication ::= [APPLICATION 7] IMPLICIT SEQUENCE
//    {
//        purgeTokenIds   SET OF TokenId
//    }

    struct PurgeTokensIndication_Send
    {
        PurgeTokensIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct PurgeTokensIndication_Recv
    {
        PurgeTokensIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    DisconnectProviderUltimatum ::= [APPLICATION 8] IMPLICIT SEQUENCE
//    {
//        reason          Reason
//    }

    struct DisconnectProviderUltimatum_Send
    {
        DisconnectProviderUltimatum_Send(Stream & stream, int encoding)
        {
        }
    };

    struct DisconnectProviderUltimatum_Recv
    {
        DisconnectProviderUltimatum_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    RejectMCSPDUUltimatum ::= [APPLICATION 9] IMPLICIT SEQUENCE
//    {
//        diagnostic      Diagnostic,
//        initialOctets   OCTET STRING
//    }

    struct RejectMCSPDUUltimatum_Send
    {
        RejectMCSPDUUltimatum_Send(Stream & stream, int encoding)
        {
        }
    };

    struct RejectMCSPDUUltimatum_Recv
    {
        RejectMCSPDUUltimatum_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    AttachUserRequest ::= [APPLICATION 10] IMPLICIT SEQUENCE
//    {
//    }

    struct AttachUserRequest_Send
    {
        AttachUserRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct AttachUserRequest_Recv
    {
        AttachUserRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    AttachUserConfirm ::= [APPLICATION 11] IMPLICIT SEQUENCE
//    {
//        result          Result,
//        initiator       UserId OPTIONAL
//    }

    struct AttachUserConfirm_Send
    {
        AttachUserConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct AttachUserConfirm_Recv
    {
        AttachUserConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    DetachUserRequest ::= [APPLICATION 12] IMPLICIT SEQUENCE
//    {
//        reason          Reason,
//        userIds         SET OF UserId
//    }

    struct DetachUserRequest_Send
    {
        DetachUserRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct DetachUserRequest_Recv
    {
        DetachUserRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    DetachUserIndication ::= [APPLICATION 13] IMPLICIT SEQUENCE
//    {
//        reason          Reason,
//        userIds         SET OF UserId
//    }

    struct DetachUserIndication_Send
    {
        DetachUserIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct DetachUserIndication_Recv
    {
        DetachUserIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId
//                        -- may be zero
//    }

    struct ChannelJoinRequest_Send
    {
        ChannelJoinRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelJoinRequest_Recv
    {
        ChannelJoinRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelJoinConfirm ::= [APPLICATION 15] IMPLICIT SEQUENCE
//    {
//        result          Result,
//        initiator       UserId,
//        requested       ChannelId,
//                        -- may be zero
//        channelId       ChannelId OPTIONAL
//    }

    struct ChannelJoinConfirm_Send
    {
        ChannelJoinConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelJoinConfirm_Recv
    {
        ChannelJoinConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelLeaveRequest ::= [APPLICATION 16] IMPLICIT SEQUENCE
//    {
//        channelIds      SET OF ChannelId
//    }

    struct ChannelLeaveRequest_Send
    {
        ChannelLeaveRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelLeaveRequest_Recv
    {
        ChannelLeaveRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelConveneRequest ::= [APPLICATION 17] IMPLICIT SEQUENCE
//    {
//        initiator       UserId
//    }

    struct ChannelConveneRequest_Send
    {
        ChannelConveneRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelConveneRequest_Recv
    {
        ChannelConveneRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        ChannelConveneConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelConveneConfirm_Recv
    {
        ChannelConveneConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelDisbandRequest ::= [APPLICATION 19] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       PrivateChannelId
//    }

    struct ChannelDisbandRequest_Send
    {
        ChannelDisbandRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelDisbandRequest_Recv
    {
        ChannelDisbandRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelDisbandIndication ::= [APPLICATION 20] IMPLICIT SEQUENCE
//    {
//        channelId       PrivateChannelId
//    }

    struct ChannelDisbandIndication_Send
    {
        ChannelDisbandIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelDisbandIndication_Recv
    {
        ChannelDisbandIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        ChannelAdmitRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelAdmitRequest_Recv
    {
        ChannelAdmitRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        ChannelAdmitIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelAdmitIndication_Recv
    {
        ChannelAdmitIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        ChannelExpelRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelExpelRequest_Recv
    {
        ChannelExpelRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    ChannelExpelIndication ::= [APPLICATION 24] IMPLICIT SEQUENCE
//    {
//        channelId       PrivateChannelId,
//        userIds         SET OF UserId
//    }

    struct ChannelExpelIndication_Send
    {
        ChannelExpelIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelExpelIndication_Recv
    {
        ChannelExpelIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    SendDataRequest ::= [APPLICATION 25] IMPLICIT SEQUENCE
//    {
//        initiator       UserId,
//        channelId       ChannelId,
//        dataPriority    DataPriority,
//        segmentation    Segmentation,
//        userData        OCTET STRING
//    }

    struct ChannelSendDataRequest_Send
    {
        ChannelSendDataRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct ChannelSendDataRequest_Recv
    {
        ChannelSendDataRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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

    struct ChannelSendDataIndication_Send
    {
        ChannelSendDataIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct SendDataIndication_Recv
    {
        SendDataIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        UniformSendDataRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct UniformSendDataRequest_Recv
    {
        UniformSendDataRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        UniformSendDataIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct UniformSendDataIndication_Recv
    {
        UniformSendDataIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenGrabRequest ::= [APPLICATION 29] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenGrabRequest_Send
    {
        TokenGrabRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGrabRequest_Recv
    {
        TokenGrabRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenGrabConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGrabConfirm_Recv
    {
        TokenGrabConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenInhibitRequest ::= [APPLICATION 31] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenInhibitRequest_Send
    {
        TokenInhibitRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenInhibitRequest_Recv
    {
        TokenInhibitRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenInhibitConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenInhibitConfirm_Recv
    {
        TokenInhibitConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenGiveRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGiveRequest_Recv
    {
        TokenGiveRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenGiveIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGiveIndication_Recv
    {
        TokenGiveIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenGiveResponse_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGiveResponse_Recv
    {
        TokenGiveResponse_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenGiveConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenGiveConfirm_Recv
    {
        TokenGiveConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenPleaseRequest ::= [APPLICATION 37] IMPLICIT SEQUENCE
//    {
//        initiator    UserId,
//        tokenId      TokenId
//    }

    struct TokenPleaseRequest_Send
    {
        TokenPleaseRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenPleaseRequest_Recv
    {
        TokenPleaseRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenPleaseIndication ::= [APPLICATION 38] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }


    struct TokenPleaseIndication_Send
    {
        TokenPleaseIndication_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenPleaseIndication_Recv
    {
        TokenPleaseIndication_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenReleaseRequest ::= [APPLICATION 39] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenReleaseRequest_Send
    {
        TokenReleaseRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenReleaseRequest_Recv
    {
        TokenReleaseRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenReleaseConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenReleaseConfirm_Recv
    {
        TokenReleaseConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

//    TokenTestRequest ::= [APPLICATION 41] IMPLICIT SEQUENCE
//    {
//        initiator   UserId,
//        tokenId     TokenId
//    }

    struct TokenTestRequest_Send
    {
        TokenTestRequest_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenTestRequest_Recv
    {
        TokenTestRequest_Recv(Stream & stream, size_t available_length, int encoding)
        {
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
        TokenTestConfirm_Send(Stream & stream, int encoding)
        {
        }
    };

    struct TokenTestConfirm_Recv
    {
        TokenTestConfirm_Recv(Stream & stream, size_t available_length, int encoding)
        {
        }
    };

};


#endif
