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

    virtual void draw(const RDPMultiDstBlt &, const Rect &) {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPMultiOpaqueRect &, const Rect &) {
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

    virtual void draw(const RDPPolygonSC &, const Rect &) {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPolygonCB &, const Rect &) {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPPolyline &, const Rect &) {
        BOOST_CHECK(false);
    }

    virtual void draw(const RDPEllipseSC&, const Rect&)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPEllipseCB&, const Rect&)
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
                     fg_color, bg_color, bg_color, edit_pos, xtext, ytext);

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

    WidgetEdit wedit(drawable, x, y, cx, parent, &notifier, "abcdef", 0, GREEN, RED, RED);
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
    bool ctrl_alt_delete;

    keymap.event(0, 16, decoded_data, ctrl_alt_delete); // 'a'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 16, decoded_data, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-1.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb2\xdb\x82\xf4\x0f\xaf\x1f\x49\x29\x51"
                   "\xc4\x1a\xf5\x3e\xac\x37\x89\xfe\x36\xb0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
    BOOST_CHECK(notifier.sender == &wedit);
    BOOST_CHECK(notifier.event == NOTIFY_TEXT_CHANGED);
    notifier.event = 0;
    notifier.sender = 0;

    keymap.event(0, 17, decoded_data, ctrl_alt_delete); // 'z'
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keymap.KBDFLAGS_DOWN|keymap.KBDFLAGS_RELEASE, 17, decoded_data, ctrl_alt_delete);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e2-2.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x9b\xc4\x46\x9e\x70\x07\x84\xe1\x9d\xd8"
                   "\xcd\xde\x68\x7e\xb2\x37\x2f\x90\x4e\x89"
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
                   "\x79\x68\x73\xaa\x18\x2d\xe7\xaa\xb4\x68"
                   "\xcb\x9c\xd7\x1d\x0d\x10\x13\xcf\x58\x36"
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
                   "\x9b\xc4\x46\x9e\x70\x07\x84\xe1\x9d\xd8"
                   "\xcd\xde\x68\x7e\xb2\x37\x2f\x90\x4e\x89"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);

    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e5.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb2\xdb\x82\xf4\x0f\xaf\x1f\x49\x29\x51"
                   "\xc4\x1a\xf5\x3e\xac\x37\x89\xfe\x36\xb0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e6.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x45\x3b\xdc\x15\x05\x48\x96\xab\x99\x72"
                   "\x19\x7d\x05\xb2\xf9\x72\x72\x53\x5d\x7c"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    wedit.rdp_input_scancode(0, 0, 0, 0, &keymap);
    wedit.rdp_input_invalidate(Rect(0, 0, wedit.cx(), wedit.cx()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e7.png");
    if (!check_sig(drawable.gd.drawable, message,
                   "\x34\x32\x7d\x9c\x6a\xdf\x6a\x58\x46\xbb"
                   "\xf7\x56\xe8\xb9\x60\x22\x53\x61\x32\x7d"
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
                   "\xeb\xd9\x3c\x01\x35\xf5\xcf\x59\x53\x2b"
                   "\x03\x60\x47\xf1\xa9\x3a\x4e\xce\xc3\x1c"
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
                   "\x42\x67\x95\x18\xda\xeb\x34\x88\xa4\x04"
                   "\xd0\x54\xf1\x22\xa7\xe9\x9f\x0a\x60\xeb"
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
                   "\xd0\xfa\xdf\x07\x35\x93\x19\x95\xe2\xc2"
                   "\x51\x97\x8c\x14\x5b\xf7\x5c\x4d\xff\x95"
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-e11.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x7b\x12\x34\x1c\xba\xcc\x95\x9d\xae\xe1"
                   "\xd1\x8b\x4e\x93\xf8\x93\x09\xfd\x91\xc4"
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
                      "abababab", 4, YELLOW, BLACK, BLACK);
    WidgetEdit wedit2(drawable, 0,100, 50, wcomposite, notifier,
                        "ggghdgh", 2, WHITE, RED, RED);
    WidgetEdit wedit3(drawable, 100,100, 50, wcomposite, notifier,
                        "lldlslql", 1, BLUE, RED, RED);
    WidgetEdit wedit4(drawable, 300,300, 50, wcomposite, notifier,
                        "LLLLMLLM", 20, PINK, DARK_GREEN, DARK_GREEN);
    WidgetEdit wedit5(drawable, 700,-10, 50, wcomposite, notifier,
                        "dsdsdjdjs", 0, LIGHT_GREEN, DARK_BLUE, DARK_BLUE);
    WidgetEdit wedit6(drawable, -10,550, 50, wcomposite, notifier,
                        "xxwwp", 2, DARK_GREY, PALE_GREEN, PALE_GREEN);

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
                   "\x67\xa6\x1d\x2d\x26\xb0\xb8\x98\xbb\xba"
                   "\x49\xb9\x64\x0c\x8f\x87\x18\x00\xe7\xf5"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "edit10.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x13\x56\x53\xc6\x78\x81\x07\x75\x16\x2e"
                   "\xe0\xf3\x37\xa0\xe5\x72\x58\x08\xe6\xe9"
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
    WidgetEdit wedit(drawable, x, y, cx, parent, &parent, "abcde", 0, BLACK, WHITE, WHITE,
                     -1u, 1, 1);
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
    bool    ctrl_alt_delete;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-1.png");

    keymap.event(keyboardFlags, keyCode + 1, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode + 2, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-2.png");
    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-3.png");

    keymap.event(keyboardFlags, keyCode, decoded_data, ctrl_alt_delete);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    // drawable.save_to_png(OUTPUT_FILE_PATH "edit-s0-4.png");

    keymap.event(keyboardFlags, keyCode + 9, decoded_data, ctrl_alt_delete);
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
