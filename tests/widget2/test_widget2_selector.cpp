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
#define BOOST_TEST_MODULE TestWidgetSelector
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/selector.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetSelector)
{
    TestDraw drawable(800, 600);

    // WidgetSelector is a selector widget at position 0,0 in it's parent context
    NotifyApi * notifier = NULL;
    int16_t w = drawable.gd.drawable.width;
    int16_t h = drawable.gd.drawable.height;
    ModContext ctx;
    {
        //number of page = 1
        KeywordValue * keyvalue = new KeywordValue;
        keyvalue->value[0] = '1';
        keyvalue->value[1] = '\0';
        ModContext::t_kmap::iterator it = ctx.map.find(STRAUTHID_SELECTOR_NUMBER_OF_PAGES);
        if (it != ctx.map.end()) {
            ctx.map.erase(STRAUTHID_SELECTOR_NUMBER_OF_PAGES);
            delete it->second;
        }
        ctx.map.insert(ModContext::t_kmap::value_type(STRAUTHID_SELECTOR_NUMBER_OF_PAGES, keyvalue));
    }

    WidgetSelector selector(ctx, &drawable, "x@127.0.0.1", w, h, notifier);

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

    drawable.save_to_png("/tmp/selector1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x53\x77\x1b\x8d\x98\xa5\x9c\x26\x24\x9d"
        "\xec\x6a\xdf\x40\x30\x4a\xbb\xe7\xb2\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    selector.set_index_list(1);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xe9\x9d\x2a\x0c\x65\xbc\xed\x90\xcd\x20"
        "\xc0\xad\x1e\xb9\x07\x38\xf3\xa8\x74\x1c")){
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
    ModContext ctx;

    WidgetSelector selector(ctx, &drawable, "x@127.0.0.1", w, h, notifier);

    // ask to widget to redraw at it's current position
    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc6\x59\xff\x75\xd9\x86\x60\x6c\x08\xf2"
        "\x97\x9d\x64\xa5\xd0\x5c\x31\xfe\x79\x3d")){
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
    ModContext ctx;

    WidgetSelector selector(ctx, &drawable, "x@127.0.0.1", w, h, notifier);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      0 + selector.dy(),
                                      selector.cx(),
                                      selector.cy()));

    drawable.save_to_png("/tmp/selector4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xde\x54\xff\xa6\x19\xf2\x3b\xdf\x83\x1b"
        "\xdb\xc3\xa9\x2c\x44\x67\x15\x10\x07\x04")){
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
    ModContext ctx;

    WidgetSelector selector(ctx, &drawable, "x@127.0.0.1", w, h, notifier);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    selector.rdp_input_invalidate(Rect(20 + selector.dx(),
                                      5 + selector.dy(),
                                      30,
                                      10));

    drawable.save_to_png("/tmp/selector5.png");

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
    ModContext ctx;

    WidgetSelector selector(ctx, &drawable, "x@127.0.0.1", w, h, notifier);

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

    selector.account_device_lines.rdp_input_mouse(CLIC_BUTTON1_DOWN,
                                                  selector.account_device_lines.dx() + 20,
                                                  selector.account_device_lines.dy() + 40,
                                                  NULL);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc7\x1e\x28\x5b\x1e\xc6\xb5\xef\x6b\x19"
        "\x2c\xc1\x90\xf5\x66\xc8\xdd\x7e\x9a\xad")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xcc\x27\xcd\xed\xc7\x4b\x9d\xbe\xca\x2b"
        "\x7d\x5b\x20\x4b\x4a\xb5\xcd\x0f\xb7\x5c")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-3.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x27\x16\xb4\xf1\xb3\x49\xbb\x9b\x02\xd0"
        "\xe9\xeb\xf7\x11\x1f\x44\x22\xbe\x2c\xa7")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-4.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x8a\xee\x21\x73\xfc\x1e\xa2\x82\x95\x07"
        "\xda\xa3\xaf\x85\xf7\x4a\x02\x63\xa6\x98")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DOWN_ARROW);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-5.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xcc\x27\xcd\xed\xc7\x4b\x9d\xbe\xca\x2b"
        "\x7d\x5b\x20\x4b\x4a\xb5\xcd\x0f\xb7\x5c")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    selector.close_time_lines.rdp_input_scancode(0,0,0,0, &keymap);

    selector.rdp_input_invalidate(selector.rect);

    drawable.save_to_png("/tmp/selector6-6.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x8a\xee\x21\x73\xfc\x1e\xa2\x82\x95\x07"
        "\xda\xa3\xaf\x85\xf7\x4a\x02\x63\xa6\x98")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

