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

   Fake Front class for Unit Testing
*/

#include "utils/log.hpp"
#include "core/front_api.hpp"
#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "gdi/graphic_api.hpp"

//#include <openssl/ssl.h>

class FakeFront : public FrontAPI
{
public:
    uint32_t                    verbose;
    ClientInfo                & info;
    CHANNELS::ChannelDefArray   cl;
    uint8_t                     mod_bpp;
    BGRPalette                  mod_palette;

    int mouse_x;
    int mouse_y;

    bool notimestamp;
    bool nomouse;

    RDPDrawable gd;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd);}

    void draw(RDPDestBlt          const & cmd, Rect clip) override {this->draw_impl(cmd, clip);}

    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override {this->draw_impl(cmd, clip);}

    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPScrBlt           const & cmd, Rect clip) override {
        this->draw_impl(cmd, clip);
    }

    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override {
        this->draw_impl(cmd, clip);
    }

    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(cmd, clip, color_ctx);
    }

    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override {
        this->draw_impl(cmd, bmp);
    }

    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override {
        this->draw_impl(cmd, clip, bmp);
    }

    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        this->draw_impl(cmd, clip, color_ctx, bmp);
    }

    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override {
        this->draw_impl(cmd, clip, color_ctx, gly_cache);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        this->draw_impl(cmd);
    }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::CachedIcon                     & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(RDPColCache   const & cmd) override {
        this->draw_impl(cmd);
    }

    void draw(RDPBrushCache const & cmd) override {
        this->draw_impl(cmd);
    }

private:
    void draw_impl(const RDPBitmapData & cmd, const Bitmap & bmp) {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, bmp);
    }

    template<class Cmd, class... Ts>
    void draw_impl(Cmd const & cmd, Rect clip, Ts const & ... args) {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd, clip, args...);
    }

    template<class Cmd, class... Ts>
    void draw_impl(Cmd const & cmd) {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.draw(cmd);
    }

public:
    bool can_be_start_capture() override { return false; }
    bool must_be_stop_capture() override { return false; }

    void set_palette(const BGRPalette &) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_palette");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void set_pointer(const Pointer & cursor) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }

        this->gd.set_pointer(cursor);
    }

    void sync() override {
        if (this->verbose > 10) {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "sync()");
             LOG(LOG_INFO, "========================================\n");
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list(void) const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef &, uint8_t const * /*data*/, size_t /*length*/
                        , size_t /*chunk_size*/, int /*flags*/) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void begin_update() override {
        //if (this->verbose > 10) {
        //    LOG(LOG_INFO, "--------- FRONT ------------------------");
        //    LOG(LOG_INFO, "begin_update");
        //    LOG(LOG_INFO, "========================================\n");
        //}
    }

    void end_update() override {
        //if (this->verbose > 10) {
        //    LOG(LOG_INFO, "--------- FRONT ------------------------");
        //    LOG(LOG_INFO, "end_update");
        //    LOG(LOG_INFO, "========================================\n");
        //}
    }

    ResizeResult server_resize(int width, int height, int bpp) override {
        this->mod_bpp = bpp;
        this->info.bpp = bpp;
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return ResizeResult::done;
    }

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

    FakeFront(ClientInfo & info, uint32_t verbose)
    : verbose(verbose)
    , info(info)
    , mod_bpp(info.bpp)
    , mod_palette(BGRPalette::classic_332())
    , mouse_x(0)
    , mouse_y(0)
    , notimestamp(true)
    , nomouse(true)
    , gd(info.width, info.height)
    {
        // -------- Start of system wide SSL_Ctx option ------------------------------

        // ERR_load_crypto_strings() registers the error strings for all libcrypto
        // functions. SSL_load_error_strings() does the same, but also registers the
        // libssl error strings.

        // One of these functions should be called before generating textual error
        // messages. However, this is not required when memory usage is an issue.

        // ERR_free_strings() frees all previously loaded error strings.

        //SSL_load_error_strings();

        // SSL_library_init() registers the available SSL/TLS ciphers and digests.
        // OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms
        // for SSL_library_init().

        // - SSL_library_init() must be called before any other action takes place.
        // - SSL_library_init() is not reentrant.
        // - SSL_library_init() always returns "1", so it is safe to discard the return
        // value.

        // Note: OpenSSL 0.9.8o and 1.0.0a and later added SHA2 algorithms to
        // SSL_library_init(). Applications which need to use SHA2 in earlier versions
        // of OpenSSL should call OpenSSL_add_all_algorithms() as well.

        //SSL_library_init();
    }

    void update_pointer_position(uint16_t, uint16_t) override {}
};
