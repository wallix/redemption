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
#define BOOST_TEST_MODULE TestWindowDialog
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_dialog.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

struct TestDraw : DrawApi
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

BOOST_AUTO_TEST_CASE(TraceWindowDialog)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test1",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(window_dialog.rect);

    //drawable.save_to_png("/tmp/window_dialog1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xd6\xfd\x08\x65\x27\xb9\x6b\xde\xf9\x17"
        "\x70\xe2\xa0\xdd\xc8\x2a\x1d\x52\x97\x15")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 10,100 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test2",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(Rect(0 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc9\xfa\xf0\xfc\xbd\xa5\x54\xb2\xe4\x75"
        "\x83\x2f\x4f\x44\x75\xfc\xb3\x9a\x9c\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog3)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position -10,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test3",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(Rect(0 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x37\xd4\xdc\xc7\x91\xf0\x8b\x96\xd2\x33"
        "\x38\xab\x44\x1d\x34\x67\x43\xaf\x87\x89")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog4)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 770,500 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test4",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(Rect(0 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x21\x88\x35\x5b\x88\xf8\x79\xc8\xc5\x16"
        "\xac\x21\xac\xd0\x56\x27\xff\xaf\x15\x38")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog5)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position -20,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test5",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(Rect(0 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x7e\x83\x3f\x0e\x06\x19\xd9\x07\xf9\xe4"
        "\x89\x40\x6a\x76\xf9\x1a\xb4\x10\xa6\x05")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog6)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    window_dialog.rdp_input_invalidate(Rect(0 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa6\x1a\x85\xb8\x24\xdb\x65\xdc\xa0\x16"
        "\x02\x4e\x94\x62\x8e\x30\x14\x76\x8c\x03")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 760,-7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    window_dialog.rdp_input_invalidate(Rect(20 + window_dialog.dx(),
                                      0 + window_dialog.dy(),
                                      window_dialog.cx(),
                                      window_dialog.cy()));

    //drawable.save_to_png("/tmp/window_dialog7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xc2\xf7\xfb\x8a\x92\xa8\x47\xe3\x5d\xdd"
        "\xba\x11\x87\x9e\x2c\xd0\x6d\x24\xc6\x8e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 10,7 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowDialog window_dialog(&drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    window_dialog.rdp_input_invalidate(Rect(20 + window_dialog.dx(),
                                      5 + window_dialog.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/window_dialog8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xbd\xe4\x8e\xb6\x24\x84\x0a\xc1\xc8"
        "\x49\x43\x66\x4a\x90\x47\x8b\xc5\xea\x9e")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestDraw drawable(800, 600);

    Widget2* parent = NULL;
    struct Notify : NotifyApi {
        Widget2* sender;
        notify_event_t event;

        Notify()
        : sender(0)
        , event(0)
        {}

        virtual void notify(Widget2* sender, notify_event_t event, long unsigned int param, long unsigned int param2)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    int16_t x = 10;
    int16_t y = 10;

    WindowDialog window_dialog(&drawable, x, y, parent, &notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_dialog.ok.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    //drawable.save_to_png("/tmp/window_dialog-clic-button-ok.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xe1\xf9\xa0\xc4\xc4\x14\x19\x0c\x71\x5d"
        "\x68\xbf\x92\x6a\x42\x50\x04\x14\x59\xd9")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.ok.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
    window_dialog.cancel.rdp_input_mouse(CLIC_BUTTON1_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    //drawable.save_to_png("/tmp/window_dialog-clic-button-cancel.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xba\x59\x10\xe7\x0b\x18\x5e\x53\x7c\x4c"
        "\x18\xae\xd8\x92\xfe\xd2\xc0\x62\xd6\xbf")){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.cancel.rdp_input_mouse(CLIC_BUTTON1_UP, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}
