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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "mod/internal/widget/flat_dialog.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


RED_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "test1",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    // drawable.save_to_png("flat_dialog1.png");

    RED_CHECK_SIG(drawable, "\x5c\x9e\xa5\x68\x89\x6f\x70\x1d\x6f\x37\x07\x25\xaa\x38\x85\xbb\x1b\xa2\xa5\xa9");
}

RED_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 640, 480, parent, notifier, "test2",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog2.png");

    RED_CHECK_SIG(drawable, "\x2b\x31\xcf\x29\xa8\x8e\x64\x50\x94\xd4\xc7\x5c\x69\x3e\x22\x22\x0b\xbb\xe3\x22");
}

RED_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 1280, 1024, parent, notifier, "test3",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog3.png");

    RED_CHECK_SIG(drawable, "\x9a\x7b\xcd\x7a\x44\xb0\xc8\x52\x21\xee\xc1\x40\xc4\x7d\xbe\x47\x47\xda\x4c\x11");
}

RED_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestGraphic drawable(1280, 1024);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(1280, 1024);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 1280, 1024, parent, notifier, "test4",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog4.png");

    RED_CHECK_SIG(drawable, "\x3b\x73\xf0\x21\x40\x52\x19\x61\x31\x97\xcc\x82\x3d\x7a\xe0\x24\xdf\x5c\xb5\xdd");
}

RED_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestGraphic drawable(640, 480);


    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(640, 480);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 640, 480, parent, notifier, "test5",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog5.png");

    RED_CHECK_SIG(drawable, "\xb4\x13\x9b\xc7\x8a\xc8\x19\xab\x95\xa4\xe8\x80\xa0\x30\xab\x33\x32\x94\x4c\x90");
}

RED_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestGraphic drawable(352, 500);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(300, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 350, 500, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog6.png");

    RED_CHECK_SIG(drawable, "\x5c\x88\xa1\x4f\x80\x39\x90\xb2\x75\x0f\x16\x6c\xb2\x02\x28\xcc\xea\x42\x69\x5e");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 300, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    // drawable.save_to_png("flat_dialog7.png");

    RED_CHECK_SIG(drawable, "\xf9\x32\x9f\x92\x53\x55\x0f\x68\x8d\xc9\xff\xdb\xf8\x42\x8d\x73\x55\x7c\xbb\xb1");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      5 + flat_dialog.y(),
                                      30,
                                      10));

    // drawable.save_to_png("flat_dialog8.png");

    RED_CHECK_SIG(drawable, "\x6a\xbe\x33\x53\x74\x19\x62\x01\x94\x1d\xf8\x8d\xf1\xba\xea\x5b\x4f\xef\x0f\xff");
}

RED_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, &notifier, "test6",
                           "line 1<br>"
                           "line 2<br>"
                           "<br>"
                           "line 3, blah blah<br>"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

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

    RED_CHECK_SIG(drawable, "\x6b\x5b\x10\x31\xa7\xc4\xda\x68\x66\x0b\x0e\x5f\x61\xd5\xb4\xc6\x6b\x9f\x5d\x87");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.x(), flat_dialog.ok.y(), nullptr);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.sender = nullptr;
    notifier.event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png("flat_dialog-clic-2-button-ok.png");


    RED_CHECK_SIG(drawable, "\x4d\xcb\x66\x8b\x05\x21\xbc\x35\x39\xe2\x59\x0c\xbc\x04\x98\xdc\xff\x9e\x4b\xc5");


    x = flat_dialog.cancel->x() + flat_dialog.cancel->cx() / 2 ;
    y = flat_dialog.cancel->y() + flat_dialog.cancel->cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
//    RED_CHECK(notifier.sender == 0);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    // drawable.save_to_png("flat_dialog-clic-3-button-cancel.png");


    RED_CHECK_SIG(drawable, "\x7c\x76\x0f\xfb\xab\x1a\xdf\x97\xdc\xc0\xbd\xf7\x34\xcc\xbf\xca\x97\xa3\x02\x58");


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
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, &notifier, "test6",
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
                           "adipiscing et arcu.", extra_button, colors, global_font_deja_vu_14(),
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

    RED_CHECK_SIG(drawable, "\xed\xb3\xbc\x4e\xd6\x1c\xe6\x0d\x43\x61\x8b\xe2\x0b\xae\x66\x4c\xd2\x1d\x4b\x06");

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.sender == &flat_dialog);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);

    notifier.sender = nullptr;
    notifier.event = 0;
}
