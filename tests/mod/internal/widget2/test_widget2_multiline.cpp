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

struct TestDraw : DrawApi
{
    RDPDrawable gd;
    Font font;

    TestDraw(uint16_t w, uint16_t h)
    : gd(w, h)
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

    virtual void draw(const RDPMem3Blt& cmd, const Rect& rect, const Bitmap& bmp)
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
                   this->gd.drawable.height, this->gd.drawable.rowsize, false);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
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
                               auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xee\xa2\x81\x4c\x50\xf0\x0d\x1e\x13\x42\x3e\xa2\x08\xf8\xc6\x7c\xea\x1d\x84\x87"
    )){
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xaf\xf6\x8b\xe7\x3e\xe8\x92\x70\x72\xb8\xcd\x9f\x91\x79\xdf\xf0\x21\x45\xc3\x49"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine3)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x2c\xca\x26\x71\x79\xec\x2d\x34\xa5\x49\xe9\xc2\xd8\x50\x6c\xda\x50\xb7\xfb\x41"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine4)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\xf4\x3a\x68\xcd\xe4\x9d\x3f\xc4\x9c\x29\x96\x88\x7f\xcc\xf5\x2a\xb0\x6e\xa1\xeb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine5)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x67\x7a\x68\x4e\x01\x53\xbf\xa1\x6b\x23\x0f\x73\xb9\x31\x75\x36\xe5\xf5\xcb\x9a"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetMultiLine6)
{
    TestDraw drawable(800, 600);

    // WidgetMultiLine is a multiline widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wmultiline.rdp_input_invalidate(Rect(0 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x7f\xc3\xfa\x61\x18\x42\x67\x11\xac\xa2\xbd\xc6\x7f\x75\x1c\x48\xb4\x38\x30\x92"
    )){
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         0 + wmultiline.dy(),
                                         wmultiline.cx(),
                                         wmultiline.cy()));

    //drawable.save_to_png("/tmp/multiline7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x34\x03\xc1\x93\x3a\x23\xba\xc5\x4e\x7f\x18\x80\x13\xdd\x4c\x32\x2e\x70\x29\xc6"
    )){
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
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(Rect(20 + wmultiline.dx(),
                                         5 + wmultiline.dy(),
                                         30,
                                         10));

    //drawable.save_to_png("/tmp/multiline8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x66\xfe\x9d\x60\x01\x62\xb7\xd7\xe6\x33\x32\x65\x67\x5a\xa8\xc2\x04\xf5\xbf\x28"
    )){
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
                               "Donec sodales mauris luctus ante ultrices blandit.",
                               auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wmultiline.rdp_input_invalidate(wmultiline.rect);

    //drawable.save_to_png("/tmp/multiline9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
    "\x91\xe3\x36\xd1\x6d\xa0\x36\xae\xaf\x26\x5d\x73\x24\x50\x0c\x6e\xad\x59\x50\x52"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
