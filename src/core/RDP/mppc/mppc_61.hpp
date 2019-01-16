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

#pragma once

#include "core/RDP/mppc.hpp"
#include "core/RDP/mppc/mppc_utils.hpp"
#include "core/RDP/mppc/mppc_50.hpp"
#include "utils/stream.hpp"

#include <type_traits> // std::is_base_of


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

// [MS-RDPEGDI] - 3.1.8.2.1 Abstract Data Model
// The shared state necessary to support the transmission and reception of RDP6.1-BC compressed data
// between a client and server requires a level-1 history buffer (HistoryBuffer) and a current offset into
// the history buffer (HistoryOffset). The size of the history buffer is fixed at 2,000,000 bytes. Any single
// block of data that is being compressed by a compliant compressor MUST be smaller in size than
// 16,383 bytes. The HistoryOffset MUST start initialized to zero, while the history buffer MUST be filled
// with zeros. After it has been initialized, the entire history buffer is immediately regarded as valid.
static const size_t RDP_61_MAX_DATA_BLOCK_SIZE = 16383 - 1;


static const size_t RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH      = 9;    // sizeof(RDP61_MATCH_DETAILS) + 1
static const size_t RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE = 1024 * 32;
static const size_t RDP_61_COMPRESSOR_LITERALS_BUFFER_SIZE      = RDP_61_MAX_DATA_BLOCK_SIZE;
static const size_t RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE        =
    2 + // MatchCount(2)
    RDP_61_COMPRESSOR_MATCH_DATAILS_BUFFER_SIZE +
    RDP_61_COMPRESSOR_LITERALS_BUFFER_SIZE
    ;


struct rdp_mppc_61_dec : public rdp_mppc_dec
{
    uint8_t   historyBuffer[RDP_61_HISTORY_BUFFER_LENGTH];    // Livel-1 history buffer.
    size_t    historyOffset{0};    // Livel-1 history buffer associated history offset.

    rdp_mppc_50_dec level_2_decompressor;

    rdp_mppc_61_dec()
      : historyBuffer{0}
    {}

private:
    static inline void prepare_compressed_data(InStream & compressed_data_stream, bool compressed,
        uint16_t & MatchCount, uint8_t const * & MatchDetails, uint8_t const * & Literals,
        size_t & literals_length)
    {
        if (compressed) {
            unsigned expected = 2; // MatchCount(2)
            if (!compressed_data_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%zu",
                    expected, compressed_data_stream.in_remain());
                throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
            }
            MatchCount = compressed_data_stream.in_uint16_le();

            expected = MatchCount * 8; // MatchCount(2) * (MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4))
            if (!compressed_data_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%zu",
                    expected, compressed_data_stream.in_remain());
                throw Error(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED);
            }
            MatchDetails = compressed_data_stream.get_current();
            compressed_data_stream.in_skip_bytes(expected);
        }
        else {
            MatchCount   = 0;
            MatchDetails = nullptr;
        }

        literals_length = compressed_data_stream.in_remain();
        Literals        = (literals_length ? compressed_data_stream.get_current() : nullptr);
    }

