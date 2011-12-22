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

#include "client_info.hpp"
#include "RDP/x224.hpp"
#include "channel_list.hpp"

#include "rsa_keys.hpp"
TODO(" ssl calls introduce some dependency on ssl system library  injecting it in the sec object would be better.")
#include "ssl_calls.hpp"

class McsOut
{
    Stream & stream;
    uint8_t offlen;
    public:
    McsOut(Stream & stream, uint8_t command, uint8_t user_id, uint16_t chan_id)
        : stream(stream), offlen(stream.p - stream.data + 6)
    {
        stream.out_uint8(command << 2);
        stream.out_uint16_be(user_id);
        stream.out_uint16_be(chan_id);
        stream.out_uint8(0x70);
        stream.skip_uint8(2); //len
    }

    void end(){
        int len = stream.p - stream.data - offlen - 2;
        stream.set_out_uint16_be(0x8000|len, this->offlen);
    }
};


class McsIn
{
    Stream & stream;
    public:
    uint8_t opcode;
    uint16_t user_id;
    uint16_t chan_id;
    uint8_t magic_0x70; // some ber header ?
    uint16_t len;

    McsIn(Stream & stream)
        : stream(stream), opcode(0), user_id(0), chan_id(0), len(0)
    {
        this->opcode = stream.in_uint8();
        this->user_id = stream.in_uint16_be();
        this->chan_id = stream.in_uint16_be();
        this->magic_0x70 = stream.in_uint8();
        this->len = stream.in_uint8();
        if (this->len & 0x80){
            this->len = ((this->len & 0x7F) << 8) + stream.in_uint8();
        }
    }

