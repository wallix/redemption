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

   MCS Connect Initial PDU with GCC Conference User Data


*/

#if !defined(__CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_CORE_HPP__)
#define __CORE_RDP_GCC_CONFERENCE_USER_DATA_CS_CORE_HPP__

#include "stream.hpp"
#include "client_info.hpp"

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


struct CSCoreGccUserData {
    uint32_t header;
    uint32_t version;
    uint16_t desktopWidth;
    uint16_t desktopHeight;
    uint16_t colorDepth;
    uint16_t SASSequence;
    uint32_t keyboardLayout;
    uint32_t clientBuild;
    uint16_t clientName[32];
    uint32_t keyboardType;
    uint32_t keyboardSubType;
    uint32_t keyboardFunctionKey;
    uint16_t imeFileName[32];
    uint16_t postBeta2ColorDepth;
    uint32_t serialNumber;
    uint16_t highColorDepth;
    uint16_t supportedColorDepths;
    uint16_t earlyCapabilityFlags;
    uint8_t  clientDigProductId[64];
    uint8_t  connectionType;
    uint8_t  pad1octet;
    uint32_t serverSelectedProtocol;
};


TODO(" use official field names from MS-RDPBCGR, see struct above")
static inline void parse_mcs_data_cs_core(Stream & stream, ClientInfo * client_info)
{
    LOG(LOG_INFO, "PARSE CS_CORE");
    uint16_t rdp_version = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: rdp_version (1=RDP1, 4=RDP5) %u", rdp_version);
    uint16_t dummy1 = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: ?? = %u", dummy1);
    client_info->width = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: width = %u", client_info->width);
    client_info->height = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: height = %u", client_info->height);
    uint16_t bpp_code = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: bpp_code = %x", bpp_code);
    uint16_t dummy2 = stream.in_uint16_le();
    LOG(LOG_INFO, "core_data: ?? = %x", dummy2);
    /* get keylayout */
    client_info->keylayout = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: layout = %x", client_info->keylayout);
    /* get build : windows build */
    client_info->build = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: build = %x", client_info->build);

    /* get hostname (it is UTF16, windows flavored widechars) */
    /* Unicode name of client is padded to 32 bytes */
    stream.in_uni_to_ascii_str(client_info->hostname, 32);
    LOG(LOG_INFO, "core_data: hostname = %s", client_info->hostname);

    uint32_t keyboard_type = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_type = %x", keyboard_type);
    uint32_t keyboard_subtype = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_subtype = %x", keyboard_subtype);
    uint32_t keyboard_functionkeys = stream.in_uint32_le();
    LOG(LOG_INFO, "core_data: keyboard_functionkeys = %x", keyboard_functionkeys);
    stream.in_skip_bytes(64);

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
    LOG(LOG_INFO, "core_data: bpp = %u", client_info->bpp);
}

static inline void mod_rdp_out_cs_core(Stream & stream, int use_rdp5, int width, int height, int rdp_bpp, int keylayout, char * hostname)
{
        stream.out_uint16_le(CS_CORE);
        LOG(LOG_INFO, "Sending Client Core Data to remote server");
        stream.out_uint16_le(212); /* length */
        LOG(LOG_INFO, "core::header::length = %u", 212);
        stream.out_uint32_le(use_rdp5?0x00080004:0x00080001); // RDP version. 1 == RDP4, 4 == RDP5.
        LOG(LOG_INFO, "core::header::version RDP 4=0x00080001 (0x00080004 = RDP 5.0, 5.1, 5.2, and 6.0 clients)");
        stream.out_uint16_le(width);
        LOG(LOG_INFO, "core::desktopWidth = %u", width);
        stream.out_uint16_le(height);
        LOG(LOG_INFO, "core::desktopHeight = %u", height);
        stream.out_uint16_le(0xca01);
        LOG(LOG_INFO, "core::colorDepth = RNS_UD_COLOR_8BPP (superseded by postBeta2ColorDepth)");
        stream.out_uint16_le(0xaa03);
        LOG(LOG_INFO, "core::SASSequence = RNS_UD_SAS_DEL");
        stream.out_uint32_le(keylayout);
        LOG(LOG_INFO, "core::keyboardLayout = %x", keylayout);
        stream.out_uint32_le(2600); /* Client build. We are now 2600 compatible :-) */
        LOG(LOG_INFO, "core::clientBuild = 2600");
        LOG(LOG_INFO, "core::clientName=%s", hostname);

        /* Added in order to limit hostlen and hostname size */
        int hostlen = 2 * strlen(hostname);
        if (hostlen > 30){
            hostlen = 30;
        }
        /* Unicode name of client, padded to 30 bytes */
        stream.out_unistr(hostname);
        stream.out_clear_bytes(30 - hostlen);

        /* See
        http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceddk40/html/cxtsksupportingremotedesktopprotocol.asp */
        TODO(" code should be updated to take care of keyboard type")
        stream.out_uint32_le(4); // g_keyboard_type
        LOG(LOG_INFO, "core::keyboardType = IBM enhanced (101- or 102-key) keyboard");
        stream.out_uint32_le(0); // g_keyboard_subtype
        LOG(LOG_INFO, "core::keyboardSubType = 0");
        stream.out_uint32_le(12); // g_keyboard_functionkeys
        LOG(LOG_INFO, "core::keyboardFunctionKey = 12 function keys");
        stream.out_clear_bytes(64); /* imeFileName */
        LOG(LOG_INFO, "core::imeFileName = \"\"");
        stream.out_uint16_le(0xca01); /* color depth 8bpp */
        LOG(LOG_INFO, "core::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (superseded by highColorDepth)");
        stream.out_uint16_le(1);
        LOG(LOG_INFO, "core::clientProductId = 1");
        stream.out_uint32_le(0);
        LOG(LOG_INFO, "core::serialNumber = 0");
        stream.out_uint16_le(rdp_bpp);
        LOG(LOG_INFO, "core::highColorDepth = %u", rdp_bpp);
        stream.out_uint16_le(0x0007);
        LOG(LOG_INFO, "core::supportedColorDepths = 24/16/15");
        stream.out_uint16_le(1);
        LOG(LOG_INFO, "core::earlyCapabilityFlags = RNS_UD_CS_SUPPORT_ERRINFO_PDU");
        stream.out_clear_bytes(64);
        LOG(LOG_INFO, "core::clientDigProductId = \"\"");
        stream.out_clear_bytes(2);
        LOG(LOG_INFO, "core::pad2octets");
    //        stream.out_uint32_le(0); // optional
    //        LOG(LOG_INFO, "core::serverSelectedProtocol = 0");
        /* End of client info */
}
#endif
