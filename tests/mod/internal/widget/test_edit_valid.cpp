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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen,
 *              Meng Tan
 */

#define RED_TEST_MODULE TestWidgetEditValid
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "core/font.hpp"
#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "test_only/check_sig.hpp"

#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLACK;
    BGRColor bg_color = WHITE;
    int id = 0;
    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = 2;

    WidgetEditValid wedit(drawable.gd, parent, notifier, "test1", id,
                          fg_color, bg_color, ANTHRACITE, bg_color, font, nullptr, false, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.set_widget_focus(&wedit, Widget::focus_reason_tabkey);
    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid1.png");

    RED_CHECK_SIG(drawable.gd, "\xbd\x7b\x96\xbf\x0f\x7f\xfd\x2b\xab\x0b\x38\x6f\x5a\x0a\x93\x2d\x7d\xc3\x3d\x46");
}


RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = BLACK;
    BGRColor bg_color = WHITE;
    int id = 0;
    int16_t x = 50;
    int16_t y = 100;
    uint16_t cx = 100;
    int xtext = 4;
    int ytext = 1;
    size_t edit_pos = -1;

    WidgetEditValid wedit(drawable.gd, parent, notifier, nullptr, id,
                          fg_color, bg_color, ANTHRACITE, bg_color, font, nullptr, false, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid2.png");

    RED_CHECK_SIG(drawable.gd, "\x60\x94\x19\x4b\xa3\xfe\xdd\x62\x2c\xc3\xc6\x26\xc4\xcd\x5e\x4d\x05\xac\xc0\x14");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid3.png");

    RED_CHECK_SIG(drawable.gd, "\x80\xbb\x8c\x86\xf5\x9e\x67\xc9\x26\xc8\xa2\xfc\x34\x4d\x63\xa4\xdd\x55\x54\xe8");

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid4.png");

    RED_CHECK_SIG(drawable.gd, "\xb8\x5c\x16\x4b\x8e\xa9\x6c\x7b\xe1\xab\x52\x5e\x68\xea\x1d\x9d\x25\x21\xbc\x0d");
    RED_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid5.png");

    RED_CHECK_SIG(drawable.gd, "\xc7\x8f\x6b\xb8\x20\xe0\x0d\x68\x0a\x2b\xb2\x27\xfa\xe6\xcd\xac\x1f\x26\xce\xd4");

    parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, font, nullptr, false, 0);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable.gd, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, font, nullptr, false, 0);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid6.png");

    RED_CHECK_SIG(drawable.gd, "\x7e\xa0\x42\x9e\x72\x4a\xd1\x26\x03\xee\x2b\x64\x01\xb4\xc4\x46\xac\x18\xa1\x21");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid7.png");

    RED_CHECK_SIG(drawable.gd, "\x77\x0d\xb5\x0c\x45\xc9\x2d\xab\x00\x95\x99\xcf\xac\xa2\xee\x07\x03\x3c\xf1\xcc");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid8.png");

    RED_CHECK_SIG(drawable.gd, "\x37\x03\x36\xe6\xd0\x9a\xac\x83\x51\x69\x5c\x71\xf1\xc7\x40\xb7\x4a\x1c\xa9\x6a");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalid9.png");

    RED_CHECK_SIG(drawable.gd, "\x36\x2b\x05\x1f\x52\x86\x11\xb9\x0a\xac\xac\x50\x46\xe5\xa9\xa4\x4b\xdf\x2c\x07");
}


