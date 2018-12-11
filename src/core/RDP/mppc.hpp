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

#include <memory>

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


struct rdp_mppc_dec
{
protected:
    explicit rdp_mppc_dec() = default;

public:
    virtual ~rdp_mppc_dec() = default;

    virtual int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) = 0;
};


class rdp_mppc_enc
{
    uint64_t total_uncompressed_data_size;
    uint64_t total_compressed_data_size;

    uint32_t max_data_block_size;

protected:
    const bool verbose;

    explicit rdp_mppc_enc(uint32_t max_data_block_size, bool verbose);

public:
    virtual ~rdp_mppc_enc() = default;

    static const uint16_t MAX_COMPRESSED_DATA_SIZE_UNUSED = static_cast<uint16_t>(-1);

    void compress(
        const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size,
        uint16_t max_compressed_data_size = MAX_COMPRESSED_DATA_SIZE_UNUSED);

    virtual void get_compressed_data(OutStream & stream) const = 0;

    uint32_t get_max_data_block_size() const;

private:
    virtual void do_compress(
        const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size,
        uint16_t max_compressed_data_size) = 0;
};

// mppc 40, 50, 60 or 61 (lazy loading)
class rdp_mppc_unified_dec : public rdp_mppc_dec
{
    rdp_mppc_dec * mppc_dec = nullptr;

public:
    rdp_mppc_unified_dec() = default;
    ~rdp_mppc_unified_dec() override;

    int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) override;
};

std::unique_ptr<rdp_mppc_enc>
rdp_mppc_load_compressor(bool log_trace, char const * trace, int type, bool verbose);