    void end(){
        if (this->stream.p != this->stream.end){
            LOG(LOG_ERR, "all data should have been consumed : remains %d", stream.end - stream.p);
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
//         0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 clients

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

TODO(" use official field names from MS-RDPBCGR")
static inline void parse_mcs_data_cs_core(Stream & stream, ClientInfo * client_info)
{
    LOG(LOG_INFO, "PARSE CS_CORE\n");
    uint16_t rdp_version = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: rdp_version (1=RDP1, 4=RDP5) %u\n", rdp_version);
    uint16_t dummy1 = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: ?? = %u\n", dummy1);
    client_info->width = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: width = %u\n", client_info->width);
    client_info->height = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: height = %u\n", client_info->height);
    uint16_t bpp_code = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: bpp_code = %x\n", bpp_code);
    uint16_t dummy2 = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: ?? = %x\n", dummy2);
    /* get keylayout */
    client_info->keylayout = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: layout = %x\n", client_info->keylayout);
    /* get build : windows build */
    client_info->build = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: build = %x\n", client_info->build);

    /* get hostname (it is UTF16, windows flavored widechars) */
    /* Unicode name of client is padded to 32 bytes */
    stream.in_uni_to_ascii_str(client_info->hostname, 32);
    LOG(LOG_INFO, "core_data: hostname = %s\n", client_info->hostname);

    uint32_t keyboard_type = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_type = %x\n", keyboard_type);
    uint32_t keyboard_subtype = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_subtype = %x\n", keyboard_subtype);
    uint32_t keyboard_functionkeys = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_functionkeys = %x\n", keyboard_functionkeys);
    stream.skip_uint8(64);

    client_info->bpp = 8;
    uint16_t i = stream.in_uint16_le();
    switch (i) {
    case 0xca01:
    {
        uint16_t clientProductId = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: clientProductId = %x", clientProductId);
        uint32_t serialNumber = stream.in_uint32_le();
        LOG(LOG_INFO, "core_data: serialNumber = %x", serialNumber);
        uint16_t rdp_bpp = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: rdp_bpp = %u", rdp_bpp);
        uint16_t supportedColorDepths = stream.in_uint16_le();
        LOG(LOG_INFO, "core_data: supportedColorDepths = %u", supportedColorDepths);

        client_info->bpp = (rdp_bpp <= 24)?rdp_bpp:24;
    }
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
    }
    LOG(LOG_INFO, "core_data: bpp = %u\n", client_info->bpp);
}

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

static inline void parse_mcs_data_cs_security(Stream & stream)
{
    LOG(LOG_INFO, "CS_SECURITY\n");
}

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
//                              control 0x00100000 transactions.


// this adds the mcs channels in the list of channels to be used when
// creating the server mcs data
static inline void parse_mcs_data_cs_net(Stream & stream, ClientInfo * client_info, ChannelList & channel_list)
{
    LOG(LOG_INFO, "CS_NET\n");
//    // this is an option set in rdpproxy.ini
//    // to disable all channels (no clipboard, no device redirection, etc)
//    if (client_info->channel_code != 1) { /* are channels on? */
//        return;
//    }
    uint32_t channelCount = stream.in_uint32_le();
    LOG(LOG_INFO, "cs_net:channel_count %u [%u]", channelCount, channel_list.size());

    for (uint32_t index = 0; index < channelCount; index++) {
        McsChannelItem channel_item;
        memcpy(channel_item.name, stream.in_uint8p(8), 8);
        channel_item.flags = stream.in_uint32_be();
        channel_item.chanid = MCS_GLOBAL_CHANNEL + (index + 1);
        TODO(" push_back is not the best choice here as we have static space already available in channel_list we could even let ChannelList manage parsing")
        channel_list.push_back(channel_item);
    }
}


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
// REDIRECTION_VERSION3                If REDIRECTION_SUPPORTED is set,
//          0x02                       server session redirection version 3
//                                     is supported by the client.
// -------------------------------------------------------------------------
// REDIRECTION_VERSION4                If REDIRECTION_SUPPORTED is set,
//          0x03                       server session redirection version 4
//                                     is supported by the client.
// -------------------------------------------------------------------------
// REDIRECTION_VERSION5                If REDIRECTION_SUPPORTED is set,
//          0x04                       server session redirection version 5
//                                     is supported by the client.
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

// This is this header that contains the console flag (undocumented ?)
static inline void parse_mcs_data_cs_cluster(Stream & stream, ClientInfo * client_info)
{
    LOG(LOG_INFO, "CS_CLUSTER\n");
    uint32_t flags = stream.in_uint32_le();
    LOG(LOG_INFO, "cluster_data: flags = %x\n", flags);
    client_info->console_session = (flags & 0x2) != 0;
}

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

static inline void parse_mcs_data_cs_monitor(Stream & stream)
{
    LOG(LOG_INFO, "CS_MONITOR\n");
}

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
// 0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, and 7.0 servers

// If the server advertises a version number greater than or equal to 0x00080004,
// it MUST support a maximum length of 512 bytes for the UserName field in the
// Info Packet (section 2.2.1.11.1.1).

// clientRequestedProtocols (4 bytes): A 32-bit, unsigned integer that contains
//  the flags sent by the client in the requestedProtocols field of the RDP
//  Negotiation Request (section 2.2.1.1.1). In the event that an RDP
//  Negotiation Request was not received from the client, this field MUST be
//  initialized to PROTOCOL_RDP (0).

TODO("Create SCCoreIn and SCCoreOut classes (on the model of SecIn/SecOut for Sec layer), or an SCCore object with emit() and receive() following the model of RDPOrders primitives")

static inline void parse_mcs_data_sc_core(Stream & stream, int & use_rdp5)
{
    LOG(LOG_INFO, "SC_CORE\n");
    uint32_t rdp_version = stream.in_uint32_le();
    LOG(LOG_DEBUG, "Remote RDP server supports version %s (was %s)\n",
            (rdp_version==0x0080001)?"RDP4":"RDP5",
            (use_rdp5)?"RDP5":"RDP4");
    if (1 == rdp_version){ // can't use rdp5
        use_rdp5 = 0;
        TODO(" why caring of server_depth here ? Quite strange")
        //        this->server_depth = 8;
    }
}

//01 0c 0c 00 -> TS_UD_HEADER::type = SC_CORE (0x0c01), length = 12
//bytes

//04 00 08 00 -> TS_UD_SC_CORE::version = 0x0008004
//00 00 00 00 -> TS_UD_SC_CORE::clientRequestedProtocols = PROTOCOL_RDP

static inline void out_mcs_data_sc_core(Stream & stream, const bool use_rdp5)
{
    LOG(LOG_INFO, "SC_CORE\n");
    // length, including tag and length fields
    stream.out_uint16_le(SC_CORE);
    stream.out_uint16_le(12); /* len */
    const uint32_t rdp_version = use_rdp5?0x0080004:0x0080001;
    LOG(LOG_DEBUG, "RDP proxy server supports version %s (was %s)\n",
            (rdp_version==0x0080001)?"RDP4":"RDP5");
    stream.out_uint32_le(rdp_version);
    const uint32_t clientRequestedProtocols = 0;
    stream.out_uint32_le(clientRequestedProtocols);
}

// 48-byte transformation used to generate master secret (6.1) and key material (6.2.2).
// Both SHA1 and MD5 algorithms are used.
static inline void sec_hash_48(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2, const uint8_t salt)
{
    uint8_t shasig[20];
    uint8_t pad[4];
    SSL_SHA1 sha1;
    SSL_MD5 md5;

    ssllib ssl;

    for (int i = 0; i < 3; i++) {
        memset(pad, salt + i, i + 1);

        ssl.sha1_init(&sha1);
        ssl.sha1_update(&sha1, pad, i + 1);
        ssl.sha1_update(&sha1, in, 48);
        ssl.sha1_update(&sha1, salt1, 32);
        ssl.sha1_update(&sha1, salt2, 32);
        ssl.sha1_final(&sha1, shasig);

        ssl.md5_init(&md5);
        ssl.md5_update(&md5, in, 48);
        ssl.md5_update(&md5, shasig, 20);
        ssl.md5_final(&md5, &out[i * 16]);
    }
}

// 16-byte transformation used to generate export keys (6.2.2).
static inline void sec_hash_16(uint8_t* out, const uint8_t* in, const uint8_t* salt1, const uint8_t* salt2)
{
    SSL_MD5 md5;

    ssllib ssl;

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, in, 16);
    ssl.md5_update(&md5, salt1, 32);
    ssl.md5_update(&md5, salt2, 32);
    ssl.md5_final(&md5, out);
}

inline static void rdp_sec_generate_keys(CryptContext & encrypt, CryptContext & decrypt, uint8_t (& sign_key)[16], uint8_t *client_random, uint8_t *server_random, uint32_t rc4_key_size)
{
    uint8_t pre_master_secret[48];
    uint8_t master_secret[48];
    uint8_t key_block[48];

    /* Construct pre-master secret (session key) */
    memcpy(pre_master_secret, client_random, 24);
    memcpy(pre_master_secret + 24, server_random, 24);

    /* Generate master secret and then key material */
    sec_hash_48(master_secret, pre_master_secret, client_random, server_random, 'A');
    sec_hash_48(key_block, master_secret, client_random, server_random, 'X');

    /* First 16 bytes of key material is MAC secret */
    memcpy(sign_key, key_block, 16);

    /* Generate export keys from next two blocks of 16 bytes */
    sec_hash_16(decrypt.key, &key_block[16], client_random, server_random);
    sec_hash_16(encrypt.key, &key_block[32], client_random, server_random);

    if (rc4_key_size == 1) {
        // LOG(LOG_DEBUG, "40-bit encryption enabled\n");
        sec_make_40bit(sign_key);
        sec_make_40bit(decrypt.key);
        sec_make_40bit(encrypt.key);
        decrypt.rc4_key_len = 8;
        encrypt.rc4_key_len = 8;
    }
    else {
        //LOG(LOG_DEBUG, "rc_4_key_size == %d, 128-bit encryption enabled\n", rc4_key_size);
        decrypt.rc4_key_len = 16;
        encrypt.rc4_key_len = 16;
    }

    /* Save initial RC4 keys as update keys */
    memcpy(decrypt.update_key, decrypt.key, 16);
    memcpy(encrypt.update_key, encrypt.key, 16);

    ssllib ssl;

    ssl.rc4_set_key(decrypt.rc4_info, decrypt.key, decrypt.rc4_key_len);
    ssl.rc4_set_key(encrypt.rc4_info, encrypt.key, encrypt.rc4_key_len);
}


static inline void recv_sec_tag_sig(Stream & stream, uint16_t len)
{
    stream.skip_uint8(len);
    /* Parse a public key structure */
    TODO("is padding always 8 bytes long ? may signature length change ? Check in documentation")
    TODO("we should check the signature is ok (using other provided parameters). This is not yet done today. Signature is just dropped")
}


static inline void recv_sec_tag_pubkey(Stream & stream, uint32_t & server_public_key_len, uint8_t* modulus, uint8_t* exponent)
{
    /* Parse a public key structure */
    uint32_t magic = stream.in_uint32_le();
    if (magic != SEC_RSA_MAGIC) {
        LOG(LOG_WARNING, "RSA magic 0x%x\n", magic);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK);
    }
    server_public_key_len = stream.in_uint32_le() - SEC_PADDING_SIZE;

    if ((server_public_key_len < SEC_MODULUS_SIZE)
    ||  (server_public_key_len > SEC_MAX_MODULUS_SIZE)) {
        LOG(LOG_WARNING, "Bad server public key size (%u bits)\n", server_public_key_len * 8);
        throw Error(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK);
    }
    stream.skip_uint8(8); /* modulus_bits, unknown */
    memcpy(exponent, stream.in_uint8p(SEC_EXPONENT_SIZE), SEC_EXPONENT_SIZE);
    memcpy(modulus, stream.in_uint8p(server_public_key_len), server_public_key_len);
    stream.skip_uint8(SEC_PADDING_SIZE);

    if (stream.p > stream.end){
        throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
    }
    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
}



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

// serverRandomLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of
// the serverRandom field. If the encryptionMethod and encryptionLevel fields
// are both set to 0 then the contents of this field MUST be ignored and the
// serverRandom field MUST NOT be present. Otherwise, this field MUST be set to
// 32 bytes.

// serverCertLen (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  serverCertificate field. If the encryptionMethod and encryptionLevel fields
//  are both set to 0 then the contents of this field MUST be ignored and the
// serverCertificate field MUST NOT be present.

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
// certChainVersion (31 bits): A 31-bit field. The certificate version.

// +---------------------------------+-----------------------------------------+
// | 0x00000001 CERT_CHAIN_VERSION_1 | The certificate contained in the        |
// |                                 | certData field is a Server Proprietary  |
// |                                 | Certificate (section 2.2.1.4.3.1.1).    |
// +---------------------------------+-----------------------------------------+
// | 0x00000002 CERT_CHAIN_VERSION_2 | The certificate contained in the        |
// |                                 | certData field is an X.509 Certificate  |
// |                                 | (see section 5.3.3.2).                  |
// +---------------------------------+-----------------------------------------+

// t (1 bit): A 1-bit field. Indicates whether the certificate contained in the
//  certData field has been permanently or temporarily issued to the server.

// 0 The certificate has been permanently issued to the server.
// 1 The certificate has been temporarily issued to the server.

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

static inline void parse_mcs_data_sc_security(Stream & cr_stream,
                                              CryptContext & encrypt,
                                              CryptContext & decrypt,
                                              uint32_t & server_public_key_len,
                                              uint8_t (& client_crypt_random)[512],
                                              int & crypt_level)
{
    LOG(LOG_INFO, "SC_SECURITY\n");

    uint8_t server_random[SEC_RANDOM_SIZE];
    uint8_t client_random[SEC_RANDOM_SIZE];
    uint8_t modulus[SEC_MAX_MODULUS_SIZE];
    uint8_t exponent[SEC_EXPONENT_SIZE];
    uint32_t rc4_key_size;

    ssllib ssl;

    memset(modulus, 0, sizeof(modulus));
    memset(exponent, 0, sizeof(exponent));
    memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));
    memset(server_random, 0, SEC_RANDOM_SIZE);

    uint16_t length;
    uint8_t* end;

    rc4_key_size = cr_stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
    LOG(LOG_INFO, "rc4_key_size = %u", rc4_key_size);
    crypt_level = cr_stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
    LOG(LOG_INFO, "crypt_level = %u", crypt_level);

    if (crypt_level == 0) { /* no encryption */
        LOG(LOG_INFO, "No encryption");
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_ENCRYPTION_REQUIRED);
    }
    uint32_t random_len = cr_stream.in_uint32_le();
    uint32_t rsa_info_len = cr_stream.in_uint32_le();

    LOG(LOG_INFO, "random_len = %u", random_len);
    LOG(LOG_INFO, "rsa_info_len = %u", rsa_info_len);

