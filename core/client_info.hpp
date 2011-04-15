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

   header file for use with libxrdp.so / xrdp.dll

*/

#if !defined(__CLIENT_INFO_HPP__)
#define __CLIENT_INFO_HPP__

#include <string.h>
#include "config.hpp"

struct ClientInfo {
    int bpp;
    int width;
    int height;
    /* bitmap cache info */
    int cache1_entries;
    int cache1_size;
    int cache2_entries;
    int cache2_size;
    int cache3_entries;
    int cache3_size;
    int bitmap_cache_persist_enable; /* 0 or 2 */
    int bitmap_cache_version; /* 0 = original version, 2 = v2 */
    /* pointer info */
    int pointer_cache_entries;
    /* other */
    int use_bitmap_comp;
    int use_bitmap_cache;
    int op1; /* use smaller bitmap header, non cache */
    int op2; /* use smaller bitmap header in bitmap cache */
    int desktop_cache;
    int use_compact_packets; /* rdp5 smaller packets */
    char hostname[32];
    int build;
    int keylayout;
    char username[256];
    char password[256];
    char domain[256];
    char program[256];
    char directory[256];
    int rdp_compression;
    int rdp_autologin;
    int crypt_level; /* 1, 2, 3 = low, medium, high */
    int channel_code; /* 0 = no channels 1 = channels */
    int sound_code; /* 1 = leave sound at server */
    int is_mce;
    int rdp5_performanceflags;
    int brush_cache_code; /* 0 = no cache 1 = 8x8 standard cache
                           2 = arbitrary dimensions */
    bool console_session;

    ClientInfo(Inifile * ini) {
        this->bpp = 0;
        this->width = 0;
        this->height = 0;
        /* bitmap cache info */
        /* default 8 bit v1 color bitmap cache entries and size */
        this->cache1_entries = 600;
        this->cache1_size = 256;
        this->cache2_entries = 300;
        this->cache2_size = 1024;
        this->cache3_entries = 262;
        this->cache3_size = 4096;

        this->bitmap_cache_persist_enable = 0; /* 0 or 2 */
        this->bitmap_cache_version = 0; /* 0 = original version, 2 = v2 */
        /* pointer info */
        this->pointer_cache_entries = 0;
        /* other */
        this->op1 = 0; /* use smaller bitmap header, non cache */
        this->op2 = 0; /* use smaller bitmap header in bitmap cache */
        this->desktop_cache = 0;
        this->use_compact_packets = 0; /* rdp5 smaller packets */
        memset(this->hostname, 0, 32);
        this->build = 0;
        this->keylayout = 0;
        memset(this->username, 0, 256);
        memset(this->password, 0, 256);
        memset(this->domain, 0, 256);
        memset(this->program, 0, 256);
        memset(this->directory, 0, 256);
        this->rdp_compression = 0;
        this->rdp_autologin = 0;
        this->sound_code = 0; /* 1 = leave sound at server */
        this->is_mce = 0;
        this->rdp5_performanceflags = 0;
        this->brush_cache_code = 0; /* 0 = no cache 1 = 8x8 standard cache
                               2 = arbitrary dimensions */
        this->console_session = false;

        /* read ini settings */
        this->use_bitmap_cache = 0;
        if (ini->globals.bitmap_cache){
            this->use_bitmap_cache = 1;
        }

        this->use_bitmap_comp = 0;
        if (ini->globals.bitmap_compression){
            this->use_bitmap_comp = 1;
        }
        /*crypt_level: 1, 2, 3 = low, medium, high */
        this->crypt_level = ini->globals.crypt_level + 1;

        /* channel_code : 0 = no channels 1 = channels */
        this->channel_code = ini->globals.channel_code;
    }
};
#endif
