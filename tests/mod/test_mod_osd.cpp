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
#include <boost/test/auto_unit_test.hpp>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINT

#include "transport/out_filename_sequence_transport.hpp"
#include "staticcapture.hpp"
#include "RDP/RDPDrawable.hpp"
#include "mod_osd.hpp"

struct FakeMod : gdi::GraphicProxy<FakeMod, mod_api>
{
    RDPDrawable gd;

    FakeMod(const uint16_t front_width, const uint16_t front_height, Font const & font)
    : FakeMod::base_type(front_width, front_height)
    , gd(front_width, front_height, 24)
    {}

    void draw_event(time_t now) override {}
    void rdp_input_invalidate(const Rect& r) override {}
    void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap) override {}
    void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap) override {}
    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override {}
    void end_update() override {}
    void send_to_front_channel(const char*const mod_channel_name, uint8_t const * data, size_t length, size_t chunk_size, int flags) override {}
    void begin_update() override {}
    void text_metrics(Font const & font, const char * text, int & width, int & height) override {}

    void server_draw_text(Font const & font, int16_t x, int16_t y, const char * text,
                                  uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) override
    {}

protected:
    friend gdi::GraphicCoreAccess;

    RDPDrawable & get_gd_proxy_impl() {
        return this->gd;
    }
};

BOOST_AUTO_TEST_CASE(TestModOSD)
{
    Inifile ini;

    Rect screen_rect(0, 0, 800, 600);
    FakeMod mod(screen_rect.cx, screen_rect.cy, ini.get<cfg::font>());
    RDPDrawable & drawable = mod.gd;

    const int groupid = 0;
    OutFilenameSequenceTransport trans(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test", ".png", groupid);

    timeval now;
    now.tv_sec = 1350998222;
    now.tv_usec = 0;

    ini.set<cfg::video::rt_display>(1);
    ini.set<cfg::video::png_limit>(-1);
    ini.set<cfg::video::png_interval>(0);
    StaticCapture consumer(now, trans, trans.seqgen(), screen_rect.cx, screen_rect.cy, false, ini, drawable.impl());

    drawable.show_mouse_cursor(false);

    bool ignore_frame_in_timeval = false;

    drawable.draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), RED), screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    {
#ifndef FIXTURES_PATH
# define FIXTURES_PATH "."
#endif
        mod_osd osd(mod, Bitmap(FIXTURES_PATH "/ad8b.bmp"), 200, 200);

        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

        RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), GREEN);
        osd.draw(cmd1, screen_rect);
        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);
    }

    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), BLUE);
    drawable.draw(cmd1, screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval);

    trans.disconnect();

    BOOST_CHECK_EQUAL(5021, ::filesize(trans.seqgen()->get(1)));
    BOOST_CHECK_EQUAL(5047, ::filesize(trans.seqgen()->get(2)));
    BOOST_CHECK_EQUAL(5054, ::filesize(trans.seqgen()->get(3)));
    ::unlink(trans.seqgen()->get(0));
    ::unlink(trans.seqgen()->get(1));
    ::unlink(trans.seqgen()->get(2));
    ::unlink(trans.seqgen()->get(3));
    ::unlink(trans.seqgen()->get(4));
}
