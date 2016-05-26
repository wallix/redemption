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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean, Clement Moroldo

*/

#pragma once


#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/bitfu.hpp"



class SslRC4_direct
{

    struct rc4 {
       uint32_t x, y;
       uint32_t data[256];
    } rc4;

    void SK_LOOP(uint32_t * d, unsigned int n, int & id1, int & id2, const unsigned char *data, int len) {
        int tmp=d[(n)];
        id2 = (data[id1] + tmp + id2) & 0xff;
        if (++id1 == len) id1=0;
        d[(n)]=d[id2];
        d[id2]=tmp;
    }


    void RC4_set_key(struct rc4 *key, int len, const unsigned char *data)
    {
        uint32_t tmp;
        int id1, id2;
        uint32_t *d;
        unsigned int i;

        d = &(key->data[0]);
        key->x = 0;
        key->y = 0;
        id1 = id2 = 0;

        for (i = 0; i < 256; i++){
            d[i] = i;
        }
        for (i = 0; i < 256; i += 4) {
            SK_LOOP(d, i + 0, id1, id2, data, len);
            SK_LOOP(d, i + 1, id1, id2, data, len);
            SK_LOOP(d, i + 2, id1, id2, data, len);
            SK_LOOP(d, i + 3, id1, id2, data, len);
        }
    }

    /*-
     * RC4 as implemented from a posting from
     * Newsgroups: sci.crypt
     * From: sterndark@netcom.com (David Sterndark)
     * Subject: RC4 Algorithm revealed.
     * Message-ID: <sternCvKL4B.Hyy@netcom.com>
     * Date: Wed, 14 Sep 1994 06:35:31 GMT
     */

    void LOOP(const unsigned char in, unsigned char & out, uint32_t * d, uint32_t & x, uint32_t & y, uint32_t & tx, uint32_t ty) {
        x=((x+1)&0xff);
        tx=d[x];
        y=(tx+y)&0xff;
        d[x]=ty=d[y];
        d[y]=tx;
        (out) = d[(tx+ty)&0xff]^ (in);
    }

    void RC4(struct rc4 *key, size_t len, const unsigned char *indata,
             unsigned char *outdata)
    {
        uint32_t *d;
        uint32_t x, y, tx, ty;
        size_t i;

        x = key->x;
        y = key->y;
        d = key->data;

        i = len >> 3;
        if (i) {
            for (;;) {
                LOOP(indata[0], outdata[0], d, x, y, tx, ty);
                LOOP(indata[1], outdata[1], d, x, y, tx, ty);
                LOOP(indata[2], outdata[2], d, x, y, tx, ty);
                LOOP(indata[3], outdata[3], d, x, y, tx, ty);
                LOOP(indata[4], outdata[4], d, x, y, tx, ty);
                LOOP(indata[5], outdata[5], d, x, y, tx, ty);
                LOOP(indata[6], outdata[6], d, x, y, tx, ty);
                LOOP(indata[7], outdata[7], d, x, y, tx, ty);
                indata += 8;
                outdata += 8;
                if (--i == 0)
                    break;
            }
        }
        i = len & 0x07;
        if (i) {
            for (;;) {
                LOOP(indata[0], outdata[0], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[1], outdata[1], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[2], outdata[2], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[3], outdata[3], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[4], outdata[4], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[5], outdata[5], d, x, y, tx, ty);
                if (--i == 0)
                    break;
                LOOP(indata[6], outdata[6], d, x, y, tx, ty);
                if (--i == 0)
                    break;
            }
        }
        key->x = x;
        key->y = y;
    }



    public:
    SslRC4_direct(){}

    void set_key(const uint8_t * const key,  size_t key_size)
    {
        SslRC4_direct::RC4_set_key(&(this->rc4), key_size, key);

    }

    void crypt(size_t data_size, const uint8_t * const indata, uint8_t * const outdata){
        SslRC4_direct::RC4(&(this->rc4), data_size, indata, outdata);
    }
};


typedef SslRC4_direct SslRC4;


