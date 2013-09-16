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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Implementation of Microsoft Point to Point Compression (MPPC) protocol

   Based on code by Laxmikant Rashinkar & Jiten Pathy from FreeRDP project
   Copyright 2011 Laxmikant Rashinkar <LK.Rashinkar@gmail.com>
   Copyright 2012 Jiten Pathy
*/

#ifndef _REDEMPTION_CORE_RDP_MPPC_HPP_
#define _REDEMPTION_CORE_RDP_MPPC_HPP_

#include <stdint.h>

#include "error.hpp"

/* Compression Types */
enum {
    PACKET_COMPRESSED = 0x20,
    PACKET_AT_FRONT   = 0x40,
    PACKET_FLUSHED    = 0x80
};

// See core/RDP/logon.hpp for documentation on compresson types

//     to get PACKET_COMPR_TYPE: (flags & CompressonTypeMask) >> 9
//     to set PACKET_COMPR_TYPE: (flags | (CompressonTypeMask & (compr_type << 9))
enum {
    PACKET_COMPR_TYPE_8K    = 0x00,
    PACKET_COMPR_TYPE_64K   = 0x01,
    PACKET_COMPR_TYPE_RDP6  = 0x02,
    PACKET_COMPR_TYPE_RDP61 = 0x03,
};

enum {
    RDP6_HISTORY_BUF_SIZE  = 65536,
    RDP6_OFFSET_CACHE_SIZE = 8
};

/* local defines */

#define RDP_40_HIST_BUF_LEN (1024 * 8) /* RDP 4.0 uses 8K history buf */
#define RDP_50_HIST_BUF_LEN (1024 * 64) /* RDP 5.0 uses 64K history buf */

static uint8_t HuffLenLEC[] = {
    0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x9, 0x8, 0x9, 0x9, 0x9, 0x9, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9,
    0x8, 0x9, 0x9, 0xa, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa,
    0x9, 0x9, 0xa, 0x9, 0xa, 0x9, 0xa, 0x9, 0x9, 0x9, 0xa, 0xa, 0x9, 0xa, 0x9, 0x9,

    0x8, 0x9, 0x9, 0x9, 0x9, 0xa, 0xa, 0xa, 0x9, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0x9, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0x8, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9,

    0x7, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xd, 0xa, 0xa, 0xa, 0xa,
    0xa, 0xa, 0xb, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,

    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa,
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa,
    0x9, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0xa, 0x9, 0xa,
    0x8, 0x9, 0x9, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0xa, 0xa, 0xa, 0x9, 0x9, 0x8, 0x7,

    0xd, 0xd, 0x7, 0x7, 0xa, 0x7, 0x7, 0x6, 0x6, 0x6, 0x6, 0x5, 0x6, 0x6, 0x6, 0x5,
    0x6, 0x5, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x8, 0x5, 0x6, 0x7, 0x7
};

