/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Implementation of Microsoft Point to Point Compression (MPPC) protocol

   Based on code by Laxmikant Rashinkar & Jiten Pathy from FreeRDP project
   Copyright 2011 Laxmikant Rashinkar <LK.Rashinkar@gmail.com>
   Copyright 2012 Jiten Pathy
*/


#pragma once

#include <stdint.h>
#include <cinttypes>

#include "utils/log.hpp"
#include "core/error.hpp"

class OutStream;

// 3.1.8 MPPC-Based Bulk Data Compression
// ======================================

// RDP uses a modified form of the Microsoft Point-to-Point Compression (MPPC)
// Protocol to perform bulk compression of the PDU contents. This protocol is
// described in [RFC2118]. There are two forms of bulk compression used at the
// server and client:

//    RDP 4.0: Based on the original MPPC Protocol, with an 8,192 byte history
// buffer (section 3.1.8.4.1).

//    RDP 5.0: A modified version of RDP 4.0 that uses a 65,536 byte history
// buffer and implements rearranged Huffman style encoding for the bitstream
// formats (section 3.1.8.4.2).

// Both the server and client may operate as the sender of compressed data.
// Server-to-client compression can be used for fast-path output data
// (see section 2.2.9.1.2.1), slow-path output data (see section 2.2.9.1.1)
// or virtual channel data (see section 2.2.6.1). Client-to-server compression
// can currently only be used for virtual channel data.

// The client advertises the maximum compression type it supports in the
// Client Info PDU (see section 2.2.1.11). In response the server selects
// a compression type within the range advertised by the client.
// This compression type is then used when performing all subsequent
// server-to-client and client-to-server bulk compression.

// The compression type usage is indicated on a per-PDU basis by compression
// flags which are set in the header flags associated with each PDU. Besides
// being used to indicate the compression type, the compression flags are also
// used to communicate compression state changes which are required to maintain
// state synchronization. The header used to transmit the compression flags
// will depend on the type of data payload, such as fast-path output data
// (see section 2.2.9.1.2.1), virtual channel data (section 2.2.6.1)
// or slow-path data (section 2.2.9.1.1).

// 3.1.8.1 Abstract Data Model

// The shared state necessary to support the transmission and reception of
// compressed data between a client and server requires a history buffer and
// a current offset into the history buffer (HistoryOffset). The size of the
// history buffer depends on the compression type being used (8 kilobytes for
// RDP 4.0 and 64 kilobytes for RDP 5.0). Any data that is being compressed MUST
// be smaller in size than the history buffer. The HistoryOffset MUST
// start initialized to zero while the history buffer MUST be filled with zeros.
// After it has been initialized, the entire history buffer is immediately
// regarded as valid.

// When compressing data, the sender MUST first check that the uncompressed
// data can be inserted into the history buffer at the position in the history
// buffer given by the HistoryOffset. If the data will not fit into the history
// buffer (the sum of the HistoryOffset and the size of the uncompressed data
// exceeds the size of the history buffer), the HistoryOffset MUST be reset to
// the start of the history buffer (offset 0). If the data will fit into the
// history buffer, the sender endpoint inserts the uncompressed data at the
// position in the history buffer given by the HistoryOffset, and then advances
// the HistoryOffset by the amount of data added.

// As the receiver endpoint decompresses the data, it inserts the decompressed
// data at the position in the history buffer given by its local copy
// HistoryOffset. If a reset occurs, the sender endpoint MUST notify the target
// receiver so it can reset its local state. In this way, the sender and
// receiver endpoints maintain an exact replica of the history buffer and
// HistoryOffset.

// 3.1.8.2 Compressing Data
// ------------------------

// The uncompressed data is first inserted into the local history buffer at the
// position indicated by HistoryOffset by the sender. The compressor then runs
// through the length of newly added uncompressed data to be sent and produces
// as output a sequence of literals (bytes to be sent uncompressed) or
// copy-tuples which consists of a <copy-offset, length-of-match> pair.

// The copy-offset component of the copy-tuple is an index into HistoryBuffer
// (counting backwards from the current byte being compressed in the history
// buffer towards the start of the buffer) where there is a match to the data
// to be sent. The length-of-match component is the length of that match in
// bytes, and MUST be larger than 2 (section 3.1.8.4.1.2.2 and 3.1.8.4.2.2.2).
// If the resulting data is not smaller than the original bytes (that is,
// expansion instead of compression results), then this results in a flush
// and the data is sent uncompressed so as never to send more data than the
// original uncompressed bytes.

// In this way the compressor aims to reduce the size of data that needs to be
// transmitted. For example, consider the following string.

// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!

// The compressor produces the following:

// for.whom.the.bell.tolls,<16,15>.<40,4><19,3>e!

// The <16,15> tuple is the compression of '.the.bell.tolls' and <40,4> is
// 'for.', <19,3> gives 'the'.

// The expansion of a copy-tuple MUST use a "replicating copy". A replicating
// copy is implemented using the following pseudocode.

// SrcPtr = HistoryPtr - CopyOffset;

// while (LengthOfMatch > 0)
// {
//    *HistoryPtr = *SrcPtr;
//
//    SrcPtr = SrcPtr + 1;
//    HistoryPtr = HistoryPtr + 1;
//
//    LengthOfMatch = LengthOfMatch - 1;
// }

// For example, consider the following compressed stream.

// Xcd<2,4>YZ

// Using a replicating copy, this is correctly decompressed to

// XcdcdcdYZ

// Literals and copy-tuples are encoded using the scheme described in section
// 3.1.8.4.1 or 3.1.8.4.2 (the scheme used depends on whether RDP 4.0 or 5.0
// bulk compression is being used).

// 3.1.8.2.1 Setting the Compression Flags

// The sender MUST always specify the compression flags associated with a
// compressed payload. These flags MUST be set in the header field appropriate
// to the type of data payload, such as fast-path output data (see section
// 2.2.9.1.2.1), virtual channel data (see section 2.2.6.1), or slow-path
// output data (see section 2.2.9.1.1).

// The compression flags are produced by performing a logical OR operation of
// the compression type with one or more of the following flags.

//
// +------------------------+--------------------------------------------------+
// | Compression flag       | Meaning                                          |
// +------------------------+--------------------------------------------------+
// | PACKET_COMPRESSED 0x20 | Used to indicate that the data is compressed.    |
// |                        | This flag is equivalent to MPPC bit C (for more  |
// |                        | information see [RFC2118] section 3.1). This flag|
// |                        | MUST be set when compression of the data was     |
// |                        | successful.                                      |
// +------------------------+--------------------------------------------------+
// | PACKET_AT_FRONT 0x40   | Used to indicate that the decompressed data MUST |
// |                        | be placed at the beginning of the local history  |
// |                        | buffer. This flag is equivalent to MPPC bit B    |
// |                        | (for more information see [RFC2118] section 3.1).|
// |                        | This flag MUST be set in conjunction with the    |
// |                        | PACKET_COMPRESSED (0x20) flag. There are two     |
// |                        | conditions on the "compressor-side" that generate|
// |                        | this scenario: (1) this is the first packet to be|
// |                        | compressed, and (2) the data to be compressed    |
// |                        | will not fit at the end of the history buffer but|
// |                        | instead needs to be placed at the start of the   |
// |                        | history buffer.                                  |
// +------------------------+--------------------------------------------------+
// | PACKET_FLUSHED 0x80    | Used to indicate that the decompressor MUST      |
// |                        | reinitialized the history buffer (by filling it  |
// |                        | with zeros) and reset the HistoryOffset to zero. |
// |                        | After it has been reinitialized, the entire      |
// |                        | history buffer is immediately regarded as valid. |
// |                        | This flag is equivalent to MPPC bit A (for more  |
// |                        | information see [RFC2118] section 3.1). If the   |
// |                        | PACKET_COMPRESSED (0x20) flag is also present,   |
// |                        | then the PACKET_FLUSHED flag MUST be processed   |
// |                        | first.                                          |
// +------------------------+--------------------------------------------------+

enum {
    PACKET_COMPRESSED = 0x20,
    PACKET_AT_FRONT   = 0x40,
    PACKET_FLUSHED    = 0x80
};

// Data that is tagged as compressed (using the PACKET_COMPRESSED flag) MUST NOT
// be larger in size than the original data. This implies that in a minority of
// cases it is possible for compressed data to be the same size as the original
// data, and still be regarded as compressed. In effect, the statement that
// "data is compressed" simply implies that the data is encoded using a
// particular scheme, and that a decoder (or decompressor) is required to obtain
// the original data.

// 3.1.8.2.2 Operation of the Bulk Compressor

// The flowchart in the following figure illustrates the general operation of
// the bulk compressor and the production of the compression flags described in
// section 3.1.8.2.1.

// TODO: convert flowchart to pseudocode to insert it here

// The constructs that follow are used throughout the flowchart.

//    Flags: The compression flags.

//    SrcData: The source bytes to be passed to the compressor.

//    HistoryBuffer: The history buffer as described in section 3.1.8.1.

//    HistoryOffset: The current offset into the history buffer as described in section 3.1.8.1.

