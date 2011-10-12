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

*/

#if !defined(__SESSION_HPP__)
#define __SESSION_HPP__

#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>

#include "colors.hpp"
#include "stream.hpp"
#include "constants.hpp"
#include "ssl_calls.hpp"
#include "file_loc.hpp"
#include "rect.hpp"
#include "client_info.hpp"

#include "config.hpp"
#include "wait_obj.hpp"
#include "transport.hpp"
#include "bitmap.hpp"
#include "modcontext.hpp"

#include "keymap.hpp"
#include "sesman.hpp"
#include "front.hpp"
#include "../mod/null/null.hpp"


using namespace std;

static ProtocolKeyword KeywordsDefinitions[] = {
    {STRAUTHID_OPT_MOVIE_PATH, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_ERROR_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_HOST, TYPE_TEXT, "!"},
    {STRAUTHID_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_DEVICE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PORT, TYPE_INTEGER, "!3389"},
    {STRAUTHID_TARGET_PROTOCOL, TYPE_TEXT, "!RDP"},
    {STRAUTHID_END_TIME, TYPE_TEXT, "!-"},
    {STRAUTHID_SELECTOR_GROUP_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_DEVICE_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_LINES_PER_PAGE, TYPE_TEXT, "!20"},
    {STRAUTHID_SELECTOR_NUMBER_OF_PAGES, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_CURRENT_PAGE, TYPE_TEXT, "!1"},
    {STRAUTHID_OPT_MOVIE, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_OPT_CLIPBOARD, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_OPT_DEVICEREDIRECTION, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_DISPLAY_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_ACCEPT_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_OPT_WIDTH, TYPE_INTEGER, "!800"},
    {STRAUTHID_OPT_HEIGHT, TYPE_INTEGER, "!600"},
    {STRAUTHID_OPT_BPP, TYPE_INTEGER, "!24"},
    {STRAUTHID_PROXY_TYPE, TYPE_TEXT, "!RDP"},
    {STRAUTHID_AUTHENTICATED, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_SELECTOR, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_KEEPALIVE, TYPE_BOOLEAN, "ASK"},
    {STRAUTHID_END_DATE_CNX, TYPE_INTEGER, "!0"},
    {STRAUTHID_OPT_BITRATE, TYPE_INTEGER, "!40000"},
    {STRAUTHID_OPT_FRAMERATE, TYPE_INTEGER, "!5"},
    {STRAUTHID_OPT_QSCALE, TYPE_INTEGER, "!15"},
    {STRAUTHID_OPT_CODEC_ID, TYPE_TEXT, "!flv"},
    {STRAUTHID_REJECTED, TYPE_TEXT, "!Connection refused by authentifier."},
    // password or AuthenticationInteractive
    {"authentication_challenge", TYPE_TEXT, "!password"},
    // Translation
    {STRAUTHID_TRANS_BUTTON_OK, TYPE_TEXT, "!OK"},
    {STRAUTHID_TRANS_BUTTON_CANCEL, TYPE_TEXT, "!Cancel"},
    {STRAUTHID_TRANS_BUTTON_HELP, TYPE_TEXT, "!Help"},
    {STRAUTHID_TRANS_BUTTON_CLOSE, TYPE_TEXT, "!Close"},
    {STRAUTHID_TRANS_BUTTON_REFUSED, TYPE_TEXT, "!Refused"},
    {STRAUTHID_TRANS_LOGIN, TYPE_TEXT, "!login"},
    {STRAUTHID_TRANS_USERNAME, TYPE_TEXT, "!username"},
    {STRAUTHID_TRANS_PASSWORD, TYPE_TEXT, "!password"},
    {STRAUTHID_TRANS_TARGET, TYPE_TEXT, "!target"},
    {STRAUTHID_TRANS_DIAGNOSTIC, TYPE_TEXT, "!diagnostic"},
    {STRAUTHID_TRANS_CONNECTION_CLOSED, TYPE_TEXT, "!Connection closed"},
    {STRAUTHID_TRANS_HELP_MESSAGE, TYPE_TEXT, "!Help message"},
    {STRAUTHID_MODE_CONSOLE, TYPE_TEXT, "!allow"},
    {STRAUTHID_VIDEO_QUALITY, TYPE_TEXT, "!medium"},
    {STRAUTHID_TIMEZONE, TYPE_INTEGER, "!-3600"},
};

enum {
    // before anything else : exchange of credentials
    SESSION_STATE_RSA_KEY_HANDSHAKE,
    // initial state no module loaded, init not done
    SESSION_STATE_ENTRY,
    // no module loaded
    // init_done
    // login window destoyed if necessary
    // user clicked on OK to run module  or provided connection info on cmd line
    // but did not received credentials yet
    SESSION_STATE_WAITING_FOR_NEXT_MODULE,
    // init_done, module loaded and running
    SESSION_STATE_RUNNING,
    // display dialog when connection is closed
    SESSION_STATE_CLOSE_CONNECTION,
    // disconnect session
    SESSION_STATE_STOP,
};

