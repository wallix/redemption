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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWindowWabClose
#include <boost/test/auto_unit_test.hpp>

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_wab_close.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"


#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"


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

    virtual void draw(const RDPGlyphIndex&, const Rect&, const GlyphCache * gly_cache)
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
        this->gd.server_draw_text(x, y, text, fgcolor, bgcolor, clip, this->font);
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
            width -= 1;
        }
    }

    void save_to_png(const char * filename)
    {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.drawable.data, this->gd.drawable.width,
                   this->gd.drawable.height, this->gd.drawable.rowsize, true);
        fclose(file);
    }
};

BOOST_AUTO_TEST_CASE(TraceWindowWabClose)
{
    BOOST_CHECK(1);

    TestDraw drawable(800, 600);

    BOOST_CHECK(1);

    // WindowWabClose is a window_wab_close widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;
    int id = 0;

    BOOST_CHECK(1);

    try {

        WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def", id, "rec", "rec");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close.png");

    } catch (Error & e) {
        LOG(LOG_INFO, "e=%u", e.id);
    };

    BOOST_CHECK(1);

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x47\xde\x18\x14\x9b\x59\xf9\xb0\x98\xd9"
        "\x38\xab\xa8\xe6\xec\xa6\xca\x0c\x47\xc8")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose2)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
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
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf4\x69\x47\x81\xc8\xf8\x49\xbb\xbd\xd1"
        "\x3f\x73\x33\x87\x34\x7f\x14\x79\xb6\xc8")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose3)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x60\x89\x44\x36\xf0\x10\xe5\x80\x2a\x90"
        "\xf5\x42\xfd\x75\xbc\x55\xe1\x6a\xc9\xd4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose4)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8a\x32\xc4\x5c\x86\xd3\xd0\xbf\x16\x3b"
        "\xef\x03\xbb\x27\xf7\xd6\xa3\x4d\x2e\x40")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose5)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd4\x7c\x88\x20\x0c\xa2\x51\x42\x43\x01"
        "\x92\x16\x17\x36\x74\x43\xd8\x44\x5f\x93")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabClose6)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at it's current position
    window_wab_close.rdp_input_invalidate(window_wab_close.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf2\x57\xf1\x5c\x1b\x12\x16\x05\x4c\x76"
        "\x3a\xeb\x78\x83\x10\xfc\xce\xfd\x4a\x2b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabCloseClip)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    window_wab_close.rdp_input_invalidate(window_wab_close.rect.offset(20,0));

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd1\x64\x9a\x20\xe1\x84\xca\x7a\x3a\xb7"
        "\x55\x59\x14\x86\x3d\x42\xf8\x9c\xe3\xd9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowWabCloseClip2)
{
    TestDraw drawable(800, 600);

    // WindowWabClose is a window_wab_close widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowWabClose window_wab_close(drawable, x, y, parent, notifier,
                                    "abc<br>def");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    window_wab_close.rdp_input_invalidate(Rect(20 + window_wab_close.dx(),
                                               5 + window_wab_close.dy(),
                                               30,
                                               10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_wab_close8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x63\x57\xbf\x01\x37\xb5\x68\xd4\x65\xc8"
        "\x75\xb1\x94\x9c\x2e\xbd\x77\x50\x5b\xa0")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}
