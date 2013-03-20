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

#include <widget2/label.hpp>
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : ModApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h, true)
    , font(FIXTURES_PATH "/dejavu-sans-10.fv1")
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
    {
        BOOST_CHECK(false);
    }

    virtual void end_update()
    {
        BOOST_CHECK(false);
    }

    virtual void server_draw_text(int x, int y, const char* text, uint32_t fgcolor, const Rect& clip)
    {
        this->gd.server_draw_text(x, y, text, fgcolor, clip, this->font);
    }

    virtual void text_metrics(const char* text, int& width, int& height)
    {
        height = 0;
        width = 0;
        uint32_t uni[256];
        size_t len_uni = UTF8toUnicode(reinterpret_cast<const uint8_t *>(text), uni, sizeof(uni)/sizeof(uni[0]));
        if (len_uni){
            for (size_t index = 0; index < len_uni; index++) {
                FontChar *font_item = this->gd.get_font(this->font, uni[index]);
                width += font_item->width + 2;
                height = std::max(height, font_item->height);
            }
            if (len_uni > 1)
                width -= 2;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetLabel)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 0,0 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(0,0, 100,20), parent, notifier, "test1", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa2\x84\xf8\x80\x33\xbb\x82\x4c\x64\x0c"
        "\xd8\x2e\x0b\x25\xd5\x37\x62\x23\x6f\x46")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel2)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(10,100, 100,20), parent, notifier, "test2", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x26\xed\x54\x49\x4e\x28\x20\xe8\x10\xd1"
        "\xf7\x93\x43\x8b\x1c\xb8\x25\x71\x3e\x88")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel3)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(-10,500, 100,20), parent, notifier, "test3", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8a\xea\xb9\xf8\x56\x84\xa5\xd4\x18\x56"
        "\x19\xb5\xe3\x33\xeb\x74\xd2\x12\xaf\x2c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel4)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(770,500, 100,20), parent, notifier, "test4", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x02\x9b\xb1\xe3\xe6\xd8\x06\xf3\x5a\xcc"
        "\x0b\xec\x3c\x3d\x2f\x79\xb4\xbb\xc3\xa1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel5)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(-20,-7, 100,20), parent, notifier, "test5", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfe\x16\xc3\x5c\xab\x20\xb5\x12\x95\xfe"
        "\x80\x9c\x0f\x0d\xc8\x79\x13\x12\x25\x01")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetLabel6)
{
    TestDraw drawable(800, 600);

    // WidgetLabel is a label widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;

    WidgetLabel wlabel(&drawable, Rect(760,-7, 100,20), parent, notifier, "test6", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wlabel.send_event(WM_DRAW, 0, 0, 0);

    drawable.save_to_png("/tmp/label6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa4\x90\xbc\xb1\x0f\x1b\xd7\x3f\x40\x48"
        "\x0f\x1e\xd9\x32\x31\x5e\xc2\x31\x6b\xab")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

