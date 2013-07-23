/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#define BOOST_TEST_MODULE TestWidgetSelector
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/selector.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

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
            width -= 2;
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

BOOST_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(&drawable, "x@127.0.0.1", w, h, notifier, "1", "1");

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP", "2013-04-20 19:56:50");

    selector.set_index_list(0);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x31\x4a\xc0\x25\x3f\x92\x51\x7a\x9b\xcc"
        "\x29\xd4\x74\x67\x39\x1d\x80\xc5\x7c\xb3")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.set_index_list(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x3e\x81\x4e\x08\x32\x8c\x51\x6c\x30\xc5"
        "\x06\x80\x9b\x77\x58\x9d\x82\x0b\x38\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelector2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,100 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(&drawable, "x@127.0.0.1", w, h, notifier, "1", "1");

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x0b\xe0\x2a\xcc\x42\x08\x9c\xd6\x20\xb5"
        "\x10\x44\x42\x51\x41\xfa\xc3\xfd\xbb\x2e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorClip)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 760,-7 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(&drawable, "x@127.0.0.1", w, h, notifier, "1", "1");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    //drawable.save_to_png("/tmp/selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\x88\xc8\x1e\xdf\xae\xf5\x3c\x70\xf2"
        "\x1b\xc7\x98\x58\x7f\x6e\x54\x74\xfb\x39")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorClip2)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(&drawable, "x@127.0.0.1", w, h, notifier, "1", "1");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      5 + selector.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/selector5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xcf\x78\x34\x58\x72\x1c\xcb\xc0\xe2\x50"
        "\xc9\xfd\x25\x83\xe9\xe4\x6f\x16\x5b\xb5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetSelectorEventSelect)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget of size 100x20 at position 10,7 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;

    WidgetSelector selector(&drawable, "x@127.0.0.1", w, h, notifier, "1", "1");

    selector.add_device("rdp", "qa\\administrateur@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.111",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.27",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.103",
                        "RDP", "2013-04-20 19:56:50");
    selector.add_device("rdp", "administrateur@qa@10.10.14.33",
                        "RDP", "2013-04-20 19:56:50");

    selector.set_index_list(0);

    selector.device_lines.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                          selector.device_lines.dx() + 20,
                                          selector.device_lines.dy() + 40,
                                          NULL);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x31\x4a\xc0\x25\x3f\x92\x51\x7a\x9b\xcc"
        "\x29\xd4\x74\x67\x39\x1d\x80\xc5\x7c\xb3")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x55\x2a\x79\x6e\x86\x9a\xfa\x74\xfd\x27"
        "\xce\x55\x59\x53\xcd\x41\x2f\x77\x9f\xce")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x55\x2a\x79\x6e\x86\x9a\xfa\x74\xfd\x27"
        "\xce\x55\x59\x53\xcd\x41\x2f\x77\x9f\xce")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-4.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x31\x4a\xc0\x25\x3f\x92\x51\x7a\x9b\xcc"
        "\x29\xd4\x74\x67\x39\x1d\x80\xc5\x7c\xb3")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-5.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x3e\x81\x4e\x08\x32\x8c\x51\x6c\x30\xc5"
        "\x06\x80\x9b\x77\x58\x9d\x82\x0b\x38\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    //drawable.save_to_png("/tmp/selector6-6.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x31\x4a\xc0\x25\x3f\x92\x51\x7a\x9b\xcc"
        "\x29\xd4\x74\x67\x39\x1d\x80\xc5\x7c\xb3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

