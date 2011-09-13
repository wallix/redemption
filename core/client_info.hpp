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
#include "stream.hpp"

struct ClientInfo {
    int bpp;
    int width;
    int height;
    /* bitmap cache info */
    uint32_t cache1_entries;
    uint32_t cache1_size;
    uint32_t cache2_entries;
    uint32_t cache2_size;
    uint32_t cache3_entries;
    uint32_t cache3_size;
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

    #warning this is ugly, rewrite that
    void unicode_in(Stream & stream, int uni_len, uint8_t* dst, int dst_len) throw (Error)
    {
        int dst_index = 0;
        int src_index = 0;
        while (src_index < uni_len) {
            if (dst_index >= dst_len || src_index > 512) {
                break;
            }
            dst[dst_index] = stream.in_uint8();
            stream.skip_uint8(1);
            dst_index++;
            src_index += 2;
        }
        stream.skip_uint8(2);
    }

    void process_logon_info(Stream & stream) throw (Error)
    {
        // LOG(LOG_DEBUG, "server_sec_process_logon_info\n");
        stream.skip_uint8(4);
        int flags = stream.in_uint32_le();
        /* this is the first test that the decrypt is working */
        if ((flags & RDP_LOGON_NORMAL) != RDP_LOGON_NORMAL) /* 0x33 */
        {                                                   /* must be or error */
            throw Error(ERR_SEC_PROCESS_LOGON_UNKNOWN_FLAGS);
        }
        if (flags & RDP_LOGON_LEAVE_AUDIO) {
            this->sound_code = 1;
        }
        if ((flags & RDP_LOGON_AUTO) && (!this->is_mce))
            /* todo, for now not allowing autologon and mce both */
        {
            this->rdp_autologin = 1;
        }
        if (flags & RDP_COMPRESSION) {
            this->rdp_compression = 1;
        }
        unsigned len_domain = stream.in_uint16_le();
        unsigned len_user = stream.in_uint16_le();
        unsigned len_password = stream.in_uint16_le();
        unsigned len_program = stream.in_uint16_le();
        unsigned len_directory = stream.in_uint16_le();
        /* todo, we should error out if any of the above lengths are > 512 */
        /* to avoid buffer overruns */
        #warning check for length overflow
        unicode_in(stream, len_domain, (uint8_t*)this->domain, 255);
        unicode_in(stream, len_user, (uint8_t*)this->username, 255);
        // LOG(LOG_DEBUG, "setting username to %s\n", this->username);

        if (flags & RDP_LOGON_AUTO) {
            unicode_in(stream, len_password, (uint8_t*)this->password, 255);
        } else {
            stream.skip_uint8(len_password + 2);
        }
        unicode_in(stream, len_program, (uint8_t*)this->program, 255);
        unicode_in(stream, len_directory, (uint8_t*)this->directory, 255);
        if (flags & RDP_LOGON_BLOB) {
            stream.skip_uint8(2);                                    /* unknown */
            unsigned len_ip = stream.in_uint16_le();
            uint8_t tmpdata[256];
            unicode_in(stream, len_ip - 2, tmpdata, 255);
            unsigned len_dll = stream.in_uint16_le();
            unicode_in(stream, len_dll - 2, tmpdata, 255);
            stream.in_uint32_le(); /* len of timetone */
            stream.skip_uint8(62); /* skip */
            stream.skip_uint8(22); /* skip misc. */
            stream.skip_uint8(62); /* skip */
            stream.skip_uint8(26); /* skip stuff */
            this->rdp5_performanceflags = stream.in_uint32_le();
        }
    }

};
#endif
