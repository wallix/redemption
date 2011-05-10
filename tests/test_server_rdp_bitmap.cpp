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

   Module: test_server_rdp_bitmap.cpp
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
    const RDPBrush brush;
//    orders->init();
//    orders->opaque_rect(Rect(0, 0, 34, 34), color.pink, Rect(0, 0, 34, 34));

#define PIXWHITE 0xFF, 0xFF, 0xFF
#define PIX4WHITE PIXWHITE, PIXWHITE, PIXWHITE, PIXWHITE
#define PIX8WHITE PIX4WHITE, PIX4WHITE
#define PIX32WHITE PIX8WHITE, PIX8WHITE, PIX8WHITE, PIX8WHITE

#define PIXBLUE 0xFF, 0x00, 0x00
#define PIX4BLUE PIXBLUE, PIXBLUE, PIXBLUE, PIXBLUE
#define PIX8BLUE PIX4BLUE, PIX4BLUE
#define PIX32BLUE PIX8BLUE, PIX8BLUE, PIX8BLUE, PIX8BLUE

#define PIX32BLUEWHITE PIX8BLUE, PIX8BLUE, PIX8WHITE, PIX8WHITE
#define PIX32WHITEBLUE PIX8WHITE, PIX8WHITE, PIX8BLUE, PIX8BLUE

//    uint8_t data[32*32*3] = {
//        PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE,
//        PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE,
//        PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE,
//        PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE, PIX32BLUEWHITE,

//        PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE,
//        PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE,
//        PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE,
//        PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE, PIX32WHITEBLUE
//    };

//    // color encoding is BGR
//    orders->send_raw_bitmap(32, 32, 24, data, cache_id, cache_idx);

//    orders->mem_blt(cache_id, 0, Rect(1, 1, 30, 30), 0xcc, 30, 30, 2, 2, cache_idx, Rect(0, 0, 34, 34));
//    orders->send();

//#define COLOR_IMAGE FIXTURES_PATH "/color_image2.bmp"
//    Bitmap bgbmp(COLOR_IMAGE, front->orders->rdp_layer->client_info.bpp);

//#define LOGO FIXTURES_PATH "/logo-redemption.bmp"
//    Bitmap logobmp(LOGO, front->orders->rdp_layer->client_info.bpp);

////#define LOGO2 FIXTURES_PATH "/logo-truncated-16x2.bmp"
////    Bitmap logobmp2(LOGO2, front->orders->rdp_layer->client_info.bpp);

//    {
//        front->begin_update();
//        #warning it is not necessary to send all the bitmap, sending the clipped region would be enough
//        front->send_bitmap_front(
//            Rect(0, 0, bgbmp.cx, bgbmp.cy),
//            Rect(0, 0, bgbmp.cx, bgbmp.cy),
//            (uint8_t*)bgbmp.data_co,
//            0, Rect(0, 0, bgbmp.cx, bgbmp.cy));
//        front->end_update();
//    }

//    {
//        front->begin_update();
//        #warning it is not necessary to send all the bitmap, sending the clipped region would be enough
//        front->send_bitmap_front(
//            Rect(0, 0, logobmp.cx, logobmp.cy),
//            Rect(0, 0, logobmp.cx, logobmp.cy),
//           (uint8_t*)logobmp.data_co,
//           0, Rect(0, 0, 800, 600));
//        front->end_update();
//    }

// 17014

// sécurité su' c

//uint8_t compressed[] = {
//0xc0, 0x30, 0xde, 0xeb, 0xef, 0x00, 0x21, 0x40, 0x32, 0xc7, 0x71, 0x5e, 0x64, 0x0e, 0x8e, 0x07,
//0x09, 0x40, 0x35, 0xf7, 0xbe, 0xf7, 0x00, 0xf7, 0xe1, 0x3d, 0x09, 0x40, 0x0e, 0x8f, 0x40, 0x1a,
//0x41, 0x81, 0x14, 0x40, 0x09, 0xcf, 0x03, 0x17, 0x40, 0x0b, 0x0f, 0x0f, 0x14, 0x41, 0xe1, 0x0e,
//0x21, 0x09, 0x40, 0x09, 0x07, 0x02, 0x0a, 0x21, 0x0a, 0x51, 0xef, 0x7b, 0x80, 0x61, 0x1f, 0x1e,
//0x60, 0x78, 0xce, 0x71, 0x51, 0x65, 0x0e, 0x5c, 0x44, 0x70, 0x80, 0x12, 0x40, 0x08, 0x01, 0x01,
//0x60, 0x05, 0xde, 0xeb, 0xef, 0x21, 0x7a, 0xde, 0xeb, 0xef, 0x21, 0x60, 0x7b, 0xde, 0xeb, 0xef,
//   };


