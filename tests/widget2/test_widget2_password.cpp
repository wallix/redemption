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
        "\x0b\x9a\x72\x8b\x2d\xc6\xd1\x70\x33\x80"
        "\x41\x77\x8d\x19\xd2\xc7\x0d\x37\x3b\xf4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    Widget * parent = NULL;
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
        "\x52\xef\xb2\x80\xf1\x4e\xc3\x08\xfa\x1a"
        "\x0e\xe8\xa9\x62\x7e\xd3\xe6\x8e\xd7\xdc")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    Widget * parent = NULL;
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
        "\x87\x73\x8e\x2e\xee\x1d\x59\x44\x79\x49"
        "\xf5\x8b\x74\x3c\x29\x63\x27\x8c\x87\xb5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    Widget * parent = NULL;
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
        "\x3c\xac\xca\x87\xb1\xc8\xe2\xe6\x56\x67"
        "\xa2\xc9\x1a\x55\xd2\xf6\x77\x3d\x4d\xf5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    Widget * parent = NULL;
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
        "\x4c\x9b\x8a\x9b\xb3\x66\x17\xf8\x50\x8c"
        "\x60\xb6\xe6\x9a\x01\x61\xbb\x93\xb2\x23")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
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
        "\xe8\x16\xe9\x21\xc1\xe7\x4e\x5b\x6d\xab"
        "\x40\xed\xc4\x1d\xb0\xf2\xb4\xed\x18\x18")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget * parent = NULL;
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
        "\xc4\x5e\xac\xd3\x24\x6a\x76\xc4\x33\x97"
        "\x56\xce\x21\x2f\x25\x05\x0b\x46\xb5\x68")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    Widget * parent = NULL;
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
        "\x73\x16\xab\xc1\x14\x4c\xaa\xaf\x40\x14"
        "\x75\x97\x68\x49\x8f\x63\xf5\xf5\x50\x21")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget * sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget* sender, notify_event_t event,
                            long unsigned int, long unsigned int)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Widget * parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x19\x0e\x94\x4e\x6c\x78\x2b\x73\xa4\xa4"
        "\x1d\x87\xb3\x6f\x84\xbe\x1f\xad\xb7\x28")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.event(0, 16); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5c\x3b\xfd\x38\x49\x3a\xaa\xf3\xd5\xed"
        "\xcf\xc3\xfc\x05\x3b\x1d\xf9\x84\xf8\x34")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x18\x30\x72\x17\x4d\x91\xd0\x9f\xa5\xa9"
        "\x1d\xae\xd2\x1f\xa3\x6a\xc4\xcf\xb1\x4d")){
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
        "\xc7\x7e\x4b\x4e\xc8\x90\x52\x27\xe7\x4f"
        "\x56\x6d\x1d\x2e\x32\x87\x72\xdc\x02\xd5")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x18\x30\x72\x17\x4d\x91\xd0\x9f\xa5\xa9"
        "\x1d\xae\xd2\x1f\xa3\x6a\xc4\xcf\xb1\x4d")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5c\x3b\xfd\x38\x49\x3a\xaa\xf3\xd5\xed"
        "\xcf\xc3\xfc\x05\x3b\x1d\xf9\x84\xf8\x34")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x20\xab\x44\xba\xcf\xbe\x03\xee\x25\xec"
        "\x94\x2b\x52\x50\x5d\x12\x74\xa6\xcf\x22")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x3a\xfc\x64\x8f\x27\xd3\xb1\xdd\x84\xde"
        "\x43\xeb\xf7\x01\x9a\xf8\x49\xf1\xa7\x7b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x1f\x9f\xf7\x15\x89\x52\x0b\x6e\xb3\x6a"
        "\xb9\xbd\x55\x54\x48\x2b\x70\x99\xc1\x96")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x19\x0e\x94\x4e\x6c\x78\x2b\x73\xa4\xa4"
        "\x1d\x87\xb3\x6f\x84\xbe\x1f\xad\xb7\x28")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    //drawable.save_to_png("/tmp/password-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xfd\x5d\x58\xee\x00\xb5\x1f\xe3\xb1\xc8"
        "\x99\x1c\xe2\x12\x72\x93\x7a\xc0\xf5\x10")){
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
//     Widget * parent = NULL;
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

