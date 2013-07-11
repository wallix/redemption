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
#define BOOST_TEST_MODULE TestAuthentifier
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "client_info.hpp"
#include "front_api.hpp"
#include "keymap2.hpp"
#include "RDP/RDPDrawable.hpp"

class Front : public FrontAPI {
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

    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {
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

    Front(const ClientInfo & info, uint32_t verbose)
        : capture_state(CAPTURE_STATE_UNKNOWN)
        , FrontAPI(false, false)
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

#include "authentifier.hpp"
#include "testtransport.hpp"

/*
BOOST_AUTO_TEST_CASE(TestAuthentifierKeepAlive)
{
    // test keep_alive functions
    // keep alive start
    // keep_alive_start should send to the Transport a message asking for keep alive value.
    BStream stream(1024);

    stream.out_uint32_be(strlen(STRAUTHID_KEEPALIVE "\nASK\n"));
    stream.out_copy_bytes(STRAUTHID_KEEPALIVE "\nASK\n", strlen(STRAUTHID_KEEPALIVE "\nASK\n"));

    CheckTransport keepalivetrans((char *)stream.get_data(), stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, keepalivetrans, 10000, 10010, 30, 30, true, 0);
    long keepalivetime = 0;

    //    BOOST_CHECK(keepalivetrans.get_status());
    //    try{
    //        sesman.start_keep_alive(keepalivetime);
    //    } catch (const Error & e){
    //        BOOST_CHECK(false);
    //    };
    //    BOOST_CHECK(keepalivetrans.get_status());
    stream.reset();
}

BOOST_AUTO_TEST_CASE(TestAuthentifierAuthChannel)
{
    BStream stream(1024);

    // TEST ask_auth_channel_target
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n",
                          strlen(STRAUTHID_AUTHCHANNEL_TARGET "\n!TEST_TARGET\n"));
    CheckTransport auth_channel_trans((char *)stream.get_data(), stream.get_offset());
    Inifile ini;
    SessionManager sesman(&ini, auth_channel_trans, 10000, 10010, 30, 30, true, 0);

    BOOST_CHECK(auth_channel_trans.get_status());
    try{
        sesman.ask_auth_channel_target("TEST_TARGET");
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(auth_channel_trans.get_status());
    stream.reset();

    // TEST set_auth_channel_result
    stream.out_uint32_be(strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
    stream.out_copy_bytes(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n",
                          strlen(STRAUTHID_AUTHCHANNEL_RESULT "\n!TEST_RESULT\n"));
    CheckTransport auth_channel_trans2((char *)stream.get_data(), stream.get_offset());
    Inifile ini2;
    SessionManager sesman2(&ini2, auth_channel_trans2, 10000, 10010, 30, 30, true, 0);

    BOOST_CHECK(auth_channel_trans2.get_status());
    try{
        sesman2.set_auth_channel_result("TEST_RESULT");
    } catch (const Error & e){
        BOOST_CHECK(false);
    };
    BOOST_CHECK(auth_channel_trans2.get_status());
}

BOOST_AUTO_TEST_CASE(TestAuthentifierGetMod)
{
    // test get mod from protocol
    LogTransport get_mod_trans;
    Inifile ini;
    SessionManager sesman(&ini, get_mod_trans, 10000, 10010, 30, 30, true, 4);
    int res;

    // no known protocol on target device yet (should be an error case)
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_EXIT), res);


    // auto test case
    ini.context_set_value(AUTHID_TARGET_DEVICE,"autotest");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK_EQUAL(static_cast<int>(MODULE_INTERNAL_TEST), res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"");

    // RDP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "RDP");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_RDP == res);

    // VNC protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "VNC");
    sesman.connected = false;
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_VNC == res);

    // XUP protocol on target
    ini.context_set_value(AUTHID_TARGET_PROTOCOL, "XUP");
    sesman.connected = false;
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_XUP == res);

    // INTERNAL STATUS
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,"INTERNAL");
    ini.context_set_value(AUTHID_TARGET_DEVICE,"selector");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_SELECTOR == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"bouncer2");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_BOUNCER2 == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"rwl");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"close");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_close");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_CLOSE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_dialog");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_DIALOG == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_message");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_MESSAGE == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"widget2_rwl_login");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_WIDGET2_RWL_LOGIN == res);

    ini.context_set_value(AUTHID_TARGET_DEVICE,"card");
    res = sesman.get_mod_from_protocol();
    BOOST_CHECK(MODULE_INTERNAL_CARD == res);
}
*/

BOOST_AUTO_TEST_CASE(TestAuthentifier)
{
    Inifile ini;

    ini.globals.keepalive_grace_delay = 2;

    int client_sck = ip_connect( ini.globals.authip
                               , ini.globals.authport
                               , 30
                               , 1000
                               , ini.debug.auth
                               );
    BOOST_CHECK(client_sck != -1);

    if (client_sck != -1) {
        SocketTransport t( "Authentifier"
                         , client_sck
                         , ini.globals.authip
                         , ini.globals.authport
                         , ini.debug.auth
                         );
        wait_obj auth_event(t.sck);

        time_t start_time = time(NULL);

        SessionManager acl( &ini
                          , t
                          , start_time // proxy start time
                          , start_time // acl start time
                          , ini.globals.keepalive_grace_delay
                          , ini.globals.max_tick
                          , ini.globals.internal_domain
                          , ini.debug.auth);
        acl.signal = BACK_EVENT_NEXT;

        ClientInfo info(1, true, true);

        info.keylayout             = 0x04C;
        info.console_session       = 0;
        info.brush_cache_code      = 0;
        info.bpp                   = 24;
        info.width                 = 800;
        info.height                = 600;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;

        int verbose = 511;

        Front         front(info, verbose);
        ModuleManager mm(front, ini);

        struct timeval time_mark = { 0, 50000 };
        bool   run               = true;

        while (run) {
            unsigned max = 0;

            fd_set rfds;
            fd_set wfds;

            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            struct timeval timeout = time_mark;

            auth_event.add_to_fd_set(rfds, max);

            int num = select(max + 1, &rfds, &wfds, 0, &timeout);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }

            if (auth_event.is_set(rfds)) {
                acl.receive();
            }

            time_t now = time(NULL);

            if (!acl.check(front, mm, now, t) ||
                acl.lost_acl) {
                break;
            }
        }
    }
}
