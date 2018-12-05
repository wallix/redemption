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

#define RED_TEST_MODULE TestVideoCropper
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "gdi/graphic_api.hpp"
#include "utils/video_cropper.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/check_sig.hpp"


RED_AUTO_TEST_CASE(TestVideoCropperFull)
{
    TestGraphic drawable(800, 600);

    VideoCropper video_cropper(drawable, 0, 0, 800, 600);

    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable->draw(RDPOpaqueRect(Rect(50, 50, 320, 200), encode_color24()(RED)), Rect(50, 50, 320, 200), color_cxt);

    RED_CHECK_SIG(drawable,
        "\x85\x0e\x04\xea\xf7\x6c\x08\xbd\x4f\xb1\xd5\xbe\xf5\xe4\x43\x4e\x1d\x93\x8e\x5e");

    //::dump_png24("./test_video_cropper_000_a.png", drawable);


    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x85\x0e\x04\xea\xf7\x6c\x08\xbd\x4f\xb1\xd5\xbe\xf5\xe4\x43\x4e\x1d\x93\x8e\x5e");

        //::dump_png24("./test_video_cropper_000_b.png", image_view);
    }


    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(125, 75, 320, 200), encode_color24()(RED)), Rect(125, 75, 320, 200), color_cxt);

    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x14\x38\x44\x99\x4f\x53\xee\x22\xb6\xff\x08\x8e\xd7\x13\xe3\x8a\xcf\xea\x7a\x11");

        //::dump_png24("./test_video_cropper_000_c.png", image_view);
    }


    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(100, 100, 370, 250), encode_color24()(RED)), Rect(100, 100, 370, 250), color_cxt);

    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x5c\x56\xb8\x03\x16\xd0\x23\x9d\x86\xdd\x1f\xa1\x19\x85\x61\x28\x84\x65\x15\x1b");

        //::dump_png24("./test_video_cropper_000_d.png", image_view);
    }
}

RED_AUTO_TEST_CASE(TestVideoCropperReset)
{
    TestGraphic drawable(800, 600);

    VideoCropper video_cropper(drawable, 0, 0, 800, 600);

    auto const color_cxt = gdi::ColorCtx::depth24();

    drawable->draw(RDPOpaqueRect(Rect(50, 50, 320, 200), encode_color24()(GREEN)), Rect(50, 50, 320, 200), color_cxt);

    RED_CHECK_SIG(drawable,
        "\xb6\xce\xad\x9e\x0c\x71\x67\xc5\x81\x59\x35\x97\xb8\xf2\x68\x0a\xf9\x7b\x88\x00");

    //::dump_png24("./test_video_cropper_001_a.png", drawable);


    video_cropper.reset(50, 50, 320, 200);

    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x2a\x32\x7a\x69\x72\x3b\xfa\x48\x49\xad\xe4\x12\x79\x3f\x11\xd1\x17\x55\x53\x07");

        //::dump_png24("./test_video_cropper_001_b.png", image_view);
    }


    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(125, 75, 320, 200), encode_color24()(GREEN)), Rect(125, 75, 320, 200), color_cxt);

    video_cropper.reset(125, 75, 320, 200);

    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x2a\x32\x7a\x69\x72\x3b\xfa\x48\x49\xad\xe4\x12\x79\x3f\x11\xd1\x17\x55\x53\x07");

        //::dump_png24("./test_video_cropper_001_c.png", image_view);
    }


    drawable->draw(RDPOpaqueRect(Rect(0, 0, 800, 600), encode_color24()(BLACK)), Rect(0, 0, 800, 600), color_cxt);
    drawable->draw(RDPOpaqueRect(Rect(100, 100, 370, 250), encode_color24()(GREEN)), Rect(100, 100, 370, 250), color_cxt);

    video_cropper.reset(100, 100, 370, 250);

    video_cropper.prepare_image_frame();

    {
        auto const image_view = video_cropper.get_image_view();

        RED_CHECK_SIG(image_view,
            "\x0e\xde\x21\xee\x3b\x11\xf6\xc8\xe0\x68\x94\x43\x9c\x68\x3a\x7c\x41\x9d\xe0\x61");

        //::dump_png24("./test_video_cropper_001_d.png", image_view);
    }
}
