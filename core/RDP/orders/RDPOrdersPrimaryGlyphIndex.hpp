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
   Author(s): Christophe Grosjean

   New RDP Orders Coder / Decoder : Primary Order GlyphIndex

*/

#if !defined(__RDPORDERSPRIMARYGLYPHINDEX_HPP__)
#define __RDPORDERSPRIMARYGLYPHINDEX_HPP__

#include <malloc.h>

class RDPGlyphIndex {
// GLYPHINDEX_ORDER fields bytes
// ------------------------------
// 0x01: cache_id (font = ID of the glyph cache)
// 0x02: flAccel (flags)
// 0x04: UICharInc
// 0x08: fOpRedundant (mix_mode)
// 0x10: back_color
// 0x20: fore_color

// Coordinates of background
// 0x40: bk_left (x coordinate)
// 0x80: bk_top (y coordinate)
// 0x100: bk_right (x + cx coordinate)
// 0x200: bk_bottom (y + cy coordinate)

// Coordinates of opaque rectangle
// 0x400: op_left (x coordinate)
// 0x800: op_top (y coordinate)
// 0x1000: op_right (x + cx coordinate)
// 0x2000: op_bottom (y + cy coordinate)

// 0x4000 - 0x40000 : brush

// 0x80000 glyph_x
// 0x100000 glyph_y
// 0x200000 data_len, data

    public:
    uint8_t cache_id;
    int16_t fl_accel;
    int16_t ui_charinc;
    int16_t f_op_redundant;
    uint32_t back_color;
    uint32_t fore_color;
    Rect bk;
    Rect op;
    RDPBrush brush;
    int16_t glyph_x;
    int16_t glyph_y;
    uint8_t data_len;
    uint8_t * data;

    RDPGlyphIndex(uint8_t cache_id, uint8_t fl_accel, uint8_t ui_charinc,
        uint8_t f_op_redundant, uint32_t back_color, uint32_t fore_color,
        const Rect & bk, const Rect & op, const RDPBrush & brush,
        int16_t glyph_x, int16_t glyph_y,
        uint8_t data_len, const uint8_t * data) :
            cache_id(cache_id),
            fl_accel(fl_accel),
            ui_charinc(ui_charinc),
            f_op_redundant(f_op_redundant),
            back_color(back_color),
            fore_color(fore_color),
            bk(bk),
            op(op),
            brush(brush),
            glyph_x(glyph_x),
            glyph_y(glyph_y),
            data_len(data_len),
            data(0)
        {
            if (data_len > 0){
                this->data = (uint8_t *)malloc(data_len);
                memcpy(this->data, data, data_len);
            }
        }

    RDPGlyphIndex(const RDPGlyphIndex & gi) :
            cache_id(gi.cache_id),
            fl_accel(gi.fl_accel),
            ui_charinc(gi.ui_charinc),
            f_op_redundant(gi.f_op_redundant),
            back_color(gi.back_color),
            fore_color(gi.fore_color),
            bk(gi.bk),
            op(gi.op),
            brush(gi.brush),
            glyph_x(gi.glyph_x),
            glyph_y(gi.glyph_y),
            data_len(gi.data_len),
            data(0)
        {
            if (gi.data_len > 0){
                this->data = (uint8_t *)malloc(gi.data_len);
                memcpy(this->data, gi.data, gi.data_len);
            }
        }

    ~RDPGlyphIndex()
    {
        if (this->data){
//            free(this->data);
        }
    }
    bool operator==(const RDPGlyphIndex &other) const {
        return  (this->cache_id == other.cache_id)
             && (this->fl_accel == other.fl_accel)
             && (this->ui_charinc == other.ui_charinc)
             && (this->f_op_redundant == other.f_op_redundant)
             && (this->back_color == other.back_color)
             && (this->fore_color == other.fore_color)
             && (this->bk == other.bk)
             && (this->op == other.op)
             && (this->brush == other.brush)
             && (this->glyph_x == other.glyph_x)
             && (this->glyph_y == other.glyph_y)
             && (this->data_len == other.data_len)
             && (0 == memcmp(this->data, other.data, this->data_len))
             ;
    }

