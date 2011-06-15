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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__NATIVECAPTURE_HPP__)
#define __NATIVECAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <time.h>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "error.hpp"
#include "config.hpp"
#include "bitmap_cache.hpp"
#include "colors.hpp"

class NativeCapture
{
    public:
    long inter_frame_interval;
    int width;
    int height;
    int bpp;
    RGBPalette palette;
    FILE * f;

    NativeCapture(int width, int height, int bpp, char * path) :
        width(width),
        height(height),
        bpp(bpp) {
        this->inter_frame_interval = 1000000; // 1 000 000 us is 1 sec (default)
        f = fopen(path, "w");
    }

    ~NativeCapture(){
        fclose(this->f);
    }

    void snapshot()
    {
        fprintf(this->f, "/* snapshot */\n");
        fflush(this->f);
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        #warning create a repr method in scr_blt
        fprintf(this->f, "    RDPScrBlt cmd(Rect(%u, %u, %u, %u), %u, %u, %u);\n",
            cmd.rect.x, cmd.rect.y, cmd.rect.cx, cmd.rect.cy,
            cmd.rop, cmd.srcx, cmd.srcy);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->scr_blt(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

    void mem_blt(const RDPMemBlt & cmd, const BitmapCache & bmp_cache, const Rect & clip)
    {
        #warning we always resend cache for now, it should be replaced by a true cache management in capture layer
        const Bitmap & bmp = *bmp_cache.get_item(cmd.cache_id, cmd.cache_idx)->pbmp;
        fprintf(this->f, "{\n");

        fprintf(this->f, "    // ------- Dumping bitmap RAW data [%p]---------\n", &bmp);
        fprintf(this->f, "    // cx=%d cy=%d bpp=%d line_size=%d bmp_size=%d data=%p pmax=%p\n",
            bmp.cx, bmp.cy, bmp.bpp, bmp.line_size, bmp.bmp_size, bmp.data_co, bmp.pmax);

        fprintf(this->f, "    uint8_t raw%p[] = {", &bmp);

        for (size_t j = 0 ; j < bmp.cy ; j++){
            fprintf(this->f, "    /* line %u */\n", (unsigned)(bmp.cy - j - 1));
            char buffer[2048];
            char * line = buffer;
            buffer[0] = 0;
            for (size_t i = 0; i < bmp.line_size; i++){
                line += snprintf(line, 1024, "0x%.2x, ", bmp.data_co[j*bmp.line_size+i]);
                if (i % 16 == 15){
                    fprintf(this->f, buffer);
                    fprintf(this->f, "\n");
                    line = buffer;
                    buffer[0] = 0;
                }
            }
            if (line != buffer){
                fprintf(this->f, buffer);
                fprintf(this->f, "\n");

            }
        }
        fprintf(this->f, "    }; /* %p */", &bmp);
        fprintf(this->f, "    Bitmap bmp%p(%d, %d, %d, raw%p, sizeof(raw%p));",
            &bmp, bmp.bpp, bmp.cx, bmp.cy, &bmp, &bmp);

        #warning create a repr method in mem_blt
        fprintf(this->f, "    RDPMemBlt cmd(%u, Rect(%u, %u, %u, %u), %u, %u, %u, %u);\n",
            cmd.cache_id, cmd.rect.x, cmd.rect.y, cmd.rect.cx, cmd.rect.cy,
            cmd.rop, cmd.srcx, cmd.srcy, cmd.cache_idx);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->mem_blt(cmd, bmp%p, this->clip);\n", &bmp);
        fprintf(this->f, "}\n");
    }

    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        #warning create a repr method in opaque_rect
        fprintf(this->f, "    RDPOpaqueRect cmd(Rect(%u, %u, %u, %u), 0x%.6x);\n",
            cmd.rect.x, cmd.rect.y, cmd.rect.cx, cmd.rect.cy,
            cmd.color);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->opaque_rect(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

    void dest_blt(const RDPDestBlt & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        #warning create a repr method in dest_blt
        fprintf(this->f, "    RDPDestBlt cmd(Rect(%u, %u, %u, %u), %u);\n",
            cmd.rect.x, cmd.rect.y, cmd.rect.cx, cmd.rect.cy,
            cmd.rop);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->dest_blt(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

    void pat_blt(const RDPPatBlt & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        #warning create a repr method in pat_blt
        fprintf(this->f, "    RDPPatBlt cmd(Rect(%u, %u, %u, %u), %u, 0x%.6x, 0x%.6x,\n "
            "        RDPBrush(%u, %u, %u, %u,"
            "            (const uint8_t *)\"\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\"));\n",
            cmd.rect.x, cmd.rect.y, cmd.rect.cx, cmd.rect.cy,
            cmd.rop, cmd.back_color, cmd.fore_color,
            cmd.brush.org_x, cmd.brush.org_y, cmd.brush.style, cmd.brush.hatch,
            cmd.brush.extra[0], cmd.brush.extra[1], cmd.brush.extra[2],
            cmd.brush.extra[3], cmd.brush.extra[4], cmd.brush.extra[5],
            cmd.brush.extra[6]);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->pat_blt(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

    void line_to(const RDPLineTo & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        #warning create a repr method in line_to
        fprintf(this->f, "    RDPLineTo cmd(%d, %d, %d, %d, 0x%.6x, %u, "
            "        RDPPen(%u, %u, 0x%.6x));\n",
            cmd.startx, cmd.starty, cmd.endx, cmd.endy,
            cmd.back_color, cmd.rop2,
            cmd.pen.style, cmd.pen.width, cmd.pen.color);
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->line_to(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

    void glyph_index(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        fprintf(this->f, "{\n");
        char * buffer = 0;
        if (cmd.data_len > 0){
            buffer = (char*)malloc(cmd.data_len);
            char * tmp = buffer;
            tmp[0] = '"';
            tmp++;
            size_t index = 0;
            tmp += sprintf(buffer+index*4+1, "\\x%.2x", cmd.data[index]);
            tmp[0] = '"';
            tmp[1] = 0;
        }
        fprintf(this->f, "    RDPGlyphIndex cmd(%u, %u, %u, %u, 0x%.6x, 0x%.6x,\n "
            "        Rect(%u, %u, %u, %u), Rect(%u, %u, %u, %u),\n"
            "        RDPBrush(%u, %u, %u, %u,"
            "            (const uint8_t *)\"\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\\x%x\"),\n"
            "        %u, %u, %u, (const uint8_t*)%s);",
            cmd.cache_id, cmd.fl_accel, cmd.ui_charinc, cmd.f_op_redundant,
            cmd.back_color, cmd.fore_color,
            cmd.bk.x, cmd.bk.y, cmd.bk.cx, cmd.bk.cy,
            cmd.op.x, cmd.op.y, cmd.op.cx, cmd.op.cy,
            cmd.brush.org_x, cmd.brush.org_y, cmd.brush.style, cmd.brush.hatch,
            cmd.brush.extra[0], cmd.brush.extra[1], cmd.brush.extra[2],
            cmd.brush.extra[3], cmd.brush.extra[4], cmd.brush.extra[5],
            cmd.brush.extra[6],
            cmd.glyph_x, cmd.glyph_y,
            cmd.data_len, buffer);
        #warning create a repr method in glyph_index
        fprintf(this->f, "    this->server_set_clip(Rect(%u, %u, %u, %u));\n",
                clip.x, clip.y, clip.cx, clip.cy);
        fprintf(this->f, "    this->front->glyph_index(cmd, this->clip);\n");
        fprintf(this->f, "}\n");
    }

};

#endif
