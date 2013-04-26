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
#define BOOST_TEST_MODULE TestWidgetMultiLine
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/multiline.hpp"
// #include "internal/widget2/widget_composite.hpp"
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
    {}

    virtual void end_update()
    {}

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, uint32_t bgcolor, const Rect& clip)
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

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

     //drawable.save_to_png("/tmp/multiline.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x13\xf6\x25\x98\x16\x57\x1d\x04\xa6\xfd"
        "\x42\x8c\x32\xba\x81\x8a\x3f\xe5\xad\xec")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine2)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x6c\xac\x5b\xc2\xd8\x65\xd6\x48\xd7\x6a"
        "\x90\xfe\x2f\x2e\xb4\x95\x82\xe3\x2e\x85")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfc\x18\xcd\x4c\x3a\x9a\x3b\x11\x3c\xa4"
        "\xb0\xf4\x1f\x13\x58\x2b\xf1\x6f\xed\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb6\x60\x93\xa4\x89\x79\xaf\xc7\x98\x88"
        "\xc8\x12\x04\x23\x48\xd8\x8c\xed\x4c\x7a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc2\x14\x1b\x6f\x29\xbe\x8c\xde\xb5\x8b"
        "\x65\xde\xab\xc8\x67\x56\x9c\x16\x40\x29")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x74\x16\x3e\x10\x81\x8e\x45\x33\xd6\x64"
        "\xf7\x6c\x05\xc5\x55\xed\x3e\x37\x4e\xb2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

     //drawable.save_to_png("/tmp/multiline7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe2\xd4\x45\x61\x0c\xca\x37\x3c\xcf\xd5"
        "\x67\x2a\xd6\x77\x0a\xb5\xe6\xaf\xba\xee")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineClip2)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         5 + wmultiline.dy(),
                                         30,
                                         10));

    //drawable.save_to_png("/tmp/multiline8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x33\xef\xc8\x21\x88\x76\xef\x04\xc1\x16"
        "\x20\x85\x4b\xb2\xa5\xf8\x01\x69\xf5\xf2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLineTooLong)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetMultiLine wmultiline(&drawable, x, y, parent, notifier,
                               "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                               "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                               "Nam non magna sit amet dui vestibulum feugiat.<br>"
                               "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                               "Nam lacinia purus luctus ante congue facilisis.<br>"
                               "Donec sodales mauris luctus ante ultrices blandit.<br>",
                               auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.rect);

    //drawable.save_to_png("/tmp/multiline9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2f\xda\xed\x89\x79\xca\x99\x59\x2c\x88"
        "\x04\x24\x41\x1f\x9a\x4a\x70\x36\x6e\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
