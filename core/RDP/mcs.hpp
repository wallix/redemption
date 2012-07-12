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

//        ChannelJoinRequest ::= [APPLICATION 14] IMPLICIT SEQUENCE
//        {
//            initiator       UserId,
//            channelId       ChannelId
//                            -- may be zero
//        }

// Not yet used: idea for mapping Mcs to actual data sructures
//struct ChannelJoinRequest
//{
//    uint16_t user_id;
//    uint16_t chan_id;

//    ChannelJoinRequest(uint16_t user_id, uint16_t chan_id)
//    : user_id(user_id)
//    , chan_id(chan_id)
//    {
//    }
//    void per_emit(){
//        stream.out_uint16_be(this->user_id);
//        stream.out_uint16_be(this->chan_id);
//    }
//    void per_recv(){
//        this->user_id = stream.in_uint16_be();
//        this->chan_id = stream.in_uint16_be();
//    }
//};

//##############################################################################
struct Mcs
//##############################################################################
{
    Stream & stream;
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

            stream.out_uint8(PER_DomainMCSPDU_CHOICE_ChannelJoinConfirm);
            stream.out_uint8(0); // Result = rt_successfull            
            stream.out_uint16_be(user_id);
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
    } // END METHOD recv_begin

    //==============================================================================
    void recv_end(){
    //==============================================================================
        if (this->stream.p != this->stream.end){
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


static inline void mcs_send_connect_initial(
            Transport * trans,
            const ChannelDefArray & channel_list,
            const uint16_t front_width,
            const uint16_t front_height,
            const uint8_t front_bpp,
            int keylayout,
            char * hostname,
            const int use_rdp5,
            const bool console_session,
            const uint32_t tls){

//    Stream data(8192);

//    int data_len = data.end - data.data;
//    int len = 7 + 3 * 34 + 4 + data_len;

    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);

    stream.out_uint16_be(BER_TAG_ConnectMCSPDU_CONNECT_INITIAL);
    uint32_t offset_data_len_connect_initial = stream.get_offset(0);
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
    uint32_t offset_data_len = stream.get_offset(0);
    stream.out_ber_len_uint16(0); // filled later, 3 bytes

    /* Generic Conference Control (T.124) ConferenceCreateRequest */

    size_t offset_gcc_conference_create_request_header_length = 0;
    gcc_write_conference_create_request_header(stream, offset_gcc_conference_create_request_header_length);

    size_t offset_user_data_length = stream.get_offset(0);
    stream.out_per_length(256); // remaining length, reserve 16 bits

    // Client User Data
    // ================
    // 158 bytes
    CSCoreGccUserData cs_core;
    cs_core.version = use_rdp5?0x00080004:0x00080001;
    cs_core.desktopWidth = front_width;
    cs_core.desktopHeight = front_height;
    cs_core.highColorDepth = front_bpp;
    cs_core.keyboardLayout = keylayout;
    uint16_t hostlen = strlen(hostname);
    uint16_t maxhostlen = std::min((uint16_t)15, hostlen);
    for (size_t i = 0; i < maxhostlen ; i++){
        cs_core.clientName[i] = hostname[i];
    }
    bzero(&(cs_core.clientName[hostlen]), 16-hostlen);
    if (tls){
        cs_core.serverSelectedProtocol = 1;
    }
    cs_core.log("Sending to Server");
    if (tls){
    }
    cs_core.emit(stream);

    CSClusterGccUserData cs_cluster;
    TODO("values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?")

    if (!tls){
         if (console_session){
            cs_cluster.flags = CSClusterGccUserData::REDIRECTED_SESSIONID_FIELD_VALID | (3 << 2) ; // REDIRECTION V4
        }
        else {
            cs_cluster.flags = CSClusterGccUserData::REDIRECTION_SUPPORTED            | (2 << 2) ; // REDIRECTION V3
        }
    }
    else {
        cs_cluster.flags = CSClusterGccUserData::REDIRECTION_SUPPORTED * ((3 << 2)|1);  // REDIRECTION V4
        if (console_session){
            cs_cluster.flags |= CSClusterGccUserData::REDIRECTED_SESSIONID_FIELD_VALID ;
        }
    }
    cs_cluster.log("Sending to server");
    cs_cluster.emit(stream);

    // 12 bytes
    CSSecGccUserData cs_sec_gccuserdata;
    cs_sec_gccuserdata.encryptionMethods = FORTY_BIT_ENCRYPTION_FLAG|HUNDRED_TWENTY_EIGHT_BIT_ENCRYPTION_FLAG;
    cs_sec_gccuserdata.log("Sending cs_sec gccuserdata to server");
    cs_sec_gccuserdata.emit(stream);

    // 12 * nbchan + 8 bytes
    mod_rdp_out_cs_net(stream, channel_list);

    stream.set_out_per_length(stream.get_offset(offset_user_data_length + 2), offset_user_data_length); // user data length

    stream.set_out_per_length(stream.get_offset(offset_gcc_conference_create_request_header_length + 2), offset_gcc_conference_create_request_header_length); // length including header

    // set mcs_data len, Stream::BER_TAG_OCTET_STRING (some kind of BLOB)
    LOG(LOG_INFO, "mcs_data_len = %u", stream.get_offset(offset_data_len + 3));
    stream.set_out_ber_len_uint16(stream.get_offset(offset_data_len + 3), offset_data_len);

    // set mcs_data len for BER_TAG_ConnectMCSPDU_CONNECT_INITIAL

    LOG(LOG_INFO, "offset_data_len_connect_initial = %u", stream.get_offset(offset_data_len_connect_initial + 3));
    stream.set_out_ber_len_uint16(stream.get_offset(offset_data_len_connect_initial + 3), offset_data_len_connect_initial);

    x224.emit_end();
    trans->send(x224.header(), x224.size());
}

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

static inline void mcs_recv_connect_response(
                        Transport * trans,
                        ChannelDefArray & mod_channel_list,
                        const ChannelDefArray & front_channel_list,
                        CryptContext & encrypt, CryptContext & decrypt,
                        uint32_t & server_public_key_len,
                        uint8_t (& client_crypt_random)[512],
                        int & crypt_level,
                        int & use_rdp5,
                        Random * gen)
{
    Stream cr_stream(32768);
    X224 x224(cr_stream);
    x224.recv_begin(trans);

    if (cr_stream.in_uint16_be() != BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE) {
        LOG(LOG_ERR, "recv connect response expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = cr_stream.in_ber_len();
    // ----------------------------------------------------------

    if (cr_stream.in_uint8() != Stream::BER_TAG_RESULT) {
        LOG(LOG_ERR, "recv connect response result expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    // ----------------------------------------------------------
    int res = cr_stream.in_uint8();

//Result ::= ENUMERATED
//-- in Connect, response, confirm
//{
//    rt-successful                  (0),
//    rt-domain-merging              (1),
//    rt-domain-not-hierarchical     (2),
//    rt-no-such-channel             (3),
//    rt-no-such-domain              (4),
//    rt-no-such-user                (5),
//    rt-not-admitted                (6),
//    rt-other-user-id               (7),
//    rt-parameters-unacceptable     (8),
//    rt-token-not-available         (9),
//    rt-token-not-possessed         (10),
//    rt-too-many-channels           (11),
//    rt-too-many-tokens             (12),
//    rt-too-many-users              (13),
//    rt-unspecified-failure         (14),
//    rt-user-rejected               (15)
//}

    if (res != 0) {
        LOG(LOG_ERR, "recv connect response result OK expected");
        throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
    }
    if (cr_stream.in_uint8() != Stream::BER_TAG_INTEGER) {
        LOG(LOG_ERR, "recv connect response result OK integer expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.in_skip_bytes(len); /* connect id */

    if (cr_stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        LOG(LOG_ERR, "recv connect response Domain param expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.in_skip_bytes(len);

    if (cr_stream.in_uint8() != Stream::BER_TAG_OCTET_STRING) {
        LOG(LOG_ERR, "recv connect response Domain param string value expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();

    cr_stream.in_skip_bytes(21); /* header (T.124 ConferenceCreateResponse) */
    len = cr_stream.in_uint8();

    if (len & 0x80) {
        len = cr_stream.in_uint8();
    }
    while (cr_stream.p < cr_stream.end) {
        uint16_t tag = cr_stream.in_uint16_le();
        uint16_t length = cr_stream.in_uint16_le();
        if (length <= 4) {
            LOG(LOG_ERR, "recv connect response parsing gcc data : short header");
            throw Error(ERR_MCS_DATA_SHORT_HEADER);
        }
        uint8_t *next_tag = (cr_stream.p + length) - 4;
        switch (tag) {
        case SC_CORE:
        {
            SCCoreGccUserData sc_core;
            sc_core.recv(cr_stream, length);
            sc_core.log("Receiving SC_CORE from server");
            if (0x0080001 == sc_core.version){ // can't use rdp5
                use_rdp5 = 0;
            }
        }
        break;
        case SC_SECURITY:
            LOG(LOG_INFO, "Receiving SC_Security from server");
            parse_mcs_data_sc_security(cr_stream, encrypt, decrypt,
                                       server_public_key_len, client_crypt_random,
                                       crypt_level,
                                       gen);
        break;
        case SC_NET:
            LOG(LOG_INFO, "Receiving SC_Net from server");
            parse_mcs_data_sc_net(cr_stream, front_channel_list, mod_channel_list);
            break;
        default:
            LOG(LOG_WARNING, "response tag 0x%x", tag);
            break;
        }
        cr_stream.p = next_tag;
    }
}


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

static inline void mcs_recv_connect_initial(
                Transport * trans,
                ClientInfo * client_info,
                ChannelDefArray & channel_list)
{
    Stream stream(32768);
    X224 x224(stream);
    x224.recv_begin(trans);

    if (stream.in_uint16_be() != BER_TAG_ConnectMCSPDU_CONNECT_INITIAL) {
        LOG(LOG_ERR, "Recv connect initial expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = stream.in_ber_len();
    if (stream.in_uint8() != Stream::BER_TAG_OCTET_STRING) {
        LOG(LOG_ERR, "Recv connect initial string expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != Stream::BER_TAG_OCTET_STRING) {
        LOG(LOG_ERR, "Recv connect initial string 2 expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);
    if (stream.in_uint8() != Stream::BER_TAG_BOOLEAN) {
        LOG(LOG_ERR, "Recv connect initial boolean expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        LOG(LOG_ERR, "Recv connect initial Domain Params expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        LOG(LOG_ERR, "Recv connect initial Domain Params 2 expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        LOG(LOG_ERR, "Recv connect initial Domain Params 3 expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.in_skip_bytes(len);

    if (stream.in_uint8() != Stream::BER_TAG_OCTET_STRING) {
        LOG(LOG_ERR, "Recv connect initial Domain Params 4 expected");
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();

    stream.in_skip_bytes(23);

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

    while (stream.check_rem(4)) {
        uint8_t * current_header = stream.p;
        uint16_t tag = stream.in_uint16_le();
        uint16_t length = stream.in_uint16_le();
        if (length < 4 || !stream.check_rem(length - 4)) {
            LOG(LOG_ERR,
                "error reading block tag %d size %d\n",
                tag, length);
            break;
        }

        switch (tag){
            case CS_CORE:
            {
                CSCoreGccUserData cs_core;
                cs_core.recv(stream, length);
                client_info->width = cs_core.desktopWidth;
                client_info->height = cs_core.desktopHeight;
                client_info->keylayout = cs_core.keyboardLayout;
                client_info->build = cs_core.clientBuild;
                for (size_t i = 0; i < 16 ; i++){
                    client_info->hostname[i] = cs_core.clientName[i];
                }
                client_info->bpp = 8;
                switch (cs_core.postBeta2ColorDepth){
                case 0xca01:
                    client_info->bpp = (cs_core.highColorDepth <= 24)?cs_core.highColorDepth:24;
                break;
                case 0xca02:
                    client_info->bpp = 15;
                break;
                case 0xca03:
                    client_info->bpp = 16;
                break;
                case 0xca04:
                    client_info->bpp = 24;
                break;
                default:
                break;
                }
                cs_core.log("Receiving from Client");
            }
            break;
            case CS_SECURITY:
                parse_mcs_data_cs_security(stream);
            break;
            case CS_NET:
                parse_mcs_data_cs_net(stream, client_info, channel_list);
            break;
            case CS_CLUSTER:
            {
                CSClusterGccUserData cs_cluster;
                cs_cluster.recv(stream, length);
                client_info->console_session =
                    (0 != (cs_cluster.flags & CSClusterGccUserData::REDIRECTED_SESSIONID_FIELD_VALID));
                cs_cluster.log("Receiving from Client");
            }
            break;
            case CS_MONITOR:
            {
                CSMonitorGccUserData cs_monitor;
                cs_monitor.recv(stream, length);
                cs_monitor.log("Receiving from Client");
            }
            break;
            default:
                LOG(LOG_INFO, "Unexpected data block tag %x\n", tag);
            break;
        }
        stream.p = current_header + length;
    }
}


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


// 2.2.1.3.2 Client Core Data (TS_UD_CS_CORE)
// ------------------------------------------

//The TS_UD_CS_CORE data block contains core client connection-related
// information.

//header (4 bytes): GCC user data block header, as specified in section
//                  2.2.1.3.1. The User Data Header type field MUST be set to
//                  CS_CORE (0xC001).

// version (4 bytes): A 32-bit, unsigned integer. Client version number for the
//                    RDP. The major version number is stored in the high 2
//                    bytes, while the minor version number is stored in the
//                    low 2 bytes.
// +------------+------------------------------------+
// |   Value    |    Meaning                         |
// +------------+------------------------------------+
// | 0x00080001 | RDP 4.0 clients                    |
// +------------+------------------------------------+
// | 0x00080004 | RDP 5.0, 5.1, 5.2, and 6.0 clients |
// +------------+------------------------------------+

// desktopWidth (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                         width in pixels (up to a maximum value of 4096
//                         pixels).

// desktopHeight (2 bytes): A 16-bit, unsigned integer. The requested desktop
//                          height in pixels (up to a maximum value of 2048
//                          pixels).

// colorDepth (2 bytes): A 16-bit, unsigned integer. The requested color depth.
//                       Values in this field MUST be ignored if the
//                       postBeta2ColorDepth field is present.
// +--------------------------+-------------------------+
// |     Value                |        Meaning          |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+

// SASSequence (2 bytes): A 16-bit, unsigned integer. Secure access sequence.
//                        This field SHOULD be set to RNS_UD_SAS_DEL (0xAA03).

// keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout (active
//                           input locale identifier). For a list of possible
//                           input locales, see [MSDN-MUI].

// clientBuild (4 bytes): A 32-bit, unsigned integer. The build number of the
//                        client.

// clientName (32 bytes): Name of the client computer. This field contains up to
//                        15 Unicode characters plus a null terminator.

// keyboardType (4 bytes): A 32-bit, unsigned integer. The keyboard type.
// +-------+--------------------------------------------+
// | Value |              Meaning                       |
// +-------+--------------------------------------------+
// |   1   | IBM PC/XT or compatible (83-key) keyboard  |
// +-------+--------------------------------------------+
// |   2   | Olivetti "ICO" (102-key) keyboard          |
// +-------+--------------------------------------------+
// |   3   | IBM PC/AT (84-key) and similar keyboards   |
// +-------+--------------------------------------------+
// |   4   | IBM enhanced (101- or 102-key) keyboard    |
// +-------+--------------------------------------------+
// |   5   | Nokia 1050 and similar keyboards           |
// +-------+--------------------------------------------+
// |   6   | Nokia 9140 and similar keyboards           |
// +-------+--------------------------------------------+
// |   7   | Japanese keyboard                          |
// +-------+--------------------------------------------+

// keyboardSubType (4 bytes): A 32-bit, unsigned integer. The keyboard subtype
//                            (an original equipment manufacturer-dependent
//                            value).

// keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. The number of
//                                function keys on the keyboard.

// imeFileName (64 bytes): A 64-byte field. The Input Method Editor (IME) file
//                         name associated with the input locale. This field
//                         contains up to 31 Unicode characters plus a null
//                         terminator.

// postBeta2ColorDepth (2 bytes): A 16-bit, unsigned integer. The requested
//                                color depth. Values in this field MUST be
//                                ignored if the highColorDepth field is
//                                present.
// +--------------------------+-------------------------+
// |      Value               |         Meaning         |
// +--------------------------+-------------------------+
// | 0xCA00 RNS_UD_COLOR_4BPP | 4 bits-per-pixel (bpp)  |
// +--------------------------+-------------------------+
// | 0xCA01 RNS_UD_COLOR_8BPP | 8 bpp                   |
// +--------------------------+-------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientProductId (2 bytes): A 16-bit, unsigned integer. The client product ID.
//                            This field SHOULD be initialized to 1. If this
//                            field is present, then all of the preceding fields
//                            MUST also be present. If this field is not
//                            present, then none of the subsequent fields MUST
//                            be present.

// serialNumber (4 bytes): A 32-bit, unsigned integer. Serial number. This field
//                         SHOULD be initialized to 0. If this field is present,
//                         then all of the preceding fields MUST also be
//                         present. If this field is not present, then none of
//                         the subsequent fields MUST be present.

// highColorDepth (2 bytes): A 16-bit, unsigned integer. The requested color
//                           depth.
// +-------+-------------------------------------------------------------------+
// | Value |                      Meaning                                      |
// +-------+-------------------------------------------------------------------+
// |     4 |   4 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |     8 |   8 bpp                                                           |
// +-------+-------------------------------------------------------------------+
// |    15 |  15-bit 555 RGB mask                                              |
// |       |  (5 bits for red, 5 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    16 |  16-bit 565 RGB mask                                              |
// |       |  (5 bits for red, 6 bits for green, and 5 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// |    24 |  24-bit RGB mask                                                  |
// |       |  (8 bits for red, 8 bits for green, and 8 bits for blue)          |
// +-------+-------------------------------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// supportedColorDepths (2 bytes): A 16-bit, unsigned integer. Specifies the
//                                 high color depths that the client is capable
//                                 of supporting.
// +-----------------------------+---------------------------------------------+
// |          Flag               |                Meaning                      |
// +-----------------------------+---------------------------------------------+
// | 0x0001 RNS_UD_24BPP_SUPPORT | 24-bit RGB mask                             |
// |                             | (8 bits for red, 8 bits for green,          |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0002 RNS_UD_16BPP_SUPPORT | 16-bit 565 RGB mask                         |
// |                             | (5 bits for red, 6 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0004 RNS_UD_15BPP_SUPPORT | 15-bit 555 RGB mask                         |
// |                             | (5 bits for red, 5 bits for green,          |
// |                             | and 5 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// | 0x0008 RNS_UD_32BPP_SUPPORT | 32-bit RGB mask                             |
// |                             | (8 bits for the alpha channel,              |
// |                             | 8 bits for red, 8 bits for green,           |
// |                             | and 8 bits for blue)                        |
// +-----------------------------+---------------------------------------------+
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// earlyCapabilityFlags (2 bytes): A 16-bit, unsigned integer. It specifies
// capabilities early in the connection sequence.
// +---------------------------------------------+-----------------------------|
// |                Flag                         |              Meaning        |
// +---------------------------------------------+-----------------------------|
// | 0x0001 RNS_UD_CS_SUPPORT_ERRINFO_PDU        | Indicates that the client   |
// |                                             | supports the Set Error Info |
// |                                             | PDU (section 2.2.5.1).      |
// +---------------------------------------------+-----------------------------|
// | 0x0002 RNS_UD_CS_WANT_32BPP_SESSION         | Indicates that the client is|
// |                                             | requesting a session color  |
// |                                             | depth of 32 bpp. This flag  |
// |                                             | is necessary because the    |
// |                                             | highColorDepth field does   |
// |                                             | not support a value of 32.  |
// |                                             | If this flag is set, the    |
// |                                             | highColorDepth field SHOULD |
// |                                             | be set to 24 to provide an  |
// |                                             | acceptable fallback for the |
// |                                             | scenario where the server   |
// |                                             | does not support 32 bpp     |
// |                                             | color.                      |
// +---------------------------------------------+-----------------------------|
// | 0x0004 RNS_UD_CS_SUPPORT_STATUSINFO_PDU     | Indicates that the client   |
// |                                             | supports the Server Status  |
// |                                             | Info PDU (section 2.2.5.2). |
// +---------------------------------------------+-----------------------------|
// | 0x0008 RNS_UD_CS_STRONG_ASYMMETRIC_KEYS     | Indicates that the client   |
// |                                             | supports asymmetric keys    |
// |                                             | larger than 512 bits for use|
// |                                             | with the Server Certificate |
// |                                             | (section 2.2.1.4.3.1) sent  |
// |                                             | in the Server Security Data |
// |                                             | block (section 2.2.1.4.3).  |
// +---------------------------------------------+-----------------------------|
// | 0x0020 RNS_UD_CS_RESERVED1                  | Reserved for future use.    |
// |                                             | This flag is ignored by the |
// |                                             | server.                     |
// +---------------------------------------------+-----------------------------+
// | 0x0040 RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU | Indicates that the client   |
// |                                             | supports the Monitor Layout |
// |                                             | PDU (section 2.2.12.1).     |
// +---------------------------------------------+-----------------------------|
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// clientDigProductId (64 bytes): Contains a value that uniquely identifies the
//                                client. If this field is present, then all of
//                                the preceding fields MUST also be present. If
//                                this field is not present, then none of the
//                                subsequent fields MUST be present.

// pad2octets (2 bytes): A 16-bit, unsigned integer. Padding to align the
//   serverSelectedProtocol field on the correct byte boundary.
// If this field is present, then all of the preceding fields MUST also be
// present. If this field is not present, then none of the subsequent fields
// MUST be present.

// serverSelectedProtocol (4 bytes): A 32-bit, unsigned integer. It contains the value returned
//   by the server in the selectedProtocol field of the RDP Negotiation Response structure
//   (section 2.2.1.2.1). In the event that an RDP Negotiation Response structure was not sent,
//   this field MUST be initialized to PROTOCOL_RDP (0). If this field is present, then all of the
//   preceding fields MUST also be present.

static inline void mcs_send_connect_response(
                        Transport * trans,
                        ClientInfo * client_info,
                        const ChannelDefArray & channel_list,
                        uint8_t (&server_random)[32],
                        int rc4_key_size,
                        uint8_t (&pub_mod)[512],
                        uint8_t (&pri_exp)[512],
                        Random * gen
                    ) throw(Error)
{
    Stream stream(32768);

    // TPKT Header (length = 337 bytes)
    // X.224 Data TPDU
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);

    // BER: Application-Defined Type = APPLICATION 102 = Connect-Response
    stream.out_uint16_be(BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE);
    uint32_t offset_len_mcs_connect_response = stream.get_offset(0);
    // BER: Type Length
    stream.out_ber_len_uint16(0); // filled later, 3 bytes

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
    stream.out_uint8(Stream::BER_TAG_OCTET_STRING);
    uint32_t offset_len_mcs_data = stream.get_offset(0);
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

//        uint16_t padding = channel_list.size() & 1;
//        uint16_t srv_channel_size = 8 + (channel_list.size() + padding) * 2;
//        stream.out_2BUE(8 + srv_channel_size + 236 + 4); // len


    uint32_t offset_user_data_len = stream.get_offset(0);
    stream.out_uint16_be(0);

    SCCoreGccUserData sc_core;
    sc_core.version = 0x00080004; // RDP 5
    sc_core.log("Sending SC_CORE to client");
    sc_core.emit(stream);

    out_mcs_data_sc_net(stream, channel_list);
    front_out_gcc_conference_user_data_sc_sec1(stream, client_info->crypt_level, server_random, rc4_key_size, pub_mod, pri_exp, gen);

    TODO(" create a function in stream that sets differed ber_len_offsets (or other len_offset)")

    // set user_data_len (TWO_BYTE_UNSIGNED_ENCODING)
    stream.set_out_uint16_be(0x8000 | (stream.get_offset(offset_user_data_len + 2)), offset_user_data_len);
    // set mcs_data len, Stream::BER_TAG_OCTET_STRING (some kind of BLOB)
    stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_data + 3), offset_len_mcs_data);
    // set BER_TAG_ConnectMCSPDU_CONNECT_RESPONSE len
    stream.set_out_ber_len_uint16(stream.get_offset(offset_len_mcs_connect_response + 3), offset_len_mcs_connect_response);

//    tpdu.end();
//    tpdu.send(trans);
    x224.emit_end();
    trans->send(x224.header(), x224.size());
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



static inline void mcs_send_erect_domain_and_attach_user_request_pdu(Transport * trans)
{
    LOG(LOG_INFO, "mcs_send_erect_domain_and_attach_user_request_pdu");
    Stream edrq_stream(32768);
    X224 edrq_x224(edrq_stream);
    edrq_x224.emit_begin(X224::DT_TPDU);
    edrq_stream.out_uint8((MCSPDU_ErectDomainRequest << 2));
    edrq_stream.out_per_integer(0); /* subHeight (INTEGER) */
    edrq_stream.out_per_integer(0); /* subInterval (INTEGER) */
    edrq_x224.emit_end();
    trans->send(edrq_x224.header(), edrq_x224.size());

    Stream aurq_stream(32768);
    X224 aurq_x224(aurq_stream);
    aurq_x224.emit_begin(X224::DT_TPDU);
    aurq_stream.out_uint8((MCSPDU_AttachUserRequest << 2));
    aurq_x224.emit_end();
    trans->send(aurq_x224.header(), aurq_x224.size());
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
    Stream cjrq_stream(32768);
    X224 x224(cjrq_stream);
    x224.emit_begin(X224::DT_TPDU);
    cjrq_stream.out_uint8((MCSPDU_ChannelJoinRequest << 2));
    cjrq_stream.out_uint16_be(userid);
    cjrq_stream.out_uint16_be(chanid);
    x224.emit_end();
    trans->send(x224.header(), x224.size());
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
    Stream stream(32768);
    X224 x224(stream);
    x224.recv_begin(trans);
    Mcs mcs(stream);
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
    x224.recv_end();
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
    Stream aucf_stream(32768);
    X224 x224(aucf_stream);
    x224.recv_begin(trans);
    int opcode = aucf_stream.in_uint8();
    if ((opcode >> 2) != MCSPDU_AttachUserConfirm) {
        LOG(LOG_ERR, "Attach user confirm pdu expected");
        throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
    }
    int res = aucf_stream.in_uint8();
    if (res != 0) {
        LOG(LOG_ERR, "Attach user confirm pdu OK expected");
        throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
    }
    if (opcode & 2) {
        userid = aucf_stream.in_uint16_be();
    }
    x224.recv_end();
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
        Stream stream(32768);
        X224 x224(stream);
        x224.recv_begin(trans);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCSPDU_ErectDomainRequest) {
            LOG(LOG_ERR, "Erect Domain Request expected");
            throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
        }
        stream.in_skip_bytes(2);
        stream.in_skip_bytes(2);
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }
        x224.recv_end();
    }

    {
        Stream stream(32768);
        X224 x224(stream);
        x224.recv_begin(trans);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCSPDU_AttachUserRequest) {
            LOG(LOG_ERR, "Attach User Request expected");
            throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
        }
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }
        x224.recv_end();
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
    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_AttachUserConfirm, userid, 0);
    mcs.emit_end();
    x224.emit_end();
    trans->send(x224.header(), x224.size());
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
    Stream stream(32768);
    X224 x224(stream);
    x224.emit_begin(X224::DT_TPDU);
    Mcs mcs(stream);
    mcs.emit_begin(MCSPDU_ChannelJoinConfirm, userid, chanid);
    x224.emit_end();
    trans->send(x224.header(), x224.size());
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
    Stream stream(32768);
    // read tpktHeader (4 bytes = 3 0 len)
    // TPDU class 0    (3 bytes = LI F0 PDU_DT)
    X224 x224(stream);
    x224.recv_begin(trans);
    Mcs mcs(stream);
    mcs.recv_begin();
    if ((mcs.opcode >> 2) != MCSPDU_ChannelJoinRequest) {
        LOG(LOG_INFO, "unexpected opcode = %u", mcs.opcode);
        throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
    }
    userid = mcs.user_id;
    chanid = mcs.chan_id;
    mcs.recv_end();
    x224.recv_end();
}

#endif
