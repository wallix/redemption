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
#define BOOST_TEST_MODULE TestRdpClientSimple
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "./test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "constants.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/sec.hpp"
#include "wait_obj.hpp"
#include "RDP/RDPGraphicDevice.hpp"
#include "channel_list.hpp"
#include "front_api.hpp"
#include "client_info.hpp"
#include "rdp/rdp.hpp"


BOOST_AUTO_TEST_CASE(TestDecodePacket)
{

    ClientInfo info(1, 1, true, true);
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;

    class Front : public FrontAPI {
        public:
        const ClientInfo & info;
        ChannelList cl;

        virtual void flush()
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "flush()");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPLineTo& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
             LOG(LOG_INFO, "========================================\n");
        }

        virtual const ChannelList & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
        {
        }

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_pointer(cache_idx=%d, data=%p, mask=%p, x=%d, y=%d",
                cache_idx, data, mask, x, y);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void send_global_palette() throw (Error)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_global_palette()");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void set_pointer(int cache_idx) throw (Error)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_pointer");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void begin_update()
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "begin_update");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void end_update()
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "end_update");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "color_cache");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void set_mod_palette(const BGRPalette & palette)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_mod_palette");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_set_pointer");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_draw_text %s", text);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void text_metrics(const char * text, int & width, int & height)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "text_metrics");
             LOG(LOG_INFO, "========================================\n");
        }
        virtual int server_resize(int width, int height, int bpp)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
             LOG(LOG_INFO, "========================================\n");
             return 0;
        }
        virtual void set_mod_bpp(uint8_t bpp)
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "set_mod_bpp(bpp=%d)", bpp);
             LOG(LOG_INFO, "========================================\n");
        }
        virtual void set_mod_bpp_to_front_bpp()
        {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "set_mod_bpp_to_front()");
             LOG(LOG_INFO, "========================================\n");
        }

        int mouse_x;
        int mouse_y;
        bool notimestamp;
        bool nomouse;

        Front(const ClientInfo & info) :
              FrontAPI(false, false),
              info(info),
              mouse_x(0),
              mouse_y(0),
              notimestamp(true),
              nomouse(true)
            {}

    } front(info);

    Stream stream(65536);
    const char * name = "RDP Target";
    int sck = connect("10.10.14.78", 3389, name);
    int verbose = 0;
    SocketTransport t(name, sck, verbose);
    wait_obj back_event(t.sck);

    LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    struct client_mod * mod = new mod_rdp(&t,
                        back_event,
                        "administrateur@qa",
                        "S3cur3!1nux",
                        front,
                        "laptop",
                        info);
    LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");

    BOOST_CHECK_EQUAL(mod->front_width, 800);
    BOOST_CHECK_EQUAL(mod->front_height, 600);

    BackEvent_t res = mod->draw_event();
    if (res != BACK_EVENT_NONE){
        LOG(LOG_INFO, "Creation of new mod 'RDP' failed\n");
        throw Error(ERR_SESSION_UNKNOWN_BACKEND);
    }
    res = mod->draw_event();
    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();
//    res = mod->draw_event();

}
