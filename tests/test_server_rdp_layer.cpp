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
   Author(s): Christophe Grosjean

   Module: test_server_rdp_layer.cpp
   Description : sample minimal automated answering rdp software

   This code is not production code. It is used to test compatibility
   with RDP client software through core server_rdp layer. This code
   issue the simple minimal drawing orders sequences we want to test.

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "stream.hpp"
#include "config.hpp"
#include "transport.hpp"
#include "server_rdp.hpp"
#include "orders.hpp"
#include "client_info.hpp"
#include "font.hpp"
#include "cache.hpp"
#include "bitmap.hpp"
#include "file_loc.hpp"
#include "rsa_keys.hpp"
#include "wait_obj.hpp"
#include "log.hpp"
#include "front.hpp"
#include "check_files.hpp"

typedef enum {
    STATE_RSA_KEY_HANDSHAKE,
    STATE_ENTRY,
    STATE_RUNNING,
    STATE_STOP
} t_internal_state;


t_internal_state step_STATE_RUNNING(struct timeval & time,
                                    int sck,
                                    RDP::Orders * orders,
                                    Cache * cache,
                                    Font * font,
                                    Front * front)
{
    cout << "sending orders\n";

    orders->init();

    orders->opaque_rect(Rect(50, 50, 150, 150), PINK, Rect(50, 50, 150, 150));
    orders->opaque_rect(Rect(520, 360, 60, 25), DARK_GREY, Rect(520, 360, 60, 25));

    // add text to glyph cache
    const char * text = "Hello";
    int len = mbstowcs(0, text, 0);
    wchar_t* wstr = new wchar_t[len + 2];
    mbstowcs(wstr, text, len + 1);
    int cx = 0;
    int cy = 0;
    uint8_t *data = new uint8_t[len * 4];
    memset(data, 0, len * 4);
    int f = 0;
    int c = 0;
    int k = 0;
    for (int index = 0; index < len; index++) {
        FontChar* font_item = font->font_items[wstr[index]];
        switch (cache->add_glyph(font_item, f, c))
        {
            case Cache::GLYPH_ADDED_TO_CACHE:
                orders->send_font(font_item, f, c);
            break;
            default:
            break;
        }
        data[index * 2] = c;
        data[index * 2 + 1] = k;
        k = font_item->incby;
        cx += k;
        cy = std::max(cy, font_item->height);
    }

    orders->glyph_index(7, 3, BLACK, YELLOW, 0, Rect(535, 363, cx+1, cy+1), Rect(0, 0, 0, 0), 536, 364+cy, data, len * 2, Rect(0, 0, 800, 600));

    //orders->screen_blt(Rect(190, 190, 50, 50), 50, 50, 0xCC, Rect(190, 190, 50, 50));

    orders->dest_blt(Rect(60, 60, 50, 50), 0, Rect(60, 60, 50, 50));

    orders->send();

    front->begin_update();
    RDPBrush brush;
    brush.org_x = 0;
    brush.org_y = 0;
    brush.style = 3;

    if (front->orders->rdp_layer->client_info.brush_cache_code == 1) {
        uint8_t pattern[8] = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
        int cache_idx = 0;
        if (BRUSH_TO_SEND == front->cache->add_brush(pattern, cache_idx)){
            front->send_brush(cache_idx);
        }
        brush.hatch = cache_idx;
        brush.style = 0x81;
    }
    front->end_update();

    orders->init();

    orders->pat_blt(
        Rect(50, 50, 150, 150),
        0x5A,
        PINK,
        WHITE,
        brush,
        Rect(100, 100, 50, 50));

    orders->pat_blt(
        Rect(63, 78, 10, 13),
        0x5A,
        BLACK,
        WHITE,
        brush,
        Rect(63, 78, 10, 13));

    orders->send();

    uint32_t picture24[100][100];

    for(int i = 0; i < 100; i++){
        for (int j = 0; j < 100; j++){
            picture24[i][j] = WHITE;
        }
    }

    orders->init();

    RDPPen pen;
    pen.style = 1;
    pen.width = 10;
    pen.color = BLUE;

    orders->line_to(1, 50, 50, 150, 150, 0x0D, PINK, pen, Rect(100, 100, 50, 50));

    orders->send();

    cout << "testing front primitives\n";

    /* Draw a big white rect at right-bottom side of screen */
    front->begin_update();
    Region region;
    Rect r1(0, 0, 800, 600);
    region.rects.push_back(r1);
    front->opaque_rect(Rect(500, 400, 200, 150), DARK_GREY, Rect(500, 400, 200, 150));
    front->end_update();

    /* Draw black line crossing right-bottom white rect of screen */

    front->begin_update();

    pen.style = 1;
    pen.width = 10;
    pen.color = PINK;

    /* Line clipped 500, 400, 201, 151*/
    front->line(0xCC, 700, 400, 500, 550, 0, pen, Rect(600, 400, 150, 150));

    /*Normal line */
    front->line(0xCC, 500, 400, 700, 550, 0, pen, Rect(500, 400, 200, 150));

    front->end_update();

    /* Write "hello" at right-bottom white rect of screen */

    front->begin_update();

    front->draw_text2(7, 3, 0,
                Rect(0, 0, 0, 0), Rect(500, 530, cx+1, cy+1),
                501, 531+cy, data, len * 2,
                BLACK, DARK_GREY, Rect(0, 0, 800, 600));
    front->end_update();

    /* Draw a little grey rect at left-bottom side of screen */

    front->begin_update();
    front->pat_blt(Rect(0, 450, 50, 50), 0x5A,
                           DARK_GREY, DARK_GREY,
                           brush,
                           Rect(0, 450, 50, 50));
    front->end_update();

    /* Draw a little rect at bottom side of screen */
    uint32_t picture16[100][100];

    for(int i = 0; i < 100; i++){
        for (int j = 0; j < 100; j++){
            picture16[i][j] = 0xFFFF00;
        }
    }


    front->begin_update();

    uint32_t cache_ref = front->bmp_cache->add_bitmap(100, 100, (uint8_t*)picture16, 0, 0, 32, 32,
                                front->orders->rdp_layer->client_info.bpp);

    uint8_t send_type2 = (cache_ref >> 24);
    uint8_t cache_b_id = (cache_ref >> 16);
    uint16_t cache_b_idx = (cache_ref & 0xFFFF);

    BitmapCacheItem * entry =  front->bmp_cache->get_item(cache_b_id, cache_b_idx);

    int e = entry->pbmp->cx % 4;
    if (e != 0) {
        e = 4 - e;
    }

    LOG(LOG_INFO, "new_b_bpp=%d new_b_cx=%d new_b_cy=%d", entry->pbmp->bpp, entry->pbmp->cx, entry->pbmp->cy);
    for (int x = 0 ; x < 100 ; x+=20){
            unsigned char * tmp = (unsigned char*)entry->pbmp->data_co + x;
        LOG(LOG_INFO, "%0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x %0.2x",
            tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], tmp[8], tmp[9],
            tmp[10], tmp[11], tmp[12], tmp[13], tmp[14], tmp[15], tmp[16], tmp[17], tmp[18], tmp[19]);
    };

    using namespace RDP;

    if (send_type2 == BITMAP_ADDED_TO_CACHE){
        LOG(LOG_INFO, "Sending bitmap\n");
        RDPBmpCache bmp(entry->pbmp, cache_b_id, cache_b_idx, &front->orders->rdp_layer->client_info);
        // check reserved size depending on version
        orders->reserve_order(align4(entry->pbmp->cx * nbbytes(entry->pbmp->bpp)) * entry->pbmp->cy + 16);
        bmp.emit(orders->out_stream);
    }
    front->mem_blt(cache_b_id, 0, Rect(100, 450, 32, 32), 0xcc, entry->pbmp->bpp, entry->pbmp->data_co, 0, 0, cache_b_idx, Rect(100, 450, 32, 32));
    front->end_update();

    /* Draw a little pink rect at bottom-right side of screen bottom-right rectangle */
    front->begin_update();
    front->screen_blt(0xcc, Rect(690, 540, 50, 50), 190, 190, Rect(690, 540, 50, 50));
    front->end_update();

    cout << "waiting for 20 seconds\n";
    sleep(20);
    return STATE_STOP;
}

