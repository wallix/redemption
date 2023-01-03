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

#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/dialog.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/dialog/"

struct TestWidgetDialogCtx
{
    TestGraphic drawable;
    CopyPaste copy_paste{false};
    WidgetScreen parent;
    WidgetDialog flat_dialog;

    TestWidgetDialogCtx(
        uint16_t w, uint16_t h,
        const char* caption, const char * text,
        NotifyApi* notify = nullptr,
        ChallengeOpt has_challenge = NO_CHALLENGE,
        char const* logo_path = nullptr)
    : TestWidgetDialogCtx(
        w, h, w, h, w, h, caption, text, notify, has_challenge, logo_path)
    {}

    TestWidgetDialogCtx(
        uint16_t w, uint16_t h,
        uint16_t dialogW, uint16_t dialogH,
        const char* caption, const char * text,
        NotifyApi* notify = nullptr,
        ChallengeOpt has_challenge = NO_CHALLENGE,
        char const* logo_path = nullptr)
    : TestWidgetDialogCtx(
        w, h, w, h, dialogW, dialogH, caption, text, notify, has_challenge, logo_path)
    {}

    TestWidgetDialogCtx(
        uint16_t w, uint16_t h,
        uint16_t parentW, uint16_t parentH,
        uint16_t dialogW, uint16_t dialogH,
        const char* caption, const char * text,
        NotifyApi* notify = nullptr,
        ChallengeOpt has_challenge = NO_CHALLENGE,
        char const* logo_path = nullptr)
    : drawable{w, h}
    , parent{drawable, parentW, parentH, global_font_deja_vu_14(), nullptr, Theme{}}
    , flat_dialog(
        drawable, copy_paste, {0, 0, dialogW, dialogH}, parent, notify,
        caption, text, /*extra_button=*/nullptr,
        [logo_path]{
            Theme colors;
            colors.global.bgcolor = DARK_BLUE_BIS;
            colors.global.fgcolor = WHITE;
            if (logo_path) {
                colors.global.enable_theme = true;
                colors.global.logo_path = logo_path;
            }
            return colors;
        }(), global_font_deja_vu_14(), "Ok", "Cancel", has_challenge)
    {
    }
};

RED_AUTO_TEST_CASE(TraceWidgetDialog)
{
    TestWidgetDialogCtx ctx(800, 600, "test1",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialog2)
{
    TestWidgetDialogCtx ctx(800, 600, 640, 480, "test2",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialog3)
{
    TestWidgetDialogCtx ctx(800, 600, 1280, 1024, "test3",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialog4)
{
    TestWidgetDialogCtx ctx(1280, 1024, "test4",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialog5)
{
    TestWidgetDialogCtx ctx(640, 480, "test5",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialog6)
{
    TestWidgetDialogCtx ctx(352, 500, 300, 600, 350, 500, "test6",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialogClip)
{
    TestWidgetDialogCtx ctx(800, 600, 300, 600, "test6",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at position 780,-7 and of size 120x20. After clip the size is of 20x13
    ctx.flat_dialog.rdp_input_invalidate(Rect(
        20 + ctx.flat_dialog.x(),
        0 + ctx.flat_dialog.y(),
        ctx.flat_dialog.cx(),
        ctx.flat_dialog.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetDialogClip2)
{
    TestWidgetDialogCtx ctx(800, 600, "test6",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at position 30,12 and of size 30x10.
    ctx.flat_dialog.rdp_input_invalidate(Rect(
        20 + ctx.flat_dialog.x(),
        5 + ctx.flat_dialog.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_8.png");
}

RED_AUTO_TEST_CASE(EventWidgetOkCancel)
{
    NotifyTrace notifier;
    TestWidgetDialogCtx ctx(800, 600, "test6",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4",
        &notifier);

    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    int x = ctx.flat_dialog.ok.x() + ctx.flat_dialog.ok.cx() / 2 ;
    int y = ctx.flat_dialog.ok.y() + ctx.flat_dialog.ok.cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    // ctx.flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_9.png");


    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    // ctx.flat_dialog.ok.rdp_input_mouse(MOUSE_FLAG_BUTTON1,
    //                                  ctx.flat_dialog.ok.x(), ctx.flat_dialog.ok.y(), nullptr);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_10.png");


    x = ctx.flat_dialog.cancel->x() + ctx.flat_dialog.cancel->cx() / 2 ;
    y = ctx.flat_dialog.cancel->y() + ctx.flat_dialog.cancel->cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    // ctx.flat_dialog.cancel->rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, 15, 15, nullptr);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == 0);


    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_11.png");


    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_CANCEL);

    notifier.last_widget = nullptr;
    notifier.last_event = 0;

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x01)); // esc
    ctx.flat_dialog.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x01), 0, keymap);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(EventWidgetChallenge)
{
    NotifyTrace notifier;
    TestWidgetDialogCtx ctx(800, 600, "test6",
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
        "adipiscing et arcu.",
        &notifier, CHALLENGE_ECHO);

    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == 0);


    ctx.flat_dialog.challenge->set_text("challenge_test");

    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == 0);

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_12.png");

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x1c)); // enter
    ctx.flat_dialog.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x1c), 0, keymap);
    RED_CHECK(notifier.last_widget == &ctx.flat_dialog);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);
}

RED_AUTO_TEST_CASE(TraceWidgetDialog_transparent_png_with_theme_color)
{
    TestWidgetDialogCtx ctx(800, 600, "test1",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4", nullptr, NO_CHALLENGE,
        FIXTURES_PATH "/wablogoblue-transparent.png");

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_13.png");
}
