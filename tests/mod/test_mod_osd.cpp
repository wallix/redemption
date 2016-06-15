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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestModOSD
#include "system/redemption_unit_tests.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "transport/out_filename_sequence_transport.hpp"
#include "capture/image_capture.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "mod/mod_osd.hpp"
#include "utils/bitmap_with_png.hpp"

struct FakeMod : mod_api
{
    FakeMod()
    : mod_api(0, 0)
    {}

    void draw_event(time_t now, gdi::GraphicApi & drawable) override {}
    void rdp_input_invalidate(const Rect& r) override {}
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {}
    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {}
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {}
    void send_to_front_channel(const char*const mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}

    void server_draw_text(Font const & font, int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
    {}
};

BOOST_AUTO_TEST_CASE(TestModOSD)
{
    Rect screen_rect(0, 0, 800, 600);
    FakeMod mod;
    RDPDrawable drawable(screen_rect.cx, screen_rect.cy, 24);

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;

    ImageCapture consumer(now, drawable.impl(), trans, {});

    drawable.show_mouse_cursor(false);

    bool ignore_frame_in_timeval = false;

    drawable.draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), RED), screen_rect);
    now.tv_sec++;

    {
        Bitmap_PNG const bmp(FIXTURES_PATH "/ad8b.bmp");
        int const bmp_x = 200;
        int const bmp_y = 200;
        Rect const bmp_rect(bmp_x, bmp_y, bmp.cx(), bmp.cy());
        Rect const rect = bmp_rect.intersect(screen_rect.cx, screen_rect.cy);
        drawable.draw(RDPMemBlt(0, bmp_rect, 0xCC, 0, 0, 0), rect, bmp);

        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

        struct OSD : ProtectGraphics
        {
            using ProtectGraphics::ProtectGraphics;
            void refresh_rects(array_view<Rect const>) {}
        } osd(drawable, rect);
        osd.draw(RDPOpaqueRect(Rect(100, 100, 200, 200), GREEN), screen_rect);
        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);
    }

    trans.disconnect();

    BOOST_CHECK_EQUAL(5021, ::filesize(trans.seqgen()->get(0)));
    BOOST_CHECK_EQUAL(5047, ::filesize(trans.seqgen()->get(1)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
}
