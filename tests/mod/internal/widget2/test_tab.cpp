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
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT
#include "log.hpp"

#include "internal/widget2/edit.hpp"
#include "internal/widget2/flat_button.hpp"
#include "internal/widget2/tab.hpp"
#include "internal/widget2/screen.hpp"
#include "internal/widget2/composite.hpp"
#include "png.hpp"
#include "ssl_calls.hpp"
#include "RDP/RDPDrawable.hpp"
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


    NotifyApi * notifier    = NULL;
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
    if (!check_sig( drawable.gd.impl(), message
                  , "\xa8\xe3\xf5\x36\x90\xc8\x52\x3b\xcb\xc6"
                    "\x06\xc3\x10\xaf\x67\x2b\x12\x65\x3a\x13")) {
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
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x5a\xb8\x6d\x93\x05\x89\xf7\x22\x59\x97"
                    "\x34\x26\xa2\xf3\xf2\xf1\x88\x0f\x66\xe0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_1.png");


    mouse_x = x + 144;
    mouse_y = y;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xa8\xe3\xf5\x36\x90\xc8\x52\x3b\xcb\xc6"
                    "\x06\xc3\x10\xaf\x67\x2b\x12\x65\x3a\x13")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_2.png");


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x5a\xb8\x6d\x93\x05\x89\xf7\x22\x59\x97"
                    "\x34\x26\xa2\xf3\xf2\xf1\x88\x0f\x66\xe0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_3.png");


    keymap.push_kevent(Keymap2::KEVENT_LEFT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x22\xd3\x7d\x83\xe2\xa4\x99\x5d\x77\xb2"
                    "\x0d\x25\xd3\x12\x4a\x6a\x1d\x0f\xf9\xbf")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_4.png");


    keymap.push_kevent(Keymap2::KEVENT_RIGHT_ARROW);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x5a\xb8\x6d\x93\x05\x89\xf7\x22\x59\x97"
                    "\x34\x26\xa2\xf3\xf2\xf1\x88\x0f\x66\xe0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_5.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x95\xca\xee\x66\x92\x86\x65\x02\x2a\x6a"
                    "\x43\xeb\x56\x9d\x35\xad\x08\x8a\x68\x55")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_6.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xea\x27\x39\x94\xf0\x64\x35\x4d\x4c\x4b"
                    "\x39\x76\x7d\x05\x9d\x99\x7e\x8b\xa9\x9e")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_7.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xee\x91\x40\xc9\x43\x40\xc3\x59\x40\xa1"
                    "\x42\xd6\xbc\x50\xa6\xa2\x03\x33\x37\xed")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_8.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x01\x37\xd7\x09\xd5\x75\x1c\x05\xf0\x55"
                    "\x1b\xd4\x95\x76\x0c\x70\x80\x48\xa3\xa5")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_9.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x5a\xb8\x6d\x93\x05\x89\xf7\x22\x59\x97"
                    "\x34\x26\xa2\xf3\xf2\xf1\x88\x0f\x66\xe0")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_10.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x01\x37\xd7\x09\xd5\x75\x1c\x05\xf0\x55"
                    "\x1b\xd4\x95\x76\x0c\x70\x80\x48\xa3\xa5")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_11.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xee\x91\x40\xc9\x43\x40\xc3\x59\x40\xa1"
                    "\x42\xd6\xbc\x50\xa6\xa2\x03\x33\x37\xed")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_12.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xea\x27\x39\x94\xf0\x64\x35\x4d\x4c\x4b"
                    "\x39\x76\x7d\x05\x9d\x99\x7e\x8b\xa9\x9e")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_13.png");


    mouse_x = x + 144;
    mouse_y = y;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xa8\xe3\xf5\x36\x90\xc8\x52\x3b\xcb\xc6"
                    "\x06\xc3\x10\xaf\x67\x2b\x12\x65\x3a\x13")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_14.png");


    mouse_x = x + 20;
    mouse_y = y - 20;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x22\x4d\x6d\xae\x35\xd7\x29\x32\xd0\x73"
                    "\xe0\x8a\xd8\xcf\x3e\x6f\x23\xa2\x18\xe6")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_15.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xa8\xe3\xf5\x36\x90\xc8\x52\x3b\xcb\xc6"
                    "\x06\xc3\x10\xaf\x67\x2b\x12\x65\x3a\x13")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_16.png");


    mouse_x = x + 20;
    mouse_y = y + cy + 15 + 5;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x64\xcf\xdc\xcc\x87\x1c\x05\xde\xdf\xf2"
                    "\x16\x7d\x0f\x7f\x21\xdc\xb8\x46\x93\x3d")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_17.png");


    mouse_x = x + 146;
    mouse_y = y + 22;
    parent.rdp_input_mouse( MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN
                          , mouse_x + 2
                          , mouse_y
                          , NULL);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xed\xfd\xbe\x01\x7e\xe5\xc7\x08\x5e\x6d"
                    "\x70\x0a\x3a\x8f\x2e\xfb\x43\x5e\xaf\xbc")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    drawable.draw(RDPOpaqueRect(Rect(mouse_x + 2, mouse_y, 1, 1), GREEN), parent.rect);

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_18.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xc9\x0b\xef\x6a\x33\x3c\xf8\xdf\xc4\x80"
                    "\x72\x4c\xba\x04\x78\xeb\x33\x11\x58\x5e")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_19.png");


    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x8a\x0b\x7b\xc4\x0f\x95\xb4\x7c\x96\xe4"
                    "\x18\x64\x6e\x14\x93\x2c\xb2\x45\x67\x1b")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_20.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xc9\x0b\xef\x6a\x33\x3c\xf8\xdf\xc4\x80"
                    "\x72\x4c\xba\x04\x78\xeb\x33\x11\x58\x5e")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_21.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\xed\xfd\xbe\x01\x7e\xe5\xc7\x08\x5e\x6d"
                    "\x70\x0a\x3a\x8f\x2e\xfb\x43\x5e\xaf\xbc")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_22.png");


    keymap.push_kevent(Keymap2::KEVENT_BACKTAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    if (!check_sig( drawable.gd.impl(), message
                  , "\x8a\x0b\x7b\xc4\x0f\x95\xb4\x7c\x96\xe4"
                    "\x18\x64\x6e\x14\x93\x2c\xb2\x45\x67\x1b")) {
        BOOST_CHECK_MESSAGE(false, message);
    }

    //drawable.save_to_png(OUTPUT_FILE_PATH "tab_23.png");
}
