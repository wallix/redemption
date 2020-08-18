/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"

#include "gdi/graphic_api.hpp"
#include "utils/video_cropper.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "test_only/gdi/test_graphic.hpp"

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/utils/video_cropper/"

RED_AUTO_TEST_CASE(TestVideoCropperFull)
{
    TestGraphic drawable(800, 600);

    VideoCropper video_cropper(drawable, 0, 0, 800, 600);

    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable->draw(RDPOpaqueRect(Rect(50, 50, 320, 200), encode_color24()(RED)), Rect(50, 50, 320, 200), color_cxt);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "video_cropper_full_1.png");

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_full_1.png");

    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(125, 75, 320, 200), encode_color24()(RED)), Rect(125, 75, 320, 200), color_cxt);

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_full_2.png");

    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(100, 100, 370, 250), encode_color24()(RED)), Rect(100, 100, 370, 250), color_cxt);

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_full_3.png");
}

RED_AUTO_TEST_CASE(TestVideoCropperReset)
{
    TestGraphic drawable(800, 600);

    VideoCropper video_cropper(drawable, 0, 0, 800, 600);

    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable->draw(RDPOpaqueRect(Rect(50, 50, 320, 200), encode_color24()(GREEN)), Rect(50, 50, 320, 200), color_cxt);

    RED_CHECK_IMG(drawable, IMG_TEST_PATH "video_cropper_reset_1.png");

    video_cropper.reset(50, 50, 320, 200);

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_reset_2.png");

    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(125, 75, 320, 200), encode_color24()(GREEN)), Rect(125, 75, 320, 200), color_cxt);

    video_cropper.reset(125, 75, 320, 200);

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_reset_2.png");

    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(100, 100, 370, 250), encode_color24()(GREEN)), Rect(100, 100, 370, 250), color_cxt);

    video_cropper.reset(100, 100, 370, 250);

    video_cropper.prepare_image_frame();

    RED_CHECK_IMG(video_cropper.get_image_view(), IMG_TEST_PATH "video_cropper_reset_3.png");
}
