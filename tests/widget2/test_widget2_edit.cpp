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

#include "internal/widget2/edit.hpp"
#include "internal/widget2/widget_composite.hpp"
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
    Widget * parent = NULL;
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
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb0\x87\xd7\x8b\x8c\x55\x2a\xfb\x43\xd8"
        "\x3e\x7f\x77\xa3\xcf\xc7\x88\x48\x17\x09")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test2", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf3\xd7\xb3\xde\x51\x50\x52\xec\x83\x43"
        "\x72\xe7\xb5\x26\x63\xe1\xf3\x72\x30\x91")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test3", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xbd\x70\x89\x0c\x9b\x61\xac\x8c\x53\x3d"
        "\x76\x31\x78\x88\x9a\xd1\xa4\x46\x99\x80")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test4", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe3\x87\xd5\xae\x47\xf2\x6f\x71\x09\x05"
        "\xd0\xac\x8c\x8e\xfe\xbc\x10\x99\x23\xb4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test5", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x3f\xb8\x31\x37\x46\xc4\xef\x6d\xea\xe3"
        "\x73\x47\x60\x89\xb9\x9b\xac\x60\xab\xb1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb8\x82\x81\x94\x16\xd9\x71\xe9\xf1\x65"
        "\xbc\x38\x40\x59\xa8\xf1\xf4\x47\xed\x22")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wedit.rdp_input_invalidate(Rect(20, 0, wedit.rect.cx, wedit.rect.cy));

    drawable.save_to_png("/tmp/edit7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x5e\x1c\x6a\xcc\xb7\xce\x1a\x5a\xa8\x1a"
        "\xdd\xb2\xaf\x00\xa8\xd4\x67\x68\x3f\x1a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetEdit wedit(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wedit.rdp_input_invalidate(Rect(20, 5, 30, 10));

    drawable.save_to_png("/tmp/edit8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8d\x38\xa4\x5c\x28\xd7\x3f\x22\x5e\xf4"
        "\xf6\xa2\x6d\x94\x6d\xa2\xf0\x15\x58\xd5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetEditEvent)
// {
//     TestDraw drawable(800, 600);
//
//     struct WidgetReceiveEvent : public WidgetComposite {
//         Widget * sender;
//         NotifyApi::notify_event_t event;
//
//         WidgetReceiveEvent(ModApi * drawable)
//         : WidgetComposite(drawable, Rect(0,0,800,600), 0, 0)
//         {}
//
//         virtual void draw(const Rect&)
//         {}
//
//         virtual void notify(Widget * sender, NotifyApi::notify_event_t event,
//                             unsigned long, unsigned long)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } widget_for_receive_event(&drawable);
//
//     struct Notify : public NotifyApi {
//         Widget * sender;
//         notify_event_t event;
//         virtual void notify(Widget* sender, notify_event_t event,
//                             long unsigned int, long unsigned int)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;
//
//     Widget * parent = &widget_for_receive_event;
//     int16_t x = 0;
//     int16_t y = 0;
//     uint16_t cx = 100;
//
//     WidgetEdit wedit(&drawable, x, y, cx, parent, &notifier, "abcdef", 0, YELLOW, BLUE);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));

//     drawable.save_to_png("/tmp/edit-e1.png");
//     char message[1024];
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     Keymap2 keymap;
//     keymap.push_kevent(Keymap2::KEVENT_KEY);
//     keymap.push_char('a');
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e2.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));

//     drawable.save_to_png("/tmp/edit-e3.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e4.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e5.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_DELETE);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e6.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_END);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e7.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_HOME);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e8.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
//     keymap.push_kevent(Keymap2::KEVENT_ENTER);
//     wedit.send_event(KEYDOWN, 0, 0, &keymap);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//
//    parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
//     drawable.save_to_png("/tmp/edit-e9.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
//
// //     wedit.send_event(CLIC_BUTTON1_DOWN, 10, 3, 0);
// //     BOOST_CHECK(widget_for_receive_event.sender == 0);
// //     BOOST_CHECK(widget_for_receive_event.event == 0);
// //     BOOST_CHECK(notifier.sender == 0);
// //     BOOST_CHECK(notifier.event == 0);
// //     notifier.sender = 0;
// //     notifier.event = 0;
// //     widget_for_receive_event.sender = 0;
// //     widget_for_receive_event.event = 0;
// //
// //     parent->rdp_input_invalidate(Rect(0, 0, parent->cx(), parent->cx()));
// //     drawable.save_to_png("/tmp/edit-e10.png");
// //     if (!check_sig(drawable.gd.drawable, message,
// //         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
// //         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
// //         BOOST_CHECK_MESSAGE(false, message);
// //     }
// }
//
// BOOST_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
// {
//     TestDraw drawable(800, 600);
//
//     // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
//     Widget * parent = NULL;
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
//     drawable.save_to_png("/tmp/edit9.png");
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
//     drawable.save_to_png("/tmp/edit10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x85\x0a\x9c\x09\x57\xd9\x99\x52\xed\xa8"
//         "\x25\x71\x91\x6c\xf4\xf4\x21\x9a\xe5\x1a")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

