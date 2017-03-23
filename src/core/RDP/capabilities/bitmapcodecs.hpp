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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Bitmap Codecs Capability Set ([MS-RDPBCGR] section 2.2.7.2.10)

*/


#pragma once

#include <string.h>
#include "common.hpp"

// 2.2.7.2.10 Bitmap Codecs Capability Set (TS_BITMAPCODECS_CAPABILITYSET)
// =======================================================================
// The TS_BITMAPCODECS_CAPABILITYSET structure advertises support for bitmap encoding and
// decoding codecs used in conjunction with the Set Surface Bits Surface Command (section 2.2.9.2.1)
// and Cache Bitmap (Revision 3) Secondary Drawing Order ([MS-RDPEGDI] section 2.2.2.2.1.2.8).
// This capability is sent by both the client and server.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of capability set. This field
//    MUST be set to 0x001D (CAPSETTYPE_BITMAP_CODECS).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//    data.

// supportedBitmapCodecs (variable): A variable-length field containing a TS_BITMAPCODECS
//    structure (section 2.2.7.2.10.1).

// 2.2.7.2.10.1 Bitmap Codecs (TS_BITMAPCODECS)
// ============================================
// The TS_BITMAPCODECS structure contains an array of bitmap codec capabilities.

// bitmapCodecCount (1 byte): An 8-bit, unsigned integer. The number of bitmap codec
//   capability entries contained in the bitmapCodecArray field (the maximum allowed is 255).
//
// bitmapCodecArray (variable): A variable-length array containing a series of
//   TS_BITMAPCODEC structures (section 2.2.7.2.10.1.1) that describes the supported bitmap
//    codecs. The number of TS_BITMAPCODEC structures contained in the array is given by the
//    bitmapCodecCount field.

// 2.2.7.2.10.1.1 Bitmap Codec (TS_BITMAPCODEC)
// ============================================
// The TS_BITMAPCODEC structure is used to describe the encoding parameters of a bitmap codec.

// codecGUID (16 bytes): A Globally Unique Identifier (section 2.2.7.2.10.1.1.1) that functions
//    as a unique ID for each bitmap codec.
//    +------------------------------------+----------------------------------------------------+
//    | Value                              | Meaning                                            |
//    +------------------------------------+----------------------------------------------------+
//    | CODEC_GUID_NSCODEC                 | The Bitmap Codec structure defines encoding        |
//    | 0xCA8D1BB9000F154F589FAE2D1A87E2D6 | parameters for the NSCodec Bitmap Codec ([MS-      |
//    |                                    | RDPNSC] sections 2 and 3). The codecProperties     |
//    |                                    | field MUST contain an NSCodec Capability Set ([MS- |
//    |                                    | RDPNSC] section 2.2.1) structure.                  |
//    +------------------------------------+----------------------------------------------------+
//    | CODEC_GUID_REMOTEFX                | The Bitmap Codec structure defines encoding        |
//    | 0x76772F12BD724463AFB3B73C9C6F7886 | parameters for the RemoteFX Bitmap Codec ([MS-     |
//    |                                    | RDPRFX] sections 2 and 3). The codecProperties     |
//    |                                    | field MUST contain a                               |
//    |                                    | TS_RFX_CLNT_CAPS_CONTAINER ([MS-RDPRFX]            |
//    |                                    | section 2.2.1.1) structure or a                    |
//    |                                    | TS_RFX_SRVR_CAPS_CONTAINER ([MS-RDPRFX]            |
//    |                                    | section 2.2.1.2) structure.                        |
//    +------------------------------------+----------------------------------------------------+

// codecID (1 byte): An 8-bit unsigned integer. When sent from the client to the server, this field
//    contains a unique 8-bit ID that can be used to identify bitmap data encoded using the codec in
//    wire traffic associated with the current connection - this ID is used in subsequent Set Surface
//    Bits commands (section 2.2.9.2.1) and Cache Bitmap (Revision 3) orders ([MS-RDPEGDI]
//    section 2.2.2.2.1.2.8). When sent from the server to the client, the value in this field is
//    ignored by the client - the client determines the 8-bit ID to use for the codec. If the
//    codecGUID field contains the CODEC_GUID_NSCODEC GUID, then this field MUST be set to
//    0x01 (the codec ID 0x01 MUST NOT be associated with any other bitmap codec).

// codecPropertiesLength (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
//    codecProperties field.

