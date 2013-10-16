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
#define BOOST_TEST_MODULE TestWidgetEdit
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/edit.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"

// #include "internal/widget2/widget_composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif
#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

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

    virtual void draw(const RDPGlyphIndex&, const Rect&, const GlyphCache * gly_cache)
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

    virtual void draw(const RDPPolyline&, const Rect&)
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
                // width += font_item->incby;
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
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xbc\xd1\x30\xc8\xef\x21\x5c\xc1\x3e\x95"
                   "\x06\xb9\xde\xe2\x54\xa4\x68\xbb\x03\x39"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xfc\xa6\xe1\x51\x73\xde\xd0\xcf\x8e\xa6"
                   "\x0a\xf7\x58\xdf\xa8\x7a\x1a\xc8\x5b\x28"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x2c\x77\x71\xfe\xba\xf2\x59\xb9\x56\x93"
                   "\xa3\x2d\x8a\xec\x9f\x40\x89\x64\x12\xd2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x5c\x87\xba\xef\xd1\x3b\x9e\x74\x39\x34"
                   "\x45\xd4\x64\xac\xd0\xec\x47\x05\xb2\x97"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xbf\x45\x46\x98\x1c\x13\x54\x5f\xbf\x83"
                   "\x81\x74\x32\x88\x9d\x7d\xf1\x74\x5c\x78"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x37\xcf\x67\x14\x4f\x58\xd3\xdd\x2e\x25"
                   "\x95\x8e\xaa\x85\x06\xf3\xea\x96\xc0\x64"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xc8\xdd\x05\x7b\x43\x3f\x1b\x1d\x04\x4f"
                   "\x80\x60\x52\x04\x1a\xf5\x10\x07\xf5\xe4"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit8.png");

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
        : Widget2(drawable, Rect(), *this, NULL)
        , sender(0)
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

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEdit wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED);
    wedit.focus();

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e1.png");
    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x1f\x0d\xf0\x54\xd2\x85\xeb\x62\x98\xa2"
                   "\x4e\x37\x47\x88\x61\xd1\x45\x3f\xa8\x0c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    BStream decoded_data(256);

    keymap.event(0, 16, decoded_data); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xdb\x5a\x41\xd8\x36\x9a\xa8\xda\x2e\x48"
                   "\x0e\x8d\xe1\xa1\x37\xbf\x7d\xf4\xf1\xbc"
                   )){
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
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd8\x11\xb1\xf0\x30\x2d\xda\x4a\x6b\xcd"
                   "\x30\xc0\x98\x21\xf0\xf2\x0c\x9c\x8f\x2e"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.push_kevent(Keymap2::KEVENT_UP_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e3.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9d\xa0\xa0\x2e\x1e\xac\xea\xba\x3d\x76"
                   "\x19\x78\xa6\x58\xf4\xe4\xd4\x02\x90\xc2"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e4.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xd8\x11\xb1\xf0\x30\x2d\xda\x4a\x6b\xcd"
                   "\x30\xc0\x98\x21\xf0\xf2\x0c\x9c\x8f\x2e"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xdb\x5a\x41\xd8\x36\x9a\xa8\xda\x2e\x48"
                   "\x0e\x8d\xe1\xa1\x37\xbf\x7d\xf4\xf1\xbc"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x4e\x64\x11\x92\x47\xf4\x34\x64\x9e\x9f"
                   "\x66\xdb\x43\x48\x21\xe1\xd1\x64\xd6\xc0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xda\x4a\x55\xb3\xb6\x77\xf2\xc7\x2e\xd1"
                   "\xfc\x0b\x45\xc3\x30\x05\xc4\x14\xbe\x9d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_DELETE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e8.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xfe\x9e\xb6\x72\x17\x09\x52\x5b\xf2\xc2"
                   "\x02\xc9\x33\xc3\x9c\x79\x53\x13\x54\x73"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_END);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e9.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x21\xb7\x51\xef\xe9\x2b\xac\x67\x87\x4f"
                   "\xd7\x88\xc8\x2b\xfe\x32\xd5\x24\x02\xad"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e10.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf9\x42\xa1\x3b\x32\x82\xed\x3e\x2a\x08"
                   "\x84\xb7\x0c\x34\xa1\x2f\xeb\x92\x0d\x8d"
                   )){
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x1c\xc6\x51\xdf\x89\x7e\x59\xe2\x12\xd3"
                   "\x1d\xc6\x34\xac\x52\xa4\x1b\x7c\xed\x1b"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestDraw drawable(800, 600);

    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;

    WidgetComposite wcomposite(drawable, Rect(0,0,800,600), parent, notifier);

    WidgetEdit wedit1(drawable, 0,0, 50, wcomposite, notifier,
                        "abababab", 4, YELLOW, BLACK);
    WidgetEdit wedit2(drawable, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED);
    WidgetEdit wedit3(drawable, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED);
    WidgetEdit wedit4(drawable, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN);
    WidgetEdit wedit5(drawable, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE);
    WidgetEdit wedit6(drawable, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, DARK_GREY, PALE_GREEN);

    wcomposite.add_widget(&wedit1);
    wcomposite.add_widget(&wedit2);
    wcomposite.add_widget(&wedit3);
    wcomposite.add_widget(&wedit4);
    wcomposite.add_widget(&wedit5);
    wcomposite.add_widget(&wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit9.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xab\x96\xd8\x1c\x19\x79\x3f\x6b\x3c\x63"
                   "\x08\x99\xcc\x4c\x9c\x8f\xbf\xdf\xbf\x3d"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x2d\xd9\x2c\x44\x19\x2c\x28\xe4\x7e\x56"
                   "\x54\xcb\xfb\x4f\xc2\x9e\xad\x6d\x7f\x07"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    wcomposite.clear();
}

BOOST_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestDraw drawable(800, 600);
    WidgetScreen parent(drawable, 800, 600);
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;
    WidgetEdit wedit(drawable, x, y, cx, parent, &parent, "abcde", 0, BLACK, WHITE, -1u, 1, 1);
    wedit.focus();
    parent.add_widget(&wedit);
    parent.current_focus = &wedit;

    char message[1024];

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0.png");
    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    BStream decoded_data(256);
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-1.png");

    keymap.event(keyboardFlags, keyCode + 1, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-2.png");
    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-3.png");

    keymap.event(keyboardFlags, keyCode, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-4.png");

    keymap.event(keyboardFlags, keyCode + 9, decoded_data);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1.png");

    keymap.push_kevent(Keymap2::KEVENT_HOME);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1-1.png");

    keymap.push_kevent(Keymap2::KEVENT_END);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s1-2.png");

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s2.png");

    for (int i = 0; i < 10; i++) {
        keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
        parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    }
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s2-1.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x99\xde\xb6\xac\xe3\x9e\x45\xf6\x42\x49"
                   "\xf3\xcd\x9b\x86\xee\xa3\x8f\x79\x50\x95"
                   )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    parent.clear();
}
