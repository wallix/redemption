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

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "/tmp/"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceWindowDialog)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog1.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xe6\x20\x02\x51\xfd\x01\x5b\xb3\x17\xe5"
                   "\xe0\xd1\xf1\x50\x5a\xdb\xc8\x76\xe6\xfe")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x24\xf2\x2a\x43\x9c\x57\x5d\x23\xe5\xec"
                   "\xec\x13\x3e\x0c\x1b\x57\x42\x80\x93\xf7")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog3)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xf6\x4c\x5c\x3f\x47\xda\xe3\x3f\x90\x66"
                   "\x10\xc0\x5e\xb9\x21\xfa\x51\xda\xbe\x4f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog4)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x1c\x3b\xdb\xfe\xe2\xa6\xd0\x54\x73\x41"
                   "\xd7\xff\xc5\x7e\x64\x99\xc2\x57\xee\xc0"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog5)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xbd\xc6\x42\x54\xc5\x5a\x8e\xed\xb8\xb7"
                   "\x6e\xf4\x7d\x71\xb0\xaf\x64\x5d\xda\x6f")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialog6)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x8b\x5b\xc7\xad\x39\x1a\x71\x0a\xd4\xbd"
                   "\x41\xb2\x14\x59\xd5\x96\xf7\x11\xb5\x5c")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xda\x6d\x2d\x09\x2e\xe4\xe2\xc3\x08\xe0"
                   "\x3d\xa7\x7c\xeb\xa3\xd9\xed\xa6\x08\x69")){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceWindowDialogClip2)
{
    TestDraw drawable(800, 600);

    // WindowDialog is a window_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600);
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog8.png");

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

    x = window_dialog.ok.rect.x + window_dialog.ok.rect.cx / 2 ;
    y = window_dialog.ok.rect.y + window_dialog.ok.rect.cy / 2 ;
    window_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, NULL);
    // window_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog-clic-1-button-ok.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x71\x57\xb6\xd7\x6c\xab\x31\x83\x26\x7f"
                   "\x2e\x5c\xa2\x4f\x89\xb6\x9b\x08\x18\xfa"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, NULL);
    // window_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  window_dialog.ok.dx(), window_dialog.ok.dy(), NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = 0;
    notifier.event = 0;

    window_dialog.rdp_input_invalidate(window_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog-clic-2-button-ok.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\xe3\x95\x3e\x4c\xa9\x07\x74\x62\x65\xf3"
                   "\xa3\x02\x64\xfa\xd0\x80\xc9\x49\x0c\xa7"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    x = window_dialog.cancel->rect.x + window_dialog.cancel->rect.cx / 2 ;
    y = window_dialog.cancel->rect.y + window_dialog.cancel->rect.cy / 2 ;
    window_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, NULL);
    // window_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, NULL);
    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.event == 0);


    window_dialog.rdp_input_invalidate(window_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "window_dialog-clic-3-button-cancel.png");

    if (!check_sig(drawable.gd.drawable, message,
                   "\x50\x0b\xd4\x88\xc3\x96\xae\xd9\x7b\x0a"
                   "\x12\x2b\x1c\x27\x45\x11\x6e\x35\xae\xf1"
                   )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    window_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1, window_dialog.cancel->dx(),
                                          window_dialog.cancel->dy(), NULL);
    BOOST_CHECK(notifier.sender == &window_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}