uint8_t raw[] = {
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,
0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0x1d, 0x99,  0xa6, 0x31,
};

uint8_t raw2[] = {
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00,
0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0x1d, 0x99, 0x00, 0xa6, 0x31, 0x00,
};

    Bitmap bmp(16, 48, 1);
    bmp.copy_upsidedown(raw, 48);

    Bitmap & bitmap = bmp;

    RGBPalette palette332;
    /* rgb332 palette */
    for (int bindex = 0; bindex < 4; bindex++) {
        for (int gindex = 0; gindex < 8; gindex++) {
            for (int rindex = 0; rindex < 8; rindex++) {
                palette332[(rindex << 5) | (gindex << 2) | bindex] =
                (RGBcolor)(
                // r1 r2 r2 r1 r2 r3 r1 r2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                    (((rindex<<5)|(rindex<<2)|(rindex>>1))<<16)
                // 0 0 0 0 0 0 0 0 g1 g2 g3 g1 g2 g3 g1 g2 0 0 0 0 0 0 0 0
                   | (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8)
                // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b1 b2 b1 b2 b1 b2
                   | ((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)));
            }
        }
    }

    const uint16_t width = bitmap.cx;
    const uint16_t height = bitmap.cy;
    const uint8_t * src = bitmap.data_co;
    const uint8_t in_bpp = bitmap.bpp;
    const uint8_t out_bpp = front->orders->rdp_layer->client_info.bpp;

    uint8_t * bmpdata = (uint8_t*)malloc(width * height * nbbytes(out_bpp));
    uint8_t * dest = bmpdata;
    for (int i = 0; i < width * height; i++) {
        uint32_t pixel = color_decode(in_bytes_le(nbbytes(in_bpp), src),
                                      in_bpp,
                                      palette332);
        uint32_t target_pixel = color_encode(pixel, out_bpp, palette332);
        printf("target_pixel = %.6x pixel = %.6x out_bpp=%d nbbout=%d in_bpp=%d nbbin=%d\n",
            target_pixel, pixel, out_bpp, nbbytes(out_bpp), in_bpp, nbbytes(in_bpp));

        target_pixel = 0xFFFFFF & target_pixel;
        out_bytes_le(dest, nbbytes(out_bpp), target_pixel);
        src += nbbytes(in_bpp);
        dest += nbbytes(out_bpp);
    }

    front->begin_update();
    front->send_bitmap_front(Rect(31, 572, 48, 1), Rect(0, 0, 48, 1), bmpdata, 0, Rect(0, 0, 1024, 768));
    front->end_update();

//    using namespace RDP;

//    uint8_t width = 228;
//    uint8_t height = 13;
//    uint8_t cid = 1;
//    front->orders->init();
//    RDPBmpCache bmp_order(&bbb, cid, 48, &front->orders->rdp_layer->client_info);
//    front->orders->reserve_order(align4(bbb.cx * nbbytes(bbb.bpp)) * bbb.cy + 16);
//    bmp_order.emit(front->orders->out_stream);
//    bmp_order.bmp = 0; // we do not want RDPBmpCache to desallocate bmp
//    front->orders->mem_blt(cid, 0, Rect(364, 356, width, height), 0xcc, 0, 0, 48, Rect(0, 0, 800, 600));
//    front->orders->send();


//orders::send_bitmap_small_headers width=64 height=64 bpp=24 data=0xbfcad96c bufsize=635 2:129
////mem_blt(color_table=0, r(0, 0, 64, 64), rop=cc, bmp_cx=64, bmp_cy=64, srcx=0, srcy=0, clip(0, 0, 800, 600) cache 2:129

//    {
//        LOG(LOG_INFO, "logo size: %d %d", logobmp2.cx, logobmp2.cy);

//        front->begin_update();
//        front->send_bitmap_front(
//        Rect(0, 0, logobmp2.cx, logobmp2.cy),
//        // src_r(0, 0, 256, 125)
//        Rect(0, 0, logobmp2.cx, logobmp2.cy),
//           (uint8_t*)logobmp2.data_co,
//           0, Rect(0, 0, logobmp2.cx, logobmp2.cy));
//        front->end_update();
//    }

    cout << "waiting for 50 seconds\n";
    sleep(50);
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
        SocketTransport * trans = new SocketTransport(sck, NULL);
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

    int port = 3389;
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
