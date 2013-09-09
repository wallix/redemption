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
#define BOOST_TEST_MODULE TestWidgetPassword
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/password.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
// #define OUTPUT_FILE_PATH "/tmp/"
#define OUTPUT_FILE_PATH "/home/mtan/work/tmp/testwidget/"

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

BOOST_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;
    int xtext = 4;
    int ytext = 1;
    size_t password_pos = 2;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test1", id,
                     fg_color, bg_color, password_pos, xtext, ytext);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dx(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x03\x6a\x4c\x0e\x14\x8a\xda\xdc\xe7\xfd"
        "\xc4\x76\xc6\x06\xeb\xec\xbf\x2c\x4a\xbd")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test2", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x6a\x46\xd1\x01\x00\x13\x71\x82\x60\xd2"
        "\x1c\xca\x2b\xbf\xeb\x51\x95\x0a\x31\x85")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test3", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x8c\xd1\x0f\xfa\xd2\xb0\x17\x1b\x14\x96"
        "\x3f\xe6\x21\xda\x34\x3d\x9f\x12\x58\x8b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test4", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x95\x1a\x73\x7b\xfb\x9a\x83\x97\x4e\x3d"
        "\xec\xcc\x69\x75\x41\xd8\x11\x58\x32\x7f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test5", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xbb\x77\xcd\x23\x6b\xd9\x1f\xa2\xb7\x1e"
        "\x29\x5f\x56\x7f\x51\xe3\xb0\x51\x5d\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at it's current position
    wpassword.rdp_input_invalidate(Rect(0 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x15\xe1\x64\xd1\x89\x70\x1f\xb3\x54\x2c"
        "\x85\xd1\x1b\x6c\x62\x98\x48\xb1\x00\x61")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    0 + wpassword.dy(),
                                    wpassword.cx(),
                                    wpassword.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x6a\xe5\x64\xf9\x84\xc8\xfd\x08\xeb\xbe"
        "\x38\xc5\xca\xde\x3d\xf0\x89\xfc\xfb\x48")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = BLUE;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 50;

    WidgetPassword wpassword(drawable, x, y, cx, parent, notifier, "test6", id, fg_color, bg_color, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wpassword.rdp_input_invalidate(Rect(20 + wpassword.dx(),
                                    5 + wpassword.dy(),
                                    30,
                                    10));

    drawable.save_to_png(OUTPUT_FILE_PATH "password8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x91\xe4\x76\x91\x59\xf4\x56\x1a\xe2\x80"
        "\xb3\xf4\x6c\x7e\x56\x69\xe0\xdb\xa6\x70")){
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
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "abcdef", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x5d\x25\x2f\x5b\xdc\x08\x8f\x2e\x7a\x5b"
        "\x4c\x17\x03\x08\x4c\xdd\xee\xd8\x0a\x42")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);

    keymap.event(0, 16, decoded_data); // 'a'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xeb\xdf\x38\xb1\x33\x78\x7f\x3a\xac\x63"
        "\x0c\xe3\x3c\x1d\x33\x35\xd2\xb9\xf5\x5b")){
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
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x71\xd8\xfe\x00\x4f\xa5\xed\xc3\xc7\x43"
        "\x9c\x71\x9b\xa1\x99\xd7\x7b\xbb\x49\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xbc\xbb\x69\x23\x6f\x52\xeb\xd0\xbe\xcb"
        "\xfe\xba\x43\x5e\xf6\x6c\x71\x2b\xdd\x24")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x71\xd8\xfe\x00\x4f\xa5\xed\xc3\xc7\x43"
        "\x9c\x71\x9b\xa1\x99\xd7\x7b\xbb\x49\x73")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xeb\xdf\x38\xb1\x33\x78\x7f\x3a\xac\x63"
        "\x0c\xe3\x3c\x1d\x33\x35\xd2\xb9\xf5\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x64\x1a\x54\x5f\x14\x44\x2f\xe6\x25\x5f"
        "\x7b\x34\x4a\x58\x7f\x83\xc6\xb6\xa7\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x66\xbf\x34\xf3\x9a\xf3\x33\xc0\x9a\xed"
        "\x5b\x69\x31\x1b\xa7\x0d\x0f\xdf\x28\xae")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xf8\x79\x0e\x30\x93\x1f\x64\xf6\x74\xed"
        "\xa4\xc3\xb8\x94\x83\x44\xe7\x01\x64\x68")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x5d\x25\x2f\x5b\xdc\x08\x8f\x2e\x7a\x5b"
        "\x4c\x17\x03\x08\x4c\xdd\xee\xd8\x0a\x42")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit1-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x62\xfc\xc0\xe7\xd1\x3a\x74\x45\xec\x9e"
        "\xab\x2d\xd9\x71\x9f\xc5\x04\xf8\x66\x2c")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    struct WidgetReceiveEvent : public Widget2 {
        Widget2* sender;
        NotifyApi::notify_event_t event;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(NULL)
        , event(0)
        {}

        virtual void draw(const Rect&)
        {}

        virtual void notify(Widget2* sender, NotifyApi::notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3, 0);
    BOOST_CHECK(widget_for_receive_event.sender == 0);
    BOOST_CHECK(widget_for_receive_event.event == 0);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    notifier.sender = 0;
    notifier.event = 0;
    widget_for_receive_event.sender = 0;
    widget_for_receive_event.event = 0;

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit-e11.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xb5\x4c\x2a\x8e\x46\x5a\x14\xd6\xe5\x66"
        "\x17\xbe\x35\xba\xf4\x59\x1e\xcd\xe1\x30")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetPassword is a password widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetPassword wpassword1(drawable, 0,0, 50, wcomposite, notifier,
                        "abababab", 4, YELLOW, BLACK);
    WidgetPassword wpassword2(drawable, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED);
    WidgetPassword wpassword3(drawable, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED);
    WidgetPassword wpassword4(drawable, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN);
    WidgetPassword wpassword5(drawable, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
    WidgetPassword wpassword6(drawable, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, DARK_GREY, PALE_GREEN);

    wcomposite.child_list.push_back(&wpassword1);
    wcomposite.child_list.push_back(&wpassword2);
    wcomposite.child_list.push_back(&wpassword3);
        wcomposite.child_list.push_back(&wpassword4);
    wcomposite.child_list.push_back(&wpassword5);
    wcomposite.child_list.push_back(&wpassword6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    drawable.save_to_png(OUTPUT_FILE_PATH "password-compo1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x79\x0b\x50\xbe\x9a\x54\x12\xd4\xd8\x2c"
        "\xe2\xc8\xbe\x0b\xb5\x7f\x2a\x04\xcc\x1f")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    drawable.save_to_png(OUTPUT_FILE_PATH "password-compo2.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\x17\x0a\xca\xd2\x8f\x4a\x58\x7f\xde\xbf"
        "\x8e\x00\x91\xf6\x55\x65\x10\x46\xed\x90")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.child_list.clear();
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xeb\xdf\x38\xb1\x33\x78\x7f\x3a\xac\x63"
        "\x0c\xe3\x3c\x1d\x33\x35\xd2\xb9\xf5\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x64\x1a\x54\x5f\x14\x44\x2f\xe6\x25\x5f"
        "\x7b\x34\x4a\x58\x7f\x83\xc6\xb6\xa7\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x66\xbf\x34\xf3\x9a\xf3\x33\xc0\x9a\xed"
        "\x5b\x69\x31\x1b\xa7\x0d\x0f\xdf\x28\xae")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\xc4\x25\x06\x6a\x4a\x8c\x9b\x91\xad"
        "\x2f\xed\x3a\x75\x1b\x01\x67\xf7\x06\x89")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit2-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x7f\xa9\x88\x97\x67\x41\x55\xa5\x28\x92"
        "\x84\xfd\x24\x20\x80\x14\xcf\xe6\xfe\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("******"), std::string(wpassword.show_text()));
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword2)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xeb\xdf\x38\xb1\x33\x78\x7f\x3a\xac\x63"
        "\x0c\xe3\x3c\x1d\x33\x35\xd2\xb9\xf5\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x64\x1a\x54\x5f\x14\x44\x2f\xe6\x25\x5f"
        "\x7b\x34\x4a\x58\x7f\x83\xc6\xb6\xa7\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x66\xbf\x34\xf3\x9a\xf3\x33\xc0\x9a\xed"
        "\x5b\x69\x31\x1b\xa7\x0d\x0f\xdf\x28\xae")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\xc4\x25\x06\x6a\x4a\x8c\x9b\x91\xad"
        "\x2f\xed\x3a\x75\x1b\x01\x67\xf7\x06\x89")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xe6\x60\xfe\x34\x13\xf2\xd8\x16\x75\x68"
        "\x85\x62\xd0\xbe\x69\x4d\xff\x2a\xb0\x72")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit3-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x7f\xa9\x88\x97\x67\x41\x55\xa5\x28\x92"
        "\x84\xfd\x24\x20\x80\x14\xcf\xe6\xfe\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    BOOST_CHECK_EQUAL(std::string("aurlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("******"), std::string(wpassword.show_text()));
}

