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
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetEditValid wedit(drawable, parent, notifier, "test1", id,
                          fg_color, bg_color, ANTHRACITE, bg_color, global_font_deja_vu_14(), nullptr, false, edit_pos, xtext, ytext);
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

    RED_CHECK_SIG(drawable, "\xd5\x9f\x2c\x4a\x66\x0d\x2e\x10\x98\x57\x54\x71\x15\x5d\xec\x3c\x2b\x44\x17\x42");
}


RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget at position 0,0 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
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

    WidgetEditValid wedit(drawable, parent, notifier, nullptr, id,
                          fg_color, bg_color, ANTHRACITE, bg_color, global_font_deja_vu_14(), nullptr, false, edit_pos, xtext, ytext);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid2.png");

    RED_CHECK_SIG(drawable, "\xe1\x81\x3e\xac\xff\x27\xb1\xb5\xd7\xc6\xc0\xf1\x87\x89\xd1\x73\xfa\x7d\x88\x30");

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

    RED_CHECK_SIG(drawable, "\xc0\x96\xb0\x4b\xe1\x9c\x7b\x45\x56\xd5\xbb\x92\xfb\x41\xa2\x0c\x6f\x1a\x09\x16");

    wedit.set_text("Ylajali");

    wedit.rdp_input_invalidate(Rect(0 + wedit.x(),
                                    0 + wedit.y(),
                                    wedit.cx(),
                                    wedit.cy()));

    // drawable.save_to_png("editvalid4.png");

    RED_CHECK_SIG(drawable, "\x75\x64\xef\x93\xbf\x0c\xd7\x36\x48\x0b\xcb\x0e\x15\x92\xc1\x3b\x33\x81\x20\x2f");
    RED_CHECK_EQUAL("Ylajali", wedit.get_text());


    wedit.set_xy(192,242);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid5.png");

    RED_CHECK_SIG(drawable, "\x7f\x2b\xe5\x47\x36\x1a\x3b\x9e\x64\x47\xcf\x88\x81\xc6\xcf\x4c\xef\x6e\x38\x3e");

    parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), nullptr, false, 0);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), nullptr, false, 0);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid6.png");

    RED_CHECK_SIG(drawable, "\x5a\xde\x02\xf9\x84\x2e\xb2\xc4\xeb\xdb\x2e\x9c\x8c\x2b\xe6\x72\x45\x91\x7b\x20");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 3, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid7.png");

    RED_CHECK_SIG(drawable, "\x66\x8f\x37\x9d\xbe\x49\xa6\x21\x52\xe9\x56\xd8\x58\x5d\x0a\x0c\xc1\xe7\xc4\x10");

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

    RED_CHECK_SIG(drawable, "\xce\xa7\xaa\x8b\xa9\x55\x5b\x12\xd9\x98\xfd\x5e\x19\xf1\x66\x24\x3a\x88\x62\xb7");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalid9.png");

    RED_CHECK_SIG(drawable, "\x4c\xd2\x07\x7c\xd5\x5d\x0b\x63\x72\x64\x3f\x94\xb7\x73\xa6\xcb\x7c\x9d\x72\x6a");
}


RED_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), "edition1", true, 0, 0, 0, false);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), "edition2", true, 0, 0, 0, false);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel1.png");

    RED_CHECK_SIG(drawable, "\x97\x1f\xaa\x9a\xf4\x9a\xe7\x33\xdd\xc5\x90\xe9\xb8\x74\xb1\x7f\x19\x84\xe1\xe5");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel2.png");

    RED_CHECK_SIG(drawable, "\xd6\x8f\xf5\xed\x62\xe5\xbc\x15\xff\x02\xe6\xe4\x38\xcf\xc9\x5c\x27\xf2\x2a\x0a");

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

    RED_CHECK_SIG(drawable, "\x4d\x04\x2a\xa3\x25\xa9\xac\x0f\x05\xf7\xd7\x66\xd6\xfa\xdd\x48\xcb\x5c\x7d\x8d");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel4.png");

    RED_CHECK_SIG(drawable, "\x4c\xd2\x07\x7c\xd5\x5d\x0b\x63\x72\x64\x3f\x94\xb7\x73\xa6\xcb\x7c\x9d\x72\x6a");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabel5.png");

    RED_CHECK_SIG(drawable, "\x49\xfd\x83\xf6\x69\xf6\xfa\x7e\x3b\xa1\x66\xb8\xb9\xa6\xd5\xea\x22\x9a\x2a\xdf");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabel6.png");

    RED_CHECK_SIG(drawable, "\x75\x82\x55\x65\x23\xf5\x35\x41\x95\x66\x64\x84\x5b\xa9\x42\x0c\xb5\x51\xf0\x4d");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabel7.png");

    RED_CHECK_SIG(drawable, "\xd6\x8f\xf5\xed\x62\xe5\xbc\x15\xff\x02\xe6\xe4\x38\xcf\xc9\x5c\x27\xf2\x2a\x0a");
}
RED_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestGraphic drawable(800, 600);


    // WidgetEdit is a edit widget of size 100x20 at position 770,500 in it's parent context
    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    NotifyApi * notifier = nullptr;
    BGRColor fg_color = RED;
    BGRColor bg_color = YELLOW;
    BGRColor bbg_color = GREEN;
    int id = 0;

    parent.tab_flag = Widget::NORMAL_TAB;

    WidgetEditValid wedit1(drawable, parent, notifier, "dLorz", id, fg_color, bg_color, bbg_color, bg_color, global_font_deja_vu_14(), "edition1", true, 0, 0, 0, true);
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(150, dim.h);
    wedit1.set_xy(54, 105);

    WidgetEditValid wedit2(drawable, parent, notifier, "", id, WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(), "edition2", true, 0, 0, 0, true);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    parent.add_widget(&wedit1);
    parent.add_widget(&wedit2);
    // ask to widget to redraw at it's current position
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass1.png");

    RED_CHECK_SIG(drawable, "\xd0\x8f\xaa\x93\x1e\x96\x5d\x2a\x1d\x6b\x96\x4d\xfa\x8e\x6e\x21\x9b\x13\x31\x5c");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);
    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit1.x() + 50, wedit1.y() + 2, nullptr);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass2.png");

    RED_CHECK_SIG(drawable, "\xdb\x3d\xb2\x1e\xa7\x7a\xb7\x03\xec\x28\x55\x40\x34\x54\xf0\x99\x52\x0d\xe2\x87");

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

    RED_CHECK_SIG(drawable, "\x93\x6c\x28\xba\xc2\xb4\xac\xb4\x4f\x58\xd0\x0a\x81\xf4\x5c\x62\x53\xe0\x92\x1b");

    keymap.event(keyboardFlags, keyCode, ctrl_alt_del);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass4.png");

    RED_CHECK_SIG(drawable, "\x56\x58\x02\xa0\x5c\x1b\x23\x51\xef\xeb\xb3\x22\xc2\x65\x72\xaa\x7f\xa5\xf5\x17");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidlabelpass5.png");

    RED_CHECK_SIG(drawable, "\x34\x6d\xf3\x43\xc4\xac\xd7\xec\x81\xed\x2d\xa7\xbb\xa4\xd9\x46\xd2\xfe\x38\x1d");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    keymap.push_kevent(Keymap2::KEVENT_BACKSPACE);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabelpass6.png");

    RED_CHECK_SIG(drawable, "\x07\x71\x86\x59\x54\x34\xd2\x39\xcb\x83\x4b\xe4\xd9\x0f\x2e\x6f\x36\xfe\xf6\x18");

    keymap.push_kevent(Keymap2::KEVENT_TAB);
    parent.rdp_input_scancode(0, 0, 0, 0, &keymap);

    // drawable.save_to_png("editvalidlabelpass7.png");

    RED_CHECK_SIG(drawable, "\xdb\x3d\xb2\x1e\xa7\x7a\xb7\x03\xec\x28\x55\x40\x34\x54\xf0\x99\x52\x0d\xe2\x87");

}

