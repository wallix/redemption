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
#define BOOST_TEST_MODULE TestWidgetRect
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/widget_rect.hpp"
#include "internal/widget2/widget_composite.hpp"
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

    virtual void draw(const RDPOpaqueRect& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
    }

    virtual void draw(const RDPScrBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPDestBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPatBlt&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMemBlt& cmd, const Rect& rect, const Bitmap& bmp)
    {
        this->gd.draw(cmd, rect, bmp);
    }

    virtual void draw(const RDPLineTo&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphIndex&, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPBrushCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPColCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPGlyphCache&)
    {
        BOOST_CHECK(false);
    }

    virtual void begin_update()
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int, int, const char*, uint32_t, const Rect&)
    {
        BOOST_CHECK(false);
    }

    virtual void text_metrics(const char* , int& , int& )
    {
        BOOST_CHECK(false);
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetComposite)
{
    TestDraw drawable(800, 600);
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int id = 0;

    WidgetComposite wcomposite(&drawable, Rect(0,0,800,600), parent, notifier);
    WidgetRect wrect1(&drawable, Rect(0,0,100,100), &wcomposite, notifier, id,
                      YELLOW);
    WidgetRect wrect2(&drawable, Rect(0,100,100,100), &wcomposite, notifier, id,
                      RED);
    WidgetRect wrect3(&drawable, Rect(100,100,100,100), &wcomposite, notifier, id,
                      BLUE);
    WidgetRect wrect4(&drawable, Rect(300,300,100,100), &wcomposite, notifier, id,
                      GREEN);
    WidgetRect wrect5(&drawable, Rect(700,-50,100,100), &wcomposite, notifier, id,
                      WHITE);
    WidgetRect wrect6(&drawable, Rect(-50,550,100,100), &wcomposite, notifier, id,
                      GREY);
    wcomposite.child_list.push_back(&wrect1);
    wcomposite.child_list.push_back(&wrect2);
    wcomposite.child_list.push_back(&wrect3);
    wcomposite.child_list.push_back(&wrect4);
    wcomposite.child_list.push_back(&wrect5);
    wcomposite.child_list.push_back(&wrect6);

    // ask to widget to redraw at position 0,500 and of size 800x600
    wcomposite.rdp_input_invalidate(Rect(150, 150, wcomposite.cx(), wcomposite.cy()));


    drawable.save_to_png("/tmp/composite.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd3\x8f\x4e\xad\x90\x32\x11\x31\x40\xce"
        "\xd3\xbf\x5d\xb9\x3b\x99\x6b\x5c\xe9\xc8")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at position 0,500 and of size 100x100
    wcomposite.rdp_input_invalidate(Rect(0, 500, 100, 100));
    drawable.save_to_png("/tmp/composite2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x66\x62\xdd\xb6\x40\x72\x72\x4e\x8e\xdf"
        "\x5b\xcb\x5f\xc5\x6b\xb4\x60\x4b\x20\xd4")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    drawable.save_to_png("/tmp/composite3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x1b\x55\xcd\xb6\x7b\x78\x5d\x7b\x13\x79"
        "\xc5\x49\x36\x7b\x21\x84\x52\x19\x53\x35")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

