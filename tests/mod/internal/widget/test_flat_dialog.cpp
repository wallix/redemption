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

#define RED_TEST_MODULE TestFlatDialog
#include "system/redemption_unit_tests.hpp"

#include "utils/log.hpp"

#include "core/font.hpp"
#include "mod/internal/widget/flat_dialog.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, notifier, "test1",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog1.png");

    RED_CHECK_SIG(drawable.gd, "\x3a\xe2\xea\xc3\xa0\xd4\x33\x56\xc9\xb9\xdd\xe5\xed\xaf\x97\x7d\xf4\xa7\xf6\xac");
}

RED_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 640, 480, parent, notifier, "test2",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog2.png");

    RED_CHECK_SIG(drawable.gd, "\xec\xa6\x6d\x84\xee\x77\x2b\x6f\xd5\x7f\x4d\xf4\x4b\x25\x76\xd4\xb6\x6e\x64\x05");
}

RED_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 1280, 1024, parent, notifier, "test3",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog3.png");

    RED_CHECK_SIG(drawable.gd, "\xf8\xb5\x49\x1d\x2c\x28\xf4\xfc\x89\x7c\xb5\x82\x51\x14\xeb\xc2\x67\x62\x2e\x4d");
}

RED_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestDraw drawable(1280, 1024);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(1280, 1024);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 1280, 1024, parent, notifier, "test4",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog4.png");

    RED_CHECK_SIG(drawable.gd, "\xc8\xfe\xeb\xb4\xa5\x0f\xdf\x65\x82\x19\x3d\xd8\xf2\x13\x95\x6d\xf0\x35\xcc\x64");
}

RED_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(640, 480);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 640, 480, parent, notifier, "test5",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog5.png");

    RED_CHECK_SIG(drawable.gd, "\x30\x67\xeb\xb9\x00\x68\xe4\xf0\x82\x94\x9f\x6f\x27\x12\xae\xab\x8e\x97\x94\x4f");
}

RED_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestDraw drawable(350, 500);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(300, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 350, 500, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog6.png");

    RED_CHECK_SIG(drawable.gd, "\x2a\xe4\x67\x13\x3e\x31\x5f\xc8\xb3\x3c\x85\xe4\xba\x1d\xfd\xe5\xe9\x22\xc5\x98");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 300, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    //drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog7.png");

    RED_CHECK_SIG(drawable.gd, "\x51\x9a\xaa\x4e\xc7\x52\xcc\x5d\xf0\x68\x2a\x23\xeb\xec\x3d\x38\xe1\xcd\x75\x1d");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      5 + flat_dialog.y(),
                                      30,
                                      10));

    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog8.png");

    RED_CHECK_SIG(drawable.gd, "\x04\xeb\x2e\x1a\x17\x55\x38\x6c\xb9\x9e\x1e\x5e\x01\x0f\x5a\x98\xe4\x6f\x4e\x0f");
}

RED_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    struct Notify : NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, font);

//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);

    unsigned x = flat_dialog.ok.x() + flat_dialog.ok.cx() / 2 ;
    unsigned y = flat_dialog.ok.y() + flat_dialog.ok.cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-1-button-ok.png");

    RED_CHECK_SIG(drawable.gd, "\x55\x9f\xd3\xe0\xe1\xe3\x5f\x7b\xa3\xe9\xdf\x2d\x2d\x05\xc0\xf7\x2a\xe3\xd1\x05");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.x(), flat_dialog.ok.y(), nullptr);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-2-button-ok.png");


    RED_CHECK_SIG(drawable.gd, "\x37\xf3\x9d\xf6\x9a\xc1\x9e\x0d\xaf\xac\x29\x9a\xe7\xd1\x56\xeb\x1b\xe9\xf7\x59");


    x = flat_dialog.cancel->x() + flat_dialog.cancel->cx() / 2 ;
    y = flat_dialog.cancel->y() + flat_dialog.cancel->cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-clic-3-button-cancel.png");


    RED_CHECK_SIG(drawable.gd, "\x3f\x0d\x03\x36\xb9\xb3\x7e\xc5\x11\xe9\x6c\xff\x88\xca\xef\x03\x5f\x33\x70\xfd");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);

    notifier.sender = nullptr;
    notifier.event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(EventWidgetChallenge)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    struct Notify : NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;

        Notify() = default;

        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable.gd, 0, 0, 800, 600, parent, &notifier, "test6",
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
                           "adipiscing et arcu.", extra_button, colors, font,
                           "Ok", "Cancel", CHALLENGE_ECHO);

//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);


    flat_dialog.challenge->set_text("challenge_test");

//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png(OUTPUT_FILE_PATH "flat_dialog-challenge-1.png");

    RED_CHECK_SIG(drawable.gd, "\x3d\xe7\xb6\x5c\x61\x8e\xc4\x9c\xbc\xf5\xe5\x32\xa9\xe8\x74\xef\x50\x60\xfd\x86");

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = nullptr;
    notifier.event = 0;
}
