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
    : gd(w, h, false)
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
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, password_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dx(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xad\xc6\xf8\x7f\x7e\x45\x8b\x3b\x00\x36"
        "\x65\x6d\x15\x86\x70\x04\x2f\x67\x54\x36")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa2\x07\x9a\xce\xd3\xa5\x83\x9f\x40\x3c"
        "\x4a\xb7\x60\x67\x1a\xa3\x16\x1b\x2d\xec")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x94\x37\xe1\x3b\x5f\x54\xf7\xe4\xb4\x80"
        "\x4f\x33\x02\x03\x53\x76\xcd\x0b\x36\x6a")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd6\x16\xae\xf8\xc0\x74\x70\x35\xc5\x9c"
        "\x4d\xb0\xce\xa2\x25\xd2\xf2\x81\x7e\x00")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x17\xe0\xaa\xf8\x8f\x87\x43\x84\x4e\x41"
        "\x7a\x60\x6c\x97\x7b\x00\x98\x19\x66\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xef\xab\x93\xed\x43\x65\x61\xf7\x06\xad"
        "\x6a\x9d\x66\x98\xf7\x50\x2a\x8b\xad\x07")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png("/tmp/password7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x67\x8c\x9c\x0c\x0b\x68\xf2\x3f\x8a\x1a"
        "\x28\xeb\xfc\x26\x42\xd8\xf0\x0f\x0b\xe3")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int fg_color = 0xFF0000; //red
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(&drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    5 + wpassword.dy(),
                                    30,
                                    10));

    drawable.save_to_png("/tmp/password8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x25\xb0\x8d\xea\xd3\x3d\xae\x20\x7f\xee"
        "\xd1\x87\xc5\x99\x54\x61\x61\xc9\x10\x66")){
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

    WidgetPassword wpassword(&drawable, x, y, cx, parent, &notifier, "abcdef", 0, 0x00FFFF, 0x0000FF);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x29\x3f\x2a\x59\x5a\x6c\x80\xbc\x0d\x23"
        "\x59\x80\xfe\x37\xa6\x9e\x61\xcc\x8b\xdb")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);

    keymap.event(0, 16, decoded_data); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x4a\x83\xd6\x99\xe6\x91\x24\xaf\x30\xe5"
        "\x92\x3f\x11\x58\x0c\x95\xbe\x40\x6e\x64")){
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
    drawable.save_to_png("/tmp/password-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x61\xdc\xdb\x01\x24\xf7\x76\x7e\x9d\x27"
        "\xf5\xbe\x3c\xb8\x12\x3e\xae\xa2\xff\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xa4\x18\x63\xb0\x11\x25\x41\x42\x62\x4f"
        "\xaa\xac\x3e\x91\xa9\xa2\x9d\xbe\x4a\x76")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x61\xdc\xdb\x01\x24\xf7\x76\x7e\x9d\x27"
        "\xf5\xbe\x3c\xb8\x12\x3e\xae\xa2\xff\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x4a\x83\xd6\x99\xe6\x91\x24\xaf\x30\xe5"
        "\x92\x3f\x11\x58\x0c\x95\xbe\x40\x6e\x64")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x94\x1e\xb1\x3f\x3d\x7a\xfb\xb2\xc9\x65"
        "\x24\xa9\x8a\x7a\xe5\x56\x04\x91\x30\x3b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x52\xe5\x7b\xe0\x0b\xf8\x4c\x0c\x68"
        "\x11\xb6\x1b\xfb\xf7\x5a\xa5\x88\xae\x43")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xa3\x74\x36\x09\x57\xe3\xac\x39\xba\x20"
        "\xe8\x5c\x58\x87\x56\xaa\xb5\x6e\x74\xdf")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x29\x3f\x2a\x59\x5a\x6c\x80\xbc\x0d\x23"
        "\x59\x80\xfe\x37\xa6\x9e\x61\xcc\x8b\xdb")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png("/tmp/password-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x60\xeb\x74\x18\x22\x17\xd4\xf9\xbb\x76"
        "\xc2\x45\xc8\xe1\xa6\x31\x98\xe9\x85\x37")){
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
//     drawable.save_to_png("/tmp/password-e10.png");
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
//     drawable.save_to_png("/tmp/password9.png");
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
//     drawable.save_to_png("/tmp/password10.png");
//
//     if (!check_sig(drawable.gd.drawable, message,
//         "\x85\x0a\x9c\x09\x57\xd9\x99\x52\xed\xa8"
//         "\x25\x71\x91\x6c\xf4\xf4\x21\x9a\xe5\x1a")){
//         BOOST_CHECK_MESSAGE(false, message);
//     }
// }

