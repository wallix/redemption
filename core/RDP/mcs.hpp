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

#include "RDP/x224.hpp"
#include "channel_list.hpp"

#include "rsa_keys.hpp"
#warning ssl calls introduce some dependency on ssl system library, injecting it in the sec object would be better.
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
    public:
    uint8_t opcode;
    uint16_t user_id;
    uint16_t chan_id;
    uint8_t magic_0x70; // some ber header ?
    uint16_t len;

    McsIn(Stream & stream)
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
        #warning put some assertion here to ensure all data has been consumed
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

#warning use official field names from MS-RDPBCGR
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
    int i = stream.in_uint16_le();
    switch (i) {
    case 0xca01:
    {
        uint16_t clientProductId = stream.in_uint16_le();
        uint32_t serialNumber = stream.in_uint32_le();
        uint16_t rdp_bpp = stream.in_uint16_le();
        uint16_t supportedColorDepths = stream.in_uint16_le();
        if (rdp_bpp <= 24){
            client_info->bpp = rdp_bpp;
        }
        else {
            client_info->bpp = 24;
        }
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


static inline void send_cs_net(Stream & stream, const ChannelList & channel_list)
{
    /* Here we need to put channel information in order to redirect channel data
    from client to server passing through the "proxy" */
    size_t num_channels = channel_list.size();

    if (num_channels > 0) {
        LOG(LOG_INFO, "cs_net");
        stream.out_uint16_le(CS_NET);
        LOG(LOG_INFO, "cs_net::len=%u", num_channels * 12 + 8);
        stream.out_uint16_le(num_channels * 12 + 8); /* length */
        LOG(LOG_INFO, "cs_net::nb_chan=%u", num_channels);
        stream.out_uint32_le(num_channels); /* number of virtual channels */
        for (size_t index = 0; index < num_channels; index++){
            const McsChannelItem & channel_item = channel_list[index];
            stream.out_copy_bytes(channel_item.name, 8);
            stream.out_uint32_be(channel_item.flags);
        }
    }


}

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
    LOG(LOG_INFO, "front:basic_settings:channel_list:cs_net:channel_count %u [%u]", channelCount, channel_list.size());

    for (uint32_t index = 0; index < channelCount; index++) {
        McsChannelItem channel_item;
        memcpy(channel_item.name, stream.in_uint8p(8), 8);
        channel_item.flags = stream.in_uint32_be();
        channel_item.chanid = MCS_GLOBAL_CHANNEL + (index + 1);
        #warning push_back is not the best choice here, as we have static space already available in channel_list, we could even let ChannelList manage parsing
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
static inline void parse_mcs_data_sc_core(Stream & stream, int & use_rdp5)
{
    LOG(LOG_INFO, "SC_CORE\n");
    uint16_t rdp_version = stream.in_uint16_le();
    LOG(LOG_DEBUG, "Server RDP version is %d\n", rdp_version);
    if (1 == rdp_version){ // can't use rdp5
        use_rdp5 = 0;
        #warning why caring of server_depth here ? Quite strange
        //        this->server_depth = 8;
    }
}

// 2.2.1.4.3 Server Security Data (TS_UD_SC_SEC1)
static inline void parse_mcs_data_sc_security(Stream & stream)
{
    LOG(LOG_INFO, "SC_SECURITY\n");
}

// 2.2.1.4.4 Server Network Data (TS_UD_SC_NET)
/* this adds the mcs channels in the list of channels to be used when creating the server mcs data */
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



static inline void recv_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                Transport * trans,
                ClientInfo * client_info,
                ChannelList & channel_list)
{
    Stream stream(8192);
    X224In(trans, stream);

    if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = stream.in_ber_len();
    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);

    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);
    if (stream.in_uint8() != BER_TAG_BOOLEAN) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);

    if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();
    stream.skip_uint8(len);

    if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = stream.in_ber_len();

    stream.skip_uint8(23);

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
                #warning we should check length to call the two variants of core_data (or begin by reading the common part then the extended part)
                parse_mcs_data_cs_core(stream, client_info);
            break;
            case CS_SECURITY:
                parse_mcs_data_cs_security(stream);
            break;
            case CS_NET:
                LOG(LOG_INFO, "front:basic_settings:channel_list:cs_net %u", channel_list.size());
                parse_mcs_data_cs_net(stream, client_info, channel_list);
            break;
            case CS_CLUSTER:
                parse_mcs_data_cs_cluster(stream, client_info);
            break;
            case CS_MONITOR:
                parse_mcs_data_cs_monitor(stream);
            break;
            case SC_SECURITY:
                parse_mcs_data_sc_security(stream);
            break;
            default:
                LOG(LOG_INFO, "Unexpected data block tag %x\n", tag);
            break;
        }
        stream.p = current_header + length;
    }
}

