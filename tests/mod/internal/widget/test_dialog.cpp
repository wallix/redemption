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
#include "test_only/front/fake_front.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"

#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/dialog.hpp"
#include "mod/internal/widget/edit.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "utils/theme.hpp"
#include "core/channel_list.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/dialog/"

namespace
{

struct TestWidgetDialogCtx
{
    TestGraphic drawable;
    NotifyTrace onsubmit;
    NotifyTrace oncancel;
    WidgetDialog flat_dialog;

    TestWidgetDialogCtx(
        uint16_t w, uint16_t h,
        const char* caption, const char * text,
        char const* logo_path = nullptr)
    : TestWidgetDialogCtx(
        w, h, w, h, caption, text, logo_path)
    {}

    TestWidgetDialogCtx(
        uint16_t w, uint16_t h,
        uint16_t dialogW, uint16_t dialogH,
        const char* caption, const char * text,
        char const* logo_path = nullptr)
    : drawable{w, h}
    , flat_dialog(
        drawable, {0, 0, dialogW, dialogH},
        {onsubmit, oncancel},
        caption, text,
        [logo_path]{
            Theme colors;
            colors.global.bgcolor = DARK_BLUE_BIS;
            colors.global.fgcolor = WHITE;
            if (logo_path) {
                colors.global.enable_theme = true;
                colors.global.logo_path = logo_path;
            }
            return colors;
        }(), global_font_deja_vu_14(), "Ok", "Cancel")
    {
        flat_dialog.init_focus();
    }
};

struct TestWidgetDialogWithChallengeCtx
{
    TestGraphic drawable;
    CopyPaste copy_paste{false};
    NotifyTrace onsubmit;
    NotifyTrace oncancel;
    WidgetDialogWithChallenge flat_dialog;

    TestWidgetDialogWithChallengeCtx(
        uint16_t w, uint16_t h,
        const char* caption, const char * text,
        WidgetDialogWithChallenge::ChallengeOpt challenge,
        char const* logo_path = nullptr)
    : drawable{w, h}
    , flat_dialog(
        drawable, {0, 0, w, h},
        {onsubmit, oncancel, WidgetEventNotifier()},
        caption, text, /*extra_button=*/nullptr, "Ok",
        global_font_deja_vu_14(),
        [logo_path]{
            Theme colors;
            colors.global.bgcolor = DARK_BLUE_BIS;
            colors.global.fgcolor = WHITE;
            if (logo_path) {
                colors.global.enable_theme = true;
                colors.global.logo_path = logo_path;
            }
            return colors;
        }(), copy_paste, challenge)
    {
        flat_dialog.init_focus();
    }
};

struct TestWidgetDialogWithCopyableLinkCtx
{
    TestGraphic drawable;
    CopyPaste copy_paste{false};
    NotifyTrace onsubmit;
    NotifyTrace oncancel;
    WidgetDialogWithCopyableLink flat_dialog;

    TestWidgetDialogWithCopyableLinkCtx(uint16_t w, uint16_t h, const char* caption, const char * text)
    : drawable{w, h}
    , flat_dialog(
        drawable, {0, 0, w, h}, {onsubmit, oncancel},
        caption, text, "value", "name", "Ok", global_font_deja_vu_14(),
        []{
            Theme colors;
            colors.global.bgcolor = DARK_BLUE_BIS;
            colors.global.fgcolor = WHITE;
            return colors;
        }(), copy_paste)
    {
        flat_dialog.init_focus();
    }
};

} // anonymous namespace


/*
 * WidgetDialog
 */

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
    TestWidgetDialogCtx ctx(352, 500, 350, 500, "test6",
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
    TestWidgetDialogCtx ctx(800, 600, "test6",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    int x = ctx.flat_dialog.ok.x() + ctx.flat_dialog.ok.cx() / 2 ;
    int y = ctx.flat_dialog.ok.y() + ctx.flat_dialog.ok.cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_9.png");

    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 1);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_10.png");


    x = ctx.flat_dialog.cancel->x() + ctx.flat_dialog.cancel->cx() / 2 ;
    y = ctx.flat_dialog.cancel->y() + ctx.flat_dialog.cancel->cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);


    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_11.png");


    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 1);

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x01)); // esc
    ctx.flat_dialog.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x01), 0, keymap);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 1);
}

RED_AUTO_TEST_CASE(TraceWidgetDialog_transparent_png_with_theme_color)
{
    TestWidgetDialogCtx ctx(800, 600, "test1",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4",
        FIXTURES_PATH "/wablogoblue-transparent.png");

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_13.png");
}

/*
 * WidgetDialogWithChallenge
 */

RED_AUTO_TEST_CASE(EventWidgetChallenge)
{
    TestWidgetDialogWithChallengeCtx ctx(800, 600, "test6",
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
        WidgetDialogWithChallenge::ChallengeOpt::Echo);

    ctx.flat_dialog.challenge->set_text("challenge_test");

    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());
    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_challenge_1.png");

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x1c)); // enter
    ctx.flat_dialog.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x1c), 0, keymap);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 1);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);
}

/*
 * WidgetDialogWithCopyableLink
 */

RED_AUTO_TEST_CASE(TraceWidgetDialogWithCopyableLink)
{
    TestWidgetDialogWithCopyableLinkCtx ctx(800, 600, "test1",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    // ask to widget to redraw at it's current position
    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_link_1.png");
}

RED_AUTO_TEST_CASE(EventWidgetDialogWithCopyableLinkOkLink)
{
    TestWidgetDialogWithCopyableLinkCtx ctx(800, 600, "test1",
        "line 1\n"
        "line 2\n"
        "\n"
        "line 3, blah blah\n"
        "line 4");

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    int x = ctx.flat_dialog.ok.x() + ctx.flat_dialog.ok.cx() / 2 ;
    int y = ctx.flat_dialog.ok.y() + ctx.flat_dialog.ok.cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_link_2.png");

    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 1);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_link_1.png");

    // link click

    ScreenInfo screen_info{ctx.drawable.width(), ctx.drawable.height(), BitsPerPixel::BitsPP24};
    FakeFront front{screen_info};
    front.get_writable_channel_list().push_back(CHANNELS::ChannelDef(channel_names::cliprdr, 0, 0));
    ctx.copy_paste.ready(front);

    struct PastedWidget : Widget
    {
        using Widget::Widget;

        void clipboard_insert_utf8(zstring_view text) override
        {
            str = text;
        }

        void rdp_input_invalidate(Rect rect) override
        {
            (void)rect;
        }

        zstring_view str;
    };

    PastedWidget pasted(ctx.drawable, Widget::Focusable::No);

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    ctx.copy_paste.paste(pasted);
    RED_CHECK(pasted.str == ""_av);

    x = ctx.flat_dialog.link->copy.x() + ctx.flat_dialog.link->copy.cx() / 2 ;
    y = ctx.flat_dialog.link->copy.y() + ctx.flat_dialog.link->copy.cy() / 2 ;
    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1|MOUSE_FLAG_DOWN, x, y);
    ctx.copy_paste.paste(pasted);
    RED_CHECK(pasted.str == "value"_av);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 0);

    ctx.flat_dialog.rdp_input_invalidate(ctx.flat_dialog.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_link_1.png");

    ctx.flat_dialog.rdp_input_mouse(MOUSE_FLAG_BUTTON1, x, y);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "dialog_link_1.png");

    // exit

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x01)); // esc
    ctx.flat_dialog.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x01), 0, keymap);
    RED_CHECK(ctx.onsubmit.get_and_reset() == 0);
    RED_CHECK(ctx.oncancel.get_and_reset() == 1);
}
