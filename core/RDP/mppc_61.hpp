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
        this->historyBuffer = static_cast<uint8_t *>(calloc(RDP_61_HISTORY_BUFFER_LENGTH, sizeof(uint8_t)));
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
//LOG(LOG_INFO, "");
//LOG(LOG_INFO, "historyOffset=%d compressed_data_size=%d compressionFlags=0x%X", this->historyOffset, compressed_data_size, compressionFlags);

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
//LOG(LOG_INFO, "Level1ComprFlags=0x%X Level2ComprFlags=0x%X", Level1ComprFlags, Level2ComprFlags);

        if (!(Level1ComprFlags & (L1_COMPRESSED | L1_NO_COMPRESSION))) {
            LOG(LOG_ERR, "Level-1 no historyBuffer update");
            throw Error(ERR_RDP61_DECOMPRESS);
        }

        uint16_t   MatchCount;
        uint8_t  * MatchDetails;
        uint8_t  * Literals;
        size_t     literals_length;

        if ((Level1ComprFlags & L1_INNER_COMPRESSION) && (Level2ComprFlags & PACKET_COMPRESSED)) {
//LOG(LOG_INFO, "(Level1ComprFlags & L1_INNER_COMPRESSION) && (Level2ComprFlags & PACKET_COMPRESSED)");

            const uint8_t * level_1_compressed_data;
            uint32_t        level_1_compressed_data_size;

            int nResult = this->level_2_decompressor.decompress(compressed_data_stream.p,
                compressed_data_stream.in_remain(), Level2ComprFlags, level_1_compressed_data,
                level_1_compressed_data_size);
            if (nResult != true) {
                LOG(LOG_ERR, "RDP 6.1 bluk compression Level-2 decompression error");
                throw Error(ERR_RDP61_DECOMPRESS_LEVEL_2);
            }
//LOG(LOG_INFO, "level_1_compressed_data_size=%d", level_1_compressed_data_size);

            StaticStream level_1_compressed_data_stream(level_1_compressed_data,
                level_1_compressed_data_size);

            rdp_mppc_61_dec::prepare_compressed_data(level_1_compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }
        else {
//LOG(LOG_INFO, "!(Level1ComprFlags & L1_INNER_COMPRESSION) || !(Level2ComprFlags & PACKET_COMPRESSED)");
            rdp_mppc_61_dec::prepare_compressed_data(compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }

//LOG(LOG_INFO, "MatchCount=%d literals_length=%d", MatchCount, literals_length);

        if (Level1ComprFlags & L1_PACKET_AT_FRONT) {
//LOG(LOG_INFO, "Level1ComprFlags & L1_PACKET_AT_FRONT");
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
//LOG(LOG_INFO, "MatchHistoryOffset=%u MatchLength=%d MatchOutputOffset=%d", MatchHistoryOffset, MatchLength, MatchOutputOffset);

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
//LOG(LOG_INFO, "uncompressed_data_size=%d historyOffset=%d", uncompressed_data_size, this->historyOffset);

        return false;
    }

    virtual void dump() {
        LOG(LOG_INFO, "Type=RDP 6.1 bulk decompressor");
    }

    virtual void mini_dump() {
        LOG(LOG_INFO, "Type=RDP 6.1 bulk decompressor");
    }
};


static const size_t RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH      = 9;    // sizeof(RDP61_MATCH_DETAILS) + 1
static const size_t RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE = 1024 * 32;
static const size_t RDP_61_COMPRESSOR_LITERALS_BUFFER_SIZE      = RDP_61_MAX_DATA_BLOCK_SIZE;
static const size_t RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE        =
    2 + // MatchCount(2)
    RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE +
    RDP_61_COMPRESSOR_LITERALS_BUFFER_SIZE
    ;

struct rdp_mppc_61_enc : public rdp_mppc_enc {
    uint8_t  * historyBuffer;   // Level-1 history buffer.
    uint32_t   historyOffset;   // Level-1 history buffer associated history offset.

    rdp_mppc_50_enc level_2_compressor;

    uint32_t * hash_table;

    uint8_t * level_1_output_buffer;

    StaticStream output_buffer_match_details;   // Level-1 output buffer for Match Datails. (Build over level_1_output_buffer)
    StaticStream output_buffer_literals;        // Level-1 output buffer for literals. (Build over level_1_output_buffer)

    uint8_t level_1_compr_flags_hold;
    uint8_t Level1ComprFlags;
    uint8_t Level2ComprFlags;

    uint8_t  * outputBuffer;
    uint16_t   bytes_in_output_buffer;

    /**
     * Initialize rdp_mppc_61_enc structure
     */
    rdp_mppc_61_enc()
        : historyBuffer(NULL)
        , historyOffset(0)
        , hash_table(NULL)
        , level_1_output_buffer(NULL)
        , level_1_compr_flags_hold(L1_PACKET_AT_FRONT)
        , Level1ComprFlags(0)
        , Level2ComprFlags(0)
        , outputBuffer(NULL)
        , bytes_in_output_buffer(0)
    {
        this->historyBuffer = static_cast<uint8_t *>(calloc(RDP_61_HISTORY_BUFFER_LENGTH, sizeof(uint8_t)));
        //this->historyOffset = 0;

        this->hash_table = static_cast<uint32_t *>(calloc(rdp_mppc_enc::HASH_BUF_LEN, sizeof(uint32_t)));

        this->level_1_output_buffer = static_cast<uint8_t *>(calloc(RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE,
            sizeof(uint8_t)));

        this->output_buffer_match_details.resize(
            this->level_1_output_buffer +
                2,  // MatchCount(2)
            RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE);
        this->output_buffer_literals.resize(
            this->level_1_output_buffer +
                2 + // MatchCount(2)
                RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE,
            RDP_61_COMPRESSOR_LITERALS_BUFFER_SIZE);

        //this->level_1_compr_flags_hold = L1_PACKET_AT_FRONT;
        //this->Level1ComprFlags         = 0;
        //this->Level2ComprFlags         = 0;

        //this->outputBuffer           = NULL;
        //this->bytes_in_output_buffer = 0;
    }

    /**
     * Deinitialize rdp_mppc_61_enc structure
     */
    virtual ~rdp_mppc_61_enc() {
        free(this->historyBuffer);

        free(this->hash_table);

        free(this->level_1_output_buffer);
    }

    virtual bool compress_61(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size) {
        //LOG(LOG_INFO, "compress_61");

        this->Level1ComprFlags = L1_INNER_COMPRESSION;
        this->Level2ComprFlags = 0;


        ::memset(this->level_1_output_buffer, 0, RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE);
        this->outputBuffer           = NULL;
        this->bytes_in_output_buffer = 0;

        this->output_buffer_match_details.reset();
        this->output_buffer_literals.reset();


        if ((uncompressed_data == NULL) || (uncompressed_data_size == 0) ||
            (uncompressed_data_size > RDP_61_MAX_DATA_BLOCK_SIZE) ||
            (uncompressed_data_size <= 2))   // Level1ComprFlags(1) + Level2ComprFlags(1)
            return true;

        if ((this->historyOffset + uncompressed_data_size + 1) >= RDP_61_HISTORY_BUFFER_LENGTH) {
            // historyBuffer cannot hold uncompressed_data - rewind it.
            this->level_1_compr_flags_hold |= L1_PACKET_AT_FRONT;
        }

        if (this->level_1_compr_flags_hold & L1_PACKET_AT_FRONT) {
            this->historyOffset =  0;

            ::memset(this->historyBuffer, 0, RDP_61_HISTORY_BUFFER_LENGTH * sizeof(uint8_t) );
            ::memset(this->hash_table,    0, rdp_mppc_enc::HASH_BUF_LEN   * sizeof(uint32_t));
        }

        // Add/append new data to historyBuffer.
        ::memcpy(this->historyBuffer + this->historyOffset, uncompressed_data, uncompressed_data_size);

        if (uncompressed_data_size < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH) {
            this->Level1ComprFlags |= L1_NO_COMPRESSION;
        }
        else {
            uint32_t   counter         = 0;
            uint8_t  * literals        = NULL;
            uint32_t   literals_length = 0;
            // If we are at start of history buffer, do not attempt to compress
            //  first RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1 bytes, because
            //  minimum LoM is RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH.
            if (this->historyOffset == 0) {
                // Encode first RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1 bytes as literals.
                literals        = this->historyBuffer;
                literals_length = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1;

                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer,     RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 0;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 1, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 1;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 2, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 2;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 3, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 3;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 4, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 4;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 5, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 5;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 6, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 6;
                this->hash_table[rdp_mppc_enc::signature(this->historyBuffer + 7, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] = 7;
                counter                                                                                                                              = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1;
            }

            uint32_t length_of_match = 0;
            // We need at least RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH bytes to look for match
            //  (> sizeof(RDP61_MATCH_DETAILS) = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1).
            for (; counter + (RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1) < uncompressed_data_size;
                 counter += length_of_match) {
                uint16_t crc16          = rdp_mppc_enc::signature(
                    this->historyBuffer + this->historyOffset + counter,
                    RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH);
                uint32_t previous_match = this->hash_table[crc16];

                this->hash_table[crc16] = this->historyOffset + counter;

                // Check that we have a pattern match, hash is not enough.

                if (0 != memcmp(this->historyBuffer + this->historyOffset + counter, this->historyBuffer + previous_match,
                                RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)) {
                    // No match found, encode literal uint8_t
                    if (!literals) {
                        literals = this->historyBuffer + this->historyOffset + counter;
                    }
                    literals_length++;
                    length_of_match = 1;
                }
                else {
                    if (literals_length) {
                        this->output_buffer_literals.out_copy_bytes(literals, literals_length);

                        literals        = NULL;
                        literals_length = 0;
                    }

                    // We have a match, compute hash and Length of Match for triplets.
                    this->hash_table[rdp_mppc_enc::signature(
                        this->historyBuffer + this->historyOffset + counter + 1,
                        RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] =
                            this->historyOffset + counter + 1;
                    // Maximum LOM is RDP_61_MAX_DATA_BLOCK_SIZE bytes.
                    for (length_of_match = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH;
                         (counter + length_of_match < uncompressed_data_size) &&
                             (length_of_match < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH);
                         length_of_match++) {
                        this->hash_table[rdp_mppc_enc::signature(
                            this->historyBuffer + this->historyOffset + counter + length_of_match - 1,
                            RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)] =
                                this->historyOffset + counter + length_of_match - 1;
                        if (this->historyBuffer[this->historyOffset + counter + length_of_match] !=
                            this->historyBuffer[previous_match + length_of_match])
                        {
                            break;
                        }

                        this->output_buffer_match_details.out_uint16_le(length_of_match);
                        this->output_buffer_match_details.out_uint16_le(counter);
                        this->output_buffer_match_details.out_uint32_le(previous_match);
                    }
                }
            }

            // Add remaining data if any to the output.
            uint32_t remaining_data_size;
            if ((remaining_data_size = uncompressed_data_size - counter) > 0) {
                this->output_buffer_literals.out_copy_bytes(uncompressed_data + counter,
                    remaining_data_size);
                counter += remaining_data_size;
            }

            this->output_buffer_match_details.mark_end();
            this->output_buffer_literals.mark_end();

            uint32_t match_details_data_size = this->output_buffer_match_details.size();
/*
            uint32_t MatchCount              = (match_details_data_size ?
                                                match_details_data_size / 8 :   // sizeof(RDP61_COMPRESSED_DATA) = 8
                                                0);
*/

            if ((match_details_data_size ? 2 : 0) +  // MatchCount(2)
                match_details_data_size +
                this->output_buffer_literals.size() >=
                uncompressed_data_size) {
                this->Level1ComprFlags |= L1_NO_COMPRESSION;
            }
            else {
                this->Level1ComprFlags |= L1_COMPRESSED;
            }
        }

        this->historyOffset += uncompressed_data_size;

/*
        uint16_t compressed_data_size;
        if ((match_details_data_size ? 2 : 0) +  // MatchCount(2)
            match_details_data_size +
            this->output_buffer_literals.size() >=
            uncompressed_data_size) {
            this->Level1ComprFlags |= L1_NO_COMPRESSION;

            this->level_2_compressor.compress(uncompressed_data, uncompressed_data_size, Level2ComprFlags,
                compressed_data_size);

            if (compressed_data_size < uncompressed_data_size) {
                this->outputBuffer           = reinterpret_cast<uint8_t *>(this->level_2_compressor.outputBuffer);
                this->bytes_in_output_buffer = this->level_2_compressor.bytes_in_opb;
            }
        }
        else {
            this->Level1ComprFlags |= L1_COMPRESSED;

            uint8_t * level_1_compressed_data =
                this->output_buffer_literals.get_data() -
                this->output_buffer_match_details.size() -
                2;  // MatchCount(2)

            StaticStream match_count_stream(level_1_compressed_data, 2); // MatchCount(2)
            match_count_stream.out_uint16_le(MatchCount);

            memmove(
                level_1_compressed_data +
                    2,                         // MatchCount(2)
                this->output_buffer_match_details.get_data(),
                this->output_buffer_match_details.size()
                );

            uint16_t level_1_compressed_data_size =
                2 + // MatchCount(2)
                this->output_buffer_match_details.size() +
                this->output_buffer_literals.size();

            this->level_2_compressor.compress(level_1_compressed_data, level_1_compressed_data_size, Level2ComprFlags,
                compressed_data_size);
        }
*/

        return true;
    }

    virtual bool compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType , uint16_t & compressed_data_size, uint16_t reserved)
    {
        this->compress_61(uncompressed_data, uncompressed_data_size);

        compressedType       = (this->bytes_in_output_buffer ? (PACKET_COMPRESSED | PACKET_COMPR_TYPE_RDP61) : 0);
        compressed_data_size = this->bytes_in_output_buffer;

        return true;
    }

    virtual void get_compressed_data(Stream & stream) const {
    }

    virtual void dump() {
    }

    virtual void mini_dump() {
    }
};  // struct rdp_mppc_61_enc

#endif  // #ifndef _REDEMPTION_CORE_RDP_MPPC_61_HPP_