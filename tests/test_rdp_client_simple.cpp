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

        virtual void flush() {}
        virtual void draw(const RDPOpaqueRect&, const Rect&){}
        virtual void draw(const RDPScrBlt&, const Rect&){}
        virtual void draw(const RDPDestBlt&, const Rect&){}
        virtual void draw(const RDPPatBlt&, const Rect&){}
        virtual void draw(const RDPMemBlt&, const Rect&, const Bitmap&){}
        virtual void draw(const RDPLineTo&, const Rect&){}
        virtual void draw(const RDPGlyphIndex&, const Rect&){}

        virtual const ChannelList & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, size_t chunk_size, int flags) {}

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error) {}
        virtual void send_global_palette() throw (Error) {}
        virtual void set_pointer(int cache_idx) throw (Error) {}
        virtual void begin_update() {}
        virtual void end_update() {}
        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex) {}
        virtual void set_mod_palette(const BGRPalette & palette) {}
        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask) {}
        virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip) {}
        virtual void text_metrics(const char * text, int & width, int & height) {}
        virtual int server_resize(int width, int height, int bpp) { return 0; }
        virtual void set_mod_bpp(uint8_t bpp) {}
        virtual void set_mod_bpp_to_front_bpp() {}

        int mouse_x;
        int mouse_y;
        bool notimestamp;
        bool nomouse;

        Front(const ClientInfo & info) :
              FrontAPI(false, false),
              info(info)
            {}

    } front(info);


    Stream stream(65536);
    const char * name = "RDP Target";
    int sck = connect("10.10.14.78", 3389, name);
    int verbose = 0;
    SocketTransport t(name, sck, verbose);
    wait_obj back_event(t.sck);
    struct client_mod * mod = new mod_rdp(&t,
                        back_event,
                        "administrateur@qa",
                        "S3cur3!1nux",
                        front,
                        "laptop",
                        info);

    BOOST_CHECK_EQUAL(mod->front_width, 800);
    BOOST_CHECK_EQUAL(mod->front_height, 600);

    BackEvent_t res = mod->draw_event();
    if (res != BACK_EVENT_NONE){
        LOG(LOG_INFO, "Creation of new mod 'RDP' failed\n");
        throw Error(ERR_SESSION_UNKNOWN_BACKEND);
    }
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();
    res = mod->draw_event();

}
