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
#define BOOST_TEST_MODULE TestWidgetPassword
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/password.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget at position 0,0 in it's parent context
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
    size_t password_pos = 2;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test1", id,
                     bg_color, fg_color, password_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dx(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x56\x8c\x5a\x16\xdf\xcc\x5c\x34\x04\xdc"
        "\x61\x7e\xa5\x31\xae\x1b\x6f\x5a\x2a\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test2", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x9c\x4a\x73\xba\x18\x9c\xc4\xde\x5a\x6f"
        "\xd7\xbb\xea\x97\x69\x37\xe3\xc8\x05\xa3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test3", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x38\xf1\x00\x4f\x9d\xe6\xa5\xa7\x36\x81"
        "\x7e\x49\x03\x3b\x21\xae\x86\x7b\xa9\x9a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test4", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x1b\xfb\xdf\x50\x47\xb7\x02\xe5\x54\x46"
        "\x15\xe7\x34\xf4\x4a\xef\x35\x72\xc7\xd5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test5", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xba\xe7\x54\x31\x41\x5c\xb2\x78\x80\x20"
        "\x46\x65\xd4\x7b\xf2\x9c\x2b\xd2\x07\x30")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xbd\xb4\xf0\x29\xfe\x50\x73\x9c\x0f\x67"
        "\x01\x75\x40\xb2\x59\xf9\xf4\xfc\x62\x59")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    //drawable.save_to_png("/tmp/password7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xb7\x65\xdf\x56\xd0\x36\x89\xb0\xf1\x02"
        "\xbf\x1d\x57\xb4\xb9\xe2\xa1\xff\x59\xe2")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, bg_color, fg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    5 + wpassword.dy(),
                                    30,
                                    10));

    //drawable.save_to_png("/tmp/password8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xbd\x4c\x99\xc3\x90\x02\xf4\x76\x35\x9a"
        "\x46\xb4\x61\xa4\x42\x17\xca\x49\x94\x6f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetPassword)
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

    WidgetPassword wpassword(&drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf8\x7c\x12\x8b\x4d\xee\xdb\xe2\x83\x1c"
        "\x18\xcc\xac\xec\x9a\xf9\xf9\x56\xaf\x46")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);

    keymap.event(0, 16, decoded_data); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x90\x07\x42\x25\x8c\x8e\x10\x6f\x0c\xa5"
        "\xce\xa7\x3e\x44\x9f\xe7\xe3\x5c\x69\x28")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17, decoded_data); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xcc\x03\x8c\x7b\xff\xd2\xa0\x3d\x33\x7f"
        "\x98\xe0\x35\x4d\x88\x5f\x4a\xb7\x68\xe8")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x86\x71\x8c\x76\x5d\x6a\xdf\xfe\x0d\xda"
        "\xb6\x4e\xfb\x0f\x9c\x69\x14\x4d\x0b\xa9")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xcc\x03\x8c\x7b\xff\xd2\xa0\x3d\x33\x7f"
        "\x98\xe0\x35\x4d\x88\x5f\x4a\xb7\x68\xe8")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x90\x07\x42\x25\x8c\x8e\x10\x6f\x0c\xa5"
        "\xce\xa7\x3e\x44\x9f\xe7\xe3\x5c\x69\x28")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5b\xbe\x78\x8a\x2d\x91\xe8\xdb\xa0\x86"
        "\x32\xf2\x59\x55\x5b\xba\xf8\x15\x51\x25")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x61\x99\xd8\x06\xde\xa1\x9c\x13\x8d\xea"
        "\x02\x17\x7f\x56\x20\x21\x14\x78\x1d\x23")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x3d\x17\xfa\x0c\xcd\x5e\x0f\x5f\x8d\xd3"
        "\xce\xbc\xf1\xeb\x5f\x8d\xe8\x0a\xb1\x37")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xf8\x7c\x12\x8b\x4d\xee\xdb\xe2\x83\x1c"
        "\x18\xcc\xac\xec\x9a\xf9\xf9\x56\xaf\x46")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x6e\x0a\x3a\x2f\xc3\xb2\xa7\x65\xb9\x4a"
        "\xa0\x27\xd1\xc4\x5b\x0b\xc2\xbb\x8f\xd3")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

//     wpassword.send_event(CLIC_BUTTON1_DOWN, 10, 3, 0);
//     BOOST_CHECK(widget_for_receive_event.sender == 0);
//     BOOST_CHECK(widget_for_receive_event.event == 0);
//     BOOST_CHECK(notifier.sender == 0);
//     BOOST_CHECK(notifier.event == 0);
//     notifier.sender = 0;
//     notifier.event = 0;
//     widget_for_receive_event.sender = 0;
//     widget_for_receive_event.event = 0;
//
//     wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
//     //drawable.save_to_png("/tmp/password-e10.png");
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x3f\x02\x08\xad\xbd\xd8\xf2\xc7\x1b\xf8"
//         "\x32\x58\x67\x66\x5d\xdb\xe5\x75\xe4\xda")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
}
//
// BOOST_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
// {
//     TestDraw drawable(800, 600);
//
//     // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
//     Widget2* parent = NULL;
//     NotifyApi * notifier = NULL;
//
//     WidgetComposite wcomposite(&drawable, Rect(0,0,800,600), parent, notifier);
//
//     WidgetPassword wpassword1(&drawable, 0,0, 50, &wcomposite, notifier,
//                         "abababab", 4, YELLOW, BLACK);
//     WidgetPassword wpassword2(&drawable, 0,100, 50, &wcomposite, notifier,
//                         "ggghdgh", 2, WHITE, RED);
//     WidgetPassword wpassword3(&drawable, 100,100, 50, &wcomposite, notifier,
//                         "lldlslql", 1, BLUE, RED);
//     WidgetPassword wpassword4(&drawable, 300,300, 50, &wcomposite, notifier,
//                         "LLLLMLLM", 20, PINK, DARK_GREEN);
//     WidgetPassword wpassword5(&drawable, 700,-10, 50, &wcomposite, notifier,
//                         "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
//     WidgetPassword wpassword6(&drawable, -10,550, 50, &wcomposite, notifier,
//                         "xxwwp", 2, DARK_GREY, PALE_GREEN);
//
//     wcomposite.child_list.push_back(&wpassword1);
//     wcomposite.child_list.push_back(&wpassword2);
//     wcomposite.child_list.push_back(&wpassword3);
//     wcomposite.child_list.push_back(&wpassword4);
//     wcomposite.child_list.push_back(&wpassword5);
//     wcomposite.child_list.push_back(&wpassword6);
//
//     // ask to widget to redraw at position 100,25 and of size 100x100.
//     wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));
//
//     //drawable.save_to_png("/tmp/password9.png");
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
//     //drawable.save_to_png("/tmp/password10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x85\x0a\x9c\x09\x57\xd9\x99\x52\xed\xa8"
//         "\x25\x71\x91\x6c\xf4\xf4\x21\x9a\xe5\x1a")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

