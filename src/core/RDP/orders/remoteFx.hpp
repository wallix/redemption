/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): David Fort
*/

#pragma once

#include "utils/stream.hpp"
#include "utils/region.hpp"
#include "core/error.hpp"
#include "core/RDP/rlgr.hpp"
#include "gdi/graphic_api.hpp"
#include "core/RDP/capabilities/bitmapcodecs.hpp"


enum {
    WBT_SYNC = 0xCCC0,
    WBT_CODEC_VERSION = 0xCCC1,
    WBT_CHANNELS = 0xCCC2,
    WBT_CONTEXT = 0xCCC3,
    WBT_FRAME_BEGIN = 0xCCC4,
    WBT_FRAME_END = 0xCCC5,
    WBT_REGION = 0xCCC6,
    WBT_EXTENSION = 0xCCC7,
};

enum {
    WF_VERSION_1_0 = 0x0100,
    WF_MAGIC = 0xCACCACCA,
    CBT_REGION = 0xCAC1,
    CBT_TILESET = 0xCAC2,
    CBT_TILE = 0xCAC3,
};

enum {
    CT_TILE_64X64 = 0x0040,
};

enum {
    COL_CONV_ICT = 0x01,
    SCALAR_QUANTIZATION = 0x01
};

/** @brief a TS_RFX_RECT */
struct TS_RFX_RECT {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;

    void recv(InStream & stream);
    void send(OutStream & stream);
};


/** @brief a TS_RFX_SYNC packet */
struct TS_RFX_SYNC {
    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief a TS_RFX_CODEC_VERSIONS packet */
struct TS_RFX_CODEC_VERSIONS {
    uint8_t numCodecs{1};
    uint8_t codecId{1};
    uint16_t version{WF_VERSION_1_0};

    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief a TS_RFX_CODEC_CHANNELT packet */
struct TS_RFX_CODEC_CHANNELT {
    uint8_t codecId{0};
    uint8_t channelId{0};

    void recv(InStream & stream);
    void send(OutStream & stream);
};


/** @brief a TS_RFX_CONTEXT packet */
struct TS_RFX_CONTEXT : TS_RFX_CODEC_CHANNELT {
    uint8_t ctxId{0};
    uint16_t tileSize{CT_TILE_64X64};
    uint16_t properties{(COL_CONV_ICT << 3) | (CLW_XFORM_DWT_53_A << 5) | (CLW_ENTROPY_RLGR3 << 9) | (SCALAR_QUANTIZATION << 13)};

    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief a TS_RFX_CHANNELT packet */
struct TS_RFX_CHANNELT {
    uint8_t channelId{0};
    uint16_t width, height;

    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief a TS_RFX_CHANNELS packet */
struct TS_RFX_CHANNELS {
    uint8_t numChannels{0};
    TS_RFX_CHANNELT *channels{nullptr};

    ~TS_RFX_CHANNELS();

    void setChannel(uint8_t channelId, uint16_t width, uint16_t height);
    void recv(InStream & stream);
    void send(OutStream & stream);
};


/** @brief TS_RFX_FRAME_BEGIN */
struct TS_RFX_FRAME_BEGIN : public TS_RFX_CODEC_CHANNELT {
    uint32_t frameIdx;
    uint16_t numRegions;

    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief TS_RFX_FRAME_END */
struct TS_RFX_FRAME_END : public TS_RFX_CODEC_CHANNELT {
};

/** @brief a TS_RFX_REGION packet */
struct TS_RFX_REGION : public TS_RFX_CODEC_CHANNELT {
    uint8_t regionFlags;
    uint16_t numRects{0};
    TS_RFX_RECT *rects{nullptr};
    uint16_t regionType{CBT_REGION};
    uint16_t numTilesets{1};

    virtual ~TS_RFX_REGION();
    void recv(InStream & stream);
    void send(OutStream & stream);
};

/** @brief a TS_RFX_CODEC_QUANT */
struct TS_RFX_CODEC_QUANT {
    uint8_t ll3;
    uint8_t lh3;
    uint8_t hl3;
    uint8_t hh3;
    uint8_t lh2;
    uint8_t hl2;
    uint8_t hh2;
    uint8_t lh1;
    uint8_t hl1;
    uint8_t hh1;

    void recv(InStream & stream);
    void send(OutStream & stream);
};

struct TS_RFX_TILESET;
class Primitives;

/** @brief a TS_RFX_TILE packet */
struct TS_RFX_TILE {
    uint8_t quantIdxY, quantIdxCb, quantIdxCr;
    uint16_t xIdx, yIdx;
    uint16_t YLen, CbLen, CrLen;
    uint8_t *YData{nullptr};
    uint8_t *CbData{nullptr};
    uint8_t *CrData{nullptr};

    virtual ~TS_RFX_TILE() {
        delete [] this->YData;
        delete [] this->CbData;
        delete [] this->CrData;
    }

    void recv(InStream & stream);

    void draw(const RDPSetSurfaceCommand &cmd, const TS_RFX_TILESET &tileset, RDPSurfaceContent & content);
    void decodeComponent(const TS_RFX_CODEC_QUANT &quant, Rlgr::RlgrMode mode, uint8_t *data, size_t len, int16_t *output);

    static void differential_decode(int16_t *buffer, size_t size);
    static void rfx_quantization_decode_block(const Primitives *prims, int16_t * buffer, int buffer_size, uint32_t factor);
    static void quantization_decode(int16_t *buffer, const TS_RFX_CODEC_QUANT &quants);
};



/** @brief a TS_RFX_TILESET packet */
struct TS_RFX_TILESET : public TS_RFX_CODEC_CHANNELT {
    uint16_t subType{CBT_TILESET};
    uint16_t idx{0};
    uint16_t properties{0};
    Rlgr::RlgrMode rlgrMode{Rlgr::RLGR1};
    uint8_t numQuant;
    uint8_t tileSize;
    uint16_t numTiles;
    uint32_t tileDataSize;
    TS_RFX_CODEC_QUANT *quantVals{nullptr};
    TS_RFX_TILE *tiles{nullptr};

    virtual ~TS_RFX_TILESET();

    void recv(InStream & stream, const RDPSetSurfaceCommand &cmd, const SubRegion &region, gdi::GraphicApi & drawable);
};


/** @brief a decoder for the remoteFx codec */
class RfxDecoder {
public:
    /** @brief decoder states */
    enum DecoderState {
        RFX_WAITING_SYNC,
        RFX_WAITING_PROPERTIES,
        RFX_WAITING_FRAME
    };

    /** @brief haveFlags */
    enum {
        FLAG_CONTEXT = 0x1,
        FLAG_VERSIONS = 0x2,
        FLAG_CHANNELS = 0x4,
        FLAG_HAVE_ALL = 0x7
    };


    RfxDecoder(DecoderState initialState = RFX_WAITING_SYNC)
        : decoderState(initialState)
    {
    }

    [[nodiscard]] DecoderState getState() const {    return decoderState; }

    void recv(InStream & stream, const RDPSetSurfaceCommand & cmd, gdi::GraphicApi & drawable);


protected:
    DecoderState decoderState;
    uint8_t haveFlags = 0;
    TS_RFX_REGION currentRegion;
};

/** @brief an encoder for the remorteFx codec */
class RfxEncoder {
public:
    RfxEncoder();

    void sendFrame();
protected:
    void sendInitSequence();
protected:
    bool initialized;
    uint32_t frameCounter;
};
