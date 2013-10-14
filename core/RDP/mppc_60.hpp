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

#ifndef _REDEMPTION_CORE_RDP_MPPC_60_HPP_
#define _REDEMPTION_CORE_RDP_MPPC_60_HPP_

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

struct rdp_mppc_60_dec : public rdp_mppc_dec {
protected:
    static const size_t RDP_60_HIST_BUF_LEN      = 1024 * 64;
    static const size_t RDP_60_OFFSET_CACHE_SIZE = 8;

public:
    uint8_t  * history_buf;
    uint16_t * offset_cache;
    uint8_t  * history_buf_end;
    uint8_t  * history_ptr;

    /**
     * Initialize rdp_mppc_60_dec structure
     */
    rdp_mppc_60_dec() {
        this->history_buf     = static_cast<uint8_t *>(calloc(RDP_60_HIST_BUF_LEN, 1));
        this->offset_cache    = static_cast<uint16_t *>(calloc(RDP_60_OFFSET_CACHE_SIZE, 1));
        this->history_ptr     = this->history_buf;
        this->history_buf_end = this->history_buf + RDP_60_HIST_BUF_LEN - 1;
    }

    /**
     * Deinitialize rdp_mppc_60_dec structure
     */
    virtual ~rdp_mppc_60_dec() {
        free(this->history_buf);
        free(this->offset_cache);
    }

    virtual void mini_dump()
    {
        LOG(LOG_INFO, "Type=RDP 6.0 bulk compressor");
        LOG(LOG_INFO, "historyBuffer");
        hexdump_d(this->history_buf, 16);
        LOG(LOG_INFO, "offsetCache");
        hexdump_d(reinterpret_cast<const char *>(this->offset_cache), RDP_60_OFFSET_CACHE_SIZE);
        LOG(LOG_INFO, "historyPointerOffset=%d",   this->history_ptr - this->history_buf);
        LOG(LOG_INFO, "historyBufferEndOffset=%d", this->history_buf_end - this->history_buf);
    }

    virtual void dump()
    {
        LOG(LOG_INFO, "Type=RDP 6.0 bulk compressor");
        LOG(LOG_INFO, "historyBuffer");
        hexdump_d(this->history_buf, RDP_60_HIST_BUF_LEN);
        LOG(LOG_INFO, "offsetCache");
        hexdump_d(reinterpret_cast<const char *>(this->offset_cache), RDP_60_OFFSET_CACHE_SIZE);
        LOG(LOG_INFO, "historyPointerOffset=%d",   this->history_ptr - this->history_buf);
        LOG(LOG_INFO, "historyBufferEndOffset=%d", this->history_buf_end - this->history_buf);
    }

protected:
    static inline uint16_t LEChash(uint16_t key) {
        return ((key & 0x1ff) ^ (key  >> 9) ^ (key >> 4) ^ (key >> 7));
    }

    static inline uint16_t LOMhash(uint16_t key) {
        return ((key & 0x1f) ^ (key  >> 5) ^ (key >> 9));
    }

    static inline uint16_t miniLEChash(uint16_t key) {
        uint16_t h;
        h = ((((key >> 8) ^ (key & 0xff)) >> 2) & 0xf);
        if (key >> 9) {
            h = ~h;
        }
        return (h % 12);
    }

    static inline uint8_t miniLOMhash(uint16_t key) {
        uint8_t h;
        h = (key >> 4) & 0xf;
        return ((h ^ (h >> 2) ^ (h >> 3)) & 0x3);
    }

    static inline uint16_t getLECindex(uint16_t huff) {
        uint16_t h = HuffIndexLEC[rdp_mppc_60_dec::LEChash(huff)];
        if ((h ^ huff) >> 9) {
            return h & 0x1ff;
        }
        return HuffIndexLEC[LECHTab[rdp_mppc_60_dec::miniLEChash(huff)]];
    }

