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

#include "log.hpp"
#include "altoco.hpp"
#include "client_info.hpp"
#include "stream.hpp"
#include "colors.hpp"
#include <inttypes.h>

#include "ssl_calls.hpp"

#include "rect.hpp"


class Bitmap {

public:
    uint8_t original_bpp;
    BGRPalette original_palette;
    uint16_t cx;
    uint16_t cy;

    size_t line_size;
    size_t bmp_size;

    struct CountdownData {
        uint8_t * ptr;
        CountdownData() {
            this->ptr = 0;
        }
        ~CountdownData(){
            if (this->ptr){
                this->ptr[0]--;
                if (!this->ptr[0]){
                    free(this->ptr);
                }
            }
        }
        uint8_t * get() const {
            return this->ptr + 16;
        }
        void alloc(uint32_t size) {
            this->ptr = (uint8_t*)malloc(size+16);
            this->ptr[0] = 1;
        }
        void use(const CountdownData & other)
        {
            this->ptr = other.ptr;
            this->ptr[0]++;
        }

    } data_bitmap;

    Bitmap(uint8_t bpp, const BGRPalette * palette, uint16_t cx, uint16_t cy, const uint8_t * data, const size_t size, bool compressed=false)
        : original_bpp(bpp)
        , cx(align4(cx))
        , cy(cy)
        , line_size(this->cx * nbbytes(this->original_bpp))
        , bmp_size(this->line_size * cy)
        , data_bitmap()
    {
        this->data_bitmap.alloc(this->bmp_size);
//        LOG(LOG_ERR, "Creating bitmap (%p) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, size, bpp);
        if (bpp == 8){
            if (palette){
                memcpy(&this->original_palette, palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }

        if (compressed) {
            this->decompress(data, cx, cy, size);
        } else {
            uint8_t * dest = this->data_bitmap.get();
            const uint8_t * src = data;
            const size_t & data_width = cx * nbbytes(bpp);
            for (uint16_t i = 0 ; i < this->cy ; i++){
                memcpy(dest, src, data_width);
                bzero(dest + this->line_size, this->line_size - data_width);
                src += data_width;
                dest += this->line_size;
            }
        }
        if (this->cx <= 0 || this->cy <= 0){
            LOG(LOG_ERR, "Bogus empty bitmap!!! cx=%u cy=%u size=%u bpp=%u", this->cx, this->cy, size, this->original_bpp);
        }
    }


    Bitmap(const Bitmap & src_bmp, const Rect & r)
        : original_bpp(src_bmp.original_bpp)
        , cx(align4(r.cx))
        , cy(r.cy)
        , line_size(this->cx * nbbytes(this->original_bpp))
        , bmp_size(this->line_size * this->cy)
        , data_bitmap()
    {
        this->data_bitmap.alloc(this->bmp_size);

//        LOG(LOG_ERR, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);
        if (this->original_bpp == 8){
            memcpy(this->original_palette, src_bmp.original_palette, sizeof(BGRPalette));
        }

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        // In redemption we ensure a more constraint restriction to avoid padding
        // bitmap width must always be a multiple of 4

        const uint8_t Bpp = nbbytes(this->original_bpp);
        uint8_t *dest = this->data_bitmap.get();
        const uint8_t *src = src_bmp.data_bitmap.get() + src_bmp.line_size * (src_bmp.cy - r.y - this->cy) + r.x * Bpp;
        const unsigned line_to_copy = r.cx * nbbytes(src_bmp.original_bpp);

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(dest, src, line_to_copy);
            if (line_to_copy < this->line_size){
                bzero(dest + line_to_copy, this->line_size - line_to_copy);
            }
            src += src_bmp.line_size;
            dest += this->line_size;
        }
    }


    TODO("add palette support");
    Bitmap(const uint8_t * vnc_raw, uint16_t vnc_cx, uint16_t vnc_cy, uint8_t vnc_bpp, const Rect & tile)
        : original_bpp(vnc_bpp)
        , cx(align4(tile.cx))
        , cy(tile.cy)
        , line_size(align4(this->cx * nbbytes(this->original_bpp)))
        , bmp_size(this->line_size * this->cy)
        , data_bitmap()
    {
//        LOG(LOG_ERR, "Creating bitmap (%p) extracting part cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);

        this->data_bitmap.alloc(this->bmp_size);

        // raw: vnc data is a bunch of pixels of size cx * cy * nbbytes(bpp)
        // line 0 is the first line (top-up)

        // bitmapDataStream (variable): A variable-sized array of bytes.
        //  Uncompressed bitmap data represents a bitmap as a bottom-up,
        //  left-to-right series of pixels. Each pixel is a whole
        //  number of bytes. Each row contains a multiple of four bytes
        // (including up to three bytes of padding, as necessary).

        const uint8_t Bpp = nbbytes(this->original_bpp);
        const unsigned src_row_size = vnc_cx * Bpp;
        uint8_t *dest = this->data_bitmap.get();
        const uint8_t *src = vnc_raw + src_row_size * (tile.y + tile.cy - 1) + tile.x * Bpp;
        const uint16_t line_to_copy_size = tile.cx * Bpp;

        for (unsigned i = 0; i < this->cy; i++) {
            memcpy(dest, src, line_to_copy_size);
            if (line_to_copy_size < this->line_size){
                bzero(dest + line_to_copy_size, this->line_size - line_to_copy_size);
            }
            src -= src_row_size;
            dest += this->line_size;
        }
    }



    Bitmap(const char* filename)
        : original_bpp(24)
        , cx(0)
        , cy(0)
        , line_size(0)
        , bmp_size(0)
        , data_bitmap()
    {
        LOG(LOG_INFO, "loading bitmap %s", filename);
        BGRPalette palette1;
        char type1[4];

        /* header for bmp file */
        struct bmp_header {
            size_t size;
            unsigned image_width;
            unsigned image_height;
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

        TODO(" reading of file and bitmap decoding should be kept appart  putting both together makes testing hard. And what if I want to read a bitmap from some network socket instead of a disk file ?")
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
        TODO("define some stream aware function to read data from file (to update stream.end by itself). It should probably not be inside stream itself because read primitives are OS dependant, and there is not need to make stream OS dependant.")
        Stream stream(8192);
        if (read(fd, stream.data, 4) < 4){
            LOG(LOG_ERR, "Widget_load: error read file size\n");
            close(fd);
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        stream.end = stream.data + 4;
        {
            TODO("Check what is this size ? header size ? used as fixed below ?")
            /* uint32_t size = */ stream.in_uint32_le();
        }

        // skip some bytes to set file pointer to bmp header
        lseek(fd, 14, SEEK_SET);
        stream.init(8192);
        if (read(fd, stream.data, 40) < 40){
            close(fd);
            LOG(LOG_ERR, "Widget_load: error read file size (2)\n");
            throw Error(ERR_BITMAP_LOAD_FAILED);
        }
        stream.end = stream.data + 40;
        TODO(" we should read header size and use it to read header instead of using magic constant 40")
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
        TODO(" add support for loading of 16 bits bmp from file")
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
            stream.end = stream.data + header.clr_used * 4;
            for (int i = 0; i < header.clr_used; i++) {
                uint8_t r = stream.in_uint8();
                uint8_t g = stream.in_uint8();
                uint8_t b = stream.in_uint8();
                stream.in_skip_bytes(1); // skip alpha channel
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

        // read bitmap data
        {
            size_t size = (header.image_width * header.image_height * file_Qpp) / 2;
            stream.init(size);
            int row_size = (header.image_width * file_Qpp) / 2;
            int padding = align4(row_size) - row_size;
            for (unsigned y = 0; y < header.image_height; y++) {
                int k = read(fd, stream.data + y * row_size, row_size + padding);
                if (k != (row_size + padding)) {
                    LOG(LOG_ERR, "Widget_load: read error reading bitmap file [%s] read\n", filename);
                    close(fd);
                    throw Error(ERR_BITMAP_LOAD_FAILED);
                }
            }
            close(fd); // from now on all is in memory
            stream.end = stream.data + size;
        }

        const uint8_t Bpp = 3;
        this->cx = align4((uint16_t)(header.image_width));
        this->cy = (uint16_t)header.image_height;
        this->line_size = this->cx * Bpp;
        this->bmp_size = this->line_size * this->cy;

        this->data_bitmap.alloc(this->bmp_size);
        uint8_t * dest = this->data_bitmap.get();

        int k = 0;
        for (unsigned y = 0; y < this->cy ; y++) {
            for (unsigned x = 0 ; x < header.image_width; x++) {
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

                uint32_t px = color_decode(pixel, (uint8_t)header.bit_count, palette1);
                ::out_bytes_le(dest + y * this->line_size + x * Bpp, Bpp, px);
            }
            if (this->line_size > header.image_width * Bpp){
                bzero(dest + y * this->line_size + header.image_width * Bpp,
                      this->line_size - header.image_width * Bpp);
            }
        }
    }

    const uint8_t* data() const
    {
        return this->data_bitmap.get();
    }

    void dump() const
    {
        const uint8_t Bpp = nbbytes(this->original_bpp);
        LOG(LOG_INFO, "------- Dumping bitmap RAW data [%p]---------\n", this);
        LOG(LOG_INFO, "cx=%d cy=%d BPP=%d line_size=%d bmp_size=%d data=%p \n",
            this->cx, this->cy, Bpp, this->line_size, this->bmp_size, this->data_bitmap.get());
        assert(this->line_size);
        assert(this->cx);
        assert(this->cy);
        assert(this->bmp_size);

        LOG(LOG_INFO, "uint8_t raw%p[] = {", this);
        const uint8_t * data = this->data_bitmap.get();

        for (size_t j = 0 ; j < this->cy ; j++){
            LOG(LOG_INFO, "/* line %d */", (this->cy - j - 1));
            char buffer[2048];
            char * line = buffer;
            buffer[0] = 0;
            for (size_t i = 0; i < this->line_size; i++){
                line += snprintf(line, 1024, "0x%.2x, ", data[j*this->line_size+i]);
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
        LOG(LOG_INFO, "Bitmap bmp%p(%d, %d, %d, raw%p, sizeof(raw%p));",
            this, this->original_bpp, this->cx, this->cy, this, this);

//        LOG(LOG_INFO, "\n-----End of dump [%p] -----------------------\n", this);
    }

private:
    TODO("move that function to external definition")
    void decompress(const uint8_t* input, uint16_t src_cx, uint16_t src_cy, size_t size)
    {
//        printf("============================================\n");
//        printf("Compressed bitmap data\n");
//        for (size_t xxx = 0 ; xxx < size ; xxx++){
//            printf("0x%.2x,", input[xxx]);
//        }
//        printf("Decompressing bitmap done\n");
//        printf("============================================\n");

        const uint8_t Bpp = nbbytes(this->original_bpp);
        uint16_t & dst_cx = this->cx;
        uint8_t* pmin = this->data_bitmap.get();
        uint8_t* pmax = pmin + this->bmp_size;
        uint16_t out_x_count = 0;
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
                opcode = (uint8_t)(code >> 5); // FILL, MIX, FOM, COLOR, COPY
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
                color1 = in_bytes_le(Bpp, input);
                input += Bpp;
                color2 = in_bytes_le(Bpp, input);
                input += Bpp;
                break;
            case COLOR:
                color2 = in_bytes_le(Bpp, input);
                input += Bpp;
                break;
            case MIX_SET:
                mix = in_bytes_le(Bpp, input);
                input += Bpp;
            break;
            case FOM_SET:
                mix = in_bytes_le(Bpp, input);
                input += Bpp;
                mask = 1;
                fom_mask = input[0]; input++;
                break;
            default: // for FILL, MIX or COPY nothing to do here
                break;
            }

            // MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
            if ((opcode == FILL)
            && (opcode == lastopcode)
            && (out != pmin + this->line_size)){
                yprev = (out - this->line_size < pmin) ? 0 : in_bytes_le(Bpp, out - this->line_size);
                out_bytes_le(out, Bpp, yprev ^ mix);
                count--;
                out += Bpp;
                out_x_count += 1;
                if (out_x_count == dst_cx){
                    bzero(out, (dst_cx - src_cx) * Bpp);
                    out_x_count = 0;
                }
            }
            lastopcode = opcode;

            /* Output body */
            while (count > 0) {
                if(out >= pmax) {
                    LOG(LOG_WARNING, "Decompressed bitmap too large. Dying.");
                    throw Error(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE);
                }
                yprev = (out - this->line_size < pmin) ? 0 : in_bytes_le(Bpp, out - this->line_size);

                switch (opcode) {
                case FILL:
                    out_bytes_le(out, Bpp, yprev);
                    break;
                case MIX_SET:
                case MIX:
                    out_bytes_le(out, Bpp, yprev ^ mix);
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
                        out_bytes_le(out, Bpp, yprev ^ mix);
                    }
                    else {
                        out_bytes_le(out, Bpp, yprev);
                    }
                    mask <<= 1;
                    break;
                case COLOR:
                    out_bytes_le(out, Bpp, color2);
                    break;
                case COPY:
                    out_bytes_le(out, Bpp, in_bytes_le(Bpp, input));
                    input += Bpp;
                    break;
                case BICOLOR:
                    if (bicolor) {
                        out_bytes_le(out, Bpp, color2);
                        bicolor = 0;
                    }
                    else {
                        out_bytes_le(out, Bpp, color1);
                        bicolor = 1;
                    }
                break;
                case WHITE:
                    out_bytes_le(out, Bpp, 0xFFFFFFFF);
                break;
                case BLACK:
                    out_bytes_le(out, Bpp, 0);
                break;
                default:
                    assert(false);
                    break;
                }
                count--;
                out += Bpp;
                out_x_count += 1;
                if (out_x_count == dst_cx){
                    bzero(out, (dst_cx - src_cx) * Bpp);
                    out_x_count = 0;
                }
            }
        }
        return;
    }

public:
    unsigned get_pixel(const uint8_t Bpp, const uint8_t * const p) const
    {
        return in_bytes_le(Bpp, p);
    }

    unsigned get_pixel_above(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * const p) const
    {
        return ((p-this->line_size) < pmin)
        ? 0
        : this->get_pixel(Bpp, p - this->line_size);
    }

    unsigned get_color_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color) const
    {
        unsigned acc = 0;
        while (p < pmax && this->get_pixel(Bpp, p) == color){
            acc++;
            p = p + Bpp;
        }
        return acc;
    }


    unsigned get_bicolor_count(const uint8_t Bpp, const uint8_t * pmax, const uint8_t * p, unsigned color1, unsigned color2) const
    {
        unsigned acc = 0;
        while ((p < pmax)
            && (color1 == this->get_pixel(Bpp, p))
            && (p + Bpp < pmax)
            && (color2 == this->get_pixel(Bpp, p + Bpp))) {
                acc = acc + 2;
                p = p + 2 * Bpp;
        }
        return acc;
    }


    unsigned get_fill_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p) const
    {
        unsigned acc = 0;
        while  (p + Bpp <= pmax) {
            unsigned pixel = this->get_pixel(Bpp, p);
            unsigned ypixel = this->get_pixel_above(Bpp, pmin, p);
            if (ypixel != pixel){
                break;
            }
            p = p + Bpp;
            acc = acc + 1;
        }
        return acc;
    }


    unsigned get_mix_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
    {
        unsigned acc = 0;
        while (p < pmax){
            if (this->get_pixel_above(Bpp, pmin, p) ^ foreground ^ this->get_pixel(Bpp, p)){
                break;
            }
            p += Bpp;
            acc += 1;
        }
        return acc;
    }


    void get_fom_masks(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * p, uint8_t * mask, const unsigned count) const
    {
        unsigned i = 0;
        for (i = 0; i < count; i += 8)
        {
            mask[i>>3] = 0;
        }
        for (i = 0 ; i < count; i++, p += Bpp)
        {
            if (get_pixel(Bpp, p) != get_pixel_above(Bpp, pmin, p)){
                mask[i>>3] |= (uint8_t)(0x01 << (i & 7));
            }
        }
    }

    unsigned get_fom_count_set(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned & foreground, unsigned & flags) const
    {
        // flags : 1 = fill, 2 = MIX, 3 = (1+2) = FOM
        {
            unsigned fill_count = this->get_fill_count(Bpp, pmin, pmax, p);

            if (fill_count >= 8) {
                flags = 1;
                return fill_count;
            }

            if (fill_count) {
                unsigned fom_count = this->get_fom_count_mix(Bpp, pmin, pmax, p + fill_count * Bpp, foreground);
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
            foreground = this->get_pixel_above(Bpp, pmin, p) ^ this->get_pixel(Bpp, p);
            if  (p < pmax) {
                mix_count = 1 + this->get_mix_count(Bpp, pmin, pmax, p + Bpp, foreground);
                if (mix_count >= 8) {
                    flags = 2;
                    return mix_count;
                }
                unsigned fom_count = this->get_fom_count_fill(Bpp, pmin, pmax, p + mix_count * Bpp, foreground);
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

    unsigned get_fom_count(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
    {
        unsigned fill_count = this->get_fill_count(Bpp, pmin, pmax, p);

        if (fill_count >= 8) {
            return 0;
        }

        if (fill_count) {
            unsigned fom_count = this->get_fom_count_mix(Bpp, pmin, pmax, p + fill_count * Bpp, foreground);
            return fom_count ? fill_count + fom_count : 0;

        }

        // fill_count and mix_count can't match at the same time.
        // this would mean that foreground is black, and we will never set
        // it to black, as it's useless because fill_count allready does that.
        // Hence it's ok to check them independently.

        unsigned mix_count = this->get_mix_count(Bpp, pmin, pmax, p, foreground);

        if (mix_count >= 8) {
            return 0;
        }

        if (mix_count){
            unsigned fom_count = this->get_fom_count_fill(Bpp, pmin, pmax, p + mix_count * Bpp, foreground);
            return fom_count ? mix_count + fom_count : 0;
        }

        return 0;

    }

    TODO(" derecursive it")
    unsigned get_fom_count_fill(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
    {

        if  (p < pmin || p >= pmax) {
            return 0;
        }

        unsigned fill_count = this->get_fill_count(Bpp, pmin, pmax, p);

        if (fill_count >= 9) {
            return 0;
        }

        if (!fill_count){
            return 0;
        }

        return fill_count + this->get_fom_count_mix(Bpp, pmin, pmax, p + fill_count * Bpp, foreground);
    }


    TODO(" derecursive it")
    unsigned get_fom_count_mix(const uint8_t Bpp, const uint8_t * pmin, const uint8_t * pmax, const uint8_t * p, unsigned foreground) const
    {
        unsigned mix_count = this->get_mix_count(Bpp, pmin, pmax, p, foreground);

        if (mix_count >= 9) {
            return 0;
        }

        if (!mix_count){
            return 0;
        }

        return mix_count + this->get_fom_count_fill(Bpp, pmin, pmax, p + mix_count * Bpp, foreground);
    }

    TODO(" simplify and enhance compression using 1 pixel orders BLACK or WHITE.")
    TODO(" keep allready compressed bitmaps in cache to avoid useless computations")
    void compress(Stream & out) const
    {
        const uint8_t Bpp = nbbytes(this->original_bpp);
        const uint8_t * pmin = this->data_bitmap.get();
        const uint8_t * p = pmin;

        // white with the right length : either 0xFF or 0xFFFF or 0xFFFFFF
        unsigned foreground = ~(-1 << (Bpp*8));
        unsigned new_foreground = foreground;
        unsigned flags = 0;
        uint8_t masks[512];
        unsigned copy_count = 0;
        const uint8_t * pmax = 0;

        uint32_t color = 0;
        uint32_t color2 = 0;

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
                pmax = pmin + this->bmp_size;
            }
            else {
                pmax = pmin + row_size(this->cx, this->original_bpp);
            }
            while (p < pmax)
            {
                uint32_t fom_count = this->get_fom_count_set(Bpp, pmin, pmax, p, new_foreground, flags);
                uint32_t color_count = 0;
                uint32_t bicolor_count = 0;

                if (p + Bpp < pmax){
                    color = this->get_pixel(Bpp, p);
                    color2 = this->get_pixel(Bpp, p + Bpp);

                    if (color == color2){
                        color_count = this->get_color_count(Bpp, pmax, p, color);
                    }
                    else {
                        bicolor_count = this->get_bicolor_count(Bpp, pmax, p, color, color2);
                    }
                }

                const unsigned fom_cost = 1                      // header
                    + (foreground != new_foreground) * Bpp       // set
                    + (flags == 3) * nbbytes(fom_count);         // mask
                const unsigned copy_fom_cost = 1 * (copy_count == 0) // start copy
                    + fom_count * Bpp;                               // pixels
                const unsigned color_cost = 1 + Bpp;
                const unsigned bicolor_cost = 1 + 2*Bpp;

                if ((fom_count >= color_count || (color_count == 0))
                && ((fom_count >= bicolor_count) || (bicolor_count == 0) || (bicolor_count < 4))
                && fom_cost < copy_fom_cost) {
                    switch (flags){
                        case 3:
                            this->get_fom_masks(Bpp, pmin, p, masks, fom_count);
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
                        + color_count * Bpp;               // pixels
                    unsigned copy_bicolor_cost = (copy_count == 0) // start copy
                        + bicolor_count * Bpp;               // pixels

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
                    out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
                    copy_count = 0;
                }

                TODO(" use symbolic values for flags")
                switch (flags){
                    case 9:
                        out.out_bicolor_sequence(Bpp, bicolor_count, color, color2);
                        p+= bicolor_count * Bpp;
                    break;

                    case 8:
                        out.out_color_sequence(Bpp, color_count, color);
                        p+= color_count * Bpp;
                    break;

                    case 7:
                        out.out_fom_sequence_set(Bpp, fom_count, new_foreground, masks);
                        foreground = new_foreground;
                        p+= fom_count * Bpp;
                    break;

                    case 6:
                        out.out_mix_count_set(fom_count);
                        out.out_bytes_le(Bpp, new_foreground);
                        foreground = new_foreground;
                        p+= fom_count * Bpp;
                    break;

                    case 3:
                        out.out_fom_sequence(fom_count, masks);
                        p+= fom_count * Bpp;
                    break;

                    case 2:
                        out.out_mix_count(fom_count);
                        p+= fom_count * Bpp;
                    break;

                    case 1:
                        out.out_fill_count(fom_count);
                        p+= fom_count * Bpp;
                    break;

                    default:
                        p += Bpp;
                    break;
                }
            }

            if (copy_count > 0){
                out.out_copy_sequence(Bpp, copy_count, p - copy_count * Bpp);
                copy_count = 0;
            }
        }
    }

    void compute_sha1(uint8_t (&sig)[20]) const
    {
        SSL_SHA1 sha1;
        ssllib ssl;
        ssl.sha1_init(&sha1);
        uint16_t rowsize = (uint16_t)(this->cx * nbbytes(this->original_bpp));
        for (size_t y = 0; y < (size_t)this->cy; y++){
            ssl.sha1_update(&sha1, this->data_bitmap.get() + y * rowsize, rowsize);
        }
        ssl.sha1_final(&sha1, sig);
    }

    uint32_t compute_crc() const
    {
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
        const uint8_t *s8 = this->data_bitmap.get();
        for (unsigned i = 0; i < this->cy; i++) {
            for (unsigned j = 0; j < width ; j++) {
                crc = crc_table[(crc ^ *s8++) & 0xff] ^ (crc >> 8);
            }
            s8 += (this->bmp_size / this->cy) - width;
        }
        return crc ^ crc_seed;
    }

    ~Bitmap(){
    }

    Bitmap(uint8_t out_bpp, const Bitmap& bmp)
    : original_bpp(out_bpp)
    , cx(align4(bmp.cx))
    , cy(bmp.cy)
    , line_size(this->cx * nbbytes(this->original_bpp))
    , bmp_size(this->line_size * cy)
    , data_bitmap()
    {
//        LOG(LOG_ERR, "Creating bitmap (%p) (copy constructor) cx=%u cy=%u size=%u bpp=%u", this, cx, cy, bmp_size, original_bpp);

        if (out_bpp != bmp.original_bpp){
            this->data_bitmap.alloc(this->bmp_size);
            uint8_t * dest = this->data_bitmap.get();
            const uint8_t * src = bmp.data_bitmap.get();
            const uint8_t src_nbbytes = nbbytes(bmp.original_bpp);
            const uint8_t Bpp = nbbytes(out_bpp);

            for (size_t y = 0; y < bmp.cy ; y++) {
                for (size_t x = 0; x < bmp.cx ; x++) {
                    uint32_t pixel = in_bytes_le(src_nbbytes, src);

                    pixel = color_decode(pixel, bmp.original_bpp, bmp.original_palette);
                    if (out_bpp == 16 || out_bpp == 15 || out_bpp == 8){
                        pixel = RGBtoBGR(pixel);
                    }
                    pixel = color_encode(pixel, out_bpp);

                    out_bytes_le(dest, Bpp, pixel);
                    src += src_nbbytes;
                    dest += Bpp;
                }
                TODO("padding code should not be necessary as source bmp width is already aligned")
                if (this->line_size < bmp.cx * Bpp){
                    uint16_t padding = this->line_size - bmp.cx * Bpp;
                    bzero(dest, padding);
                    dest += padding;
                }
                TODO("padding code should not be necessary for source either as source bmp width is already aligned")
                src += bmp.line_size - bmp.cx * nbbytes(bmp.original_bpp);
            }
        }
        else {
            this->data_bitmap.use(bmp.data_bitmap);
        }

        if (out_bpp == 8){
            if (bmp.original_palette){
                memcpy(&this->original_palette, bmp.original_palette, sizeof(BGRPalette));
            }
            else {
                init_palette332(this->original_palette);
            }
        }
    }
};

#endif
