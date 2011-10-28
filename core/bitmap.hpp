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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni, Martin Potier
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   This file implement the bitmap items data structure
   including RDP RLE compression and decompression algorithms

   It also features storage and color versionning of the bitmap
   returning a pointer on a table, corresponding to the required
   color model.
*/



#if !defined(__BITMAP_HPP__)
#define __BITMAP_HPP__

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include "altoco.hpp"
#include "log.hpp"
#include "client_info.hpp"
#include "stream.hpp"
#include "colors.hpp"
#include <inttypes.h>
#include "rect.hpp"

struct Bitmap {
    public:
    // data_co is allocated on demand
    uint8_t *data_co24;
    uint8_t *data_co16;
    uint8_t *data_co15;
    uint8_t *data_co8;

    int original_bpp;
    BGRPalette original_palette;
    unsigned cx;
    unsigned cy;

    private:
    uint32_t crc;
    bool crc_computed;

    public:

    #warning there is way too many constructors, doing things much too complicated. We should split that in two stages. First prepare minimal context for bitmap (but keep a flag or something to mark it as non ready), then load actual data into bitmap.
    Bitmap(int bpp, const BGRPalette * palette, unsigned cx, unsigned cy, const uint8_t * data, const size_t size, bool compressed=false, int upsidedown=false)
        : data_co24(0), data_co16(0), data_co15(0), data_co8(0),
          original_bpp(bpp), cx(cx), cy(cy),
          crc(0), crc_computed(false)
    {
//        LOG(LOG_ERR, "Creating bitmap cx=%u cy=%u size=%u bpp=%u", cx, cy, size, bpp);
        if (bpp == 8){
            if (palette){
                memcpy(&this->original_palette, palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }
        this->set_data_co(bpp);

        if (compressed) {
            this->decompress(bpp, data, size);
        } else if (upsidedown) {
            this->copy_upsidedown(bpp, data, cx);
        } else {
            assert(size == this->bmp_size(bpp));
            memcpy(this->data_co(bpp), data, size);
        }
        if (this->cx <= 0 || this->cy <= 0){
            LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u size=%u bpp=%u", this->cx, this->cy, size, this->original_bpp);
        }
    }


    Bitmap(int bpp, BGRPalette * palette, const Rect & r, int src_cx, int src_cy, const uint8_t * src_data)
        : data_co24(0), data_co16(0), data_co15(0), data_co8(0), original_bpp(bpp),
        cx(0), cy(0), crc(0), crc_computed(false)
    {
//        LOG(LOG_ERR, "Creating bitmap (2) src_cx=%u src_cy=%u cx=%u cy=%u bpp=%u r(%u, %u, %u, %u)", src_cx, src_cy, cx, cy, bpp, r.x, r.y, r.cx, r.cy);

        if (bpp == 8){
            if (palette){
                memcpy(&this->original_palette, palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }
        unsigned cx = std::min(r.cx, src_cx - r.x);
        #warning there is both cx and this->cx and both can't be interchanged. this is intended to always store bitmaps that are multiple of 4 pixels to override a compatibility problem with rdesktop. This is not necessary for Microsoft clients. See MSRDP-CGR MS-RDPBCGR: 2.2.9.1.1.3.1.2.2 Bitmap Data (TS_BITMAP_DATA)
        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        this->cx = align4(cx);
        unsigned cy = std::min(r.cy, src_cy - r.y);
        this->cy = cy;

        // Important and only once
        this->set_data_co(bpp);

        #warning We could reserve statical space for caches thus avoiding many memory allocation. RDP sets maximum size for cache items anyway and we MUST check this size is never larger than allowed.

        #warning case 32 bits is (certainly) not working
        uint8_t *d8 = this->data_co(bpp);
        unsigned src_row_size = row_size(src_cx, bpp);
        unsigned int width = cx * nbbytes(bpp);

        assert(src_cy > r.y);

        const uint8_t *s8 = src_data + src_row_size * (src_cy - r.y - this->cy) + r.x * nbbytes(bpp);

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(d8, s8, width);
            if (this->line_size(bpp) > width){
                memset(d8+width, 0, this->line_size(bpp) - width);
            }
            s8 += src_row_size;
            d8 += this->line_size(bpp);
        }
        if (this->cx <= 0 || this->cy <= 0){
            LOG(LOG_ERR, "Bogus empty bitmap (2)!!! cx=%u cy=%u bpp=%u src_cx=%u, src_cy=%u r(%u, %u, %u, %u)", this->cx, this->cy, this->original_bpp, src_cx, src_cy, r.x, r.y, r.cx, r.cy);
        }

    }


    Bitmap(const char* filename)
        : data_co24(0), data_co16(0), data_co15(0), data_co8(0), original_bpp(0),
        cx(0), cy(0), crc(0), crc_computed(false)
    {
        LOG(LOG_INFO, "loading bitmap %s", filename);
        int size;
        BGRPalette palette1;
        char type1[4];

        /* header for bmp file */
        struct bmp_header {
            int size;
            int image_width;
            int image_height;
            short planes;
            short bit_count;
            int compression;
            int image_size;
            int x_pels_per_meter;
            int y_pels_per_meter;
            int clr_used;
            int clr_important;
            bmp_header() {
                this->size = 0;
                this->image_width = 0;
                this->image_height = 0;
                this->planes = 0;
                this->bit_count = 0;
                this->compression = 0;
                this->image_size = 0;
                this->x_pels_per_meter = 0;
                this->y_pels_per_meter = 0;
                this->clr_used = 0;
                this->clr_important = 0;
            }
        } header;

        #warning reading of file and bitmap decoding should be kept appart, putting both together makes testing hard. And what if I want to read a bitmap from some network socket instead of a disk file ?
        /* Code related to g_file_open os_call */
        int fd =  open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            /* can't open read / write, try to open read only */
            fd =  open(filename, O_RDONLY);
        }
        if (fd == -1) {
            LOG(LOG_ERR, "Widget_load: error loading bitmap from file [%s]\n", filename);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }

        /* read file type */
        if (read(fd, type1, 2) != 2) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] read error\n", filename);
            close(fd);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        if ((type1[0] != 'B') || (type1[1] != 'M')) {
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s] not BMP file\n", filename);
            close(fd);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        /* read file size */
        Stream stream(8192);
        if (read(fd, stream.data, 4) < 4){
            LOG(LOG_ERR, "Widget_load: error read file size\n");
            close(fd);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        size = stream.in_uint32_le();

        // skip some bytes to set file pointer to bmp header
        lseek(fd, 14, SEEK_SET);
        stream.init(8192);
        if (read(fd, stream.data, 40) < 40){
            close(fd);
            LOG(LOG_ERR, "Widget_load: error read file size (2)\n");
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        #warning we should read header size and use it to read header instead of using magic constant 40
        header.size = stream.in_uint32_le();
        if (header.size != 40){
            LOG(LOG_INFO, "Wrong header size: expected 40, got %d", header.size);
            assert(header.size == 40);
        }

        header.image_width = stream.in_uint32_le();
        header.image_height = stream.in_uint32_le();
        header.planes = stream.in_uint16_le();
        header.bit_count = stream.in_uint16_le();
        header.compression = stream.in_uint32_le();
        header.image_size = stream.in_uint32_le();
        header.x_pels_per_meter = stream.in_uint32_le();
        header.y_pels_per_meter = stream.in_uint32_le();
        header.clr_used = stream.in_uint32_le();
        header.clr_important = stream.in_uint32_le();

        // skip header (including more fields that we do not read if any)
        lseek(fd, 14 + header.size, SEEK_SET);

        // compute pixel size (in Quartet) and read palette if needed
        int file_Qpp = 1;
        #warning add support for loading of 16 bits bmp from file
        switch (header.bit_count){
        // Qpp = groups of 4 bytes per pixel
        case 24:
            file_Qpp = 6;
            break;
        case 8:
            file_Qpp = 2;
        case 4:
            stream.init(8192);
            if (read(fd, stream.data, header.clr_used * 4) < header.clr_used * 4){
                close(fd);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }
            for (int i = 0; i < header.clr_used; i++) {
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
                stream.skip_uint8(1); // skip alpha channel
                palette1[i] = (b << 16)|(g << 8)|r;
            }
            break;
        default:
            LOG(LOG_ERR, "Widget_load: error bitmap file [%s]"
                         " unsupported bpp %d\n", filename,
                         header.bit_count);
            close(fd);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }

        LOG(LOG_INFO, "loading file %d x %d x %d", header.image_width, header.image_height, header.bit_count);

        // bitmap loaded from files are always converted to 24 bits
        // this avoid palette problems for 8 bits,
        // and 4 bits is not supported in other parts of code anyway

        this->cx = header.image_width;
        this->cy = header.image_height;

        // read bitmap data
        size = (header.image_width * header.image_height * file_Qpp) / 2;
        stream.init(size);
        int row_size = (this->cx * file_Qpp) / 2;
        int padding = align4(row_size) - row_size;
        for (unsigned y = 0; y < this->cy; y++) {
            int k = read(fd, stream.data + y * row_size, row_size + padding);
            if (k != (row_size + padding)) {
                LOG(LOG_ERR, "Widget_load: read error reading bitmap file [%s] read\n", filename);
                close(fd);
                throw Error(ERR_BITMAP_LOAD_FAILED);
            }
        }
        close(fd); // from now on all is in memory

        this->original_bpp = 24;
        this->set_data_co(this->original_bpp);
        uint8_t * dest = this->data_co(this->original_bpp);
        const uint8_t nbbytes_dest = ::nbbytes(this->original_bpp);
        row_size = this->line_size(this->original_bpp);

        int k = 0;
        for (unsigned y = 0; y < this->cy ; y++) {
            for (unsigned x = 0 ; x < this->cx; x++) {
                uint32_t pixel = 0;
                switch (header.bit_count){
                case 24:
                {
                    uint8_t r = stream.in_uint8();
                    uint8_t g = stream.in_uint8();
                    uint8_t b = stream.in_uint8();
                    pixel = (b << 16) | (g << 8) | r;
                }
                break;
                case 8:
                    pixel = stream.in_uint8();
                break;
                case 4:
                    if ((x & 1) == 0) {
                        k = stream.in_uint8();
                        pixel = (k >> 4) & 0xf;
                    }
                    else {
                        pixel = k & 0xf;
                    }
                    pixel = palette1[pixel];
                break;
                }
                uint32_t px = color_decode(pixel, header.bit_count, palette1);
                #warning extract constants from loop
                ::out_bytes_le(dest + y * row_size + x * nbbytes_dest, nbbytes_dest, px);
            }
        }
        return;
    }

    void dump(int bpp){
        LOG(LOG_INFO, "------- Dumping bitmap RAW data [%p]---------\n", this);
        LOG(LOG_INFO, "cx=%d cy=%d bpp=%d BPP=%d line_size=%d bmp_size=%d data=%p \n",
            this->cx, this->cy, bpp, nbbytes(bpp), this->line_size(bpp), this->bmp_size(bpp), this->data_co(bpp));
        assert(bpp);
        assert(this->line_size(bpp));
        assert(this->cx);
        assert(this->cy);
        assert(this->data_co(bpp));
        assert(this->bmp_size(bpp));

        LOG(LOG_INFO, "uint8_t raw%p[] = {", this);
        uint8_t * data = this->data_co(bpp);

        for (size_t j = 0 ; j < this->cy ; j++){
            LOG(LOG_INFO, "/* line %d */", (this->cy - j - 1));
            char buffer[2048];
            char * line = buffer;
            buffer[0] = 0;
            for (size_t i = 0; i < this->line_size(bpp); i++){
                line += snprintf(line, 1024, "0x%.2x, ", data[j*this->line_size(bpp)+i]);
                if (i % 16 == 15){
                    LOG(LOG_INFO, buffer);
                    line = buffer;
                    buffer[0] = 0;
                }
            }
            if (line != buffer){
                LOG(LOG_INFO, buffer);
            }
        }
        LOG(LOG_INFO, "}; /* %p */", this);
        LOG(LOG_INFO, "Bitmap bmp%p(%d, %d, %d, raw%p, sizeof(raw%p));", this, bpp, this->cx, this->cy, this, this);

//        LOG(LOG_INFO, "\n-----End of dump [%p] -----------------------\n", this);
    }

    bool isset_bpp(int bpp) {
        switch(bpp) {
            case 24:
                return this->data_co24 != 0;
            case 16:
                return this->data_co16 != 0;
            case 15:
                return this->data_co15 != 0;
            case 8:
                return this->data_co8  != 0;
            default:
                return false;
        }
    }

    void copy_upsidedown(int bpp, const uint8_t* input, uint16_t cx)
    {
        #warning without this evil alnment we are expirimenting problems with VNC bitmaps, but there should be a better fix.
        this->cx = align4(cx);
        uint8_t * d8 = this->data_co(bpp) + (this->cy-1) * this->line_size(bpp);
        const uint8_t * s8 = input;
        uint32_t src_width = cx * nbbytes(bpp);

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(d8, s8, src_width);
            if (this->line_size(bpp) > src_width){
                memset(d8+src_width, 0, this->line_size(bpp) - src_width);
            }
            s8 += src_width;
            d8 -= this->line_size(bpp);
        }
    }

    #warning unifying with decompress, the decompress function should be able to do both. Need probably some decompressor object, that would be a factory that creates bitmap or dump decompression data. Compression should also probably go in the same object (from principale that opening and closing parenthesis should be kept together).
    void dump_decompress(int bpp, const uint8_t* input, size_t size)
    {
        unsigned yprev = 0;
        const uint8_t* end = input + size;
        unsigned color1;
        unsigned color2;
        unsigned mix;
        uint8_t code;
        unsigned mask = 0;
        unsigned fom_mask = 0;
        unsigned count = 0;
        int bicolor = 0;

        struct PixelDumper
        {
            uint8_t bpp;
            uint8_t * buffer;
            uint32_t offset;
            uint32_t line_width;
            uint32_t height;
            PixelDumper(uint8_t bpp, uint32_t height, uint32_t line_width, size_t size)
                : bpp(bpp), buffer(0), offset(0), line_width(line_width), height(height)
            {
                this->buffer = (uint8_t*)malloc(size + 10);
                memset(this->buffer, 0, size);
            }
            ~PixelDumper()
            {
                free(this->buffer);
            }
            void out_dump_pixel(uint32_t pix)
            {
                ::out_bytes_le(&this->buffer[offset], this->bpp, pix);
                offset += nbbytes(this->bpp);
            }
            void dump(uint32_t start_offset)
            {
                if (start_offset % this->line_width == 0){
                    printf("// LINE %u\n", (unsigned)(this->height - 1 - (offset / this->line_width)));
                }

                if (start_offset % 16 != 0){
                    printf("// NEW %*c", 6 * (start_offset % 16), ' ');
                }
                else {
                    printf("// NEW ");
                }

                for (size_t offset = start_offset ; offset < this->offset ; offset++){
                    if ((offset % this->line_width == 0) && (offset > start_offset)){
                        printf("\n// LINE %u\n", (unsigned)(this->height - 1 - (offset / this->line_width)));
                    }

                    if ((offset % 16 == 0) && (offset > start_offset)){
                        printf("\n// NEW ");
                    }
                    printf("0x%.2x, ", this->buffer[offset]);
                }
                printf("\n");
            }
        } buffer(bpp, this->cy, this->line_size(bpp), this->bmp_size(bpp));

        printf("// DUMP OUPUT: cx=%u cy=%u line_width=%u bmp_size=%u\n", this->cx, this->cy, (unsigned)this->line_size(bpp), (unsigned)this->bmp_size(bpp));

        assert(nbbytes(bpp) <= 3);

        color1 = 0;
        color2 = 0;
        mix = 0xFFFFFFFF;

        enum {
            FILL    = 0,
            MIX     = 1,
            FOM     = 2,
            COLOR   = 3,
            COPY    = 4,
            MIX_SET = 6,
            FOM_SET = 7,
            BICOLOR = 8,
            SPECIAL_FGBG_1 = 9,
            SPECIAL_FGBG_2 = 10,
            WHITE = 13,
            BLACK = 14
        };

        uint8_t opcode;
        uint8_t lastopcode = 0xFF;

        while (input < end) {
            const uint8_t * last_input = input;
            uint32_t last_offset = buffer.offset;

            // Read RLE operators, handle short and long forms
            code = input[0]; input++;

            switch (code >> 4) {
            case 0xf:
                switch (code){
                    case 0xFD:
                        opcode = WHITE;
                        count = 1;
                    break;
                    case 0xFE:
                        opcode = BLACK;
                        count = 1;
                    break;
                    case 0xFA:
                        opcode = SPECIAL_FGBG_1;
                        count = 8;
                    break;
                    case 0xF9:
                        opcode = SPECIAL_FGBG_2;
                        count = 8;
                    break;
                    case 0xF8:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        count += count;
                        input += 2;
                    break;
                    default:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        input += 2;
                        // Opcodes 0xFB, 0xFC, 0xFF are some unknown orders of length 1 ?
                    break;
                }
            break;
            case 0x0e: // Bicolor, short form (1 or 2 bytes)
                opcode = BICOLOR;
                count = code & 0xf;
                if (!count){
                    count = input[0] + 16; input++;
                }
                count += count;
                break;
            case 0x0d:  // FOM SET, short form  (1 or 2 bytes)
                opcode = FOM_SET;
                count = code & 0x0F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x05:
            case 0x04:  // FOM, short form  (1 or 2 bytes)
                opcode = FOM;
                count = code & 0x1F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x0c: // MIX SET, short form (1 or 2 bytes)
                opcode = MIX_SET;
                count = code & 0x0f;
                if (!count){
                    count = input[0] + 16; input++;
                }
            break;
            default:
                opcode = code >> 5; // FILL, MIX, FOM, COLOR, COPY
                count = code & 0x1f;
                if (!count){
                    count = input[0] + 32; input++;
                }

                assert(opcode < 5);
                break;
            }

            /* Read preliminary data */
            switch (opcode) {
            case FOM:
                mask = 1;
                fom_mask = input[0]; input++;
                printf("// FOM %u [%.2x] ", count, fom_mask);
            break;
            case SPECIAL_FGBG_1:
                mask = 1;
                fom_mask = 7;
                printf("// SPECIAL_FGBG_1 %u [%.2x] ", count, fom_mask);
            break;
            case SPECIAL_FGBG_2:
                mask = 1;
                fom_mask = 3;
                printf("// SPECIAL_FGBG_2 %u [%.2x] ", count, fom_mask);
            break;
            case BICOLOR:

                bicolor = 0;
                color1 = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                color2 = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                printf("// BICOLOR %u %.6u %.6u", count, color1, color2);
                break;
            case COLOR:
                color2 = in_bytes_le(nbbytes(bpp), input);
                printf("// COLOR %u %.6u", count, color2);
                input += nbbytes(bpp);
                break;
            case MIX_SET:
                mix = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                printf("// MIX SET %u %.6u", count, mix);
            break;
            case FOM_SET:
                mix = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                mask = 1;
                fom_mask = input[0]; input++;
                printf("// FOM SET %u %.6u [%.2x] ", count, mix, fom_mask);
                break;
            case MIX:
                printf("// MIX %u", count);
                break;
            case FILL:
                printf("// FILL %u", count);
                break;
            case COPY:
                printf("// COPY %u", count);
                break;
            case WHITE:
                printf("// WHITE %u", count);
                break;
            case BLACK:
                printf("// BLACK %u", count);
                break;
            default: // for FILL, MIX or COPY nothing to do here
                break;
            }

            // MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
            if ((opcode == FILL)
            && (opcode == lastopcode)
            && (buffer.offset != this->line_size(bpp))){
                printf(" MAGIC MIX");
                if (buffer.offset < this->cx * nbbytes(bpp)){
                    yprev = 0;
                }
                else {
                    yprev = in_bytes_le(nbbytes(bpp), buffer.buffer+buffer.offset - this->cx * nbbytes(bpp));
                }
                buffer.out_dump_pixel(yprev ^ mix);
                count--;
            }
            lastopcode = opcode;

            /* Output body */
            while (count > 0) {
                if(buffer.offset >= this->bmp_size(bpp)) {
                    LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");
                    throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
                }
                if (buffer.offset < this->cx * nbbytes(bpp)){
                    yprev = 0;
                }
                else {
                    yprev = in_bytes_le(nbbytes(bpp), buffer.buffer+buffer.offset - this->cx * nbbytes(bpp));
                }
                switch (opcode) {
                case FILL:
                    buffer.out_dump_pixel(yprev);
                    break;
                case MIX_SET:
                case MIX:
                    buffer.out_dump_pixel(yprev ^ mix);
                    break;
                case FOM_SET:
                case FOM:
                    if (mask == 0x100){
                        mask = 1;
                        fom_mask = input[0]; input++;
                        printf(" [%.2x] ", fom_mask);
                    }
                case SPECIAL_FGBG_1:
                case SPECIAL_FGBG_2:
                    if (mask & fom_mask){
                        printf("1");
                        buffer.out_dump_pixel(yprev ^ mix);
                    }
                    else {
                        printf("0");
                        buffer.out_dump_pixel(yprev);
                    }
                    mask <<= 1;
                    if (mask == 0x10){
                        printf(" ");
                    }
                    break;
                case COLOR:
                    buffer.out_dump_pixel(color2);
                    break;
                case COPY:
                    buffer.out_dump_pixel(in_bytes_le(nbbytes(bpp), input));
                    input += nbbytes(bpp);
                    break;
                case BICOLOR:
                    if (bicolor) {
                        buffer.out_dump_pixel(color2);
                        bicolor = 0;
                    }
                    else {
                        buffer.out_dump_pixel(color1);
                        bicolor = 1;
                    }
                break;
                case WHITE:
                    buffer.out_dump_pixel(0xFFFFFFFF);
                break;
                case BLACK:
                    buffer.out_dump_pixel(0);
                break;
                default:
                    assert(false);
                    break;
                }
                count--;
            }
            printf("\n");
            for (const uint8_t * p = last_input ; p < input ; p++){
                printf("0x%.2x, ",*p);
            }
            printf("\n");
            buffer.dump(last_offset);
            printf("\n");

        }
        return;
    }

    void decompress(int bpp, const uint8_t* input, size_t size)
    {
//        printf("============================================\n");
//        printf("Compressed bitmap data\n");
//        for (size_t xxx = 0 ; xxx < size ; xxx++){
//            printf("0x%.2x,", input[xxx]);
//        }
//        printf("Decompressing bitmap done\n");
//        printf("============================================\n");

        uint8_t* pmin = this->data_co(bpp);
        uint8_t* pmax = pmin + this->bmp_size(bpp);
        unsigned yprev = 0;
        uint8_t* out = pmin;
        const uint8_t* end = input + size;
        unsigned color1;
        unsigned color2;
        unsigned mix;
        uint8_t code;
        unsigned mask = 0;
        unsigned fom_mask = 0;
        unsigned count = 0;
        int bicolor = 0;

        assert(nbbytes(bpp) <= 3);

        color1 = 0;
        color2 = 0;
        mix = 0xFFFFFFFF;

        enum {
            FILL    = 0,
            MIX     = 1,
            FOM     = 2,
            COLOR   = 3,
            COPY    = 4,
            MIX_SET = 6,
            FOM_SET = 7,
            BICOLOR = 8,
            SPECIAL_FGBG_1 = 9,
            SPECIAL_FGBG_2 = 10,
            WHITE = 13,
            BLACK = 14
        };

        uint8_t opcode;
        uint8_t lastopcode = 0xFF;

        while (input < end) {

            // Read RLE operators, handle short and long forms
            code = input[0]; input++;

            switch (code >> 4) {
            case 0xf:
                switch (code){
                    case 0xFD:
                        opcode = WHITE;
                        count = 1;
                    break;
                    case 0xFE:
                        opcode = BLACK;
                        count = 1;
                    break;
                    case 0xFA:
                        opcode = SPECIAL_FGBG_1;
                        count = 8;
                    break;
                    case 0xF9:
                        opcode = SPECIAL_FGBG_2;
                        count = 8;
                    break;
                    case 0xF8:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        count += count;
                        input += 2;
                    break;
                    default:
                        opcode = code & 0xf;
                        assert(opcode != 11 && opcode != 12 && opcode != 15);
                        count = input[0]|(input[1] << 8);
                        input += 2;
                        // Opcodes 0xFB, 0xFC, 0xFF are some unknown orders of length 1 ?
                    break;
                }
            break;
            case 0x0e: // Bicolor, short form (1 or 2 bytes)
                opcode = BICOLOR;
                count = code & 0xf;
                if (!count){
                    count = input[0] + 16; input++;
                }
                count += count;
                break;
            case 0x0d:  // FOM SET, short form  (1 or 2 bytes)
                opcode = FOM_SET;
                count = code & 0x0F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x05:
            case 0x04:  // FOM, short form  (1 or 2 bytes)
                opcode = FOM;
                count = code & 0x1F;
                if (count){
                    count <<= 3;
                }
                else {
                    count = input[0] + 1; input++;
                }
            break;
            case 0x0c: // MIX SET, short form (1 or 2 bytes)
                opcode = MIX_SET;
                count = code & 0x0f;
                if (!count){
                    count = input[0] + 16; input++;
                }
            break;
            default:
                opcode = code >> 5; // FILL, MIX, FOM, COLOR, COPY
                count = code & 0x1f;
                if (!count){
                    count = input[0] + 32; input++;
                }

                assert(opcode < 5);
                break;
            }

            /* Read preliminary data */
            switch (opcode) {
            case FOM:
                mask = 1;
                fom_mask = input[0]; input++;
            break;
            case SPECIAL_FGBG_1:
                mask = 1;
                fom_mask = 7;
            break;
            case SPECIAL_FGBG_2:
                mask = 1;
                fom_mask = 3;
            break;
            case BICOLOR:
                bicolor = 0;
                color1 = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                color2 = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                break;
            case COLOR:
                color2 = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                break;
            case MIX_SET:
                mix = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
            break;
            case FOM_SET:
                mix = in_bytes_le(nbbytes(bpp), input);
                input += nbbytes(bpp);
                mask = 1;
                fom_mask = input[0]; input++;
                break;
            default: // for FILL, MIX or COPY nothing to do here
                break;
            }

            // MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
            if ((opcode == FILL)
            && (opcode == lastopcode)
            && (out != pmin + this->line_size(bpp))){
                if (out - this->cx * nbbytes(bpp) < pmin){
                    yprev = 0;
                }
                else {
                     yprev = in_bytes_le(nbbytes(bpp), out - this->cx * nbbytes(bpp));
                }
                out_bytes_le(out, nbbytes(bpp), yprev ^ mix);
                count--;
                out+= nbbytes(bpp);
            }
            lastopcode = opcode;

            /* Output body */
            while (count > 0) {
                if(out >= pmax) {
                    LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");
                    throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
                }
                if (out - this->cx * nbbytes(bpp) < pmin){
                    yprev = 0;
                }
                else {
                    yprev = in_bytes_le(nbbytes(bpp), out - this->cx * nbbytes(bpp));
                }
                switch (opcode) {
                case FILL:
                    out_bytes_le(out, nbbytes(bpp), yprev);
                    break;
                case MIX_SET:
                case MIX:
                    out_bytes_le(out, nbbytes(bpp), yprev ^ mix);
                    break;
                case FOM_SET:
                case FOM:
                    if (mask == 0x100){
                        mask = 1;
                        fom_mask = input[0]; input++;
                    }
                case SPECIAL_FGBG_1:
                case SPECIAL_FGBG_2:
                    if (mask & fom_mask){
                        out_bytes_le(out, nbbytes(bpp), yprev ^ mix);
                    }
                    else {
                        out_bytes_le(out, nbbytes(bpp), yprev);
                    }
                    mask <<= 1;
                    break;
                case COLOR:
                    out_bytes_le(out, nbbytes(bpp), color2);
                    break;
                case COPY:
                    out_bytes_le(out, nbbytes(bpp), in_bytes_le(nbbytes(bpp), input));
                    input += nbbytes(bpp);
                    break;
                case BICOLOR:
                    if (bicolor) {
                        out_bytes_le(out, nbbytes(bpp), color2);
                        bicolor = 0;
                    }
                    else {
                        out_bytes_le(out, nbbytes(bpp), color1);
                        bicolor = 1;
                    }
                break;
                case WHITE:
                    out_bytes_le(out, nbbytes(bpp), 0xFFFFFFFF);
                break;
                case BLACK:
                    out_bytes_le(out, nbbytes(bpp), 0);
                break;
                default:
                    assert(false);
                    break;
                }
                count--;
                out += nbbytes(bpp);
            }
        }
        return;
    }


    unsigned get_pixel(int bpp, const uint8_t * const p) const
    {
        return in_bytes_le(nbbytes(bpp), p);
    }

    unsigned get_pixel_above(int bpp, const uint8_t * pmin, const uint8_t * const p)
    {
        return ((p-this->line_size(bpp)) < pmin)
        ? 0
        : this->get_pixel(bpp, p - this->line_size(bpp));
    }

    unsigned get_color_count(int bpp, uint8_t * pmax, const uint8_t * p, unsigned color)
    {
        unsigned acc = 0;
        while (p < pmax && get_pixel(bpp, p) == color){
            acc++;
            p = p + nbbytes(bpp);
        }
        return acc;
    }


    unsigned get_bicolor_count(int bpp, uint8_t * pmax, const uint8_t * p, unsigned color1, unsigned color2)
    {
        unsigned acc = 0;
        while ((p < pmax)
            && (color1 == get_pixel(bpp, p))
            && (p+nbbytes(bpp) < pmax)
            && (color2 == get_pixel(bpp, p+nbbytes(bpp)))) {
                acc = acc + 2;
                p = p+2*nbbytes(bpp);
        }
        return acc;
    }


    unsigned get_fill_count(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p)
    {
        unsigned acc = 0;
        while  (p + nbbytes(bpp) <= pmax) {
            unsigned pixel = this->get_pixel(bpp, p);
            unsigned ypixel = this->get_pixel_above(bpp, pmin, p);
            if (ypixel != pixel){
                break;
            }
            p = p+nbbytes(bpp);
            acc = acc + 1;
        }
        return acc;
    }


    unsigned get_mix_count(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p, unsigned foreground)
    {
        unsigned acc = 0;
        while (p < pmax){
            if (this->get_pixel_above(bpp, pmin, p) ^ foreground ^ this->get_pixel(bpp, p)){
                break;
            }
            p+= nbbytes(bpp);
            acc += 1;
        }
        return acc;
    }


    void get_fom_masks(int bpp, const uint8_t * pmin, const uint8_t * p, uint8_t * mask, const unsigned count)
    {
        unsigned i = 0;
        for (i = 0; i < count; i+=8)
        {
            mask[i>>3] = 0;
        }
        for (i = 0 ; i < count; i++, p += nbbytes(bpp))
        {
            if (get_pixel(bpp, p) != get_pixel_above(bpp, pmin, p)){
                mask[i>>3] |= (0x01 << (i & 7));
            }
        }
    }

    unsigned get_fom_count_set(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p, unsigned & foreground, unsigned & flags)
    {
        // flags : 1 = fill, 2 = MIX, 3 = (1+2) = FOM
        {
            unsigned fill_count = this->get_fill_count(bpp, pmin, pmax, p);

            if (fill_count >= 8) {
                flags = 1;
                return fill_count;
            }

            if (fill_count) {
                unsigned fom_count = this->get_fom_count_mix(bpp, pmin, pmax, p + fill_count * nbbytes(bpp), foreground);
                if (fom_count){
                    flags = 3;
                    return fill_count + fom_count;
                }
                else {
                    flags = 1;
                    return fill_count;
                }
            }
        }
        // fill_count and mix_count can't match at the same time.
        // this would mean that foreground is black, and we will never set
        // it to black, as it's useless because fill_count allready does that.
        // Hence it's ok to check them independently.
        {
            unsigned mix_count = 0;
            foreground = this->get_pixel_above(bpp, pmin, p) ^ this->get_pixel(bpp, p);
            if  (p < pmax) {
                mix_count = 1 + get_mix_count(bpp, pmin, pmax, p+nbbytes(bpp), foreground);
                if (mix_count >= 8) {
                    flags = 2;
                    return mix_count;
                }
                unsigned fom_count = this->get_fom_count_fill(bpp, pmin, pmax, p + mix_count * nbbytes(bpp), foreground);
                if (fom_count){
                    flags = 3;
                    return mix_count + fom_count;
                }
                else {
                    flags = 2;
                    return mix_count;
                }
            }
        }
        flags = 0;
        return 0;
    }

    unsigned get_fom_count(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p, unsigned foreground)
    {
        unsigned fill_count = this->get_fill_count(bpp, pmin, pmax, p);

        if (fill_count >= 8) {
            return 0;
        }

        if (fill_count) {
            unsigned fom_count = this->get_fom_count_mix(bpp, pmin, pmax, p + fill_count * nbbytes(bpp), foreground);
            return fom_count ? fill_count + fom_count : 0;

        }

        // fill_count and mix_count can't match at the same time.
        // this would mean that foreground is black, and we will never set
        // it to black, as it's useless because fill_count allready does that.
        // Hence it's ok to check them independently.

        unsigned mix_count = this->get_mix_count(bpp, pmin, pmax, p, foreground);

        if (mix_count >= 8) {
            return 0;
        }

        if (mix_count){
            unsigned fom_count = this->get_fom_count_fill(bpp, pmin, pmax, p + mix_count * nbbytes(bpp), foreground);
            return fom_count ? mix_count + fom_count : 0;
        }

        return 0;

    }

    #warning derecursive it
    unsigned get_fom_count_fill(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p, unsigned foreground)
    {

        if  (p < pmin || p >= pmax) {
            return 0;
        }

        unsigned fill_count = get_fill_count(bpp, pmin, pmax, p);

        if (fill_count >= 9) {
            return 0;
        }

        if (!fill_count){
            return 0;
        }

        return fill_count + this->get_fom_count_mix(bpp, pmin, pmax, p + fill_count * nbbytes(bpp), foreground);
    }


    #warning derecursive it
    unsigned get_fom_count_mix(int bpp, const uint8_t * pmin, uint8_t * pmax, const uint8_t * p, unsigned foreground)
    {
        unsigned mix_count = get_mix_count(bpp, pmin, pmax, p, foreground);

        if (mix_count >= 9) {
            return 0;
        }

        if (!mix_count){
            return 0;
        }

        return mix_count + this->get_fom_count_fill(bpp, pmin, pmax, p + mix_count * nbbytes(bpp), foreground);
    }

    #warning simplify and enhance compression using 1 pixel orders BLACK or WHITE.
    #warning keep allready compressed bitmaps in cache to avoid useless computations
    void compress(int bpp, Stream & out)
    {
        const uint8_t Bpp = nbbytes(bpp);
        uint8_t * oldp = 0;
//        LOG(LOG_INFO, "compressing bitmap from %u to %u", this->original_bpp, bpp);
        uint8_t * pmin = this->data_co(bpp);
        uint8_t * p = pmin;

        // white with the right length : either 0xFF or 0xFFFF or 0xFFFFFF
        unsigned foreground = ~(-1 << (nbbytes(bpp)*8));
        unsigned new_foreground = foreground;
        unsigned flags = 0;
        uint8_t masks[512];
        unsigned copy_count = 0;
        uint8_t * pmax = 0;

        uint32_t color = 0;
        uint32_t color2 = 0;

        #warning we should also check out_size and truncate if it overflow buffer or find another to ensure out buffer is large enough
        for (int part = 0 ; part < 2 ; part++){
            // As far as I can see the specs of bitmap RLE compressor is crap here
            // Fill orders between first scanline and all others must be splitted
            // (or on windows RDP clients black pixels are inserted at beginning of line,
            // on rdesktop this corner case works just fine)...
            // but if the first scanline contains two successive FILL or
            // if all the remaining scanlines contains two consecutive fill
            // orders, a magic MIX pixel is inserted between fills.
            // This explains the surprising loop above and the test below.pp
            if (part){
                pmax = pmin + this->bmp_size(bpp);
            }
            else {
                pmax = pmin + this->line_size(bpp);
            }
            while (p < pmax)
            {
                #warning remove this, not necessary any more
                if (oldp == p){
                    abort();
                }
                oldp = p;

                uint32_t fom_count = this->get_fom_count_set(bpp, pmin, pmax, p, new_foreground, flags);
                uint32_t color_count = 0;
                uint32_t bicolor_count = 0;

                if (p + nbbytes(bpp) < pmax){
                    color = get_pixel(bpp, p);
                    color2 = get_pixel(bpp, p+nbbytes(bpp));

                    if (color == color2){
                        color_count = get_color_count(bpp, pmax, p, color);
                    }
                    else {
                        bicolor_count = get_bicolor_count(bpp, pmax, p, color, color2);
                    }
                }

                const unsigned fom_cost = 1                      // header
                    + (foreground != new_foreground) * nbbytes(bpp) // set
                    + (flags == 3) * nbbytes(fom_count);         // mask
                const unsigned copy_fom_cost = 1 * (copy_count == 0) // start copy
                    + fom_count * nbbytes(bpp);                         // pixels
                const unsigned color_cost = 1 + nbbytes(bpp);
                const unsigned bicolor_cost = 1 + 2*nbbytes(bpp);

                if ((fom_count >= color_count || (color_count == 0))
                && ((fom_count >= bicolor_count) || (bicolor_count == 0) || (bicolor_count < 4))
                && fom_cost < copy_fom_cost) {
                    switch (flags){
                        case 3:
                            get_fom_masks(bpp, pmin, p, masks, fom_count);
                            if (new_foreground != foreground){
                                flags += 4;
                            }
                        break;
                        case 2:
                            if (new_foreground != foreground){
                                flags += 4;
                            }
                        break;
                        default:
                        break;
                    }
                }
                else {
                    unsigned copy_color_cost = (copy_count == 0) // start copy
                        + color_count * nbbytes(bpp);               // pixels
                    unsigned copy_bicolor_cost = (copy_count == 0) // start copy
                        + bicolor_count * nbbytes(bpp);               // pixels

                    if ((color_cost < copy_color_cost)
                    && (color_count > 0)){
                        flags = 8;
                    }
                    else if ((bicolor_cost < copy_bicolor_cost)
                    && (bicolor_count > 0)){
                        flags = 9;
                    }
                    else {
                        flags = 0;
                        copy_count++;
                    }
                }

                if (flags && copy_count > 0){
                    out.out_copy_sequence(Bpp, copy_count, p - copy_count * nbbytes(bpp));
                    copy_count = 0;
                }

                #warning use symbolic values for flags
                switch (flags){
                    case 9:
                        out.out_bicolor_sequence(Bpp, bicolor_count, color, color2);
                        p+= bicolor_count * nbbytes(bpp);
                    break;

                    case 8:
                        out.out_color_sequence(Bpp, color_count, color);
                        p+= color_count * nbbytes(bpp);
                    break;

                    case 7:
                        out.out_fom_sequence_set(nbbytes(bpp), fom_count, new_foreground, masks);
                        foreground = new_foreground;
                        p+= fom_count * nbbytes(bpp);
                    break;

                    case 6:
                        out.out_mix_count_set(fom_count);
                        out.out_bytes_le(nbbytes(bpp), new_foreground);
                        foreground = new_foreground;
                        p+= fom_count * nbbytes(bpp);
                    break;

                    case 3:
                        out.out_fom_sequence(fom_count, masks);
                        p+= fom_count * nbbytes(bpp);
                    break;

                    case 2:
                        out.out_mix_count(fom_count);
                        p+= fom_count * nbbytes(bpp);
                    break;

                    case 1:
                        out.out_fill_count(fom_count);
                        p+= fom_count * nbbytes(bpp);
                    break;

                    default:
                        p += nbbytes(bpp);
                    break;
                }
            }

            if (copy_count > 0){
                out.out_copy_sequence(Bpp, copy_count, p - copy_count * nbbytes(bpp));
                copy_count = 0;
            }
        }
    }


    uint32_t get_crc()
    {
        if (this->crc_computed){
            return this->crc;
        }
        #warning is this memory table really necessary for crc computing ? I undersatnd some dispersion of values is a good thing, but other simpler signing scheme should be more efficient with less memory access (maybe even md5 or aes).

        const static int crc_seed = 0xffffffff;
        const static int crc_table[256] = {
            0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
            0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
            0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
            0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
            0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
            0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
            0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
            0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
            0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
            0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
            0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
            0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
            0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
            0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
            0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
            0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
            0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
            0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
            0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
            0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
            0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
            0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
            0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
            0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
            0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
            0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
            0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
            0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
            0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
            0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
            0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
            0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
            0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
            0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
            0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
            0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
            0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
            0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
            0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
            0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
            0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
            0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
            0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
        };

        uint8_t Bpp = nbbytes(this->original_bpp);
        unsigned crc = crc_seed;
        unsigned width = this->cx * Bpp;
        const uint8_t *s8 = this->data_co(this->original_bpp);
        for (unsigned i = 0; i < this->cy; i++) {
            for (unsigned j = 0; j < width ; j++) {
                crc = crc_table[(crc ^ *s8++) & 0xff] ^ (crc >> 8);
            }
            s8 += this->line_size(this->original_bpp) - width;
        }
        this->crc = crc ^ crc_seed;
        crc_computed = true;
        return this->crc;
    }

    ~Bitmap(){
        if (this->data_co24){
            memset(this->data_co24, -1, this->bmp_size(24));
            free(this->data_co24);
        }
        if (this->data_co16){
            memset(this->data_co16, -1, this->bmp_size(16));
            free(this->data_co16);
        }
        if (this->data_co15){
            memset(this->data_co15, -1, this->bmp_size(15));
            free(this->data_co15);
        }
        if (this->data_co8){
            memset(this->data_co8, -1, this->bmp_size(8));
            free(this->data_co8);
        }
        this->data_co8 = this->data_co24 = this->data_co15 = this->data_co16 = 0;
        this->cx = 0;
        this->cy = 0;
        this->original_bpp = 0;
    }

    uint8_t * data_co(const int bpp) {
        uint8_t * dest = 0;
        switch (bpp) {
            case 24:
                dest = this->data_co24;
                break;
            case 16:
                dest = this->data_co16;
                break;
            case 15:
                dest = this->data_co15;
                break;
            case 8:
                dest = this->data_co8;
                break;
            default:
                throw Error(ERR_BITMAP_UNSUPPORTED_COLOR_DEPTH);
        }
        if (!dest) {
            dest = set_data_co(bpp);
            convert_data_co(bpp, dest);
        }
        return dest;
    }

    void convert_data_co(int out_bpp, uint8_t * dest) {

        uint8_t * src = data_co(this->original_bpp);

        #warning code below looks time consuming (applies to every pixels) and should probably be optimized
        // Color decode/encode
        #warning heavy optimization is possible here
        const uint8_t src_nbbytes = nbbytes(this->original_bpp);
        const uint8_t dest_nbbytes = nbbytes(out_bpp);
        for (size_t i = 0; i < this->cx * this->cy; i++) {
            uint32_t pixel = in_bytes_le(src_nbbytes, src);

            if (!(this->original_bpp == 8 && out_bpp == 8)){
                #warning is it the same on actual 24 bits server ? It may be a color inversion in widget layer
                pixel = color_decode(pixel, this->original_bpp, this->original_palette);
                if ((this->original_bpp == 24)
                && (out_bpp == 16 || out_bpp == 15 || out_bpp == 8)){
                    pixel = RGBtoBGR(pixel);
                }
                if ((this->original_bpp == 8)
                && (out_bpp == 16 || out_bpp == 15)){
                    pixel = RGBtoBGR(pixel);
                }
                pixel = color_encode(pixel, out_bpp);
            }

            out_bytes_le(dest, dest_nbbytes, pixel);
            src += src_nbbytes;
            dest += dest_nbbytes;
        }
    }

    // Initialize room for data_coXX.
    uint8_t * set_data_co(int bpp)
    {
        size_t size = this->bmp_size(bpp);
        switch (bpp) {
            case 24:
                return this->data_co24 = (uint8_t*)malloc(size);
            case 16:
                return this->data_co16 = (uint8_t*)malloc(size);
            case 15:
                return this->data_co15 = (uint8_t*)malloc(size);
            case 8:
                return this->data_co8  = (uint8_t*)malloc(size);
            default:
                throw Error(ERR_BITMAP_UNSUPPORTED_COLOR_DEPTH);
        }
        return (uint8_t *)0;
    }

    size_t line_size(const int bpp) const {
        return row_size(this->cx, bpp);
    }

    size_t bmp_size(const int bpp) const {
        #warning without this evil alignment we are experimenting problems with VNC bitmaps, but there should be a better fix.
        return row_size(align4(this->cx), bpp) * cy;
    }
};

#endif