public:
    int decompress(uint8_t const * compressed_data, int compressed_data_size,
        int compressionFlags, const uint8_t *& uncompressed_data, uint32_t & uncompressed_data_size) override
    {
        (void)compressionFlags;
        //LOG(LOG_INFO, "decompress_61: historyOffset=%d compressed_data_size=%d compressionFlags=0x%X",
        //    this->historyOffset, compressed_data_size, compressionFlags);

        uncompressed_data      = nullptr;
        uncompressed_data_size = 0;

        InStream compressed_data_stream(compressed_data, compressed_data_size);

        unsigned expected = 2; // Level1ComprFlags(1) + Level2ComprFlags(1)
        if (!compressed_data_stream.in_check_rem(expected)) {
            LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%zu",
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

        uint8_t const * MatchDetails;
        uint8_t const * Literals;
        uint16_t MatchCount;
        size_t   literals_length;

        if ((Level1ComprFlags & L1_INNER_COMPRESSION) && (Level2ComprFlags & PACKET_COMPRESSED)) {

            const uint8_t * level_1_compressed_data;
            uint32_t        level_1_compressed_data_size;

            bool nResult = this->level_2_decompressor.decompress(compressed_data_stream.get_current(),
                compressed_data_stream.in_remain(), Level2ComprFlags, level_1_compressed_data,
                level_1_compressed_data_size);
            if (!nResult) {
                LOG(LOG_ERR, "RDP 6.1 bluk compression Level-2 decompression error");
                throw Error(ERR_RDP61_DECOMPRESS_LEVEL_2);
            }
            //LOG(LOG_INFO, "level_1_compressed_data_size=%d", level_1_compressed_data_size);

            InStream level_1_compressed_data_stream(level_1_compressed_data, level_1_compressed_data_size);

            prepare_compressed_data(level_1_compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }
        else {
            prepare_compressed_data(compressed_data_stream,
                !(Level1ComprFlags & L1_NO_COMPRESSION),
                MatchCount, MatchDetails, Literals, literals_length);
        }

        //LOG(LOG_INFO, "MatchCount=%d literals_length=%d", MatchCount, literals_length);

        if (Level1ComprFlags & L1_PACKET_AT_FRONT) {
            this->historyOffset = 0;
        }


        InStream match_details_stream(MatchDetails,
            MatchCount *
            8);   // MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4)
        InStream literals_stream(Literals, literals_length);

        uint8_t  * current_output_buffer = this->historyBuffer + this->historyOffset;
        uint16_t   current_output_offset = 0;

        for (uint16_t match_index = 0; match_index < MatchCount; match_index++) {
            expected = 8;   // MatchLength(2) + MatchOutputOffset(2) + MatchHistoryOffset(4)
            if (!match_details_stream.in_check_rem(expected)) {
                LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%zu",
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
                    LOG(LOG_ERR, "RDP61_COMPRESSED_DATA: data truncated, expected=%u remains=%zu",
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
            for (uint16_t i = 0; i < MatchLength; i++, src++, dest++) {
                *dest = *src;
            }

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

        return true;
    }
};


struct rdp_mppc_enc_match_finder
{
    StaticOutStream<65536> match_details_stream;

    virtual ~rdp_mppc_enc_match_finder() = default;

    /**
     * @param historyOffset HistoryOffset before data compression.
     */
    virtual void find_match(const uint8_t * historyBuffer, uint32_t historyOffset, uint16_t uncompressed_data_size) = 0;

    virtual void process_packet_at_front() {}

    virtual bool undo_last_changes() { return true; }
};


struct rdp_mppc_61_enc_sequential_search_match_finder : public rdp_mppc_enc_match_finder
{
    inline void get_match_length(const uint8_t * output_data, uint16_t output_data_size,
        const uint8_t * history_data, uint32_t history_data_size, uint16_t & MatchLength,
        uint32_t & MatchHistoryOffset)
    {
        MatchLength        = 0;
        MatchHistoryOffset = 0;

        if (history_data_size < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH) {
            return;
        }

        assert(output_data_size >= RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH);

        const uint8_t * history_data_begin = history_data;
        const uint8_t * history_data_end   = history_data_begin + history_data_size - (RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1);
        while (history_data_begin != history_data_end) {
            assert(history_data_begin <= history_data_end);
            if (0 != memcmp(output_data, history_data_begin, RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)) {
                history_data_begin++;
                continue;
            }

            uint16_t        CurrentMatchLength = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH;
            const uint8_t * o = output_data + CurrentMatchLength;
            const uint8_t * h = history_data_begin + CurrentMatchLength;
            for (; (o < output_data + output_data_size) && (h < history_data + history_data_size) && (*o == *h);
                o++, h++, CurrentMatchLength++);

            if (CurrentMatchLength > MatchLength) {
                if (MatchLength > 0) {
                    history_data_end = history_data + history_data_size - (CurrentMatchLength - 1);
                }

                MatchLength        = CurrentMatchLength;
                MatchHistoryOffset = history_data_begin - history_data;
            }

            history_data_begin++;
        }
    }

    void find_match(const uint8_t * historyBuffer, uint32_t historyOffset, uint16_t uncompressed_data_size) override
    {
        if (uncompressed_data_size < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH){
            return;
        }

        this->match_details_stream.rewind();

        uint32_t        history_data_size  = historyOffset;
        uint16_t        output_data_length = uncompressed_data_size;
        const uint8_t * output_data_begin  = historyBuffer + historyOffset;
        const uint8_t * output_data_end    = output_data_begin + uncompressed_data_size - (RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1);
        while (output_data_begin < output_data_end) {
            assert(output_data_length >= RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH);

            uint16_t MatchLength;
            uint32_t MatchHistoryOffset;
            this->get_match_length(output_data_begin, output_data_length,
                historyBuffer, history_data_size, MatchLength, MatchHistoryOffset);
            if (MatchLength) {
                assert(MatchLength >= RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH);

                this->match_details_stream.out_uint16_le(MatchLength);
                //LOG(LOG_INFO, "MatchLength=%d", MatchLength);
                this->match_details_stream.out_uint16_le(output_data_begin - (historyBuffer + historyOffset));
                this->match_details_stream.out_uint32_le(MatchHistoryOffset);

                history_data_size  += MatchLength;
                output_data_length -= MatchLength;
                output_data_begin  += MatchLength;
            }
            else {
                history_data_size++;
                output_data_length--;
                output_data_begin++;
            }
        }
    }
};

struct rdp_mppc_61_enc_hash_based_match_finder : public rdp_mppc_enc_match_finder
{
    static const size_t MAXIMUM_HASH_BUFFER_UNDO_ELEMENT = 256;

    using offset_type = uint32_t;
    using hash_table_manager = rdp_mppc_enc_hash_table_manager<offset_type,
                                                               RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH,
                                                               MAXIMUM_HASH_BUFFER_UNDO_ELEMENT>;
    using hash_type = hash_table_manager::hash_type;

    hash_table_manager hash_tab_mgr;

    rdp_mppc_61_enc_hash_based_match_finder() = default;

    void find_match(const uint8_t * historyBuffer, offset_type historyOffset,
        uint16_t uncompressed_data_size) override
    {
        this->match_details_stream.rewind();

        this->hash_tab_mgr.clear_undo_history();

        if (uncompressed_data_size < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH) {
            return;
        }

        offset_type counter = 0;
        // If we are at start of history buffer, do not attempt to compress
        //  first RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1 bytes, because
        //  minimum LoM is RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH.
        if (historyOffset == 0) {
            counter = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1;
            for (offset_type i = 0; i < counter; ++i) {
                this->hash_tab_mgr.update_indirect(historyBuffer, i);
            }
        }

        uint16_t length_of_match = 0;
        // We need at least RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH bytes to look for match
        //  (> sizeof(RDP61_MATCH_DETAILS) = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1).
        for (; counter + (RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1) < uncompressed_data_size;
             counter += length_of_match) {
            offset_type     offset         = historyOffset + counter;
            const uint8_t * data           = historyBuffer + offset;
            hash_type       hash           = this->hash_tab_mgr.sign(data);
            offset_type     previous_match = this->hash_tab_mgr.get_offset(hash);

            this->hash_tab_mgr.update(hash, offset);

            // Check that we have a pattern match, hash is not enough.

            if (0 != ::memcmp(data, historyBuffer + previous_match,
                              RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH)) {
                length_of_match = 1;
            }
            else {
                // We have a match, compute hash and Length of Match for triplets.
                this->hash_tab_mgr.update_indirect(historyBuffer, offset + 1);

                // Maximum LOM is RDP_61_MAX_DATA_BLOCK_SIZE bytes.
                for (length_of_match = RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH;
                     (counter + length_of_match < uncompressed_data_size) &&
                         (length_of_match < RDP_61_MAX_DATA_BLOCK_SIZE);
                     length_of_match++) {
                    this->hash_tab_mgr.update_indirect(historyBuffer, offset + length_of_match - 1);
                    if (historyBuffer[offset + length_of_match] !=
                        historyBuffer[previous_match + length_of_match])
                    {
                        break;
                    }
                }

                this->match_details_stream.out_uint16_le(length_of_match);
                this->match_details_stream.out_uint16_le(counter);
                this->match_details_stream.out_uint32_le(previous_match);
            }
        }
    }

public:
    void process_packet_at_front() override
    {
        this->hash_tab_mgr.reset();
    }

    bool undo_last_changes() override
    {
        return this->hash_tab_mgr.undo_last_changes();
    }
};

template<class MatchFinder>
class rdp_mppc_61_enc : public rdp_mppc_enc {
    static_assert(
        std::is_base_of<rdp_mppc_enc_match_finder, MatchFinder>::value
      , "MatchFinder must be derived from rdp_mppc_enc_match_finder");

    uint8_t    historyBuffer[RDP_61_HISTORY_BUFFER_LENGTH];   // Level-1 history buffer.
    uint32_t   historyOffset{0};   // Level-1 history buffer associated history offset.

    rdp_mppc_50_enc level_2_compressor;

    uint8_t    level_1_output_buffer[RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE];
    uint16_t   level_1_compressed_data_size{0};

    uint8_t level_1_compr_flags_hold{L1_PACKET_AT_FRONT};
    uint8_t Level1ComprFlags{0};
    uint8_t Level2ComprFlags{0};

    uint8_t  * outputBuffer{nullptr};
    uint16_t   bytes_in_output_buffer{0};

    MatchFinder match_finder;

public:
    explicit rdp_mppc_61_enc(bool verbose = false)
        : rdp_mppc_enc(RDP_61_MAX_DATA_BLOCK_SIZE, verbose)
        , historyBuffer{0}
        ,
         level_1_output_buffer{0}

    {}

private:
    void compress_61(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "compress_61: uncompressed_data_size=%" PRIu16 " historyOffset=%" PRIu32,
                uncompressed_data_size, this->historyOffset);
        }

        if (uncompressed_data_size > RDP_61_MAX_DATA_BLOCK_SIZE) {
            LOG(LOG_ERR, "compress_61: input stream too large, max=%zu got=%u",
                RDP_61_MAX_DATA_BLOCK_SIZE, uncompressed_data_size);
            throw Error(ERR_RDP_PROTOCOL);
        }

        this->Level1ComprFlags = L1_INNER_COMPRESSION;
        this->Level2ComprFlags = 0;


        ::memset(this->level_1_output_buffer, 0, RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE);
        this->level_1_compressed_data_size = 0;
        this->outputBuffer                 = nullptr;
        this->bytes_in_output_buffer       = 0;


        if ((uncompressed_data == nullptr) || (uncompressed_data_size == 0) ||
            (uncompressed_data_size > RDP_61_MAX_DATA_BLOCK_SIZE) ||
            (uncompressed_data_size <= 2)) {  // Level1ComprFlags(1) + Level2ComprFlags(1)
            return;
        }

        if ((this->historyOffset + uncompressed_data_size + 1) >= RDP_61_HISTORY_BUFFER_LENGTH) {
            // historyBuffer cannot hold uncompressed_data - rewind it.
            this->level_1_compr_flags_hold |= L1_PACKET_AT_FRONT;
        }

        if (this->level_1_compr_flags_hold & L1_PACKET_AT_FRONT) {
            if (this->verbose) {
                LOG(LOG_INFO, "level_1_compr_flags_hold & L1_PACKET_AT_FRONT");
            }
            this->historyOffset = 0;

            ::memset(this->historyBuffer, 0, RDP_61_HISTORY_BUFFER_LENGTH * sizeof(uint8_t));
            this->match_finder.process_packet_at_front();
        }

        // Add/append new data to historyBuffer.
        ::memcpy(this->historyBuffer + this->historyOffset, uncompressed_data, uncompressed_data_size);

        if (uncompressed_data_size < RDP_61_COMPRESSOR_MINIMUM_MATCH_LENGTH) {
            this->Level1ComprFlags |= L1_NO_COMPRESSION;
        }
        else {
            this->match_finder.find_match(this->historyBuffer, this->historyOffset, uncompressed_data_size);
            OutStream level_1_output_stream(this->level_1_output_buffer, RDP_61_COMPRESSOR_OUTPUT_BUFFER_SIZE);
            uint32_t match_details_data_size = this->match_finder.match_details_stream.get_offset();
            uint32_t MatchCount = (match_details_data_size ?
                                   match_details_data_size / 8 :   // sizeof(RDP61_COMPRESSED_DATA) = 8
                                   0);
            unsigned expected = 2 + match_details_data_size; /* MatchCount(2) + MatchDetails (variable) */
            if (!level_1_output_stream.has_room(expected)) {
                LOG(LOG_ERR, "RDP 6.1 bulk compressor: output stream memory too small, need=%u remains=%zu",
                    expected, level_1_output_stream.tailroom());
                throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
            }
            level_1_output_stream.out_uint16_le(MatchCount);
            level_1_output_stream.out_copy_bytes(this->match_finder.match_details_stream.get_data(),
                match_details_data_size);
            InStream match_details_in_stream(this->match_finder.match_details_stream.get_data(),
                match_details_data_size);
            uint16_t current_output_offset = 0;
            for (uint32_t match_index = 0; match_index < MatchCount; match_index++) {
                uint16_t match_length        = match_details_in_stream.in_uint16_le();
                uint16_t match_output_offset = match_details_in_stream.in_uint16_le();
                match_details_in_stream.in_skip_bytes(4);

                if (match_output_offset > current_output_offset) {
                    expected = match_output_offset - current_output_offset;
                    if (!level_1_output_stream.has_room(expected)) {
                        LOG(LOG_ERR, "RDP 6.1 bulk compressor: output stream memory too small, need=%u remains=%zu",
                            expected, level_1_output_stream.tailroom());
                        throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
                    }
                    level_1_output_stream.out_copy_bytes(uncompressed_data + current_output_offset, expected);
                    current_output_offset = match_output_offset + match_length;
                }
                else {
                    current_output_offset += match_length;
                }
            }
            if (uncompressed_data_size > current_output_offset) {
                expected = uncompressed_data_size - current_output_offset;
                if (!level_1_output_stream.has_room(expected)) {
                    LOG(LOG_ERR, "RDP 6.1 bulk compressor: output stream memory too small, need=%u remains=%zu",
                        expected, level_1_output_stream.tailroom());
                    throw Error(ERR_STREAM_MEMORY_TOO_SMALL);
                }
                level_1_output_stream.out_copy_bytes(uncompressed_data + current_output_offset, expected);
            }
            this->level_1_compressed_data_size = level_1_output_stream.get_offset();


            if (this->level_1_compressed_data_size >= uncompressed_data_size) {
                this->Level1ComprFlags |= L1_NO_COMPRESSION;
            }
            else {
                this->Level1ComprFlags |= L1_COMPRESSED;
            }
        }


        uint16_t compressed_data_size;
        if (this->Level1ComprFlags & L1_NO_COMPRESSION) {
            this->level_2_compressor.compress(uncompressed_data, uncompressed_data_size, Level2ComprFlags,
                compressed_data_size,
                uncompressed_data_size -
                    2); // Level1ComprFlags(1) + Level2ComprFlags(1)

            if (Level2ComprFlags & PACKET_COMPRESSED) {
                this->outputBuffer           = this->level_2_compressor.outputBuffer;
                this->bytes_in_output_buffer = this->level_2_compressor.bytes_in_opb;
            }
            else {
                // Data compression results in an expansion of the data size.
                if (!this->match_finder.undo_last_changes()) {
                    this->level_1_compr_flags_hold |= L1_PACKET_AT_FRONT;
                }
                else {
                    return;
                }
            }
        }
        else {
            this->level_2_compressor.compress(
                this->level_1_output_buffer, this->level_1_compressed_data_size, Level2ComprFlags,
                compressed_data_size, rdp_mppc_enc::MAX_COMPRESSED_DATA_SIZE_UNUSED);

            if (Level2ComprFlags & PACKET_COMPRESSED) {
                this->outputBuffer           = this->level_2_compressor.outputBuffer;
                this->bytes_in_output_buffer = this->level_2_compressor.bytes_in_opb;
            }
            else {
                this->outputBuffer           = this->level_1_output_buffer;
                this->bytes_in_output_buffer = this->level_1_compressed_data_size;
            }
        }

        this->historyOffset += uncompressed_data_size;

        if (this->bytes_in_output_buffer) {
            this->Level1ComprFlags |= this->level_1_compr_flags_hold;

            this->level_1_compr_flags_hold = 0;
        }

        if (this->verbose) {
            LOG(LOG_INFO, "Level1ComprFlags=0x%02X Level2ComprFlags=0x%02X",
                static_cast<unsigned>(this->Level1ComprFlags),
                static_cast<unsigned>(this->Level2ComprFlags));
        }
    }

    void do_compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType , uint16_t & compressed_data_size, uint16_t /*reserved*/) override
    {
        this->compress_61(uncompressed_data, uncompressed_data_size);
        compressedType       = (this->bytes_in_output_buffer) ? (PACKET_COMPRESSED | PACKET_COMPR_TYPE_RDP61) : 0;
        compressed_data_size = (this->bytes_in_output_buffer) ? 2 + // Level1ComprFlags(1) + Level2ComprFlags(1)
                                                              this->bytes_in_output_buffer                    : 0;
    }

public:

    void get_compressed_data(OutStream & stream) const override
    {
        if (stream.tailroom() <
            static_cast<size_t>(2) + // Level1ComprFlags(1) + Level2ComprFlags(1)
                this->bytes_in_output_buffer) {
            LOG(LOG_ERR, "rdp_mppc_61_enc::get_compressed_data: Buffer too small");
            throw Error(ERR_BUFFER_TOO_SMALL);
        }

        stream.out_uint8(this->Level1ComprFlags);
        stream.out_uint8(this->Level2ComprFlags);
        stream.out_copy_bytes(this->outputBuffer, this->bytes_in_output_buffer);
    }
};  // struct rdp_mppc_61_enc

using rdp_mppc_61_enc_hash_based = rdp_mppc_61_enc<rdp_mppc_61_enc_hash_based_match_finder>;
