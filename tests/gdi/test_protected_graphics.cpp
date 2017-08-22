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
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include <memory>

#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/difftimeval.hpp"
#include "transport/transport.hpp"
#include "transport/out_filename_sequence_transport.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/protected_graphics.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/fileutils.hpp"
#include "utils/timestamp_tracer.hpp"

RED_AUTO_TEST_CASE(TestModOSD)
{
    Rect screen_rect(0, 0, 800, 600);
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    auto const color_cxt = gdi::ColorCtx::depth24();

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid, ReportError{});

    class ImageCaptureLocal
    {
        Transport & trans;
        const Drawable & drawable;
        timeval start_capture;

        TimestampTracer timestamp_tracer;

    public:
        ImageCaptureLocal(const Drawable & drawable, Transport & trans)
        : trans(trans)
        , drawable(drawable)
        , timestamp_tracer(drawable.width(), drawable.height(), drawable.Bpp,
              const_cast<Drawable&>(drawable).first_pixel(), drawable.rowsize())
        {}

        std::chrono::microseconds do_snapshot(const timeval & now)
        {
            const_cast<Drawable&>(this->drawable).trace_mouse();
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->timestamp_tracer.trace(ptm);
            this->dump24();
            this->trans.next();
            this->timestamp_tracer.clear();
            this->start_capture = now;
            const_cast<Drawable&>(this->drawable).clear_mouse();
            return std::chrono::microseconds::zero();
        }

        void dump24() const {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }
    };


    ImageCaptureLocal consumer(drawable.impl(), trans);

    drawable.show_mouse_cursor(false);

    drawable.draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), encode_color24()(RED)), screen_rect, color_cxt);

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
        drawable.draw(RDPMemBlt(0, bmp_rect, 0xCC, 0, 0, 0), rect, bmp);

        now.tv_sec++;
        consumer.do_snapshot(now);

        struct OSD : gdi::ProtectedGraphics
        {
            OSD(GraphicApi & drawable, Rect const rect)
            : gdi::ProtectedGraphics(drawable, rect)
            {}

            void refresh_rects(array_view<Rect const>) override {}
        } osd(drawable, rect);
        osd.draw(RDPOpaqueRect(Rect(100, 100, 200, 200), encode_color24()(GREEN)), screen_rect, color_cxt);
        now.tv_sec++;
        consumer.do_snapshot(now);
    }

    trans.disconnect();

    RED_CHECK_EQUAL(5021, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(5047, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}
