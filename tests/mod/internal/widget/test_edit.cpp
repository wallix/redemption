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

#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "mod/internal/copy_paste.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/edit/"

struct TestWidgetEditCtx
{
    TestGraphic drawable{800, 600};
    CopyPaste copy_paste{false};
    WidgetEdit wedit;

    struct Colors
    {
        BGRColor fg_color = RED;
        BGRColor bg_color = YELLOW;
        BGRColor focus_color = bg_color;
    };

    TestWidgetEditCtx(
        char const* text, Colors colors, uint16_t edit_width = 50,
        WidgetEventNotifier onsubmit = WidgetEventNotifier(),
        size_t edit_pos = -1u, int xtext = 0, int ytext = 0)
    : wedit(
        drawable, copy_paste, text, onsubmit,
        colors.fg_color, colors.bg_color, colors.bg_color,
        global_font_deja_vu_14(), edit_pos, xtext, ytext)
    {
        Dimension dim = wedit.get_optimal_dim();
        wedit.set_wh(edit_width, dim.h);
    }

    struct Keyboard
    {
        TestWidgetEditCtx& ctx;
        Keymap keymap{*find_layout_by_id(KeyLayout::KbdId(0x040C))};

        void rdp_input_scancode(uint16_t scancode_and_flags)
        {
            auto ukeycode = underlying_cast(Keymap::KeyCode(scancode_and_flags));
            auto scancode = Keymap::Scancode(ukeycode & 0x7F);
            auto flags = (ukeycode & 0x80) ? Keymap::KbdFlags::Extended : Keymap::KbdFlags();
            keymap.event(flags, scancode);
            ctx.wedit.rdp_input_scancode(flags, scancode, 0, keymap);
            ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());
        }
    };

    Keyboard keyboard()
    {
        return {*this};
    }
};