    static inline uint16_t getLOMindex(uint16_t huff) {
        uint16_t h = HuffIndexLOM[rdp_mppc_60_dec::LOMhash(huff)];
        if ((h ^ huff) >> 5) {
            return h & 0x1f;
        }
        return HuffIndexLOM[LOMHTab[rdp_mppc_60_dec::miniLOMhash(huff)]];
    }

    static inline uint32_t transposebits(uint32_t x) {
        x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
        x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
        x = ((x & 0x0f0f0f0f) << 4) | ((x >> 4) & 0x0f0f0f0f);
        if ((x >> 8) == 0) {
            return x;
        }
        x = ((x & 0x00ff00ff) << 8) | ((x >> 8) & 0x00ff00ff);
        if ((x >> 16) == 0) {
            return x;
        }
        x = ((x & 0x0000ffff) << 16) | ((x >> 16) & 0x0000ffff);
        return x;
    }

    static inline void cache_add(uint16_t * offset_cache, uint16_t copy_offset) {
        *((uint32_t*)(offset_cache+2)) <<= 16;
        *((uint32_t*)(offset_cache+2)) |=  (*((uint32_t*)offset_cache) >> 16);
        *((uint32_t*)offset_cache)     =   (*((uint32_t*)offset_cache) << 16) | copy_offset;
    }