static inline void send_mcs_connect_initial_pdu_with_gcc_conference_create_request(
                Transport * trans,
                const ChannelList & channel_list,
                int width,
                int height,
                int rdp_bpp,
                int keylayout,
                bool console_session,
                char * hostname){

    Stream data(8192);

    int length = 158 + 76 + 12 + 4;

    #warning another option could be to emit channel list even if number of channel is zero. It looks more logical to me than not passing any channel information (what happens in this case ?)
    if (channel_list.size() > 0){
        length += channel_list.size() * 12 + 8;
    }

    /* Generic Conference Control (T.124) ConferenceCreateRequest */
    data.out_uint16_be(5);
    data.out_uint16_be(0x14);
    data.out_uint8(0x7c);
    data.out_uint16_be(1);

    data.out_uint16_be((length | 0x8000)); /* remaining length */

    data.out_uint16_be(8); /* length? */
    data.out_uint16_be(16);
    data.out_uint8(0);
    data.out_uint16_le(0xc001);
    data.out_uint8(0);

    data.out_copy_bytes("Duca", 4); /* OEM ID: "Duca", as in Ducati. */
    data.out_uint16_be(((length - 14) | 0x8000)); /* remaining length */

    /* Client information */
    data.out_uint16_le(CS_CORE);
    LOG(LOG_INFO, "Sending Client Core Data to remote server\n");
    data.out_uint16_le(212); /* length */
    LOG(LOG_INFO, "core::header::length = %u\n", 212);
    data.out_uint32_le(0x00080004); // RDP version. 1 == RDP4, 4 == RDP5.
    LOG(LOG_INFO, "core::header::version (0x00080004 = RDP 5.0, 5.1, 5.2, and 6.0 clients)");
    data.out_uint16_le(width);
    LOG(LOG_INFO, "core::desktopWidth = %u\n", width);
    data.out_uint16_le(height);
    LOG(LOG_INFO, "core::desktopHeight = %u\n", height);
    data.out_uint16_le(0xca01);
    LOG(LOG_INFO, "core::colorDepth = RNS_UD_COLOR_8BPP (superseded by postBeta2ColorDepth)");
    data.out_uint16_le(0xaa03);
    LOG(LOG_INFO, "core::SASSequence = RNS_UD_SAS_DEL");
    data.out_uint32_le(keylayout);
    LOG(LOG_INFO, "core::keyboardLayout = %x", keylayout);
    data.out_uint32_le(2600); /* Client build. We are now 2600 compatible :-) */
    LOG(LOG_INFO, "core::clientBuild = 2600");
    LOG(LOG_INFO, "core::clientName=%s\n", hostname);

    /* Added in order to limit hostlen and hostname size */
    int hostlen = 2 * strlen(hostname);
    if (hostlen > 30){
        hostlen = 30;
    }
    /* Unicode name of client, padded to 30 bytes */
    data.out_unistr(hostname);
    data.out_clear_bytes(30 - hostlen);

    /* See
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceddk40/html/cxtsksupportingremotedesktopprotocol.asp */
    #warning code should be updated to take care of keyboard type
    data.out_uint32_le(4); // g_keyboard_type
    LOG(LOG_INFO, "core::keyboardType = IBM enhanced (101- or 102-key) keyboard");
    data.out_uint32_le(0); // g_keyboard_subtype
    LOG(LOG_INFO, "core::keyboardSubType = 0");
    data.out_uint32_le(12); // g_keyboard_functionkeys
    LOG(LOG_INFO, "core::keyboardFunctionKey = 12 function keys");
    data.out_clear_bytes(64); /* imeFileName */
    LOG(LOG_INFO, "core::imeFileName = \"\"");
    data.out_uint16_le(0xca01); /* color depth 8bpp */
    LOG(LOG_INFO, "core::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (superseded by highColorDepth)");
    data.out_uint16_le(1);
    LOG(LOG_INFO, "core::clientProductId = 1");
    data.out_uint32_le(0);
    LOG(LOG_INFO, "core::serialNumber = 0");
    data.out_uint16_le(rdp_bpp);
    LOG(LOG_INFO, "core::highColorDepth = %u", rdp_bpp);
    data.out_uint16_le(0x0007);
    LOG(LOG_INFO, "core::supportedColorDepths = 24/16/15");
    data.out_uint16_le(1);
    LOG(LOG_INFO, "core::earlyCapabilityFlags = RNS_UD_CS_SUPPORT_ERRINFO_PDU");
    data.out_clear_bytes(64);
    LOG(LOG_INFO, "core::clientDigProductId = \"\"");
    data.out_clear_bytes(2);
    LOG(LOG_INFO, "core::pad2octets");
//        data.out_uint32_le(0); // optional
//        LOG(LOG_INFO, "core::serverSelectedProtocol = 0");
    /* End of client info */

    data.out_uint16_le(CS_CLUSTER);
    data.out_uint16_le(12);
    #warning check that should depend on g_console_session
    data.out_uint32_le(console_session ? 0xb : 9);
    data.out_uint32_le(0);

    /* Client encryption settings */
    data.out_uint16_le(CS_SECURITY);
    data.out_uint16_le(12); /* length */

    #warning check that, should depend on g_encryption
    /* encryption supported, 128-bit supported */
    data.out_uint32_le(0x3);
    data.out_uint32_le(0); /* Unknown */

    send_cs_net(data, channel_list);

    data.mark_end();

    int data_len = data.end - data.data;
    int len = 7 + 3 * 34 + 4 + data_len;

    Stream ci_stream(8192);
    X224Out ci_tpdu(X224Packet::DT_TPDU, ci_stream);

    ci_stream.out_uint16_be(BER_TAG_MCS_CONNECT_INITIAL);
    ci_stream.out_ber_len(len);
    ci_stream.out_uint8(BER_TAG_OCTET_STRING);
    ci_stream.out_ber_len(0); /* calling domain */
    ci_stream.out_uint8(BER_TAG_OCTET_STRING);
    ci_stream.out_ber_len(0); /* called domain */
    ci_stream.out_uint8(BER_TAG_BOOLEAN);
    ci_stream.out_ber_len(1);
    ci_stream.out_uint8(0xff); /* upward flag */

    // target params
    ci_stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
    ci_stream.out_ber_len(32);
    ci_stream.out_ber_int16(34);     // max_channels
    ci_stream.out_ber_int16(2);      // max_users
    ci_stream.out_ber_int16(0);      // max_tokens
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0);
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0xffff); // max_pdu_size
    ci_stream.out_ber_int16(2);

    // min params
    ci_stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
    ci_stream.out_ber_len(32);
    ci_stream.out_ber_int16(1);     // max_channels
    ci_stream.out_ber_int16(1);     // max_users
    ci_stream.out_ber_int16(1);     // max_tokens
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0);
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0x420); // max_pdu_size
    ci_stream.out_ber_int16(2);

    // max params
    ci_stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
    ci_stream.out_ber_len(32);
    ci_stream.out_ber_int16(0xffff); // max_channels
    ci_stream.out_ber_int16(0xfc17); // max_users
    ci_stream.out_ber_int16(0xffff); // max_tokens
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0);
    ci_stream.out_ber_int16(1);
    ci_stream.out_ber_int16(0xffff); // max_pdu_size
    ci_stream.out_ber_int16(2);

    ci_stream.out_uint8(BER_TAG_OCTET_STRING);
    ci_stream.out_ber_len(data_len);
    ci_stream.out_copy_bytes(data.data, data_len);

    ci_tpdu.end();
    ci_tpdu.send(trans);
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