static uint16_t HuffIndexLEC[512] = {
    0x007b, 0xff1f, 0xff0d, 0xfe27, 0xfe00, 0xff05, 0xff17, 0xfe68, 0x00c5, 0xfe07,
    0xff13, 0xfec0, 0xff08, 0xfe18, 0xff1b, 0xfeb3, 0xfe03, 0x00a2, 0xfe42, 0xff10,
    0xfe0b, 0xfe02, 0xfe91, 0xff19, 0xfe80, 0x00e9, 0xfe3a, 0xff15, 0xfe12, 0x0057,
    0xfed7, 0xff1d, 0xff0e, 0xfe35, 0xfe69, 0xff22, 0xff18, 0xfe7a, 0xfe01, 0xff23,
    0xff14, 0xfef4, 0xfeb4, 0xfe09, 0xff1c, 0xfec4, 0xff09, 0xfe60, 0xfe70, 0xff12,
    0xfe05, 0xfe92, 0xfea1, 0xff1a, 0xfe0f, 0xff07, 0xfe56, 0xff16, 0xff02, 0xfed8,
    0xfee8, 0xff1e, 0xfe1d, 0x003b, 0xffff, 0xff06, 0xffff, 0xfe71, 0xfe89, 0xffff,
    0xffff, 0xfe2c, 0xfe2b, 0xfe20, 0xffff, 0xfebb, 0xfecf, 0xfe08, 0xffff, 0xfee0,
    0xfe0d, 0xffff, 0xfe99, 0xffff, 0xfe04, 0xfeaa, 0xfe49, 0xffff, 0xfe17, 0xfe61,
    0xfedf, 0xffff, 0xfeff, 0xfef6, 0xfe4c, 0xffff, 0xffff, 0xfe87, 0xffff, 0xff24,
    0xffff, 0xfe3c, 0xfe72, 0xffff, 0xffff, 0xfece, 0xffff, 0xfefe, 0xffff, 0xfe23,
    0xfebc, 0xfe0a, 0xfea9, 0xffff, 0xfe11, 0xffff, 0xfe82, 0xffff, 0xfe06, 0xfe9a,
    0xfef5, 0xffff, 0xfe22, 0xfe4d, 0xfe5f, 0xffff, 0xff03, 0xfee1, 0xffff, 0xfeca,
    0xfecc, 0xffff, 0xfe19, 0xffff, 0xfeb7, 0xffff, 0xffff, 0xfe83, 0xfe29, 0xffff,
    0xffff, 0xffff, 0xfe6c, 0xffff, 0xfeed, 0xffff, 0xffff, 0xfe46, 0xfe5c, 0xfe15,
    0xffff, 0xfedb, 0xfea6, 0xffff, 0xffff, 0xfe44, 0xffff, 0xfe0c, 0xffff, 0xfe95,
    0xfefc, 0xffff, 0xffff, 0xfeb8, 0x16c9, 0xffff, 0xfef0, 0xffff, 0xfe38, 0xffff,
    0xffff, 0xfe6d, 0xfe7e, 0xffff, 0xffff, 0xffff, 0xffff, 0xfe5b, 0xfedc, 0xffff,
    0xffff, 0xfeec, 0xfe47, 0xfe1f, 0xffff, 0xfe7f, 0xfe96, 0xffff, 0xffff, 0xfea5,
    0xffff, 0xfe10, 0xfe40, 0xfe32, 0xfebf, 0xffff, 0xffff, 0xfed4, 0xfef1, 0xffff,
    0xffff, 0xffff, 0xfe75, 0xffff, 0xffff, 0xfe8d, 0xfe31, 0xffff, 0xfe65, 0xfe1b,
    0xffff, 0xfee4, 0xfefb, 0xffff, 0xffff, 0xfe52, 0xffff, 0xfe0e, 0xffff, 0xfe9d,
    0xfeaf, 0xffff, 0xffff, 0xfe51, 0xfed3, 0xffff, 0xff20, 0xffff, 0xfe2f, 0xffff,
    0xffff, 0xfec1, 0xfe8c, 0xffff, 0xffff, 0xffff, 0xfe3f, 0xffff, 0xffff, 0xfe76,
    0xffff, 0xfefa, 0xfe53, 0xfe25, 0xffff, 0xfe64, 0xfee5, 0xffff, 0xffff, 0xfeae,
    0xffff, 0xfe13, 0xffff, 0xfe88, 0xfe9e, 0xffff, 0xfe43, 0xffff, 0xffff, 0xfea4,
    0xfe93, 0xffff, 0xffff, 0xffff, 0xfe3d, 0xffff, 0xffff, 0xfeeb, 0xfed9, 0xffff,
    0xfe14, 0xfe5a, 0xffff, 0xfe28, 0xfe7d, 0xffff, 0xffff, 0xfe6a, 0xffff, 0xffff,
    0xff01, 0xfec6, 0xfec8, 0xffff, 0xffff, 0xfeb5, 0xffff, 0xffff, 0xffff, 0xfe94,
    0xfe78, 0xffff, 0xffff, 0xffff, 0xfea3, 0xffff, 0xffff, 0xfeda, 0xfe58, 0xffff,
    0xfe1e, 0xfe45, 0xfeea, 0xffff, 0xfe6b, 0xffff, 0xffff, 0xfe37, 0xffff, 0xffff,
    0xffff, 0xfe7c, 0xfeb6, 0xffff, 0xffff, 0xfef8, 0xffff, 0xffff, 0xffff, 0xfec7,
    0xfe9b, 0xffff, 0xffff, 0xffff, 0xfe50, 0xffff, 0xffff, 0xfead, 0xfee2, 0xffff,
    0xfe1a, 0xfe63, 0xfe4e, 0xffff, 0xffff, 0xfef9, 0xffff, 0xfe73, 0xffff, 0xffff,
    0xffff, 0xfe30, 0xfe8b, 0xffff, 0xffff, 0xfebd, 0xfe2e, 0x0100, 0xffff, 0xfeee,
    0xfed2, 0xffff, 0xffff, 0xffff, 0xfeac, 0xffff, 0xffff, 0xfe9c, 0xfe84, 0xffff,
    0xfe24, 0xfe4f, 0xfef7, 0xffff, 0xffff, 0xfee3, 0xfe62, 0xffff, 0xffff, 0xffff,
    0xffff, 0xfe8a, 0xfe74, 0xffff, 0xffff, 0xfe3e, 0xffff, 0xffff, 0xffff, 0xfed1,
    0xfebe, 0xffff, 0xffff, 0xfe2d, 0xffff, 0xfe4a, 0xfef3, 0xffff, 0xffff, 0xfedd,
    0xfe5e, 0xfe16, 0xffff, 0xfe48, 0xfea8, 0xffff, 0xfeab, 0xfe97, 0xffff, 0xffff,
    0xfed0, 0xffff, 0xffff, 0xfecd, 0xfeb9, 0xffff, 0xffff, 0xffff, 0xfe2a, 0xffff,
    0xffff, 0xfe86, 0xfe6e, 0xffff, 0xffff, 0xffff, 0xfede, 0xffff, 0xffff, 0xfe5d,
    0xfe4b, 0xfe21, 0xffff, 0xfeef, 0xfe98, 0xffff, 0xffff, 0xfe81, 0xffff, 0xffff,
    0xffff, 0xfea7, 0xffff, 0xfeba, 0xfefd, 0xffff, 0xffff, 0xffff, 0xfecb, 0xffff,
    0xffff, 0xfe6f, 0xfe39, 0xffff, 0xffff, 0xffff, 0xfe85, 0xffff, 0x010c, 0xfee6,
    0xfe67, 0xfe1c, 0xffff, 0xfe54, 0xfeb2, 0xffff, 0xffff, 0xfe9f, 0xffff, 0xffff,
    0xffff, 0xfe59, 0xfeb1, 0xffff, 0xfec2, 0xffff, 0xffff, 0xfe36, 0xfef2, 0xffff,
    0xffff, 0xfed6, 0xfe77, 0xffff, 0xffff, 0xffff, 0xfe33, 0xffff, 0xffff, 0xfe8f,
    0xfe55, 0xfe26, 0x010a, 0xff04, 0xfee7, 0xffff, 0x0121, 0xfe66, 0xffff, 0xffff,
    0xffff, 0xfeb0, 0xfea0, 0xffff, 0x010f, 0xfe90, 0xffff, 0xffff, 0xfed5, 0xffff,
    0xffff, 0xfec3, 0xfe34, 0xffff, 0xffff, 0xffff, 0xfe8e, 0xffff, 0x0111, 0xfe79,
    0xfe41, 0x010b
};

static uint16_t LECHTab[] = { 511, 0, 508, 448, 494, 347, 486, 482 };

static uint8_t HuffLenLOM[] = {
    0x4, 0x2, 0x3, 0x4, 0x3, 0x4, 0x4, 0x5, 0x4, 0x5, 0x5, 0x6, 0x6, 0x7, 0x7, 0x8,
    0x7, 0x8, 0x8, 0x9, 0x9, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9, 0x9
};

static uint16_t HuffIndexLOM[] = {
    0xfe1, 0xfe0, 0xfe2, 0xfe8, 0xe, 0xfe5, 0xfe4, 0xfea, 0xff1, 0xfe3, 0x15, 0xfe7,
    0xfef, 0x46, 0xff0, 0xfed, 0xfff, 0xff7, 0xffb, 0x19, 0xffd, 0xff4, 0x12c, 0xfeb,
    0xffe, 0xff6, 0xffa, 0x89, 0xffc, 0xff3, 0xff8, 0xff2
};

static uint8_t LOMHTab[] = { 0, 4, 10, 19 };

static uint8_t CopyOffsetBitsLUT[] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11,
    11, 12, 12, 13, 13, 14, 14, 15
};

static uint32_t CopyOffsetBaseLUT[] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 32769, 49153,
    65537
};

static uint8_t LOMBitsLUT[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 6, 6,
    8, 8, 14, 14
};

static uint16_t LOMBaseLUT[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 22, 26, 30, 34, 42, 50, 58, 66,
    82, 98, 114, 130, 194, 258, 514, 2, 2
};

struct rdp_mppc_dec
{
    uint8_t  * history_buf;
    uint16_t * offset_cache;
    uint8_t  * history_buf_end;
    uint8_t  * history_ptr;

