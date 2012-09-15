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
#define BOOST_TEST_MODULE TestRdpClientTLSW2008
#include <boost/test/auto_unit_test.hpp>
#include <errno.h>
#include <algorithm>

//#define LOGPRINT
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
#include "ssl_calls.hpp"
#include "png.hpp"
#include "RDP/RDPDrawable.hpp"
#include "staticcapture.hpp"


BOOST_AUTO_TEST_CASE(TestDecodePacket)
{

    ClientInfo info(1, 1, true, true);
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1024;
    info.height = 768;
    int verbose = 256;


    class Front : public FrontAPI {
        public:
        uint32_t verbose;
        const ClientInfo & info;
        ChannelDefArray cl;
        uint8_t mod_bpp;
        BGRPalette mod_palette;

        virtual void flush()
        {
        }
        virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
        {
            RDPOpaqueRect new_cmd24 = cmd;
            new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
            this->gd.draw(new_cmd24, clip);
        }
        virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
        {
            this->gd.draw(cmd, clip);
        }
        virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
        {
            this->gd.draw(cmd, clip);
        }
        virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
        {
            RDPPatBlt new_cmd24 = cmd;
            new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
            new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
            this->gd.draw(new_cmd24, clip);
        }

        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bitmap)
        {
            this->gd.draw(cmd, clip, bitmap);
        }

        virtual void draw(const RDPLineTo& cmd, const Rect& clip)
        {
            RDPLineTo new_cmd24 = cmd;
            new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
            new_cmd24.pen.color = color_decode_opaquerect(cmd.pen.color, this->mod_bpp, this->mod_palette);
            this->gd.draw(new_cmd24, clip);

        }
        virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
        {
            exit(0);
        }

        virtual const ChannelDefArray & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const ChannelDef & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
        {}

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
        {}

        virtual void send_global_palette() throw (Error)
        {}

        virtual void set_pointer(int cache_idx) throw (Error)
        {}

        virtual void begin_update()
        {}

        virtual void end_update()
        {}

        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
        {}

        virtual void set_mod_palette(const BGRPalette & palette)
        {}

        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
        {}

        virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
        {}
        virtual void text_metrics(const char * text, int & width, int & height)
        {}

        virtual int server_resize(int width, int height, int bpp)
        {
            this->mod_bpp = bpp;
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
                LOG(LOG_INFO, "========================================\n");
            }
            return 0;
        }

        int mouse_x;
        int mouse_y;
        bool notimestamp;
        bool nomouse;

        BGRPalette palette;
        RDPDrawable gd;

        void dump_png(const char * prefix)
        {
            char tmpname[128];
            sprintf(tmpname, "%sXXXXXX.png", prefix);
            int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
            FILE * f = fdopen(fd, "wb");
            ::dump_png24(f, this->gd.drawable.data, this->gd.drawable.width, this->gd.drawable.height, this->gd.drawable.rowsize);
            ::fclose(f);
        }

        Front(const ClientInfo & info, uint32_t verbose) :
              FrontAPI(false, false),
              verbose(verbose),
              info(info),
              mouse_x(0),
              mouse_y(0),
              notimestamp(true),
              nomouse(true),
              gd(info.width, info.height, true)
            {

            }

    } front(info, verbose);

    const char * name = "RDP W2008 TLS Target";

//    ClientSocketTransport t(name, "10.10.14.88", 3389, 3, 1000, verbose);
//    t.connect();

    #include "./fixtures/dump_TLSw2008.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    // To always get the same client random, in tests
    LCGRandom gen(0);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }
    const bool tls = true;
    struct client_mod * mod = new mod_rdp(&t, "administrateur@qa", "S3cur3!1nux", front, "195-132-203-212", tls, info, &gen, 511);

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
    BOOST_CHECK(t.status);

    BOOST_CHECK_EQUAL(mod->front_width, 1024);
    BOOST_CHECK_EQUAL(mod->front_height, 768);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "===================> count = %u", count);
        if (count++ >= 40) break;
        res = mod->draw_event();
        BOOST_CHECK_EQUAL((BackEvent_t)BACK_EVENT_NONE, (BackEvent_t)res);
    }

//    front.dump_png("trace_w2008_tls_");

}
