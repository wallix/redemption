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
#define BOOST_TEST_MODULE TestWindowWabClose
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_wab_close.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWindowWabClose)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget at position 0,0 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;
    int id = 0;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def", id, "rec", "rec");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    drawable.save_to_png("/tmp/window_wab_close.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xdc\xb7\x3c\x08\xeb\xdf\xb3\xa1\xa2\x9a"
        "\xce\x2d\x6f\x73\x22\x2c\x26\xcf\x02\x01")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose2)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
        "Lorem ipsum dolor sit amet, consectetur<br>"
        "adipiscing elit. Nam purus lacus, luctus sit<br>"
        "amet suscipit vel, posuere quis turpis. Sed<br>"
        "venenatis rutrum sem ac posuere. Phasellus<br>"
        "feugiat dui eu mauris adipiscing sodales.<br>"
        "Mauris rutrum molestie purus, in tempor lacus<br>"
        "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
        "est. Vestibulum in nunc vel massa condimentum<br>"
        "iaculis nec in arcu. Pellentesque accumsan,<br>"
        "quam sit amet aliquam mattis, odio purus<br>"
        "porttitor tortor, sit amet tincidunt odio<br>"
        "erat ut ligula. Fusce sit amet mauris neque.<br>"
        "Sed orci augue, luctus in ornare sed,<br>"
        "adipiscing et arcu."
    );

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(Rect(0 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x06\xc2\xde\x8a\xa6\x4f\x43\x85\x1c\x57"
        "\xe5\x1a\xb7\x90\x60\x44\x36\x6a\x57\x78")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose3)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(Rect(0 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x40\x2a\x2a\xe3\xfa\xaa\xa2\xbc\x09\x0b"
        "\x32\x1e\x01\xe2\xd2\x1d\x7e\x51\x2c\xe2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose4)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(Rect(0 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7d\x2b\xac\x12\x29\xed\xb6\x55\x13\x10"
        "\xd6\xe5\x69\xd8\x03\x7a\x7b\xa7\xe3\x2f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose5)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(Rect(0 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x37\x43\x15\xf6\x04\x8a\x5b\xe4\xbc\xba"
        "\xa8\x15\xbd\xc8\x5d\xc0\x79\xb3\x02\x3f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose6)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(Rect(0 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x38\xdc\xd0\x01\x3d\x25\x77\xe8\xe5\xea"
        "\x5e\x46\x85\xf3\xda\xdf\x49\x3c\xc9\xa2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabCloseClip)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    window_wab_close.rdp_input_invalidate(Rect(20 + window_wab_close.dx(),
                                      0 + window_wab_close.dy(),
                                      window_wab_close.cx(),
                                      window_wab_close.cy()));

    drawable.save_to_png("/tmp/window_wab_close7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe5\x71\x2d\xbb\x25\xc5\x04\xe0\x2f\x9c"
        "\x52\x59\xa0\xbf\x8c\x75\x77\x81\x7a\x45")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabCloseClip2)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 10,7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowWabClose window_wab_close(&drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    window_wab_close.rdp_input_invalidate(Rect(20 + window_wab_close.dx(),
                                      5 + window_wab_close.dy(),
                                      30,
                                      10));

    drawable.save_to_png("/tmp/window_wab_close8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xab\xc0\xb8\xff\x4f\xd0\x3b\x60\x22\x8b"
        "\x9e\x38\x11\xff\xbd\x81\xf3\x81\x62\x5d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