t_internal_state step_STATE_ENTRY(struct timeval & time, wait_obj & evt, int sck, Front * front)
{

    cout << "STATE ENTRY\n";

    unsigned max = 0;
    fd_set rfds;
    fd_set wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    evt.add_to_fd_set(rfds, max);

    int i = select(max + 1, &rfds, &wfds, 0, &time);
    if (((i < 0)
            && (errno != EAGAIN)
            && (errno != EWOULDBLOCK)
            && (errno != EINPROGRESS)
            && (errno != EINTR)))
    {
        return STATE_STOP;
    }

    Stream s(8192);
    if (evt.is_set()) {
        try {
            LOG(LOG_DEBUG, "waiting for up_and_runing\n");
            front->orders->rdp_layer->activate_and_process_data(s);
            LOG(LOG_DEBUG, "now up_and_running\n");

        front->reset(front->orders, front->cache, front->font);

        LOG(LOG_INFO, "width=%d height=%d bpp=%d "
                  "cache1_entries=%d cache1_size=%d "
                  "cache2_entries=%d cache2_size=%d "
                  "cache2_entries=%d cache2_size=%d ",
        front->orders->rdp_layer->client_info.width,
        front->orders->rdp_layer->client_info.height,
        front->orders->rdp_layer->client_info.bpp,
        front->orders->rdp_layer->client_info.cache1_entries,
        front->orders->rdp_layer->client_info.cache1_size,
        front->orders->rdp_layer->client_info.cache2_entries,
        front->orders->rdp_layer->client_info.cache2_size,
        front->orders->rdp_layer->client_info.cache3_entries,
        front->orders->rdp_layer->client_info.cache3_size);

            return STATE_RUNNING;
        }
        catch (Error e){
            cout << "catched exception " << e.id << "\n";
            exit(0);
        }
        catch(...){
            cout << "catched exception\n";
            return STATE_STOP;
        };
    }
    return STATE_ENTRY;
}