// codecProperties (variable): A variable-length array of bytes containing data that describes
//    the encoding parameter of the bitmap codec. If the codecGUID field is set to
//    CODEC_GUID_NSCODEC, this field MUST contain an NSCodec Capability Set ([MS-RDPNSC]
//    section 2.2.1) structure. Otherwise, if the codecGUID field is set to
//    CODEC_GUID_REMOTEFX, this field MUST contain a TS_RFX_CLNT_CAPS_CONTAINER ([MS-
//    RDPRFX] section 2.2.1.1) structure when sent from client to server, and a
//    TS_RFX_SRVR_CAPS_CONTAINER ([MS-RDPRFX] section 2.2.1.2) structure when sent from
//    server to client.

// 2.2.1  NSCodec Capability Set (TS_NSCODEC_CAPABILITYSET)
// ========================================================
// The TS_NSCODEC_CAPABILITYSET structure advertises properties of the NSCodec Bitmap
// Codec. This capability set is encapsulated in the codecProperties field of the Bitmap Codec ([MS-
// RDPBCGR] section 2.2.7.2.10.1.1) structure, which is ultimately encapsulated in the Bitmap Codecs
// Capability Set ([MS-RDPBCGR] section 2.2.7.2.10), which is encapsulated in a server-to-client
// Demand Active PDU ([MS-RDPBCGR] section 2.2.1.13.1) or client-to-server Confirm Active PDU
// ([MS-RDPBCGR] section 2.2.1.13.2).

//  fAllowDynamicFidelity (1 byte): An 8-bit unsigned integer that indicates support for lossy
//  bitmap compression by reducing color fidelity ([MS-RDPEGDI] section 3.1.9.1.4).

//    FALSE = 0x00
//    TRUE = 0x01

// fAllowSubsampling (1 byte): An 8-bit unsigned integer that indicates support for chroma
//    subsampling ([MS-RDPEGDI] section 3.1.9.1.3).

//    FALSE = 0x00
//    TRUE = 0x01

// colorLossLevel (1 byte): An 8-bit unsigned integer that indicates the maximum supported
//    Color Loss Level ([MS-RDPEGDI] section 3.1.9.1.4). This value MUST be between 1 and 7
//    (inclusive).


// 2.2.1.1 TS_RFX_CLNT_CAPS_CONTAINER
// ==================================
// The TS_RFX_CLNT_CAPS_CONTAINER structure is the top-level client capability container that
// wraps a TS_RFX_CAPS (section 2.2.1.1.1) structure and is sent from the client to the server. It is
// encapsulated in the codecProperties field of the Bitmap Codec ([MS-RDPBCGR] section
// 2.2.7.2.10.1.1) structure, which is ultimately encapsulated in the Bitmap Codecs Capability Set
// ([MS-RDPBCGR] section 2.2.7.2.10), which is encapsulated in a client-to-server Confirm Active PDU
// ([MS-RDPBCGR] section 2.2.1.13.2).

// length (4 bytes): A 32-bit, unsigned integer. Specifies the combined size, in bytes, of the
//    length, captureFlags, capsLength, and capsData fields.

// captureFlags (4 bytes): A 32-bit, unsigned integer. A collection of flags that allow a client to
//    control how data is captured and transmitted by the server.
//    +----------------------------+---------------------------------------------------------------+
//    | Flag                       | Meaning                                                       |
//    +----------------------------+---------------------------------------------------------------+
//    | CARDP_CAPS_CAPTURE_NON_CAC | The client supports mixing RemoteFX data with data            |
//    | 0x00000001                 | compressed by other codecs. The set of other codecs           |
//    |                            | supported by the client will be negotiated using the Bitmap   |
//    |                            | Codecs Capability Set ([MS-RDPBCGR] section 2.2.7.2.10).      |
//    +----------------------------+---------------------------------------------------------------+
// capsLength (4 bytes): A 32-bit, unsigned integer. Specifies the size, in bytes, of the
//    capsData field.
//
// capsData (variable): A variable-sized field that contains a TS_RFX_CAPS (section 2.2.1.1.1)
//    structure.

// 2.2.1.1.1 TS_RFX_CAPS
// =====================
// The TS_RFX_CAPS structure contains information about the decoder capabilities.