// serverRandom (variable): The variable-length server random value used to
// derive session keys (see sections 5.3.4 and 5.3.5). The length in bytes is
// given by the serverRandomLen field. If the encryptionMethod and
// encryptionLevel fields are both set to 0 then this field MUST NOT be present.

    if (random_len != SEC_RANDOM_SIZE) {
        LOG(LOG_ERR,
            "parse_crypt_info_error: random len %d, expected %d\n",
            random_len, SEC_RANDOM_SIZE);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN);
    }
    memcpy(server_random, cr_stream.in_uint8p(random_len), random_len);

// serverCertificate (variable): The variable-length certificate containing the
//  server's public key information. The length in bytes is given by the
// serverCertLen field. If the encryptionMethod and encryptionLevel fields are
// both set to 0 then this field MUST NOT be present.

    /* RSA info */
    end = cr_stream.p + rsa_info_len;
    if (end > cr_stream.end) {
        LOG(LOG_ERR,
            "rsa_info_len outside of buffer %u remains: %u", rsa_info_len, cr_stream.end - cr_stream.p);
        throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN);
    }

    uint32_t flags = cr_stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
    LOG(LOG_INFO, "crypt flags %x\n", flags);
    if (flags & 1) {

        LOG(LOG_DEBUG, "We're going for the RDP4-style encryption\n");
        cr_stream.skip_uint8(8); /* unknown */

        while (cr_stream.p < end) {
            uint16_t tag = cr_stream.in_uint16_le();
            length = cr_stream.in_uint16_le();
            TODO(" this should not be necessary any more as received tag are fully decoded (but we should check length does not lead accessing data out of buffer)")
            uint8_t * next_tag = cr_stream.p + length;

            switch (tag) {
            case SEC_TAG_PUBKEY:
                LOG(LOG_DEBUG, "ReceivingPublic key, RDP4-style\n");
                recv_sec_tag_pubkey(cr_stream, server_public_key_len, modulus, exponent);
                LOG(LOG_DEBUG, "Got Public key, RDP4-style");
            break;
            case SEC_TAG_KEYSIG:
                LOG(LOG_DEBUG, "Receiving key sig RDP4-style\n");
                recv_sec_tag_sig(cr_stream, length);
                LOG(LOG_DEBUG, "Got key sig RDP4-style\n");
                break;
            default:
                LOG(LOG_DEBUG, "unimplemented: crypt tag 0x%x\n", tag);
                throw Error(ERR_SEC_PARSE_CRYPT_INFO_UNIMPLEMENTED_TAG);
                break;
            }
            cr_stream.p = next_tag;
        }
    }
    else {
        LOG(LOG_DEBUG, "We're going for the RDP5-style encryption\n");
        uint32_t certcount = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certcount = %u\n", certcount);

        if (certcount < 2){
            LOG(LOG_DEBUG, "Server didn't send enough X509 certificates\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK);
        }
        for (; certcount > 2; certcount--){
            /* ignore all the certificates between the root and the signing CA */
            LOG(LOG_WARNING, " Ignored certs left: %d\n", certcount);
            uint32_t ignorelen = cr_stream.in_uint32_le();
            LOG(LOG_WARNING, "Ignored Certificate length is %d\n", ignorelen);
            SSL_CERT *ignorecert = ssl_cert_read(cr_stream.p, ignorelen);
            cr_stream.skip_uint8(ignorelen);
            if (ignorecert == NULL){
                LOG(LOG_WARNING,
                    "got a bad cert: this will probably screw up"
                    " the rest of the communication\n");
            }
            LOG(LOG_WARNING, "cert #%d (ignored)", certcount);
        }

        /* Do da funky X.509 stuffy

       "How did I find out about this?  I looked up and saw a
       bright light and when I came to I had a scar on my forehead
       and knew about X.500"
       - Peter Gutman in a early version of
       http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
       */

        /* Loading CA_Certificate from server*/
        uint32_t cacert_len = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "CA Certificate length is %d\n", cacert_len);
        SSL_CERT *cacert = ssl_cert_read(cr_stream.p, cacert_len);
        cr_stream.skip_uint8(cacert_len);
        if (NULL == cacert){
            LOG(LOG_DEBUG, "Couldn't load CA Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
        }

        /* Loading Certificate from server*/
        uint32_t cert_len = cr_stream.in_uint32_le();
        LOG(LOG_DEBUG, "Certificate length is %d\n", cert_len);
        SSL_CERT *server_cert = ssl_cert_read(cr_stream.p, cert_len);
        cr_stream.skip_uint8(cert_len);
        if (NULL == server_cert){
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Couldn't load Certificate from server\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED);
        }

        /* Matching certificates */
        if (!ssl_certs_ok(server_cert, cacert)){
            ssl_cert_free(server_cert);
            ssl_cert_free(cacert);
            LOG(LOG_DEBUG, "Security error CA Certificate invalid\n");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH);
        }
        ssl_cert_free(cacert);
        cr_stream.skip_uint8(16); /* Padding */
        SSL_RKEY *server_public_key = ssl_cert_to_rkey(server_cert, server_public_key_len);
        LOG(LOG_DEBUG, "Server public key length=%u\n", (unsigned)server_public_key_len);

        if (NULL == server_public_key){
            LOG(LOG_DEBUG, "Didn't parse X509 correctly\n");
            ssl_cert_free(server_cert);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED);

        }
        ssl_cert_free(server_cert);

        LOG(LOG_INFO, "server_public_key_len=%d, MODULUS_SIZE=%d MAX_MODULUS_SIZE=%d\n", server_public_key_len, SEC_MODULUS_SIZE, SEC_MAX_MODULUS_SIZE);
        if ((server_public_key_len < SEC_MODULUS_SIZE) ||
            (server_public_key_len > SEC_MAX_MODULUS_SIZE)){
            LOG(LOG_DEBUG, "Bad server public key size (%u bits)\n",
                server_public_key_len * 8);
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
        }
        if (ssl_rkey_get_exp_mod(server_public_key, exponent, SEC_EXPONENT_SIZE,
            modulus, SEC_MAX_MODULUS_SIZE) != 0){
            LOG(LOG_DEBUG, "Problem extracting RSA exponent, modulus");
            ssl.rkey_free(server_public_key);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK);
        }
        ssl.rkey_free(server_public_key);
        TODO(" find a way to correctly dispose of garbage at end of buffer")
        /* There's some garbage here we don't care about */
    }

    /* Generate a client random, and determine encryption keys */
    memset(client_random, 0x44, SEC_RANDOM_SIZE);
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fd = open("/dev/random", O_RDONLY);
    }
    if (fd != -1) {
        if (read(fd, client_random, SEC_RANDOM_SIZE) != SEC_RANDOM_SIZE) {
            LOG(LOG_WARNING, "random source failed to provide random data\n");
        }
        close(fd);
    }
    else {
        LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
    }

    memset(client_random, 0, SEC_RANDOM_SIZE);

    ssl.rsa_encrypt(client_crypt_random, client_random, SEC_RANDOM_SIZE, server_public_key_len, modulus, exponent);
    rdp_sec_generate_keys(encrypt, decrypt, encrypt.sign_key, client_random, server_random, rc4_key_size);
}

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

