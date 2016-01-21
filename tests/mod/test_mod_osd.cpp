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
#include "../front/fake_front.hpp"
#include "mod_osd.hpp"

struct FakeMod : mod_api
{
    RDPDrawable gd;

    FakeMod(const uint16_t front_width, const uint16_t front_height, Font const & font)
    : mod_api(front_width, front_height)
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

    using mod_api::draw;

    void draw(const RDPOpaqueRect      & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPScrBlt          & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPDestBlt         & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPMultiDstBlt     & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDP::RDPMultiPatBlt& cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDP::RDPMultiScrBlt& cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPPatBlt          & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPMemBlt          & cmd, const Rect & clip, const Bitmap & bmp) override
    { this->gd.draw(cmd, clip, bmp); }
    void draw(const RDPMem3Blt         & cmd, const Rect & clip, const Bitmap & bmp) override
    { this->gd.draw(cmd, clip, bmp); }
    void draw(const RDPLineTo          & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPGlyphIndex      & cmd, const Rect & clip, const GlyphCache * gly_cache)
    { this->gd.draw(cmd, clip, gly_cache); }
    void draw(const RDPPolygonSC       & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPPolygonCB       & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPPolyline        & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPEllipseSC       & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDPEllipseCB       & cmd, const Rect & clip) override { this->gd.draw(cmd, clip); }
    void draw(const RDP::FrameMarker   & order)                  override { this->gd.draw(order);     }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override { this->gd.draw(order); }
    void draw(const RDP::RAIL::WindowIcon          & order) override { this->gd.draw(order); }
    void draw(const RDP::RAIL::CachedIcon          & order) override { this->gd.draw(order); }
    void draw(const RDP::RAIL::DeletedWindow       & order) override { this->gd.draw(order); }

    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
        size_t size, const Bitmap & bmp) override {
        this->gd.draw(bitmap_data, data, size, bmp);
    }

    void server_set_pointer(const Pointer & cursor) override { this->gd.server_set_pointer(cursor); }
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
    bool requested_to_stop       = false;

    drawable.draw(RDPOpaqueRect(Rect(0, 0, screen_rect.cx, screen_rect.cy), RED), screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval, requested_to_stop);

    {
#ifndef FIXTURES_PATH
# define FIXTURES_PATH "."
#endif
        ClientInfo info;
        info.width = 1;
        info.height = 1;
        FakeFront front(info, 0);
        mod_osd osd(front, mod, Bitmap(FIXTURES_PATH "/ad8b.bmp"), 200, 200);

        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval, requested_to_stop);

        RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), GREEN);
        osd.draw(cmd1, screen_rect);
        now.tv_sec++;
        consumer.snapshot(now, 10, 10, ignore_frame_in_timeval, requested_to_stop);
    }

    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval, requested_to_stop);

    RDPOpaqueRect cmd1(Rect(100, 100, 200, 200), BLUE);
    drawable.draw(cmd1, screen_rect);
    now.tv_sec++;
    consumer.snapshot(now, 10, 10, ignore_frame_in_timeval, requested_to_stop);

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
