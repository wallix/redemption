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
#include "utils/log.hpp"

#include "core/front_api.hpp"
#include "core/client_info.hpp"
#include "transport/socket_transport.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/wait_obj.hpp"
#include "mod/mod_api.hpp"
#include "utils/redirection_info.hpp"
#include "core/channel_list.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "utils/genrandom.hpp"
#include "mod/rdp/rdp.hpp"
#include "program_options/program_options.hpp"

namespace po = program_options;
//using namespace std;

class ClientFront : public FrontAPI
{
    bool verbose;
    ClientInfo &info;
    uint8_t                     mod_bpp;
    BGRPalette                  mod_palette;
    RDPDrawable gd;
    CHANNELS::ChannelDefArray   cl;

public:
    bool can_be_start_capture() override { return false; }
    bool must_be_stop_capture() override { return false; }

    void flush() {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "flush()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPOpaqueRect new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx);
    }

    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, bitmap);
    }

    void draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx, bitmap);
    }

    void draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPLineTo new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPGlyphIndex new_cmd24 = cmd;
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx, gly_cache);
    }

    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolygonCB new_cmd24 = cmd;
        new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
        new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPEllipseCB new_cmd24 = cmd;
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(const RDPColCache   & cmd) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDP::FrameMarker & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            bitmap_data.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(bitmap_data, bmp);

    }

    void set_palette(const BGRPalette&) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "set_palette()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    ResizeResult server_resize(int width, int height, int bpp) override {
        this->mod_bpp = bpp;
        this->info.bpp = bpp;
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return ResizeResult::done;
    }

    void set_pointer(const Pointer & cursor) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "server_set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.set_pointer(cursor);
    }

    void begin_update() override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "begin_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void end_update() override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "end_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list() const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef &, const uint8_t * data, std::size_t length
                        , std::size_t chunk_size, int flags) override {
        (void)data;
        (void)length;
        (void)chunk_size;
        (void)flags;
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    ClientFront(ClientInfo & info, bool verbose)
    : FrontAPI(false, false)
    , verbose(verbose)
    , info(info)
    , mod_bpp(info.bpp)
    , mod_palette(BGRPalette::no_init())
    , gd(info.width, info.height)
    {
        if (this->mod_bpp == 8) {
            this->mod_palette = BGRPalette::classic_332();
        }

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
            "Usage: rdpproxy [options]\n\n"
            << desc << std::endl
        ;
        return 0;
    }


    Inifile ini;
    ClientFront front(client_info, verbose > 10);
    ModRDPParams mod_rdp_params( username.c_str()
                               , password.c_str()
                               , target_device.c_str()
                               , "0.0.0.0"   // client ip is silenced
                               , /*front.keymap.key_flags*/ 0
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , to_verbose_flags(verbose));

    if (verbose > 128) {
        mod_rdp_params.log();
    }

    /* SocketTransport mod_trans */
    int client_sck = ip_connect(target_device.c_str(), target_port, nbretry, retry_delai_ms);
    SocketTransport mod_trans( "RDP Server", client_sck, target_device.c_str(), target_port, to_verbose_flags(verbose), nullptr);


    wait_obj front_event;

    /* Random */
    UdevRandom gen;
    TimeSystem timeobj;

    NullAuthentifier authentifier;

    /* mod_api */
    mod_rdp mod( mod_trans, front, client_info, redir_info, gen, timeobj, mod_rdp_params, authentifier);

    run_mod(mod, front, front_event, &mod_trans, nullptr);

    return 0;
}



void run_mod(mod_api & mod, ClientFront & front, wait_obj &, SocketTransport * st_mod, SocketTransport *) {
    struct      timeval time_mark = { 0, 50000 };
    bool        run_session       = true;

    while (run_session) {
        try {
            unsigned max = 0;
            fd_set   rfds;
            fd_set   wfds;

            io_fd_zero(rfds);
            io_fd_zero(wfds);
            struct timeval timeout = time_mark;

            mod.get_event().wait_on_fd(st_mod?st_mod->sck:INVALID_SOCKET, rfds, max, timeout);

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
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

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                LOG(LOG_INFO, "RDP CLIENT :: draw_event");
                mod.draw_event(time(nullptr), front);
            }

        } catch (Error & e) {
            LOG(LOG_ERR, "RDP CLIENT :: Exception raised = %d!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
    return;
}
