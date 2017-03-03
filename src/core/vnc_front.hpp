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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean

   VNC Front class for Unit Testing
*/

#include "utils/log.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "gdi/graphic_api.hpp"

//#include <openssl/ssl.h>

class VncFront : public FrontAPI
{
public:
    bool verbose;
    ClientInfo &info;
    uint8_t                     mod_bpp;
    BGRPalette                  mod_palette;
    RDPDrawable gd;
    CHANNELS::ChannelDefArray   cl;
    int height = 0;
    int width = 0;


    bool can_be_start_capture() override { return false; }
    bool must_be_stop_capture() override { return false; }

    void flush() {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "flush()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(RDPOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPOpaqueRect new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(RDPMultiOpaqueRect const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx);
    }

    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip);
    }

    void draw(RDPPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, bitmap);
    }

    void draw(RDPMem3Blt const & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, color_ctx, bitmap);
    }

    void draw(RDPLineTo const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip, color_ctx);
    }

    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDPBrushCache & cmd) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void draw(const RDP::FrameMarker & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::WindowIcon & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::CachedIcon & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::DeletedWindow & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop & order) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(order);
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            bitmap_data.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(bitmap_data, bmp);

    }

    void set_palette(const BGRPalette&) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "set_palette()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    ResizeResult server_resize(int width, int height, int bpp) override {
        this->height = height;
        this->width = width;
        this->mod_bpp = bpp;
        this->info.bpp = bpp;

        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return ResizeResult::no_need;
    }

    void set_pointer(const Pointer & cursor) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.set_pointer(cursor);
    }

    void begin_update() override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "begin_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void end_update() override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- VncFront ------------------");
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
            LOG(LOG_INFO, "--------- VncFront ------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    VncFront(bool param1, bool param2, ClientInfo & info, uint32_t verbose)
    : FrontAPI(param1, param2)
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


    void dump_png(const char * prefix) {
        char tmpname[128];
        sprintf(tmpname, "%sXXXXXX.png", prefix);
        int fd = ::mkostemps(tmpname, 4, O_WRONLY | O_CREAT);
        FILE * f = fdopen(fd, "wb");
        ::dump_png24( f, this->gd.data(), this->gd.width(), this->gd.height()
                    , this->gd.rowsize(), true);
        ::fclose(f);
    }

    void save_to_png(const char * filename) {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.data(), this->gd.width(),
                   this->gd.height(), this->gd.rowsize(), true);
        fclose(file);
    }


};
