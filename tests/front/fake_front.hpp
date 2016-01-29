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

#include "log.hpp"
#include "front_api.hpp"
#include "channel_list.hpp"
#include "client_info.hpp"
#include "RDP/RDPDrawable.hpp"
#include "RDP/orders/RDPOrdersSecondaryBrushCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "gdi/graphic_cmd_color_converter.hpp"

//#include <openssl/ssl.h>

class DrawLogged {
    struct ColorDecoder {
        uint8_t    mod_bpp;
        uint32_t operator()(uint32_t c) const {
            return color_decode_opaquerect(c, this->mod_bpp, BGRPalette::classic_332());
        }
    };

    RDPDrawable & gd;
    gdi::CmdColorConverterProxy<ColorDecoder> converter;
    uint32_t verbose;

public:
    DrawLogged(RDPDrawable & gd, uint8_t mod_bpp, uint32_t verbose)
    : gd(gd)
    , converter{ColorDecoder{mod_bpp}}
    , verbose(verbose)
    {}

    template<class Api, class Cmd, class... Ts>
    void operator()(gdi::GraphicProxy::draw_tag tag, Api & api, Cmd const & cmd, Rect const & clip, Ts const & ... args) {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        this->converter(tag, this->gd, cmd, clip, args...);
    }

    template<class Api, class Cmd>
    void operator()(gdi::GraphicProxy::draw_tag tag, Api & api, Cmd const & cmd) {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        this->converter(tag, this->gd, cmd);
    }

    template<class Tag, class Api, class... Ts>
    void operator()(Tag tag, Api & api, Ts const & ... args) {
        gdi::GraphicProxy()(tag, this->gd, args...);
    }
};

using FrontWithDrawLog = gdi::GraphicAdapter<DrawLogged, FrontAPI>;

class FakeFront : public FrontWithDrawLog {
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

    void sync() override {
        if (this->verbose > 10) {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "sync()");
             LOG(LOG_INFO, "========================================\n");
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list(void) const override { return cl; }

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t length
                        , size_t chunk_size, int flags) override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_to_channel");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    void send_global_palette() override {
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_global_palette()");
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

    void set_palette(const BGRPalette & palette) override {
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

    int server_resize(int width, int height, int bpp) override {
        this->mod_bpp = bpp;
        this->info.bpp = bpp;
        if (this->verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return 1;
    }

    void dump_png(const char * prefix) {
        char tmpname[128];
        sprintf(tmpname, "%sXXXXXX.png", prefix);
        int fd = ::mkostemps(tmpname, 4, O_WRONLY | O_CREAT);
        FILE * f = fdopen(fd, "wb");
        ::dump_png24( f, this->gd.data(), this->gd.width(), this->gd.height()
                    , this->gd.rowsize(), false);
        ::fclose(f);
    }

    void save_to_png(const char * filename) {
        std::FILE * file = fopen(filename, "w+");
        dump_png24(file, this->gd.data(), this->gd.width(),
                   this->gd.height(), this->gd.rowsize(), true);
        fclose(file);
    }

    FakeFront(ClientInfo & info, uint32_t verbose)
    : FrontWithDrawLog(DrawLogged(this->gd, info.bpp, verbose), false, false)
    , verbose(verbose)
    , info(info)
    , mod_bpp(info.bpp)
    , mod_palette(BGRPalette::no_init())
    , mouse_x(0)
    , mouse_y(0)
    , notimestamp(true)
    , nomouse(true)
    , gd(info.width, info.height, 24) {
        if (this->mod_bpp == 8) {
            this->mod_palette = BGRPalette::classic_332();
        }
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