static inline void send_mcs_erect_domain_and_attach_user_request_pdu(Transport * trans)
{
    #warning there should be a way to merge both packets in the same stream to only perform one unique send
    Stream edrq_stream(8192);
    X224Out edrq_tpdu(X224Packet::DT_TPDU, edrq_stream);
    edrq_stream.out_uint8((MCS_EDRQ << 2));
    edrq_stream.out_uint16_be(0x100); /* height */
    edrq_stream.out_uint16_be(0x100); /* interval */
    edrq_tpdu.end();
    edrq_tpdu.send(trans);

    Stream aurq_stream(8192);
    X224Out aurq_tpdu(X224Packet::DT_TPDU, aurq_stream);
    aurq_stream.out_uint8((MCS_AURQ << 2));
    aurq_tpdu.end();
    aurq_tpdu.send(trans);
}

static inline void recv_mcs_erect_domain_and_attach_user_request_pdu(Transport * trans, uint16_t & userid)
{
    #warning this code could lead to some problem if both MCS are combined in the same TPDU, we should manage this case
    {
        Stream stream(8192);
        X224In in(trans, stream);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_EDRQ) {
            throw Error(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ);
        }
        stream.skip_uint8(2);
        stream.skip_uint8(2);
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }
        in.end();
    }

    {
        Stream stream(8192);
        X224In in(trans, stream);
        uint8_t opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_AURQ) {
            throw Error(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ);
        }
        if (opcode & 2) {
            userid = stream.in_uint16_be();
        }
        in.end();
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

