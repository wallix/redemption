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
#include "internal/widget2/screen.hpp"
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

    // WindowDialog is a dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    const char * cancel = 0;

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test1",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0, 0, wdialog.cx(), wdialog.cy()));


    //drawable.save_to_png("/tmp/dialog.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfb\x84\x4d\x48\x61\x0d\x08\x8d\x5f\xa0\x63\xb6\x8a\x5d\x42\x97\x37\x7a\x97\x7c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 100;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test2",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x79\x4a\x0f\xed\xad\xae\xbd\x3d\x94\x99\xe3\xf9\x10\xfb\x6b\xf8\x7e\x4e\x0c\x26")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog3)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -10;
    int16_t y = 500;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test3",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x1d\xc2\x6b\x72\x14\x5f\xeb\x64\xfc\x1a"
        "\x69\x4f\xce\x55\xef\xef\xd1\x53\x66\xc7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog4)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 770;
    int16_t y = 500;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test4",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xf7\x26\x94\xdb\x18\x6a\x9b\xfd\xb6\x22"
        "\x53\xe2\x9e\x06\x51\xff\xc7\x91\xeb\x7d")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog5)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = -20;
    int16_t y = -7;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test5",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xfe\x24\x78\x02\x5a\x9d\x58\x21\xdb\xbe\xb8\x11\xaf\x79\xdb\x8f\x34\xf2\xe0\x68")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog6)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at it's current position
    wdialog.rdp_input_invalidate(Rect(0 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x92\xde\xd8\x4e\x62\x0a\x59\xa6\x6a\xd1\x03\x0b\x82\x8c\xc5\xc2\xcd\xf3\x55\xb1")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 760;
    int16_t y = -7;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    wdialog.rdp_input_invalidate(Rect(20 + wdialog.dx(),
                                      0 + wdialog.dy(),
                                      wdialog.cx(),
                                      wdialog.cy()));

    //drawable.save_to_png("/tmp/dialog7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xa3\x90\xd4\x2b\x42\xb5\x24\xf1\x0d\x76\xb5\xe5\x9c\x40\xa0\x61\xb0\x07\xb0\x22")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    wdialog.rdp_input_invalidate(Rect(20 + wdialog.dx(),
                                      5 + wdialog.dy(),
                                      30,
                                      10));

    //drawable.save_to_png("/tmp/dialog8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x41\x37\xe3\x2f\xb2\xfb\x1e\x6f\x6c\x9a"
        "\x93\x72\x4f\x8c\x7c\x90\xf3\x9d\x0d\xa4")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOk)
{
    TestDraw drawable(800, 600);

    WidgetScreen parent(drawable, 800, 600);
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
    int fg_color = RED;
    int bg_color = YELLOW;
    int id = 0;
    int16_t x = 10;
    int16_t y = 10;
    const char * cancel = "Close";

    WindowDialog wdialog(drawable, x, y, parent, &notifier, "test6",
                         "Lorem ipsum dolor sit amet, consectetur adipiscing elit.<br>"
                         "Curabitur sit amet eros rutrum mi ultricies tempor.<br>"
                         "Nam non magna sit amet dui vestibulum feugiat.<br>"
                         "Praesent vitae purus et lacus tincidunt lobortis.<br>"
                         "Nam lacinia purus luctus ante congue facilisis.<br>"
                         "Donec sodales mauris luctus ante ultrices blandit.",
                         id, "Ok", cancel, fg_color, bg_color, fg_color, bg_color);


    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wdialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                               wdialog.ok.dx(), wdialog.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wdialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                               wdialog.ok.dx(), wdialog.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &wdialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = 0;
    notifier.event = 0;
    wdialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                                    wdialog.cancel->dx(), wdialog.cancel->dy(), NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);
    wdialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                                    wdialog.cancel->dx(), wdialog.cancel->dy(), NULL);
    BOOST_CHECK(notifier.sender == &wdialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}

