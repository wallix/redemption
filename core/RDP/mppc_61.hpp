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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_MPPC_61_HPP_
#define _REDEMPTION_CORE_RDP_MPPC_61_HPP_

// [MS-RDPEGDI] 2.2.2.4.1 RDP 6.1 Compressed Data (RDP61_COMPRESSED_DATA)
// ======================================================================

// The RDP61_COMPRESSED_DATA structure stores chain-compressed data that has
//  been processed by the RDP 6.1 Level-1 Compression Engine and possibly the
//  RDP 5.0 Level-2 Compression Engine (see section 3.1.8.2.1 for a
//  description of chained compression and [MS-RDPBCGR] section 3.1.8.4.2 for
//  a description of RDP 5.0 bulk compression).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Level1Compr- |  Level2Compr- |     MatchCount (optional)     |
// |      Flags    |      Flags    |                               |
// +-------------------------------+-------------------------------+
// |                    MatchDetails (variable)                    |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      Literals (variable)                      |
// +---------------------------------------------------------------+

// Level1ComprFlags (1 byte): An 8-bit, unsigned integer. Level-1 compressor
//  flags.

// +----------------------+----------------------------------------------------+
// | Value                |  Meaning                                           |
// +----------------------+----------------------------------------------------+
// | L1_PACKET_AT_FRONT   | The level-1 history buffer MUST be reinitialized   |
// | 0x04                 | (by filling it with zeros).                        |
// +----------------------+----------------------------------------------------+
// | L1_NO_COMPRESSION    | No compression was performed. In this case, the    |
// | 0x02                 | MatchCount and MatchDetails fields MUST NOT be     |
// |                      | present. The Literals field MUST be present.       |
// +----------------------+----------------------------------------------------+
// | L1_COMPRESSED        | Compression with the level-1 compressor was        |
// | 0x01                 | performed and the MatchCount and MatchDetails      |
// |                      | fields MUST be present and contain at least one    |
// |                      | match. The Literals field MUST also be present.    |
// +----------------------+----------------------------------------------------+
// | L1_INNER_COMPRESSION | Indicates that additional level-2 compression has  |
// | 0x10                 | been performed on the level-1 compressor output    |
// |                      | and that the Level2ComprFlags field contains valid |
// |                      | data and MUST be processed.                        |
// +----------------------+----------------------------------------------------+

//  See sections 3.1.8.2.2.2 and 3.1.8.2.3 for more details regarding these
//   flags.

enum {
      L1_PACKET_AT_FRONT   = 0x04
    , L1_NO_COMPRESSION    = 0x02
    , L1_COMPRESSED        = 0x01
    , L1_INNER_COMPRESSION = 0x10
};

// Level2ComprFlags (1 byte): An 8-bit, unsigned integer. Level-2 compressor
//  flags.

// +-----------------------+---------------------------------------------------+
// | Value                 | Meaning                                           |
// +-----------------------+---------------------------------------------------+
// | PACKET_COMPR_TYPE_64K | Indicates that RDP 5.0 bulk compression (see      |
// | 0x01                  | [MS-RDPBCGR] section 3.1.8.4.2) was used.         |
// +-----------------------+---------------------------------------------------+
// | PACKET_COMPRESSED     | The data in the MatchCount, MatchDetails, and     |
// | 0x20                  | Literals fields has been compressed with the      |
// |                       | level-2 compressor.                               |
// +-----------------------+---------------------------------------------------+
// | PACKET_AT_FRONT       | The decompressed data MUST be placed at the       |
// | 0x40                  | beginning of the level-2 history buffer.          |
// +-----------------------+---------------------------------------------------+
// | PACKET_FLUSHED        | The level-2 history buffer MUST be reinitialized  |
// | 0x80                  | (by filling it with zeros).                       |
// +-----------------------+---------------------------------------------------+

//  See [MS-RDPBCGR] sections 3.1.8.2.1 and 3.1.8.3 for more details
//   regarding these flags.

//  The Level2ComprFlags field MUST be ignored if the L1_INNER_COMPRESSION
//   flag (0x10) is not set.

// MatchCount (2 bytes): A 16-bit, unsigned integer. Contains the number of
//  RDP 6.1 Match Details structures (see section 2.2.2.4.1.1) present in the
//  MatchDetails field. This field MUST be present if the L1_COMPRESSED flag
//  (0x01) is set in the Level1ComprFlags field.

// MatchDetails (variable): A variable-length field. Contains an array of RDP
//  6.1 Match Details structures (see section 2.2.2.4.1.1). This field MUST
//  NOT be present if the MatchCount field is not present. If the MatchCount
//  field is present and contains a value greater than zero, the MatchDetails
//  field MUST be present, and the number of RDP 6.1 Match Details structures
//  that it contains is given by the MatchCount field.

// Literals (variable): A variable-length field. Contains raw noncompressed
//  literals. The size of the Literals field is given by the equation,
//  CompressedDataSize - (2 + MatchCountSize + MatchDetailsSize).