static inline void parse_mcs_data_sc_net(Stream & stream, const ChannelList & front_channel_list, ChannelList & mod_channel_list)
{
    LOG(LOG_INFO, "SC_NET\n");

    stream.in_uint16_le(); /* base_channel */
    size_t num_channels = stream.in_uint16_le();

    /* We assume that the channel_id array is confirmed in the same order
    that it has been sent. If there are any channels not confirmed, they're
    going to be the last channels on the array sent in MCS Connect Initial */
    for (size_t index = 0; index < num_channels; index++){
        mod_channel_list.push_back(front_channel_list[index]);
    }
}

static inline void send_sec_tag_sig(Stream & stream, const uint8_t (&pub_sig)[512])
{
    stream.out_uint16_le(SEC_TAG_KEYSIG);
    stream.out_uint16_le(72); /* len */
    stream.out_copy_bytes(pub_sig, 64); /* pub sig */
    stream.out_clear_bytes(8); /* pad */
}

static inline void send_sec_tag_pubkey(Stream & stream, const char (&pub_exp)[4], const uint8_t (&pub_mod)[512])
{
    stream.out_uint16_le(SEC_TAG_PUBKEY);
    stream.out_uint16_le(92); // length
    stream.out_uint32_le(SEC_RSA_MAGIC);
    stream.out_uint32_le(72); /* 72 bytes modulus len */
    stream.out_uint32_be(0x00020000);
    stream.out_uint32_be(0x3f000000);
    stream.out_copy_bytes(pub_exp, 4); /* pub exp */
    stream.out_copy_bytes(pub_mod, 64); /* pub mod */
    stream.out_clear_bytes(8); /* pad */
}





#endif
