/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2015
 *
 * free RDP client main program
 *
 */

#include <iostream>
#include <string>

#define LOGPRINT
#include "log.hpp"

#include "front_api.hpp"
//#include "RDP/x224.hpp"
#include "client_info.hpp"
#include "transport/socket_transport.hpp"
#include "utils/socket_transport_utility.hpp"
#include "RDP/RDPDrawable.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "wait_obj.hpp"
#include "mod_api.hpp"
#include "redirection_info.hpp"
#include "channel_list.hpp"
#include "rdp/rdp_params.hpp"
#include "genrandom.hpp"
#include "rdp/rdp.hpp"
#include "program_options/program_options.hpp"

namespace po = program_options;
//using namespace std;

class ClientFront : public FrontAPI {

    public:
    uint32_t verbose;
    ClientInfo &info;
    uint8_t                     mod_bpp;
    BGRPalette                  mod_palette;
    RDPDrawable gd;
    CHANNELS::ChannelDefArray   cl;


    void flush() {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "flush()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDPOpaqueRect & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPOpaqueRect new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPScrBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPPatBlt & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, bitmap);
    }

    void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, bitmap);
    }

    void draw(const RDPLineTo & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPLineTo new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPGlyphIndex new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolygonCB & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolygonCB new_cmd24 = cmd;
        new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
        new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolyline & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseSC & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseCB & cmd, const Rect & clip) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPEllipseCB new_cmd24 = cmd;
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPColCache   & cmd) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDP::FrameMarker & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            bitmap_data.log(LOG_INFO, "ClientFront");
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(bitmap_data, bmp);

    }

    void set_palette(const BGRPalette&) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "set_palette()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    int server_resize(int width, int height, int bpp) override {
        this->mod_bpp = bpp;
        this->info.bpp = bpp;
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return 1;
    }

    void set_pointer(const Pointer & cursor) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.set_pointer(cursor);
    }

    void begin_update() override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "begin_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void end_update() override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "end_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }


    // reutiliser le FakeFront
    // creer un main calqué sur celui de transparent.cpp et reussir a lancer un mod_rdp
    const CHANNELS::ChannelDefArray & get_channel_list(void) const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef & channel, const uint8_t * data, std::size_t length
                        , std::size_t chunk_size, int flags) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    int getVerbose() { return this->verbose; }

    ClientFront(ClientInfo & info, uint32_t verbose)
    : FrontAPI(false, false), verbose(verbose), info(info), mod_bpp(info.bpp), mod_palette(BGRPalette::no_init()), gd(info.width, info.height, 24) {
        if (this->mod_bpp == 8) {
            this->mod_palette = BGRPalette::classic_332();
        }

        this->verbose = verbose;

        SSL_library_init();
    }

    void update_pointer_position(uint16_t, uint16_t) override {}
};

void run_mod(mod_api & mod, ClientFront & front, wait_obj & front_event, SocketTransport * st_mod, SocketTransport * st_front);

int main(int argc, char** argv)
{
    RedirectionInfo redir_info;
    int verbose = 0;
    std::string target_device = "10.10.47.205";
    int target_port = 3389;
    int nbretry = 3;
    int retry_delai_ms = 1000;

    std::string username = "administrateur";
    std::string password = "SecureLinux";
    ClientInfo client_info;

    client_info.width = 800;
    client_info.height = 600;
    client_info.bpp = 32;

    /* Program options */
    po::options_description desc({
        {'h', "help","produce help message"},
        {'t', "target-device", &target_device, "target device"},
        {'u', "username", &username, "username"},
        {'p', "password", &password, "password"},
        {"verbose", &verbose, "verbose"},
    });

    auto options = po::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout <<
            "\n"
            "ReDemPtion Stand alone RDP Client.\n"
            "Copyright (C) Wallix 2010-2015.\n"
            "\n"
            "Usage: rdptproxy [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }


    ClientFront front(client_info, verbose);
    ModRDPParams mod_rdp_params( username.c_str()
                               , password.c_str()
                               , target_device.c_str()
                               , "0.0.0.0"   // client ip is silenced
                               , /*front.keymap.key_flags*/ 0
                               , verbose);

    if (verbose > 128) {
        mod_rdp_params.log();
    }

    /* SocketTransport mod_trans */
    int client_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms, verbose);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device.c_str(), target_port, verbose, nullptr);


    wait_obj front_event;

    /* Random */
    LCGRandom gen(0);

    /* mod_api */
    mod_rdp mod( mod_trans, front, client_info, redir_info, gen, mod_rdp_params);

    run_mod(mod, front, front_event, &mod_trans, nullptr);

    return 0;
}



void run_mod(mod_api & mod, ClientFront & front, wait_obj & front_event, SocketTransport * st_mod, SocketTransport * st_front) {
    struct      timeval time_mark = { 0, 50000 };
    bool        run_session       = true;

    while (run_session) {
        try {
            unsigned max = 0;
            fd_set   rfds;
            fd_set   wfds;

            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            struct timeval timeout = time_mark;

            add_to_fd_set(mod.get_event(), st_mod, rfds, max, timeout);

            if (is_set(mod.get_event(), st_mod, rfds)) {
                timeout.tv_sec  = 2;
                timeout.tv_usec = 0;
            }

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

            LOG(LOG_INFO, "RDP CLIENT :: select num = %d\n", num);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                LOG(LOG_INFO, "RDP CLIENT :: errno = %d\n", errno);
                break;
            }

            if (is_set(mod.get_event(), st_mod, rfds)) {
                LOG(LOG_INFO, "RDP CLIENT :: draw_event");
                mod.draw_event(time(nullptr));
            }

        } catch (Error & e) {
            LOG(LOG_ERR, "RDP CLIENT :: Exception raised = %d!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
    return;
}
