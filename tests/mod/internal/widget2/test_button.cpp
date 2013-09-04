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
#define BOOST_TEST_MODULE TestWidgetButton
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/button.hpp"
#include "internal/widget2/composite.hpp"
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

    virtual void draw(const RDPPatBlt& cmd, const Rect& rect)
    {
        this->gd.draw(cmd, rect);
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test1", auto_resize, id, fg_color, bg_color, xtext, ytext);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0, 0, wbutton.cx(), wbutton.cy()));


    //drawable.save_to_png("/tmp/button.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7d\xfe\xb4\x41\x31\x06\x68\xe8\xbb\x75"
        "\x8c\x35\x11\x19\x97\x2a\x16\x0f\x65\x28")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test2", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2f\xea\x44\x73\x16\x6d\xea\xcf\xa0\xf6"
        "\xc2\x89\x1f\xec\xf3\xb4\xb7\xba\x92\x1b")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test3", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe2\x04\x73\x0e\x1b\xa8\xb9\x28\x5b\x31"
        "\x50\x60\x43\x67\x2c\x71\xbe\xc3\x7d\x4a")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test4", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x0e\x13\x6c\x6a\x14\x0b\x5b\x1a\x51\x01"
        "\x17\xff\x5a\xf3\xd5\x09\x2c\xc1\x78\x75")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test5", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x5b\xb1\x08\xa2\x09\xd1\x50\x87\x24\xc1"
        "\x5e\xfb\x62\x51\x47\x93\xaa\xc7\x92\xb2")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wbutton.rdp_input_invalidate(Rect(0 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xed\x10\xc4\xa5\x1a\x55\x26\x9d\xca\x2e"
        "\x78\x21\x2a\x38\x83\x16\x44\x9d\x0d\xad")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      0 + wbutton.dy(),
                                      wbutton.cx(),
                                      wbutton.cy()));

    //drawable.save_to_png("/tmp/button7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x42\xe2\xf6\xef\xb6\x1d\xbf\x59\x8a\x39"
        "\x55\x39\x23\x66\x1b\xee\x85\xe6\x0f\xe9")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wbutton.rdp_input_invalidate(Rect(20 + wbutton.dx(),
                                      5 + wbutton.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/button8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x11\x66\x8f\x68\x02\x7d\xb5\xa7\xec\xcd"
        "\xdf\x0d\x32\xd0\x68\x9f\x25\x35\x07\xc9")){
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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test6", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x96\x86\xc7\x47\xc4\x9d\x08\xbd\xf1\x6e"
        "\x81\xaf\xc3\xcb\xeb\xfa\x31\x4d\x02\x71")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xec\x8d\x4f\x52\xc9\xf1\x5e\x5e\x7c\xd3\x1e\x6c\xd1\xe2\xa3\x55\x03\xf7\x20\x56"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 15, 15, NULL);
    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button11.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xfb\x67\x5f\x09\x35\x9f\xf8\x75\xa7\x33\x8c\x52\x78\xdc\x84\x05\xab\x66\x6c\xdc"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonEvent)
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
        {
        }
        virtual void notify(Widget2* sender, notify_event_t event,
                            long unsigned int, long unsigned int)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget2* parent = &widget_for_receive_event;
    bool auto_resize = false;
    int16_t x = 0;
    int16_t y = 0;

    WidgetButton wbutton(drawable, x, y, parent, &notifier, "", auto_resize);

    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
    wbutton.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char('a');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_KEY);
    keymap.push_char(' ');
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wbutton.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == &wbutton);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
}

BOOST_AUTO_TEST_CASE(TraceWidgetButtonAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetButton is a button widget of size 256x125 at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetButton wbutton1(drawable, 0,0, &wcomposite, notifier,
                        "abababab", true, 0, YELLOW, BLACK);
    WidgetButton wbutton2(drawable, 0,100, &wcomposite, notifier,
                        "ggghdgh", true, 0, WHITE, RED);
    WidgetButton wbutton3(drawable, 100,100, &wcomposite, notifier,
                        "lldlslql", true, 0, BLUE, RED);
    WidgetButton wbutton4(drawable, 300,300, &wcomposite, notifier,
                        "LLLLMLLM", true, 0, PINK, DARK_GREEN);
    WidgetButton wbutton5(drawable, 700,-10, &wcomposite, notifier,
                        "dsdsdjdjs", true, 0, LIGHT_GREEN, DARK_BLUE);
    WidgetButton wbutton6(drawable, -10,550, &wcomposite, notifier,
                        "xxwwp", true, 0, DARK_GREY, PALE_GREEN);

    wcomposite.child_list.push_back(&wbutton1);
    wcomposite.child_list.push_back(&wbutton2);
    wcomposite.child_list.push_back(&wbutton3);
    wcomposite.child_list.push_back(&wbutton4);
    wcomposite.child_list.push_back(&wbutton5);
    wcomposite.child_list.push_back(&wbutton6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    //drawable.save_to_png("/tmp/button12.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x27\xaa\x91\x51\x0b\x39\xf7\xf1\xfd\x55"
        "\x4f\xb0\x33\xac\x7a\x45\x56\x16\x69\x12")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    //drawable.save_to_png("/tmp/button13.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x6b\x18\x4b\x47\x59\xd9\xca\xe7\xe4\xd1"
        "\x57\x26\x23\x8d\x10\x48\x26\x8e\x6d\xcf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}


BOOST_AUTO_TEST_CASE(TraceWidgetButtonFocus)
{
    TestDraw drawable(70, 40);

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

    WidgetButton wbutton(drawable, x, y, parent, notifier, "test7", auto_resize, id, fg_color, bg_color, xtext, ytext);

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button10.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc5\x83\xfc\x1a\x58\xc0\xc7\xfb\xcd\x10\x54\x90\xf0\xd4\x7d\x4b\x2d\x88\x40\x5f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button11.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x6b\xf5\xb3\x35\x77\xa1\x95\xac\x33\x17\x67\x5e\x0f\x75\x83\x88\x0e\x24\xa7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.blur();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button12.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xc5\x83\xfc\x1a\x58\xc0\xc7\xfb\xcd\x10\x54\x90\xf0\xd4\x7d\x4b\x2d\x88\x40\x5f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    wbutton.focus();

    wbutton.rdp_input_invalidate(wbutton.rect);

    //drawable.save_to_png("/tmp/button13.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xd4\x6b\xf5\xb3\x35\x77\xa1\x95\xac\x33\x17\x67\x5e\x0f\x75\x83\x88\x0e\x24\xa7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