//    HistoryPtr: A pointer to the current byte in the history buffer which is being encoded.

//    OutputBuffer: The output buffer that will contain the encoded bytes.

// 3.1.8.2.3 Data Compression Example

// This example is based on the flowchart in the preceding figure that describes the operation of the bulk compressor.

// Source Data (ANSI characters):
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!

// HistoryPtr = 0
// HistoryOffset = 0

// (1) Copy the source data to the history buffer.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
// ^ (HistoryPtr = 0)

// HistoryOffset = 49

// Output Buffer:
// <empty>

// (2) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('f') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//  ^ (HistoryPtr = 1)

// Output Buffer:
// f

// (3) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('o') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//   ^ (HistoryPtr = 2)

// Output Buffer:
// fo

// (4) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('r') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//    ^ (HistoryPtr = 3)

// Output Buffer:
// for

// (5) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('.') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//    ^ (HistoryPtr = 4)

// Output Buffer:
// for.

// For the sake of brevity, we skip the next 19 steps where we just add ANSI characters to the output buffer.

// (6) Current value of HistoryPtr is 23. No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr (',') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                        ^ (HistoryPtr = 24)

// Output Buffer:
// for.whom.the.bell.tolls,

// (7) We find a match in the history buffer at position 8 of length 15 characters (".the.bell.tolls"). Encode the copy-tuple and add it to the output buffer and advance HistoryPtr by the size of the match. Recall from section 3.1.8.2 that the copy-offset component of the copy-tuple is an index into HistoryBuffer (counting backwards from the HistoryPtr towards the start of the buffer) where there is a match to the data to be sent.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                       ^ (HistoryPtr = 39)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>

// (8) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('.') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                        ^ (HistoryPtr = 40)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>.

// (9) We find a match in the history buffer at position 0 of length 4 characters ("for."). Encode the copy-tuple and add it to the output buffer and advance HistoryPtr by the size of the match.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                            ^ (HistoryPtr = 44)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>.<40,4>

// (10) We find a match in the history buffer at position 25 of length 3 characters ("the"). Encode the copy-tuple and add it to the output buffer and advance HistoryPtr by the size of the match.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                               ^ (HistoryPtr = 47)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>.<40,4><19,3>

// (11) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('e') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                                ^ (HistoryPtr = 48)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>.<40,4><19,3>e

// (12) No match larger than 2 characters found at the current position. Add the ANSI character at HistoryPtr ('!') to the output buffer and advance HistoryPtr.

// History Buffer:
// 0         1         2         3         4
// 012345678901234567890123456789012345678901234567890
// for.whom.the.bell.tolls,.the.bell.tolls.for.thee!
//                                                 ^ (HistoryPtr = 49)

// Output Buffer:
// for.whom.the.bell.tolls,<16,15>.<40,4><19,3>e!

// (13) HistoryPtr (49) is not less than HistoryOffset (49), so we add the PACKET_COMPRESSED flag to the output packet and send the Output Buffer.

/* Compression Types */

// See core/RDP/logon.hpp for documentation on compresson types


// If neither the CHANNEL_FLAG_FIRST (0x00000001) nor the CHANNEL_FLAG_LAST
//  (0x00000002) flag is present, the chunk is from the middle of a sequence.

//  Instructions specifying how to set the compression flags can be found in
//  section 3.1.8.2.1.

// Possible compression types are as follows.

// +-------------------------+-------------------------------------------------+
// | Value                   | Meaning                                         |
// +-------------------------+-------------------------------------------------+
// | PACKET_COMPR_TYPE_8K    | RDP 4.0 bulk compression (see section           |
// | 0x0                     | 3.1.8.4.1).                                     |
// +-------------------------+-------------------------------------------------+
// | PACKET_COMPR_TYPE_64K   | RDP 5.0 bulk compression (see section           |
// | 0x1                     | 3.1.8.4.2).                                     |
// +-------------------------+-------------------------------------------------+
// | PACKET_COMPR_TYPE_RDP6  | RDP 6.0 bulk compression (see [MS-RDPEGDI]      |
// | 0x2                     | section 3.1.8.1).                               |
// +-------------------------+-------------------------------------------------+
// | PACKET_COMPR_TYPE_RDP61 | RDP 6.1 bulk compression (see [MS-RDPEGDI]      |
// | 0x3                     | section 3.1.8.2).                               |
// +-------------------------+-------------------------------------------------+

