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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Session object, in charge of session

*/

#include "colors.hpp"
#include "rect.hpp"
#include "../mod/login/login.hpp"
#include "../mod/null/null.hpp"
#include "../mod/rdp/rdp.hpp"
#include "../mod/vnc/vnc.hpp"
#include "../mod/xup/xup.hpp"
#include "../mod/close/close.hpp"
#include "../mod/dialog/dialog.hpp"
#include "../mod/transitory/transitory.hpp"
#include "../mod/bouncer/bouncer.hpp"
#include "../mod/cli/cli_mod.hpp"

#include "session.hpp"

#include <unistd.h>
#include <stdio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/time.h>

#include "region.hpp"
#include "log.hpp"
#include "client_mod.hpp"
#include "widget.hpp"
#include "transport.hpp"
#include "config.hpp"
#include "front.hpp"
#include "orders.hpp"

#include "error.hpp"
#include "wait_obj.hpp"
#include "constants.hpp"
#include "cache.hpp"

#define DEV_REDIRECTION_ENABLE true

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
    {STRAUTHID_AUTHENTICATED, TYPE_BOOLEAN, "!True"},
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


Session::Session(int sck, const char * ip_source, wait_obj * terminated_event, Inifile * ini) {
    this->context = new ModContext(
            KeywordsDefinitions,
            sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword));
    this->sesman = new SessionManager(*this->context);
    this->sesman->auth_trans_t = 0;

    this->mod = 0;

    this->internal_state = SESSION_STATE_RSA_KEY_HANDSHAKE;
    this->self_term_event = 0;
    this->ini = ini;
    this->sck = sck;
    this->client_event = new wait_obj(sck);

    /* create these when up and running */
    this->terminated_event = terminated_event;

    char event_name[256];
    snprintf(event_name, 255, "redemption_%8.8x_session_self_term_event_%p", getpid(), this);
    this->self_term_event = new wait_obj(event_name);

    snprintf(event_name, 255, "redemption_%8.8x_session_accept_event_%p", getpid(), this);
    this->accept_event = new wait_obj(event_name);
    this->trans = new SocketTransport(sck, g_is_term);
    this->session_callback = new SessionCallback(*this);
    this->server = new server_rdp(*this->session_callback, this->trans, ini);
    this->orders = new RDP::Orders(this->server);
    this->default_font = new Font(SHARE_PATH "/" DEFAULT_FONT_NAME);
    this->cache = new Cache(this->orders);
    this->server_stream.init(8192*2);

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

    /* keyboard info */
    memset(this->keys, 0, 256 * sizeof(int)); /* key states 0 up 1 down*/
    #warning we should be able to move all key management code to some object
    // scrool_lock = 1, num_lock = 2, caps_lock = 4
    this->key_flags = 0;

    #warning no need to create the front here the window size is not yet available
    this->front = new Front(this->orders, this->cache,
        this->default_font,
        this->ini->globals.nomouse,
        this->ini->globals.notimestamp,
        atoi(this->context->get(STRAUTHID_TIMEZONE))
        );
    this->no_mod = new null_mod(this->keys, this->key_flags, this->keymap, *this->context, *(this->front));
    this->mod = this->no_mod;

    this->context->cpy(STRAUTHID_HOST, ip_source);

    /* module interface */
    this->mod_event = 0;
    this->keymap = 0;
    this->keep_alive_time = 0;

}

/*****************************************************************************/
Session::~Session()
{
    if (this->keymap){
        delete this->keymap;
    }
    delete this->cache;
    delete this->front;
    delete this->default_font;
    delete this->orders;
    delete this->server;
    delete this->self_term_event;
    delete this->client_event;
    delete this->trans;
    if (this->mod_event){
        delete this->mod_event;
    }
    if (this->mod != this->no_mod){
        delete this->mod;
        this->mod = this->no_mod;
    }
    delete this->accept_event;
    delete this->no_mod;
    delete this->sesman;
    delete this->context;
}

/*****************************************************************************/
int Session::session_input_mouse(int device_flags, int x, int y)
{
    if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
        this->mod->mod_event(WM_MOUSEMOVE, x, y, 0, 0);
        this->front->mouse_x = x;
        this->front->mouse_y = y;

    }
    if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
        this->mod->mod_event(
            WM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
        this->mod->mod_event(
            WM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
        this->mod->mod_event(
            WM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
            x, y, 0, 0);
    }
    if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
        this->mod->mod_event(WM_BUTTON4DOWN, x, y, 0, 0);
        this->mod->mod_event(WM_BUTTON4UP, x, y, 0, 0);
    }
    if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
        this->mod->mod_event(WM_BUTTON5DOWN, x, y, 0, 0);
        this->mod->mod_event(WM_BUTTON5UP, x, y, 0, 0);
    }
    return 0;
}