// blockType (2 bytes): A 16-bit, unsigned integer. Specifies the data block type. This field MUST
//    be set to CBY_CAPS (0xCBC0).
//
// blockLen (4 bytes): A 32-bit, unsigned integer. Specifies the combined size, in bytes, of the
//    blockType, blockLen, and numCapsets fields. This field MUST be set to 0x0008.
//
// numCapsets (2 bytes): A 16-bit, unsigned integer. Specifies the number of TS_RFX_CAPSET
//    (section 2.2.1.1.1.1) structures contained in the capsetsData field. This field MUST be set to
//    0x0001.
//
// capsetsData (variable): A variable-sized array of TS_RFX_CAPSET (section 2.2.1.1.1.1)
//    structures. The structures in this array MUST be packed on byte boundaries. The blockType
//    and blockLen fields of each TS_RFX_CAPSET structure identify the type and size of the
//    structure.

// 2.2.1.1.1.1 TS_RFX_CAPSET
// =========================
// The TS_RFX_CAPSET structure contains the capability information specific to the RemoteFX codec. It
// contains a variable number of TS_RFX_ICAP (section 2.2.1.1.1.1.1) structures that are used to
// configure the encoder state.

// blockType (2 bytes): A 16-bit, unsigned integer. Specifies the data block type. This field MUST
//    be set to CBY_CAPSET (0xCBC1).
//
// blockLen (4 bytes): A 32-bit, unsigned integer. Specifies the combined size, in bytes, of the
//    blockType, blockLen, codecId, capsetType, numIcaps, icapLen, and icapsData fields.
//
// codecId (1 byte): An 8-bit, unsigned integer. Specifies the codec ID. This field MUST be set to
//    0x01.
// capsetType (2 bytes): A 16-bit, unsigned integer. This field MUST be set to CLY_CAPSET
//    (0xCFC0).
//
// numIcaps (2 bytes): A 16-bit, unsigned integer. The number of TS_RFX_ICAP structures
//    contained in the icapsData field.
//
// icapLen (2 bytes): A 16-bit, unsigned integer. Specifies the size, in bytes, of each
//    TS_RFX_ICAP structure contained in the icapsData field.
//
// icapsData (variable): A variable-length array of TS_RFX_ICAP (section 2.2.1.1.1.1.1)
//    structures. Each structure MUST be packed on byte boundaries. The size of each
//    TS_RFX_ICAP structure within the array is specified in the icapLen field.

// 2.2.1.1.1.1.1  TS_RFX_ICAP
// ==========================
// The TS_RFX_ICAP structure specifies the set of codec properties that the decoder supports.

// version (2 bytes): A 16-bit, unsigned integer. Specifies the codec version. This field MUST be
//    set to 0x0100 CLW_VERSION_1_0, to indicate protocol version 1.0.

// tileSize (2 bytes): A 16-bit, signed integer. Specifies the width and height of a tile. This field
//    MUST be set to CT_TILE_64x64 (0x0040), indicating that a tile is 64 x 64 pixels.

// flags (1 byte): An 8-bit, unsigned integer. Specifies operational flags.
//    +------------+---------------------------------------------------------------------------------------+
//    | Flag       | Meaning                                                                               |
//    +------------+---------------------------------------------------------------------------------------+
//    | CODEC_MODE | The codec will operate in image mode. If this flag is not set, the codec will operate |
//    | 0x02       | in video mode.                                                                        |
//    +------------+---------------------------------------------------------------------------------------+
//    When operating in image mode, the encode headers messages (section 2.2.2.2) MUST always
//    precede an encoded frame. When operating in video mode, the header messages MUST be
//    present at the beginning of the stream and are optional elsewhere.

// colConvBits (1 byte): An 8-bit, unsigned integer. Specifies the color conversion transform.
// This field MUST be set to CLW_COL_CONV_ICT (0x1), and the transformation is by the
//  equations in sections 3.1.8.1.3 and 3.1.8.2.5.
//
// transformBits (1 byte): An 8-bit, unsigned integer. Specifies the DWT. This field MUST be set
//    to CLW_XFORM_DWT_53_A (0x1), the DWT transform given by the lifting equations for the
//    DWT shown in section 3.1.8.1.4 and by the lifting equations for the inverse DWT shown in
//    section 3.1.8.2.4.
//
// entropyBits (1 byte): An 8-bit, unsigned integer. Specifies the entropy algorithm. This field
//    MUST be set to one of the following values.
//    +-------------------+-----------------------------------------------------+
//    | Value             | Meaning                                             |
//    +-------------------+-----------------------------------------------------+
//    | CLW_ENTROPY_RLGR1 | RLGR algorithm as described in 3.1.8.1.7.1.         |
//    | 0x01              |                                                     |
//    +-------------------+-----------------------------------------------------+
//    | CLW_ENTROPY_RLGR3 | RLGR algorithm as described in section 3.1.8.1.7.2. |
//    | 0x04              |                                                     |
//    +-------------------+-----------------------------------------------------+

