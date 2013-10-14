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

#ifndef _REDEMPTION_CORE_RDP_MPPC_50_HPP_
#define _REDEMPTION_CORE_RDP_MPPC_50_HPP_

#define RDP_50_HIST_BUF_LEN (1024 * 64) /* RDP 5.0 uses 64K history buf */

struct rdp_mppc_50_enc : public rdp_mppc_enc {
    char     * historyBuffer;       /* contains uncompressed data */
    char     * outputBuffer;        /* contains compressed data */
    char     * outputBufferPlus;
    int        historyOffset;       /* next free slot in historyBuffer */
    int        buf_len;             /* length of historyBuffer, protocol dependant */
    int        bytes_in_opb;        /* compressed bytes available in outputBuffer */
    int        flags;               /* PACKET_COMPRESSED, PACKET_AT_FRONT, PACKET_FLUSHED etc */
    int        flagsHold;
    int        first_pkt;           /* this is the first pkt passing through enc */
    uint16_t * hash_table;

    /**
     * Initialize mppc_enc structure
     *
     * @param   protocol_type   PROTO_RDP_40 or PROTO_RDP_50
     */
    rdp_mppc_50_enc() : rdp_mppc_enc()
    {
        this->historyBuffer     = NULL; /* contains uncompressed data */
        this->outputBuffer      = NULL; /* contains compressed data */
        this->outputBufferPlus  = NULL;
        this->historyOffset     = 0;    /* next free slot in historyBuffer */
        this->buf_len           = 0;    /* length of historyBuffer, protocol dependant */
        this->bytes_in_opb      = 0;    /* compressed bytes available in outputBuffer */
        this->flags             = 0;    /* PACKET_COMPRESSED, PACKET_AT_FRONT, PACKET_FLUSHED etc */
        this->flagsHold         = 0;
        this->first_pkt         = 0;    /* this is the first pkt passing through enc */
        this->hash_table        = NULL;

        this->buf_len = RDP_50_HIST_BUF_LEN;

        this->first_pkt = 1;
        this->historyBuffer = static_cast<char*>(malloc(this->buf_len));
        TODO("making it static and large enough should be good for both RDP4 and RDP5");
        memset(this->historyBuffer, 0, this->buf_len);

        this->outputBufferPlus = static_cast<char*>(malloc(this->buf_len + 64 + 8));
        memset(this->outputBufferPlus, 0, this->buf_len + 64);

        this->outputBuffer = this->outputBufferPlus + 64;
        this->hash_table = static_cast<uint16_t*>(malloc(HASH_BUF_LEN * 2));
        memset(this->hash_table, 0, HASH_BUF_LEN * 2);
    }

    /**
     * deinit mppc_enc structure
     *
     * @param   enc  struct to be deinited
     */
    ~rdp_mppc_50_enc()
    {
        free(this->historyBuffer);
        free(this->outputBufferPlus);
        free(this->hash_table);
    }

    void mini_dump() {
        LOG(LOG_INFO, "historyBuffer");
        hexdump_d(this->historyBuffer,         16);
        LOG(LOG_INFO, "outputBuffer");
        hexdump_d(this->outputBuffer,          16);
        LOG(LOG_INFO, "outputBufferPlus");
        hexdump_d(this->outputBufferPlus,      16);
        LOG(LOG_INFO, "historyOffset=%d",      this->historyOffset);
        LOG(LOG_INFO, "buf_len=%d",            this->buf_len);
        LOG(LOG_INFO, "bytes_in_opb=%d",       this->bytes_in_opb);
        LOG(LOG_INFO, "flags=0x%X",              this->flags);
        LOG(LOG_INFO, "flagsHold=0x%X",          this->flagsHold);
        LOG(LOG_INFO, "first_pkt=%d",          this->first_pkt);
        LOG(LOG_INFO, "hash_table");
        hexdump_d((uint8_t *)this->hash_table, 16);
    }

    void dump() {
        LOG(LOG_INFO, "historyBuffer");
        hexdump_d(this->historyBuffer,         this->buf_len);
        LOG(LOG_INFO, "outputBufferPlus");
        hexdump_d(this->outputBufferPlus,      this->buf_len + 64);
        LOG(LOG_INFO, "historyOffset=%d",      this->historyOffset);
        LOG(LOG_INFO, "buf_len=%d",            this->buf_len);
        LOG(LOG_INFO, "bytes_in_opb=%d",       this->bytes_in_opb);
        LOG(LOG_INFO, "flags=0x%X",              this->flags);
        LOG(LOG_INFO, "flagsHold=0x%X",          this->flagsHold);
        LOG(LOG_INFO, "first_pkt=%d",          this->first_pkt);
        LOG(LOG_INFO, "hash_table");
        hexdump_d((uint8_t *)this->hash_table, /*this->buf_len*/RDP_50_HIST_BUF_LEN * 2);
    }

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

