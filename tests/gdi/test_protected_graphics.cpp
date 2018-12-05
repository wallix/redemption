/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#define RED_TEST_MODULE GraphicsOSD
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "gdi/protected_graphics.hpp"
#include "transport/out_filename_sequence_transport.hpp"
#include "transport/transport.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/fileutils.hpp"
#include "utils/png.hpp"
#include "utils/timestamp_tracer.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/gdi/test_graphic.hpp"

#include <chrono>


RED_AUTO_TEST_CASE(TestModOSD)
{
    Rect screen_rect(0, 0, 800, 600);
    TestGraphic drawable(screen_rect.cx, screen_rect.cy);
    auto const color_cxt = gdi::ColorCtx::depth24();

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid, ReportError{});

    auto do_snapshot = [](TestGraphic& drawable, Transport& trans, timeval const& now){
        drawable.trace_mouse();
        tm ptm;
        localtime_r(&now.tv_sec, &ptm);
        TimestampTracer timestamp_tracer(drawable.get_mutable_image_view());
        timestamp_tracer.trace(ptm);
        ::dump_png24(trans, drawable, true);
        trans.next();
        timestamp_tracer.clear();
        drawable.clear_mouse();
    };

    drawable.show_mouse_cursor(false);

    drawable->draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), encode_color24()(RED)), screen_rect, color_cxt);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;
    now.tv_sec++;

    {
        Bitmap const bmp = bitmap_from_file(FIXTURES_PATH "/ad8b.bmp");
        int const bmp_x = 200;
        int const bmp_y = 200;
        Rect const bmp_rect(bmp_x, bmp_y, bmp.cx(), bmp.cy());
        Rect const rect = bmp_rect.intersect(screen_rect.cx, screen_rect.cy);
        drawable->draw(RDPMemBlt(0, bmp_rect, 0xCC, 0, 0, 0), rect, bmp);

        now.tv_sec++;
        do_snapshot(drawable, trans, now);

        struct OSD : gdi::ProtectedGraphics
        {
            OSD(GraphicApi & drawable, Rect const rect)
            : gdi::ProtectedGraphics(drawable, rect)
            {}

            void refresh_rects(array_view<Rect const>) override
            {
                RED_FAIL("refresh_rects is called");
            }
        } osd(drawable, rect);
        osd.draw(RDPOpaqueRect(Rect(100, 100, 200, 200), encode_color24()(GREEN)), screen_rect, color_cxt);
        now.tv_sec++;
        do_snapshot(drawable, trans, now);
    }

    trans.disconnect();

    RED_CHECK_EQUAL(5021, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(5047, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}

RED_AUTO_TEST_CASE(TestModOSD2)
{
    Rect screen_rect(0, 0, 800, 600);
    TestGraphic drawable(screen_rect.cx, screen_rect.cy);
    auto const color_cxt = gdi::ColorCtx::depth24();

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "test2", ".png", groupid, ReportError{});

    class ImageCaptureLocal
    {
        Transport & trans;
        TestGraphic & drawable;
        timeval start_capture;
        TimestampTracer timestamp_tracer;

    public:
        ImageCaptureLocal(TestGraphic & drawable, Transport & trans)
        : trans(trans)
        , drawable(drawable)
        , timestamp_tracer(drawable.get_mutable_image_view())
        {
        }

        std::chrono::microseconds do_snapshot(const timeval & now)
        {
            this->drawable.trace_mouse();
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->timestamp_tracer.trace(ptm);
            this->dump24();
            this->trans.next();
            this->timestamp_tracer.clear();
            this->start_capture = now;
            this->drawable.clear_mouse();
            return std::chrono::microseconds::zero();
        }

        void dump24() const {
            ::dump_png24(this->trans, this->drawable, true);
        }
    };


    ImageCaptureLocal consumer(drawable, trans);

    drawable.show_mouse_cursor(false);

    drawable->draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), encode_color24()(RED)), screen_rect, color_cxt);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;
    now.tv_sec++;

    {
        Rect const rect = Rect(100, 100, 200, 200);
        drawable->draw(RDPOpaqueRect(rect, encode_color24()(GREEN)), screen_rect, color_cxt);

        now.tv_sec++;
        consumer.do_snapshot(now);

        RED_CHECK_SIG(drawable, "\x67\x3a\xb4\xb9\x9f\x7f\xe9\x47\xbb\x49\xd3\xf7\x03\xf1\x5c\x07\x80\xeb\x1f\x62");

        Bitmap const bmp = bitmap_from_file(FIXTURES_PATH "/ad8b.bmp");
        int const bmp_x = 200;
        int const bmp_y = 200;
        Rect const bmp_rect(bmp_x, bmp_y, bmp.cx(), bmp.cy());
        struct OSD : gdi::ProtectedGraphics
        {
            OSD(GraphicApi & drawable, Rect const rect)
            : gdi::ProtectedGraphics(drawable, rect)
            {}

            void refresh_rects(array_view<Rect const>) override {}
        } osd(drawable, rect);
        osd.draw(RDPMemBlt(0, bmp_rect, 0xCC, 0, 0, 0), bmp_rect.intersect(screen_rect.cx, screen_rect.cy), bmp);
        now.tv_sec++;
        consumer.do_snapshot(now);

        RED_CHECK_SIG(drawable, "\x04\xb7\xd8\x57\xf0\xde\x62\x8c\x42\x6f\x4d\x2a\x26\xc4\x68\xfc\xa1\xf5\x29\x9f");
    }

    trans.disconnect();

    RED_CHECK_EQUAL(3083, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(3628, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}

RED_AUTO_TEST_CASE(TestModOSD3)
{
    Rect screen_rect(0, 0, 800, 600);
    TestGraphic drawable(screen_rect.cx, screen_rect.cy);
    auto const color_cxt = gdi::ColorCtx::depth24();

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "./", "test3", ".png", groupid, ReportError{});

    class ImageCaptureLocal
    {
        Transport & trans;
        TestGraphic & drawable;
        timeval start_capture;
        TimestampTracer timestamp_tracer;

    public:
        ImageCaptureLocal(TestGraphic & drawable, Transport & trans)
        : trans(trans)
        , drawable(drawable)
        , timestamp_tracer(drawable.get_mutable_image_view())
        {
        }

        std::chrono::microseconds do_snapshot(const timeval & now)
        {
            this->drawable.trace_mouse();
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->timestamp_tracer.trace(ptm);
            this->dump24();
            this->trans.next();
            this->timestamp_tracer.clear();
            this->start_capture = now;
            this->drawable.clear_mouse();
            return std::chrono::microseconds::zero();
        }

        void dump24() const {
            ::dump_png24(this->trans, this->drawable, true);
        }
    };


    ImageCaptureLocal consumer(drawable, trans);

    drawable.show_mouse_cursor(false);

    drawable->draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), encode_color24()(RED)), screen_rect, color_cxt);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;
    now.tv_sec++;

    {
        Rect const rect = Rect(100, 100, 200, 200);
        drawable->draw(RDPOpaqueRect(rect, encode_color24()(GREEN)), screen_rect, color_cxt);

        now.tv_sec++;
        consumer.do_snapshot(now);

        RED_CHECK_SIG(drawable, "\x67\x3a\xb4\xb9\x9f\x7f\xe9\x47\xbb\x49\xd3\xf7\x03\xf1\x5c\x07\x80\xeb\x1f\x62");

        Bitmap const bmp = bitmap_from_file(FIXTURES_PATH "/ad8b.bmp");
        int const bmp_x = 200;
        int const bmp_y = 200;
        Rect const bmp_rect(bmp_x, bmp_y, bmp.cx(), bmp.cy());
        struct OSD : gdi::ProtectedGraphics
        {
            OSD(GraphicApi & drawable, Rect const rect)
            : gdi::ProtectedGraphics(drawable, rect)
            {}

            void refresh_rects(array_view<Rect const>) override {}
        } osd(drawable, rect);

        RDPBitmapData bmp_data;
        bmp_data.dest_left = bmp_rect.x;
        bmp_data.dest_top = bmp_rect.y;
        bmp_data.dest_right = bmp_data.dest_left + bmp_rect.cx - 1;
        bmp_data.dest_bottom = bmp_data.dest_top + bmp_rect.cy - 1;
        bmp_data.width = bmp.cx();
        bmp_data.height = bmp.cy();
        bmp_data.bits_per_pixel = safe_int(bmp.bpp());
        bmp_data.flags = 0;

        bmp_data.bitmap_length = bmp.bmp_size();
        osd.draw(bmp_data, bmp);
        now.tv_sec++;
        consumer.do_snapshot(now);

        RED_CHECK_SIG(drawable, "\x04\xb7\xd8\x57\xf0\xde\x62\x8c\x42\x6f\x4d\x2a\x26\xc4\x68\xfc\xa1\xf5\x29\x9f");
    }

    trans.disconnect();

    RED_CHECK_EQUAL(3083, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(3628, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}