// 2.2.1.2 TS_RFX_SRVR_CAPS_CONTAINER
// ==================================
// The TS_RFX_SRVR_CAPS_CONTAINER structure is the top-level server capability container, which is
// sent from the server to the client. It is encapsulated in the codecProperties field of the Bitmap
// Codec structure ([MS-RDPBCGR] section 2.2.7.2.10.1.1), which is ultimately encapsulated in the
// Bitmap Codecs Capability Set ([MS-RDPBCGR] section 2.2.7.2.10). The Bitmap Codecs Capability
// Set is encapsulated in a server-to-client Demand Active PDU ([MS-RDPBCGR] section 2.2.1.13.1).

// reserved (variable): A variable-sized array of bytes. All the bytes in this field MUST be set to
//    0. The size of the field is given by the corresponding codecPropertiesLength field of the
//    parent TS_BITMAPCODEC, as specified in [MS-RDPBCGR] section 2.2.7.2.10.1.1 Bitmap
//    Codecs Capability Set.


enum {
       CLW_VERSION_1_0 = 0x0100
     };

enum {
       CT_TILE_64X64 = 0x40
     };

enum {
       CODEC_MODE = 0x02
     };

enum {
       CLW_COL_CONV_ICT = 0x01
     };

enum {
       CLW_XFORM_DWT_53_A = 0x01
     };

enum {
       CLW_ENTROPY_RLGR1 = 0x01
     , CLW_ENTROPY_RLGR3 = 0x04
     };

enum {
       CBY_CAPS = 0xCBC0
     , CBY_CAPSET = 0xCBC1
     };

enum {
       CLY_CAPSET = 0xCFC0
     };

enum {
       CARDP_CAPS_CAPTURE_NON_CAC = 0x01
     };

enum {
       BITMAPCODECS_MAX_SIZE = 0xFF
     };

enum {
       CODEC_GUID_NSCODEC
     , CODEC_GUID_REMOTEFX
     };

struct NSCodecCaps {

    uint8_t fAllowDynamicFidelity;
    uint8_t fAllowSubsampling;
    uint8_t colorLossLevel;

    NSCodecCaps()
    : fAllowDynamicFidelity(0)  // true/false
    , fAllowSubsampling(0)      // true/false
    , colorLossLevel(1)         // Between 1 and 7
    {
    }
};

struct RFXGenCaps {
    // Super class for Codecs (REMOTEFX & NSCODEC
    RFXGenCaps()
    {
    }
};

struct RFXSrvrCaps : public RFXGenCaps {

    uint8_t * reserved = nullptr;

    RFXSrvrCaps() = default;
    RFXSrvrCaps(RFXSrvrCaps const &) = delete;
    RFXSrvrCaps & operator = (RFXSrvrCaps const &) = delete;

    ~RFXSrvrCaps() {
        delete [] this->reserved;
    }

    void setReserved(uint16_t len) {
        this->reserved = new uint8_t[len];
        for (size_t i = 0; i < len; i++) {
            this->reserved[i] = 0xff;
        }
    }
};

struct RFXICap {

    uint16_t version;
    uint16_t tileSize;
    uint8_t  flags;
    uint8_t  colConvBits;
    uint8_t  transformBits;
    uint8_t  entropyBits;

//    RFXICap * icapsData;

    RFXICap()
    : version(CLW_VERSION_1_0)                // MUST be set to 0x0100 CLW_VERSION_1_0
    , tileSize(CT_TILE_64X64)    // MUST be set to CT_TILE_64x64 (0x0040
    , flags(0)                                     // flag from enum
    , colConvBits(CLW_COL_CONV_ICT)                // MUST be set to CLW_COL_CONV_ICT (0x1)
    , transformBits(CLW_XFORM_DWT_53_A)    // MUST be set to CLW_COL_CONV_ICT (0x1)
    , entropyBits(CLW_ENTROPY_RLGR1)            // MUST be set to one of the following values :
                                                //   - CLW_ENTROPY_RLGR1
                                                //   - CLW_ENTROPY_RLGR3
    {
    }
};

