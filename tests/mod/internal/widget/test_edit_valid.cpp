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


#include "core/font.hpp"
#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/mod/fake_draw.hpp"

RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalid1.png");

    RED_CHECK_SIG(drawable.gd, "\xa8\xb3\x98\x7d\x2c\xd6\x4f\xe4\x25\xbe\x0a\x99\x9e\x3a\x40\x66\x95\xf0\x56\x9a");
}


RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalid2.png");

    RED_CHECK_SIG(drawable.gd, "\xfc\xcd\x9f\xa7\x1e\x3c\x23\xb6\xc5\x4c\x2b\x52\x01\xfc\x15\x02\xc3\x0a\x1e\xe1");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);

    // ask to widget to redraw at it's current position
    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("editvalid3.png");

    RED_CHECK_SIG(drawable.gd, "\x33\x1c\xa5\x7e\x19\xfc\xf1\x2b\x35\xa1\x55\x8e\xda\x4e\xef\xb5\x3b\x67\x53\x73");

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("editvalid4.png");

    RED_CHECK_SIG(drawable.gd, "\xd5\xee\xc3\x41\xe5\x77\x0a\x69\xa0\x76\x39\xa2\x00\x83\x16\x31\xcf\x60\xe3\xa0");
    RED_CHECK_EQUAL(std::string("Ylajali"), std::string(wedit.get_text()));


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid5.png");

    RED_CHECK_SIG(drawable.gd, "\x5d\xd1\x06\x0f\x6c\x8f\xfa\x03\x34\x5a\x5b\x64\x88\x80\x19\x8d\xd2\xfa\x33\xbd");

    parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalid6.png");

    RED_CHECK_SIG(drawable.gd, "\xe8\x69\x9f\x9f\x8b\x5d\x40\xec\x07\x87\xe2\xe1\x4e\x97\x18\xb8\xd5\x01\x02\xff");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid7.png");

    RED_CHECK_SIG(drawable.gd, "\x8d\x82\x02\x4d\xfe\x8c\x52\xae\x51\x27\xc7\x95\xc7\x95\x71\x97\x3f\x24\xbb\xc2");

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

    // drawable.save_to_png("editvalid8.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\x3d\x46\xb2\x0f\x6f\x61\x78\x72\xc5\x59\x2d\xa7\x60\xa9\x11\x86\x93\x45\x25");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid9.png");

    RED_CHECK_SIG(drawable.gd, "\xa6\x53\x5c\x99\x64\xc4\x6f\x68\x2b\x11\xd5\x14\xaa\x39\xa1\x21\x81\xc5\xbd\x53");
}