int Session::callback(int msg, long param1, long param2, long param3, long param4)
{
    //printf("msg=%x param1=%lx param2=%lx param3=%lx param4=%lx\n",msg, param1, param2, param3, param4);
    int rv = 0;
    switch (msg) {
    case 0: /* RDP_INPUT_SYNCHRONIZE */
        /* happens when client gets focus and sends key modifier info */
        this->key_flags = param1;
        // why do we not keep device flags ?
        this->mod->mod_event(17, param1, param3, param1, param3);
        break;
    case RDP_INPUT_SCANCODE:
        this->mod->mod_event_scancode(param1, param2, param3, param4, this->key_flags, *this->keymap, this->keys);
        break;
    case 0x8001: /* RDP_INPUT_MOUSE */
        rv = this->session_input_mouse(param3, param1, param2);
        break;
    case 0x4444:
        /* invalidate, this is not from RDP_DATA_PDU_INPUT */
        /* like the rest, its from RDP_PDU_DATA with code 33 */
        /* its the rdp client asking for a screen update */
        this->mod->invalidate(Rect(param1, param2, param3, param4));
        break;
    case WM_CHANNELDATA:
        /* called from server_channel.c, channel data has come in,
        pass it to module if there is one */
        rv = this->mod->mod_event(WM_CHANNELDATA, param1, param2, param3, param4);
        break;
    default:
        break;
    }
    return rv;
}

static int get_pixel(uint8_t* data, int x, int y, int width, int bpp)
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


static int load_pointer(const char* file_name, uint8_t* data, uint8_t* mask, int* x, int* y)
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

        RGBPalette palette;
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


int Session::step_STATE_ENTRY(struct timeval & time_mark)
{
    unsigned max = 0;
    fd_set rfds;
    fd_set wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);


    #warning it should be possible to merge terminated_event (client socket closed) and self_term_event (proxy killed)
    this->terminated_event->add_to_fd_set(rfds, max);
    this->self_term_event->add_to_fd_set(rfds, max);
    this->client_event->add_to_fd_set(rfds, max);

    int i = select(max + 1, &rfds, &wfds, 0, &time_mark);
    if (((i < 0)
            && (errno != EAGAIN)
            && (errno != EWOULDBLOCK)
            && (errno != EINPROGRESS)
            && (errno != EINTR))
        || this->terminated_event->is_set()
        || this->self_term_event->is_set())
    {
        return SESSION_STATE_STOP;
    }

    if (this->client_event->is_set()) {
        try {
            this->server->activate_and_process_data(this->server_stream);
        }
        catch(...){
            return SESSION_STATE_STOP;
        };


        #warning there seems to be some code here that belongs to mod or to front

        // if we reach this point we are up_and_running,
        // hence width and height and colors and keymap are availables
        /* resize the main window */
        this->mod->screen.rect.cx = this->server->client_info.width;
        this->mod->screen.rect.cy = this->server->client_info.height;
        this->mod->screen.bpp = this->server->client_info.bpp;

        this->mod->server_reset_clip();

        if (this->cache){
            delete this->cache;
        }
        this->cache = new Cache(this->orders);
        this->front->reset(this->orders, this->cache, this->default_font);

        LOG(LOG_INFO, "width=%d height=%d bpp=%d "
                  "cache1_entries=%d cache1_size=%d "
                  "cache2_entries=%d cache2_size=%d "
                  "cache2_entries=%d cache2_size=%d ",
        this->server->client_info.width, this->server->client_info.height, this->server->client_info.bpp,
        this->server->client_info.cache1_entries, this->server->client_info.cache1_size,
        this->server->client_info.cache2_entries, this->server->client_info.cache2_size,
        this->server->client_info.cache3_entries, this->server->client_info.cache3_size);


        /* initialising keymap */
        char filename[256];
        snprintf(filename, 255, CFG_PATH "/km-%4.4x.ini", this->server->client_info.keylayout);
        LOG(LOG_DEBUG, "loading keymap %s\n", filename);
        this->keymap = new Keymap(filename);

        this->log.clear();
        RGBPalette palette;
        /* rgb332 palette */
        for (int rindex = 0; rindex < 8; rindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int bindex = 0; bindex < 4; bindex++) {
                    palette[(rindex << 5) | (gindex << 2) | bindex] =
                    (RGBcolor)(
                    // r1 r2 r2 r1 r2 r3 r1 r2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                        (((rindex<<5)|(rindex<<2)|(rindex>>1))<<0)
                    // 0 0 0 0 0 0 0 0 g1 g2 g3 g1 g2 g3 g1 g2 0 0 0 0 0 0 0 0
                       | (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8)
                    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b1 b2 b1 b2 b1 b2
                       | (((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)) << 16) );
                }
            }
        }

        this->front->send_palette(palette);

        struct pointer_item pointer_item;

        memset(&pointer_item, 0, sizeof(pointer_item));
        load_pointer(SHARE_PATH "/" CURSOR0,
            pointer_item.data,
            pointer_item.mask,
            &pointer_item.x,
            &pointer_item.y);

        this->cache->add_pointer_static(&pointer_item, 0);
        this->server->server_rdp_send_pointer(0,
                         pointer_item.data,
                         pointer_item.mask,
                         pointer_item.x,
                         pointer_item.y);

        memset(&pointer_item, 0, sizeof(pointer_item));
        load_pointer(SHARE_PATH "/" CURSOR1,
            pointer_item.data,
            pointer_item.mask,
            &pointer_item.x,
            &pointer_item.y);
        this->cache->add_pointer_static(&pointer_item, 1);

        this->server->server_rdp_send_pointer(1,
                         pointer_item.data,
                         pointer_item.mask,
                         pointer_item.x,
                         pointer_item.y);

        if (this->server->client_info.username[0]){
            this->context->parse_username(this->server->client_info.username);
        }

        if (this->server->client_info.password[0]){
            this->context->cpy(STRAUTHID_PASSWORD, this->server->client_info.password);
        }

        this->internal_state = SESSION_STATE_RUNNING;
        this->context->mod_state = MOD_STATE_INIT;
        this->session_setup_mod(MCTX_STATUS_CLI, this->context);
    }

    return this->internal_state;
}

