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
#define BOOST_TEST_MODULE TestRdpClientW2008
#include <boost/test/auto_unit_test.hpp>
#include <errno.h>
#include <algorithm>

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
#include "ssl_calls.hpp"
#include "png.hpp"
#include "RDP/RDPDrawable.hpp"
#include "staticcapture.hpp"


BOOST_AUTO_TEST_CASE(TestDecodePacket)
{

    ClientInfo info(1, 1, true, true);
    info.keylayout = 0x04C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    int verbose = 1;


    class Front : public FrontAPI {
        public:
        uint32_t verbose;
        const ClientInfo & info;
        ChannelList cl;
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

        void draw_tile(const Rect & dst_tile, const Rect & src_tile, const RDPMemBlt & cmd, const Bitmap & bitmap, const Rect & clip)
        {
            // No need to resize bitmap
            if (src_tile == Rect(0, 0, bitmap.cx, bitmap.cy)){
                const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
                this->gd.draw(cmd2, clip, bitmap);
            }
            else {
                const Bitmap tiled_bmp(bitmap, src_tile);
                const RDPMemBlt cmd2(0, dst_tile, cmd.rop, 0, 0, 0);
                this->gd.draw(cmd2, clip, tiled_bmp);
            }
        }


        virtual void draw(const RDPMemBlt& cmd, const Rect& clip, const Bitmap& bitmap)
        {
            if (bitmap.cx < cmd.srcx || bitmap.cy < cmd.srcy){
                return;
            }

            this->send_global_palette();

            // if not we have to split it
            const uint16_t TILE_CX = 32;
            const uint16_t TILE_CY = 32;

            const uint16_t dst_x = cmd.rect.x;
            const uint16_t dst_y = cmd.rect.y;
            // clip dst as it can be larger than source bitmap
            const uint16_t dst_cx = std::min<uint16_t>(bitmap.cx - cmd.srcx, cmd.rect.cx);
            const uint16_t dst_cy = std::min<uint16_t>(bitmap.cy - cmd.srcy, cmd.rect.cy);

            // check if target bitmap can be fully stored inside one front cache entry
            // if so no need to tile it.
            uint32_t front_bitmap_size = ::nbbytes(this->info.bpp) * align4(dst_cx) * dst_cy;
            // even if cache seems to be large enough, cache entries cant be used
            // for values whose width is larger or equal to 256 after alignment
            // hence, we check for this case. There does not seem to exist any
            // similar restriction on cy actual reason of this is unclear
            // (I don't even know if it's related to redemption code or client code).
    //        LOG(LOG_INFO, "cache1=%u cache2=%u cache3=%u bmp_size==%u",
    //            this->client_info.cache1_size,
    //            this->client_info.cache2_size,
    //            this->client_info.cache3_size,
    //            front_bitmap_size);
            if (front_bitmap_size <= this->info.cache3_size
                && align4(dst_cx) < 256 && dst_cy < 256){
                // clip dst as it can be larger than source bitmap
                const Rect dst_tile(dst_x, dst_y, dst_cx, dst_cy);
                const Rect src_tile(cmd.srcx, cmd.srcy, dst_cx, dst_cy);
                this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
            }
            else {
                for (int y = 0; y < dst_cy ; y += TILE_CY) {
                    int cy = std::min(TILE_CY, (uint16_t)(dst_cy - y));

                    for (int x = 0; x < dst_cx ; x += TILE_CX) {
                        int cx = std::min(TILE_CX, (uint16_t)(dst_cx - x));

                        const Rect dst_tile(dst_x + x, dst_y + y, cx, cy);
                        const Rect src_tile(cmd.srcx + x, cmd.srcy + y, cx, cy);
                        this->draw_tile(dst_tile, src_tile, cmd, bitmap, clip);
                    }
                }
            }
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

        virtual const ChannelList & get_channel_list(void) const { return cl; }
        virtual void send_to_channel(const McsChannelItem & channel, uint8_t* data, size_t length, size_t chunk_size, int flags)
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
//            exit(0);
        }
        virtual void set_mod_palette(const BGRPalette & palette)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "set_mod_palette");
                LOG(LOG_INFO, "========================================\n");
            }
            exit(0);
        }
        virtual void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
        {
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_set_pointer");
                LOG(LOG_INFO, "========================================\n");
            }
        }
        virtual void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor, const Rect & clip)
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
            if (verbose > 10){
                LOG(LOG_INFO, "--------- FRONT ------------------------");
                LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
                LOG(LOG_INFO, "========================================\n");
            }
            return 0;
        }
        virtual void set_mod_bpp(uint8_t bpp)
        {
            this->mod_bpp = bpp;
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

//    const char * name = "RDP W2008 Target";
//    int sck = connect("10.10.14.78", 3389, name);
//    SocketTransport t(name, sck, verbose);

    #include "./fixtures/dump_w2008.hpp"
    TestTransport t("test_rdp_client_w2008", indata, sizeof(indata), outdata, sizeof(outdata), verbose);

    // To always get the same client random, in tests
    LCGRandom gen(0);

    if (verbose > 2){
        LOG(LOG_INFO, "--------- CREATION OF MOD ------------------------");
    }
    struct client_mod * mod = new mod_rdp(&t, "administrateur@qa", "S3cur3!1nux", front, "test", info, &gen);

    if (verbose > 2){
        LOG(LOG_INFO, "========= CREATION OF MOD DONE ====================\n\n");
    }
//    BOOST_CHECK(t.status);

    BOOST_CHECK_EQUAL(mod->front_width, 800);
    BOOST_CHECK_EQUAL(mod->front_height, 600);

    uint32_t count = 0;
    BackEvent_t res = BACK_EVENT_NONE;
    while (res == BACK_EVENT_NONE){
        if (count++ >= 50) break;
        printf("count=%d\n", count);
        res = mod->draw_event();
        BOOST_CHECK_EQUAL((BackEvent_t)BACK_EVENT_NONE, (BackEvent_t)res);
    }

    front.dump_png("trace_w2008_");

}
