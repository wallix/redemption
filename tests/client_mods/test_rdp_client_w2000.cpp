/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to check back-end behavior stays identical
   when connecting to reference W2000 server (mocked up)
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdpClientW2000
#include <boost/test/auto_unit_test.hpp>
#include <errno.h>
#include <algorithm>

#define LOGNULL
// #define LOGPRINT
#include "test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "testtransport.hpp"
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

    ClientInfo info(1, true, true);
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    int verbose = 256;

    class Front : public FrontAPI {
        public:
        uint32_t verbose;
        const ClientInfo & info;
        CHANNELS::ChannelDefArray cl;
        uint8_t mod_bpp;
        BGRPalette mod_palette;


        virtual void flush()
        {
            if (verbose > 10){
                 LOG(LOG_INFO, "--------- FRONT ------------------------");
                 LOG(LOG_INFO, "flush()");
                 LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void draw(const RDPOpaqueRect& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            RDPOpaqueRect new_cmd24 = cmd;
            new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
            this->gd.draw(new_cmd24, clip);
        }
        virtual void draw(const RDPScrBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            this->gd.draw(cmd, clip);
        }
        virtual void draw(const RDPDestBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            this->gd.draw(cmd, clip);

        }
        virtual void draw(const RDPPatBlt& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            const BGRColor back_color24 = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
            const BGRColor fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
            RDPPatBlt new_cmd24 = cmd;
            new_cmd24.back_color = back_color24;
            new_cmd24.fore_color = fore_color24;
            this->gd.draw(new_cmd24, clip);
        }
        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            this->gd.draw(cmd, clip, bmp);
        }
        virtual void draw(const RDPMem3Blt& cmd, const Rect& clip, const Bitmap& bmp)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            this->gd.draw(cmd, clip, bmp);
        }
        virtual void draw(const RDPLineTo& cmd, const Rect& clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                cmd.log(LOG_INFO, clip);
                LOG(LOG_INFO, "========================================\n");
            }
            this->gd.draw(cmd, clip);

        }
        virtual void draw(const RDPGlyphIndex& cmd, const Rect& clip)
        {
            exit(0);
        }

        virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const CHANNELS::ChannelDef & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
        {
        }

        virtual void send_pointer(int cache_idx, uint8_t* data, uint8_t* mask, int x, int y) throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "send_pointer(cache_idx=%d, data=%p, mask=%p, x=%d, y=%d",
                    cache_idx, data, mask, x, y);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void send_global_palette() throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "send_global_palette()");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void set_pointer(int cache_idx) throw (Error)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "set_pointer");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void begin_update()
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "begin_update");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void end_update()
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "end_update");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "color_cache");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void set_mod_palette(const BGRPalette & palette)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "set_mod_palette");
                LOG(LOG_INFO, "========================================\n");
            }
//            this->mod_palette_setted = true;
            for (unsigned i = 0; i < 256 ; i++){
                this->mod_palette[i] = palette[i];
            }
//            this->palette_sent = false;
        }
        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_set_pointer");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void server_draw_text(int16_t x, int16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_draw_text %s", text);
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void text_metrics(const char * text, int & width, int & height)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "text_metrics");
                LOG(LOG_INFO, "========================================\n");
            }
        }
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

    const char * name = "RDP W2000 Target";

//    int client_sck = ip_connect("10.10.46.64", 3389, 3, 1000, verbose);
//    redemption::string error_message;
//    SocketTransport t( name
//                     , client_sck
//                     , "10.10.46.64"
//                     , 3389
//                     , verbose
//                     , &error_message
//                     );


    #include "fixtures/dump_w2000_mem3blt.hpp"
    TestTransport t(name, indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    // To always get the same client random, in tests
    LCGRandom gen(0);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }

    struct mod_rdp * mod = new mod_rdp( &t
                                      , "administrateur"
                                      , "SecureLinux$42"
                                      , "0.0.0.0"
                                      , front
                                      , "test"
                                      , false   /* tls                     */
                                      , info
                                      , &gen
                                      , 2
                                      , NULL
                                      , ""
                                      , ""      /* alternate_shell         */
                                      , ""      /* shell_working_directory */
                                      , true    /* clipbaord               */
                                      , false   /* fast-path support       */
                                      , true    /* mem3blt support         */
                                      , 0       /* verbose                 */
                                      , false); /* enable new pointer      */

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }

    BOOST_CHECK(t.get_status());
    BOOST_CHECK_EQUAL(mod->front_width, 800);
    BOOST_CHECK_EQUAL(mod->front_height, 600);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        LOG(LOG_INFO, "=======================> count=%u", count);

        if (count++ >= 25) break;
//        if (count == 10){
//            front.dump_png("trace_w2000_10_");
//        }
//        if (count == 20){
//            front.dump_png("trace_w2000_20_");
//        }
        res = mod->draw_event();
        BOOST_CHECK_EQUAL((BackEvent_t)BACK_EVENT_NONE, (BackEvent_t)res);
    }

//    front.dump_png("trace_w2000_");
}
