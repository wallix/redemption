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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetNotify
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include <widget2/widget_rect.hpp>
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : ModApi
{
    RDPDrawable gd;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, true)
    {}

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
    {
        gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
    {
        gd.draw(cmd, clip);
    }

    virtual void draw(const RDPBrushCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void draw(const RDPColCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void draw(const RDPGlyphCache& cmd)
    {
        gd.draw(cmd);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int x, int y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
    {}

    virtual void text_metrics(const char* , int& , int& )
    {}

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetRect)
{
    TestDraw drawable(800, 600);
    WidgetRect wrect(&drawable, Rect(0,0,800,600), 0, 0, 0, 0, 325324);

    wrect.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/rect.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xea\xe1\x3b\x4b\xdb\xda\xa6\x75\xf1\x17"
        "\xa2\xe8\x09\xf1\xd2\x42\x7a\xdf\x85\x6d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    WidgetRect wrect2(&drawable, Rect(100,200,70,100), &wrect, 0, 0, 0, 0);

    wrect2.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/rect2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xab\xe3\xa7\x2a\x52\xf3\xa0\xc6\xaa\x75"
        "\x9d\xe9\x98\xd7\x92\x59\xd4\xd9\x11\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}