RED_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, font, "edition1", true, 0, 0, 0, false);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable.gd, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, font, "edition2", true, 0, 0, 0, false);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel1.png");

    RED_CHECK_SIG(drawable.gd, "\x1b\x00\x44\x22\x30\x91\xb4\xf6\x2f\xd4\x5a\x08\xfe\x59\xf0\xaf\x8d\xad\xa0\x1c");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel2.png");

    RED_CHECK_SIG(drawable.gd, "\xbe\xaf\xc9\x87\x0d\xd6\x30\x7c\x8a\xe3\xed\x85\xba\xa2\x3d\x48\x46\x1b\x68\x1e");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel3.png");

    RED_CHECK_SIG(drawable.gd, "\xa8\xbd\x98\x2c\xe7\xf9\x76\xeb\xd4\x9c\x73\x95\x19\xd4\x94\x72\x0d\xd0\xdd\x85");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel4.png");

    RED_CHECK_SIG(drawable.gd, "\x36\x2b\x05\x1f\x52\x86\x11\xb9\x0a\xac\xac\x50\x46\xe5\xa9\xa4\x4b\xdf\x2c\x07");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel5.png");

    RED_CHECK_SIG(drawable.gd, "\x50\xba\xcc\xab\x26\x75\x81\x3d\xaa\xe8\x21\xd9\x86\xa6\x38\x32\xa4\x3f\x22\xaa");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel6.png");

    RED_CHECK_SIG(drawable.gd, "\xc4\x4f\x38\x33\x7a\x27\x35\x44\x63\xba\x16\xfd\x35\x1e\x0c\x6f\x54\xc5\xd7\xd8");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabel7.png");

    RED_CHECK_SIG(drawable.gd, "\xbe\xaf\xc9\x87\x0d\xd6\x30\x7c\x8a\xe3\xed\x85\xba\xa2\x3d\x48\x46\x1b\x68\x1e");
}
RED_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable.gd, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, font, "edition1", true, 0, 0, 0, true);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable.gd, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, font, "edition2", true, 0, 0, 0, true);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass1.png");

    RED_CHECK_SIG(drawable.gd, "\xa3\xaf\x62\x5c\x4c\x0d\xe4\x51\x66\x04\x4c\x4d\x32\x07\xe5\x9c\x1e\x11\x64\x13");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass2.png");

    RED_CHECK_SIG(drawable.gd, "\xee\x77\xf2\x26\x74\xea\xa5\x69\xaa\x04\xf0\xc9\x45\x8c\x89\xd0\x04\x17\xf0\x6c");

    Keymap2 keymap;
    const int layout = 0x040C;
    keymap.init_layout(layout);
    bool    ctrl_alt_del;
    uint16_t keyboardFlags = 0 ;
    uint16_t keyCode = 0;
    keyboardFlags = 0 ;
    keyCode = 16 ; // key is 'a'

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass3.png");

    RED_CHECK_SIG(drawable.gd, "\xc6\xc0\x75\x9a\x5b\x71\x44\xdc\x45\x8a\x06\x1a\x79\x0c\x53\x5b\xe5\xcb\x50\xe7");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass4.png");

    RED_CHECK_SIG(drawable.gd, "\x7f\xdd\xc5\x1d\x9b\xb2\xf6\x1b\x36\xa8\xdc\xda\x54\xbe\x9b\x1b\xf4\x12\x19\x03");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass5.png");

    RED_CHECK_SIG(drawable.gd, "\x17\x49\xc8\x96\xa4\x29\x4f\x2e\xf5\x25\x91\x55\xc7\x81\xd9\x07\xe2\x33\x1d\xe6");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass6.png");

    RED_CHECK_SIG(drawable.gd, "\xb5\x4c\x6c\xaa\x95\x3c\xa6\x61\x8d\x2c\x89\xb8\xd5\x32\x00\xd5\x2e\x62\x30\xd8");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidlabelpass7.png");

    RED_CHECK_SIG(drawable.gd, "\xee\x77\xf2\x26\x74\xea\xa5\x69\xaa\x04\xf0\xc9\x45\x8c\x89\xd0\x04\x17\xf0\x6c");

}

RED_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestDraw drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestDraw& drawable)
        : Widget(drawable.gd, *this, nullptr)
        {}

        void rdp_input_invalidate(const Rect) override
        {}

        void notify(Widget* sender, NotifyApi::notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } widget_for_receive_event(drawable);

    struct Notify : public NotifyApi {
        Widget* sender = nullptr;
        notify_event_t event = 0;
        Notify() = default;
        void notify(Widget* sender, notify_event_t event) override
        {
            this->sender = sender;
            this->event = event;
        }
    } notifier;

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(drawable.gd, font, nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEditValid wedit(drawable.gd, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, WHITE, font, nullptr, false);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit, Widget::focus_reason_tabkey);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush1.png");

    RED_CHECK_SIG(drawable.gd, "\xe3\xc7\xaa\x49\xda\x08\x4b\xaa\x47\x94\x24\xf9\x84\x23\xe7\x1e\x9e\x19\xbe\xfe");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush2.png");

    RED_CHECK_SIG(drawable.gd, "\xaa\x20\xdf\x8a\x13\x2d\x9f\xf1\xf6\x3b\x29\x9c\x7b\xd7\xaf\x19\x1a\x5e\x6c\x4c");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush3.png");

    RED_CHECK_SIG(drawable.gd, "\xe3\xc7\xaa\x49\xda\x08\x4b\xaa\x47\x94\x24\xf9\x84\x23\xe7\x1e\x9e\x19\xbe\xfe");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush4.png");

    RED_CHECK_SIG(drawable.gd, "\xaa\x20\xdf\x8a\x13\x2d\x9f\xf1\xf6\x3b\x29\x9c\x7b\xd7\xaf\x19\x1a\x5e\x6c\x4c");


    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush5.png");

    RED_CHECK_SIG(drawable.gd, "\xe3\xc7\xaa\x49\xda\x08\x4b\xaa\x47\x94\x24\xf9\x84\x23\xe7\x1e\x9e\x19\xbe\xfe");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush6.png");

    RED_CHECK_SIG(drawable.gd, "\xaa\x20\xdf\x8a\x13\x2d\x9f\xf1\xf6\x3b\x29\x9c\x7b\xd7\xaf\x19\x1a\x5e\x6c\x4c");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png(OUTPUT_FILE_PATH "editvalidpush7.png");

    RED_CHECK_SIG(drawable.gd, "\xaa\x20\xdf\x8a\x13\x2d\x9f\xf1\xf6\x3b\x29\x9c\x7b\xd7\xaf\x19\x1a\x5e\x6c\x4c");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
