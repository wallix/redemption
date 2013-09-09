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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestModuleManager
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "module_manager.hpp"
#include "counttransport.hpp"
#include "testtransport.hpp"

#include "client_info.hpp"
#include "front_api.hpp"
#include "keymap2.hpp"
#include "RDP/RDPDrawable.hpp"
class FakeFront : public FrontAPI {
public:
    enum {
          CAPTURE_STATE_UNKNOWN
        , CAPTURE_STATE_STARTED
        , CAPTURE_STATE_PAUSED
        , CAPTURE_STATE_STOPED
    } capture_state;

    uint32_t                    verbose;
    const ClientInfo          & client_info;
    CHANNELS::ChannelDefArray   cl;
    uint8_t                     mod_bpp;
    BGRPalette                  mod_palette;
    int                         mouse_x;
    int                         mouse_y;
    bool                        notimestamp;
    bool                        nomouse;

    BGRPalette  palette;
    RDPDrawable gd;

    Keymap2 keymap;

    virtual void flush() {
        if (verbose > 10) {
             LOG(LOG_INFO, "--------- FRONT ------------------------");
             LOG(LOG_INFO, "flush()");
             LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        RDPOpaqueRect new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
        this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDPScrBlt & cmd, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPDestBlt & cmd, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPPatBlt & cmd, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        const BGRColor back_color24 = color_decode_opaquerect( cmd.back_color, this->mod_bpp
                                                             , this->mod_palette);
        const BGRColor fore_color24 = color_decode_opaquerect( cmd.fore_color, this->mod_bpp
                                                             , this->mod_palette);
        RDPPatBlt new_cmd24 = cmd;
        new_cmd24.back_color = back_color24;
        new_cmd24.fore_color = fore_color24;
        this->gd.draw(new_cmd24, clip);
    }

    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bmp) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        this->gd.draw(cmd, clip, bmp);
    }

    virtual void draw(const RDPLineTo & cmd, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        this->gd.draw(cmd, clip);
    }

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache * gly_cache) {
        exit(0);
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const {
        return cl;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t * data
                                , size_t length, size_t chunk_size, int flags) {
    }

    virtual void send_pointer( int cache_idx, uint8_t * data, uint8_t * mask
                             , int x, int y) throw(Error) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG( LOG_INFO, "send_pointer(cache_idx=%d, data=%p, mask=%p, x=%d, y=%d"
               , cache_idx, data, mask, x, y);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void send_global_palette() throw(Error) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "send_global_palette()");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void set_pointer(int cache_idx) throw(Error) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void begin_update() {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "begin_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void end_update() {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "end_update");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "color_cache");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void set_mod_palette(const BGRPalette & palette) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "set_mod_palette");
            LOG(LOG_INFO, "========================================\n");
        }

        for (unsigned i = 0; i < 256 ; i++) {
            this->mod_palette[i] = palette[i];
        }
    }

    virtual void server_set_pointer(int x, int y, uint8_t * data, uint8_t * mask) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_set_pointer");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void server_draw_text( int16_t x, int16_t y, const char * text, uint32_t fgcolor
                                 , uint32_t bgcolor, const Rect & clip) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_draw_text %s", text);
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual void text_metrics(const char * text, int & width, int & height) {
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "text_metrics");
            LOG(LOG_INFO, "========================================\n");
        }
    }

    virtual int server_resize(int width, int height, int bpp) {
        this->mod_bpp = bpp;
        if (verbose > 10) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }
        return 0;
    }

    void dump_png(const char * prefix) {
        char tmpname[128];
        sprintf(tmpname, "%sXXXXXX.png", prefix);
        int fd = ::mkostemps(tmpname, 4, O_WRONLY | O_CREAT);
        FILE * f = fdopen(fd, "wb");
        ::dump_png24( f, this->gd.drawable.data, this->gd.drawable.width, this->gd.drawable.height
                    , this->gd.drawable.rowsize, true);
        ::fclose(f);
    }

    void init_pointers() {}

    FakeFront(const ClientInfo & info, uint32_t verbose)
        : FrontAPI(false, false)
        , capture_state(CAPTURE_STATE_UNKNOWN)
        , verbose(verbose)
        , client_info(info)
        , mouse_x(0)
        , mouse_y(0)
        , notimestamp(true)
        , nomouse(true)
        , gd(info.width, info.height) {
    }

    void start_capture(int width, int height, Inifile & ini) {
        this->capture_state = CAPTURE_STATE_STARTED;
    }

    void pause_capture() {
        if (this->capture_state != CAPTURE_STATE_STARTED) {
            return;
        }

        this->capture_state = CAPTURE_STATE_PAUSED;
    }

    void resume_capture() {
        if (this->capture_state != CAPTURE_STATE_PAUSED) {
            return;
        }

        this->capture_state = CAPTURE_STATE_STARTED;
    }
};

BOOST_AUTO_TEST_CASE(TestModuleManagerNextMod)
{
    Inifile ini;

    MMIni mm(ini);
    int res;

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");

    ini.context.selector.set(true);
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_SELECTOR);

    ini.globals.target_user.ask();
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_WIDGET2_LOGIN);

    ini.globals.auth_user.set_from_cstr("user");
    ini.context.password.set_from_cstr("securepassword");
    ini.globals.target_device.set_from_cstr("redjenkins-vbox@127.0.0.1 internal@display_message replay@autotest");
    ini.globals.target_user.set_from_cstr("rdp internal internal");
    ini.context.selector.set(false);
    ini.context.display_message.ask();

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE);

    ini.context.display_message.set_from_cstr("message");
    ini.context.accept_message.ask();
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_DIALOG_VALID_MESSAGE);
    ini.context.accept_message.set_from_cstr("message");

    res = mm.next_module();
    BOOST_CHECK(res == MODULE_INTERNAL_CLOSE);

    ini.context.authenticated.set(true);
    ini.context.target_protocol.set_from_cstr("RDP");
    res = mm.next_module();
    BOOST_CHECK(res == MODULE_RDP);

}

BOOST_AUTO_TEST_CASE(TestModuleManagerGetMod)
{
    // test get mod from protocol
    Inifile ini;
    ini.globals.keepalive_grace_delay = 30;
    ini.globals.max_tick = 30;
    ini.globals.internal_domain = true;
    ini.debug.auth = 4;

    MMIni mm(ini);
    int res;

    // no known protocol on target device yet (should be an error case)
    res = mm.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_EXIT), res);


    // auto test case
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_INTERNAL_TEST), res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    // RDP protocol on target
    // ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    ini.context.target_protocol.set_from_cstr("RDP");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_RDP == res);

    // VNC protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_VNC == res);

    // XUP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    mm.connected = false;
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_XUP == res);

    // INTERNAL STATUS
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,"INTERNAL");
    ini.context_set_value(AUTHID_TARGET_DEVICE,"selector");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_SELECTOR == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"bouncer2");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_BOUNCER2 == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl_login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"close");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_close");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_dialog");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_DIALOG == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_message");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_MESSAGE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl_login");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"card");
    res = mm.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CARD == res);
}