    rdp_mppc_dec()
    {
        this->history_buf = (uint8_t *)malloc(RDP6_HISTORY_BUF_SIZE);
        memset(this->history_buf, 0, RDP6_HISTORY_BUF_SIZE);

        this->offset_cache = (uint16_t *)malloc(RDP6_OFFSET_CACHE_SIZE);
        memset(this->offset_cache, 0, RDP6_OFFSET_CACHE_SIZE);

        this->history_ptr     = this->history_buf;
        this->history_buf_end = this->history_buf + RDP6_HISTORY_BUF_SIZE - 1;
    }

    ~rdp_mppc_dec()
    {
        free(this->history_buf);
        free(this->offset_cache);
    }

    /**
     * decompress RDP 4 data
     *
     * @param cbuf    compressed data
     * @param len     length of compressed data
     * @param ctype   compression flags
     * @param roff    starting offset of uncompressed data
     * @param rlen    length of uncompressed data
     *
     * @return        true on success, False on failure
     */
    int decompress_rdp_4(uint8_t * cbuf, int len, int ctype, uint32_t * roff,
        uint32_t * rlen)
    {
//        LOG(LOG_INFO, "decompress_rdp_4");

        uint8_t  * src_ptr       = 0;       /* used while copying compressed data         */
        uint8_t  * cptr          = cbuf;    /* points to next uint8_t in cbuf             */
        uint16_t   copy_offset   = 0;       /* location to copy data from                 */
        uint16_t   lom           = 0;       /* length of match                            */
        int        bits_left     = 0;       /* bits left in d34 for processing            */
        int        cur_bits_left = 0;       /* bits left in cur_uint8_t for processing    */
        uint32_t   d32           = 0;       /* we process 4 compressed uint8_ts at a time */
        uint8_t    cur_uint8_t   = 0;       /* last uint8_t fetched from cbuf             */

        *rlen = 0;

        /* get next free slot in history buffer    */
        /* points to next free slot in history_buf */
        uint8_t * history_ptr = this->history_ptr;
        *roff = history_ptr - this->history_buf;

        if (ctype & PACKET_AT_FRONT)
        {
            /* place compressed data at start of history buffer */
            history_ptr       = this->history_buf;
            this->history_ptr = this->history_buf;
            *roff = 0;
        }

        if (ctype & PACKET_FLUSHED)
        {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP6_HISTORY_BUF_SIZE);
            *roff = 0;
        }

        if ((ctype & PACKET_COMPRESSED) != PACKET_COMPRESSED)
        {
            /* data in cbuf is not compressed - copy to history buf as is */
            memcpy(history_ptr, cbuf, len);
            history_ptr       += len;
            *rlen             =  history_ptr - this->history_ptr;
            this->history_ptr =  history_ptr;
            return true;
        }

        /* load initial data */
        int tmp = 24;
        while (cptr < cbuf + len)
        {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       -= 8;
            if (tmp < 0)
            {
                break;
            }
        }

