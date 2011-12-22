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

#if !defined(__STATICCAPTURE_HPP__)
#define __STATICCAPTURE_HPP__

#include <iostream>
#include <stdio.h>
#include "rdtsc.hpp"
#include <sstream>
#include "bitmap.hpp"
#include "rect.hpp"
#include "constants.hpp"
#include <sys/time.h>
#include <time.h>
#include <png.h>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersSecondaryHeader.hpp"
#include "RDP/orders/RDPOrdersSecondaryColorCache.hpp"
#include "RDP/orders/RDPOrdersSecondaryBmpCache.hpp"

#include "RDP/orders/RDPOrdersPrimaryHeader.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"

#include "png.hpp"

#include "error.hpp"
#include "config.hpp"
#include "bitmap_cache.hpp"
#include "colors.hpp"
#include "drawable.hpp"


class StaticCapture : public Drawable
{
    enum {
        ts_width  = 133,
        ts_height =  11
    };

    int framenb;
    uint64_t inter_frame_interval;

    char timestamp_data[ts_width * ts_height * 3];
    char previous_timestamp[50];
    struct timeval start;

    public:
    BGRPalette palette;
    char path[1024];

    StaticCapture(int width, int height, int bpp, const BGRPalette & palette, BmpCache & bmpcache, char * path, const char * codec_id, const char * video_quality)
        : Drawable(width, height, bpp, palette, bmpcache, false),
          framenb(0)
    {
        gettimeofday(&this->start, NULL);
        this->inter_frame_interval = 1000000; // 1 000 000 us is 1 sec (default)

        this->draw_11x7_digits(this->timestamp_data, ts_width, 19,
        "                   ", "XXXXXXXXXXXXXXXXXXX");
        memcpy(this->previous_timestamp, "                   ", 20);
        strcpy(this->path, path);
    }

    ~StaticCapture(){
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

    uint64_t difftimeval(const struct timeval endtime, const struct timeval starttime)
    {
      uint64_t sec = (endtime.tv_sec  - starttime.tv_sec ) * 1000000
                   + (endtime.tv_usec - starttime.tv_usec);
      return sec;
    }

    void snapshot(int x, int y, bool pointer_already_displayed, bool no_timestamp)
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        if (difftimeval(now, this->start) < this->inter_frame_interval){
            return;
        }
        this->start = now;

        TODO(" the mouse structure below should be created at run time from a simpler (more readable) format like the one used for digits above in timestamp.")
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
                        && (x < this->full.cx - 12)
                        && (y > 0)
                        && (y < this->full.cy - mouse_height)){
                    uint8_t * psave = mouse_save;
                    for (size_t i = 0 ; i < 20 ; i++){
                        unsigned yy = mouse_cursor[i].y;
                        unsigned xx = mouse_cursor[i].x;
                        unsigned lg = mouse_cursor[i].lg;
                        const char * line = mouse_cursor[i].line;
                        char * pixel_start = (char*)this->data + ((yy+y)*this->full.cx+x+xx)*3;
                        memcpy(psave, pixel_start, lg);
                        psave += lg;
                        memcpy(pixel_start, line, lg);
                    }
                }
            }

            if (!no_timestamp){
                time_t rawtime;
                time(&rawtime);
                tm *ptm = localtime(&rawtime);
                char rawdate[50];
                snprintf(rawdate, 50, "%4d-%02d-%02d %02d:%02d:%02d",
                        ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
                        ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

                this->draw_11x7_digits(this->timestamp_data, ts_width, 19, rawdate, this->previous_timestamp);
                strncpy(this->previous_timestamp, rawdate, 19);
                uint8_t * tsave = timestamp_save;
                for (size_t y = 0; y < ts_height ; ++y){
                    memcpy(tsave, (char*)data+y * this->full.cx * 3, ts_width*3);
                    tsave += ts_width*3;
                    memcpy((char*)data + y * this->full.cx * 3,
                            this->timestamp_data + y*ts_width*3, ts_width*3);
                }
            }

            this->dump_png();

            // Time to restore mouse/timestamp for the next frame (otherwise it piles up)
            if (!pointer_already_displayed){
                if ((x > 0)
                        && (x < this->full.cx - 12)
                        && (y > 0)
                        && (y < this->full.cy - mouse_height)){
                    uint8_t * psave = mouse_save;
                    for (size_t i = 0 ; i < 20 ; i++){
                        unsigned yy = mouse_cursor[i].y;
                        unsigned xx = mouse_cursor[i].x;
                        unsigned lg = mouse_cursor[i].lg;
                        char * pixel_start = (char*)this->data + ((yy+y)*this->full.cx+x+xx)*3;
                        memcpy(pixel_start, psave, lg);
                        psave += lg;
                    }
                }
            }
            if (!no_timestamp){
                uint8_t * tsave = timestamp_save;
                for (size_t y = 0; y < ts_height ; ++y){
                    memcpy((char*)data+y * this->full.cx * 3, tsave, ts_width*3);
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

    void dump_png(void){
        char rawImagePath[256]     = {0};
        char rawImageMetaPath[256] = {0};
        snprintf(rawImagePath,     254, "%s.%u.%u.png", this->path, getpid(), this->framenb++);
        snprintf(rawImageMetaPath, 254, "%s.meta", rawImagePath);
        LOG(LOG_INFO, "Dumping to file %s", rawImagePath);
        FILE * fd = fopen(rawImageMetaPath, "w");
        if (fd) {
           fprintf(fd, "%d,%d,%s\n", this->full.cx, this->full.cy, this->previous_timestamp);
        }
        fclose(fd);
        fd = fopen(rawImagePath, "w");
        if (fd) {
            ::dump_png24(fd, this->data, this->full.cx, this->full.cy, this->rowsize);
        }
        fclose(fd);
    }

    void glyph_index(const RDPGlyphIndex & glyph_index, const Rect & clip)
    {
       return;
    }

};

#endif