int Session::step_STATE_CLOSE_CONNECTION()
{
    unsigned max = 0;
    fd_set rfds;

    FD_ZERO(&rfds);

    this->client_event->add_to_fd_set(rfds, max);
    this->mod_event->add_to_fd_set(rfds, max);

    if (this->mod_event->is_set()) {
        return SESSION_STATE_STOP;
    }
    if (this->client_event->is_set()) {
        try {
            this->server->activate_and_process_data(this->server_stream);
        }
        catch(...){
            return SESSION_STATE_STOP;
        };
    }

    return this->internal_state;
}


int Session::step_STATE_WAITING_FOR_NEXT_MODULE(struct timeval & time_mark)
{
    assert(this->server->up_and_running);

    unsigned max = 0;
    fd_set rfds;
    fd_set wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    this->terminated_event->add_to_fd_set(rfds, max);
    this->client_event->add_to_fd_set(rfds, max);
    this->self_term_event->add_to_fd_set(rfds, max);
    this->sesman->add_to_fd_set(rfds, max);

    int i = select(max + 1, &rfds, &wfds, 0, &time_mark);
    if (((i < 0)
            && (errno != EAGAIN)
            && (errno != EWOULDBLOCK)
            && (errno != EINPROGRESS)
            && (errno != EINTR))
        || this->terminated_event->is_set()
        || this->self_term_event->is_set())
    {
        return SESSION_STATE_STOP;
    }

    if (this->client_event->is_set()) { /* incoming client data */
        try {
            this->server->activate_and_process_data(this->server_stream);
        }
        catch(...){
            return SESSION_STATE_STOP;
        };
    }

    if (this->sesman->event()){
        this->sesman->receive_next_module();
        this->context->mod_state = MOD_STATE_RECEIVED_CREDENTIALS;
        if (this->session_setup_mod(MCTX_STATUS_TRANSITORY, this->context)){
                this->internal_state = SESSION_STATE_RUNNING;
        }
    }
    return this->internal_state;
}