//     to get PACKET_COMPR_TYPE: (flags & CompressonTypeMask) >> 9
//     to set PACKET_COMPR_TYPE: (flags | (CompressonTypeMask & (compr_type << 9))
enum {
    PACKET_COMPR_TYPE_8K    = 0x00,
    PACKET_COMPR_TYPE_64K   = 0x01,
    PACKET_COMPR_TYPE_RDP6  = 0x02,
    PACKET_COMPR_TYPE_RDP61 = 0x03
};


struct rdp_mppc_dec {
protected:
    /**
     * Initialize rdp_mppc_dec structure
     */
    rdp_mppc_dec() {}

public:
    /**
     * Deinitialize rdp_mppc_dec structure
     */
    virtual ~rdp_mppc_dec() {}

    virtual int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) = 0;

    virtual void mini_dump() = 0;

    virtual void dump() = 0;
};  // rdp_mppc_dec


////////////////////
//
// Decompressor
//
////////////////////
static inline void insert_n_bits_40_50(uint8_t n, uint32_t data, uint8_t * outputBuffer,
                                       uint8_t & bits_left, uint16_t & opb_index,
                                       const uint16_t outputBufferSize)
{
    uint8_t bits_to_serialize = n;
    if (bits_left > bits_to_serialize) {
        const int i = bits_left - bits_to_serialize;
        outputBuffer[opb_index] |= data << i;
        bits_left = i;
    }
    else {
        if (opb_index + 1 > outputBufferSize) {
            throw Error(ERR_RDP45_COMPRESS_BUFFER_OVERFLOW);
        }
        outputBuffer[opb_index++] |= data >> (bits_to_serialize - bits_left);
        data                      &= (0xFFFFFFFFull >> (32 - (bits_to_serialize - bits_left)));
        bits_to_serialize         -= bits_left;
        for (; bits_to_serialize >= 8 ; bits_to_serialize -= 8) {
            outputBuffer[opb_index++] =  (data >> (bits_to_serialize - 8));
            data                      &= (0xFFFFFFFFull >> (32 - (bits_to_serialize - 8)));
        }
        outputBuffer[opb_index] = (bits_to_serialize > 0)           ?
                                  (data << (8 - bits_to_serialize)) :
                                  0                                 ;
        bits_left               = 8 - bits_to_serialize;
    }
}

static inline void encode_literal_40_50(char c, uint8_t * outputBuffer, uint8_t & bits_left,
                                        uint16_t & opb_index, const uint16_t outputBufferSize)
{
    insert_n_bits_40_50(
        (c & 0x80) ? 9 : 8,
        (c & 0x80) ? (0x02 << 7) | (c & 0x7F) : c,
        outputBuffer, bits_left, opb_index, outputBufferSize);
}

struct rdp_mppc_enc
{
    uint64_t total_uncompressed_data_size;
    uint64_t total_compressed_data_size;
    bool verbose;

protected:
    /**
     * Initialize rdp_mppc_enc structure
     */
    explicit rdp_mppc_enc(bool verbose)
        : total_uncompressed_data_size(0)
        , total_compressed_data_size(0)
        , verbose(verbose)
    {}
    
public:
    /**
     * Deinitialize rdp_mppc_enc structure
     */
    virtual ~rdp_mppc_enc() {}

private:
    virtual void _compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size,
        uint16_t max_compressed_data_size) = 0;

public:
    static const uint16_t MAX_COMPRESSED_DATA_SIZE_UNUSED = static_cast<uint16_t>(-1);

    void compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size,
        uint16_t max_compressed_data_size = MAX_COMPRESSED_DATA_SIZE_UNUSED)
    {
        this->total_uncompressed_data_size += uncompressed_data_size;
        this->_compress(uncompressed_data, uncompressed_data_size,
            compressedType, compressed_data_size, max_compressed_data_size);

        this->total_compressed_data_size +=
            ((compressedType & PACKET_COMPRESSED) ? compressed_data_size :
                uncompressed_data_size);

        if (this->verbose) {
            LOG(LOG_INFO, "compressedType=0x%02X", static_cast<unsigned>(compressedType));
            LOG(LOG_INFO, "uncompressed_data_size=%" PRIu16 " compressed_data_size=%" PRIu16 " rate=%.2f",
                uncompressed_data_size, compressed_data_size,
                compressed_data_size * 100.0 / uncompressed_data_size);
            LOG(LOG_INFO, "total_uncompressed_data_size=%" PRIu64 " total_compressed_data_size=%" PRIu64 " total_rate=%.2Lf",
                total_uncompressed_data_size, total_compressed_data_size,
                static_cast<long double>(total_compressed_data_size) * 100
                / static_cast<long double>(total_uncompressed_data_size));
        }
    }

    virtual void dump(bool mini_dump) const = 0;

    virtual void get_compressed_data(OutStream & stream) const = 0;
};  // struct rdp_mppc_enc


