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
#include "cxx/diagnostic.hpp"
#include "utils/stream.hpp"

#include <limits>
#include <cinttypes>

static const size_t RDP_50_HIST_BUF_LEN = (1024 * 64); /* RDP 5.0 uses 64K history buf */


struct rdp_mppc_50_dec : public rdp_mppc_dec
{
    uint8_t    history_buf[RDP_50_HIST_BUF_LEN];
    uint8_t  * history_buf_end;
    uint8_t  * history_ptr;

    rdp_mppc_50_dec()
    : history_buf{0}
    , history_buf_end(this->history_buf + RDP_50_HIST_BUF_LEN - 1)
    , history_ptr(this->history_buf)
    {}

    /**
     * decompress RDP 5 data
     *
     * @param cbuf    compressed data
     * @param len     length of compressed data
     * @param ctype   compression flags
     * @param roff    starting offset of uncompressed data
     * @param rlen    length of uncompressed data
     *
     * @return        true on success, False on failure
     */
    bool decompress_50(uint8_t const * cbuf, int len, int ctype, uint32_t * roff, uint32_t * rlen)
    {
        //LOG(LOG_INFO, "decompress_50");

        uint8_t  * history_ptr  = this->history_ptr;    /* points to next free slot in bistory_buf    */
        uint32_t   d32          = 0;                    /* we process 4 compressed uint8_ts at a time */
        uint16_t   lom          = 0;                    /* length of match                            */
        uint8_t  * src_ptr      = nullptr;              /* used while copying compressed data         */
        uint8_t const * cptr    = cbuf;                 /* points to next uint8_t in cbuf             */
        uint8_t    cur_uint8_t  = 0;                    /* last uint8_t fetched from cbuf             */
        int        bits_left    = 0;                    /* bits left in d32 for processing            */
        int        cur_bits_left;                       /* bits left in cur_uint8_t for processing    */
        int        tmp;

        *rlen = 0;

        /* get next free slot in history buffer */
        *roff       = history_ptr - this->history_buf;

        if (ctype & PACKET_AT_FRONT) {
            /* place compressed data at start of history buffer */
            history_ptr       = this->history_buf;
            this->history_ptr = this->history_buf;
            *roff             = 0;
        }

        if (ctype & PACKET_FLUSHED) {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP_50_HIST_BUF_LEN);
            *roff = 0;
        }

        if ((ctype & PACKET_COMPRESSED) != PACKET_COMPRESSED) {
            /* data in cbuf is not compressed - copy to history buf as is */
            memcpy(history_ptr, cbuf, len);
            history_ptr       += len;
            *rlen             =  history_ptr - this->history_ptr;
            this->history_ptr =  history_ptr;
            return true;
        }

        /* load initial data */
        tmp = 24;
        while (cptr < cbuf + len) {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       -= 8;
            if (tmp < 0) {
                break;
            }
        }

