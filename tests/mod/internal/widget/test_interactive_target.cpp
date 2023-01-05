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
 *   Author(s): Christophe Grosjean, Jonathan Poelen, Meng Tan, Raphael Zhou,
 *              Jennifer Inthavong
 *
 */

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/interactive_target.hpp"
#include "mod/internal/widget/screen.hpp"

#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/interactive_target/"

struct TestWidgetInteractivePasswordCtx
{
    TestGraphic drawable{800, 600};
    WidgetScreen parent{drawable, 800, 600, global_font_lato_light_16(), nullptr, Theme()};
    CopyPaste copy_paste{false};
    WidgetInteractiveTarget interactive;

    TestWidgetInteractivePasswordCtx(
        bool ask_device, bool ask_login, bool ask_password,
        const char * caption,
        const char * text_device,
        const char * device_str)
    : interactive(
        drawable, copy_paste, 0, 0, 800, 600, parent, /*notifier=*/nullptr,
        ask_device, ask_login, ask_password, []{
            Theme colors;
            colors.global.bgcolor = DARK_BLUE_BIS;
            colors.global.fgcolor = WHITE;
            return colors;
        }(),
        caption, text_device, device_str,
        "Login", "user1", "Password",
        global_font_lato_light_16(), nullptr)
    {
        interactive.rdp_input_invalidate(interactive.get_rect());
    }
};

RED_AUTO_TEST_CASE(TraceWidgetInteractivePassword)
{
    {
        TestWidgetInteractivePasswordCtx ctx(
            true, true, true, "Target Infos",
            "Host", "in 192.168.16.0/24 subnet");

        RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "interactive_target_1.png");
    }
    {
        TestWidgetInteractivePasswordCtx ctx(
            true, false, false, "Target Infos",
            "Host", "in 192.168.16.0/24 subnet");

        RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "interactive_target_2.png");
    }
    {
        TestWidgetInteractivePasswordCtx ctx(
            false, false, true, "Target Infos",
            "Host", "machinetruc");

        RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "interactive_target_3.png");
    }
    {
        TestWidgetInteractivePasswordCtx ctx(
            false, true, true, "Target Infos",
            "Host", "machinetruc");

        RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "interactive_target_4.png");
    }
    {
        TestWidgetInteractivePasswordCtx ctx(
            true, false, true, "Target Infos",
            "Host", "in 192.168.16.0/24 subnet");

        RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "interactive_target_5.png");
    }
}
