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
#define BOOST_TEST_MODULE TestWidgetButton
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/button.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetButton)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget at position 0,0 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;
    int xtext = 4;
    int ytext = 1;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test1", auto_resize, id, bg_color, fg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    //drawable.save_to_png("/tmp/button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x4a\x59\xc1\x8e\x1e\x26\x6c\x75\x92\x20"
        "\x67\x69\x9c\x7c\x44\x37\xe5\x3e\xbf\x4c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 100;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test2", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa6\x5c\xd7\x1e\xeb\x5b\xb9\x5d\x34\xf9"
        "\x18\x2a\x1b\x4a\xe8\xf4\x78\x23\x3a\xcf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton3)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -10;
    int16_t y = 500;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test3", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x28\xb5\x4c\x5f\x3c\x84\xc5\xa3\x07\x4e"
        "\xd0\xf0\x1d\x6a\x0e\x5c\xa0\xee\x34\x1f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton4)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 770;
    int16_t y = 500;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test4", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x1c\x8b\x36\xbc\xfa\x3b\xd7\x68\x8d\xd4"
        "\x3b\x55\x02\x86\x0b\x08\x43\x82\x8b\xfa")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton5)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = -20;
    int16_t y = -7;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test5", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x11\x17\x16\x5f\xd6\x92\x4e\x3c\xf4\x66"
        "\x22\xe8\x64\xa7\x8e\x63\x0b\xbe\x3a\x29")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton6)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test6", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb9\xb2\x66\xcc\x86\x65\xcf\x54\xb8\x4a"
        "\x99\x19\xf0\xad\x92\xd0\x40\xe6\xbc\x10")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 760;
    int16_t y = -7;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test6", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x64\xfa\xa6\xb6\xe3\x07\x50\xd5\x91\x38"
        "\xb6\x49\x03\xb5\x11\x0b\x59\x86\xac\x7c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,7 in it's parent context
    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 0;
    int16_t y = 0;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test6", auto_resize, id, bg_color, fg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd2\x7b\x11\x85\x5e\x8f\x5f\x28\x7d\xad"
        "\x20\x31\x21\x2e\x87\xc5\x28\xc7\xa5\xd0")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    Widget * parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    bool auto_resize = true;
    int16_t x = 10;
    int16_t y = 10;
    int xtext = 4;
    int ytext = 1;

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test6", auto_resize, id, bg_color, fg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x40\xef\x1c\x6b\xee\x41\x2e\xb8\x29\xbd"
        "\xb8\x1e\x12\xb7\x3b\x03\x6b\x6e\x4c\x6b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button10.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x73\x66\xdd\xe1\x17\x6c\xdb\x3a\x93\xac"
        "\x9b\x0a\xd1\x8a\x2f\x98\x6d\x95\x50\x06")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button11.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x40\xef\x1c\x6b\xee\x41\x2e\xb8\x29\xbd"
        "\xb8\x1e\x12\xb7\x3b\x03\x6b\x6e\x4c\x6b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetButtonEvent)
// {
//     struct WidgetReceiveEvent : public Widget {
//         Widget * sender;
//         NotifyApi::notify_event_t event;
//
//         WidgetReceiveEvent()
//         : Widget(NULL, Rect(), NULL, NULL)
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
//     } widget_for_receive_event;
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
//     ModApi * drawable = NULL;
//     bool auto_resize = false;
//     int16_t x = 0;
//     int16_t y = 0;
//
//     WidgetButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize);
//
//     wbutton.rdp_input_mouse(CLIC_BUTTON1_UP, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//     wbutton.rdp_input_mouse(CLIC_BUTTON1_DOWN, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//     wbutton.rdp_input_mouse(CLIC_BUTTON1_UP, 0, 0, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == &wbutton);
//     BOOST_CHECK(widget_for_receive_event.event == NOTIFY_SUBMIT);
//     BOOST_CHECK(notifier.sender == &wbutton);
//     BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
// }

// BOOST_AUTO_TEST_CASE(TraceWidgetButtonAndComposite)
// {
//     TestDraw drawable(800, 600);
//
//     // WidgetButton is a button widget of size 256x125 at position 0,0 in it's parent context
//     Widget * parent = NULL;
//     NotifyApi * notifier = NULL;
//
//     WidgetComposite wcomposite(&drawable, Rect(0,0,800,600), parent, notifier);
//
//     WidgetButton wbutton1(&drawable, 0,0, &wcomposite, notifier,
//                         "abababab", true, 0, YELLOW, BLACK);
//     WidgetButton wbutton2(&drawable, 0,100, &wcomposite, notifier,
//                         "ggghdgh", true, 0, WHITE, RED);
//     WidgetButton wbutton3(&drawable, 100,100, &wcomposite, notifier,
//                         "lldlslql", true, 0, BLUE, RED);
//     WidgetButton wbutton4(&drawable, 300,300, &wcomposite, notifier,
//                         "LLLLMLLM", true, 0, PINK, DARK_GREEN);
//     WidgetButton wbutton5(&drawable, 700,-10, &wcomposite, notifier,
//                         "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE);
//     WidgetButton wbutton6(&drawable, -10,550, &wcomposite, notifier,
//                         "xxwwp", true, 0, DARK_GREY, PALE_GREEN);
//
//     wcomposite.child_list.push_back(&wbutton1);
//     wcomposite.child_list.push_back(&wbutton2);
//     wcomposite.child_list.push_back(&wbutton3);
//     wcomposite.child_list.push_back(&wbutton4);
//     wcomposite.child_list.push_back(&wbutton5);
//     wcomposite.child_list.push_back(&wbutton6);
//
//     // ask to widget to redraw at position 100,25 and of size 100x100.
//     wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));
//
//     //drawable.save_to_png("/tmp/button9.png");
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
//     //drawable.save_to_png("/tmp/button10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x85\x0a\x9c\x09\x57\xd9\x99\x52\xed\xa8"
//         "\x25\x71\x91\x6c\xf4\xf4\x21\x9a\xe5\x1a")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