int Session::step_STATE_RUNNING(struct timeval & time_mark)
{
    unsigned max = 0;
    fd_set rfds;
    fd_set wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    this->terminated_event->add_to_fd_set(rfds, max);
    this->client_event->add_to_fd_set(rfds, max);
    this->self_term_event->add_to_fd_set(rfds, max);

    this->mod_event->add_to_fd_set(rfds, max);
    this->sesman->add_to_fd_set(rfds, max);

    int i = select(max + 1, &rfds, &wfds, 0, &time_mark);
    if (((i < 0)
            && (errno != EAGAIN)
            && (errno != EWOULDBLOCK)
            && (errno != EINPROGRESS)
            && (errno != EINTR))
        || this->terminated_event->is_set()
        || this->self_term_event->is_set())
    {
        return SESSION_STATE_STOP;
    }

    time_t timestamp = time(NULL);
    this->front->periodic_snapshot(this->mod->get_pointer_displayed());

    if (this->client_event->is_set()) { /* incoming client data */
        try {
        #warning it should be possible to remove the while hidden in activate_and_process_data and work only with the external loop (need to understand well the next_packet working)
            this->server->activate_and_process_data(this->server_stream);
        }
        catch(...){
            return SESSION_STATE_STOP;
        };
    }

    if (this->sesman->close_on_timestamp(timestamp) ||
       !this->sesman->keep_alive(this->keep_alive_time, timestamp)){
        this->internal_state = SESSION_STATE_STOP;
        if (this->session_setup_mod(MCTX_STATUS_CLOSE, this->context)){
            this->keep_alive_time = 0;
            #warning move that to sesman (to hide implementation details)
            if (this->sesman->auth_event){
                delete this->sesman->auth_event;
                this->sesman->auth_event = 0;
            }
            this->internal_state = SESSION_STATE_RUNNING;
        }
        this->front->stop_capture();
    }

    if (this->mod_event->is_set()){ // data incoming from server module
        int signal = this->mod->mod_signal();
        if (signal){ // signal is the return status from module
                     // (used only for internal modules)
            if (this->mod != this->no_mod){
                delete this->mod;
                this->mod = this->no_mod;
            }
            snprintf(this->context->get(STRAUTHID_OPT_WIDTH), 10, "%d", this->server->client_info.width);
            snprintf(this->context->get(STRAUTHID_OPT_HEIGHT), 10, "%d", this->server->client_info.height);
            snprintf(this->context->get(STRAUTHID_OPT_BPP), 10, "%d", this->server->client_info.bpp);
            bool record_video = false;
            bool keep_alive = false;
            int next_state = this->sesman->ask_next_module(
                                                this->keep_alive_time,
                                                this->ini->globals.authip,
                                                3350,
                                                record_video, keep_alive);
            if (next_state != MCTX_STATUS_WAITING){
                this->internal_state = SESSION_STATE_STOP;
                if (this->session_setup_mod(next_state, this->context)){
                    if (record_video) {
                        this->front->start_capture(
                            this->mod->screen.rect.cx,
                            this->mod->screen.rect.cy,
                            this->context->get_bool(STRAUTHID_OPT_MOVIE),
                            this->context->get(STRAUTHID_OPT_MOVIE_PATH),
                            this->context->get(STRAUTHID_OPT_CODEC_ID),
                            this->context->get(STRAUTHID_VIDEO_QUALITY));
                    }
                    else {
                        this->front->stop_capture();
                    }
                    if (keep_alive){
                        this->sesman->start_keep_alive(keep_alive_time);
                    }
                    this->internal_state = SESSION_STATE_RUNNING;
                }
            }
            else {
                // this->mod_event->reset();
                this->internal_state = SESSION_STATE_WAITING_FOR_NEXT_MODULE;
            }
        }
    }
    return this->internal_state;
}