template<typename T> struct rdp_mppc_enc_hash_table_manager {
    static const uint32_t MAX_HASH_TABLE_ELEMENT = 65536;

    T * hash_table;

    uint8_t * undo_buffer_begin;
    uint8_t * undo_buffer_end;
    uint8_t * undo_buffer_current;

    const unsigned int length_of_data_to_sign;

    const unsigned int max_undo_element;
    const unsigned int undo_element_size;

    typedef uint16_t hash_type;

    rdp_mppc_enc_hash_table_manager(unsigned int length_of_data_to_sign, unsigned int max_undo_element)
        : hash_table(nullptr)
        , undo_buffer_begin(nullptr)
        , undo_buffer_end(nullptr)
        , undo_buffer_current(nullptr)
        , length_of_data_to_sign(length_of_data_to_sign)
        , max_undo_element(max_undo_element)
        , undo_element_size(sizeof(hash_type) + sizeof(T))
    {
        this->hash_table = static_cast<T *>(calloc(MAX_HASH_TABLE_ELEMENT, sizeof(T)));

        this->undo_buffer_begin   = static_cast<uint8_t *>(calloc(this->max_undo_element, this->undo_element_size));
        this->undo_buffer_end     = this->undo_buffer_begin + this->max_undo_element * this->undo_element_size;
        this->undo_buffer_current = this->undo_buffer_begin;
    }

    ~rdp_mppc_enc_hash_table_manager() {
        free(this->hash_table);
        free(this->undo_buffer_begin);
    }

    inline void clear_undo_history() {
        this->undo_buffer_current = this->undo_buffer_begin;
    }

    void dump(bool mini_dump) const {
        LOG(LOG_INFO, "Type=RDP X.X bulk compressor hash table manager");
        LOG(LOG_INFO, "hashTable");
        hexdump_d(reinterpret_cast<const char *>(this->hash_table),
            (mini_dump ? 16 : get_table_size()));
    }

    inline T get_offset(hash_type hash) const {
        return this->hash_table[hash];
    }

    constexpr static size_t get_table_size() {
        return MAX_HASH_TABLE_ELEMENT * sizeof(T);
    }

    inline hash_type sign(const uint8_t * data) {
        /* CRC16 defs */
        static const uint16_t crc_table[256] = {
            0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
            0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
            0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
            0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
            0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
            0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
            0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
            0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
            0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
            0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
            0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
            0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
            0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
            0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
            0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
            0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
            0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
            0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
            0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
            0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
            0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
            0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
            0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
            0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
            0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
            0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
            0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
            0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
            0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
            0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
            0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
            0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
        };

        hash_type crc = 0xFFFF;
        for (unsigned int index = 0; index < length_of_data_to_sign; index++)
            crc = (crc >> 8) ^ crc_table[(crc ^ data[index]) & 0xff];
        return crc;
    }

    inline void update(hash_type hash, T offset) {
        if (this->undo_buffer_current != this->undo_buffer_end) {
            *(reinterpret_cast<hash_type *>(this->undo_buffer_current                   )) = hash;
            *(reinterpret_cast<T *>       (this->undo_buffer_current + sizeof(hash_type))) = hash_table[hash];

            this->undo_buffer_current += this->undo_element_size;
        }
        this->hash_table[hash] = offset;
    }

    inline void update_indirect(const uint8_t * data, T offset) {
        hash_type hash = this->sign(data + offset);

        if (this->undo_buffer_current != this->undo_buffer_end) {
            *(reinterpret_cast<hash_type *>(this->undo_buffer_current                   )) = hash;
            *(reinterpret_cast<T *>       (this->undo_buffer_current + sizeof(hash_type))) = hash_table[hash];

            this->undo_buffer_current += this->undo_element_size;
        }
        this->hash_table[hash] = offset;
    }

    inline void reset() {
        ::memset(this->hash_table, 0, get_table_size());

        this->undo_buffer_current = this->undo_buffer_begin;
    }

    inline bool undo_last_changes() {
        if (this->undo_buffer_current == this->undo_buffer_end) {
            return false;
        }

        while (this->undo_buffer_current != this->undo_buffer_begin) {
            this->undo_buffer_current -= this->undo_element_size;

            this->hash_table[*(reinterpret_cast<hash_type *>(this->undo_buffer_current))] =
                *(reinterpret_cast<T *>(this->undo_buffer_current + sizeof(hash_type)));
        }

        return true;
    }
};


static const size_t RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH = 3;
