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
#include "mod/internal/widget/login.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap.hpp"
#include "keyboard/keylayouts.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"
#include "test_only/mod/internal/widget/notify_trace.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/login/"

constexpr const char * LOGON_MESSAGE = "Warning! Unauthorized access to this system is forbidden and will be prosecuted by law.";

struct TestWidgetLoginCtx
{
    TestGraphic drawable{800, 600};
    CopyPaste copy_paste{false};
    WidgetScreen parent{drawable, 800, 600, global_font_deja_vu_14(), nullptr, Theme{}};
    WidgetLogin flat_login;

    TestWidgetLoginCtx(
        const char * caption,
        const char * login,
        const char * password,
        const char * target,
        const char * login_message = LOGON_MESSAGE,
        NotifyApi* notifier = nullptr,
        Theme theme = Theme(),
        bool enable_target_field = false)
    : flat_login(
        drawable, copy_paste, 0, 0, parent.cx(), parent.cy(), parent, notifier,
        caption, login, password, target,
        "Login", "Password", "Target", "",
        login_message, nullptr, enable_target_field, global_font_deja_vu_14(),
        Translator{Language::en}, theme)
    {}
};

RED_AUTO_TEST_CASE(TraceWidgetLogin)
{
    TestWidgetLoginCtx ctx("test1", "rec", "rec", "");

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLogin2)
{
    TestWidgetLoginCtx ctx("test2", nullptr, nullptr, nullptr);

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLogin3)
{
    NotifyTrace notifier;
    TestWidgetLoginCtx ctx(
        "test3", nullptr, nullptr, nullptr,
        LOGON_MESSAGE, &notifier);

    ctx.flat_login.set_widget_focus(&ctx.flat_login.password_edit, Widget::focus_reason_tabkey);

    RED_CHECK(notifier.last_widget == nullptr);
    RED_CHECK(notifier.last_event == 0);

    Keymap keymap(*find_layout_by_id(KeyLayout::KbdId(0x040C)));
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x1c));
    ctx.flat_login.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x1c), 0, keymap);
    RED_CHECK(notifier.last_widget == &ctx.flat_login);
    RED_CHECK(notifier.last_event == NOTIFY_SUBMIT);

    // ask to widget to redraw at it's current position
    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_3.png");

    notifier.last_widget = nullptr;
    notifier.last_event = 0;
    keymap.event(Keymap::KbdFlags(), Keymap::Scancode(0x01));
    ctx.flat_login.rdp_input_scancode(Keymap::KbdFlags(), Keymap::Scancode(0x01), 0, keymap);
    RED_CHECK(notifier.last_widget == &ctx.flat_login);
    RED_CHECK(notifier.last_event == NOTIFY_CANCEL);
}

RED_AUTO_TEST_CASE(TraceWidgetLoginHelp)
{
    TestWidgetLoginCtx ctx("test4", nullptr, nullptr, nullptr);

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_help_1.png");

    ctx.flat_login.rdp_input_mouse(
        MOUSE_FLAG_MOVE,
        ctx.flat_login.helpicon.x() + ctx.flat_login.helpicon.cx() / 2,
        ctx.flat_login.helpicon.y() + ctx.flat_login.helpicon.cy() / 2);

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_help_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLoginClip)
{
    TestWidgetLoginCtx ctx("test6", nullptr, nullptr, nullptr);

    ctx.flat_login.rdp_input_invalidate(Rect(
        20 + ctx.flat_login.x(),
        ctx.flat_login.y(),
        ctx.flat_login.cx(),
        ctx.flat_login.cy()
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLoginClip2)
{
    TestWidgetLoginCtx ctx("test6", nullptr, nullptr, nullptr);

    ctx.flat_login.rdp_input_invalidate(Rect(
        20 + ctx.flat_login.x(),
        5 + ctx.flat_login.y(),
        30,
        10
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLogin4)
{
    TestWidgetLoginCtx ctx(
        "test1",
        "rec", "rec", "rec",
        "WARNING: Unauthorized access to this system is forbidden and will be prosecuted by law.\n\n"
        "By accessing this system, you agree that your actions may be monitored if unauthorized usage is suspected.");

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLogin_transparent_png_with_theme_color)
{
    Theme colors;
    colors.global.enable_theme = true;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";

    TestWidgetLoginCtx ctx(
        "test1", "rec", "rec", "rec",
        LOGON_MESSAGE, nullptr, colors);

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_7.png");
}

RED_AUTO_TEST_CASE(TraceWidgetLogin_target_field)
{
    Theme colors;
    colors.global.enable_theme = true;
    colors.global.logo_path = FIXTURES_PATH"/wablogoblue-transparent.png";

    TestWidgetLoginCtx ctx(
        "test1", "rec", "rec", "",
        LOGON_MESSAGE, nullptr, colors, true);

    ctx.flat_login.rdp_input_invalidate(ctx.flat_login.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "login_8.png");
}