RED_AUTO_TEST_CASE(TraceWidgetEdit)
{
    TestWidgetEditCtx ctx("test1", {}, 50, WidgetEventNotifier(), 2, 4, 1);

    ctx.wedit.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit2)
{
    TestWidgetEditCtx ctx("test2", {.focus_color = BLACK});

    ctx.wedit.set_xy(10, 100);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit3)
{
    TestWidgetEditCtx ctx("test3", {.focus_color = BLACK});

    ctx.wedit.set_xy(-10, 500);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit4)
{
    TestWidgetEditCtx ctx("test4", {.focus_color = BLACK});

    ctx.wedit.set_xy(770, 500);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit5)
{
    TestWidgetEditCtx ctx("test5", {.focus_color = BLACK});

    ctx.wedit.set_xy(-20, -7);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEdit6)
{
    TestWidgetEditCtx ctx("test6", {.focus_color = BLACK});

    ctx.wedit.set_xy(760, -7);

    // ask to widget to redraw at it's current position
    ctx.wedit.rdp_input_invalidate(ctx.wedit.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip)
{
    TestWidgetEditCtx ctx("test6", {.focus_color = BLACK});

    ctx.wedit.set_xy(760, -7);

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    ctx.wedit.rdp_input_invalidate(Rect(
        20 + ctx.wedit.x(),
        ctx.wedit.y(),
        ctx.wedit.cx(),
        ctx.wedit.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_8.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditClip2)
{
    TestWidgetEditCtx ctx("test6", {.focus_color = BLACK});

    ctx.wedit.set_xy(0, 0);

    // ask to widget to redraw at position 30,12 and of size 30x10.
    ctx.wedit.rdp_input_invalidate(Rect(
        20 + ctx.wedit.x(),
        5 + ctx.wedit.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_9.png");
}

RED_AUTO_TEST_CASE(EventWidgetEdit)
{
    NotifyTrace onsubmit;
    TestWidgetEditCtx ctx("abcdef", {GREEN, RED, RED}, 100, onsubmit);

    ctx.wedit.set_xy(0, 0);

    ctx.wedit.focus(Widget::focus_reason_tabkey);

    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_10.png");

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(0x10); // 'a'
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_11.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(0x11); // 'z'
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_12.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(0x148); // up
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_13.png");
    RED_CHECK(onsubmit.get_and_reset() == 0);

    keyboard.rdp_input_scancode(0x14d); // right
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_12.png");

    keyboard.rdp_input_scancode(0x0e); // backspace
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_11.png");

    keyboard.rdp_input_scancode(0x14b); // left
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_16.png");

    keyboard.rdp_input_scancode(0x14b); // left
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_17.png");

    keyboard.rdp_input_scancode(0x153); // delete
    RED_CHECK(onsubmit.get_and_reset() == 0);
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_18.png");

    keyboard.rdp_input_scancode(0x14f); // end
    RED_CHECK(onsubmit.get_and_reset() == 0);
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_19.png");

    keyboard.rdp_input_scancode(0x147); // home
    RED_CHECK(onsubmit.get_and_reset() == 0);
    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_20.png");

    RED_CHECK(onsubmit.get_and_reset() == 0);
    keyboard.rdp_input_scancode(0x1c); // enter
    RED_CHECK(onsubmit.get_and_reset() == 1);

    ctx.wedit.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 10, 3);
    RED_CHECK(onsubmit.get_and_reset() == 0);

    ctx.wedit.rdp_input_invalidate(Rect(0, 0, ctx.wedit.cx(), ctx.wedit.cx()));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_21.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditAndComposite)
{
    TestGraphic drawable(800, 600);
    CopyPaste copy_paste(false);

    // WidgetEdit is a edit widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font_deja_vu_14(), Theme{});

    WidgetComposite wcomposite(drawable, Widget::Focusable::No);
    wcomposite.set_wh(800, 600);
    wcomposite.set_xy(0, 0);

    WidgetEdit wedit1(drawable, copy_paste, "abababab",
                      {WidgetEventNotifier()}, YELLOW, BLACK, BLACK, global_font_deja_vu_14());
    Dimension dim = wedit1.get_optimal_dim();
    wedit1.set_wh(50, dim.h);
    wedit1.set_xy(0, 0);

    WidgetEdit wedit2(drawable, copy_paste, "ggghdgh",
                      {WidgetEventNotifier()}, WHITE, RED, RED, global_font_deja_vu_14());
    dim = wedit2.get_optimal_dim();
    wedit2.set_wh(50, dim.h);
    wedit2.set_xy(0, 100);

    WidgetEdit wedit3(drawable, copy_paste, "lldlslql",
                      {WidgetEventNotifier()}, BLUE, RED, RED, global_font_deja_vu_14());
    dim = wedit3.get_optimal_dim();
    wedit3.set_wh(50, dim.h);
    wedit3.set_xy(100, 100);

    WidgetEdit wedit4(drawable, copy_paste, "LLLLMLLM",
                      {WidgetEventNotifier()}, PINK, DARK_GREEN, DARK_GREEN, global_font_deja_vu_14());
    dim = wedit4.get_optimal_dim();
    wedit4.set_wh(50, dim.h);
    wedit4.set_xy(300, 300);

    WidgetEdit wedit5(drawable, copy_paste, "dsdsdjdjs",
                      {WidgetEventNotifier()}, LIGHT_GREEN, DARK_BLUE, DARK_BLUE, global_font_deja_vu_14());
    dim = wedit5.get_optimal_dim();
    wedit5.set_wh(50, dim.h);
    wedit5.set_xy(700, -10);

    WidgetEdit wedit6(drawable, copy_paste, "xxwwp",
                      {WidgetEventNotifier()}, ANTHRACITE, PALE_GREEN, PALE_GREEN, global_font_deja_vu_14());
    dim = wedit6.get_optimal_dim();
    wedit6.set_wh(50, dim.h);
    wedit6.set_xy(-10, 550);

    wcomposite.add_widget(wedit1);
    wcomposite.add_widget(wedit2);
    wcomposite.add_widget(wedit3);
    wcomposite.add_widget(wedit4);
    wcomposite.add_widget(wedit5);
    wcomposite.add_widget(wedit6);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wcomposite.rdp_input_invalidate(Rect(100, 25, 100, 100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_22.png");

    // ask to widget to redraw at it's current position
    wcomposite.rdp_input_invalidate(Rect(0, 0, wcomposite.cx(), wcomposite.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "edit_23.png");
}

RED_AUTO_TEST_CASE(TraceWidgetEditScrolling)
{
    TestWidgetEditCtx ctx("abcde", {BLACK, WHITE, WHITE}, 100, {WidgetEventNotifier()}, size_t(-1u), 1, 1);
    WidgetScreen parent{ctx.drawable, 800, 600, global_font_deja_vu_14(), Theme{}};

    ctx.wedit.set_xy(0, 0);

    ctx.wedit.focus(Widget::focus_reason_tabkey);
    parent.add_widget(ctx.wedit);
    parent.current_focus = &ctx.wedit;

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    auto keyboard = ctx.keyboard();

    keyboard.rdp_input_scancode(0x10); // 'a'

    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_24.png");

    keyboard.rdp_input_scancode(0x11); // 'z'
    keyboard.rdp_input_scancode(0x12); // 'e'
    keyboard.rdp_input_scancode(0x10); // 'a'
    keyboard.rdp_input_scancode(0x10); // 'a'
    keyboard.rdp_input_scancode(0x10); // 'a'
    keyboard.rdp_input_scancode(0x10); // 'a'
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_25.png");

    keyboard.rdp_input_scancode(0x10); // 'a'
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_26.png");

    keyboard.rdp_input_scancode(0x10); // 'a'
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_27.png");

    keyboard.rdp_input_scancode(0x19); // 'p'
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_28.png");

    keyboard.rdp_input_scancode(0x147); // home
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_29.png");

    keyboard.rdp_input_scancode(0x14f); // end
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_28.png");

    keyboard.rdp_input_scancode(0x0e); // backspace
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_31.png");

    for (int i = 0; i < 10; i++) {
        keyboard.rdp_input_scancode(0x14b); // left
    }
    parent.rdp_input_invalidate(Rect(0, 0, parent.cx(), parent.cy()));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "edit_32.png");
}