// [MS-RDPEGDI] 2.2.2.4.1.1 RDP 6.1 Match Details (RDP61_MATCH_DETAILS)
// ====================================================================

// The RDP61_MATCH_DETAILS structure encapsulates all of the details, which
//  describes a compression match in a history buffer. (See section 3.1.8.2.2
//  for a description of how matches are employed within the RDP 6.1
//  Compression Engine.)

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          MatchLength          |       MatchOutputOffset       |
// +-------------------------------+-------------------------------+
// |                       MatchHistoryOffset                      |
// +---------------------------------------------------------------+

// MatchLength (2 bytes): A 16-bit, unsigned integer. The length in bytes of
//  the match in the level-1 history buffer.

// MatchOutputOffset (2 bytes): A 16-bit, unsigned integer. The relative
//  offset into the output buffer where the match MUST be copied to from the
//  history buffer.

// MatchHistoryOffset (4 bytes): A 32-bit, unsigned integer. The byte offset
//  into the current level-1 history buffer specifying where the match
//  begins.


static const size_t RDP_61_HISTORY_BUFFER_LENGTH = 2000000;
static const size_t RDP_61_MAX_DATA_BLOCK_SIZE   = 16382;


struct rdp_mppc_61_dec : public rdp_mppc_dec {
    uint8_t * historyBuffer;    // Livel-1 history buffer.
    size_t    historyOffset;    // Livel-1 history buffer associated history offset.

    rdp_mppc_50_dec level_2_decompressor;

    /**
     * Initialize rdp_mppc_61_dec structure
     */
    rdp_mppc_61_dec() {
        this->historyBuffer = static_cast<uint8_t *>(calloc(RDP_61_HISTORY_BUFFER_LENGTH, 1));
        this->historyOffset = 0;
    }

    /**
     * Deinitialize rdp_mppc_61_dec structure
     */
    virtual ~rdp_mppc_61_dec() {
        free(this->historyBuffer);
    }

private:
    static inline void prepare_compressed_data(Stream & compressed_data_stream, bool compressed,
        uint16_t & MatchCount, uint8_t *& MatchDetails, uint8_t *& Literals,
        size_t & literals_length)
    {
        if (compressed) {
            unsigned expected = 2; // MatchCount(2)
            if (!compressed_data_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%u",
                    expected, compressed_data_stream.in_remain());
                throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
            }
            MatchCount = compressed_data_stream.in_uint16_le();

            expected = MatchCount * 8; // MatchCount(2) * (MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4))
            if (!compressed_data_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%u",
                    expected, compressed_data_stream.in_remain());
                throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
            }
            MatchDetails = compressed_data_stream.p;
            compressed_data_stream.in_skip_bytes(expected);
        }
        else {
            MatchCount   = 0;
            MatchDetails = NULL;
        }

        literals_length = compressed_data_stream.in_remain();
        Literals        = (literals_length ? compressed_data_stream.p : NULL);
    }

public:
    virtual int decompress(uint8_t * compressed_data, int compressed_data_size,
        int compressionFlags, const uint8_t *& uncompressed_data, uint32_t & uncompressed_data_size)
    {
        //LOG(LOG_INFO, "decompress_61");
LOG(LOG_INFO, "");
LOG(LOG_INFO, "historyOffset=%d compressed_data_size=%d compressionFlags=0x%X", this->historyOffset, compressed_data_size, compressionFlags);

        uncompressed_data      = NULL;
        uncompressed_data_size = 0;

        StaticStream compressed_data_stream(compressed_data, compressed_data_size);

        unsigned expected = 2; // Level1ComprFlags(1) + Level2ComprFlags(1)
        if (!compressed_data_stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%u",
                expected, compressed_data_stream.in_remain());
            throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
        }

        uint8_t Level1ComprFlags = compressed_data_stream.in_uint8();
        uint8_t Level2ComprFlags = compressed_data_stream.in_uint8();