        if (cptr < cbuf + len) {
            cur_uint8_t   = *cptr++;
            cur_bits_left = 8;
        }
        else {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        while (bits_left >= 8) {
            /*
               value 0xxxxxxx  = literal, not encoded
               value 10xxxxxx  = literal, encoded
               value 11111xxx  = copy offset     0 - 63
               value 11110xxx  = copy offset    64 - 319
               value 1110xxxx  = copy offset   320 - 2367
               value 110xxxxx  = copy offset  2368+
            */

            /*
               at this point, we are guaranteed that d32 has 32 bits to
               be processed, unless we have reached end of cbuf
            */

            uint16_t copy_offset = 0;

            if ((d32 & 0x80000000) == 0) {
                /* got a literal */
                *history_ptr++ =   d32 >> 24;
                d32            <<= 8;
                bits_left      -=  8;
            }
            else if ((d32 & 0xc0000000) == 0x80000000) {
                /* got encoded literal */
                d32            <<= 2;
                *history_ptr++ =   (d32 >> 25) | 0x80;
                d32            <<= 7;
                bits_left      -=  9;
            }
            else if ((d32 & 0xf8000000) == 0xf8000000) {
                /* got copy offset in range 0 - 63, */
                /* with 6 bit copy offset */
                d32         <<= 5;
                copy_offset =   d32 >>  26;
                d32         <<= 6;
                bits_left   -=  11;
            }
            else if ((d32 & 0xf8000000) == 0xf0000000) {
                /* got copy offset in range 64 - 319, */
                /* with 8 bit copy offset */
                d32         <<= 5;
                copy_offset =   d32 >> 24;
                copy_offset +=  64;
                d32         <<= 8;
                bits_left   -=  13;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000) {
                /* got copy offset in range 320 - 2367, */
                /* with 11 bits copy offset */
                d32         <<= 4;
                copy_offset =   d32 >> 21;
                copy_offset +=  320;
                d32         <<= 11;
                bits_left   -=  15;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000) {
                /* got copy offset in range 2368+, */
                /* with 16 bits copy offset */
                d32         <<= 3;
                copy_offset =   d32 >> 16;
                copy_offset +=  2368;
                d32         <<= 16;
                bits_left   -=  19;
            }

            /*
            ** get more bits before we process length of match
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;

            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    if (cur_bits_left) {
                        uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                        d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                        bits_left += cur_bits_left;
                        tmp       -= cur_bits_left;
                    }
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp) {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len) {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset) {
                continue;
            }

            /*
            ** compute Length of Match
            */

            /*
               lengh of match  Encoding (binary header + LoM bits
               --------------  ----------------------------------
               3               0
               4..7            10 + 2 lower bits of LoM
               8..15           110 + 3 lower bits of LoM
               16..31          1110 + 4 lower bits of LoM
               32..63          1111-0 + 5 lower bits of LoM
               64..127         1111-10 + 6 lower bits of LoM
               128..255        1111-110 + 7 lower bits of LoM
               256..511        1111-1110 + 8 lower bits of LoM
               512..1023       1111-1111-0 + 9 lower bits of LoM
               1024..2047      1111-1111-10 + 10 lower bits of LoM
               2048..4095      1111-1111-110 + 11 lower bits of LoM
               4096..8191      1111-1111-1110 + 12 lower bits of LoM
               8192..16383     1111-1111-1111-0 + 13 lower bits of LoM
               16384..32767    1111-1111-1111-10 + 14 lower bits of LoM
               32768..65535    1111-1111-1111-110 + 15 lower bits of LoM
            */

            if ((d32 & 0x80000000) == 0) {
                /* lom is fixed to 3 */
                lom       =   3;
                d32       <<= 1;
                bits_left -=  1;
            }
            else if ((d32 & 0xc0000000) == 0x80000000) {
                /* 2 lower bits of LoM */
                lom       =   ((d32 >> 28) & 0x03) + 4;
                d32       <<= 4;
                bits_left -=  4;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000) {
                /* 3 lower bits of LoM */
                lom       =   ((d32 >> 26) & 0x07) + 8;
                d32       <<= 6;
                bits_left -=  6;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000) {
                /* 4 lower bits of LoM */
                lom       =   ((d32 >> 24) & 0x0f) + 16;
                d32       <<= 8;
                bits_left -=  8;
            }
            else if ((d32 & 0xf8000000) == 0xf0000000) {
                /* 5 lower bits of LoM */
                lom       =   ((d32 >> 22) & 0x1f) + 32;
                d32       <<= 10;
                bits_left -=  10;
            }
            else if ((d32 & 0xfc000000) == 0xf8000000) {
                /* 6 lower bits of LoM */
                lom       =   ((d32 >> 20) & 0x3f) + 64;
                d32       <<= 12;
                bits_left -=  12;
            }
            else if ((d32 & 0xfe000000) == 0xfc000000) {
                /* 7 lower bits of LoM */
                lom       =   ((d32 >> 18) & 0x7f) + 128;
                d32       <<= 14;
                bits_left -=  14;
            }
            else if ((d32 & 0xff000000) == 0xfe000000) {
                /* 8 lower bits of LoM */
                lom       =   ((d32 >> 16) & 0xff) + 256;
                d32       <<= 16;
                bits_left -=  16;
            }
            else if ((d32 & 0xff800000) == 0xff000000) {
                /* 9 lower bits of LoM */
                lom       =   ((d32 >> 14) & 0x1ff) + 512;
                d32       <<= 18;
                bits_left -=  18;
            }
            else if ((d32 & 0xffc00000) == 0xff800000) {
                /* 10 lower bits of LoM */
                lom       =   ((d32 >> 12) & 0x3ff) + 1024;
                d32       <<= 20;
                bits_left -=  20;
            }
            else if ((d32 & 0xffe00000) == 0xffc00000) {
                /* 11 lower bits of LoM */
                lom       =   ((d32 >> 10) & 0x7ff) + 2048;
                d32       <<= 22;
                bits_left -=  22;
            }
            else if ((d32 & 0xfff00000) == 0xffe00000) {
                /* 12 lower bits of LoM */
                lom       =   ((d32 >> 8) & 0xfff) + 4096;
                d32       <<= 24;
                bits_left -=  24;
            }
            else if ((d32 & 0xfff80000) == 0xfff00000) {
                /* 13 lower bits of LoM */
                lom       =   ((d32 >> 6) & 0x1fff) + 8192;
                d32       <<= 26;
                bits_left -=  26;
            }
            else if ((d32 & 0xfffc0000) == 0xfff80000) {
                /* 14 lower bits of LoM */
                lom       =   ((d32 >> 4) & 0x3fff) + 16384;
                d32       <<= 28;
                bits_left -=  28;
            }
            else if ((d32 & 0xfffe0000) == 0xfffc0000) {
                /* 15 lower bits of LoM */
                lom       =   ((d32 >> 2) & 0x7fff) + 32768;
                d32       <<= 30;
                bits_left -=  30;
            }

            /* now that we have copy_offset and LoM, process them */

            src_ptr = history_ptr - copy_offset;
            if (src_ptr >= this->history_buf) {
                /* data does not wrap around */
                while (lom > 0) {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }
            else {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (lom && (src_ptr <= this->history_buf_end)) {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }

                src_ptr = this->history_buf;
                while (lom > 0) {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }

            /*
            ** get more bits before we restart the loop
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;

            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    if (cur_bits_left)
                    {
                        uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                        d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                        bits_left += cur_bits_left;
                        tmp       -= cur_bits_left;
                    }
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp) {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len) {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

        } /* end while (cptr < cbuf + len) */

        *rlen = history_ptr - this->history_ptr;

        this->history_ptr = history_ptr;

        return true;
    }   // decompress_50

    int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) override
    {
        uint32_t roff   = 0;
        int      result;

        rlen   = 0;
        result = this->decompress_50(cbuf, len, ctype, &roff, &rlen);
        rdata  = this->history_buf + roff;

        return result;
    }
};  // struct rdp_mppc_50_dec


struct rdp_mppc_50_enc : public rdp_mppc_enc
{
    static const size_t MAXIMUM_HASH_BUFFER_UNDO_ELEMENT = 256;

    using offset_type = uint16_t;
    using hash_table_manager = rdp_mppc_enc_hash_table_manager<offset_type,
                                                               RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH,
                                                               MAXIMUM_HASH_BUFFER_UNDO_ELEMENT>;
    using hash_type = hash_table_manager::hash_type;

    // TODO making it static and large enough should be good for both RDP4 and RDP5
    uint8_t    historyBuffer[RDP_50_HIST_BUF_LEN];       /* contains uncompressed data */
    uint8_t  * outputBuffer;        /* contains compressed data */
    uint8_t    outputBufferPlus[RDP_50_HIST_BUF_LEN + 64 + 8];
    uint16_t   outputBufferSize;
    uint16_t   historyOffset{0};       /* next free slot in historyBuffer */
    uint16_t   bytes_in_opb{0};        /* compressed bytes available in outputBuffer */
    uint8_t    flags{0};               /* PACKET_COMPRESSED, PACKET_AT_FRONT, PACKET_FLUSHED etc */
    uint8_t    flagsHold{0};
    bool       first_pkt{true};           /* this is the first pkt passing through enc */

    hash_table_manager hash_tab_mgr;

    explicit rdp_mppc_50_enc(bool verbose = 0)
        : rdp_mppc_enc(RDP_50_HIST_BUF_LEN - 1, verbose)
        , historyBuffer{0}
        , outputBuffer(this->outputBufferPlus + 64)  /* contains compressed data */
        , outputBufferPlus{0}
        , outputBufferSize(RDP_50_HIST_BUF_LEN - 1)
    {}

// 3.1.8.4.2 RDP 5.0
// =================

// The rules for RDP 5.0 are very similar to those of RDP 4.0 (section 3.1.8.4.1).
// RDP 5.0 has a history buffer size of 64 kilobytes, thus both endpoints MUST
// maintain a 64 kilobyte window.

// 3.1.8.4.2.1 Literal Encoding
// ----------------------------

// Literals are bytes sent uncompressed. If the value of a literal is below
// 0x80, it is not encoded in any special manner. If the literal has a value
// greater than 0x7F it is sent as the bits 10 followed by the lower 7 bits of
// the literal. For example, 0x56 is transmitted as the binary value 01010110,
// while 0xE7 is transmitted as the binary value 101100111

// 3.1.8.4.2.2 Copy-Tuple Encoding
// -------------------------------

// Copy-tuples consist of a <copy-offset> and <length-of-match> pair
// (see section 3.1.8.2 for more details).


// 3.1.8.4.2.2.1 Copy-Offset Encoding

// Encoding of the copy-offset value is performed according to the following table.

// Copy-offset range   | Encoding (binary header + copy-offset bits)
//     0...63          | 11111 + lower 6 bits of copy-offset
//   64...319          | 11110 + lower 8 bits of (copy-offset – 64)
// 320...2367          | 1110 + lower 11 bits of (copy-offset – 320)
// 2368+               | 110 + lower 16 bits of (copy-offset – 2368)

// A copy-offset value MUST be followed by a length-of-match value.

// 3.1.8.4.2.2.2 Length-of-Match Encoding

// Encoding of the length-of-match (L-o-M) value is performed according
// to the following table.

// L-o-M range  | Encoding (binary header + L-o-M bits)
// -------------+----------------------------------------
// 3            | 0
// 4..7         | 10 + 2 lower bits of L-o-M
// 8..15        | 110 + 3 lower bits of L-o-M
// 16..31       | 1110 + 4 lower bits of L-o-M
// 32..63       | 11110 + 5 lower bits of L-o-M
// 64..127      | 111110 + 6 lower bits of L-o-M
// 128..255     | 1111110 + 7 lower bits of L-o-M
// 256..511     | 11111110 + 8 lower bits of L-o-M
// 512..1023    | 111111110 + 9 lower bits of L-o-M
// 1024..2047   | 1111111110 + 10 lower bits of L-o-M
// 2048..4095   | 11111111110 + 11 lower bits of L-o-M
// 4096..8191   | 111111111110 + 12 lower bits of L-o-M
// 8192..16383  | 1111111111110 + 13 lower bits of L-o-M
// 16384..32767 | 11111111111110 + 14 lower bits of L-o-M
// 32768..65535 | 111111111111110 + 15 lower bits of L-o-M

private:
    /**
     * encode (compress) data using RDP 5.0 protocol using hash table
     *
     * @param uncompressed_data      uncompressed data
     * @param uncompressed_data_size length of uncompressed_data
     */
    void compress_50(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint16_t max_compressed_data_size)
    {
        if (this->verbose) {
            LOG(LOG_INFO, "compress_50");
        }

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wtype-limits")
        static_assert(std::numeric_limits<decltype(uncompressed_data_size)>::max() < RDP_50_HIST_BUF_LEN,
          "LOG(LOG_ERR, \"compress_50: input stream too large, max=%zu got=%u\","
          "\nRDP_50_HIST_BUF_LEN - 1u, uncompressed_data_size)");
        REDEMPTION_DIAGNOSTIC_POP

        this->flags = PACKET_COMPR_TYPE_64K;

        this->hash_tab_mgr.clear_undo_history();

        if ((uncompressed_data == nullptr) || (uncompressed_data_size <= 0) ||
            (uncompressed_data_size >= RDP_50_HIST_BUF_LEN - 2)) {
            return;
        }

        uint16_t opb_index = 0; /* index into outputBuffer                        */
        uint8_t  bits_left = 8; /* unused bits in current uint8_t in outputBuffer */

        ::memset(this->outputBuffer, 0, uncompressed_data_size);

        if (this->first_pkt) {
            this->first_pkt =  false;
            this->flagsHold |= PACKET_AT_FRONT;
        }

        if ((this->historyOffset + uncompressed_data_size + 2) >= static_cast<ssize_t>(RDP_50_HIST_BUF_LEN)) {
            /* historyBuffer cannot hold uncompressed_data - rewind it */
            this->historyOffset =  0;
            this->flagsHold     |= PACKET_AT_FRONT;
            this->hash_tab_mgr.reset();
        }

        /* add/append new data to historyBuffer */
        memcpy(this->historyBuffer + this->historyOffset, uncompressed_data, uncompressed_data_size);

        /* if we are at start of history buffer, do not attempt to compress    */
        /*  first RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1 bytes, because */
        /*  minimum LoM is RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH           */
        try {
            offset_type ctr = 0;

            if (this->historyOffset == 0) {
                /* encode first two bytes as literals */
                ctr = RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1;
                for (offset_type i = 0; i < ctr; ++i) {
                    ::encode_literal_40_50(this->historyBuffer[this->historyOffset + i],
                                           this->outputBuffer, bits_left, opb_index,
                                           this->outputBufferSize);
                }

                this->hash_tab_mgr.update_indirect(this->historyBuffer, 0);
                this->hash_tab_mgr.update_indirect(this->historyBuffer, 1);
            }

            uint16_t lom = 0;
            for (; ctr + (RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH - 1) < uncompressed_data_size;
                 ctr += lom) { // we need at least 3 bytes to look for match
                offset_type     offset         = this->historyOffset + ctr;
                const uint8_t * data           = this->historyBuffer + offset;
                hash_type       hash           = this->hash_tab_mgr.sign(data);
                offset_type     previous_match = this->hash_tab_mgr.get_offset(hash);

                this->hash_tab_mgr.update(hash, offset);

                /* check that we have a pattern match, hash is not enough */
                if (0 != ::memcmp(data, this->historyBuffer + previous_match,
                                  RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH)) {
                    /* no match found; encode literal uint8_t */
                    ::encode_literal_40_50(*data, this->outputBuffer, bits_left,
                                           opb_index, this->outputBufferSize);
                    lom = 1;
                }
                else {
                    /* we have a match - compute hash and Length of Match for triplets */
                    this->hash_tab_mgr.update_indirect(this->historyBuffer, offset + 1);

                    for (lom = RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH; ctr + lom < uncompressed_data_size; lom++) {
                        this->hash_tab_mgr.update_indirect(this->historyBuffer, offset + lom - 1);
                        if (this->historyBuffer[offset + lom] !=
                            this->historyBuffer[previous_match + lom]) {
                            break;
                        }
                    }

                    /* encode copy_offset and insert into output buffer */
                    offset_type copy_offset = offset - previous_match;
                    const int nbbits[4]   = { 11, 13, 15, 19 };
                    const int headers[4]  = { 0x7c0, 0x1e00, 0x7000, 0x060000 };
                    const int base[4]     = { 0, 0x40, 0x140, 0x940 };
                    int       range       = (copy_offset <= 0x3F)  ? 0 :
                        (copy_offset <= 0x13F) ? 1 :
                        (copy_offset <= 0x93F) ? 2 :
                        3 ;
                    ::insert_n_bits_40_50(nbbits[range],
                                          headers[range] | (copy_offset - base[range]),
                                          this->outputBuffer, bits_left, opb_index,
                                          this->outputBufferSize);

                    int log_lom = 31 - __builtin_clz(lom);

                    /* encode length of match and insert into output buffer */
                    ::insert_n_bits_40_50((lom == 3) ? 1 : 2 * log_lom,
                                          (lom == 3) ? 0 : (((((1 << log_lom) - 1) & 0xFFFE) << log_lom) | (lom - (1 << log_lom))),
                                          this->outputBuffer, bits_left, opb_index,
                                          this->outputBufferSize);
                }
            }

            /* add remaining data if any to the output */
            while (uncompressed_data_size - ctr > 0) {
                ::encode_literal_40_50(uncompressed_data[ctr], this->outputBuffer,
                                       bits_left, opb_index, this->outputBufferSize);
                ++ctr;
            }
        }
        catch (Error const &) {
            opb_index = uncompressed_data_size;
            LOG(LOG_INFO, "This happens if chicks have theeth");
            LOG(LOG_INFO, "MPPC 50 ENCODING => BUFFER OVERFLOW !!!");
        }
        if (opb_index >= std::min<uint16_t>(uncompressed_data_size, max_compressed_data_size)) {
            /* compressed data longer or same size than uncompressed data */
            /* give up */
            if (!this->hash_tab_mgr.undo_last_changes()) {
                this->historyOffset =  0;
                this->flagsHold     |= PACKET_FLUSHED;
                this->first_pkt     =  true;

                this->hash_tab_mgr.reset();

                if (this->verbose) {
                    LOG(LOG_INFO, "Unable to undo changes made in hash table.");
                }
            }
            return;
        }

        this->historyOffset += uncompressed_data_size;
        this->flags         |= PACKET_COMPRESSED;
        /* if bits_left == 8, opb_index has already been incremented */
        this->bytes_in_opb  =  opb_index + (bits_left != 8);
        this->flags         |= this->flagsHold;
        this->flagsHold     =  0;
    }

    void do_compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
        uint8_t & compressedType, uint16_t & compressed_data_size,
        uint16_t max_compressed_data_size) override
    {
        this->compress_50(uncompressed_data, uncompressed_data_size, max_compressed_data_size);
        compressedType       = (this->flags & PACKET_COMPRESSED) ? this->flags        : 0;
        compressed_data_size = (this->flags & PACKET_COMPRESSED) ? this->bytes_in_opb : 0;
    }

public:
    void get_compressed_data(OutStream & stream) const override
    {
        if (stream.tailroom() < static_cast<size_t>(this->bytes_in_opb)) {
            LOG(LOG_ERR, "rdp_mppc_50_enc::get_compressed_data: Buffer too small");
            throw Error(ERR_BUFFER_TOO_SMALL);
        }

        stream.out_copy_bytes(this->outputBuffer, this->bytes_in_opb);
    }
};  // struct rdp_mppc_50_enc
