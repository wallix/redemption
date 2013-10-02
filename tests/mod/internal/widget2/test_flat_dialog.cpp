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

#undef FIXTURES_PATH
#define FIXTURES_PATH "./tests/fixtures"
#undef SHARE_PATH
#define SHARE_PATH "./tests/fixtures"

#include "internal/widget2/flat_dialog.hpp"
#include "internal/widget2/screen.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
#include "check_sig.hpp"

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

    // char message[1024];
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
    //     "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }

    char message[1024];
    if (!check_sig(drawable.gd.drawable, message,
                   "\xa4\xdf\x36\xbc\xf7\x73\xd2\x64\x2c\x72"
                   "\xa0\xfd\x44\xbc\x01\x7b\x16\x94\x79\x4c"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //                "\xcc\xd9\x69\xc0\x0c\x8a\x18\xea\x84\x1a"
    //                "\x98\x05\x75\x64\xd2\xe0\xfe\x33\xa3\xf1"
    //                )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x06\x66\x4f\x75\x53\x1f\x57\x7b\x20\xde"
                   "\x95\xe3\x9b\x98\xa2\xa7\xd7\xe0\xae\xdf"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
    //     "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x99\x0f\x52\x98\x0a\xa0\x7b\x43\xc1\xba"
                   "\x03\x90\x5b\x18\x1e\xcc\xd5\xf9\xec\x54"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x3e\xd3\x61\xa2\xe8\x24\x78\x30\x91\xa8"
    //     "\x09\x7d\x0d\x4f\x83\x9e\x3b\xfa\xe6\xd3"
    //                )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xe7\xab\xf1\xd1\xcf\xfc\xbb\xf1\x01\x52"
                   "\x2e\x9d\xb2\xf8\x50\x21\xef\xaf\xc0\xe8"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
    //     "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xea\x0d\x85\xe8\xde\x9c\x76\x03\xf1\xff"
                   "\x44\xde\xa4\xc9\x1f\x67\x8b\x86\x89\x8f"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
    //     "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xea\x0d\x85\xe8\xde\x9c\x76\x03\xf1\xff"
                   "\x44\xde\xa4\xc9\x1f\x67\x8b\x86\x89\x8f"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x2b\x42\x09\x08\xdb\x16\xac\x0a\x02\xad"
    //     "\x3f\x13\xe4\xa6\xa2\x3f\x9d\x52\x61\xa6"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xea\x0d\x85\xe8\xde\x9c\x76\x03\xf1\xff"
                   "\x44\xde\xa4\xc9\x1f\x67\x8b\x86\x89\x8f"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x96\xff\xd9\x05\x44\x9b\x19\x2b\x07\xeb"
    //     "\x28\x2c\xf2\xc4\x82\x16\x1d\x36\x06\x70"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\xb3\x33\x3a\xa5\xe8\xba\xaa\xd2\x48\xbe"
                   "\x20\x97\x62\xf3\x73\xb8\x8c\x7f\x27\x73"
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
    // if (!check_sig(drawable.gd.drawable, message,
    //     "\x75\xb0\xc0\x5b\xb9\x6c\x89\xb1\xdb\x71"
    //     "\xc1\x91\x85\xbb\x58\xf8\x38\x40\x81\x40"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x8e\x1c\xef\x0f\xe2\xd8\x46\x00\xbb\x76"
                   "\xea\x6a\x41\xa8\xd2\xfe\xec\x64\x78\x9f"
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

    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xa6\x6f\x34\x42\x49\x3c\x36\x5b\x8f\x80"
    //     "\xc5\xca\xb6\xb3\x24\x66\x9e\x78\x5f\xb1"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }
    if (!check_sig(drawable.gd.drawable, message,
                   "\x3a\xe0\xc2\x68\xa5\xe0\xb7\x6e\xfd\xcb"
                   "\xe8\x82\x5f\xb5\x56\xe1\xcb\xf4\xde\x04"
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

    // if (!check_sig(drawable.gd.drawable, message,
    //     "\xef\x7d\xc6\x6f\xaf\x9b\xf4\x62\x37\x44"
    //     "\x5d\xbe\x44\x5b\x33\x81\x46\x53\xdd\x00"
    // )){
    //     BOOST_CHECK_MESSAGE(false, message);
    // }

    if (!check_sig(drawable.gd.drawable, message,
                   "\x26\xe8\xbb\x12\xed\xa1\x95\x30\x86\x69"
                   "\x71\x76\xb4\x5f\xf2\xa2\x6d\xb1\xd0\xc9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, NULL);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}