    static inline void cache_swap(uint16_t * offset_cache, uint16_t LUTIndex) {
        uint16_t t = *offset_cache;
        *offset_cache              = *(offset_cache + LUTIndex);
        *(offset_cache + LUTIndex) = t;
    }

public:
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
    int decompress_60(uint8_t * cbuf, int len, int ctype, uint32_t * roff, uint32_t * rlen) {
//        LOG(LOG_INFO, "decompress_60");

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

        if (ctype & PACKET_AT_FRONT) {
            /* slid history_buf and reset history_buf to middle */
            memmove(this->history_buf,
                (this->history_buf + (history_ptr - this->history_buf - 32768)),
                32768);
            history_ptr       = this->history_buf + 32768;
            this->history_ptr = history_ptr;
            *roff             = 32768;
        }

        if (ctype & PACKET_FLUSHED) {
            /* re-init history buffer */
            history_ptr = this->history_buf;
            memset(this->history_buf, 0, RDP_60_HIST_BUF_LEN);
            memset(offset_cache, 0, RDP_60_OFFSET_CACHE_SIZE);
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
        tmp = 0;
        while (cptr < cbuf + len) {
            uint32_t i32 = *cptr++;
            d32       |= i32 << tmp;
            bits_left += 8;
            tmp       += 8;
            if (tmp >= 32) {
                break;
            }
        }

        d32 = rdp_mppc_60_dec::transposebits(d32);

        if (cptr < cbuf + len) {
            cur_uint8_t   = rdp_mppc_60_dec::transposebits(*cptr++);
            cur_bits_left = 8;
        }
        else {
            cur_bits_left = 0;
        }

        /*
        ** start uncompressing data in cbuf
        */

        uint32_t i32 = 0;
        while (bits_left >= 8) {
            /* Decode Huffman Code for Literal/EOS/CopyOffset */
            copy_offset = 0;
            for (i = 0x5; i <= 0xd; i++) {
                if (i == 0xc) {
                    continue;
                }
                i32 = rdp_mppc_60_dec::transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = rdp_mppc_60_dec::getLECindex(i32);
                if (i == HuffLenLEC[i32]) {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            if (i32 < 256) {
                *history_ptr++ = static_cast<uint8_t>(i32);
            }
            else if (i32 > 256 && i32 < 289) {
                LUTIndex    = i32 - 257;
                tmp         = CopyOffsetBitsLUT[LUTIndex];
                copy_offset = CopyOffsetBaseLUT[LUTIndex] - 0x1;
                if (tmp != 0) {
                    copy_offset += rdp_mppc_60_dec::transposebits(d32 & (0xffffffff << (32 - tmp)));
                }
                rdp_mppc_60_dec::cache_add(offset_cache, copy_offset);
                d32       <<= tmp;
                bits_left -=  tmp;
            }
            else if ( i32 > 288 && i32 < 293) {
                LUTIndex    = i32 - 289;
                copy_offset = *(offset_cache + LUTIndex);
                if (LUTIndex != 0) {
                    rdp_mppc_60_dec::cache_swap(offset_cache, LUTIndex);
                }
            }
            else if (i32 == 256) {
                break;
            }

            /*
            ** get more bits before we process length of match
            */

            /* how may bits do we need to get? */
            tmp = 32 - bits_left;
            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = rdp_mppc_60_dec::transposebits(*cptr++);
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
                        cur_uint8_t   = rdp_mppc_60_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

            if (!copy_offset)
                continue;

            for (i = 0x2; i <= 0x9; i++) {
                i32 = rdp_mppc_60_dec::transposebits((d32 & (0xffffffff << (32 - i))));
                i32 = rdp_mppc_60_dec::getLOMindex(i32);
                if (i == HuffLenLOM[i32]) {
                    break;
                }
            }
            d32       <<= i;
            bits_left -=  i;
            tmp       =   LOMBitsLUT[i32];
            lom       =   LOMBaseLUT[i32];
            if(tmp != 0) {
                lom += rdp_mppc_60_dec::transposebits(d32 & (0xffffffff << (32 - tmp)));
            }
            d32       <<= tmp;
            bits_left -=  tmp;

            /* now that we have copy_offset and LoM, process them */

            src_ptr = history_ptr - copy_offset;
            tmp     = (lom > copy_offset) ? copy_offset : lom;
            uint32_t i32 = 0;
            if (src_ptr >= this->history_buf) {
                while (tmp > 0) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset) {
                    i32            = ((i32 >= copy_offset)) ? 0 : i32;
                    *history_ptr++ = *(src_ptr + i32++);
                    lom--;
                }
            }
            else {
                src_ptr = this->history_buf_end - (copy_offset - (history_ptr - this->history_buf));
                src_ptr++;
                while (tmp && (src_ptr <= this->history_buf_end)) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                src_ptr = this->history_buf;
                while (tmp > 0) {
                    *history_ptr++ = *src_ptr++;
                    tmp--;
                }
                while (lom > copy_offset) {
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

            while (tmp) {
                if (cur_bits_left < tmp) {
                    /* we have less bits than we need */
                    uint32_t i32 = cur_uint8_t >> (8 - cur_bits_left);
                    d32       |= i32 << ((32 - bits_left) - cur_bits_left);
                    bits_left += cur_bits_left;
                    tmp       -= cur_bits_left;
                    if (cptr < cbuf + len) {
                        /* more compressed data available */
                        cur_uint8_t   = rdp_mppc_60_dec::transposebits(*cptr++);
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
                        cur_uint8_t   = rdp_mppc_60_dec::transposebits(*cptr++);
                        cur_bits_left = 8;
                    }
                    else {
                        cur_bits_left = 0;
                    }
                    break;
                }
            }

        }   /* end while (bits_left >= 8) */

        if (ctype & PACKET_FLUSHED) {
            *rlen = history_ptr - this->history_buf;
        }
        else {
            *rlen = history_ptr - this->history_ptr;
        }

        this->history_ptr = history_ptr;

        return true;
    }   // decompress_60

    virtual int decompress(uint8_t * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) {
        uint32_t roff = 0;
        int      result;

        rlen   = 0;
        result = this->decompress_60(cbuf, len, ctype, &roff, &rlen);
        rdata  = this->history_buf + roff;

        return result;
    }
};  // struct rdp_mppc_60_dec

#endif  // #ifndef _REDEMPTION_CORE_RDP_MPPC_60_HPP_