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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {


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
//         0x00080004 RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1 and 8.0 clients

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

// If the Layout Manager entry points for LayoutMgrGetKeyboardType and
// LayoutMgrGetKeyboardLayoutName do not exist, the values in certain registry
// keys are queried and their values are returned instead. The following
// registry key example shows the registry keys to configure to support RDP.

// [HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\KEYBD]
//    "Keyboard Type"=dword:<type>
//    "Keyboard SubType"=dword:<subtype>
//    "Keyboard Function Keys"=dword:<function keys>
//    "Keyboard Layout"="<layout>"

// To set these values for the desired locale, set the variable DEFINE_KEYBOARD_TYPE
// in Platform.reg before including Keybd.reg. The following code sample shows
// how to set the DEFINE_KEYBOARD_TYPE in Platform.reg before including Keybd.reg.

//    #define DEFINE_KEYBOARD_TYPE
//    #include "$(DRIVERS_DIR)\keybd\keybd.reg"
//    This will bring in the proper values for the current LOCALE, if it is
//    supported. Logic in Keybd.reg sets these values. The following registry
//    example shows this logic.
//    ; Define this variable in platform.reg if your keyboard driver does not
//    ; report its type information.
//    #if defined DEFINE_KEYBOARD_TYPE

//    #if $(LOCALE)==0411

//    ; Japanese keyboard layout
//        "Keyboard Type"=dword:7
//        "Keyboard SubType"=dword:2
//        "Keyboard Function Keys"=dword:c
//        "Keyboard Layout"="00000411"

//    #elif $(LOCALE)==0412

//    ; Korean keyboard layout
//        "Keyboard Type"=dword:8
//        "Keyboard SubType"=dword:3
//        "Keyboard Function Keys"=dword:c
//        "Keyboard Layout"="00000412"

//    #else

//    ; Default to US keyboard layout
//        "Keyboard Type"=dword:4
//        "Keyboard SubType"=dword:0
//        "Keyboard Function Keys"=dword:c
//        "Keyboard Layout"="00000409"

//    #endif

//    #endif ; DEFINE_KEYBOARD_TYPE


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

enum {
      HIGH_COLOR_4BPP  = 0x0004
    , HIGH_COLOR_8BPP  = 0x0008
    , HIGH_COLOR_15BPP = 0x000F
    , HIGH_COLOR_16BPP = 0x0010
    , HIGH_COLOR_24BPP = 0x0018
};

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

enum {
      RNS_UD_CS_SUPPORT_ERRINFO_PDU        = 0x0001
    , RNS_UD_CS_WANT_32BPP_SESSION         = 0x0002
    , RNS_UD_CS_SUPPORT_STATUSINFO_PDU     = 0x0004
    , RNS_UD_CS_STRONG_ASYMMETRIC_KEYS     = 0x0008
    , RNS_UD_CS_UNUSED                     = 0x0010
    , RNS_UD_CS_VALID_CONNECTION_TYPE      = 0x0020
    , RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU = 0x0040
    , RNS_UD_CS_SUPPORT_NETCHAR_AUTODETECT = 0x0080
    , RNS_UD_CS_SUPPORT_DYNVC_GFX_PROTOCOL = 0x0100
    , RNS_UD_CS_SUPPORT_DYNAMIC_TIME_ZONE  = 0x0200
    , RNS_UD_CS_SUPPORT_HEARTBEAT_PDU      = 0x0400
};

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


struct CSCore {
    // header
    uint16_t userDataType{CS_CORE};
    uint16_t length{216};            // default: everything including serverSelectedProtocol
    uint32_t version{0x00080001};    // RDP version. 1 == RDP4, 4 == RDP5
    uint16_t desktopWidth;
    uint16_t desktopHeight;
    uint16_t colorDepth{0xca01};
    uint16_t SASSequence{0xaa03};
    uint32_t keyboardLayout{0x040c}; // default to French
    uint32_t clientBuild{2600};
    uint16_t clientName[16];
    uint32_t keyboardType{4};
    uint32_t keyboardSubType{0};
    uint32_t keyboardFunctionKey{12};
    uint16_t imeFileName[32];
    // optional payload
    uint16_t postBeta2ColorDepth{0xca01};
    uint16_t clientProductId{1};
    uint32_t serialNumber{0};
    uint16_t highColorDepth{0};
    uint16_t supportedColorDepths{7};
    uint16_t earlyCapabilityFlags{1};
    uint8_t  clientDigProductId[64];
    uint8_t  connectionType{0};
    uint8_t  pad1octet{0};
    uint32_t serverSelectedProtocol{0};

    // we do not provide parameters in constructor,
    // because setting them one field at a time is more explicit and maintainable
    // (drawback: danger is different, not swapping parameters, but we may forget to define some...)
    CSCore()
    {
        memset(this->clientName, 0, 32);
        memset(this->imeFileName, 0, 64);
        memset(this->clientDigProductId, 0, 64);
    }

    void recv(InStream & stream)
    {
        if (!stream.in_check_rem(36)){
            LOG(LOG_ERR, "CSCore::recv short header");
            throw Error(ERR_GCC);
        }

        this->userDataType = stream.in_uint16_le();
        this->length = stream.in_uint16_le();

        if (!stream.in_check_rem(this->length - 4)){
            LOG(LOG_ERR, "CSCore::recv short length=%d", this->length);
            throw Error(ERR_GCC);
        }

        this->version = stream.in_uint32_le();
        this->desktopWidth = stream.in_uint16_le();
        this->desktopHeight = stream.in_uint16_le();
        this->colorDepth = stream.in_uint16_le();
        this->SASSequence = stream.in_uint16_le();
        this->keyboardLayout = stream.in_uint32_le();
        this->clientBuild = stream.in_uint32_le();
        // utf16 hostname fixed length,
        // including mandatory terminal 0
        // length is a number of utf16 characters
        stream.in_utf16(this->clientName, 16);
        this->keyboardType = stream.in_uint32_le();
        this->keyboardSubType = stream.in_uint32_le();
        this->keyboardFunctionKey = stream.in_uint32_le();
        // utf16 fixed length,
        // including mandatory terminal 0
        // length is a number of utf16 characters
        stream.in_utf16(this->imeFileName, 32);
        // --------------------- Optional Fields ---------------------------------------
        if (this->length < 134) { return; }
        this->postBeta2ColorDepth = stream.in_uint16_le();
        if (this->length < 136) { return; }
        this->clientProductId = stream.in_uint16_le();
        if (this->length < 140) { return; }
        this->serialNumber = stream.in_uint32_le();
        if (this->length < 142) { return; }
        this->highColorDepth = stream.in_uint16_le();
        if (this->length < 144) { return; }
        this->supportedColorDepths = stream.in_uint16_le();
        if (this->length < 146) { return; }
        this->earlyCapabilityFlags = stream.in_uint16_le();
        if (this->length < 210) { return; }
        stream.in_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
        if (this->length < 211) { return; }
        this->connectionType = stream.in_uint8();
        if (this->length < 212) { return; }
        this->pad1octet = stream.in_uint8();
        if (this->length < 216) { return; }
        this->serverSelectedProtocol = stream.in_uint32_le();
        // TODO Missing desktopPhysicalWith, desktopPhysicalHeight, desktopOrientation, desktopScaleFactor, deviceScaleFactor, see [MS-RDPBCGR] 2.2.1.3.2

    }

    void emit(OutStream & stream) const
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->version);
        stream.out_uint16_le(this->desktopWidth);
        stream.out_uint16_le(this->desktopHeight);
        stream.out_uint16_le(this->colorDepth);
        stream.out_uint16_le(this->SASSequence);
        stream.out_uint32_le(this->keyboardLayout);
        stream.out_uint32_le(this->clientBuild);
        // utf16 hostname fixed length,
        // including mandatory terminal 0
        // length is a number of utf16 characters
        stream.out_utf16(this->clientName, 16);
        stream.out_uint32_le(this->keyboardType);
        stream.out_uint32_le(this->keyboardSubType);
        stream.out_uint32_le(this->keyboardFunctionKey);
        // utf16 fixed length,
        // including mandatory terminal 0
        // length is a number of utf16 characters
        stream.out_utf16(this->imeFileName, 32);

        // --------------------- Optional Fields ---------------------------------------
        this->emit_optional(stream);
    }

    private:
    void emit_optional(OutStream & stream) const
    {
        if (this->length < 134) { return; }
        stream.out_uint16_le(this->postBeta2ColorDepth);
        if (this->length < 136) { return; }
        stream.out_uint16_le(this->clientProductId);
        if (this->length < 140) { return; }
        stream.out_uint32_le(this->serialNumber);
        if (this->length < 142) { return; }
        stream.out_uint16_le(this->highColorDepth);
        if (this->length < 144) { return; }
        stream.out_uint16_le(this->supportedColorDepths);
        if (this->length < 146) { return; }
        stream.out_uint16_le(this->earlyCapabilityFlags);
        if (this->length < 210) { return; }
        stream.out_copy_bytes(this->clientDigProductId, sizeof(this->clientDigProductId));
        if (this->length < 211) { return; }
        stream.out_uint8(this->connectionType);
        if (this->length < 212) { return; }
        stream.out_uint8(this->pad1octet);
        if (this->length < 216) { return; }
        stream.out_uint32_le(this->serverSelectedProtocol);
    }

    public:
    void log(const char * msg) const
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data CS_CORE (%u bytes)", msg, this->length);

        if (this->length < 132){
            LOG(LOG_INFO, "GCC User Data CS_CORE truncated");
            return;
        }

        LOG(LOG_INFO, "cs_core::version [%04x] %s", this->version,
              (this->version == 0x00080001) ? "RDP 4 client"
             :(this->version == 0x00080004) ? "RDP 5.0, 5.1, 5.2, and 6.0 clients)"
                                          : "Unknown client");
        LOG(LOG_INFO, "cs_core::desktopWidth  = %u",  this->desktopWidth);
        LOG(LOG_INFO, "cs_core::desktopHeight = %u", this->desktopHeight);
        LOG(LOG_INFO, "cs_core::colorDepth    = [%04x] [%s] superseded by postBeta2ColorDepth", this->colorDepth,
            (this->colorDepth == 0xCA00) ? "RNS_UD_COLOR_4BPP"
          : (this->colorDepth == 0xCA01) ? "RNS_UD_COLOR_8BPP"
                                         : "Unknown");
        LOG(LOG_INFO, "cs_core::SASSequence   = [%04x] [%s]", this->SASSequence,
            (this->SASSequence == 0xCA00) ? "RNS_UD_SAS_DEL"
                                          : "Unknown");
        LOG(LOG_INFO, "cs_core::keyboardLayout= %04x",  this->keyboardLayout);
        LOG(LOG_INFO, "cs_core::clientBuild   = %u",  this->clientBuild);
        char hostname[16];
        for (size_t i = 0; i < 16 ; i++) {
            hostname[i] = static_cast<char>(this->clientName[i]);
        }
        LOG(LOG_INFO, "cs_core::clientName    = %s",  hostname);
        LOG(LOG_INFO, "cs_core::keyboardType  = [%04x] %s",  this->keyboardType,
              (this->keyboardType == 0x00000001) ? "IBM PC/XT or compatible (83-key) keyboard"
            : (this->keyboardType == 0x00000002) ? "Olivetti \"ICO\" (102-key) keyboard"
            : (this->keyboardType == 0x00000003) ? "IBM PC/AT (84-key) and similar keyboards"
            : (this->keyboardType == 0x00000004) ? "IBM enhanced (101-key or 102-key) keyboard"
            : (this->keyboardType == 0x00000005) ? "Nokia 1050 and similar keyboards"
            : (this->keyboardType == 0x00000006) ? "Nokia 9140 and similar keyboards"
            : (this->keyboardType == 0x00000007) ? "Japanese keyboard"
                                                 : "Unknown");
        LOG(LOG_INFO, "cs_core::keyboardSubType      = [%04x] OEM code",  this->keyboardSubType);
        LOG(LOG_INFO, "cs_core::keyboardFunctionKey  = %u function keys",  this->keyboardFunctionKey);
        char imename[32];
        for (size_t i = 0; i < 32 ; i++){
            imename[i] = static_cast<char>(this->imeFileName[i]);
        }
        LOG(LOG_INFO, "cs_core::imeFileName    = %s",  imename);

        // --------------------- Optional Fields ---------------------------------------
        if (this->length < 134) { return; }
        LOG(LOG_INFO, "cs_core::postBeta2ColorDepth  = [%04x] [%s]", this->postBeta2ColorDepth,
            (this->postBeta2ColorDepth == 0xCA00) ? "4 bpp"
          : (this->postBeta2ColorDepth == 0xCA01) ? "8 bpp"
          : (this->postBeta2ColorDepth == 0xCA02) ? "15-bit 555 RGB mask"
          : (this->postBeta2ColorDepth == 0xCA03) ? "16-bit 565 RGB mask"
          : (this->postBeta2ColorDepth == 0xCA04) ? "24-bit RGB mask"
                                                  : "Unknown");
        if (this->length < 136) { return; }
        LOG(LOG_INFO, "cs_core::clientProductId = %u", this->clientProductId);
        if (this->length < 140) { return; }
        LOG(LOG_INFO, "cs_core::serialNumber = %u", this->serialNumber);
        if (this->length < 142) { return; }
        LOG(LOG_INFO, "cs_core::highColorDepth  = [%04x] [%s]", this->highColorDepth,
            (this->highColorDepth == 4)  ? "4 bpp"
          : (this->highColorDepth == 8)  ? "8 bpp"
          : (this->highColorDepth == 15) ? "15-bit 555 RGB mask"
          : (this->highColorDepth == 16) ? "16-bit 565 RGB mask"
          : (this->highColorDepth == 24) ? "24-bit RGB mask"
                                         : "Unknown");
        if (this->length < 144) { return; }
        LOG(LOG_INFO, "cs_core::supportedColorDepths  = [%04x] [%s/%s/%s/%s]", this->supportedColorDepths,
            (this->supportedColorDepths & 1) ? "24":"",
            (this->supportedColorDepths & 2) ? "16":"",
            (this->supportedColorDepths & 4) ? "15":"",
            (this->supportedColorDepths & 8) ? "32":"");
        if (this->length < 146) { return; }
        LOG(LOG_INFO, "cs_core::earlyCapabilityFlags  = [%04x]", this->earlyCapabilityFlags);
        if (this->earlyCapabilityFlags & 0x0001){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_ERRINFO_PDU");
        }
        if (this->earlyCapabilityFlags & 0x0002){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_WANT_32BPP_SESSION");
        }
        if (this->earlyCapabilityFlags & 0x0004){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_STATUSINFO_PDU");
        }
        if (this->earlyCapabilityFlags & 0x0008){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_STRONG_ASYMMETRIC_KEYS");
        }
        if (this->earlyCapabilityFlags & 0x00020){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_VALID_CONNECTION_TYPE");
        }
        if (this->earlyCapabilityFlags & 0x00040){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:RNS_UD_CS_SUPPORT_MONITOR_LAYOUT_PDU");
        }
        if (this->earlyCapabilityFlags & 0xFF10){
            LOG(LOG_INFO, "cs_core::earlyCapabilityFlags:Unknown early capability flag");
        }
        if (this->length < 210) { return; }
        LOG(LOG_INFO, "cs_core::clientDigProductId=[%s]", log_array_02x_format(this->clientDigProductId));
        if (this->length < 211) { return; }
        LOG(LOG_INFO, "cs_core::connectionType = %u", this->connectionType);
        if (this->length < 212) { return; }
        LOG(LOG_INFO, "cs_core::pad1octet = %u", this->pad1octet);
        if (this->length < 216) { return; }
        LOG(LOG_INFO, "cs_core::serverSelectedProtocol = %u", this->serverSelectedProtocol);
    }
};
} // namespace UserData
} // namespace GCC
