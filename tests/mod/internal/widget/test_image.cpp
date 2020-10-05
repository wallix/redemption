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

RED_AUTO_TEST_CASE(TraceWidgetImage)
{
    TestGraphic drawable(800, 600);

    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(0, 0);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage2)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position 10,100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(10, 100);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_2.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage3)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position -100,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(-100, 500);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_3.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage4)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position 700,500 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(700, 500);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_4.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage5)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position -100,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(-100, -100);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_5.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImage6)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(700, -100);

    // ask to widget to redraw at it's current position
    wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                     0 + wimage.y(),
                                     wimage.cx(),
                                     wimage.cy()));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_6.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImageClip)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position 700,-100 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(700, -100);

    // ask to widget to redraw at position 80,10 and of size 50x100. After clip the size is of 20x15
    wimage.rdp_input_invalidate(Rect(80 + wimage.x(),
                                     10 + wimage.y(),
                                     50,
                                     100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_clip_1.png");
}

RED_AUTO_TEST_CASE(TraceWidgetImageClip2)
{
    TestGraphic drawable(800, 600);


    // WidgetImage is a image widget of size 256x125 at position 0,0 in it's parent context
    WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, Theme{});

    NotifyApi * notifier = nullptr;

    WidgetImage wimage(drawable, FIXTURES_PATH"/logo-redemption.png", parent, notifier, BLACK);
    Dimension dim = wimage.get_optimal_dim();
    wimage.set_wh(dim);
    wimage.set_xy(0, 0);

    // ask to widget to redraw at position 100,25 and of size 100x100.
    wimage.rdp_input_invalidate(Rect(100 + wimage.x(),
                                     25 + wimage.y(),
                                     100,
                                     100));

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "image_clip_2.png");
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
        TestGraphic drawable(800, 600);
        WidgetScreen parent(drawable, 800, 600, global_font(), nullptr, { });
        NotifyApi *notifier = nullptr;
        WidgetImage wimage(drawable,
                           png.filename,
                           parent,
                           notifier,
                           png.applied_bg_color);
        Dimension dim = wimage.get_optimal_dim();

        wimage.set_wh(dim);
        wimage.set_xy(0, 0);

        wimage.rdp_input_invalidate(Rect(0 + wimage.x(),
                                         0 + wimage.y(),
                                         wimage.cx(),
                                         wimage.cy()));

        RED_CHECK_IMG(drawable, png.imgref);
    }
}
