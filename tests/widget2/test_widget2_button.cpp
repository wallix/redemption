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

    virtual void server_draw_text(int16_t x, int16_t y, const char* text, uint32_t fgcolor, const Rect& clip)
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

BOOST_AUTO_TEST_CASE(TraceWidgetButton)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget at position 0,0 in it's parent context
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

    WidgetButton wbutton(&drawable, x, y, parent, notifier, "test1", auto_resize, id, bg_color, fg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    //drawable.save_to_png("/tmp/button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x85\x4e\x2f\x59\xa8\xf4\xfe\x23\xdd\xf1"
        "\x62\xb0\x07\x9b\xca\x67\xeb\x6a\x2f\xc3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
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
        "\x33\x72\xd3\x07\xe1\x74\x72\xd2\xbc\x3d"
        "\xc7\xdb\x7b\x2d\xb0\x00\xca\x72\xeb\xa2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton3)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
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
        "\xc3\xf9\x7b\x0f\x0a\x84\x56\x35\x2c\x1f"
        "\x78\xb3\x6a\xbc\x8c\x80\xb4\x0c\x26\x2c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton4)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
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
        "\x4a\xea\x03\x9a\xcd\xb1\xed\x8b\xb2\x37"
        "\x2a\xdf\x87\xb7\xd4\x23\xc9\xe2\xc8\x95")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton5)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
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
        "\xab\xcf\xd5\x7b\x56\x9e\x9f\x78\x94\xd8"
        "\xa3\xe6\xe5\xaa\x97\x5b\x90\xed\xf6\x57")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButton6)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
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
        "\xa1\x79\x08\x2f\x10\xa4\x61\x16\xe5\x54"
        "\x77\xb7\xbc\x57\xf0\x05\xa8\x5a\x62\xf7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
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
        "\x91\x29\x0b\xc1\xe8\xa8\xd8\xd5\xe7\xfa"
        "\x8f\x03\xae\x0a\x28\xaf\x8f\xbf\x16\x9f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonClip2)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
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
        "\xc9\x6a\xc3\x39\x5b\xcc\xd6\x67\xbc\xb9"
        "\x20\xac\x42\x2b\xd6\x6a\x07\xaa\xa1\x80")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonDownAndUp)
{
    TestDraw drawable(800, 600);

    Widget2* parent = NULL;
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
        "\xd8\xfe\x97\x63\xe2\x1d\x94\xdb\xa1\xa7"
        "\x2c\xbf\x68\x2e\xf6\x6c\x96\xcd\xb7\x8a")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button10.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x38\x0a\xc1\x87\x68\x1b\x4a\xc7\x2f\x94"
        "\x14\x6f\x5e\xc5\x21\xbf\xd0\xbd\xac\xff")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button11.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xd8\xfe\x97\x63\xe2\x1d\x94\xdb\xa1\xa7"
        "\x2c\xbf\x68\x2e\xf6\x6c\x96\xcd\xb7\x8a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

// BOOST_AUTO_TEST_CASE(TraceWidgetButtonEvent)
// {
//     struct WidgetReceiveEvent : public Widget {
//         Widget2* sender;
//         NotifyApi::notify_event_t event;
//
//         WidgetReceiveEvent()
//         : Widget(NULL, Rect(), NULL, NULL)
//         {}
//
//         virtual void draw(const Rect&)
//         {}
//
//         virtual void notify(Widget2* sender, NotifyApi::notify_event_t event,
//                             unsigned long, unsigned long)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } widget_for_receive_event;
//
//     struct Notify : public NotifyApi {
//         Widget2* sender;
//         notify_event_t event;
//         virtual void notify(Widget2* sender, notify_event_t event,
//                             long unsigned int, long unsigned int)
//         {
//             this->sender = sender;
//             this->event = event;
//         }
//     } notifier;
//
//     Widget2* parent = &widget_for_receive_event;
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
//     Widget2* parent = NULL;
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

