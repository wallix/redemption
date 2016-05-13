/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWidgetTab
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "mod/internal/widget2/edit.hpp"
#include "mod/internal/widget2/flat_button.hpp"
#include "mod/internal/widget2/tab.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "check_sig.hpp"
#include "fake_draw.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif

#undef OUTPUT_FILE_PATH
#define OUTPUT_FILE_PATH "./"

BOOST_AUTO_TEST_CASE(TraceWidgetTab)
{
    TestDraw drawable(800, 600);

    // WidgetGroupBox is a widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600);


    NotifyApi * notifier    = nullptr;
    int         fg_color    = RED;
    int         bg_color    = YELLOW;
    int         focuscolor  = LIGHT_YELLOW;
    int         group_id    = 0;
    int16_t     x           = 200;
    int16_t     y           = 100;
    uint16_t    cx          = 300;
    uint16_t    cy          = 200;


    WidgetEdit edit1(drawable, x, y - 30, 80, parent, &parent, "", 0, fg_color, bg_color, focuscolor);

    parent.add_widget(&edit1);


    WidgetTabDPDefault drawing_policy(drawable);

    TODO("I believe users of this widget may wish to control text position and behavior inside rectangle"
         "ie: text may be centered, aligned left, aligned right, or even upside down, etc"
         "these possibilities (and others) are supported in RDPGlyphIndex")
    WidgetTab wtab( drawable, drawing_policy, x, y, cx, cy, parent, &parent, group_id
                  , fg_color, bg_color);

    parent.add_widget(&wtab);


    bool auto_resize = true;
    int  xtext       = 4;
    int  ytext       = 1;

    WidgetFlatButton button1( drawable, x, y + cy + 15, parent, &parent
                            , "Submit", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    parent.add_widget(&button1);


    size_t tab_0_index = static_cast<size_t>(-1);
    size_t tab_1_index = static_cast<size_t>(-1);
    size_t tab_2_index = static_cast<size_t>(-1);

    BOOST_CHECK((tab_0_index = wtab.add_item("First tab")) == 0);
    BOOST_CHECK((tab_1_index = wtab.add_item("Second tab")) == 1);
    BOOST_CHECK((tab_2_index = wtab.add_item("Third tab")) == 2);


    WidgetFlatButton wbutton_on_first_tab( drawable, 10, 20, wtab.get_item(tab_0_index), notifier
                            , "Button on First tab", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    wtab.add_widget(tab_0_index, &wbutton_on_first_tab);

    WidgetFlatButton wbutton_1_on_first_tab( drawable, 10, 45, wtab.get_item(tab_0_index), notifier
                            , "Button 1 on First tab", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    wtab.add_widget(tab_0_index, &wbutton_1_on_first_tab);

    WidgetFlatButton wbutton_on_second_tab( drawable, 10, 20, wtab.get_item(tab_1_index), notifier
                            , "Button on second tab", auto_resize, group_id, fg_color, bg_color
                            , focuscolor, xtext, ytext);
    wtab.add_widget(tab_1_index, &wbutton_on_second_tab);


    wtab.set_current_item(tab_1_index);

    parent.set_widget_focus(&wtab, Widget2::focus_reason_tabkey);

    wtab.child_has_focus = false;

    parent.refresh(parent.rect);

    char message[1024];
    if (!check_sig( drawable.gd.impl(), message,
        "\x86\x38\xfb\x75\xbc\xd0\xdd\xa1\x77\x2e\x95\xa6\x1b\x9e\x97\x76\x66\xab\xc2\x65"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_0.png");


    int16_t mouse_x;
    int16_t mouse_y;


    // mouse_x = x + 2;
    // mouse_y = y;
    mouse_x = x + 62;
    mouse_y = y + 22;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\x88\x12\xe6\x6b\xd3\x8a\x60\x75\x6f\x93\xaf\xa1\xdf\x10\x9d\xce\xaf\x39\xc7\xe9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_1.png");


    mouse_x = x + 144;
    mouse_y = y;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\x86\x38\xfb\x75\xbc\xd0\xdd\xa1\x77\x2e\x95\xa6\x1b\x9e\x97\x76\x66\xab\xc2\x65"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_2.png");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x88\x12\xe6\x6b\xd3\x8a\x60\x75\x6f\x93\xaf\xa1\xdf\x10\x9d\xce\xaf\x39\xc7\xe9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_3.png");


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x24\x64\x90\xb5\x4a\x35\x94\xf2\x2c\x92\x10\xf7\x8f\xea\xf8\xf4\x41\x94\x5b\xbb"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_4.png");


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x88\x12\xe6\x6b\xd3\x8a\x60\x75\x6f\x93\xaf\xa1\xdf\x10\x9d\xce\xaf\x39\xc7\xe9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_5.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\xd0\x15\x2d\x69\x77\xa7\xf6\x14\x12\x92\xc5\xb5\x1a\x41\xb3\x4b\x4a\x83\xd3\xa9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_6.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x00\x07\xc7\x57\xd3\xc3\xfd\x4c\xd8\x9b\xe6\x45\x22\x42\x9e\x18\x37\x68\xae\xdb"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_7.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x81\x42\xc8\xfc\x91\xe0\xa0\xac\xbb\xbe\x40\x65\xd4\x80\xf1\x14\xa7\xbb\x01\xc9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_8.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x7a\xfa\x1e\xc3\xa8\x96\xaa\x54\xd9\x6d\xed\xd6\x55\xb9\xed\x64\xcf\xde\x9d\x83"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_9.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x88\x12\xe6\x6b\xd3\x8a\x60\x75\x6f\x93\xaf\xa1\xdf\x10\x9d\xce\xaf\x39\xc7\xe9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_10.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x7a\xfa\x1e\xc3\xa8\x96\xaa\x54\xd9\x6d\xed\xd6\x55\xb9\xed\x64\xcf\xde\x9d\x83"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_11.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x81\x42\xc8\xfc\x91\xe0\xa0\xac\xbb\xbe\x40\x65\xd4\x80\xf1\x14\xa7\xbb\x01\xc9"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_12.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x00\x07\xc7\x57\xd3\xc3\xfd\x4c\xd8\x9b\xe6\x45\x22\x42\x9e\x18\x37\x68\xae\xdb"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_13.png");


    mouse_x = x + 144;
    mouse_y = y;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\x86\x38\xfb\x75\xbc\xd0\xdd\xa1\x77\x2e\x95\xa6\x1b\x9e\x97\x76\x66\xab\xc2\x65"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_14.png");


    mouse_x = x + 20;
    mouse_y = y - 20;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\xd1\x12\xb0\xcb\xfd\x3f\xa0\x94\x96\xb8\xdb\xe0\xdd\xe0\x9c\xf5\x3d\x09\x71\xd4"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_15.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x86\x38\xfb\x75\xbc\xd0\xdd\xa1\x77\x2e\x95\xa6\x1b\x9e\x97\x76\x66\xab\xc2\x65"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_16.png");


    mouse_x = x + 20;
    mouse_y = y + cy + 15 + 5;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\xe0\xce\xb6\x80\xa1\xd1\xce\xdb\x8c\xe9\x0c\xf7\x33\xf0\xd3\xaf\x30\x32\xc2\xd6"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_17.png");


    mouse_x = x + 146;
    mouse_y = y + 22;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , nullptr);

    if (!check_sig( drawable.gd.impl(), message,
        "\x34\x76\x85\xdf\xb5\xa8\x96\x87\x04\x64\xa3\xc6\x53\x82\xe8\x58\xe1\x33\xee\xf7"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_18.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x08\x47\x84\x17\x8b\xbc\xd6\xe6\x38\xa8\x19\xf7\x30\xf7\x09\xcc\xa7\x7d\x82\x41"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_19.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\xe0\xce\xb6\x80\xa1\xd1\xce\xdb\x8c\xe9\x0c\xf7\x33\xf0\xd3\xaf\x30\x32\xc2\xd6"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_20.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x08\x47\x84\x17\x8b\xbc\xd6\xe6\x38\xa8\x19\xf7\x30\xf7\x09\xcc\xa7\x7d\x82\x41"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_21.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x34\x76\x85\xdf\xb5\xa8\x96\x87\x04\x64\xa3\xc6\x53\x82\xe8\x58\xe1\x33\xee\xf7"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_22.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message,
        "\x83\xe6\x4e\x57\xd9\x5e\x20\x63\x24\x29\x2b\x35\x7e\x0f\x7f\xb5\x75\xf1\x92\x4e"
    )) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_23.png");
}