static inline void send_mcs_channel_join_request_pdu(Transport * trans, int userid, int chanid)
{
    LOG(LOG_INFO, "send_mcs_channel_join_request_pdu userid=%u chanid=%u", userid, chanid);
    Stream cjrq_stream(8192);
    X224Out cjrq_tpdu(X224Packet::DT_TPDU, cjrq_stream);
    cjrq_stream.out_uint8((MCS_CJRQ << 2));
    cjrq_stream.out_uint16_be(userid);
    cjrq_stream.out_uint16_be(chanid);
    cjrq_tpdu.end();
    cjrq_tpdu.send(trans);
}

static inline void recv_mcs_channel_join_request_pdu(Transport * trans, uint16_t & userid, uint16_t & chanid){
    Stream stream(8192);
    // read tpktHeader (4 bytes = 3 0 len)
    // TPDU class 0    (3 bytes = LI F0 PDU_DT)
    X224In in(trans, stream);

    uint8_t opcode = stream.in_uint8();
    if ((opcode >> 2) != MCS_CJRQ) {
        LOG(LOG_INFO, "unexpected opcode = %u", opcode);
        throw Error(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ);
    }
    userid = stream.in_uint16_be();
    chanid = stream.in_uint16_be();

    if (opcode & 2) {
        stream.skip_uint8(2);
    }

    LOG(LOG_INFO, "recv cjrq done");
    LOG(LOG_INFO, "recv_mcs_channel_join_request_pdu(userid=%u, chanid=%u)", userid, chanid);

    in.end();
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

static inline void recv_mcs_channel_join_confirm_pdu(Transport * trans, uint16_t & mcs_userid, uint16_t & req_chanid, uint16_t & join_chanid)
{
    LOG(LOG_INFO, "recv_mcs_channel_join_confirm_pdu");
    Stream cjcf_stream(8192);
    X224In cjcf_tpdu(trans, cjcf_stream);
    int opcode = cjcf_stream.in_uint8();
    if ((opcode >> 2) != MCS_CJCF) {
        throw Error(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF);
    }
    uint8_t result = cjcf_stream.in_uint8();
    if (0 != result) {
        LOG(LOG_INFO, "recv_mcs_channel_join_confirm_pdu exception, expected 0, got %u", result);
        throw Error(ERR_MCS_RECV_CJCF_EMPTY);
    }
    mcs_userid = cjcf_stream.in_uint16_be();
    req_chanid = join_chanid = cjcf_stream.in_uint16_be();
    if (opcode & 2) {
        join_chanid = cjcf_stream.in_uint16_be();
    }
    LOG(LOG_INFO, "recv_mcs_channel_join_confirm_pdu opcode=%u userid=%u req_chanid=%u join_chanid=%u", opcode, mcs_userid, req_chanid, join_chanid);
    cjcf_tpdu.end();
}

static inline void send_mcs_channel_join_confirm_pdu(Transport * trans, uint16_t userid, uint16_t chanid)
{
    LOG(LOG_INFO, "send_mcs_channel_join_confirm_pdu(userid=%u, chanid=%u)", userid, chanid);
    Stream stream(8192);
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    stream.out_uint8((MCS_CJCF << 2) | 2);
    stream.out_uint8(0);
    stream.out_uint16_be(userid);
    stream.out_uint16_be(chanid);
    #warning this should be sent only if different from requested chan_id
    stream.out_uint16_be(chanid);
    tpdu.end();
    tpdu.send(trans);
}

static inline void send_mcs_channel_join_request_and_recv_confirm_pdu(Transport * trans,
                    uint16_t userid, ChannelList & channel_list)
{
    #warning the array size below is arbitrary, it should be checked to avoid buffer overflow

    LOG(LOG_INFO, "send_cjrq_recv_cf userid=%u", userid);

    size_t num_channels = channel_list.size();
    uint16_t channels_id[100];
    channels_id[0] = userid + MCS_USERCHANNEL_BASE;
    channels_id[1] = MCS_GLOBAL_CHANNEL;
    for (size_t index = 0; index < num_channels; index++){
        channels_id[index+2] = channel_list[index].chanid;
    }

    LOG(LOG_INFO, "num_channels=%u", num_channels);
    for (size_t index = 0; index < num_channels+2; index++){
        send_mcs_channel_join_request_pdu(trans, userid, channels_id[index]);
        {
            uint16_t tmp_userid;
            uint16_t tmp_req_chanid;
            uint16_t tmp_join_chanid;
            recv_mcs_channel_join_confirm_pdu(trans, tmp_userid, tmp_req_chanid, tmp_join_chanid);
        }
        LOG(LOG_INFO, "----------------------- %u ---------------------------", index);
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

static inline void recv_mcs_attach_user_confirm_pdu(Transport * trans, int & userid)
{
    Stream aucf_stream(8192);
    X224In aucf_tpdu(trans, aucf_stream);
    int opcode = aucf_stream.in_uint8();
    if ((opcode >> 2) != MCS_AUCF) {
        throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
    }
    int res = aucf_stream.in_uint8();
    if (res != 0) {
        throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
    }
    LOG(LOG_INFO, "aucf opcode=%u, userid=%u", opcode, userid);
    if (opcode & 2) {
        userid = aucf_stream.in_uint16_be();
    }
    LOG(LOG_INFO, "aucf2 opcode=%u, userid=%u", opcode, userid);
    aucf_tpdu.end();
}

static inline void send_mcs_attach_user_confirm_pdu(Transport * trans, uint16_t userid)
{
    Stream stream(8192);
    X224Out tpdu(X224Packet::DT_TPDU, stream);
    stream.out_uint8(((MCS_AUCF << 2) | 2));
    stream.out_uint8(0);
    stream.out_uint16_be(userid);
    tpdu.end();
    tpdu.send(trans);
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


static inline int recv_sec_tag_sig(Stream & stream, uint16_t len)
{
    stream.skip_uint8(len);
    /* Parse a public key structure */
    #warning is padding always 8 bytes long, may signature length change ? Check in documentation
    #warning we should check the signature is ok (using other provided parameters), this is not yet done today, signature is just dropped
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

    if (!stream.check()){
        throw Error(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM);
    }
    LOG(LOG_DEBUG, "Got Public key, RDP4-style\n");
}



static inline void send_mcs_connect_response_pdu_with_gcc_conference_create_response(
                                        Transport * trans,
                                        ClientInfo * client_info,
                                        const ChannelList & channel_list,
                                        uint8_t (&server_random)[32],
                                        int rc4_key_size,
                                        uint8_t (&pub_mod)[512],
                                        uint8_t (&pri_exp)[512]
                                    ) throw(Error)
{
    Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);
    memset(server_random, 0x44, 32);
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fd = open("/dev/random", O_RDONLY);
    }
    if (fd != -1) {
        if (read(fd, server_random, 32) != 32) {
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
    stream.out_copy_bytes(server_random, 32);
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



static inline void recv_mcs_connect_response_pdu_with_gcc_conference_create_response(
                            Transport * trans,
                            ChannelList & mod_channel_list,
                            const ChannelList & front_channel_list,
                            CryptContext & encrypt, CryptContext & decrypt,
                            uint32_t & server_public_key_len,
                            uint8_t (& client_crypt_random)[512],
                            int crypt_level,
                            int & use_rdp5)
{
    Stream cr_stream(8192);
    X224In(trans, cr_stream);
    if (cr_stream.in_uint16_be() != BER_TAG_MCS_CONNECT_RESPONSE) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    int len = cr_stream.in_ber_len();

    if (cr_stream.in_uint8() != BER_TAG_RESULT) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();

    int res = cr_stream.in_uint8();

    if (res != 0) {
        throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
    }
    if (cr_stream.in_uint8() != BER_TAG_INTEGER) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.skip_uint8(len); /* connect id */

    if (cr_stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();
    cr_stream.skip_uint8(len);

    if (cr_stream.in_uint8() != BER_TAG_OCTET_STRING) {
        throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
    }
    len = cr_stream.in_ber_len();

    cr_stream.skip_uint8(21); /* header (T.124 ConferenceCreateResponse) */
    len = cr_stream.in_uint8();

    if (len & 0x80) {
        len = cr_stream.in_uint8();
    }
    while (cr_stream.p < cr_stream.end) {
        uint16_t tag = cr_stream.in_uint16_le();
        uint16_t length = cr_stream.in_uint16_le();
        if (length <= 4) {
            throw Error(ERR_MCS_DATA_SHORT_HEADER);
        }
        uint8_t *next_tag = (cr_stream.p + length) - 4;
        switch (tag) {
        case SC_CORE:
            parse_mcs_data_sc_core(cr_stream, use_rdp5);
        break;
        case SC_SECURITY:
        {
        uint8_t server_random[SEC_RANDOM_SIZE];
        uint8_t client_random[SEC_RANDOM_SIZE];
        uint8_t modulus[SEC_MAX_MODULUS_SIZE];
        uint8_t exponent[SEC_EXPONENT_SIZE];
        uint32_t rc4_key_size;

        memset(modulus, 0, sizeof(modulus));
        memset(exponent, 0, sizeof(exponent));
        memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));
        #warning check for the true size
        memset(server_random, 0, SEC_RANDOM_SIZE);

        uint32_t random_len;
        uint32_t rsa_info_len;
        uint32_t cacert_len;
        uint32_t cert_len;
        uint32_t flags;
        SSL_CERT *cacert;
        SSL_CERT *server_cert;
        SSL_RKEY *server_public_key;
        uint16_t tag;
        uint16_t length;
        uint8_t* next_tag;
        uint8_t* end;

        rc4_key_size = cr_stream.in_uint32_le(); /* 1 = 40-bit, 2 = 128-bit */
        crypt_level = cr_stream.in_uint32_le(); /* 1 = low, 2 = medium, 3 = high */
        if (crypt_level == 0) { /* no encryption */
            LOG(LOG_INFO, "No encryption");
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_ENCRYPTION_REQUIRED);
        }
        random_len = cr_stream.in_uint32_le();
        rsa_info_len = cr_stream.in_uint32_le();
        if (random_len != SEC_RANDOM_SIZE) {
            LOG(LOG_ERR,
                "parse_crypt_info_error: random len %d, expected %d\n",
                random_len, SEC_RANDOM_SIZE);
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN);
        }
        memcpy(server_random, cr_stream.in_uint8p(random_len), random_len);

        /* RSA info */
        end = cr_stream.p + rsa_info_len;
        if (end > cr_stream.end) {
            throw Error(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN);
        }

        flags = cr_stream.in_uint32_le(); /* 1 = RDP4-style, 0x80000002 = X.509 */
        LOG(LOG_INFO, "crypt flags %x\n", flags);
        if (flags & 1) {

            LOG(LOG_DEBUG, "We're going for the RDP4-style encryption\n");
            cr_stream.skip_uint8(8); /* unknown */

            while (cr_stream.p < end) {
                tag = cr_stream.in_uint16_le();
                length = cr_stream.in_uint16_le();
                #warning this should not be necessary any more as received tag are fully decoded (but we should check length does not lead accessing data out of buffer)
                next_tag = cr_stream.p + length;

                switch (tag) {
                case SEC_TAG_PUBKEY:
                    recv_sec_tag_pubkey(cr_stream, server_public_key_len, modulus, exponent);
                break;
                case SEC_TAG_KEYSIG:
                    LOG(LOG_DEBUG, "SEC_TAG_KEYSIG RDP4-style\n");
                    recv_sec_tag_sig(cr_stream, length);
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
            LOG(LOG_DEBUG, "RDP5-style encryption with certificates not available\n");
            uint32_t certcount = cr_stream.in_uint32_le();
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
            }

            /* Do da funky X.509 stuffy

           "How did I find out about this?  I looked up and saw a
           bright light and when I came to I had a scar on my forehead
           and knew about X.500"
           - Peter Gutman in a early version of
           http://www.cs.auckland.ac.nz/~pgut001/pubs/x509guide.txt
           */

            /* Loading CA_Certificate from server*/
            cacert_len = cr_stream.in_uint32_le();
            LOG(LOG_DEBUG, "CA Certificate length is %d\n", cacert_len);
            cacert = ssl_cert_read(cr_stream.p, cacert_len);
            cr_stream.skip_uint8(cacert_len);
            if (NULL == cacert){
                LOG(LOG_DEBUG, "Couldn't load CA Certificate from server\n");
                throw Error(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL);
            }

            ssllib ssl;

            /* Loading Certificate from server*/
            cert_len = cr_stream.in_uint32_le();
            LOG(LOG_DEBUG, "Certificate length is %d\n", cert_len);
            server_cert = ssl_cert_read(cr_stream.p, cert_len);
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
            server_public_key = ssl_cert_to_rkey(server_cert, server_public_key_len);
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
            #warning find a way to correctly dispose of garbage at end of buffer
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
        ssllib ssl;
        ssl.rsa_encrypt(client_crypt_random, client_random, SEC_RANDOM_SIZE, server_public_key_len, modulus, exponent);

        rdp_sec_generate_keys(encrypt, decrypt, encrypt.sign_key, client_random, server_random, rc4_key_size);
        }
        break;
        case SC_NET:
            parse_mcs_data_sc_net(cr_stream, front_channel_list, mod_channel_list);
            break;
        default:
            LOG(LOG_WARNING, "response tag 0x%x\n", tag);
            break;
        }
        cr_stream.p = next_tag;
    }
}

#endif
