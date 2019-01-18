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

#include "utils/log.hpp"
#include "core/error.hpp"
#include "utils/stream.hpp"
#include "core/RDP/capabilities/common.hpp"

#include <memory>
#include <cstring>


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
     , CODEC_GUID_IMAGE_REMOTEFX
     };


struct CodecGenCaps {
    // Super class for Codecs (REMOTEFX & NSCODEC)
    CodecGenCaps() = default;

    virtual ~CodecGenCaps() = default;

    virtual void emit(OutStream & out) const = 0;
    virtual void recv(InStream & stream, uint16_t len) = 0;
    virtual size_t computeSize() const = 0;
};

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

struct NSCodecCaps : public CodecGenCaps {

    uint8_t fAllowDynamicFidelity{0}; // true/false
    uint8_t fAllowSubsampling{0};     // true/false
    uint8_t colorLossLevel{1};        // Between 1 and 7

    NSCodecCaps() = default;

    void emit(OutStream & out) const override {
        out.out_uint8(this->fAllowDynamicFidelity);
        out.out_uint8(this->fAllowSubsampling);
        out.out_uint8(this->colorLossLevel);
    }

    void recv(InStream & stream, uint16_t len) override {
        size_t expected = 3;
        if (len < 3) {
            LOG(LOG_ERR, "Truncated NSCodecCaps, needs=%zu remains=%u", expected, len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->fAllowDynamicFidelity = stream.in_uint8();
        this->fAllowSubsampling = stream.in_uint8();
        this->colorLossLevel = stream.in_uint8();
    }

    size_t computeSize() const override {
        return 3;
    }
};


struct RFXSrvrCaps : public CodecGenCaps {

    std::unique_ptr<uint8_t[]> reserved;

    RFXSrvrCaps() = default;

    void setReserved(uint16_t len) {
        this->reserved = std::make_unique<uint8_t[]>(len);
        memset(this->reserved.get(), 0xff, len);
    }

    void emit(OutStream & /*out_stream*/) const override {
    }

    void recv(InStream & stream, uint16_t len) override {
        stream.in_skip_bytes(len);
    }

    size_t computeSize() const override {
        return 0;
    }
};

struct RFXICap {
	enum {
	   CT_TILE_64X64 = 0x40
	};

    uint16_t version{CLW_VERSION_1_0};          // MUST be set to 0x0100 CLW_VERSION_1_0
    uint16_t tileSize{CT_TILE_64X64};           // MUST be set to CT_TILE_64x64 (0x0040)
    uint8_t  flags{0};                          // flag from enum
    uint8_t  colConvBits{CLW_COL_CONV_ICT};     // MUST be set to CLW_COL_CONV_ICT (0x1)
    uint8_t  transformBits{CLW_XFORM_DWT_53_A}; // MUST be set to CLW_COL_CONV_ICT (0x1)
    uint8_t  entropyBits{CLW_ENTROPY_RLGR1};    // MUST be set to one of the following values :
                                                //   - CLW_ENTROPY_RLGR1
                                                //   - CLW_ENTROPY_RLGR3

    RFXICap() = default;

    void recv(InStream & stream, uint16_t len) {

        if (len < 7) {
            LOG(LOG_ERR, "Truncated RFXICap, needs=7 remains=%u", len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }


        this->version = stream.in_uint16_le();
        if (this->version != CLW_VERSION_1_0) {
            LOG(LOG_ERR, "RFXICap expecting version=1.0");
        }

        this->tileSize = stream.in_uint16_le();
        if (this->tileSize != CT_TILE_64X64) {
            LOG(LOG_ERR, "RFXICap expecting tileSize=64x64");
        }
        this->flags = stream.in_uint8();
        this->colConvBits = stream.in_uint8();
        if (this->colConvBits != CLW_COL_CONV_ICT) {
            LOG(LOG_ERR, "RFXICap expecting colConvBits=CLW_COL_CONV_ICT");
        }
        this->transformBits = stream.in_uint8();
        if (this->transformBits != CLW_XFORM_DWT_53_A) {
            LOG(LOG_ERR, "RFXICap expecting transformBits=CLW_XFORM_DWT_53_A");
        }

        this->entropyBits = stream.in_uint8();
        switch(this->entropyBits) {
        case CLW_ENTROPY_RLGR1:
        case CLW_ENTROPY_RLGR3:
            break;
        default:
            LOG(LOG_ERR, "RFXICap unknown entropyBits");
            break;
        }
    }

    void emit(OutStream & out) const {
        out.out_uint16_le(this->version);
        out.out_uint16_le(this->tileSize);
        out.out_uint8(this->flags);
        out.out_uint8(this->colConvBits);
        out.out_uint8(this->transformBits);
        out.out_uint8(this->entropyBits);
    }

    size_t computeSize() const {
        return 8;
    }
};

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

struct RFXCapset {

    uint16_t blockType{CBY_CAPSET}; // MUST be set to CBY_CAPSET (0xCBC1)
    uint32_t blockLen{0};           // total length in bytes of the fields of that structure
    uint8_t  codecId{1};            // MUST be set to 0x01
    uint16_t capsetType{CLY_CAPSET};// MUST be set to CLY_CAPSET (0xCFC0)
    uint16_t numIcaps{0};           // number of elements in icapsData array
    uint16_t icapLen{8};            // total length in bytes of the fields of a icap structure (i.e. a row in icapsData array)

    std::unique_ptr<RFXICap[]> icapsData;

    RFXCapset()
    : numIcaps(1)
    , icapsData(std::make_unique<RFXICap[]>(this->numIcaps))
    {
        this->blockLen = 13 + 8;
    }

    void emit(OutStream & out) const {
        out.out_uint16_le(this->blockType);
        out.out_uint32_le(13 + this->icapLen * this->numIcaps);
        out.out_uint8(this->codecId);
        out.out_uint16_le(this->capsetType);
        out.out_uint16_le(this->numIcaps);
        out.out_uint16_le(this->icapLen);

        if (this->icapsData) {
            for (int i = 0; i < this->numIcaps; i++) {
                this->icapsData[i].emit(out);
            }
        }
    }

    void recv(InStream & stream, uint16_t len) {
        this->blockType = stream.in_uint16_le();
        if (this->blockType != CBY_CAPSET) {
            LOG(LOG_ERR, "RFXCapset expecting blockType=CBY_CAPSET");
        }

        this->blockLen = stream.in_uint32_le();
        if (this->blockLen < 12 || blockLen-12 > len) {
            LOG(LOG_ERR, "Truncated RFXCapset, needs=%u remains=%zu", this->blockLen, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->codecId = stream.in_uint8();
        if (this->codecId != 1) {
            LOG(LOG_ERR, "RFXCapset expecting codecId=1");
        }
        this->capsetType = stream.in_uint16_le();
        this->numIcaps = stream.in_uint16_le();
        this->icapLen = stream.in_uint16_le();

        if (this->numIcaps){
            icapsData = std::make_unique<RFXICap[]>(numIcaps);
        }

        unsigned expected = this->numIcaps * this->icapLen;
        if (len < expected + 13) {
            LOG(LOG_ERR, "Truncated RFXCapset, needs=%u remains=%zu", expected, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        for (int i = 0; i < this->numIcaps; i++) {
            this->icapsData[i].recv(stream, this->icapLen);
        }
    }

    size_t computeSize() const {
        return 13 + (this->numIcaps * this->icapLen);
    }
};

struct RFXCaps {

    uint16_t blockType{CBY_CAPS}; // MUST be set to CBY_CAPS (0xCBC0)
    uint32_t blockLen{8};         // MUST be set to 0x0008
    uint16_t numCapsets{1};       // MUST be set to 0x0001

    std::unique_ptr<RFXCapset> capsetsData{nullptr};

    RFXCaps()
    : blockType(CBY_CAPS)
    , blockLen(8)
    , numCapsets(1)
    , capsetsData(std::make_unique<RFXCapset>())
    {
    }

    void emit(OutStream & out) const {
        out.out_uint16_le(this->blockType);
        out.out_uint32_le(this->blockLen);
        out.out_uint16_le(this->numCapsets);
        capsetsData->emit(out);
    }

    void recv(InStream & stream, uint16_t len) {
        if (len < 8){
            LOG(LOG_ERR, "RFXCaps, truncated pdu need=8 got=%u", len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }
        this->blockType = stream.in_uint16_le();
        if (this->blockType != CBY_CAPS) {
            LOG(LOG_ERR, "RFXCaps, expecting blockType=CBY_CAPS");
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->blockLen = stream.in_uint32_le();
        if (this->blockLen != 8) {
            LOG(LOG_ERR, "RFXCaps, expecting blockLen=8");
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->numCapsets = stream.in_uint16_le();
        if (this->numCapsets != 1) {
            LOG(LOG_ERR, "RFXCaps, expecting numCapsets=1");
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }
    }

    size_t computeSize() const {
        return 8 + this->capsetsData->computeSize();
    }
};

struct RFXClntCaps : public CodecGenCaps {

    uint32_t length{0};
    uint32_t captureFlags{0};
    uint32_t capsLength{0};

    std::unique_ptr<RFXCaps> capsData{nullptr};

    RFXClntCaps()
    : captureFlags(CARDP_CAPS_CAPTURE_NON_CAC)
    , capsData(std::make_unique<RFXCaps>())
    {
         this->capsLength = capsData->computeSize();
         this->length = 12 + this->capsLength;
    }

    void emit(OutStream & out) const override {
        out.out_uint32_le(this->length);
        out.out_uint32_le(this->captureFlags);
        out.out_uint32_le(this->capsLength);

        if (capsData){
            this->capsData->emit(out);
        }
    }

    void recv(InStream & stream, uint16_t len) override {
        if (len < 12) {
            LOG(LOG_ERR, "Truncated RFXClntCaps, need=%u remains=%zu", this->capsLength, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->length = stream.in_uint32_le();
        this->captureFlags = stream.in_uint32_le();
        this->capsLength = stream.in_uint32_le();

        if (len < this->capsLength + 12) {
            LOG(LOG_ERR, "Truncated RFXClntCaps, need=%u remains=%zu", this->capsLength, stream.in_remain());
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        if (capsLength) {
            this->capsData = std::make_unique<RFXCaps>();
            this->capsData->recv(stream, this->capsLength);
        }
    }

    size_t computeSize() const override {
        return length;
    }

};

// TODO enum class
enum BitmapCodecType {
    CODEC_REMOTEFX,
    CODEC_NS,
    CODEC_IGNORE,
    CODEC_UNKNOWN
};

struct BitmapCodec {

    uint8_t  codecGUID[16];
    uint8_t  codecID{0};  // CS : a bitmap data identifier code
                          // SC :
                          //    - if codecGUID == CODEC_GUID_NSCODEC, MUST be set to 1
    uint16_t codecPropertiesLength{0}; // size in bytes of the next field

    BitmapCodecType codecType{CODEC_UNKNOWN};
    std::unique_ptr<CodecGenCaps> codecProperties{nullptr};

    BitmapCodec()
    {
        memset(this->codecGUID, 0, 16); // 16 bits array filled with fixed lists of values
    }

    void setCodecGUID(uint8_t codecGUID, bool client) {

        switch(codecGUID) {
        case CODEC_GUID_NSCODEC:
            memcpy(this->codecGUID, "\xB9\x1B\x8D\xCA\x0F\x00\x4F\x15\x58\x9F\xAE\x2D\x1A\x87\xE2\xD6", 16);
            this->codecID = 1;
            this->codecType = CODEC_NS;
            this->codecProperties = nullptr;
            break;
        case CODEC_GUID_REMOTEFX:
        case CODEC_GUID_IMAGE_REMOTEFX:
            if (codecGUID == CODEC_GUID_IMAGE_REMOTEFX){
                memcpy(this->codecGUID, "\xD4\xCC\x44\x27\x8A\x9D\x74\x4E\x80\x3C\x0E\xCB\xEE\xA1\x9C\x54", 16);
            }
            else {
                memcpy(this->codecGUID, "\x12\x2F\x77\x76\x72\xBD\x63\x44\xAF\xB3\xB7\x3C\x9C\x6F\x78\x86", 16);
            }

            if (client) {
                this->codecProperties = std::make_unique<RFXClntCaps>();
            }
            else {
                this->codecProperties = std::make_unique<RFXSrvrCaps>();
            }
            this->codecType = CODEC_REMOTEFX;
            break;
        default:
            memset(this->codecGUID, 0, 16);
            this->codecType = CODEC_UNKNOWN;
            this->codecProperties = nullptr;
            break;
        }

        if (this->codecProperties) {
            this->codecPropertiesLength = this->codecProperties->computeSize();
        }
    }

    size_t computeSize() const {
        size_t ret = 19;
        if (this->codecProperties) {
            ret += this->codecProperties->computeSize();
        }
        return ret;
    }

    void recv(InStream & stream, uint16_t & len, bool clientMode) {
        if (len < 19){
            LOG(LOG_ERR, "Truncated BitmapCodecs, need=19 remains=%u", len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        stream.in_copy_bytes(codecGUID, 16);
        this->codecID = stream.in_uint8();
        this->codecPropertiesLength = stream.in_uint16_le();
        len -= 19;

        uint16_t expected = this->codecPropertiesLength;
        if (len < expected){
            LOG(LOG_ERR, "Truncated codec properties in BitmapCodecs, need=%u remains=%u", expected, len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }
        len -= this->codecPropertiesLength;

        if (memcmp(codecGUID, "\xB9\x1B\x8D\xCA\x0F\x00\x4F\x15\x58\x9F\xAE\x2D\x1A\x87\xE2\xD6", 16) == 0) {
            /* CODEC_GUID_NSCODEC */
            this->codecProperties = std::make_unique<NSCodecCaps>();
            this->codecType = CODEC_NS;
            this->codecProperties->recv(stream, this->codecPropertiesLength);
        } else if((memcmp(codecGUID, "\x12\x2F\x77\x76\x72\xBD\x63\x44\xAF\xB3\xB7\x3C\x9C\x6F\x78\x86", 16) == 0)
               || (memcmp(codecGUID, "\xD4\xCC\x44\x27\x8A\x9D\x74\x4E\x80\x3C\x0E\xCB\xEE\xA1\x9C\x54", 16) == 0)) {
            /* CODEC_GUID_REMOTEFX or CODEC_GUID_IMAGE_REMOTEFX */
            if (clientMode){
                this->codecProperties = std::make_unique<RFXClntCaps>();
            }
            else {
                this->codecProperties = std::make_unique<RFXSrvrCaps>();
            }
            this->codecProperties->recv(stream, this->codecPropertiesLength);
            this->codecType = CODEC_REMOTEFX;
        } else if (memcmp(codecGUID, "\xA6\x51\x43\x9C\x35\x35\xAE\x42\x91\x0C\xCD\xFC\xE5\x76\x0B\x58", 16) == 0) {
            /* CODEC_GUID_IGNORE */
            this->codecType = CODEC_IGNORE;
            stream.in_skip_bytes(this->codecPropertiesLength);
        } else {
            this->codecType = CODEC_UNKNOWN;
            stream.in_skip_bytes(this->codecPropertiesLength);
            LOG(LOG_ERR, "unknown codec");
        }
    }

    void emit(OutStream & out) const {
        out.out_copy_bytes(this->codecGUID, 16);
        out.out_uint8(this->codecID);
        out.out_uint16_le(this->codecPropertiesLength);
        if (this->codecProperties) {
            this->codecProperties->emit(out);
        }
    }
};


struct BitmapCodecs {

    uint8_t bitmapCodecCount{0};

    BitmapCodec bitmapCodecArray[BITMAPCODECS_MAX_SIZE];

    BitmapCodecs() = default;
};

enum {
    CAPLEN_BITMAP_CODECS_CAPS = 5
};

struct BitmapCodecCaps : public Capability {

    BitmapCodecs supportedBitmapCodecs;
    bool clientMode;
    uint8_t remoteFxCodecId;

    BitmapCodecCaps(bool client)
    : Capability(CAPSETTYPE_BITMAP_CODECS, CAPLEN_BITMAP_CODECS_CAPS)
    , clientMode(client)
    , remoteFxCodecId(1)
    {
    }

    void addCodec(uint8_t codecType, uint8_t codecId) {
        switch(codecType) {
        case CODEC_GUID_REMOTEFX:
        case CODEC_GUID_IMAGE_REMOTEFX: {
            BitmapCodec *codec = &supportedBitmapCodecs.bitmapCodecArray[supportedBitmapCodecs.bitmapCodecCount];
            codec->setCodecGUID(codecType, this->clientMode);
            codec->codecID = this->remoteFxCodecId = codecId;
            supportedBitmapCodecs.bitmapCodecCount++;
            break;
        }
        case CODEC_GUID_NSCODEC:
        default:
            LOG(LOG_ERR, "unsupported codecType=%u", codecType);
            break;
        }
    }

    void emit(OutStream & out) const {
        size_t codecsLen = 0;
        for (int i = 0; i < supportedBitmapCodecs.bitmapCodecCount; i++) {
            codecsLen += supportedBitmapCodecs.bitmapCodecArray[i].computeSize();
        }
        out.out_uint16_le(this->capabilityType);
        out.out_uint16_le(CAPLEN_BITMAP_CODECS_CAPS + codecsLen);

        out.out_uint8(supportedBitmapCodecs.bitmapCodecCount);

        for (int i = 0; i < supportedBitmapCodecs.bitmapCodecCount; i++) {
            supportedBitmapCodecs.bitmapCodecArray[i].emit(out);
        }
    }

    void recv(InStream & stream, uint16_t len) {
        this->len = len;

        unsigned expected = 1;
        if (this->len < expected){
            LOG(LOG_ERR, "Truncated BitmapCodecs, need=%u remains=%hu", expected, this->len);
            throw Error(ERR_MCS_PDU_TRUNCATED);
        }

        this->supportedBitmapCodecs.bitmapCodecCount = stream.in_uint8();
        uint16_t tmplen = this->len-5;
        for (int i = 0; i < this->supportedBitmapCodecs.bitmapCodecCount; i++) {
            this->supportedBitmapCodecs.bitmapCodecArray[i].recv(stream, tmplen, this->clientMode);
            if (this->supportedBitmapCodecs.bitmapCodecArray[i].codecType == CODEC_REMOTEFX){
                this->remoteFxCodecId = this->supportedBitmapCodecs.bitmapCodecArray[i].codecID;
            }

        }
    }

    void log(const char * msg) const {
        LOG(LOG_INFO, "%s BitmapCodecCaps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "BitmapCodecsCaps::BitmapCodecs::bitmapCodecCount %u", this->supportedBitmapCodecs.bitmapCodecCount);
        LOG(LOG_INFO, "BitmapCodecCaps::supportedBitmapCodecs %p", static_cast<void const*>(&this->supportedBitmapCodecs));
    }
};