    void emit(Stream & stream,
            RDPOrderCommon & common,
            const RDPOrderCommon & oldcommon,
            const RDPGlyphIndex & oldcmd) const
    {
        using namespace RDP;
        RDPPrimaryOrderHeader header(STANDARD, 0);

        if (!common.clip.contains(this->bk)){
            header.control |= BOUNDS;
        }

        if (this->op.cx > 1 && !common.clip.contains(this->op)){
            header.control |= BOUNDS;
        }

        // DELTA flag Does not seems to works as usual for this function
        // (at least in rdesktop)
        // hence coordinates fields are transmitted as absolute fields
        // DELTA may also raise some problems with empty rects,
        // if initial rect is empty DELTA should not be used
//        control |=
//            (is_1_byte(this->bk.x               - oldcmd.bk.x                )
//          && is_1_byte(this->bk.y               - oldcmd.bk.y                )
//          && is_1_byte(this->bk.x + this->bk.cx - oldcmd.bk.x + oldcmd.bk.cx )
//          && is_1_byte(this->bk.y + this->bk.cy - oldcmd.bk.y + oldcmd.bk.cy )
//          && is_1_byte(this->op.x               - oldcmd.op.x                )
//          && is_1_byte(this->op.y               - oldcmd.op.y                )
//          && is_1_byte(this->op.x + this->op.cx - oldcmd.op.x + oldcmd.op.cx )
//          && is_1_byte(this->op.y + this->op.cy - oldcmd.op.y + oldcmd.op.cy )
//          && is_1_byte(this->glyph_x            - oldcmd.glyph_x             )
//          && is_1_byte(this->glyph_y            - oldcmd.glyph_y             )
//                                                                    ) * DELTA;

        header.fields =
            (this->cache_id           != oldcmd.cache_id            ) * 0x000001
          | (this->fl_accel           != oldcmd.fl_accel            ) * 0x000002
          | (this->ui_charinc         != oldcmd.ui_charinc          ) * 0x000004
          | (this->f_op_redundant     != oldcmd.f_op_redundant      ) * 0x000008

          | (this->back_color         != oldcmd.back_color          ) * 0x000010
          | (this->fore_color         != oldcmd.fore_color          ) * 0x000020

          | (this->bk.x               != oldcmd.bk.x                ) * 0x000040
          | (this->bk.y               != oldcmd.bk.y                ) * 0x000080
          | (this->bk.x + this->bk.cx != oldcmd.bk.x + oldcmd.bk.cx ) * 0x000100
          | (this->bk.y + this->bk.cy != oldcmd.bk.y + oldcmd.bk.cy ) * 0x000200

          | (this->op.x               != oldcmd.op.x                ) * 0x000400
          | (this->op.y               != oldcmd.op.y                ) * 0x000800
          | (this->op.x + this->op.cx != oldcmd.op.x + oldcmd.op.cx ) * 0x001000
          | (this->op.y + this->op.cy != oldcmd.op.y + oldcmd.op.cy ) * 0x002000

          | (this->brush.org_x        != oldcmd.brush.org_x         ) * 0x004000
          | (this->brush.org_y        != oldcmd.brush.org_y         ) * 0x008000
          | (this->brush.style        != oldcmd.brush.style         ) * 0x010000
          | (this->brush.hatch        != oldcmd.brush.hatch         ) * 0x020000
          | (memcmp(this->brush.extra, oldcmd.brush.extra, 7) != 0  ) * 0x040000

          | (this->glyph_x            != oldcmd.glyph_x             ) * 0x080000
          | (this->glyph_y            != oldcmd.glyph_y             ) * 0x100000
          | (data_len                 != 0                          ) * 0x200000
          ;

        common.emit(stream, header, oldcommon);

        if (header.fields & 0x001) { stream.out_uint8(this->cache_id); }
        if (header.fields & 0x002) { stream.out_uint8(this->fl_accel); }
        if (header.fields & 0x004) { stream.out_uint8(this->ui_charinc); }
        if (header.fields & 0x008) { stream.out_uint8(this->f_op_redundant); }
        if (header.fields & 0x010) {
            stream.out_uint8(this->back_color);
            stream.out_uint8(this->back_color >> 8);
            stream.out_uint8(this->back_color >> 16);
        }
        if (header.fields & 0x020) {
            stream.out_uint8(this->fore_color);
            stream.out_uint8(this->fore_color >> 8);
            stream.out_uint8(this->fore_color >> 16);
        }

        header.emit_coord(stream, 0x0040, this->bk.x, oldcmd.bk.x);
        header.emit_coord(stream, 0x0080, this->bk.y, oldcmd.bk.y);
        header.emit_coord(stream, 0x0100, this->bk.x + this->bk.cx - 1,
                                         oldcmd.bk.x + oldcmd.bk.cx - 1);
        header.emit_coord(stream, 0x0200, this->bk.y + this->bk.cy - 1,
                                         oldcmd.bk.y + oldcmd.bk.cy - 1);

        header.emit_coord(stream, 0x0400, this->op.x, oldcmd.op.x);
        header.emit_coord(stream, 0x0800, this->op.y, oldcmd.op.y);
        header.emit_coord(stream, 0x1000, this->op.x + this->op.cx - 1,
                                         oldcmd.op.x + oldcmd.op.cx - 1);
        header.emit_coord(stream, 0x2000, this->op.y + this->op.cy - 1,
                                         oldcmd.op.y + oldcmd.op.cy - 1);

        header.emit_brush(stream, 0x4000, this->brush, oldcmd.brush);

        header.emit_coord(stream, 0x080000, this->glyph_x, oldcmd.glyph_x);
        header.emit_coord(stream, 0x100000, this->glyph_y, oldcmd.glyph_y);

        if (header.fields & 0x200000) {
            stream.out_uint8(this->data_len);
            stream.out_copy_bytes(this->data, this->data_len);
        }

    }

