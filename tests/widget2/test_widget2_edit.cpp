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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test1", id,
                     bg_color, fg_color, edit_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dx(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x5c\x81\x3e\xbd\x24\xf0\xa2\x56\xb0\x21"
        "\x60\xb2\x1d\xf8\x0d\xcc\x3b\x71\x73\x3d")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test2", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x4e\x45\x7d\x3d\xca\xa5\xb0\x13\x78\xc0"
        "\xea\x3c\x6c\xae\x82\x43\xa3\xce\xbe\x99")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test3", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x96\x06\x6f\x58\x6e\x6e\xae\x7b\x5c\x60"
        "\x93\xd5\x9d\xb7\x8d\x50\x60\xd3\x31\x0a")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test4", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa2\x0a\xe6\x84\xb1\xda\x63\xe0\x34\xa2"
        "\x75\x0b\xb0\xc7\x10\xcc\x28\xc8\x4b\x3d")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test5", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x30\x1f\xa4\x66\x0f\xff\x14\x56\xad\xf3"
        "\xa5\x3b\x90\x4a\x3b\x15\x38\xbe\x87\xf5")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xcc\xdd\x2a\x6b\xfa\xff\x61\xcb\x7f"
        "\xe4\xfe\x63\x06\xb4\x62\x4d\x0c\xac\x2c")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    0 + wedit.dy(),
                                    wedit.cx(),
                                    wedit.cy()));

    //drawable.save_to_png("/tmp/edit7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x13\x67\x4c\xf5\xd4\xfc\xda\x77\x16\x29"
        "\x11\x0b\x91\x45\x00\x4b\xb4\x12\x5a\x78")){
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

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20 + wedit.dx(),
                                    5 + wedit.dy(),
                                    30,
                                    10));

    //drawable.save_to_png("/tmp/edit8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe9\xf8\xf0\xe0\x9a\x10\x25\x0c\x02\x3f"
        "\xae\x05\xf6\xd0\x23\xde\x2f\xf6\xf4\xba")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetEdit)
{
    TestDraw drawable(800, 600);

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

    WidgetEdit wedit(&drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x59\x41\xdf\x6e\x2c\x8d\x84\xbf\x94\x91"
        "\x4d\xda\x2b\x1f\xce\x2d\x3d\xd7\xa0\x26")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.event(0, 16); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5e\x92\x36\xf9\x4d\x3d\x81\x8b\x06\x3e"
        "\xdd\x83\x04\x11\x95\xb8\xc6\xa2\x06\x3b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xcc\x9d\xca\x1e\x52\x9f\x7a\x52\x3c"
        "\x76\xf2\xe3\x4d\xc4\x21\x60\xa4\x13\xfb")){
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
        "\x50\x38\x4d\x21\x1d\x86\xc2\x58\xa9\x19"
        "\x69\x00\x45\xc9\xcd\xbe\x9e\xb6\x78\x9b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xcc\x9d\xca\x1e\x52\x9f\x7a\x52\x3c"
        "\x76\xf2\xe3\x4d\xc4\x21\x60\xa4\x13\xfb")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5e\x92\x36\xf9\x4d\x3d\x81\x8b\x06\x3e"
        "\xdd\x83\x04\x11\x95\xb8\xc6\xa2\x06\x3b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xa1\xa7\x3f\xa7\x5d\xa4\x42\x7f\x61\x59"
        "\xde\xe5\x97\x7a\x4f\x43\x66\x3d\x3d\xf0")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x77\x2e\xdd\xbd\x2c\x59\x44\xec\xdb\xcc"
        "\x17\xe7\x0b\x33\x7c\x10\x98\x6f\xa5\x32")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xf6\x72\xe4\x2d\xba\x5e\xa3\x38\x20\xc1"
        "\x68\x5f\xab\x5c\x10\xa0\xe4\xb7\xb0\xf2")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x20\xd3\x26\xdc\x20\x6c\x8f\x74\xbb\xcc"
        "\x3b\xf3\x50\x38\x46\x3d\x31\x5d\x20\x82")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    //drawable.save_to_png("/tmp/edit-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x53\xd3\x2c\x25\x22\xe4\x9c\x6c\x18\x37"
        "\xf8\xe0\x14\x65\x14\xc5\x9a\x69\x4e\xab")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

//     wedit.send_event(CLIC_BUTTON1_DOWN, 10, 3, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//     notifier.sender = 0;
//     notifier.event = 0;
//     widget_for_receive_event.sender = 0;
//     widget_for_receive_event.event = 0;
//
//     wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
//     //drawable.save_to_png("/tmp/edit-e10.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
}
//
// BOOST_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
// {
//     TestDraw drawable(800, 600);
//
//     // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
//     Widget2* parent = NULL;
//     NotifyApi * notifier = NULL;
//
//     WidgetComposite wcomposite(&drawable, Rect(0,0,800,600), parent, notifier);
//
//     WidgetEdit wedit1(&drawable, 0,0, 50, &wcomposite, notifier,
//                         "abababab", 4, YELLOW, BLACK);
//     WidgetEdit wedit2(&drawable, 0,100, 50, &wcomposite, notifier,
//                         "ggghdgh", 2, WHITE, RED);
//     WidgetEdit wedit3(&drawable, 100,100, 50, &wcomposite, notifier,
//                         "lldlslql", 1, BLUE, RED);
//     WidgetEdit wedit4(&drawable, 300,300, 50, &wcomposite, notifier,
//                         "LLLLMLLM", 20, PINK, DARK_GREEN);
//     WidgetEdit wedit5(&drawable, 700,-10, 50, &wcomposite, notifier,
//                         "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
//     WidgetEdit wedit6(&drawable, -10,550, 50, &wcomposite, notifier,
//                         "xxwwp", 2, DARK_GREY, PALE_GREEN);
//
//     wcomposite.child_list.push_back(&wedit1);
//     wcomposite.child_list.push_back(&wedit2);
//     wcomposite.child_list.push_back(&wedit3);
//     wcomposite.child_list.push_back(&wedit4);
//     wcomposite.child_list.push_back(&wedit5);
//     wcomposite.child_list.push_back(&wedit6);
//
//     // ask to widget to redraw at position 100,25 and of size 100x100.
//     wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));
//
//     //drawable.save_to_png("/tmp/edit9.png");
//
//     char message[1024];
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     // ask to widget to redraw at it's current position
//     wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));
//
//     //drawable.save_to_png("/tmp/edit10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x85\x0a\x9c\x09\x57\xd9\x99\x52\xed\xa8"
//         "\x25\x71\x91\x6c\xf4\xf4\x21\x9a\xe5\x1a")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