LOG(LOG_INFO, "Level1ComprFlags=0x%X Level2ComprFlags=0x%X", Level1ComprFlags, Level2ComprFlags);

        if (!(Level1ComprFlags & (L1_COMPRESSED | L1_NO_COMPRESSION))) {
            LOG(LOG_ERR, "Level-1 no historyBuffer update");
            throw Error(ERR_RDP61_DECOMPRESS);
        }

        uint16_t   MatchCount;
        uint8_t  * MatchDetails;
        uint8_t  * Literals;
        size_t     literals_length;

        if ((Level1ComprFlags & L1_INNER_COMPRESSION) && (Level2ComprFlags & PACKET_COMPRESSED)) {
LOG(LOG_INFO, "(Level1ComprFlags & L1_INNER_COMPRESSION) && (Level2ComprFlags & PACKET_COMPRESSED)");

            const uint8_t * level_1_compressed_data;
            uint32_t        level_1_compressed_data_size;

            int nResult = this->level_2_decompressor.decompress(compressed_data_stream.p,
                compressed_data_stream.in_remain(), Level2ComprFlags, level_1_compressed_data,
                level_1_compressed_data_size);
            if (nResult != true) {
                LOG(LOG_ERR, "RDP 6.1 bluk compression Level-2 decompression error");
                throw Error(ERR_RDP61_DECOMPRESS_LEVEL_2);
            }
LOG(LOG_INFO, "level_1_compressed_data_size=%d", level_1_compressed_data_size);

            StaticStream level_1_compressed_data_stream(level_1_compressed_data,
                level_1_compressed_data_size);

            rdp_mppc_61_dec::prepare_compressed_data(level_1_compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }
        else {
LOG(LOG_INFO, "!(Level1ComprFlags & L1_INNER_COMPRESSION) || !(Level2ComprFlags & PACKET_COMPRESSED)");
            rdp_mppc_61_dec::prepare_compressed_data(compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }

LOG(LOG_INFO, "MatchCount=%d literals_length=%d", MatchCount, literals_length);

        if (Level1ComprFlags & L1_PACKET_AT_FRONT) {
LOG(LOG_INFO, "Level1ComprFlags & L1_PACKET_AT_FRONT");
//            ::memset(this->historyBuffer, 0, RDP_61_HISTORY_BUFFER_LENGTH);
            this->historyOffset = 0;
        }

        StaticStream match_details_stream(MatchDetails,
            MatchCount *
            8);   // MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4)
        StaticStream literals_stream(Literals, literals_length);

        uint8_t  * current_output_buffer = this->historyBuffer + this->historyOffset;
        uint16_t   current_output_offset = 0;

        for (uint16_t match_index = 0; match_index < MatchCount; match_index++) {
            expected = 8;   // MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4)
            if (!match_details_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%u",
                    expected, match_details_stream.in_remain());
                throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
            }
            uint16_t MatchLength        = match_details_stream.in_uint16_le();
            uint16_t MatchOutputOffset  = match_details_stream.in_uint16_le();
            uint32_t MatchHistoryOffset = match_details_stream.in_uint32_le();
LOG(LOG_INFO, "MatchHistoryOffset=%u MatchLength=%d MatchOutputOffset=%d", MatchHistoryOffset, MatchLength, MatchOutputOffset);

            if (MatchOutputOffset > current_output_offset) {
                expected = MatchOutputOffset - current_output_offset;
                if (!literals_stream.in_check_rem(expected)) {
                    LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%u",
                        expected, literals_stream.in_remain());
                    throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
                }

                literals_stream.in_copy_bytes(
                    current_output_buffer + current_output_offset,
                    expected);

                current_output_offset = MatchOutputOffset;
            }

//            ::memcpy(current_output_buffer + current_output_offset,
//                this->historyBuffer + MatchHistoryOffset, MatchLength);
            uint8_t * src  = this->historyBuffer + MatchHistoryOffset;
            uint8_t * dest = current_output_buffer + current_output_offset;
            for (uint16_t i = 0; i < MatchLength; i++, src++, dest++)
                *dest = *src;

            current_output_offset += MatchLength;
        }

        if (size_t remaining_bytes = literals_stream.in_remain()) {
            literals_stream.in_copy_bytes(
                current_output_buffer + current_output_offset,
                remaining_bytes);

            current_output_offset += remaining_bytes;
        }

        uncompressed_data      = this->historyBuffer + this->historyOffset;
        uncompressed_data_size = current_output_offset;

        this->historyOffset += current_output_offset;
LOG(LOG_INFO, "uncompressed_data_size=%d historyOffset=%d", uncompressed_data_size, this->historyOffset);

        return false;
    }

    virtual void dump() {
        LOG(LOG_INFO, "Type=RDP 6.1 bulk decompressor");
    }

    virtual void mini_dump() {
        LOG(LOG_INFO, "Type=RDP 6.1 bulk decompressor");
    }
};


struct rdp_mppc_61_enc : public rdp_mppc_enc {
    uint8_t * historyBuffer;    // Livel-1 history buffer.
    size_t    historyOffset;    // Livel-1 history buffer associated history offset.

    rdp_mppc_50_enc level_2_compressor;

    /**
     * Initialize rdp_mppc_61_enc structure
     */
    rdp_mppc_61_enc() {
        this->historyBuffer = static_cast<uint8_t *>(calloc(RDP_61_HISTORY_BUFFER_LENGTH, 1));
        this->historyOffset = 0;
    }

    /**
     * Deinitialize rdp_mppc_61_enc structure
     */
    virtual ~rdp_mppc_61_enc() {
        free(this->historyBuffer);
    }

    virtual bool compress_61(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size) {
        //LOG(LOG_INFO, "compress_61");

        if (uncompressed_data_size > RDP_61_MAX_DATA_BLOCK_SIZE)
            return true;

        return false;
    }

    virtual bool compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size, uint8_t & flags, uint16_t & compressedLength) {
        return false;
    }

    virtual void get_compressed_data(Stream & stream) const {
    }

    virtual void dump() {
    }

    virtual void mini_dump() {
    }
};  // struct rdp_mppc_61_enc

#endif  // #ifndef _REDEMPTION_CORE_RDP_MPPC_61_HPP_