static inline int get_pixel(uint8_t* data, int x, int y, int width, int bpp)
{
    int pixels_per_byte = 8/bpp;
    int real_width = (width + pixels_per_byte-1) / pixels_per_byte;
    int start = y * real_width + x / pixels_per_byte;
    int shift = x & (pixels_per_byte-1);

    #warning this need some cleanup, but we should define unit tests before correcting it, because mistaking is easy in these kind of things.
    if (bpp == 1) {
        return (data[start] & (0x80 >> shift)) != 0;
    } else if (bpp == 4) {
        if (shift == 0) {
            return (data[start] & 0xf0) >> 4;
        } else {
            return data[start] & 0x0f;
        }
    }
    return 0;
}


static inline int load_pointer(const char* file_name, uint8_t* data, uint8_t* mask, int* x, int* y)
{
    int rv = 0;

    try {
        if (access(file_name, F_OK)){
            LOG(LOG_WARNING, "pointer file [%s] does not exist\n", file_name);
            throw 1;
        }
        Stream stream(8192);
        int fd = open(file_name, O_RDONLY);
        if (fd < 1) {
            LOG(LOG_WARNING, "loading pointer from file [%s] failed\n", file_name);
            throw 1;
        }
        int lg = read(fd, stream.data, 8192);
        if (!lg){
            throw 1;
        }
        close(fd);

        #warning : the ways we do it now we have some risk of reading out of buffer (data that are not from file)

        stream.skip_uint8(6);
        int w = stream.in_uint8();
        int h = stream.in_uint8();
        stream.skip_uint8(2);
        *x = stream.in_uint16_le();
        *y = stream.in_uint16_le();
        stream.skip_uint8(22);
        int bpp = stream.in_uint8();
        stream.skip_uint8(25);

        BGRPalette palette;
        if (w == 32 && h == 32) {
            if (bpp == 1) {
                memcpy(palette, stream.in_uint8p(8), 8);
                // read next 32x32 bytes
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.skip_uint8(128);
            } else if (bpp == 4) {
                memcpy(palette, stream.in_uint8p(64), 64);
                for (int i = 0; i < 32; i++) {
                    for (int j = 0; j < 32; j++) {
            #warning probably bogus, we are in case bpp = 4 and we call get_pixel with 1 as bpp
                        int pixel = palette[get_pixel(stream.p, j, i, 32, 1)];
                        *data = pixel;
                        data++;
                        *data = pixel >> 8;
                        data++;
                        *data = pixel >> 16;
                        data++;
                    }
                }
                stream.skip_uint8(512);
            }
            memcpy(mask, stream.p, 128); /* mask */
        }
    }
    catch(...){
        rv = 1;
    }
    return rv;
}

struct Session {

    ModContext * context;
    int internal_state;
    long id;
    struct SocketTransport * trans;
    time_t keep_alive_time;

    int sck;
    wait_obj * front_event;
    wait_obj * back_event;

    Inifile * ini;

    struct client_mod * mod; /* module interface */
    struct client_mod * no_mod;

    struct Front* front;
    int mouse_x;
    int mouse_y;

    struct Keymap * keymap;

    SessionManager * sesman;

    Session(int sck, const char * ip_source, Inifile * ini)
    {
        this->context = new ModContext(
                KeywordsDefinitions,
                sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword));
        this->sesman = new SessionManager(*this->context);
        this->sesman->auth_trans_t = 0;

        this->mod = 0;

        this->internal_state = SESSION_STATE_RSA_KEY_HANDSHAKE;
        this->ini = ini;
        this->sck = sck;
        this->front_event = new wait_obj(sck);

        /* create these when up and running */
        this->trans = new SocketTransport(sck);

        /* set non blocking */
        int rv = 0;
    //    rv = fcntl(this->sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);
    //    if (rv < 0){
    //        /* 1 session_main_loop fnctl socket error */
    //        throw 1;
    //    }
        int nodelay = 1;

        /* SOL_TCP IPPROTO_TCP */
        rv = setsockopt(this->sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
        if (rv < 0){
            /* 2 session_main_loop fnctl socket error */
            throw 2;
        }

        this->front = new Front(this->trans, ini, this->keymap);
        this->no_mod = new null_mod(*this->context, *(this->front));
        this->mod = this->no_mod;

        this->context->cpy(STRAUTHID_HOST, ip_source);

        /* module interface */
        this->back_event = 0;
        this->keymap = 0;
        this->keep_alive_time = 0;
    }


    ~Session()
    {
        if (this->keymap){
            delete this->keymap;
        }
        delete this->front;
        delete this->front_event;
        delete this->trans;
        if (this->back_event){
            delete this->back_event;
        }
        if (this->mod != this->no_mod){
            delete this->mod;
            this->mod = this->no_mod;
        }
        delete this->no_mod;
        delete this->sesman;
        delete this->context;
    }

    int pointer(char* data, char* mask, int x, int y);
    void rdp_input_invalidate(const Rect & rect);

    int session_main_loop();

    int step_STATE_KEY_HANDSHAKE(const struct timeval & time);
    int step_STATE_ENTRY(const struct timeval & time);
    int step_STATE_WAITING_FOR_NEXT_MODULE(const struct timeval & time);
    int step_STATE_RUNNING(const struct timeval & time);
    int step_STATE_CLOSE_CONNECTION();

    bool session_setup_mod(int next_state, const ModContext * context);


};

#endif