BOOST_AUTO_TEST_CASE(DataWidgetPassword3)
{
    TestDraw drawable(800, 600);

    struct Notify : public NotifyApi {
        Widget2* sender;
        notify_event_t event;
        Notify()
        : sender(0)
        , event(0)
        {}
        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    WidgetScreen parent(drawable, 800, 600);
    // Widget2* parent = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetPassword wpassword(drawable, x, y, cx, parent, &notifier, "aurélie", 0, YELLOW, 0x0000FF);
    wpassword.focus();
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xeb\xdf\x38\xb1\x33\x78\x7f\x3a\xac\x63"
        "\x0c\xe3\x3c\x1d\x33\x35\xd2\xb9\xf5\x5b")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurélie"), std::string(wpassword.get_text()));

    Keymap2 keymap;
    keymap.init_layout(0x040C);


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e2.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x64\x1a\x54\x5f\x14\x44\x2f\xe6\x25\x5f"
        "\x7b\x34\x4a\x58\x7f\x83\xc6\xb6\xa7\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x66\xbf\x34\xf3\x9a\xf3\x33\xc0\x9a\xed"
        "\x5b\x69\x31\x1b\xa7\x0d\x0f\xdf\x28\xae")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\xc4\x25\x06\x6a\x4a\x8c\x9b\x91\xad"
        "\x2f\xed\x3a\x75\x1b\x01\x67\xf7\x06\x89")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xe6\x60\xfe\x34\x13\xf2\xd8\x16\x75\x68"
        "\x85\x62\xd0\xbe\x69\x4d\xff\x2a\xb0\x72")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\xec\xc4\x25\x06\x6a\x4a\x8c\x9b\x91\xad"
        "\x2f\xed\x3a\x75\x1b\x01\x67\xf7\x06\x89")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    BStream decoded_data(256);

    keymap.event(0, 17, decoded_data); // 'z'
    wpassword.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data);
    wpassword.rdp_input_invalidate(wpassword.rect);
    drawable.save_to_png(OUTPUT_FILE_PATH "password-edit4-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
        "\x44\xa7\xf0\xb0\x27\xa2\x49\x0e\xac\x0d"
        "\x3b\x31\x51\x3c\xf2\x8f\x86\xf6\x65\x1d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wpassword);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    BOOST_CHECK_EQUAL(std::string("aurézlie"), std::string(wpassword.get_text()));
    BOOST_CHECK_EQUAL(std::string("********"), std::string(wpassword.show_text()));
}
