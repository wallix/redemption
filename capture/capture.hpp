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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Martin Potier
*/

#if !defined(__CAPTURE_HPP__)
#define __CAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <time.h>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "error.hpp"
#include "config.hpp"
#include "bitmap_cache.hpp"
#include "colors.hpp"

class Capture
{
    enum {
        ts_width  = 133,
        ts_height =  11
    };

    unsigned long pix_len;
    int real_time;
    uint8_t * data;
    int count;
    int framenb;
    char * path;

    char timestamp_data[ts_width * ts_height * 3];
    char previous_timestamp[50];

    public:
    long inter_frame_interval;
    int width;
    int height;
    int bpp;
    RGBPalette palette;

    Capture(int width, int height, int bpp, char * path, const char * codec_id, const char * video_quality) {
        this->bpp = bpp;
        this->pix_len = 0;
        this->count = 0;
        this->inter_frame_interval = 1000000; // 1 000 000 us is 1 sec (default)

        /* rgb332 palette */
        for (int bindex = 0; bindex < 4; bindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int rindex = 0; rindex < 8; rindex++) {
                    this->palette[(rindex << 5) | (gindex << 2) | bindex] =
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


        this->pix_len = width * height * 3;
        if (!this->pix_len) {
            throw Error(ERR_RECORDER_EMPTY_IMAGE);
        }

        this->draw_11x7_digits(this->timestamp_data, ts_width, 19,
        "                   ", "XXXXXXXXXXXXXXXXXXX");
        strncpy(this->previous_timestamp, "                   ", 19);

        this->data = (uint8_t *)malloc(this->pix_len);
        memset(this->data, 0, height*width);
        if (this->data == 0){
            throw Error(ERR_RECORDER_FRAME_ALLOCATION_FAILED);
        }

        LOG(LOG_INFO, "video_path is :%s\n", path);
        this->width = width;
        this->height = height;
    }

    ~Capture(){
        if (this->data){
            free(data);
        }
    }

    static void draw_11x7_digits(char * rgbpixbuf, unsigned width, unsigned lg_message, const char * message, const char * old_message)
    {
        static const char * digits =
        "       "
        "  XX   "
        " X  X  "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        " X  X  "
        "  XX   "
        "       "

        "       "
        "  XX   "
        " XXX   "
        "X XX   "
        "  XX   "
        "  XX   "
        "  XX   "
        "  XX   "
        "  XX   "
        "XXXXXX "
        "       "

        "       "
        " XXXX  "
        "XX  XX "
        "XX  XX "
        "    XX "
        "  XXX  "
        " XX    "
        "XX     "
        "XX     "
        "XXXXXX "
        "       "


        "       "
        "XXXXXX "
        "    XX "
        "   XX  "
        "  XX   "
        " XXXX  "
        "    XX "
        "    XX "
        "XX  XX "
        " XXXX  "
        "       "


        "       "
        "    XX "
        "   XXX "
        "  XXXX "
        " XX XX "
        "XX  XX "
        "XX  XX "
        "XXXXXX "
        "    XX "
        "    XX "
        "       "

        "       "
        "XXXXXX "
        "XX     "
        "XX     "
        "XXXXX  "
        "XX  XX "
        "    XX "
        "    XX "
        "XX  XX "
        " XXXX  "
        "       "

        "       "
        " XXXX  "
        "XX  XX "
        "XX     "
        "XX     "
        "XXXXX  "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        " XXXX  "
        "       "

        "       "
        "XXXXXX "
        "    XX "
        "    XX "
        "   XX  "
        "   XX  "
        "  XX   "
        "  XX   "
        " XX    "
        " XX    "
        "       "

        "       "
        " XXXX  "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        " XXXX  "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        " XXXX  "
        "       "

        "       "
        " XXXX  "
        "XX  XX "
        "XX  XX "
        "XX  XX "
        " XXXXX "
        "    XX "
        "    XX "
        "XX  XX "
        " XXXX  "
        "       "

        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "XXXXXX "
        "       "
        "       "
        "       "
        "       "

        "       "
        "       "
        "       "
        "  XX   "
        " XXXX  "
        "  XX   "
        "       "
        "       "
        "  XX   "
        " XXXX  "
        "  XX   "

        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "
        "       "

        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        "XXXXXXX"
        ;

        for (size_t i = 0 ; i < lg_message ; ++i){
            char newch = message[i];
            char oldch = old_message[i];
            if (newch != oldch){
                const char * pnewch = digits + 7 * 11 *
                                  (isdigit(newch) ? newch-'0'
                                 :   newch == '-' ?       10
                                 :   newch == ':' ?       11
                                 :   newch == ' ' ?       12
                                 :                        13);
                const char * poldch =  digits + 7 * 11 *
                                  (isdigit(oldch) ? oldch-'0'
                                 :   oldch == '-' ?       10
                                 :   oldch == ':' ?       11
                                 :   oldch == ' ' ?       12
                                 :                        13);

                for (size_t y = 0 ; y < 11 ; ++y){
                    for (size_t x = 0 ; x <  7 ; ++x){
                        unsigned pix = x+y*7;
                        if (pnewch[pix] != poldch[pix]){
                            uint8_t pixcolorcomponent = (pnewch[pix] == 'X')?0xFF:0;
                            unsigned pixindex = 3*(x+i*7+y*width);
                            rgbpixbuf[pixindex] = pixcolorcomponent;
                            rgbpixbuf[pixindex+1] = pixcolorcomponent;
                            rgbpixbuf[pixindex+2] = pixcolorcomponent;
                        }
                    }
                }
            }
        }
    }


    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp, int timezone)
    {
        #warning the mouse structure below should be created at run time from a simpler (more readable) format like the one used for digits above in timestamp.
        static struct {
            uint8_t y;
            uint8_t x;
            uint8_t lg;
            const char * line;
} mouse_cursor[20] =
        {
            {0,  0, 3*1, "\x00\x00\x00"},

            {1,  0, 3*2, "\x00\x00\x00\x00\x00\x00"},

            {2,  0, 3*3, "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},

            {3,  0, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {4,  0, 3*5, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {5,  0, 3*6, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {6,  0, 3*7, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {7,  0, 3*8, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {8,  0, 3*9, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {9,  0, 3*10,"\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {10, 0, 3*11,"\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {11, 0, 3*12,"\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {12, 0, 3*12,"\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"},

            {13, 0, 3*8, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},

            {14, 0, 3*4, "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {14, 5, 3*4,                                                             "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {15, 0, 3*3, "\x00\x00\x00\xFF\xFF\xFF\x00\x00\x00"},
            {15, 5, 3*4,                                                             "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"},
            {16, 1, 3*1,             "\x00\x00\x00"},
            {16, 6, 3*4,                                                                         "\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00"}
        };
        enum { mouse_height = 17 };

        pointer_already_displayed = no_timestamp = false;
        try {
            uint8_t mouse_save[20*3*12];
            uint8_t timestamp_save[ts_height*ts_width*3];
            // If pointer is drawn by the server (like in Windows Seven), no need to do anything.
            if (!pointer_already_displayed){
                if ((x > 0)
                        && (x < this->width - 12)
                        && (y > 0)
                        && (y < this->height - mouse_height)){
                    uint8_t * psave = mouse_save;
                    for (size_t i = 0 ; i < 20 ; i++){
                        unsigned yy = mouse_cursor[i].y;
                        unsigned xx = mouse_cursor[i].x;
                        unsigned lg = mouse_cursor[i].lg;
                        const char * line = mouse_cursor[i].line;
                        char * pixel_start = (char*)this->data + ((yy+y)*this->width+x+xx)*3;
                        memcpy(psave, pixel_start, lg);
                        psave += lg;
                        memcpy(pixel_start, line, lg);
                    }
                }
            }

            if (!no_timestamp){
                time_t rawtime;
                time(&rawtime);
                rawtime -= timezone;
                tm *ptm = gmtime(&rawtime);
                char rawdate[50];
                snprintf(rawdate, 50, "%4d-%02d-%02d %02d:%02d:%02d",
                        ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
                        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

                this->draw_11x7_digits(this->timestamp_data, ts_width, 19, rawdate, this->previous_timestamp);
                strncpy(this->previous_timestamp, rawdate, 19);
                uint8_t * tsave = timestamp_save;
                for (size_t y = 0; y < ts_height ; ++y){
                    memcpy(tsave, (char*)data+y*width*3, ts_width*3);
                    tsave += ts_width*3;
                    memcpy((char*)data+y*width*3, this->timestamp_data + y*ts_width*3, ts_width*3);
                }
            }

            // HERE WE CAN CAPTURE RAW FRAMES
            char rawImagePath[256]     = {0};
            char rawImageMetaPath[256] = {0};
            snprintf(rawImagePath,     254, "/dev/shm/%d-%d", getpid(), this->framenb++);
            snprintf(rawImageMetaPath, 254, "%s.meta", rawImagePath);
            FILE * fd = fopen(rawImageMetaPath, "w");
            if (fd) {
               fprintf(fd, "%d,%d,%s\n", this->width, this->height, this->previous_timestamp);
            }
            fclose(fd);
            fd = fopen(rawImagePath, "w");
            if (fd) {
                fwrite(this->data, 3, this->width*this->height, fd);
            }
            fclose(fd);

            // Time to restore mouse/timestamp for the next frame (otherwise it piles up)
            if (!pointer_already_displayed){
                if ((x > 0)
                        && (x < this->width - 12)
                        && (y > 0)
                        && (y < this->height - mouse_height)){
                    uint8_t * psave = mouse_save;
                    for (size_t i = 0 ; i < 20 ; i++){
                        unsigned yy = mouse_cursor[i].y;
                        unsigned xx = mouse_cursor[i].x;
                        unsigned lg = mouse_cursor[i].lg;
                        char * pixel_start = (char*)this->data + ((yy+y)*this->width+x+xx)*3;
                        memcpy(pixel_start, psave, lg);
                        psave += lg;
                    }
                }
            }
            if (!no_timestamp){
                uint8_t * tsave = timestamp_save;
                for (size_t y = 0; y < ts_height ; ++y){
                    memcpy((char*)data+y*width*3, tsave, ts_width*3);
                    tsave += ts_width*3;
                }
            }
        } catch (Error e){
            throw;
        } catch (...){ // used to catch any unexpected exception
            LOG(LOG_WARNING, "exception caught in snapshot\n");
            throw Error(ERR_RECORDER_SNAPSHOT_FAILED);
        };
    }

    void scr_blt(const RDPScrBlt & cmd, const Rect & clip)
    {
        // Destination rectangle : drect
        const Rect & drect = cmd.rect.intersect(clip);
        if (drect.isempty()){ return; }

        // Source rectangle : srect
        const Rect srect((cmd.srcx + drect.x - cmd.rect.x), (cmd.srcy + drect.y - cmd.rect.y), drect.cx, drect.cy);

        // If the destination area overlaps the source area, then the src
        // is broken in three non overlapping zones
        const Rect & overlap = srect.intersect(drect);
        if(!overlap.isempty()) {
            if(srect.equal(drect)) { return; }
            /*
             * There are many different cases, for instance the source rect can be broken
             * like the following
             *          +-------------------------+ \
             *          |                         |  \
             *          |           big           |   \
             *          |                         |    > Source rectangle
             *          +---------+---------------+   /
             *          | brother | conflict_zone |  /
             *          +---------+---------------+ /
             */
            int deltax = drect.x - srect.x; int deltay = drect.y - srect.y;

            // Break in three non intersecting rectangles and call screen_blt again.
            Rect conflict_zone = overlap.offset(-deltax, -deltay);
             // The "big" one
            Rect big(srect.x, srect.y, srect.cx, srect.cy - overlap.cy);
             // The "little" one
            Rect brother(srect.x, srect.y, srect.cx - overlap.cx, overlap.cy);

            // Four cases:
            // Conflict zone is in the upper left.  SE
            if ((overlap.x == drect.x) && (overlap.y == drect.y)) {
                big     = big.offset(0, overlap.cy);
                brother = brother.offset(overlap.cx, 0);
            }
            // Conflict zone is in the lower left.  NE
            else if ((overlap.x == drect.x) && (overlap.y == srect.y)) {
                brother = brother.offset(overlap.cx, big.cy);
            }
            // Conflict zone is in the upper right. SW
            else if ((overlap.x == srect.x) && (overlap.y == drect.y)) {
                big     = big.offset(0, overlap.cy);
            }
            // Conflict zone is in the lower right. NW
            else if ((overlap.x == srect.x) && (overlap.y == srect.y)) {
                brother = brother.offset(0, big.cy);
             }

            this->scr_blt(RDPScrBlt(big.offset(deltax, deltay), cmd.rop, big.x, big.y), clip);
            this->scr_blt(RDPScrBlt(brother.offset(deltax, deltay), cmd.rop, brother.x, brother.y), clip);

            // Last thing to do : copy the conflict zone.
            this->scr_blt(RDPScrBlt(overlap, cmd.rop, conflict_zone.x, conflict_zone.y), clip);
            return;
        }

        // The source is copied to the target
        // Where we draw -> target
        uint8_t * target = this->data + (drect.y * this->width + drect.x) * 3;
        // From where we read the source
        uint8_t * source = this->data + (srect.y * this->width + srect.x) * 3;
        for (int j = 0; j < drect.cy ; j++) {
            for (int i = 0; i < drect.cx ; i++) {
                uint8_t * pt = target + (j * this->width + i) * 3;
                uint8_t * ps = source + (j * this->width + i) * 3;
                pt[0] = ps[0];
                pt[1] = ps[1];
                pt[2] = ps[2];
            }
        }
    }

    /*
     * The name doesn't say it : mem_blt COPIES a decoded bitmap from
     * a cache (data) and insert a subpart (srcx, srcy) to the local
     * image cache (this->data) at the given position (rect).
     */
    void mem_blt(const RDPMemBlt & memblt, const BitmapCache & bmp_cache, const Rect & clip)
    {
        #warning we should use rop parameter to change mem_blt behavior and palette_id part of cache_id
        const uint8_t cache_id = memblt.cache_id & 0xFF;
        const Rect & rect = memblt.rect;
        const uint16_t srcx = memblt.srcx;
        const uint16_t srcy = memblt.srcy;
        const uint16_t cache_idx = memblt.cache_idx;
        const BitmapCacheItem * entry =  bmp_cache.get_item(cache_id & 0xFF, cache_idx);
        const uint8_t * const bmp_data = entry->pbmp->data_co;
        const uint8_t bpp = entry->pbmp->bpp;
        assert(bpp == this->bpp);

        // Where we draw -> target
        uint32_t px = 0;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t * target = this->data + (rect.y * this->width + rect.x) * 3;
        for (int j = 0; j < rect.cy ; j++){
            for (int i = 0; i < rect.cx ; i++){
                #warning: it would be nicer to manage clipping earlier and not test every pixel
                if (!(clip.rect_contains_pt(i + rect.x, j + rect.y))) {
                  continue;
                }
                #warning this should not be done here, implement bitmap color conversion and use it here
                uint32_t src_px_offset = ((rect.cy - j - srcy - 1) * align4(rect.cx) + i + srcx) * nbbytes(bpp);
                switch (bpp){
                    default:
                    case 32:
                        assert(false);
                    break;
                    case 24:
                        {
                            px = (bmp_data[src_px_offset+2]<<16)
                               + (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = (px >> 16) & 0xFF;
                            g = (px >> 8)  & 0xFF;
                            b =  px        & 0xFF;
                        }
                        break;
                    case 16:
                        {
                            px = (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = (((px >> 8) & 0xf8) | ((px >> 13) & 0x7));
                            g = (((px >> 3) & 0xfc) | ((px >> 9) & 0x3));
                            b = (((px << 3) & 0xf8) | ((px >> 2) & 0x7));
                        }
                        break;
                    case 15:
                        {
                            px = (bmp_data[src_px_offset+1]<<8)
                               + (bmp_data[src_px_offset+0]);

                            r = ((px >> 7) & 0xf8) | ((px >> 12) & 0x7);
                            g = ((px >> 2) & 0xf8) | ((px >> 8) & 0x7);
                            b = ((px << 3) & 0xf8) | ((px >> 2) & 0x7);
                        }
                        break;
                    case 8:
                        {
                            px = bmp_data[src_px_offset+0];

                            r = px & 7;
                            r = (r << 5) | (r << 2) | (r >> 1);
                            g = (px >> 3) & 7;
                            g = (g << 5) | (g << 2) | (g >> 1);
                            b =  (px >> 6) & 3;
                            b = (b << 6) | (b << 4) | (b << 2) | b;
                        }
                        break;
                }
                // Pixel assignment (!)
                uint8_t * pt = target + (j * this->width + i) * 3;
                pt[0] = b;
                pt[1] = g;
                pt[2] = r;
            }
        }
    }

    void opaque_rect(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        const Rect trect = clip.intersect(cmd.rect);
        uint32_t color = color_decode(cmd.color, this->bpp, this->palette);

        // base adress (*3 because it has 3 color components)
        uint8_t * base = this->data + (trect.y * this->width + trect.x) * 3;
        for (int j = 0; j < trect.cy ; j++){
            for (int i = 0; i < trect.cx ; i++){
               uint8_t * p = base + (j * this->width + i) * 3;
               p[0] = color >> 16; // r
               p[1] = color >> 8;  // g
               p[2] = color;       // b
            }
        }
    }


/*
 *
 *            +----+----+
 *            |\   |   /|  4 cases.
 *            | \  |  / |  > Case 1 is the normal case
 *            |  \ | /  |  > Case 2 has a negative coeff
 *            | 3 \|/ 2 |  > Case 3 and 4 are the same as
 *            +----0---->x    Case 1 and 2 but one needs to
 *            | 4 /|\ 1 |     exchange begin and end.
 *            |  / | \  |
 *            | /  |  \ |
 *            |/   |   \|
 *            +----v----+
 *                 y
 *  Anyway, we base the line drawing on bresenham's algorithm
 */


    void line_to(const RDPLineTo & lineto, const Rect & clip)
    {

        if (lineto.startx >= lineto.starty){
            line(lineto.back_mode,
                 lineto.startx, lineto.starty, lineto.endx, lineto.endy,
                 lineto.rop2, lineto.back_color, lineto.pen, clip);
        }
        else {
            line(lineto.back_mode,
                 lineto.endx, lineto.starty, lineto.startx, lineto.endy,
                 lineto.rop2, lineto.back_color, lineto.pen, clip);
        }
    }

    void line(const int mix_mode, const int startx, const int starty, const int endx, const int endy, const int rop2,
              const int bg_color, const RDPPen & pen, const Rect & clip)
    {
        int minx = std::min(startx, endx);
        int miny = std::min(starty, endy);

        Rect drect = Rect(minx, miny, abs(endx - startx), abs(endy - starty));
        #warning clip should be managed
        const Rect trect = clip.intersect(drect);

        // Color handling
        const uint32_t color = color_decode(pen.color, this->bpp, this->palette);

        // base adress (*3 because it has 3 color components) also base of the new coordinate system
        uint8_t * base = this->data + (starty * this->width + startx) * 3;

        // Prep
        int x0, y0, x1, y1, err, dx, dy, sy;
        x0 = 0; y0 = 0; x1 = endx - startx; y1 = endy - starty;

        dx = abs(x1-x0);
        dy = abs(y1-y0);
        if (y0 < y1) { sy = 1; } else { sy = -1; }
        err = dx - dy;

        while (true) {
            // Pixel position
            uint8_t * p = base + (y0 * this->width + x0) * 3;

            // Drawing of a pixel
            p[0] = color >> 16; // r
            p[1] = color >> 8;  // g
            p[2] = color;       // b

            if ((x0 == x1) && (y0 == y1)) { break; }
            // Calculating pixel position
            int e2 = err * 2; //prevents use of floating point
            if (e2 > -dy) {
                err -= dy;
                x0++;
            }
            if (e2 <  dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
       return;
    }

};

#endif
