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

/* used in sec */
struct mcs_channel_item {
    char name[16];
    int flags;
    int chanid;
    mcs_channel_item(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

struct Sec
{
    int userid;
    vector<struct mcs_channel_item *> channel_list;
    Stream client_mcs_data;

    #warning windows 2008 does not write trailer because of overflow of buffer below, checked actual size: 64 bytes on xp, 256 bytes on windows 2008
    uint8_t client_crypt_random[512];

    int decrypt_use_count;
    int encrypt_use_count;
    uint8_t decrypt_key[16];
    uint8_t encrypt_key[16];
    uint8_t decrypt_update_key[16];
    uint8_t encrypt_update_key[16];

    uint8_t crypt_level;
    int rc4_key_size; /* 1 = 40-bit, 2 = 128-bit */
    int rc4_key_len; /* 8 or 16 */
    uint8_t sign_key[16];

    char hostname[16];
    char username[128];

    Sec(uint8_t crypt_level) :
      decrypt_use_count(0),
      encrypt_use_count(0),
      crypt_level(crypt_level)
    {
        memset(this->decrypt_key, 0, 16);
        memset(this->encrypt_key, 0, 16);
        memset(this->decrypt_update_key, 0, 16);
        memset(this->encrypt_update_key, 0, 16);
        switch (crypt_level) {
        case 1:
        case 2:
            this->rc4_key_size = 1; /* 40 bits */
            this->rc4_key_len = 8; /* 8 = 40 bit */
        break;
        default:
        case 3:
            this->rc4_key_size = 2; /* 128 bits */
            this->rc4_key_len = 16; /* 16 = 128 bit */
        break;
        }

    }


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


    /* Output connect initial data blob */
    void rdp_sec_out_mcs_data(
            Stream & client_mcs_data,
            vector<mcs_channel_item*> channel_list,
            int width, int height, int rdp_bpp, int keylayout, bool console_session)
    {
        int length = 158 + 76 + 12 + 4;

        if (channel_list.size() > 0){
            length += channel_list.size() * 12 + 8;
        }

        /* Generic Conference Control (T.124) ConferenceCreateRequest */
        client_mcs_data.out_uint16_be(5);
        client_mcs_data.out_uint16_be(0x14);
        client_mcs_data.out_uint8(0x7c);
        client_mcs_data.out_uint16_be(1);

        client_mcs_data.out_uint16_be((length | 0x8000)); /* remaining length */

        client_mcs_data.out_uint16_be(8); /* length? */
        client_mcs_data.out_uint16_be(16);
        client_mcs_data.out_uint8(0);
        client_mcs_data.out_uint16_le(0xc001);
        client_mcs_data.out_uint8(0);

        client_mcs_data.out_uint32_le(0x61637544); /* OEM ID: "Duca", as in Ducati. */
        client_mcs_data.out_uint16_be(((length - 14) | 0x8000)); /* remaining length */

        /* Client information */
        client_mcs_data.out_uint16_le(CS_CORE);
        LOG(LOG_INFO, "Sending Client Core Data to remote server\n");
        client_mcs_data.out_uint16_le(212); /* length */
        LOG(LOG_INFO, "core::header::length = %u\n", 212);
        client_mcs_data.out_uint32_le(0x00080004); // RDP version. 1 == RDP4, 4 == RDP5.
        LOG(LOG_INFO, "core::header::version (0x00080004 = RDP 5.0, 5.1, 5.2, and 6.0 clients)");
        client_mcs_data.out_uint16_le(width);
        LOG(LOG_INFO, "core::desktopWidth = %u\n", width);
        client_mcs_data.out_uint16_le(height);
        LOG(LOG_INFO, "core::desktopHeight = %u\n", height);
        client_mcs_data.out_uint16_le(0xca01);
        LOG(LOG_INFO, "core::colorDepth = RNS_UD_COLOR_8BPP (superseded by postBeta2ColorDepth)");
        client_mcs_data.out_uint16_le(0xaa03);
        LOG(LOG_INFO, "core::SASSequence = RNS_UD_SAS_DEL");
        client_mcs_data.out_uint32_le(keylayout);
        LOG(LOG_INFO, "core::keyboardLayout = %x", keylayout);
        client_mcs_data.out_uint32_le(2600); /* Client build. We are now 2600 compatible :-) */
        LOG(LOG_INFO, "core::clientBuild = 2600");
        LOG(LOG_INFO, "core::clientName=%s\n", this->hostname);

        /* Added in order to limit hostlen and hostname size */
        int hostlen = 2 * strlen(this->hostname);
        if (hostlen > 30){
            hostlen = 30;
        }
        /* Unicode name of client, padded to 30 bytes */
        client_mcs_data.out_unistr(this->hostname);
        client_mcs_data.out_clear_bytes(30 - hostlen);

    /* See
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceddk40/html/cxtsksupportingremotedesktopprotocol.asp */
    #warning code should be updated to take care of keyboard type
        client_mcs_data.out_uint32_le(4); // g_keyboard_type
        LOG(LOG_INFO, "core::keyboardType = IBM enhanced (101- or 102-key) keyboard");
        client_mcs_data.out_uint32_le(0); // g_keyboard_subtype
        LOG(LOG_INFO, "core::keyboardSubType = 0");
        client_mcs_data.out_uint32_le(12); // g_keyboard_functionkeys
        LOG(LOG_INFO, "core::keyboardFunctionKey = 12 function keys");
        client_mcs_data.out_clear_bytes(64); /* imeFileName */
        LOG(LOG_INFO, "core::imeFileName = \"\"");
        client_mcs_data.out_uint16_le(0xca01); /* color depth 8bpp */
        LOG(LOG_INFO, "core::postBeta2ColorDepth = RNS_UD_COLOR_8BPP (superseded by highColorDepth)");
        client_mcs_data.out_uint16_le(1);
        LOG(LOG_INFO, "core::clientProductId = 1");
        client_mcs_data.out_uint32_le(0);
        LOG(LOG_INFO, "core::serialNumber = 0");
        client_mcs_data.out_uint16_le(rdp_bpp);
        LOG(LOG_INFO, "core::highColorDepth = %u", rdp_bpp);
        client_mcs_data.out_uint16_le(0x0007);
        LOG(LOG_INFO, "core::supportedColorDepths = 24/16/15");
        client_mcs_data.out_uint16_le(1);
        LOG(LOG_INFO, "core::earlyCapabilityFlags = RNS_UD_CS_SUPPORT_ERRINFO_PDU");
        client_mcs_data.out_clear_bytes(64);
        LOG(LOG_INFO, "core::clientDigProductId = \"\"");
        client_mcs_data.out_clear_bytes(2);
        LOG(LOG_INFO, "core::pad2octets");
//        client_mcs_data.out_uint32_le(0); // optional
//        LOG(LOG_INFO, "core::serverSelectedProtocol = 0");
        /* End of client info */

        client_mcs_data.out_uint16_le(CS_CLUSTER);
        client_mcs_data.out_uint16_le(12);
        #warning check that should depend on g_console_session
        client_mcs_data.out_uint32_le(console_session ? 0xb : 9);
        client_mcs_data.out_uint32_le(0);

        /* Client encryption settings */
        client_mcs_data.out_uint16_le(CS_SECURITY);
        client_mcs_data.out_uint16_le(12); /* length */
        #warning check that, should depend on g_encryption
        /* encryption supported, 128-bit supported */
        client_mcs_data.out_uint32_le(0x3);
        client_mcs_data.out_uint32_le(0); /* Unknown */

        /* Here we need to put channel information in order to redirect channel data
        from client to server passing through the "proxy" */
        size_t num_channels = channel_list.size();

        if (num_channels > 0) {
            client_mcs_data.out_uint16_le(CS_NET);
            client_mcs_data.out_uint16_le(num_channels * 12 + 8); /* length */
            client_mcs_data.out_uint32_le(num_channels); /* number of virtual channels */
            for (size_t i = 0; i < num_channels; i++){
                const mcs_channel_item* channel_item = channel_list[i];

                LOG(LOG_DEBUG, "Requesting channel %s\n", channel_item->name);
                memcpy(client_mcs_data.p, channel_item->name, 8);
                client_mcs_data.p += 8;

                client_mcs_data.out_uint32_be(channel_item->flags);
            }
        }
        client_mcs_data.mark_end();
    }
};

#endif
