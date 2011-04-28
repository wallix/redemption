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
#include "bitmap_cache.hpp"
#include "capture.hpp"
#include "file_loc.hpp"
#include "rsa_keys.hpp"
#include "wait_obj.hpp"
#include "log.hpp"
#include "front.hpp"
#include "check_files.hpp"
#include "NewRDPOrders.hpp"

#define DONT_CAPTURE_MOVIE true
#define COLOR_IMAGE FIXTURES_PATH "/color_image2.bmp"
#define BACKGROUND true

typedef enum {
    STATE_RSA_KEY_HANDSHAKE,
    STATE_ENTRY,
    STATE_RUNNING,
    STATE_STOP
} t_internal_state;


typedef enum {
    NOT_STATE,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    LEFT_WALLIX,
    RIGHT_WALLIX,
    TOP_WALLIX,
    BOTTOM_WALLIX
} bouncing_state;

t_internal_state step_STATE_RUNNING(struct timeval & time,
                                    int sck,
                                    RDP::Orders * orders,
                                    Cache * cache,
                                    Font * font,
                                    Front * front,
                                    Capture * capture)
{
    cout << "sending orders\n";

    Bitmap bmp_background(COLOR_IMAGE, front->orders->rdp_layer->client_info.bpp);

    front->begin_update();
    front->send_bitmap_front(
        Rect(0, 0, bmp_background.cx, bmp_background.cy),
        Rect(0, 0, bmp_background.cx, bmp_background.cy),
        (uint8_t*)bmp_background.data_co,
        0, Rect(0, 0, 800, 600));
    front->screen_blt(0xcc, Rect(0,0,800,600), 0, 0, Rect(0,0,800,600));
    front->end_update();

    int index;
    index = 0;
    int src_x = 100;
    int src_y = 100;
    int src_x_wallix = 200;
    int src_y_wallix = 300;
    Bitmap bmp(SHARE_PATH "/ad24b.bmp", front->orders->rdp_layer->client_info.bpp);
    Bitmap bmp_wallix(SHARE_PATH "/ad256.bmp", front->orders->rdp_layer->client_info.bpp);
    uint8_t cache_b_id;
    uint16_t cache_b_idx;
    uint8_t cache_b_id_h;
    uint16_t cache_b_idx_h;
    uint8_t cache_b_id_wallix;
    uint16_t cache_b_idx_wallix;
    uint8_t cache_b_id_h_wallix;
    uint16_t cache_b_idx_h_wallix;
    bouncing_state state = LEFT;
    bouncing_state wallix_state = LEFT_WALLIX;

    for (;;){
        //LOG(LOG_DEBUG, "Index value is : %d\n", index);
        const Rect dst_r = Rect(src_x + 10, src_y + 10, 140, 140);
        Rect rect(src_x, src_y, 140, 140);
        Rect rectang = rect.intersect(dst_r);
        Rect fill_rect1 = Rect(src_x, src_y, 10, 140);
        Rect fill_rect2 = Rect(src_x, src_y, 140, 10);

        const Rect dst_r_wallix = Rect(src_x_wallix + 10, src_y_wallix + 10, 140, 140);
        Rect fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 10, 140);
        Rect fill_rect_wallix_h = Rect(src_x_wallix, src_y_wallix, 140, 10);

        front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x - 10, src_y, 10, 140,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id, cache_b_idx);
        front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x, src_y - 10, 140, 10,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_h, cache_b_idx_h);

        front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix, src_y_wallix, 10, 140,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_wallix, cache_b_idx_wallix);
        front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix, src_y_wallix - 10, 140, 10,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_h_wallix, cache_b_idx_h_wallix);
        switch (state){
            case BOTTOM:
                fill_rect1 = Rect(src_x , src_y, 10, 140);
                fill_rect2 = Rect(src_x , src_y + 140, 140, 10);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x , src_y, 10, 140,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id, cache_b_idx);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x , src_y + 140, 140, 10,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_h, cache_b_idx_h);
            break;
            case RIGHT:
                fill_rect1 = Rect(src_x + 140, src_y, 10, 140);
                fill_rect2 = Rect(src_x + 10, src_y + 140, 140, 10);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co,src_x + 140, src_y, 10, 140,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id, cache_b_idx);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x + 10, src_y + 140, 140, 10,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_h, cache_b_idx_h);
            break;
            default:
            break;
            case TOP:
                fill_rect1 = Rect(src_x, src_y, 140, 10);
                fill_rect2 = Rect(src_x + 140, src_y, 20, 140);

                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x, src_y, 140, 10,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id, cache_b_idx);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x + 140, src_y, 20, 140,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_h, cache_b_idx_h);
            break;
        }
        switch (wallix_state){
            case BOTTOM_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 10, 140);
                fill_rect_wallix_h = Rect(src_x_wallix, src_y_wallix + 140, 140, 10);

                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix , src_y_wallix, 10, 140,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_wallix, cache_b_idx_wallix);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix , src_y_wallix + 140, 140, 10,
                                        front->orders->rdp_layer->client_info.bpp, cache_b_id_h_wallix, cache_b_idx_h_wallix);
            break;
            case RIGHT_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix + 140, src_y_wallix, 10, 140);
                fill_rect_wallix_h = Rect(src_x_wallix + 10, src_y_wallix + 140, 140, 10);

                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix + 140 , src_y_wallix, 10, 140,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_wallix, cache_b_idx_wallix);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix + 10, src_y_wallix + 140, 140, 10,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_h_wallix, cache_b_idx_h_wallix);
            break;
            default:
            break;
            case TOP_WALLIX:
                fill_rect_wallix = Rect(src_x_wallix, src_y_wallix, 140, 10);
                fill_rect_wallix_h = Rect(src_x_wallix + 140, src_y_wallix, 20, 140);

                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix , src_y_wallix, 140, 10,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_wallix, cache_b_idx_wallix);
                front->bmp_cache->add_bitmap(800, 600, bmp_background.data_co, src_x_wallix + 140, src_y_wallix, 20, 140,
                                            front->orders->rdp_layer->client_info.bpp, cache_b_id_h_wallix, cache_b_idx_h_wallix);
            break;
        }

        front->begin_update();
        front->send_bitmap_front(
            Rect(src_x, src_y, bmp.cx, bmp.cy),
            Rect(0, 0, bmp.cx, bmp.cy),
            bmp.data_co, 0, Rect(0, 0, 800, 600));
        front->send_bitmap_front(
            Rect(src_x_wallix , src_y_wallix, bmp_wallix.cx, bmp_wallix.cy),
            Rect(0, 0, bmp_wallix.cx, bmp_wallix.cy),
            bmp_wallix.data_co, 0, Rect(0, 0, 800, 600));
        front->end_update();

        orders->init();


        BitmapCacheItem * entry_b = front->bmp_cache->get_item(cache_b_id, cache_b_idx);
        {
            #warning RDPBmpCache can probably be merged with bitmap object
            RDPBmpCache bmp(1, entry_b->bmp, cache_b_id, cache_b_idx);
        // check reserved size depending on version
            orders->reserve_order(align4(entry_b->bmp.cx * nbbytes(entry_b->bmp.bpp)) * entry_b->bmp.cy + 16);
            bmp.emit(orders->out_stream);
            bmp.data = 0;
        }

        BitmapCacheItem * entry_b_h = front->bmp_cache->get_item(cache_b_id_h, cache_b_idx_h);
        {
            #warning RDPBmpCache can probably be merged with bitmap object
            RDPBmpCache bmp(1, entry_b_h->bmp, cache_b_id_h, cache_b_idx_h);
        // check reserved size depending on version
            orders->reserve_order(align4(entry_b_h->bmp.cx * nbbytes(entry_b_h->bmp.bpp)) * entry_b_h->bmp.cy + 16);
            bmp.emit(orders->out_stream);
            bmp.data = 0;
        }


        BitmapCacheItem * entry_b_wallix = front->bmp_cache->get_item(cache_b_id_wallix, cache_b_idx_wallix);
        {
            #warning RDPBmpCache can probably be merged with bitmap object
            RDPBmpCache bmp(1, entry_b_wallix->bmp, cache_b_id_wallix, cache_b_idx_wallix);
           // check reserved size depending on version
            orders->reserve_order(align4(entry_b_wallix->bmp.cx * nbbytes(entry_b_wallix->bmp.bpp)) * entry_b_wallix->bmp.cy + 16);
            bmp.emit(orders->out_stream);
            bmp.data = 0;
        }

        BitmapCacheItem * entry_b_h_wallix = front->bmp_cache->get_item(cache_b_id_h_wallix, cache_b_idx_h_wallix);
        {
            #warning RDPBmpCache can probably be merged with bitmap object
            RDPBmpCache bmp(1, entry_b_h_wallix->bmp, cache_b_id_h_wallix, cache_b_idx_h_wallix);
        // check reserved size depending on version
            orders->reserve_order(align4(entry_b_h_wallix->bmp.cx * nbbytes(entry_b_h_wallix->bmp.bpp)) * entry_b_h_wallix->bmp.cy + 16);
            bmp.emit(orders->out_stream);
            bmp.data = 0;
        }
        orders->send();

        front->begin_update();
        front->mem_blt(cache_b_id, 0, fill_rect1, 0xcc, entry_b->bmp.bpp, entry_b->bmp.data_co, 0, 0, cache_b_idx,fill_rect1);
        front->mem_blt(cache_b_id_h, 0, fill_rect2, 0xcc, entry_b_h->bmp.bpp, entry_b_h->bmp.data_co, 0, 0, cache_b_idx_h, fill_rect2);
        front->mem_blt(cache_b_id_wallix, 0, fill_rect_wallix, 0xcc, entry_b_wallix->bmp.bpp, entry_b_wallix->bmp.data_co, 0, 0, cache_b_idx_wallix,fill_rect_wallix);
        front->mem_blt(cache_b_id_h_wallix, 0, fill_rect_wallix_h, 0xcc, entry_b_h_wallix->bmp.bpp, entry_b_h_wallix->bmp.data_co, 0, 0, cache_b_idx_h_wallix, fill_rect_wallix_h);
        front->end_update();

        src_x = src_x + 10;
        src_y = src_y + 10;
        src_x_wallix = src_x_wallix + 10;
        src_y_wallix = src_y_wallix + 10;

        if (((((src_y_wallix + 140) >= 600) && ((src_x_wallix + 140) < 800)) && (src_x_wallix > 0)) || (wallix_state == BOTTOM_WALLIX)){
            src_y_wallix = src_y_wallix - 20;
            wallix_state = BOTTOM_WALLIX;
        }
        if (((src_y_wallix > 0) && ((src_y_wallix + 140) < 600) && ((src_x_wallix + 140) >= 800)) || (wallix_state == RIGHT_WALLIX)){
            src_x_wallix = src_x_wallix - 20;
            src_y_wallix = src_y_wallix - 20;
            wallix_state = RIGHT_WALLIX;
        }
        if (((src_y_wallix  <= 10) && ((src_x_wallix + 140) < 800) && (src_x_wallix > 0)) || (wallix_state == TOP_WALLIX)){
            src_x_wallix = src_x_wallix - 20;
            wallix_state = TOP_WALLIX;
        }
        if (((src_y_wallix  > 0) && ((src_y_wallix + 140) < 600) && (src_x_wallix <= 10)) || (wallix_state == LEFT_WALLIX)){
            wallix_state = LEFT_WALLIX;
        }
        if (((((src_y + 140) >= 600) && ((src_x + 140) < 800)) && (src_x > 0)) || (state == BOTTOM)){
            src_y = src_y - 20;
            state = BOTTOM;
        }
        if (((src_y > 0) && ((src_y + 140) < 600) && ((src_x + 140) >= 800)) ||  (state == RIGHT)){
            src_x = src_x - 20;
            src_y = src_y - 20;
            state = RIGHT;
        }
        if (((src_y  <= 10) && ((src_x + 140) < 800) && (src_x > 0)) ||  (state == TOP)){
            src_x = src_x - 20;
            state = TOP;
        }
        if (((src_y  > 0) && ((src_y + 140) < 600) && (src_x <= 10)) ||(state == LEFT)){
            state = LEFT;
        }
        //capture->snapshot(1, 1, false, false);
        // Sleep for 100 msec
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        select(0, NULL, NULL, NULL, &tv);
    }
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

   cout << "hook\n";
   struct VoidCallback : public Callback
    {
        VoidCallback(){}

        virtual int callback(int msg, long param1, long param2, long param3, long param4)
        {
           /* cout << "callback ("
                 << "msg = " << msg
                 << " param1 = " << param1
                 << " param2 = " << param2
                 << " param3 = " << param3
                 << " param4 = " << param4
                 << ")\n";*/
            return 0;
        }
    } void_callback;

    try {
        LOG(LOG_DEBUG, "Initializing object\n");

        char path[256];
        int timezone = -3600;
        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        snprintf(path, 255, "/tmp/session_test_%s.flv", asctime (timeinfo));

        cout << path;
        cout << "\n";

        Inifile ini(CFG_PATH "/rdpproxy.ini");
        SocketTransport * trans = new SocketTransport(sck, NULL);
        server_rdp * server = new server_rdp(void_callback, trans, &ini);
        RDP::Orders * orders = new RDP::Orders(server);
        Font * default_font = new Font(SHARE_PATH "/" DEFAULT_FONT_NAME);
        Cache * cache = new Cache(orders);
        Front * front = new Front(orders, cache, default_font, false, false, timezone);

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
//                cout << "Capturing image\n";
//                front->capture->snapshot(1, 1, false, false);
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
                                                    front,
                                                    front->capture);
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

    int port = 3390;
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
