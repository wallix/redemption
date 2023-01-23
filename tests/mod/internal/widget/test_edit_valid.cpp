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

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/widget/edit_valid.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/copy_paste.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"

#include <string_view>

using namespace std::string_view_literals;


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/edit_valid/"

struct TestWidgetEditValid
{
    struct Colors
    {
        BGRColor fg = BLACK;
        BGRColor bg = WHITE;
        BGRColor focus = ANTHRACITE;
    };

    TestGraphic drawable{800, 600};
    CopyPaste copy_paste{false};
    WidgetScreen parent{drawable, 800, 600, global_font_deja_vu_14(), Theme{}};
    NotifyTrace onsubmit;
    WidgetEditValid wedit;

    TestWidgetEditValid(
        Colors colors, const char * text, std::size_t edit_position = -1u,
        const char * title = "", bool pass = false)
    : wedit(drawable, copy_paste, text, onsubmit,
            colors.fg, colors.bg, colors.focus, colors.bg,
            global_font_deja_vu_14(), title, false, edit_position, 0, 0, pass)
    {
    }

    void click_down(int x, int y)
    {
        parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN,
                               wedit.x() + x, wedit.y() + y);
    }

    void click_up(int x, int y)
    {
        parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
                               wedit.x() + x, wedit.y() + y);
    }

    void click(int x, int y)
    {
        click_down(x, y);
        click_up(x, y);
    }

    struct KeyBoard
    {
        TestWidgetEditValid& ctx;
        Keymap keymap;

        void send_scancode(uint16_t scancode_and_flags)
        {
            using KFlags = Keymap::KbdFlags;
            using Scancode = Keymap::Scancode;
            auto scancode = Scancode(scancode_and_flags);
            auto flags = KFlags(scancode_and_flags & 0xff00u);
            keymap.event(flags, scancode);
            ctx.parent.rdp_input_scancode(flags, scancode, 0, keymap);
            keymap.event(flags | KFlags::Release, scancode);
            ctx.parent.rdp_input_scancode(flags | KFlags::Release, scancode, 0, keymap);
        }
    };

    KeyBoard keyboard()
    {
        return KeyBoard{*this, Keymap{*find_layout_by_id(KeyLayout::KbdId(0x040C))}};
    }
};


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestWidgetEditValid ctx({}, "test1", 2);

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(100, dim.h);
    ctx.wedit.set_xy(0, 0);

    ctx.parent.set_widget_focus(ctx.wedit, Widget::focus_reason_tabkey);

    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestWidgetEditValid ctx({}, "");

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(100, dim.h);
    ctx.wedit.set_xy(50, 100);

    ctx.parent.add_widget(ctx.wedit);

    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_2.png");

    ctx.click(2, 2);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_3.png");

    ctx.wedit.set_text("Ylajali");

    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_4.png");

    RED_CHECK_EQUAL("Ylajali"sv, ctx.wedit.get_text());


    ctx.wedit.set_xy(192, 242);

    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_5.png");

    ctx.parent.clear();
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestWidgetEditValid ctx({RED, YELLOW, GREEN}, "dLorz", 0);

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(150, dim.h);
    ctx.wedit.set_xy(54, 105);

    WidgetEditValid wedit2(ctx.drawable, ctx.copy_paste, "", WidgetEventNotifier(),
                           WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(),
                           nullptr, false, 0);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    ctx.parent.add_widget(ctx.wedit);
    ctx.parent.add_widget(wedit2);
    // ask to widget to redraw at it's current position
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_6.png");

    ctx.click(50, 3);

    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_7.png");

    auto keyboard = ctx.keyboard();

    keyboard.send_scancode(0x0F); // tab
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_8.png");

    keyboard.send_scancode(0x10); // 'a'
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_9.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditLabels)
{
    TestWidgetEditValid ctx({RED, YELLOW, GREEN}, "dLorz", 0, "edition1");

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(150, dim.h);
    ctx.wedit.set_xy(54, 105);

    WidgetEditValid wedit2(ctx.drawable, ctx.copy_paste, "", WidgetEventNotifier(),
                           WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(),
                           "edition2", true, 0, 0, 0, false);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    ctx.parent.add_widget(ctx.wedit);
    ctx.parent.add_widget(wedit2);
    // ask to widget to redraw at it's current position
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_10.png");

    ctx.click(50, 2);

    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_11.png");

    auto keyboard = ctx.keyboard();

    keyboard.send_scancode(0x0F); // tab
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_12.png");

    keyboard.send_scancode(0x10); // 'a'
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_9.png");

    keyboard.send_scancode(0x0F); // tab
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_14.png");

    keyboard.send_scancode(0x0F); // tab
    keyboard.send_scancode(0x0E); // backspace
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_15.png");

    keyboard.send_scancode(0x0F); // tab
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_11.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditLabelsPassword)
{
    TestWidgetEditValid ctx({RED, YELLOW, GREEN}, "dLorz", 0, "edition1", true);

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(150, dim.h);
    ctx.wedit.set_xy(54, 105);

    WidgetEditValid wedit2(ctx.drawable, ctx.copy_paste, "", WidgetEventNotifier(),
                           WHITE, DARK_BLUE, RED, DARK_BLUE, global_font_deja_vu_14(),
                           "edition2", true, 0, 0, 0, true);
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(200, dim.h);
    wedit2.set_xy(400, 354);

    ctx.parent.add_widget(ctx.wedit);
    ctx.parent.add_widget(wedit2);
    // ask to widget to redraw at it's current position
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_17.png");

    ctx.click(50, 2);

    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_18.png");

    auto keyboard = ctx.keyboard();

    keyboard.send_scancode(0x0F); // tab
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_19.png");

    keyboard.send_scancode(0x10); // 'a'
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_20.png");

    keyboard.send_scancode(0x0F); // tab
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_21.png");

    keyboard.send_scancode(0x0F); // tab
    keyboard.send_scancode(0x0E); // backspace
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_22.png");

    keyboard.send_scancode(0x0F); // tab
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_18.png");
}

RED_AUTO_TEST_CASE(EventWidgetEditEvents)
{
    TestWidgetEditValid ctx({BLACK, WHITE, DARK_BLUE}, "abcdef");

    Dimension dim = ctx.wedit.get_optimal_dim();
    ctx.wedit.set_wh(100, dim.h);
    ctx.wedit.set_xy(0, 0);

    ctx.parent.add_widget(ctx.wedit);
    ctx.parent.set_widget_focus(ctx.wedit, Widget::focus_reason_tabkey);

    ctx.click_down(95, 2);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_24.png");

    ctx.click_up(95, 2);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(ctx.onsubmit.get_and_reset() == 1);

    ctx.click_down(95, 2);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_24.png");

    ctx.click_up(2, 2);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);

    ctx.click_down(95, 2);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_24.png");

    ctx.parent.rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0);
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);

    auto keyboard = ctx.keyboard();

    keyboard.send_scancode(0x1C); // enter
    ctx.parent.rdp_input_invalidate(ctx.parent.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_valid_25.png");

    RED_CHECK(ctx.onsubmit.get_and_reset() == 1);
}
