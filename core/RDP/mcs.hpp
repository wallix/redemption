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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   mcs layer at core module

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

/* used in mcs */
struct mcs_channel_item {
    char name[16];
    int flags;
    int chanid;
    mcs_channel_item(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

struct Mcs {

    static void mcs_ber_out_header(Stream & stream, int tag_val, int len)
    {
        if (tag_val > 0xff) {
            stream.out_uint16_be(tag_val);
        } else {
            stream.out_uint8(tag_val);
        }
        if (len >= 0x80) {
            stream.out_uint8(0x82);
            stream.out_uint16_be(len);
        } else {
            stream.out_uint8(len);
        }
    }

    void mcs_ber_out_int8(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(value);
    }

    void mcs_ber_out_int16(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 2);
        stream.out_uint8((value >> 8));
        stream.out_uint8(value);
    }

    void mcs_ber_out_int24(Stream & stream, int value)
    {
        this->mcs_ber_out_header(stream, BER_TAG_INTEGER, 3);
        stream.out_uint8(value >> 16);
        stream.out_uint8(value >> 8);
        stream.out_uint8(value);
    }

    int ber_parse_header(Stream & stream, int tag_val) throw (Error)
    {
        #warning this should be some kind of check val stream primitive
        int tag = 0;
        if (tag_val > 0xff) {
            tag = stream.in_uint16_be();
        }
        else {
            tag = stream.in_uint8();
        }
        if (tag != tag_val) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        #warning seems to be some kind of multi bytes read. Use explicit primitive in stream.
        int l = stream.in_uint8();
        int len = l;
        if (l & 0x80) {
            len = 0;
            for (l = l & ~0x80; l > 0 ; l--) {
                len = (len << 8) | stream.in_uint8();
            }
        }
        #warning we should change check behavior here and check before accessing data, not after, use check_rem
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_HEADER_TRUNCATED);
        }
        return len;
    }


};

#endif