int Session::session_main_loop()
{
    Rsakeys rsa_keys(CFG_PATH "/" RSAKEYS_INI);

    int rv = 0;
    try {
        int previous_state = SESSION_STATE_STOP;
        while (1) {
            int timeout = 50;
            static struct timeval time_mark = { 0, 0 };
            if (time_mark.tv_sec == 0 && time_mark.tv_usec < 500){
                time_mark.tv_sec = timeout / 1000;
                time_mark.tv_usec = (timeout % 1000) * 1000;
            }
            switch (this->internal_state)
            {
                case SESSION_STATE_RSA_KEY_HANDSHAKE:
                    if (this->internal_state != previous_state)
                        LOG(LOG_DEBUG, "RSA Key Handshake\n");
                    this->server->server_rdp_incoming(&rsa_keys);
                    this->internal_state = SESSION_STATE_ENTRY;
                break;
                case SESSION_STATE_ENTRY:
                    if (this->internal_state != previous_state)
                        LOG(LOG_DEBUG, "Initializing client session\n");
                    previous_state = this->internal_state;
                    this->internal_state = this->step_STATE_ENTRY(time_mark);
                break;
                case SESSION_STATE_WAITING_FOR_NEXT_MODULE:
                    if (this->internal_state != previous_state)
                        LOG(LOG_DEBUG, "Waiting for authentifier\n");
                    previous_state = this->internal_state;
                    this->internal_state = this->step_STATE_WAITING_FOR_NEXT_MODULE(time_mark);
                break;
                case SESSION_STATE_RUNNING:
                    if (this->internal_state != previous_state)
                        LOG(LOG_DEBUG, "Running\n");
                    previous_state = this->internal_state;
                    this->internal_state = this->step_STATE_RUNNING(time_mark);
                break;
                case SESSION_STATE_CLOSE_CONNECTION:
                    if (this->internal_state != previous_state)
                        LOG(LOG_DEBUG, "Close connection");
                    previous_state = this->internal_state;
                    this->internal_state = this->step_STATE_CLOSE_CONNECTION();
                break;
            }
            if (this->internal_state == SESSION_STATE_STOP){
                break;
            }
        }
        this->server->server_rdp_disconnect();
    }
    catch(...){
        rv = 1;
    };
    LOG(LOG_INFO, "Client Session Disconnected\n");
    this->front->stop_capture();
    if (this->sck){
        shutdown(this->sck, 2);
        close(this->sck);
    }
   return rv;
}

