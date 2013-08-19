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
#define BOOST_TEST_MODULE TestWidgetEdit
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/edit.hpp"
#include "internal/widget2/composite.hpp"
// #include "internal/widget2/widget_composite.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, edit_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dx(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2f\x3c\xc6\x6e\x95\xda\xe3\x40\x8a\xdb"
        "\xf2\xf3\x86\xe6\xf1\x43\x53\x56\x1d\x3e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x01\x1a\x13\xec\x87\x60\x43\x1c\xa4\x3f"
        "\xb4\x10\x89\x97\x5f\x60\x30\xfc\x92\x52")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd2\x43\x8f\x1c\x3e\xcf\xeb\xbe\x4e\xdc"
        "\x78\x81\x48\x2d\x70\x1d\x28\x48\xda\x64")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x4f\x94\x66\x08\x1a\xe5\xc0\xa5\x1b\x5f"
        "\x6b\x9c\x6d\x5b\xcc\x57\x4b\x55\x50\x7d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x3e\xe5\xb4\x09\x8a\x68\x28\x85\xe4\x3b"
        "\x0d\x9b\x64\xc9\x21\xfa\xb0\xed\x39\x78")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xed\x68\xb5\x5c\x2d\x26\xcc\x43\xb8\x25"
        "\xef\xd7\x4f\x83\x75\xf5\x7b\x69\x3a\x06")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x78\xf5\xe4\x38\x47\xc8\x9b\x3e\x87\x18"
        "\x20\x5b\xb9\x1f\x24\xeb\x31\x3d\x7d\xc7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    5 + wedit.dy(),
                                    30,
                                    10));

    //drawable.save_to_png("/tmp/edit8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x22\xb4\x05\xf5\x69\x0d\x69\x30\x13\x11"
        "\x01\xd2\xfc\x20\x59\xac\xad\x9e\x11\xf4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetEdit)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), NULL, NULL)
        , sender(0)
        , event(0)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event,
                            unsigned long, unsigned long)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event,
                            long unsigned int, long unsigned int)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED);
    wedit.focus(0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb7\xac\xcd\x09\x45\x2e\x92\x80\x35\xf1"
        "\xdb\x38\xd5\x24\x4c\x7b\xed\x44\x85\x7d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);

    keymap.event(0, 16, decoded_data); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x4c\xf8\x17\x33\xff\x14\xd3\xc7\xe0\xce"
        "\xfb\x5e\x46\xe5\x90\xab\x9d\xa5\x92\xe7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17, decoded_data); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xc6\x4b\xf6\xb2\x76\xd9\xef\xe2\xe3\x66"
        "\x09\x94\x39\x50\x46\xb5\xae\xd4\xdf\x4f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x7d\xf2\xc6\x0e\xc9\x4e\x07\xd0\x11\x84"
        "\x19\xd5\x98\xba\x6e\x6a\x6a\x03\x95\x5d"
        )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xc6\x4b\xf6\xb2\x76\xd9\xef\xe2\xe3\x66"
        "\x09\x94\x39\x50\x46\xb5\xae\xd4\xdf\x4f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x4c\xf8\x17\x33\xff\x14\xd3\xc7\xe0\xce"
        "\xfb\x5e\x46\xe5\x90\xab\x9d\xa5\x92\xe7")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xff\x0c\x43\xde\xa2\x4c\x22\xbe\xfc\x98"
        "\x63\xe3\x10\x9e\x7e\x49\x45\x78\x83\x61")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x92\xe3\x46\x0e\x91\x55\xb1\xca\x80\xad"
        "\x3d\xe5\x09\xb1\x68\xcc\x31\xeb\x75\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xd4\x6a\x59\xd1\x20\xc1\x50\xac\x40\x52"
        "\xf6\x39\x37\x11\x4d\x2f\x25\x7a\x90\x0f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x02\x8b\x52\x84\xe0\xd9\x58\x89\xdb\x0f"
        "\x25\x7d\x46\xa5\x62\xb4\x4e\x4b\x5a\x01")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xc4\xd8\x39\x91\xd1\x42\x68\x2f\x65\xec"
        "\x9c\xa7\xad\xf1\x81\xce\x5e\x14\x35\x7a")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    wedit.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x+10, y+3, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));

    //drawable.save_to_png("/tmp/edit-e10.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x56\xad\x48\xb5\xfc\x61\xcc\xad\x3c\xc7"
        "\x0c\xa8\xa3\xb3\x71\x7c\x06\x2e\xdf\x87")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetEdit wedit1(drawable, 0,0, 50, &wcomposite, notifier,
                        "abababab", 4, YELLOW, BLACK);
    WidgetEdit wedit2(drawable, 0,100, 50, &wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED);
    WidgetEdit wedit3(drawable, 100,100, 50, &wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED);
    WidgetEdit wedit4(drawable, 300,300, 50, &wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN);
    WidgetEdit wedit5(drawable, 700,-10, 50, &wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
    WidgetEdit wedit6(drawable, -10,550, 50, &wcomposite, notifier,
                        "xxwwp", 2, DARK_GREY, PALE_GREEN);

    wcomposite.child_list.push_back(&wedit1);
    wcomposite.child_list.push_back(&wedit2);
    wcomposite.child_list.push_back(&wedit3);
    wcomposite.child_list.push_back(&wedit4);
    wcomposite.child_list.push_back(&wedit5);
    wcomposite.child_list.push_back(&wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    //drawable.save_to_png("/tmp/edit9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2e\xdd\x45\x63\x07\x2e\x81\xdf\xff\xa2"
        "\x07\xb5\x85\xac\x5b\x31\x91\x61\x36\x6d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    //drawable.save_to_png("/tmp/edit10.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x3f\xcb\x49\xfe\x0f\xe0\xc6\xbc\x53\x0b"
        "\xd7\xd5\x5f\xe7\xbe\x08\xc8\x5b\xad\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