        if (cptr < cbuf + len)
        {
            cur_uint8_t   = *cptr++;
            cur_bits_left = 8;
        }
        else
        {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        while (bits_left >= 8)
        {
            /*
               value 0xxxxxxx  = literal, not encoded
               value 10xxxxxx  = literal, encoded
               value 1111xxxx  = copy offset   0 - 63
               value 1110xxxx  = copy offset  64 - 319
               value 110xxxxx  = copy offset 320 - 8191
            */

            /*
               at this point, we are guaranteed that d32 has 32 bits to
               be processed, unless we have reached end of cbuf
            */

            copy_offset = 0;

            if ((d32 & 0x80000000) == 0)
            {
                /* got a literal */
                *history_ptr++ =   d32 >> 24;
                d32            <<= 8;
                bits_left      -=  8;
            }
            else if ((d32 & 0xc0000000) == 0x80000000)
            {
                /* got encoded literal */
                d32            <<= 2;
                *history_ptr++ =   (d32 >> 25) | 0x80;
                d32            <<= 7;
                bits_left      -= 9;
            }
            else if ((d32 & 0xf0000000) == 0xf0000000)
            {
                /* got copy offset in range 0 - 63, */
                /* with 6 bit copy offset */
                d32         <<= 4;
                copy_offset =   d32 >> 26;
                d32         <<= 6;
                bits_left   -= 10;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000)
            {
                /* got copy offset in range 64 - 319, */
                /* with 8 bit copy offset */
                d32         <<= 4;
                copy_offset =   d32 >> 24;
                copy_offset +=  64;
                d32         <<= 8;
                bits_left   -=  12;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000)
            {
                /* got copy offset in range 320 - 8191, */
                /* with 13 bits copy offset */
                d32         <<= 3;
                copy_offset =   d32 >> 19;
                copy_offset +=  320;
                d32         <<= 13;
                bits_left   -=  16;
            }

            /*
            ** get more bits before we process length of match
            */

            /* how may bits do we need to get? */
            int tmp2 = 32 - bits_left;

            while (tmp2)
            {
                if (cur_bits_left < tmp2)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp2      -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp2          = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp2)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp2);
                    cur_uint8_t   <<= tmp2;
                    cur_bits_left -=  tmp2;
                    bits_left     = 32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp2);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset)
            {
                continue;
            }

            /*
            ** compute Length of Match
            */

            /*
               lengh of match  Encoding (binary header + LoM bits
               --------------  ----------------------------------
               3               0
               4...7           10 + 2 lower bits of L-o-M
               8...15          110 + 3 lower bits of L-o-M
               16...31         1110 + 4 lower bits of L-o-M
               32...63         11110 + 5 lower bits of L-o-M
               64...127        111110 + 6 lower bits of L-o-M
               128...255       1111110 + 7 lower bits of L-o-M
               256...511       11111110 + 8 lower bits of L-o-M
               512...1023      111111110 + 9 lower bits of L-o-M
               1024...2047     1111111110 + 10 lower bits of L-o-M
               2048...4095     11111111110 + 11 lower bits of L-o-M
               4096...8191     111111111110 + 12 lower bits of L-o-M
            */

            if ((d32 & 0x80000000) == 0)
            {
                /* lom is fixed to 3 */
                lom       =   3;
                d32       <<= 1;
                bits_left -=  1;
            }
            else if ((d32 & 0xc0000000) == 0x80000000)
            {
                /* 2 lower bits of LoM */
                lom       =   ((d32 >> 28) & 0x03) + 4;
                d32       <<= 4;
                bits_left -=  4;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000)
            {
                /* 3 lower bits of LoM */
                lom       =   ((d32 >> 26) & 0x07) + 8;
                d32       <<= 6;
                bits_left -=  6;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000)
            {
                /* 4 lower bits of LoM */
                lom       =   ((d32 >> 24) & 0x0f) + 16;
                d32       <<= 8;
                bits_left -=  8;
            }
            else if ((d32 & 0xf8000000) == 0xf0000000)
            {
                /* 5 lower bits of LoM */
                lom       =   ((d32 >> 22) & 0x1f) + 32;
                d32       <<= 10;
                bits_left -=  10;
            }
            else if ((d32 & 0xfc000000) == 0xf8000000)
            {
                /* 6 lower bits of LoM */
                lom       =   ((d32 >> 20) & 0x3f) + 64;
                d32       <<= 12;
                bits_left -=  12;
            }
            else if ((d32 & 0xfe000000) == 0xfc000000)
            {
                /* 7 lower bits of LoM */
                lom       =   ((d32 >> 18) & 0x7f) + 128;
                d32       <<= 14;
                bits_left -=  14;
            }
            else if ((d32 & 0xff000000) == 0xfe000000)
            {
                /* 8 lower bits of LoM */
                lom       =   ((d32 >> 16) & 0xff) + 256;
                d32       <<= 16;
                bits_left -=  16;
            }
            else if ((d32 & 0xff800000) == 0xff000000)
            {
                /* 9 lower bits of LoM */
                lom       =   ((d32 >> 14) & 0x1ff) + 512;
                d32       <<= 18;
                bits_left -=  18;
            }
            else if ((d32 & 0xffc00000) == 0xff800000)
            {
                /* 10 lower bits of LoM */
                lom       =   ((d32 >> 12) & 0x3ff) + 1024;
                d32       <<= 20;
                bits_left -=  20;
            }
            else if ((d32 & 0xffe00000) == 0xffc00000)
            {
                /* 11 lower bits of LoM */
                lom       =   ((d32 >> 10) & 0x7ff) + 2048;
                d32       <<= 22;
                bits_left -=  22;
            }
            else if ((d32 & 0xfff00000) == 0xffe00000)
            {
                /* 12 lower bits of LoM */
                lom       =   ((d32 >> 8) & 0xfff) + 4096;
                d32       <<= 24;
                bits_left -=  24;
            }

            /* now that we have copy_offset and LoM, process them */

            src_ptr = history_ptr - copy_offset;
            if (src_ptr >= this->history_buf)
            {
                /* data does not wrap around */
                while (lom > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }
            else
            {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (lom && (src_ptr <= this->history_buf_end))
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }

                src_ptr = this->history_buf;
                while (lom > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }

            /*
            ** get more bits before we restart the loop
            */

            /* how many bits do we need to get? */
            int tmp3 = 32 - bits_left;

            while (tmp3)
            {
                if (cur_bits_left < tmp3)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp3      -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp3          = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp3)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp3);
                    cur_uint8_t   <<= tmp3;
                    cur_bits_left -=  tmp3;
                    bits_left     =   32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp3);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }
        } /* end while (bits_left >= 8) */

        *rlen = history_ptr - this->history_ptr;

        this->history_ptr = history_ptr;

        return true;
    }

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
    int decompress_rdp_5(uint8_t * cbuf, int len, int ctype, uint32_t * roff,
        uint32_t * rlen)
    {
//        LOG(LOG_INFO, "decompress_rdp_5");

        uint8_t  * history_ptr;     /* points to next free slot in bistory_buf    */
        uint32_t   d32;             /* we process 4 compressed uint8_ts at a time */
        uint16_t   copy_offset;     /* location to copy data from                 */
        uint16_t   lom;             /* length of match                            */
        uint8_t  * src_ptr;         /* used while copying compressed data         */
        uint8_t  * cptr;            /* points to next uint8_t in cbuf             */
        uint8_t    cur_uint8_t;     /* last uint8_t fetched from cbuf             */
        int        bits_left;       /* bits left in d32 for processing            */
        int        cur_bits_left;   /* bits left in cur_uint8_t for processing    */
        int        tmp;

        src_ptr       = 0;
        cptr          = cbuf;
        copy_offset   = 0;
        lom           = 0;
        bits_left     = 0;
        cur_bits_left = 0;
        d32           = 0;
        cur_uint8_t   = 0;
        *rlen         = 0;

        /* get next free slot in history buffer */
        history_ptr = this->history_ptr;
        *roff       = history_ptr - this->history_buf;

        if (ctype & PACKET_AT_FRONT)
        {
            /* place compressed data at start of history buffer */
            history_ptr       = this->history_buf;
            this->history_ptr = this->history_buf;
            *roff             = 0;
        }

        if (ctype & PACKET_FLUSHED)
        {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP6_HISTORY_BUF_SIZE);
            *roff = 0;
        }

        if ((ctype & PACKET_COMPRESSED) != PACKET_COMPRESSED)
        {
            /* data in cbuf is not compressed - copy to history buf as is */
            memcpy(history_ptr, cbuf, len);
            history_ptr       += len;
            *rlen             =  history_ptr - this->history_ptr;
            this->history_ptr =  history_ptr;
            return true;
        }

        /* load initial data */
        tmp = 24;
        while (cptr < cbuf + len)
        {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       -= 8;
            if (tmp < 0)
            {
                break;
            }
        }

        if (cptr < cbuf + len)
        {
            cur_uint8_t   = *cptr++;
            cur_bits_left = 8;
        }
        else
        {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        while (bits_left >= 8)
        {
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

            copy_offset = 0;

            if ((d32 & 0x80000000) == 0)
            {
                /* got a literal */
                *history_ptr++ =   d32 >> 24;
                d32            <<= 8;
                bits_left      -=  8;
            }
            else if ((d32 & 0xc0000000) == 0x80000000)
            {
                /* got encoded literal */
                d32            <<= 2;
                *history_ptr++ =   (d32 >> 25) | 0x80;
                d32            <<= 7;
                bits_left      -=  9;
            }
            else if ((d32 & 0xf8000000) == 0xf8000000)
            {
                /* got copy offset in range 0 - 63, */
                /* with 6 bit copy offset */
                d32         <<= 5;
                copy_offset =   d32 >>  26;
                d32         <<= 6;
                bits_left   -=  11;
            }
            else if ((d32 & 0xf8000000) == 0xf0000000)
            {
                /* got copy offset in range 64 - 319, */
                /* with 8 bit copy offset */
                d32         <<= 5;
                copy_offset =   d32 >> 24;
                copy_offset +=  64;
                d32         <<= 8;
                bits_left   -=  13;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000)
            {
                /* got copy offset in range 320 - 2367, */
                /* with 11 bits copy offset */
                d32         <<= 4;
                copy_offset =   d32 >> 21;
                copy_offset +=  320;
                d32         <<= 11;
                bits_left   -=  15;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000)
            {
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

            while (tmp)
            {
                if (cur_bits_left < tmp)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset)
            {
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

            if ((d32 & 0x80000000) == 0)
            {
                /* lom is fixed to 3 */
                lom       =   3;
                d32       <<= 1;
                bits_left -=  1;
            }
            else if ((d32 & 0xc0000000) == 0x80000000)
            {
                /* 2 lower bits of LoM */
                lom       =   ((d32 >> 28) & 0x03) + 4;
                d32       <<= 4;
                bits_left -=  4;
            }
            else if ((d32 & 0xe0000000) == 0xc0000000)
            {
                /* 3 lower bits of LoM */
                lom       =   ((d32 >> 26) & 0x07) + 8;
                d32       <<= 6;
                bits_left -=  6;
            }
            else if ((d32 & 0xf0000000) == 0xe0000000)
            {
                /* 4 lower bits of LoM */
                lom       =   ((d32 >> 24) & 0x0f) + 16;
                d32       <<= 8;
                bits_left -=  8;
            }
            else if ((d32 & 0xf8000000) == 0xf0000000)
            {
                /* 5 lower bits of LoM */
                lom       =   ((d32 >> 22) & 0x1f) + 32;
                d32       <<= 10;
                bits_left -=  10;
            }
            else if ((d32 & 0xfc000000) == 0xf8000000)
            {
                /* 6 lower bits of LoM */
                lom       =   ((d32 >> 20) & 0x3f) + 64;
                d32       <<= 12;
                bits_left -=  12;
            }
            else if ((d32 & 0xfe000000) == 0xfc000000)
            {
                /* 7 lower bits of LoM */
                lom       =   ((d32 >> 18) & 0x7f) + 128;
                d32       <<= 14;
                bits_left -=  14;
            }
            else if ((d32 & 0xff000000) == 0xfe000000)
            {
                /* 8 lower bits of LoM */
                lom       =   ((d32 >> 16) & 0xff) + 256;
                d32       <<= 16;
                bits_left -=  16;
            }
            else if ((d32 & 0xff800000) == 0xff000000)
            {
                /* 9 lower bits of LoM */
                lom       =   ((d32 >> 14) & 0x1ff) + 512;
                d32       <<= 18;
                bits_left -=  18;
            }
            else if ((d32 & 0xffc00000) == 0xff800000)
            {
                /* 10 lower bits of LoM */
                lom       =   ((d32 >> 12) & 0x3ff) + 1024;
                d32       <<= 20;
                bits_left -=  20;
            }
            else if ((d32 & 0xffe00000) == 0xffc00000)
            {
                /* 11 lower bits of LoM */
                lom       =   ((d32 >> 10) & 0x7ff) + 2048;
                d32       <<= 22;
                bits_left -=  22;
            }
            else if ((d32 & 0xfff00000) == 0xffe00000)
            {
                /* 12 lower bits of LoM */
                lom       =   ((d32 >> 8) & 0xfff) + 4096;
                d32       <<= 24;
                bits_left -=  24;
            }
            else if ((d32 & 0xfff80000) == 0xfff00000)
            {
                /* 13 lower bits of LoM */
                lom       =   ((d32 >> 6) & 0x1fff) + 8192;
                d32       <<= 26;
                bits_left -=  26;
            }
            else if ((d32 & 0xfffc0000) == 0xfff80000)
            {
                /* 14 lower bits of LoM */
                lom       =   ((d32 >> 4) & 0x3fff) + 16384;
                d32       <<= 28;
                bits_left -=  28;
            }
            else if ((d32 & 0xfffe0000) == 0xfffc0000)
            {
                /* 15 lower bits of LoM */
                lom       =   ((d32 >> 2) & 0x7fff) + 32768;
                d32       <<= 30;
                bits_left -=  30;
            }

            /* now that we have copy_offset and LoM, process them */

            src_ptr = history_ptr - copy_offset;
            if (src_ptr >= this->history_buf)
            {
                /* data does not wrap around */
                while (lom > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }
            else
            {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (lom && (src_ptr <= this->history_buf_end))
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }

                src_ptr = this->history_buf;
                while (lom > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    lom--;
                }
            }

            /*
            ** get more bits before we restart the loop
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;

            while (tmp)
            {
                if (cur_bits_left < tmp)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = *cptr++;
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

        } /* end while (cptr < cbuf + len) */

        *rlen = history_ptr - this->history_ptr;

        this->history_ptr = history_ptr;

        return true;
    }

    static inline uint16_t LEChash(uint16_t key)
    {
        return ((key & 0x1ff) ^ (key  >> 9) ^ (key >> 4) ^ (key >> 7));
    }

    static inline uint16_t LOMhash(uint16_t key)
    {
        return ((key & 0x1f) ^ (key  >> 5) ^ (key >> 9));
    }

    static inline uint16_t miniLEChash(uint16_t key)
    {
        uint16_t h;
        h = ((((key >> 8) ^ (key & 0xff)) >> 2) & 0xf);
        if (key >> 9)
        {
            h = ~h;
        }
        return (h % 12);
    }

    static inline uint8_t miniLOMhash(uint16_t key)
    {
        uint8_t h;
        h = (key >> 4) & 0xf;
        return ((h ^ (h >> 2) ^ (h >> 3)) & 0x3);
    }

    static inline uint16_t getLECindex(uint16_t huff)
    {
        uint16_t h = HuffIndexLEC[rdp_mppc_dec::LEChash(huff)];
        if ((h ^ huff) >> 9)
        {
            return h & 0x1ff;
        }
        return HuffIndexLEC[LECHTab[rdp_mppc_dec::miniLEChash(huff)]];
    }

    static inline uint16_t getLOMindex(uint16_t huff)
    {
        uint16_t h = HuffIndexLOM[rdp_mppc_dec::LOMhash(huff)];
        if ((h ^ huff) >> 5)
        {
            return h & 0x1f;
        }
        return HuffIndexLOM[LOMHTab[rdp_mppc_dec::miniLOMhash(huff)]];
    }

    static inline uint32_t transposebits(uint32_t x)
    {
        x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
        x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
        x = ((x & 0x0f0f0f0f) << 4) | ((x >> 4) & 0x0f0f0f0f);
        if ((x >> 8) == 0)
        {
            return x;
        }
        x = ((x & 0x00ff00ff) << 8) | ((x >> 8) & 0x00ff00ff);
        if ((x >> 16) == 0)
        {
            return x;
        }
        x = ((x & 0x0000ffff) << 16) | ((x >> 16) & 0x0000ffff);
        return x;
    }

    static inline void cache_add(uint16_t * offset_cache, uint16_t copy_offset)
    {
        *((uint32_t*)(offset_cache+2)) <<= 16;
        *((uint32_t*)(offset_cache+2)) |=  (*((uint32_t*)offset_cache) >> 16);
        *((uint32_t*)offset_cache)     =   (*((uint32_t*)offset_cache) << 16) | copy_offset;
    }

    static inline void cache_swap(uint16_t * offset_cache, uint16_t LUTIndex)
    {
        uint16_t t = *offset_cache;
        *offset_cache              = *(offset_cache + LUTIndex);
        *(offset_cache + LUTIndex) = t;
    }

    /**
     * decompress RDP 6 data
     *
     * @param cbuf    compressed data
     * @param len     length of compressed data
     * @param ctype   compression flags
     * @param roff    starting offset of uncompressed data
     * @param rlen    length of uncompressed data
     *
     * @return        True on success, False on failure
     */
    int decompress_rdp_6(uint8_t * cbuf, int len, int ctype, uint32_t * roff,
        uint32_t * rlen)
    {
        uint16_t * offset_cache;    /* Copy Offset cache                          */
        uint8_t  * history_ptr;     /* points to next free slot in bistory_buf    */
        uint32_t   d32;             /* we process 4 compressed uint8_ts at a time */
        uint16_t   copy_offset;     /* location to copy data from                 */
        uint16_t   lom;             /* length of match                            */
        uint16_t   LUTIndex;        /* LookUp table Index                         */
        uint8_t  * src_ptr;         /* used while copying compressed data         */
        uint8_t  * cptr;            /* points to next uint8_t in cbuf             */
        uint8_t    cur_uint8_t;     /* last uint8_t fetched from cbuf             */
        int        bits_left;       /* bits left in d32 for processing            */
        int        cur_bits_left;   /* bits left in cur_uint8_t for processing    */
        int        tmp, i;

        src_ptr       = 0;
        cptr          = cbuf;
        copy_offset   = 0;
        lom           = 0;
        bits_left     = 0;
        cur_bits_left = 0;
        d32           = 0;
        cur_uint8_t   = 0;
        *rlen         = 0;

        /* get start of offset_cache */
        offset_cache = this->offset_cache;

        /* get next free slot in history buffer */
        history_ptr = this->history_ptr;
        *roff       = history_ptr - this->history_buf;

        if (ctype & PACKET_AT_FRONT)
        {
            /* slid history_buf and reset history_buf to middle */
            memmove(this->history_buf,
                (this->history_buf + (history_ptr - this->history_buf - 32768)),
                32768);
            history_ptr       = this->history_buf + 32768;
            this->history_ptr = history_ptr;
            *roff             = 32768;
        }

        if (ctype & PACKET_FLUSHED)
        {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP6_HISTORY_BUF_SIZE);
            memset(offset_cache, 0, RDP6_OFFSET_CACHE_SIZE);
            *roff = 0;
        }

        if ((ctype & PACKET_COMPRESSED) != PACKET_COMPRESSED)
        {
            /* data in cbuf is not compressed - copy to history buf as is */
            memcpy(history_ptr, cbuf, len);
            history_ptr       += len;
            *rlen             =  history_ptr - this->history_ptr;
            this->history_ptr =  history_ptr;
            return true;
        }

        /* load initial data */
        tmp = 0;
        while (cptr < cbuf + len)
        {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       += 8;
            if (tmp >= 32)
            {
                break;
            }
        }

        d32 = rdp_mppc_dec::transposebits(d32);

        if (cptr < cbuf + len)
        {
            cur_uint8_t   = rdp_mppc_dec::transposebits(*cptr++);
            cur_bits_left = 8;
        }
        else
        {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        uint32_t i32 = 0;
        while (bits_left >= 8)
        {
            /* Decode Huffman Code for Literal/EOS/CopyOffset */
            copy_offset = 0;
            for (i = 0x5; i <= 0xd; i++)
            {
                if (i == 0xc)
                {
                    continue;
                }
                i32 = rdp_mppc_dec::transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = rdp_mppc_dec::getLECindex(i32);
                if (i == HuffLenLEC[i32])
                {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            if (i32 < 256)
            {
                *history_ptr++ = (uint8_t)i32;
            }
            else if (i32 > 256 && i32 < 289)
            {
                LUTIndex    = i32 - 257;
                tmp         = CopyOffsetBitsLUT[LUTIndex];
                copy_offset = CopyOffsetBaseLUT[LUTIndex] - 0x1;
                if (tmp != 0)
                {
                    copy_offset += rdp_mppc_dec::transposebits(d32 & (0xffffffff << (32 - tmp)));
                }
                rdp_mppc_dec::cache_add(offset_cache, copy_offset);
                d32       <<= tmp;
                bits_left -=  tmp;
            }
            else if ( i32 > 288 && i32 < 293)
            {
                LUTIndex    = i32 - 289;
                copy_offset = *(offset_cache + LUTIndex);
                if (LUTIndex != 0)
                {
                    rdp_mppc_dec::cache_swap(offset_cache, LUTIndex);
                }
            }
            else if (i32 == 256)
            {
                break;
            }


            /*
            ** get more bits before we process length of match
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;
            while (tmp)
            {
                if (cur_bits_left < tmp)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = rdp_mppc_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = rdp_mppc_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset)
                continue;

            for (i = 0x2; i <= 0x9; i++)
            {
                i32 = rdp_mppc_dec::transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = rdp_mppc_dec::getLOMindex(i32);
                if (i == HuffLenLOM[i32])
                {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            tmp       =   LOMBitsLUT[i32];
            lom       =   LOMBaseLUT[i32];
            if(tmp != 0)
            {
                lom += rdp_mppc_dec::transposebits(d32 & (0xffffffff << (32 - tmp)));
            }
            d32       <<= tmp;
            bits_left -=  tmp;

            /* now that we have copy_offset and LoM, process them */

            src_ptr = history_ptr - copy_offset;
            tmp     = (lom > copy_offset) ? copy_offset : lom;
            uint32_t i32 = 0;
            if (src_ptr >= this->history_buf)
            {
                while (tmp > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset)
                {
                    i32            = ((i32 >= copy_offset)) ? 0 : i32;
                    *history_ptr++ = *(src_ptr + i32++);
                    lom--;
                }
            }
            else
            {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (tmp && (src_ptr <= this->history_buf_end))
                {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                src_ptr = this->history_buf;
                while (tmp > 0)
                {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset)
                {
                    i32            = ((i32 > copy_offset)) ? 0 : i32;
                    *history_ptr++ = *(src_ptr + i32++);
                    lom--;
                }
            }

            /*
            ** get more bits before we restart the loop
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;

            while (tmp)
            {
                if (cur_bits_left < tmp)
                {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len)
                    {
                        /* more compressed data available */
                        cur_uint8_t   = rdp_mppc_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else
                    {
                        /* no more compressed data available */
                        tmp           = 0;
                        cur_bits_left = 0;
                    }
                }
                else if (cur_bits_left > tmp)
                {
                    /* we have more bits than we need */
                    d32           |=  cur_uint8_t >> (8 - tmp);
                    cur_uint8_t   <<= tmp;
                    cur_bits_left -=  tmp;
                    bits_left     =   32;
                    break;
                }
                else
                {
                    /* we have just the right amount of bits */
                    d32       |= cur_uint8_t >> (8 - tmp);
                    bits_left =  32;
                    if (cptr < cbuf + len)
                    {
                        cur_uint8_t   = rdp_mppc_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else
                    {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

        }/* end while (bits_left >= 8) */

        if (ctype & PACKET_FLUSHED)
        {
            *rlen = history_ptr - this->history_buf;
        }
        else
        {
            *rlen = history_ptr - this->history_ptr;
        }

        this->history_ptr = history_ptr;

        return true;
    }

    /**
     * decompress RDP 6.1 data
     *
     * @param cbuf    compressed data
     * @param len     length of compressed data
     * @param ctype   compression flags
     * @param roff    starting offset of uncompressed data
     * @param rlen    length of uncompressed data
     *
     * @return        true on success, False on failure
     */
    int decompress_rdp_61(uint8_t * cbuf, int len, int ctype, uint32_t * roff,
        uint32_t * rlen)
    {
        LOG(LOG_INFO,
            "Unsupported RDP decompression method (%d) is invoked!",
            PACKET_COMPR_TYPE_RDP61);
        throw Error(ERR_RDP_UNSUPPORTED);

        return false;
    }

    int decompress_rdp(uint8_t * cbuf, int len, int ctype, uint32_t * roff,
        uint32_t * rlen)
    {
        int type = ctype & 0x0f;

        switch (type)
        {
            case PACKET_COMPR_TYPE_8K:
                return this->decompress_rdp_4(cbuf, len, ctype, roff, rlen);

            case PACKET_COMPR_TYPE_64K:
                return this->decompress_rdp_5(cbuf, len, ctype, roff, rlen);

            case PACKET_COMPR_TYPE_RDP6:
                return this->decompress_rdp_6(cbuf, len, ctype, roff, rlen);

            case PACKET_COMPR_TYPE_RDP61:
                return this->decompress_rdp_61(cbuf, len, ctype, roff, rlen);
        }

        LOG(LOG_ERR, "decompress_rdp: invalid RDP compression code 0x%2.2x", type);
        return false;
    }
};



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

enum {
    PROTO_RDP_40 = 1,
    PROTO_RDP_50 = 2
};

struct rdp_mppc_enc {
    int        protocol_type;       /* PROTO_RDP_40, PROTO_RDP_50 etc */
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
    rdp_mppc_enc(int protocol_type)
    {
        memset(this, 0, sizeof(*this));

        switch (protocol_type)
        {
            case PROTO_RDP_40:
                this->protocol_type = PROTO_RDP_40;
                this->buf_len = RDP_40_HIST_BUF_LEN;
                break;

            case PROTO_RDP_50:
                this->protocol_type = PROTO_RDP_50;
                this->buf_len = RDP_50_HIST_BUF_LEN;
                break;
            default:
                LOG(LOG_INFO,
                    "Unsupported RDP compression method (%d) is invoked!",
                    protocol_type);
                throw Error(ERR_RDP_UNSUPPORTED);
                ;
        }

        this->first_pkt = 1;
        this->historyBuffer = (char*) malloc(this->buf_len);
        TODO("making it static and large enough should be good for both RDP4 and RDP5")
        memset(this->historyBuffer, 0, this->buf_len);

        this->outputBufferPlus = (char*) malloc(this->buf_len + 64 + 8);
        memset(this->outputBufferPlus, 0, this->buf_len + 64);

        this->outputBuffer = this->outputBufferPlus + 64;
        this->hash_table = (uint16_t*) malloc(/*this->buf_len*/RDP_50_HIST_BUF_LEN * 2);
        memset(this->hash_table, 0, /*this->buf_len*/RDP_50_HIST_BUF_LEN * 2);
    }

    /**
     * deinit mppc_enc structure
     *
     * @param   enc  struct to be deinited
     */
    ~rdp_mppc_enc()
    {
        free(this->historyBuffer);
        free(this->outputBufferPlus);
        free(this->hash_table);
    }

    void mini_dump() {
        LOG(LOG_INFO, "protocol_type=%d",      this->protocol_type);
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
        LOG(LOG_INFO, "protocol_type=%d",      this->protocol_type);
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

    static uint32_t signature(const char v[])
    {
        /* CRC16 defs */
        static const uint16_t crc_table[256] =
        {
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

        uint32_t crc = 0xFFFF;
        crc = (crc >> 8) ^ crc_table[(crc ^ v[0]) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ v[1]) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ v[2]) & 0xff];
        return crc;
    }

    static inline void insert_n_bits(int n, uint32_t _data, char * outputBuffer,
        int & bits_left, int & opb_index)
    {
        int bits_to_serialize = n;
        if (bits_left >= bits_to_serialize + 1) {
            const int i = bits_left - bits_to_serialize;
            outputBuffer[opb_index] |= _data << i;
            bits_left = i;
        }
        else {
            outputBuffer[opb_index++] |= _data >> (bits_to_serialize - bits_left);
            _data                     &= (0xFFFFFFFF >> (32 - (bits_to_serialize - bits_left)));
            bits_to_serialize -= bits_left;
            for (; bits_to_serialize >= 8 ; bits_to_serialize -= 8) {
                outputBuffer[opb_index++] =  (_data >> (bits_to_serialize - 8));
                _data                     &= (0xFFFFFFFF >> (32 - (bits_to_serialize - 8)));
            }
            outputBuffer[opb_index] =
                (bits_to_serialize > 0) ? (_data << (8 - bits_to_serialize)) : 0;
            bits_left               = 8 - bits_to_serialize;
        }
    }

    static inline void encode_literal(char c, char * outputBuffer, int & bits_left,
        int & opb_index)
    {
        rdp_mppc_enc::insert_n_bits(
            (c & 0x80) ? 9 : 8,
            (c & 0x80) ? (0x02 << 7) | (c & 0x7F) : c,
            outputBuffer, bits_left, opb_index);
    }

// 3.1.8.4.1 RDP 4.0
// =================

// 3.1.8.4.1.1 Literal Encoding

// Literals are bytes sent uncompressed. If the value of a literal is below
// 0x80, it is not encoded in any special manner. If the literal has a value
// greater than 0x7F it is sent as the bits 10 followed by the lower 7 bits of
// the literal. For example, 0x56 is transmitted as the binary value 01010110,
// while 0xE7 is transmitted as the binary value 101100111.

// 3.1.8.4.1.2 Copy-Tuple Encoding
// ===============================

// Copy-tuples consist of a <copy-offset> and <length-of-match> pair (see
// section 3.1.8.2 for more details).

// 3.1.8.4.1.2.1 Copy-Offset Encoding
// ----------------------------------

// Encoding of the copy-offset value is performed according to the following
// table.

// Copy-offset range    | Encoding (binary header + copy-offset bits)
// 0...63               | 1111 + lower 6 bits of copy-offset
// 64...319             | 1110 + lower 8 bits of (copy-offset – 64)
// 320...8191           | 110 + lower 13 bits of (copy-offset – 320)

// For example:

//    A copy-offset value of 3 is encoded as the binary value 1111 000011.

//    A copy-offset value of 128 is encoded as the binary value 1110 01000000.

//    A copy-offset value of 1024 is encoded as the binary value 110 0001011000000.

// A copy-offset value MUST be followed by a length-of-match (L-o-M) value.

// 3.1.8.4.1.2.2 Length-of-Match Encoding
// --------------------------------------

// Encoding of the length-of-match (L-o-M) value is performed according to the
// following table.

// L-o-M range | Encoding (binary header + L-o-M bits)
// ------------+-----------------------------------------
// 3           |            0
// 4...7       |           10 + 2 lower bits of L-o-M
// 8...15      |          110 + 3 lower bits of L-o-M
// 16...31     |         1110 + 4 lower bits of L-o-M
// 32...63     |        11110 + 5 lower bits of L-o-M
// 64...127    |       111110 + 6 lower bits of L-o-M
// 128...255   |      1111110 + 7 lower bits of L-o-M
// 256...511   |     11111110 + 8 lower bits of L-o-M
// 512...1023  |    111111110 + 9 lower bits of L-o-M
// 1024...2047 |   1111111110 + 10 lower bits of L-o-M
// 2048...4095 |  11111111110 + 11 lower bits of L-o-M
// 4096...8191 | 111111111110 + 12 lower bits of L-o-M

// For example:
//    A length-of-match value of 15 is encoded as the binary value 110 111.
//    A length-of-match value of 120 is encoded as the binary value 111110 111000.
//    A length-of-match value of 4097 is encoded as the binary value 111111111110 000000000001.

    /**
     * encode (compress) data using RDP 4.0 protocol
     *
     * @param   srcData       uncompressed data
     * @param   len           length of srcData
     *
     * @return  true on success, false on failure
     */
    bool compress_rdp_4(uint8_t * srcData, int len)
    {
        int        opb_index    = 0;                    /* index into outputBuffer                        */
        int        bits_left    = 8;                    /* unused bits in current uint8_t in outputBuffer */
        uint16_t * hash_table   = this->hash_table;     /* hash table for pattern matching                */
        char     * outputBuffer = this->outputBuffer;   /* points to enc->outputBuffer                    */

        TODO("this memset should not be necessary")
        memset(outputBuffer, 0, len);

        this->flags = PACKET_COMPR_TYPE_8K;

        if (this->first_pkt) {
            this->first_pkt =  0;
            this->flagsHold |= PACKET_AT_FRONT;
        }

        if ((this->historyOffset + len + 2) >= this->buf_len)
        {
            /* historyBuffer cannot hold srcData - rewind it */
            this->historyOffset =  0;
            this->flagsHold     |= PACKET_AT_FRONT;
            memset(hash_table, 0, RDP_50_HIST_BUF_LEN * 2);
        }

        /* add / append new data to historyBuffer */
        memcpy(&(this->historyBuffer[this->historyOffset]), srcData, len);

        int      ctr         = 0;
        uint32_t copy_offset = 0;   /* pattern match starts here... */

        /* if we are at start of history buffer, do not attempt to compress */
        /* first 2 bytes,because minimum LoM is 3                           */
        if (this->historyOffset == 0) {
            /* encode first two bytes as literals */
            for (int x = 0; x < 2; x++) {
                rdp_mppc_enc::encode_literal(
                    this->historyBuffer[this->historyOffset + x],
                    outputBuffer, bits_left, opb_index);
            }

            hash_table[rdp_mppc_enc::signature(this->historyBuffer)]   = 0;
            hash_table[rdp_mppc_enc::signature(this->historyBuffer+1)] = 1;
            ctr                                          = 2;
        }

        int lom = 0;
        for (; ctr + 2 < len ; ctr += lom) { // we need at least 3 bytes to look for match
            uint32_t crc2           = rdp_mppc_enc::signature(
                this->historyBuffer + this->historyOffset + ctr);
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
                const int nbbits[3]  = {10, 12, 16};
                const int headers[3] = {0x3c0, 0xe00, 0xc000};
                const int base[3]    = {0, 0x40, 0x140};
                int range = (copy_offset <= 0x3F)  ? 0 :
                            (copy_offset <= 0x13F) ? 1 :
                                                     2 ;
                rdp_mppc_enc::insert_n_bits(nbbits[range],
                    headers[range]|(copy_offset - base[range]),
                    outputBuffer, bits_left, opb_index);

                int log_lom = 8 * sizeof(lom) - 1 - __builtin_clz(lom);

                /* encode length of match and insert into output buffer */
                rdp_mppc_enc::insert_n_bits(
                    (lom == 3) ? 1 : 2 * log_lom,
                    (lom == 3) ? 0 : (((((1 << log_lom) - 1) & 0xFFE) << log_lom) | (lom - (1 << log_lom))),
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
            memset(hash_table, 0, RDP_50_HIST_BUF_LEN * 2);
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
    bool compress_rdp_5(uint8_t * srcData, int len)
    {
//        LOG(LOG_INFO, "compress_rdp_5");

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

    /**
     * encode (compress) data
     *
     * @param   srcData       uncompressed data
     * @param   len           length of srcData
     *
     * @return  true on success, false on failure
     */
    bool compress_rdp(uint8_t * srcData, int len)
    {
        if ((srcData == NULL) || (len <= 0) || (len > this->buf_len))
            return false;

        switch (this->protocol_type)
        {
            case PROTO_RDP_40:
                return this->compress_rdp_4(srcData, len);

            case PROTO_RDP_50:
                return this->compress_rdp_5(srcData, len);
        }

        return false;
    }
};

#endif  /* _REDEMPTION_CORE_RDP_MPPC_HPP_ */
