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


    // drawable.save_to_png(OUTPUT_FILE_PATH "dialog.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xc3\x64\xa9\x43\x6e\x5f\x7d\xda\x67\xf6"
                   "\x14\x31\x72\xfc\x79\x52\x2b\x19\x46\x4d")){
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "dialog2.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x52\x7e\xf8\xe8\x8a\x17\x88\x2e\x82\xf5"
                   "\x42\x3e\x43\xef\x4a\xa7\xcf\x9d\x40\x78")){
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "dialog3.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x00\x41\x86\x08\x90\x72\xa4\x88\xf8\x7a"
                   "\xa7\x74\x62\xa6\x43\x4d\xce\x32\xe4\x16")){
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "dialog4.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xff\xc7\x17\x44\x65\x08\x98\x28\x65\xf6"
                   "\x39\x5d\x51\x38\x8a\xdb\x5a\xce\x02\x33")){
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

    // drawable.save_to_png(OUTPUT_FILE_PATH "dialog5.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x5b\x93\xb4\x9b\x2e\x8a\x34\x03\x40\xec"
                   "\x4a\xaf\xd1\x69\x62\x0d\x3b\xb2\x75\xd2")){
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

    // drawable.save_to_png("/tmp/dialog6.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa7\xc7\x21\x29\x1b\x12\x05\xca\xbb\xe7"
                   "\x39\xe7\x24\xb4\x9c\x30\x0c\xfe\x82\xaf")){
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

    // drawable.save_to_png("/tmp/dialog7.png");

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\x05\x8b\x3f\x74\xad\xc4\xb4\x23\x7f\x2c"
                   "\x93\x6e\xbd\x35\xb9\x35\xe7\xf0\xb6\x61")){
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

    // drawable.save_to_png("/tmp/dialog8.png");

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

        virtual void notify(Widget2* sender, notify_event_t event)
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

