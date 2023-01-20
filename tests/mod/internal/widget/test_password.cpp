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
 *              Meng Tan
 */

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/password.hpp"
#include "mod/internal/widget/composite.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/password/"

struct TestWidgetPasswordCtx
{
    TestGraphic drawable{800, 600};
    CopyPaste copy_paste{false};
    WidgetPassword wpassword;

    struct Colors
    {
        BGRColor fg_color = BLUE;
        BGRColor bg_color = YELLOW;
        BGRColor focus_color = bg_color;
    };

    TestWidgetPasswordCtx(
        char const* text, Colors colors, uint16_t edit_width = 50,
        WidgetEventNotifier onsubmit = WidgetEventNotifier(),
        size_t password_pos = -1u, int xtext = 0, int ytext = 0)
    : wpassword(
        drawable, copy_paste, text, onsubmit,
        colors.fg_color, colors.bg_color, colors.focus_color,
        global_font_lato_light_16(), password_pos, xtext, ytext)
    {
        Dimension dim = wpassword.get_optimal_dim();
        wpassword.set_wh(edit_width, dim.h);
    }

    struct Keyboard
    {
        TestWidgetPasswordCtx& ctx;
        Keymap keymap{*find_layout_by_id(KeyLayout::KbdId(0x040C))};

        void rdp_input_scancode(Keymap::KeyCode keycode)
        {
            auto ukeycode = underlying_cast(keycode);
            auto scancode = Keymap::Scancode(ukeycode & 0x7F);
            auto flags = (ukeycode & 0x80) ? Keymap::KbdFlags::Extended : Keymap::KbdFlags();
            keymap.event(flags, scancode);
            ctx.wpassword.rdp_input_scancode(flags, scancode, 0, keymap);
            ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());
        }
    };

    Keyboard keyboard()
    {
        return {*this};
    }
};

RED_AUTO_TEST_CASE(TraceWidgetPassword)
{
    TestWidgetPasswordCtx ctx("test1", {}, 50, WidgetEventNotifier(), 2, 4, 1);

    ctx.wpassword.set_xy(0, 0);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword2)
{
    TestWidgetPasswordCtx ctx("test2", {}, 50);
    ctx.wpassword.set_xy(10, 100);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword3)
{
    TestWidgetPasswordCtx ctx("test3", {}, 50);
    ctx.wpassword.set_xy(-10, 500);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword4)
{
    TestWidgetPasswordCtx ctx("test4", {}, 50);
    ctx.wpassword.set_xy(770, 500);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword5)
{
    TestWidgetPasswordCtx ctx("test5", {}, 50);
    ctx.wpassword.set_xy(-20, -7);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPassword6)
{
    TestWidgetPasswordCtx ctx("test6", {}, 50);
    ctx.wpassword.set_xy(760, -7);
    ctx.wpassword.rdp_input_invalidate(ctx.wpassword.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip)
{
    TestWidgetPasswordCtx ctx("test6", {}, 50);
    ctx.wpassword.set_xy(760, -7);
    ctx.wpassword.rdp_input_invalidate(Rect(
        20 + ctx.wpassword.x(),
        0 + ctx.wpassword.y(),
        ctx.wpassword.cx(),
        ctx.wpassword.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordClip2)
{
    TestWidgetPasswordCtx ctx("test6", {}, 50);
    ctx.wpassword.set_xy(0, 0);
    ctx.wpassword.rdp_input_invalidate(Rect(
        20 + ctx.wpassword.x(),
        5 + ctx.wpassword.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "password_8.png");
}

RED_AUTO_TEST_CASE(EventWidgetPassword)
{
    NotifyTrace onsubmit;
    TestWidgetPasswordCtx ctx("abcdef", {YELLOW, RED}, 100, onsubmit);

    auto& wpassword = ctx.wpassword;
    auto& drawable = ctx.drawable;

    wpassword.set_xy(0, 0);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_9.png");

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(Keymap::KeyCode(0x10)); // 'a'
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode(0x11)); // 'z'
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_11.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::UpArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_12.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::RightArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_11.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::Backspace);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::Delete);
    RED_CHECK(onsubmit.get_and_reset() == 0);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_17.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::End);
    RED_CHECK(onsubmit.get_and_reset() == 0);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_9.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::Home);
    RED_CHECK(onsubmit.get_and_reset() == 0);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_19.png");

    keyboard.rdp_input_scancode(Keymap::KeyCode::Enter);
    RED_CHECK(onsubmit.get_and_reset() == 1);

    wpassword.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3);
    RED_CHECK(onsubmit.get_and_reset() == 0);

    wpassword.rdp_input_invalidate(Rect(0, 0, wpassword.cx(), wpassword.cx()));
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_20.png");
}

