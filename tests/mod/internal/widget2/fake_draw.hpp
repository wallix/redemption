/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TESTS_MOD_INTERNAL_WIDGET2_FAKE_DRAW_HPP
#define REDEMPTION_TESTS_MOD_INTERNAL_WIDGET2_FAKE_DRAW_HPP

#include <draw_api.hpp>
#include <RDP/RDPDrawable.hpp>
#include <cstdio>

#ifdef IN_IDE_PARSER
# define FIXTURES_PATH
#endif

struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, 24)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
    {}

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseSC & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPEllipseCB & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip)
    {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache)
    {
        this->gd.draw(cmd, clip, gly_cache);
    }

    virtual void draw(const RDPBrushCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPColCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPGlyphCache & cmd)
    {
        this->gd.draw(cmd);
    }

    virtual void draw(const RDPPolygonSC & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolygonCB & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPolyline & cmd, const Rect & clip) {
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDP::FrameMarker& order)
    {
        this->gd.draw(order);
    }

    virtual void draw(const RDPBitmapData& bitmap_data, const uint8_t* data, size_t size, const Bitmap& bmp)
    {
        this->gd.draw(bitmap_data, data, size, bmp);
    }

    virtual void begin_update() {}

    virtual void end_update() {}

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        this->gd.text_metrics(text, width, height, this->font);
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

#endif