RED_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalidlabel1.png");

    RED_CHECK_SIG(drawable.gd, "\x92\xef\xef\x38\x6b\x1c\xb9\xb1\xa4\xd4\x09\xdb\x35\xa1\x83\xd1\xcf\xf6\xf8\xe0");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel2.png");

    RED_CHECK_SIG(drawable.gd, "\x93\x5d\x0d\x6f\x12\xfb\x12\x02\xb1\x96\x4e\x3c\x94\xf3\x77\x12\x11\x02\x43\xc1");

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

    // drawable.save_to_png("editvalidlabel3.png");

    RED_CHECK_SIG(drawable.gd, "\x4d\xdc\x42\xde\xad\x46\x07\xf3\xf2\x7b\xf7\x83\x5f\xce\x57\x51\xa7\x93\x7f\xd0");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel4.png");

    RED_CHECK_SIG(drawable.gd, "\xa6\x53\x5c\x99\x64\xc4\x6f\x68\x2b\x11\xd5\x14\xaa\x39\xa1\x21\x81\xc5\xbd\x53");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel5.png");

    RED_CHECK_SIG(drawable.gd, "\xd0\x14\x71\x7b\x62\x5f\xe7\x84\xda\xeb\x7f\x09\x0c\x5a\xde\x9e\x44\xdb\x81\x6a");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabel6.png");

    RED_CHECK_SIG(drawable.gd, "\xb2\xbd\x05\xe0\x43\x51\x6d\x31\x56\x46\xfa\xab\x17\xa6\x5f\x4d\x3e\x20\xa7\x0a");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabel7.png");

    RED_CHECK_SIG(drawable.gd, "\x93\x5d\x0d\x6f\x12\xfb\x12\x02\xb1\x96\x4e\x3c\x94\xf3\x77\x12\x11\x02\x43\xc1");
}
RED_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestDraw drawable(800, 600);

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalidlabelpass1.png");

    RED_CHECK_SIG(drawable.gd, "\xcb\x4e\x98\x33\xeb\x7b\x94\x5f\x6d\x1c\x96\xef\xeb\x5e\xdb\x24\xc7\xfc\xa0\xd9");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass2.png");

    RED_CHECK_SIG(drawable.gd, "\x42\x4b\x4f\x88\x21\xdd\x84\x39\x94\x64\xfc\xb0\xde\x01\x54\x82\x4e\x2e\xd7\xf5");

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

    // drawable.save_to_png("editvalidlabelpass3.png");

    RED_CHECK_SIG(drawable.gd, "\xec\x91\xf3\xf7\x80\xd2\x50\x2d\xe7\x89\x50\x01\x51\xfd\x3f\x0e\xeb\x3e\x87\xc1");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass4.png");

    RED_CHECK_SIG(drawable.gd, "\xdf\x0a\xcc\xee\x04\x4d\x20\xcd\xd5\x9c\x7a\xfa\x89\x00\xfe\x14\xc6\xed\x8e\x08");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass5.png");

    RED_CHECK_SIG(drawable.gd, "\x05\x1a\xd8\x5a\x5e\x97\x35\x55\x04\x8d\x25\x3a\x4f\x71\x31\x4e\xfe\x63\x4b\x1f");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabelpass6.png");

    RED_CHECK_SIG(drawable.gd, "\xbf\x79\x9c\x4c\xa4\x1c\x2b\xcf\x01\x22\x70\xdc\x35\x05\x9d\x3a\x1a\xe5\x6b\xa2");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabelpass7.png");

    RED_CHECK_SIG(drawable.gd, "\x42\x4b\x4f\x88\x21\xdd\x84\x39\x94\x64\xfc\xb0\xde\x01\x54\x82\x4e\x2e\xd7\xf5");

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

    Font font(FIXTURES_PATH "/Lato-Light_16.rbf");

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

    // drawable.save_to_png("editvalidpush1.png");

    RED_CHECK_SIG(drawable.gd, "\x13\x82\xfe\x60\x60\xfe\x86\xe4\x3e\x1c\x48\xfc\x65\xaa\x6b\x87\x55\x3a\x35\xeb");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush2.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\xeb\xb4\xde\x6c\x9a\xa9\xe5\x46\xc3\x18\x70\x21\x5b\x15\xb5\x48\x82\x69\x1a");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush3.png");

    RED_CHECK_SIG(drawable.gd, "\x13\x82\xfe\x60\x60\xfe\x86\xe4\x3e\x1c\x48\xfc\x65\xaa\x6b\x87\x55\x3a\x35\xeb");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush4.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\xeb\xb4\xde\x6c\x9a\xa9\xe5\x46\xc3\x18\x70\x21\x5b\x15\xb5\x48\x82\x69\x1a");


    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush5.png");

    RED_CHECK_SIG(drawable.gd, "\x13\x82\xfe\x60\x60\xfe\x86\xe4\x3e\x1c\x48\xfc\x65\xaa\x6b\x87\x55\x3a\x35\xeb");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush6.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\xeb\xb4\xde\x6c\x9a\xa9\xe5\x46\xc3\x18\x70\x21\x5b\x15\xb5\x48\x82\x69\x1a");

    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;


    Keymap2 keymap;
    keymap.init_layout(0x040C);

    keymap.push_kevent(Keymap2::KEVENT_ENTER);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush7.png");

    RED_CHECK_SIG(drawable.gd, "\x4b\xeb\xb4\xde\x6c\x9a\xa9\xe5\x46\xc3\x18\x70\x21\x5b\x15\xb5\x48\x82\x69\x1a");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
