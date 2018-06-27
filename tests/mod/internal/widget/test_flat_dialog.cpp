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

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog1.png");

    RED_CHECK_SIG(drawable.gd, "\xb1\x58\x70\xb2\xa3\xf1\x8f\x2f\xdd\x4a\xc1\xbb\xcd\xf3\x50\xad\x11\xc1\xe0\x9c");
}

RED_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog2.png");

    RED_CHECK_SIG(drawable.gd, "\xd4\x59\x7a\x79\x69\xfd\x80\x03\xe8\xb8\x04\xf4\x57\x2b\xd0\xbc\x66\x8f\x31\xb7");
}

RED_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog3.png");

    RED_CHECK_SIG(drawable.gd, "\x50\xc0\xdf\x77\x69\x22\xb7\x1d\x6c\xd5\x9b\x06\x52\xd6\x3f\xa1\x72\xfb\xe3\xb1");
}

RED_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestDraw drawable(1280, 1024);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog4.png");

    RED_CHECK_SIG(drawable.gd, "\x2a\x3c\xe5\xf6\x37\xe5\xc2\x38\x98\xdc\x05\xe6\x83\x95\x70\x7b\xe8\x84\x48\xda");
}

RED_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestDraw drawable(640, 480);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog5.png");

    RED_CHECK_SIG(drawable.gd, "\xb9\xc1\x6e\xcf\x52\x8d\xae\x31\xe7\xfe\xe8\x5a\x8f\x16\x7a\xaf\x5c\x35\xca\x7b");
}

RED_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestDraw drawable(352, 500);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog6.png");

    RED_CHECK_SIG(drawable.gd, "\xd0\xda\x5c\x84\xf1\x08\xd6\x94\xbf\x28\x1e\x64\xa3\x9a\x05\xb3\x91\x91\xce\x8e");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog7.png");

    RED_CHECK_SIG(drawable.gd, "\xcd\x06\xae\x60\x85\x3c\x96\x41\x71\xea\x41\x83\x8b\xa0\x47\x52\x8a\xe6\xa8\x9c");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("flat_dialog8.png");

    RED_CHECK_SIG(drawable.gd, "\x58\x1e\xa2\x28\xe9\xe8\x75\x01\x56\x43\x0a\x09\xc5\xb9\x5d\x72\x43\x30\xba\x66");
}

RED_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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
    // drawable.save_to_png("flat_dialog-clic-1-button-ok.png");

    RED_CHECK_SIG(drawable.gd, "\xaf\x07\xd6\x95\xf2\x9a\x4b\x8e\x8a\x7e\xc8\x29\x85\xcc\x07\x43\xec\x55\xc8\x44");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.x(), flat_dialog.ok.y(), nullptr);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png("flat_dialog-clic-2-button-ok.png");


    RED_CHECK_SIG(drawable.gd, "\xc3\xb6\x9f\xb0\xed\xf9\x3f\x37\xde\xbb\x21\x27\x23\x92\x7a\x96\x10\x7a\x48\x88");


    x = flat_dialog.cancel->x() + flat_dialog.cancel->cx() / 2 ;
    y = flat_dialog.cancel->y() + flat_dialog.cancel->cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png("flat_dialog-clic-3-button-cancel.png");


    RED_CHECK_SIG(drawable.gd, "\x5f\xdf\x5b\x4e\x76\x7b\x5b\x31\x02\x91\x9b\x18\xd6\x9a\x14\xae\xb3\x95\x31\xe5");


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

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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
    // drawable.save_to_png("flat_dialog-challenge-1.png");

    RED_CHECK_SIG(drawable.gd, "\x10\xfb\x06\xd7\x29\xc8\xda\x3f\x01\xe0\xd1\x57\x22\x5d\xf0\x18\x62\x4f\xb6\xf4");

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = nullptr;
    notifier.event = 0;
}