    /**
     * encode (compress) data using RDP 5.0 protocol using hash table
     *
     * @param   srcData       uncompressed data
     * @param   len           length of srcData
     *
     * @return  true on success, false on failure
     */
    bool compress_50(uint8_t * srcData, int len)
    {
//        LOG(LOG_INFO, "compress_rdp_5");

        if ((srcData == NULL) || (len <= 0) || (len > this->buf_len))
            return false;

        int        opb_index    = 0;                    /* index into outputBuffer                        */
        int        bits_left    = 8;                    /* unused bits in current uint8_t in outputBuffer */
        uint16_t * hash_table   = this->hash_table;     /* hash table for pattern matching                */
        char     * outputBuffer = this->outputBuffer;   /* points to this->outputBuffer                   */
        memset(outputBuffer, 0, len);

        this->flags = PACKET_COMPR_TYPE_64K;

        if (this->first_pkt) {
            this->first_pkt =  0;
            this->flagsHold |= PACKET_AT_FRONT;
        }

        if ((this->historyOffset + len + 2) >= this->buf_len)
        {
            /* historyBuffer cannot hold srcData - rewind it */
            this->historyOffset =  0;
            this->flagsHold     |= PACKET_AT_FRONT;
            memset(hash_table, 0, this->buf_len * 2);
        }

        /* add / append new data to historyBuffer */
        memcpy(&(this->historyBuffer[this->historyOffset]), srcData, len);

        int      ctr         = 0;
        uint32_t copy_offset = 0; /* pattern match starts here... */

        /* if we are at start of history buffer, do not attempt to compress */
        /* first 2 bytes,because minimum LoM is 3                           */
        if (this->historyOffset == 0) {
            /* encode first two bytes as literals */
            for (int x = 0; x < 2; x++) {
                rdp_mppc_enc::encode_literal(
                    this->historyBuffer[this->historyOffset + x],
                    outputBuffer, bits_left, opb_index);
            }

            hash_table[rdp_mppc_enc::signature(this->historyBuffer)]     = 0;
            hash_table[rdp_mppc_enc::signature(this->historyBuffer + 1)] = 1;
            ctr                                            = 2;
        }

        int lom = 0;
        for (; ctr + 2 < len; ctr += lom) { // we need at least 3 bytes to look for match
            uint32_t crc2           =
                rdp_mppc_enc::signature(this->historyBuffer + this->historyOffset + ctr);
            int      previous_match = hash_table[crc2];
            hash_table[crc2] = this->historyOffset + ctr;

            /* check that we have a pattern match, hash is not enough */
            if (0 != memcmp(this->historyBuffer + this->historyOffset + ctr,
                            this->historyBuffer + previous_match, 3)) {
                /* no match found; encode literal uint8_t */
                rdp_mppc_enc::encode_literal(
                    this->historyBuffer[this->historyOffset + ctr],
                    outputBuffer, bits_left, opb_index);
                lom = 1;
            }
            else {
                /* we have a match - compute hash and Length of Match for triplets */
                hash_table[rdp_mppc_enc::signature(this->historyBuffer + this->historyOffset + ctr + 1)] =
                    this->historyOffset + ctr + 1;
                for (lom = 3; ctr + lom < len; lom++) {
                    hash_table[rdp_mppc_enc::signature(this->historyBuffer + this->historyOffset + ctr + lom - 1)] =
                        this->historyOffset + ctr + lom - 1;
                    if (this->historyBuffer[this->historyOffset + ctr + lom] !=
                        this->historyBuffer[previous_match + lom]) {
                        break;
                    }
                }

                /* encode copy_offset and insert into output buffer */
                copy_offset = this->historyOffset + ctr - previous_match;
                const int nbbits[4]  = { 11, 13, 15, 19 };
                const int headers[4] = { 0x7c0, 0x1e00, 0x7000, 0x060000 };
                const int base[4]    = { 0, 0x40, 0x140, 0x940 };
                int range = (copy_offset <= 0x3F)  ? 0 :
                            (copy_offset <= 0x13F) ? 1 :
                            (copy_offset <= 0x93F) ? 2 :
                                                     3 ;
                rdp_mppc_enc::insert_n_bits(nbbits[range],
                    headers[range]|(copy_offset - base[range]),
                    outputBuffer, bits_left, opb_index);

                int log_lom = 31 - __builtin_clz(lom);

                /* encode length of match and insert into output buffer */
                rdp_mppc_enc::insert_n_bits(
                    (lom == 3) ? 1 : 2 * log_lom,
                    (lom == 3) ? 0 : (((((1 << log_lom) - 1) & 0xFFFE) << log_lom) | (lom - (1 << log_lom))),
                    outputBuffer, bits_left, opb_index);
            }
        }

        /* add remaining data if any to the output */
        while (len - ctr > 0) {
            rdp_mppc_enc::encode_literal(srcData[ctr], outputBuffer, bits_left,
                opb_index);
            ctr++;
        }

        if (opb_index >= len)
        {
            /* compressed data longer or same size than uncompressed data */
            /* give up */
            this->historyOffset = 0;
            memset(hash_table, 0, this->buf_len * 2);
            this->flagsHold |= PACKET_FLUSHED;
            this->first_pkt =  1;
            return true;
        }

        this->historyOffset += len;
        this->flags         |= PACKET_COMPRESSED;
        /* if bits_left == 8, opb_index has already been incremented */
        this->bytes_in_opb =  opb_index + (bits_left != 8);
        this->flags        |= this->flagsHold;
        this->flagsHold    =  0;

        return true;
    }

    virtual bool compress(uint8_t * srcData, int len, uint8_t & flags, uint16_t & compressedLength)
    {
        bool compress_result = this->compress_50(srcData, len);
        if (this->flags & PACKET_COMPRESSED) {
            flags            = this->flags;
            compressedLength = this->bytes_in_opb;
        }
        else {
            flags            = 0;
            compressedLength = 0;
        }
        return compress_result;
    }

    virtual void get_compressed_data(Stream & stream) const
    {
        if (stream.room() < static_cast<size_t>(this->bytes_in_opb))
        {
            LOG(LOG_ERR, "rdp_mppc_50_enc::get_compressed_data: Buffer too small");
            throw Error(ERR_BUFFER_TOO_SMALL);
        }

        stream.out_copy_bytes(this->outputBuffer, this->bytes_in_opb);
    }
};


#endif  // #ifndef _REDEMPTION_CORE_RDP_MPPC_50_HPP_