bool Session::session_setup_mod(int status, const ModContext * context)
{
    try {
        if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"force")==0){
            this->server->client_info.console_session=true;
            LOG(LOG_INFO, "mode console : force");
        }
        else if (strcmp(this->context->get(STRAUTHID_MODE_CONSOLE),"forbid")==0){
            this->server->client_info.console_session=false;
            LOG(LOG_INFO, "mode console : forbid");
        }
        else {
            // default is "allow", do nothing special
        }

        #warning wait_obj should become implementation details of modules, sesman and front end
        if (this->mod_event) {
            delete this->mod_event;
            this->mod_event = 0;
        }
        if (this->mod != this->no_mod) {
            delete this->mod;
            this->mod = this->no_mod;
        }

        switch (status)
        {
            case MCTX_STATUS_CLI:
            {
                char event_name[256];
                snprintf(event_name, 255, "redemption_%8.8x_wm_transitory_mode_event_%p", getpid(), this);
                this->mod_event = new wait_obj(event_name);
                this->mod = new cli_mod(this->keys, this->key_flags, this->keymap, *this->context, *(this->front));
                this->mod_event->set();
                LOG(LOG_INFO, "Creation of new mod 'CLI parse' suceeded\n");
            }
            break;

            case MCTX_STATUS_TRANSITORY:
            {
                char event_name[256];
                snprintf(event_name, 255, "redemption_%8.8x_wm_transitory_mode_event_%p", getpid(), this);
                this->mod_event = new wait_obj(event_name);
                this->mod = new transitory_mod(this->keys, this->key_flags, this->keymap, *this->context, *(this->front));
                // Transitory finish immediately
                this->mod_event->set();
                LOG(LOG_INFO, "Creation of new mod 'TRANSITORY' suceeded\n");
            }
            break;

            case MCTX_STATUS_LOGIN:
            {
                char event_name[256];
                snprintf(event_name, 255, "redemption_%8.8x_wm_login_mode_event_%p", getpid(), this);
                this->mod_event = new wait_obj(event_name);
                this->mod = new login_mod(this->mod_event, this->keys, this->key_flags, this->keymap,  *this->context, *(this->front), this);
                // force a WM_INVALIDATE on all screen
                this->callback(0x4444, this->mod->screen.rect.x, this->mod->screen.rect.y, this->mod->screen.rect.cx, this->mod->screen.rect.cy);
                LOG(LOG_INFO, "Creation of new mod 'LOGIN DIALOG' (%d,%d,%d,%d) suceeded\n",
                    this->mod->screen.rect.x,
                    this->mod->screen.rect.y,
                    this->mod->screen.rect.cx,
                    this->mod->screen.rect.cy
                );
            }
            break;

            case MCTX_STATUS_DIALOG:
            {
                #warning change that to two different dialog modules
                const char * message = NULL;
                const char * button = NULL;
                if (this->context->mod_state == MOD_STATE_DISPLAY_MESSAGE){
                    message = this->context->get(STRAUTHID_MESSAGE);
                    button = NULL;
                }
                else {
                    message = this->context->get(STRAUTHID_MESSAGE);
                    button = this->context->get(STRAUTHID_TRANS_BUTTON_REFUSED);
                }

                LOG(LOG_INFO, "Creation of new mod 'STATUS DIALOG' suceeded\n");
                char text[256];
                snprintf(text, 255, "session_mod_%8.8x_event_%p", getpid(), this);
                this->mod_event = new wait_obj(text);
                this->mod = new dialog_mod(this->mod_event, this->keys, this->key_flags, this->keymap, *this->context, *(this->front), this, message, button);
                // force a WM_INVALIDATE on all screen
                this->callback(0x4444, this->mod->screen.rect.x, this->mod->screen.rect.y, this->mod->screen.rect.cx, this->mod->screen.rect.cy);
                Inifile ini(CFG_PATH "/" RDPPROXY_INI);
                if (htons(ini.globals.autovalidate)) {
                    LOG(LOG_INFO, "dialog autovalidated");
                    this->mod->mod_event(WM_KEYUP, 0, 0, 28, 0);
                }

            }
            break;

            case MCTX_STATUS_CLOSE:
            {
                if (this->context->get(STRAUTHID_AUTH_ERROR_MESSAGE)[0] == 0){
                    this->context->cpy(STRAUTHID_AUTH_ERROR_MESSAGE, "Connection to server failed");
                }
                char text[256];
                snprintf(text, 255, "session_mod_%8.8x_event_%p", getpid(), this);
                this->mod_event = new wait_obj(text);
                this->mod = new close_mod(this->mod_event, this->keys, this->key_flags, this->keymap, *this->context, *(this->front), this);
                // force a WM_INVALIDATE on all screen
                this->callback(0x4444, this->mod->screen.rect.x, this->mod->screen.rect.y, this->mod->screen.rect.cx, this->mod->screen.rect.cy);
                LOG(LOG_INFO, "Creation of new mod 'CLOSE DIALOG' suceeded\n");
            }
            break;

            case MCTX_STATUS_XUP:
            {
                SocketTransport * t = new SocketTransport(
                                            this->context->get(STRAUTHID_TARGET_DEVICE),
                                            atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                            0, 4, 2500000);

                this->mod_event = new wait_obj(t->sck);
                this->mod = new xup_mod(t, this->keys, this->key_flags, this->keymap, *this->context, *(this->front));
                LOG(LOG_INFO, "Creation of new mod 'XUP' suceeded\n");

            }
            break;

            case MCTX_STATUS_RDP:
            {
                // hostname is the name of the RDP host ("windows" hostname)
                // it is **not** used to get an ip address.
                char hostname[255];
                hostname[0] = 0;
                if (this->server->client_info.hostname){
                    strcpy(hostname, this->server->client_info.hostname);
                }
                SocketTransport * t = new SocketTransport(
                                        this->context->get(STRAUTHID_TARGET_DEVICE),
                                        atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                        0);
                this->mod = new mod_rdp(t,
                                    this->keys,
                                    this->key_flags,
                                    this->keymap,
                                    *this->context,
                                    *(this->front),
                                    &this->server->client_info,
                                    this->server->sec_layer.mcs_layer.channel_list,
                                    hostname,
                                    this->server->client_info.keylayout,
                                    this->context->get_bool(STRAUTHID_OPT_CLIPBOARD),
                                    this->context->get_bool(STRAUTHID_OPT_DEVICEREDIRECTION));
                this->mod_event = new wait_obj(t->sck);
                LOG(LOG_INFO, "Creation of new mod 'RDP' suceeded\n");
            }
            break;

            case MCTX_STATUS_VNC:
            {
                SocketTransport *t = new SocketTransport(
                                        this->context->get(STRAUTHID_TARGET_DEVICE),
                                        atoi(this->context->get(STRAUTHID_TARGET_PORT)),
                                        0);
                this->mod = new mod_vnc(t, this->keys, this->key_flags, this->keymap, *this->context, *(this->front), this->server->client_info.keylayout);
                this->mod_event = new wait_obj(t->sck);
                LOG(LOG_INFO, "Creation of new mod 'VNC' suceeded\n");
            }
            break;

            default:
            {
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }
    catch (...) {
        return false;
    };

    /* sync modifiers */
    this->mod->mod_event(17, this->key_flags, 0, this->key_flags, 0);

    return true;
}
