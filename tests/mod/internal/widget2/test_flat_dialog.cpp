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
 *              Meng Tan, Jennifer Inthavong
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFlatDialog
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
#include "utils/log.hpp"

#include "core/font.hpp"
#include "mod/internal/widget2/flat_dialog.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "check_sig.hpp"

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

#include "fake_draw.hpp"

BOOST_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 800, 600, parent, notifier, "test1",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.rect);

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog1.png");

    char message[1024];
    if (!check_sig(drawable.gd.impl(), message,
        "\xf6\x79\x28\xaa\x73\x62\x02\x09\x8a\x7b\xbc\x6d\x81\xc7\xa1\xb2\xd7\x6e\xd2\x4e"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 640, 480, parent, notifier, "test2",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog2.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x64\x4f\xfb\xad\xa1\x7d\xd0\x11\x43\x1c\xfd\xe2\x3a\xcd\x67\xe7\x77\xa3\x54\xce"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 1280, 1024, parent, notifier, "test3",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog3.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xa1\xb7\xce\xbd\x69\x55\x5c\x97\x38\x96\xd4\x89\x5d\x96\x39\xd0\x99\xea\x7b\x29"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestDraw drawable(1280, 1024);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 1280, 1024, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 1280, 1024, parent, notifier, "test4",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog4.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xd7\x57\x39\xbd\x99\x5d\xff\xee\x76\x3c\xf0\xd3\x1b\x31\xfd\xba\xb1\xa4\x78\xe9"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 640, 480, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 640, 480, parent, notifier, "test5",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog5.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x56\x37\x5b\xd5\x40\xc4\xe7\x54\x23\xe0\x51\x32\xe9\x9c\x2e\xb6\x4e\x90\x3f\xdb"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestDraw drawable(350, 500);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 300, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 350, 500, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog6.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xbc\x1a\x27\x17\xaf\x9e\xe2\x71\xb6\x7c\x6f\xdb\xdb\x0d\x93\x6e\x3b\x7a\x0c\x7c"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 300, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      0 + flat_dialog.dy(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog7.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\xe3\x0c\xbc\xfd\x66\xd7\xf2\x26\x43\x59\xb6\x59\x53\x86\x3f\x07\xcd\x8a\x5e\x34"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, font);
    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 800, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.dx(),
                                      5 + flat_dialog.dy(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog8.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x00\x23\xbe\x2f\x39\x72\xfe\x7f\xad\x7d\x89\x90\x88\x85\xac\xbb\xed\x3b\x91\x86"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }
}

BOOST_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, font);
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        virtual void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 800, 600, parent, &notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           0, extra_button, colors, font);

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    unsigned x = flat_dialog.ok.rect.x + flat_dialog.ok.rect.cx / 2 ;
    unsigned y = flat_dialog.ok.rect.y + flat_dialog.ok.rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-1-button-ok.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x7a\x05\x83\xc8\xf7\x52\x91\xce\x20\xa5\xb6\xd8\x11\x04\x47\xb9\xc7\x52\x18\x72"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.dx(), flat_dialog.ok.dy(), nullptr);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-2-button-ok.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x71\xa3\x07\xf2\x93\xb2\x6a\x6e\x63\xe7\x3a\x37\xb0\x89\x8f\xba\xb6\x47\x88\x12"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    x = flat_dialog.cancel->rect.x + flat_dialog.cancel->rect.cx / 2 ;
    y = flat_dialog.cancel->rect.y + flat_dialog.cancel->rect.cy / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-3-button-cancel.png");


    if (!check_sig(drawable.gd.impl(), message,
        "\x27\x41\xda\x14\x2d\xe8\xd1\x8e\x2b\xe3\x66\x1d\x6f\x49\x0f\xfe\x5d\x65\x7c\x91"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_CANCEL);
}

BOOST_AUTO_TEST_CASE(EventWidgetChallenge)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable, 800, 600, font);
    struct Notify : NotifyApi {
        Widget2* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        virtual void notify(Widget2* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 800, 600, parent, &notifier, "test6",
                           "Lorem ipsum dolor sit amet, consectetur<br>"
                           "adipiscing elit. Nam purus lacus, luctus sit<br>"
                           "amet suscipit vel, posuere quis turpis. Sed<br>"
                           "venenatis rutrum sem ac posuere. Phasellus<br>"
                           "feugiat dui eu mauris adipiscing sodales.<br>"
                           "Mauris rutrum molestie purus, in tempor lacus<br>"
                           "tincidunt et. Sed eu ligula mauris, a rutrum<br>"
                           "est. Vestibulum in nunc vel massa condimentum<br>"
                           "iaculis nec in arcu. Pellentesque accumsan,<br>"
                           "quam sit amet aliquam mattis, odio purus<br>"
                           "porttitor tortor, sit amet tincidunt odio<br>"
                           "erat ut ligula. Fusce sit amet mauris neque.<br>"
                           "Sed orci augue, luctus in ornare sed,<br>"
                           "adipiscing et arcu.", 0, extra_button, colors, font,
                           "Ok", "Cancel", CHALLENGE_ECHO);

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);


    flat_dialog.challenge->set_text("challenge_test");

//    BOOST_CHECK(notifier.sender == 0);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.rect);
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-challenge-1.png");

    char message[1024];

    if (!check_sig(drawable.gd.impl(), message,
        "\x18\xfb\x60\x47\x6d\xca\x82\x0b\x54\xf3\xbc\xda\x46\x29\xd2\x12\x5d\x42\x1f\x02"
    )){
        BOOST_CHECK_MESSAGE(false, message);
    }

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    BOOST_CHECK(notifier.sender == &flat_dialog);
    BOOST_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = nullptr;
    notifier.event = 0;
}
