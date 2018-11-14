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

#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "core/error.hpp"
#include "utils/sugar/byte_ptr.hpp"

#include <memory>
#include <cstring>
#include <cstdlib>

////////////////////
//
// Decompressor
//
////////////////////
static inline void insert_n_bits_40_50(
    uint8_t n, uint32_t data, uint8_t * outputBuffer,
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

static inline void encode_literal_40_50(
    char c, uint8_t * outputBuffer, uint8_t & bits_left,
    uint16_t & opb_index, const uint16_t outputBufferSize)
{
    insert_n_bits_40_50(
        (c & 0x80) ? 9 : 8,
        (c & 0x80) ? (0x02 << 7) | (c & 0x7F) : c,
        outputBuffer, bits_left, opb_index, outputBufferSize);
}

template<typename T>
class rdp_mppc_enc_hash_table_manager
{
    static const uint32_t MAX_HASH_TABLE_ELEMENT = 65536;

public:
    std::unique_ptr<T[]> hash_table;

private:
    std::unique_ptr<uint8_t[]> undo_buffer_begin;
    uint8_t * undo_buffer_end = nullptr;
    uint8_t * undo_buffer_current = nullptr;

    const unsigned int length_of_data_to_sign;

    const unsigned int undo_element_size;

public:
    using hash_type = uint16_t;

    rdp_mppc_enc_hash_table_manager(unsigned int length_of_data_to_sign, unsigned int max_undo_element)
        : hash_table(new T[MAX_HASH_TABLE_ELEMENT]{})
        , length_of_data_to_sign(length_of_data_to_sign)
        , undo_element_size(sizeof(hash_type) + sizeof(T))
    {
        auto const undo_buf_size = max_undo_element * this->undo_element_size;

        this->undo_buffer_begin = std::make_unique<uint8_t[]>(undo_buf_size);
        std::fill(this->undo_buffer_begin.get(), this->undo_buffer_begin.get() + undo_buf_size, uint8_t{});

        this->undo_buffer_end     = this->undo_buffer_begin.get() + undo_buf_size;
        this->undo_buffer_current = this->undo_buffer_begin.get();
    }

    inline void clear_undo_history() {
        this->undo_buffer_current = this->undo_buffer_begin.get();
    }

    void dump(bool mini_dump) const
    {
        LOG(LOG_INFO, "Type=RDP X.X bulk compressor hash table manager");
        LOG(LOG_INFO, "hashTable");
        hexdump_d(reinterpret_cast<uint8_t const *>(&this->hash_table.get()[0]), (mini_dump ? 16 : get_table_size())); /*NOLINT*/
    }

    inline T get_offset(hash_type hash) const
    {
        return this->hash_table[hash];
    }

    constexpr static size_t get_table_size()
    {
        return MAX_HASH_TABLE_ELEMENT * sizeof(T);
    }

    inline hash_type sign(const uint8_t * data)
    {
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
        for (unsigned int index = 0; index < length_of_data_to_sign; index++) {
            crc = (crc >> 8) ^ crc_table[(crc ^ data[index]) & 0xff];
        }
        return crc;
    }

    inline void update(hash_type hash, T offset)
    {
        if (this->undo_buffer_current != this->undo_buffer_end) {
            *(reinterpret_cast<hash_type *>(this->undo_buffer_current                   )) = hash;
            *(reinterpret_cast<T *>       (this->undo_buffer_current + sizeof(hash_type))) = hash_table[hash];

            this->undo_buffer_current += this->undo_element_size;
        }
        this->hash_table[hash] = offset;
    }

    inline void update_indirect(const uint8_t * data, T offset)
    {
        this->update(this->sign(data + offset), offset);
    }

    inline void reset()
    {
        ::memset(this->hash_table.get(), 0, get_table_size());

        this->undo_buffer_current = this->undo_buffer_begin.get();
    }

    inline bool undo_last_changes()
    {
        if (this->undo_buffer_current == this->undo_buffer_end) {
            return false;
        }

        while (this->undo_buffer_current != this->undo_buffer_begin.get()) {
            this->undo_buffer_current -= this->undo_element_size;

            this->hash_table[*(reinterpret_cast<hash_type *>(this->undo_buffer_current))] =
                *(reinterpret_cast<T *>(this->undo_buffer_current + sizeof(hash_type)));
        }

        return true;
    }
};


static const size_t RDP_40_50_COMPRESSOR_MINIMUM_MATCH_LENGTH = 3;