RED_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestGraphic drawable(800, 600);

    struct WidgetReceiveEvent : public Widget {
        Widget* sender = nullptr;
        NotifyApi::notify_event_t event = 0;

        WidgetReceiveEvent(TestGraphic& drawable)
        : Widget(drawable, *this, nullptr)
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


    WidgetScreen parent(drawable, global_font_deja_vu_14(), nullptr, Theme{});
    parent.set_wh(800, 600);

    int16_t x = 0;
    int16_t y = 0;
    uint16_t cx = 100;

    WidgetEditValid wedit(drawable, parent, &notifier, "abcdef", 0, BLACK, WHITE, DARK_BLUE, WHITE, global_font_deja_vu_14(), nullptr, false);
    Dimension dim = wedit.get_optimal_dim();
    wedit.set_wh(cx, dim.h);
    wedit.set_xy(x, y);

    parent.add_widget(&wedit);
    parent.set_widget_focus(&wedit, Widget::focus_reason_tabkey);

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush1.png");

    RED_CHECK_SIG(drawable, "\x43\x3a\x16\xce\x44\xab\xea\x4f\x73\x87\xc3\x13\x7b\x5f\xcf\xf9\x79\x9e\xad\xb4");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush2.png");

    RED_CHECK_SIG(drawable, "\x2a\x41\x84\x30\x51\xab\xcf\x48\x3e\x3f\x89\xa3\x92\x57\x32\x78\xe2\x5d\x01\xa3");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush3.png");

    RED_CHECK_SIG(drawable, "\x43\x3a\x16\xce\x44\xab\xea\x4f\x73\x87\xc3\x13\x7b\x5f\xcf\xf9\x79\x9e\xad\xb4");


    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          wedit.x() + 2, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush4.png");

    RED_CHECK_SIG(drawable, "\x2a\x41\x84\x30\x51\xab\xcf\x48\x3e\x3f\x89\xa3\x92\x57\x32\x78\xe2\x5d\x01\xa3");


    RED_CHECK(notifier.sender == nullptr);
    RED_CHECK(notifier.event == 0);
    notifier.event = 0;
    notifier.sender = nullptr;

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN,
                          wedit.right() - 5, wedit.y() + 2, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush5.png");

    RED_CHECK_SIG(drawable, "\x43\x3a\x16\xce\x44\xab\xea\x4f\x73\x87\xc3\x13\x7b\x5f\xcf\xf9\x79\x9e\xad\xb4");

    parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                          0, 0, nullptr);
    parent.rdp_input_invalidate(parent.get_rect());

    // drawable.save_to_png("editvalidpush6.png");

    RED_CHECK_SIG(drawable, "\x2a\x41\x84\x30\x51\xab\xcf\x48\x3e\x3f\x89\xa3\x92\x57\x32\x78\xe2\x5d\x01\xa3");

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

    RED_CHECK_SIG(drawable, "\x2a\x41\x84\x30\x51\xab\xcf\x48\x3e\x3f\x89\xa3\x92\x57\x32\x78\xe2\x5d\x01\xa3");

    RED_CHECK(notifier.sender == &wedit);
    RED_CHECK(notifier.event == NOTIFY_SUBMIT);
    notifier.event = 0;
    notifier.sender = nullptr;
}
