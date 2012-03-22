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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdpClientTestCard
#include <boost/test/auto_unit_test.hpp>


#include "RDP/RDPGraphicDevice.hpp"
#include "channel_list.hpp"
#include "cache.hpp"
#include "front_api.hpp"
#include "wait_obj.hpp"

#include "internal/test_card.hpp"

BOOST_AUTO_TEST_CASE(TestShowTestCard)
{
    wait_obj back_event(-1);
//     const uint16_t width = 800;
//     const uint16_t height = 600;
//    class Front : public FrontAPI {
//        public:
//            Front() : FrontAPI() {}
//            virtual void flush() {}
//            virtual void draw(const RDPOpaqueRect&, const Rect&) {}
//            virtual void draw(const RDPScrBlt&, const Rect&) {}
//            virtual void draw(const RDPDestBlt&, const Rect&) {}
//            virtual void draw(const RDPPatBlt&, const Rect&) {}
//            virtual void draw(const RDPMemBlt&, const Rect&, const Bitmap&) {}
//            virtual void draw(const RDPLineTo&, const Rect&) {}
//            virtual void draw(const RDPGlyphIndex&, const Rect&) {}

//            virtual int get_front_console_session() const {return 0;}
//            virtual int get_front_brush_cache_code() const { return 0;}
//            virtual const ChannelList& get_channel_list() const {}
//            virtual void send_to_channel(const McsChannelItem&, uint8_t*, size_t, size_t, int) {}
//            virtual void send_pointer(int, uint8_t*, uint8_t*, int, int) {}
//            virtual void send_global_palette() {}
//            virtual void set_pointer(int) {}
//            virtual void begin_update() {}
//            virtual void end_update() {}
//            virtual void color_cache(const BGRColor (&)[256], uint8_t) {}
//            virtual void set_mod_palette(const BGRColor (&)[256]) {}
//            virtual void server_set_pointer(int, int, uint8_t*, uint8_t*) {}
//            virtual void server_draw_text(uint16_t, uint16_t, const char*, uint32_t, uint32_t, const Rect&) {}
//            virtual void text_metrics(const char*, int&, int&) {}
//            virtual void init_mod() {}
//            virtual int server_resize(int, int, int) { return 0;}
//            virtual void set_mod_bpp(uint8_t) {}
//            virtual void set_mod_bpp_to_front_bpp() {}
//    } front;
//    test_card_mod mod(back_event, front, width, height);


}