    void receive(Stream & stream, const RDPPrimaryOrderHeader & header)
    {
        using namespace RDP;

        if (header.fields & 0x001) { this->cache_id = stream.in_uint8(); }
        if (header.fields & 0x002) { this->fl_accel = stream.in_uint8(); }
        if (header.fields & 0x004) { this->ui_charinc = stream.in_uint8(); }
        if (header.fields & 0x008) { this->f_op_redundant = stream.in_uint8(); }

        if (header.fields & 0x010) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->back_color = r + (g << 8) + (b << 16);
        }

        if (header.fields & 0x020) {
            uint8_t r = stream.in_uint8();
            uint8_t g = stream.in_uint8();
            uint8_t b = stream.in_uint8();
            this->fore_color = r + (g << 8) + (b << 16);
        }

        int16_t bk_left = this->bk.x;
        int16_t bk_top = this->bk.y;
        int16_t bk_right = this->bk.x + this->bk.cx - 1;
        int16_t bk_bottom = this->bk.y + this->bk.cy - 1;

        header.receive_coord(stream, 0x0040, bk_left);
        header.receive_coord(stream, 0x0080, bk_top);
        header.receive_coord(stream, 0x0100, bk_right);
        header.receive_coord(stream, 0x0200, bk_bottom);

        this->bk.x = bk_left;
        this->bk.y = bk_top;
        this->bk.cx = bk_right - this->bk.x + 1;
        this->bk.cy = bk_bottom - this->bk.y + 1;

        int16_t op_left = this->op.x;
        int16_t op_top = this->op.y;
        int16_t op_right = this->op.x + this->op.cx - 1;
        int16_t op_bottom = this->op.y + this->op.cy - 1;

        header.receive_coord(stream, 0x0400, op_left);
        header.receive_coord(stream, 0x0800, op_top);
        header.receive_coord(stream, 0x1000, op_right);
        header.receive_coord(stream, 0x2000, op_bottom);

        this->op.x = op_left;
        this->op.y = op_top;
        this->op.cx = op_right - this->op.x + 1;
        this->op.cy = op_bottom - this->op.y + 1;

        header.receive_brush(stream, 0x4000, this->brush);

        header.receive_coord(stream, 0x080000, this->glyph_x);
        header.receive_coord(stream, 0x100000, this->glyph_y);

        if (header.fields & 0x200000){
            this->data_len = stream.in_uint8();
            if (this->data){
                free(this->data);
            }
            this->data = (uint8_t *)malloc(this->data_len);
            memcpy(this->data, stream.in_uint8p(this->data_len), this->data_len);
        }
    }

    size_t str(char * buffer, size_t sz, const RDPOrderCommon & common) const
    {
        size_t lg = common.str(buffer, sz);
        lg += snprintf(
            buffer+lg,
            sz-lg,
            "glyphIndex(cache_id=%.2x "
                "fl_accel=%d ui_charinc=%d f_op_redundant=%d "
                "back_color=%.6x fore_color=%.6x "
                "bk=(%d,%d,%d,%d) "
                "op=(%d,%d,%d,%d) "
                "brush.(org_x=%.2x, org_y=%.2x, style=%d hatch=%d extra=[%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x]) "
                "glyph_x=%.2x glyph_y=%.2x data_len=%d "
                "\n",
            this->cache_id,
            this->fl_accel,
            this->ui_charinc,
            this->f_op_redundant,
            this->back_color,
            this->fore_color,
            this->bk.x, this->bk.y, this->bk.cx, this->bk.cy,
            this->op.x, this->op.y, this->op.cx, this->op.cy,
            this->brush.org_x,this->brush.org_y,this->brush.style,this->brush.hatch,
            this->brush.extra[0],this->brush.extra[1],this->brush.extra[2],this->brush.extra[3],
            this->brush.extra[4],this->brush.extra[5],this->brush.extra[6],
            this->glyph_x, this->glyph_y,
            this->data_len);
        lg += snprintf(buffer+lg, sz-lg,"[");
        for (int i = 0; i < this->data_len; i++){
            lg += snprintf(buffer+lg, sz-lg,"%.2x(%c) ",
            (char)this->data[i],
            this->data[i] & 0x80 ? ' ':(char)this->data[i]);
        }
        lg += snprintf(buffer+lg, sz-lg,"]");
        if (lg >= sz){
            return sz;
        }
        return lg;
    }
};

#endif
