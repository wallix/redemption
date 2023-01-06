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
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/core/font.hpp"

#include "mod/internal/widget/image.hpp"
#include "mod/internal/widget/screen.hpp"

#include "utils/sugar/array_view.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/mod/internal/widget/image/"

struct TestWidgetimageCtx
{
    TestGraphic drawable{800, 600};
    WidgetScreen parent{drawable, 800, 600, global_font(), Theme{}};
    WidgetImage wimage;

    TestWidgetimageCtx(const char * filename, BGRColor bg_color = BLACK)
    : wimage(drawable, filename, parent, bg_color)
    {
        wimage.set_wh(wimage.get_optimal_dim());
    }
};

RED_AUTO_TEST_CASE(TraceWidgetImage)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(0, 0);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage2)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(10, 100);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage3)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(-100, 500);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage4)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(700, 500);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage5)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(-100, -100);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage6)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(700, -100);
    ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImageClip)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(700, -100);
    ctx.wimage.rdp_input_invalidate(Rect(
        80 + ctx.wimage.x(),
        10 + ctx.wimage.y(),
        50,
        100
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_clip_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImageClip2)
{
    TestWidgetimageCtx ctx(FIXTURES_PATH"/logo-redemption.png");
    ctx.wimage.set_xy(0, 0);
    ctx.wimage.rdp_input_invalidate(Rect(
        100 + ctx.wimage.x(),
        25 + ctx.wimage.y(),
        100,
        100
    ));

    RED_CHECK_IMG(ctx.drawable, IMG_TEST_PATH "image_clip_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage_transparent_png)
{
    struct Png
    {
        const char *filename;
        BGRColor applied_bg_color;
        char const* imgref;
    };

    RED_TEST_CONTEXT_DATA(const Png& png, "filename: " << png.filename,
    {
        Png
        {
            FIXTURES_PATH"/logo-redemption-transparent.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_1.png",
        },
        Png
        {
            FIXTURES_PATH"/logo-redemption-transparent2.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_2.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-transparent.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_1.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-transparent2.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_2.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-transparent3.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_3.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-transparent4.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_4.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-without-background-transparent.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_1.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-without-background-transparent2.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_2.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-without-background-transparent3.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_3.png",
        },
        Png
        {
            FIXTURES_PATH"/alpha-channel-without-background-transparent4.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_alpha_channel_4.png",
        },
        Png
        {
            FIXTURES_PATH"/logo-redemption-half-transparent.png",
            WHITE,
            IMG_TEST_PATH "image_transparent_half_transparent_1.png",
        },
        Png
        {
            FIXTURES_PATH"/checkers-half-transparent.png",
            DARK_BLUE_BIS,
            IMG_TEST_PATH "image_transparent_half_transparent_2.png",
        },
    })
    {
        TestWidgetimageCtx ctx(png.filename, png.applied_bg_color);
        ctx.wimage.set_xy(0, 0);

        ctx.wimage.rdp_input_invalidate(ctx.wimage.get_rect());

        RED_CHECK_IMG(ctx.drawable, png.imgref);
    }
}
