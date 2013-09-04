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
#define BOOST_TEST_MODULE TestWindowDialog
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/window_dialog.hpp"
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

BOOST_AUTO_TEST_CASE(TraceWindowDialog)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget at position 0,0 in it's parent context
    Widget2* parent = NULL;
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test1",
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
        "\xe5\x62\xb8\x3d\x8e\x5b\x08\x30\xca\xac"
        "\xe2\x80\x2e\xc7\x43\x08\x00\x48\x3f\x74")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test2",
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
        "\x23\xc4\x23\x5f\xae\x9f\x05\x50\x4a\xdf"
        "\x49\x68\x93\x2c\xd6\x5d\x73\x49\xef\xe4")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test3",
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
        "\x0e\x87\xc7\x07\x96\x5c\x04\x9e\x9c\xee"
        "\x57\xfd\x80\xc3\x76\x43\x9d\xf0\xd6\xe6")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test4",
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
        "\xbb\x41\xb2\x44\xeb\x72\x00\xb9\xdf\x4f"
        "\x4b\xe0\x01\x3e\x21\x48\x39\xfb\x49\xd2")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test5",
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
        "\x79\xdc\xfe\x77\x14\xae\x16\x29\x52\x9a"
        "\xd2\xe5\xe6\x3d\x85\x52\x1c\xc2\xb7\x1f")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test6",
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
        "\x28\x2a\xaa\xc7\x98\x97\x81\xf4\x97\x6f"
        "\x4c\x49\xc2\xe9\xaf\xbd\xec\x2d\xf4\x5b")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test6",
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
        "\x54\xe8\xcf\xef\xdc\x04\x96\x0b\x92\xf9"
        "\xec\x18\x85\x4c\xdf\x36\xf7\x94\x3d\xdf")){
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

    WindowDialog window_dialog(drawable, x, y, parent, notifier, "test6",
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
        "\x41\x37\xe3\x2f\xb2\xfb\x1e\x6f\x6c\x9a"
        "\x93\x72\x4f\x8c\x7c\x90\xf3\x9d\x0d\xa4")){
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

    WindowDialog window_dialog(drawable, x, y, parent, &notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    window_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    //drawable.save_to_png("/tmp/window_dialog-clic-button-ok.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x1c\xe9\xa8\x5e\x73\x8a\xae\x52\x43\x01\x69\x29\xb6\xc7\xea\x88\xc2\x51\xfc\x05"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                     window_dialog.ok.dx(), window_dialog.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;
    window_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    //drawable.save_to_png("/tmp/window_dialog-clic-button-cancel.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x5c\x1f\xfe\x98\x20\x6d\x2a\x79\x76\x2f\x11\x14\x78\xf4\x3f\x76\x25\x53\xd4\x10"
        // "\x6c\x65\xdb\xf8\xc7\xbd\xf4\xd5\x08\x5f"
        // "\xd1\x16\x78\xae\x55\x1c\x79\x94\x99\x20"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1, window_dialog.cancel->dx(),
                                          window_dialog.cancel->dy(), NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}