int hook(int sck)
{
    openlog("rdpproxy", LOG_CONS | LOG_PERROR, LOG_USER);

        /* set non blocking */
    int rv = fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);
    if (rv < 0){
        /* 1 session_main_loop fnctl socket error */
        throw 1;
    }
    int nodelay = 1;

    /* SOL_TCP IPPROTO_TCP */
    setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    if (rv < 0){
        /* 2 session_main_loop fnctl socket error */
        throw 2;
    }

    wait_obj client_event(sck);

   struct VoidCallback : public Callback
    {
        Front * front;
        VoidCallback(){}

        virtual int callback(int msg, long param1, long param2, long param3, long param4)
        {
            return 0;
        }
    } void_callback;

    try {

        Inifile ini(CFG_PATH "/rdpproxy.ini");
        SocketTransport * trans = new SocketTransport(sck);
        server_rdp * server = new server_rdp(void_callback, trans, &ini);
        RDP::Orders * orders = new RDP::Orders(server);
        Font * default_font = new Font(SHARE_PATH "/" DEFAULT_FONT_NAME);
        Cache * cache = new Cache(orders);
        int timezone = -3600;
        Front * front = new Front(orders, cache, default_font, false, false, timezone);
        void_callback.front = front;

        Rsakeys * rsa_keys = new Rsakeys(CFG_PATH "/rsakeys.ini");

        t_internal_state internal_state = STATE_RSA_KEY_HANDSHAKE;

        cout << "Entering main loop\n";

        bool loop_flag = true;
        while (loop_flag) {
            int timeout = 50;
            static struct timeval time = { 0, 0};
            if (time.tv_sec == 0 && time.tv_usec < 100){
                time.tv_sec = timeout / 100000;
                time.tv_usec = (timeout % 1000) * 1000;
                // about one snapshot each second
            }
            switch (internal_state)
            {
                case STATE_RSA_KEY_HANDSHAKE:
                    cout << "handshaking\n";
                    server->server_rdp_incoming(rsa_keys);
                    internal_state = STATE_ENTRY;
                break;
                case STATE_ENTRY:
                    cout << "state entry\n";
                    internal_state = step_STATE_ENTRY(time, client_event, sck, front);
                break;
                case STATE_RUNNING:
                    cout << "state running\n";
                    internal_state = step_STATE_RUNNING(
                                                    time,
                                                    sck,
                                                    orders,
                                                    cache,
                                                    default_font,
                                                    front);
                break;
                default:
                    loop_flag = false;
                break;
            }
        }
    } catch (...) {
        cout << "Exception catched End of test server\n";
    };

    sleep(10);
    cout << "out of hook\n";

    return 0;
}


int main()
{
    int sck = socket(AF_INET, SOCK_STREAM, 0);

    // reuse same port if a previous daemon was stopped
    int allow_reuse = 1;
    setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, (char*)&allow_reuse, sizeof(allow_reuse));

    int port = 3391;
    const char *ip = "0.0.0.0";

    struct sockaddr_in saddr;

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);
    bind(sck, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in));
    listen(sck, 2);

    struct pollfd fds[1];
    printf("Running server\n");
    for (;;)
    {
        fds[0].fd = sck;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        switch(poll(fds, 1, 1000000)){
            case 0:
                printf("No event occured in 1000s, looping\n");
            break;
            // error
            case -1:
                switch(errno){
                    // An invalid file descriptor was given in one of the sets.
                    case EBADF:
                        printf("EBADF error\n");
                    break;
                    // The array given as argument was not contained in the
                    // calling program's address space.
                    case EFAULT:
                        printf("EFAULT error\n");
                    break;

                    // A signal occurred before any requested event; see signal(7).
                    case EINTR:
                        printf("EINTR error\n");
                    break;

                    // The nfds value exceeds the RLIMIT_NOFILE value.
                    case EINVAL:
                        printf("EINVAL error\n");
                    break;

                    // There was no space to allocate file descriptor tables.
                    case ENOMEM:
                        printf("ENOMEM error\n");
                    break;
                }
                exit(-1);
            break;
            // normal behavior, a positive number of events occured
            default:
                struct sockaddr_in sin;
                unsigned int sin_size = sizeof(struct sockaddr_in);
                memset(&sin, 0, sin_size);
                int in_sck = accept(sck, (struct sockaddr*)&sin, &sin_size);
                if (!fork()){
                    exit(hook(in_sck));
                }
                close(in_sck);
            break;
        }
    }
    return 0;
}