RED_AUTO_TEST_CASE(TraceWidgetPasswordAndComposite)
{
    TestGraphic drawable(800, 600);
    CopyPaste copy_paste(false);

    WidgetComposite wcomposite(drawable);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetPassword wpassword1(drawable, copy_paste, "abababab",
                              {WidgetEventNotifier()}, YELLOW, BLACK, BLACK, global_font_lato_light_16());
    Dimension dim = wpassword1.get_optimal_dim();
    wpassword1.set_wh(50, dim.h);
    wpassword1.set_xy(0, 0);

    WidgetPassword wpassword2(drawable, copy_paste, "ggghdgh",
                              {WidgetEventNotifier()}, WHITE, RED, RED, global_font_lato_light_16());
    dim = wpassword2.get_optimal_dim();
    wpassword2.set_wh(50, dim.h);
    wpassword2.set_xy(0, 100);

    WidgetPassword wpassword3(drawable, copy_paste, "lldlslql",
                              {WidgetEventNotifier()}, BLUE, RED, RED, global_font_lato_light_16());
    dim = wpassword3.get_optimal_dim();
    wpassword3.set_wh(50, dim.h);
    wpassword3.set_xy(100, 100);

    WidgetPassword wpassword4(drawable, copy_paste, "LLLLMLLM",
                              {WidgetEventNotifier()}, PINK, DARK_GREEN, DARK_GREEN, global_font_lato_light_16());
    dim = wpassword4.get_optimal_dim();
    wpassword4.set_wh(50, dim.h);
    wpassword4.set_xy(300, 300);

    WidgetPassword wpassword5(drawable, copy_paste, "dsdsdjdjs",
                              {WidgetEventNotifier()}, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_lato_light_16());
    dim = wpassword5.get_optimal_dim();
    wpassword5.set_wh(50, dim.h);
    wpassword5.set_xy(700, -10);

    WidgetPassword wpassword6(drawable, copy_paste, "xxwwp",
                              {WidgetEventNotifier()}, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_lato_light_16());
    dim = wpassword6.get_optimal_dim();
    wpassword6.set_wh(50, dim.h);
    wpassword6.set_xy(-10, 550);

    wcomposite.add_widget(&wpassword1);
    wcomposite.add_widget(&wpassword2);
    wcomposite.add_widget(&wpassword3);
    wcomposite.add_widget(&wpassword4);
    wcomposite.add_widget(&wpassword5);
    wcomposite.add_widget(&wpassword6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_21.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    // drawable.save_to_png("password-compo2.png");

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_22.png");
    wcomposite.clear();
}

RED_AUTO_TEST_CASE(DataWidgetPassword)
{
    NotifyTrace notifier;
    TestWidgetPasswordCtx ctx("aurélie", {YELLOW, RED}, 100, notifier);

    auto& wpassword = ctx.wpassword;
    auto& drawable = ctx.drawable;

    wpassword.set_xy(0, 0);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(notifier.get_and_reset() == 0);

    RED_CHECK("aurélie"_av == wpassword.get_text());

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(notifier.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(notifier.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(notifier.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::Backspace);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_27.png");

    RED_CHECK("aurlie"_av == wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword2)
{
    NotifyTrace onsubmit;
    TestWidgetPasswordCtx ctx("aurélie", {YELLOW, RED}, 100, onsubmit);

    auto& wpassword = ctx.wpassword;
    auto& drawable = ctx.drawable;

    wpassword.set_xy(0, 0);

    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    RED_CHECK("aurélie"_av == wpassword.get_text());

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_32.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::Delete);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_27.png");

    RED_CHECK("aurlie"_av == wpassword.get_text());
}

RED_AUTO_TEST_CASE(DataWidgetPassword3)
{
    NotifyTrace onsubmit;
    TestWidgetPasswordCtx ctx("aurélie", {YELLOW, RED}, 100, onsubmit);

    auto& wpassword = ctx.wpassword;
    auto& drawable = ctx.drawable;

    wpassword.set_xy(0, 0);

    Dimension dim = wpassword.get_optimal_dim();
    wpassword.set_wh(100, dim.h);
    wpassword.set_xy(0, 0);


    wpassword.focus(Widget::focus_reason_tabkey);
    wpassword.rdp_input_invalidate(wpassword.get_rect());
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_10.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    RED_CHECK("aurélie"_av == wpassword.get_text());

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_15.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_16.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::LeftArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_32.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode::RightArrow);
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_26.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(Keymap::KeyCode(0x11)); // 'z'
    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_40.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    RED_CHECK("aurézlie"_av == wpassword.get_text());

    // cursor overflow

    for (int i = 0; i < 10; i++) {
        keyboard.rdp_input_scancode(Keymap::KeyCode(0x11)); // 'z'
    }
    wpassword.rdp_input_invalidate(wpassword.get_rect());

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "password_41.png");
}
