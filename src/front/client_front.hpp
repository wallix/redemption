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

#include "utils/log.hpp"

#include "core/front_api.hpp"
#include "core/client_info.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/channel_list.hpp"
#include "gdi/graphic_cmd_color.hpp"

class ClientFront : public FrontAPI
{
    bool verbose;
    ClientInfo &info;
    gdi::Depth                  mod_bpp;
    BGRPalette                  mod_palette;
    RDPDrawable gd;
    CHANNELS::ChannelDefArray   cl;

public:
    ClientFront(bool notimestamp, bool nomouse, ClientInfo & info, bool verbose)
    : FrontAPI(notimestamp, nomouse)
    , verbose(verbose)
    , info(info)
    , mod_bpp(gdi::Depth::from_bpp(info.bpp))
    , mod_palette(BGRPalette::classic_332())
    , gd(info.width, info.height)
    {}

    ClientFront(ClientInfo & info, bool verbose)
    : ClientFront(false, false, info, verbose)
    {}

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

        this->draw_impl(cmd, clip, color_ctx);
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

        this->draw_impl(cmd, clip, color_ctx);
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

        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPGlyphIndex const & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx, gly_cache);
    }

    void draw(RDPPolygonSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPPolygonCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPPolyline const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPEllipseSC const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPEllipseCB const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->verbose) {
            LOG(LOG_INFO, "--------- ClientFront ------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->draw_impl(cmd, clip, color_ctx);
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
        this->mod_bpp = gdi::Depth::from_bpp(bpp);
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
            LOG(LOG_INFO, "set_pointer");
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

    void update_pointer_position(uint16_t, uint16_t) override {}

private:
    template<class Cmd, class... Ts>
    void draw_impl(Cmd const & cmd, Rect clip, gdi::ColorCtx color_ctx, Ts const & ... args)
    {
        if (color_ctx.depth() == this->mod_bpp) {
            this->gd.draw(cmd, clip, color_ctx, args...);
        }
        else {
            Cmd new_cmd = cmd;
            auto re_encoder = [&](RDPColor c){ return color_encode(color_decode(c, color_ctx), this->mod_bpp); };
            gdi::GraphicCmdColor::encode_cmd_color(re_encoder, new_cmd);
            this->gd.draw(new_cmd, clip, gdi::ColorCtx(this->mod_bpp, this->mod_palette), args...);
        }
    }
};
