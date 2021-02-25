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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"

#include "mod/internal/widget/flat_dialog.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/widget/flat_button.hpp"
#include "keyboard/keymap2.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/dialog/"


RED_AUTO_TEST_CASE(TraceFlatDialog)
{
    TestGraphic drawable(800, 600);

    // FlatDialog is a flat_dialog widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "test1",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_1.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialog2)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 640, 480, parent, notifier, "test2",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_2.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialog3)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position -10,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 1280, 1024, parent, notifier, "test3",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_3.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialog4)
{
    TestGraphic drawable(1280, 1024);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, 1280, 1024, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 1280, 1024, parent, notifier, "test4",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_4.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialog5)
{
    TestGraphic drawable(640, 480);


    // FlatDialog is a flat_dialog widget of size 100x20 at position -20,-7 in it's parent context
    WidgetScreen parent(drawable, 640, 480, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 640, 480, parent, notifier, "test5",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_5.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialog6)
{
    TestGraphic drawable(352, 500);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 300, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 350, 500, parent, notifier, "test6",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at it's current position
    flat_dialog.rdp_input_invalidate(Rect(0 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_6.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 760,-7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 300, 600, parent, notifier, "test6",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      0 + flat_dialog.y(),
                                      flat_dialog.cx(),
                                      flat_dialog.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_7.png");
}

RED_AUTO_TEST_CASE(TraceFlatDialogClip2)
{
    TestGraphic drawable(800, 600);


    // FlatDialog is a flat_dialog widget of size 100x20 at position 10,7 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, notifier, "test6",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    // ask to widget to redraw at position 30,12 and of size 30x10.
    flat_dialog.rdp_input_invalidate(Rect(20 + flat_dialog.x(),
                                      5 + flat_dialog.y(),
                                      30,
                                      10));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_8.png");
}

RED_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});
    NotifyTrace notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, &notifier, "test6",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button, colors, global_font_deja_vu_14());

    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    int x = flat_dialog.ok.x() + flat_dialog.ok.cx() / 2 ;
    int y = flat_dialog.ok.y() + flat_dialog.ok.cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_9.png");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    // flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  flat_dialog.ok.x(), flat_dialog.ok.y(), nullptr);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_10.png");


    x = flat_dialog.cancel->x() + flat_dialog.cancel->cx() / 2 ;
    y = flat_dialog.cancel->y() + flat_dialog.cancel->cy() / 2 ;
    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y, nullptr);
    // flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == 0);


    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_11.png");


    flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y, nullptr);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_CANCEL);

    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ESC);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(EventWidgetChallenge)
{
    TestGraphic drawable(800, 600);


    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{});

    NotifyTrace notifier;
    Theme colors;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    WidgetFlatButton * extra_button = nullptr;
    FlatDialog flat_dialog(drawable, 0, 0, 800, 600, parent, &notifier, "test6",
                           "Lorem ipsum dolor sit amet, consectetur\n"
                           "adipiscing elit. Nam purus lacus, luctus sit\n"
                           "amet suscipit vel, posuere quis turpis. Sed\n"
                           "venenatis rutrum sem ac posuere. Phasellus\n"
                           "feugiat dui eu mauris adipiscing sodales.\n"
                           "Mauris rutrum molestie purus, in tempor lacus\n"
                           "tincidunt et. Sed eu ligula mauris, a rutrum\n"
                           "est. Vestibulum in nunc vel massa condimentum\n"
                           "iaculis nec in arcu. Pellentesque accumsan,\n"
                           "quam sit amet aliquam mattis, odio purus\n"
                           "porttitor tortor, sit amet tincidunt odio\n"
                           "erat ut ligula. Fusce sit amet mauris neque.\n"
                           "Sed orci augue, luctus in ornare sed,\n"
                           "adipiscing et arcu.", extra_button, colors, global_font_deja_vu_14(),
                           "Ok", "Cancel", CHALLENGE_ECHO);

    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == 0);


    flat_dialog.challenge->set_text("challenge_test");

    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_12.png");

    Keymap2 keymap;
    keymap.init_layout(0x040C);
    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    flat_dialog.rdp_input_scancode(0, 0, 0, 0, &keymap);
    RED_CHECK(notifier.last_widget == &flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);

    notifier.last_widget = nullptr;
    notifier.last_event = 0;
}



RED_AUTO_TEST_CASE(TraceFlatDialog_transparent_png_with_theme_color)
{
    TestGraphic drawable(800, 600);
    WidgetScreen parent(drawable,
                        800,
                        600,
                        global_font_deja_vu_14(),
                        nullptr,
                        Theme{ });
    NotifyApi *notifier = nullptr;
    Theme colors;


    colors.global.enable_theme = true;
    colors.global.bgcolor = DARK_BLUE_BIS;
    colors.global.fgcolor = WHITE;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";

    WidgetFlatButton *extra_button = nullptr;
    FlatDialog flat_dialog(drawable,
                           0,
                           0,
                           800,
                           600,
                           parent,
                           notifier,
                           "test1",
                           "line 1\n"
                           "line 2\n"
                           "\n"
                           "line 3, blah blah\n"
                           "line 4",
                           extra_button,
                           colors,
                           global_font_deja_vu_14());

    flat_dialog.rdp_input_invalidate(flat_dialog.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "dialog_13.png");
}
