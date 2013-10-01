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
#define BOOST_TEST_MODULE TestFlatDialog
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "internal/widget2/flat_dialog.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

#ifndef FIXTURES_PATH
# define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test1",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
        "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 10;
    int16_t y = 100;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test2",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\xcc\xd9\x69\xc0\x0c\x8a\x18\xea\x84\x1a"
        "\x98\x05\x75\x64\xd2\xe0\xfe\x33\xa3\xf1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = -10;
    int16_t y = 500;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test3",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
        "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 770;
    int16_t y = 500;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test4",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x3e\xd3\x61\xa2\xe8\x24\x78\x30\x91\xa8"
        "\x09\x7d\x0d\x4f\x83\x9e\x3b\xfa\xe6\xd3"
   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = -20;
    int16_t y = -7;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test5",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
        "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
        "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 760;
    int16_t y = -7;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
        "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestDraw drawable(800, 600);

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
    NotifyApi * notifier = NULL;
    int16_t x = 0;
    int16_t y = 0;

    FlatDialog flat_dialog(drawable, x, y, parent, notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      5 + flat_dialog.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog8.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x96\xff\xd9\x05\x44\x9b\x19\x2b\x07\xeb"
        "\x28\x2c\xf2\xc4\x82\x16\x1d\x36\x06\x70"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOkCancel)
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

        virtual void notify(Widget2* sender, notify_event_t event)
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    FlatDialog flat_dialog(drawable, 800, 600, parent, &notifier, "test6",
                               "line 1<br>"
                               "line 2<br>"
                               "<br>"
                               "line 3, blah blah<br>"
                               "line 4");

    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    unsigned x = flat_dialog.ok.rect.x + flat_dialog.ok.rect.cx / 2 ;
    unsigned y = flat_dialog.ok.rect.y + flat_dialog.ok.rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, NULL);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-1-button-ok.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
        "\x75\xb0\xc0\x5b\xb9\x6c\x89\xb1\xdb\x71"
        "\xc1\x91\x85\xbb\x58\xf8\x38\x40\x81\x40"
   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, NULL);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.dx(), flat_dialog.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-2-button-ok.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xa6\x6f\x34\x42\x49\x3c\x36\x5b\x8f\x80"
        "\xc5\xca\xb6\xb3\x24\x66\x9e\x78\x5f\xb1"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    x = flat_dialog.cancel->rect.x + flat_dialog.cancel->rect.cx / 2 ;
    y = flat_dialog.cancel->rect.y + flat_dialog.cancel->rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, NULL);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-3-button-cancel.png");

    if (!check_sig(drawable.gd.drawable, message,
        "\xef\x7d\xc6\x6f\xaf\x9b\xf4\x62\x37\x44"
        "\x5d\xbe\x44\x5b\x33\x81\x46\x53\xdd\x00"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, NULL);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}