struct RFXCapset {

    uint16_t blockType;
    uint32_t blockLen;
    uint8_t  codecId;
    uint16_t capsetType;
    uint16_t numIcaps;
    uint16_t icapLen;

    RFXICap * icapsData;

    RFXCapset()
    : blockType(CBY_CAPSET)    // MUST be set to CBY_CAPSET (0xCBC1)
    , blockLen(0)             // total length in bytes of the fields of that structure
    , codecId(1)            // MUST be set to 0x01
    , capsetType(1)            // MUST be set to CLY_CAPSET (0xCFC0)
    , numIcaps(0)            // number of elements in icapsData array
    , icapLen(8)             // total length in bytes of the fields of a icap structure (i.e. a row in icapsData array)
    , icapsData(nullptr)
    {
    }
};

struct RFXCaps {

    uint16_t blockType;
    uint32_t blockLen;
    uint16_t numCapsets;

    RFXCapset * capsetsData;

    RFXCaps()
    : blockType(CBY_CAPS)    // MUST be set to CBY_CAPS (0xCBC0)
    , blockLen(8)             // MUST be set to 0x0008
    , numCapsets(1)            // MUST be set to 0x0001
    , capsetsData(nullptr)
    {
    }
};

struct RFXClntCaps : public RFXGenCaps {

    uint32_t length;
    uint32_t captureFlags;
    uint32_t capsLength;

    RFXCaps * capsData;

    RFXClntCaps()
    : length(0)          // Total length in bytes of that structure fields
    , captureFlags(0)    // flag from enum
    , capsLength(0)        // length in bytes of the next field
    , capsData(nullptr)
    {
    }
};

struct BitmapCodec {

    uint8_t  codecGUID[16];
    uint8_t  codecID;
    uint16_t codecPropertiesLength;

    RFXGenCaps * codecProperties;

    BitmapCodec()
    : codecID(0)                // CS : a bitmap data identifier code
                                // SC :
                                //    - if codecGUID == CODEC_GUID_NSCODEC, MUST be set to 1
    , codecPropertiesLength(0)  // size in bytes of the next field
    {
        memset(this->codecGUID, 0, 16); // 16 bits array filled with fixed lists of values
    }

    void setCodecGUID(uint8_t codecGUID) {

        if (codecGUID == CODEC_GUID_NSCODEC) {
            memcpy(this->codecGUID, "\xCA\x8D\x1B\xB9\x00\x0F\x15\x4F\x58\x9F\xAE\x2D\x1A\x87\xE2\xD6", 16);
            this->codecID = 1;
        }
        else if (codecGUID == CODEC_GUID_REMOTEFX)
            memcpy(this->codecGUID, "\x76\x77\x2F\x12\xBD\x72\x44\x63\xAF\xB3\xB7\x3C\x9C\x6F\x78\x86", 16);
        else
            memset(this->codecGUID, 0, 16);
    }
};


struct BitmapCodecs {

    uint8_t bitmapCodecCount;

    BitmapCodec bitmapCodecArray[BITMAPCODECS_MAX_SIZE];

    BitmapCodecs()
    : bitmapCodecCount(0)  // actual number of entries (max 255) in the array in the next field
    {
    }

};

enum {
    CAPLEN_BITMAP_CODECS = 1
};

struct BitmapCodecCaps : public Capability {

    BitmapCodecs supportedBitmapCodecs;

    BitmapCodecCaps()
    : Capability(CAPSTYPE_SHARE, CAPLEN_BITMAP_CODECS)
    {
    }

    void emit(OutStream &) {
        // TODO BitmapCodec::emit unimplemented
        LOG(LOG_INFO, "BitmapCodec caps emit not implemented");
    }

    void recv(InStream &, uint16_t len) {
        (void)len;
        // TODO BitmapCodec::recv unimplemented
        LOG(LOG_INFO, "BitmapCodec caps recv not implemented");
    }

    void log(const char * msg) {
        LOG(LOG_INFO, "%s BitmapCodec caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "BitmapCodec caps::supportedBitmapCodecs %p", static_cast<void*>(&this->supportedBitmapCodecs));
    }
};
