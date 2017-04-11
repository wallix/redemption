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

#include "capture/capture.hpp"
#include "utils/png.hpp"
#include "utils/drawable.hpp"
#include "utils/difftimeval.hpp"
#include "transport/transport.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/protected_graphics.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/fileutils.hpp"

RED_AUTO_TEST_CASE(TestModOSD)
{
    Rect screen_rect(0, 0, 800, 600);
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy);
    auto const color_cxt = gdi::ColorCtx::depth24();

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid, nullptr);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;

    class ImageCaptureLocal
    {
        Transport & trans;
        const Drawable & drawable;
        timeval start_capture;
        std::chrono::microseconds frame_interval;

    public:
        ImageCaptureLocal (
            const timeval & now, const Drawable & drawable, Transport & trans,
            std::chrono::microseconds png_interval)
        : trans(trans)
        , drawable(drawable)
        , start_capture(now)
        , frame_interval(png_interval)
        {}

        std::chrono::microseconds do_snapshot(const timeval & now) {
            using std::chrono::microseconds;
            uint64_t const duration = difftimeval(now, this->start_capture);
            uint64_t const interval = this->frame_interval.count();
            if (duration >= interval) {
                if (this->logical_frame_ended()
                    // Force snapshot if diff_time_val >= 1.5 x frame_interval.
                    || (duration >= interval * 3 / 2)) {
                    const_cast<Drawable&>(this->drawable).trace_mouse();
                    tm ptm;
                    localtime_r(&now.tv_sec, &ptm);
                    const_cast<Drawable&>(this->drawable).trace_timestamp(ptm);
                    this->flush();
                    this->trans.next();
                    const_cast<Drawable&>(this->drawable).clear_timestamp();
                    this->start_capture = now;
                    const_cast<Drawable&>(this->drawable).clear_mouse();

                    return microseconds(interval ? interval - duration % interval : 0u);
                }
                else {
                    // Wait 0.3 x frame_interval.
                    return this->frame_interval / 3;
                }
            }
            return microseconds(interval - duration);
        }

        void dump24() const {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }

    private:
        bool logical_frame_ended() const {
            return this->drawable.logical_frame_ended;
        }

        void flush() {
            this->dump24();
        }
    };


    ImageCaptureLocal consumer(now, drawable.impl(), trans, {});

    drawable.show_mouse_cursor(false);

    drawable.draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), RED), screen_rect, color_cxt);
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
        osd.draw(RDPOpaqueRect(Rect(100, 100, 200, 200), GREEN), screen_rect, color_cxt);
        now.tv_sec++;
        consumer.do_snapshot(now);
    }

    trans.disconnect();

    RED_CHECK_EQUAL(5021, ::filesize(trans.seqgen()->get(0)));
    RED_CHECK_EQUAL(5047, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}
