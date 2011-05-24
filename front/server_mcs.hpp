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

   mcs layer at core module

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

#include "stream.hpp"
#include "constants.hpp"
#include "file_loc.hpp"
#include "log.hpp"
#include "iso_layer.hpp"

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <vector>
using namespace std;


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

/* mcs */
struct server_mcs {
    struct IsoLayer iso_layer;
    int userid;
    int chanid;
    Stream data;
    vector<struct mcs_channel_item *> channel_list;
    server_mcs(struct Transport *trans)
        : iso_layer(trans), userid(1), chanid(1001)
    {
    }

    ~server_mcs(){
        int count = (int) this->channel_list.size();
        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                delete channel_item;
            }
        }
    }

    enum {
        CHANNEL_CHUNK_LENGTH = 8192,
        CHANNEL_FLAG_FIRST = 0x01,
        CHANNEL_FLAG_LAST = 0x02,
        CHANNEL_FLAG_SHOW_PROTOCOL = 0x10,
    };

    public:

    void server_channel_init(Stream* s) throw (Error)
    {
        s->channel_hdr = s->p;
        s->p += 8;
    }

    void server_channel_send(Stream & stream, int channel_id, int total_data_len, int flags) throw (Error)
    {
        int chanid = (channel_id - MCS_GLOBAL_CHANNEL) - 1;

        struct mcs_channel_item* channel = this->get_channel(chanid);
        stream.p = stream.channel_hdr;
        stream.out_uint32_le(total_data_len);
        if (channel->flags & CHANNEL_OPTION_SHOW_PROTOCOL) {
            flags |= CHANNEL_FLAG_SHOW_PROTOCOL;
        }
        stream.out_uint32_le(flags);
        assert(channel->chanid == channel_id);
    }

    int channel_count(){
        int rv = (int) this->channel_list.size();
        return rv;
    }

    mcs_channel_item * get_channel(int index) throw (Error)
    {
        int count = (int) this->channel_list.size();
        if (index < 0 || index >= count) {
            throw Error(ERR_MCS_CHANNEL_NOT_FOUND);
        }
        return this->channel_list[index];
    }

    void server_mcs_send_channel_join_confirm_PDU(int userid, int chanid) throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_CJCF << 2) | 2);
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.out_uint16_be(chanid);
        stream.out_uint16_be(chanid);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send_attach_user_confirm_PDU(int userid) throw(Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8(((MCS_AUCF << 2) | 2));
        stream.out_uint8(0);
        stream.out_uint16_be(userid);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send_connect_response() throw(Error)
    {
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        int data_len = this->data.end - this->data.data;
        this->iso_layer.iso_init(stream);
        this->server_mcs_ber_out_header(stream, MCS_CONNECT_RESPONSE, data_len + 38);
        this->server_mcs_ber_out_header(stream, BER_TAG_RESULT, 1);
        stream.out_uint8(0);
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(0);
        this->server_mcs_out_domain_params(stream, 22, 3, 0, 0xfff8);
        this->server_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        /* mcs data */
        stream.out_copy_bytes(this->data.data, data_len);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_send(Stream & stream, int chan) throw (Error)
    {
        stream.p = stream.mcs_hdr;
        int len = (stream.end - stream.p) - 8;
        if (len > 8192 * 2) {
            LOG(LOG_ERR,
                "error in server_mcs_send, size too long, its %d (buffer=%d)\n",
                len, stream.capacity);
        }
        stream.out_uint8(MCS_SDIN << 2);
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chan);
        stream.out_uint8(0x70);
        if (len >= 128) {
            len = len | 0x8000;
            stream.out_uint16_be(len);
        }
        else {
            stream.out_uint8(len);
            /* move everything up one byte */
            uint8_t *lp = stream.p;
            while (lp < stream.end) {
                lp[0] = lp[1];
                lp++;
            }
            stream.end--;
        }
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_disconnect() throw (Error)
    {
        Stream stream(8192);
        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_DPUM << 2) | 1);
        stream.out_uint8(0x80);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    void server_mcs_init(Stream & stream)
    {
        this->iso_layer.iso_init(stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
    }

    /* returns a zero based index of the channel,
      -1 if error or if it dosen't exist */
    int server_mcs_get_channel_id(const char * name)
    {
        int rv = -1;

        int count = (int) this->channel_list.size();

        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                if (0 == strcasecmp(name, channel_item->name)) {
                    rv = index;
                    break;
                }
            }
        }
        return rv;
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

    void server_mcs_parse_domain_params(Stream & stream)
    {
        int len = this->ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS);
        stream.skip_uint8(len);
        #warning we should change check behavior here and check before accessing data, not after, use check_rem
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_HEADER_TRUNCATED);
        }
    }

    private:

    static void server_mcs_ber_out_header(Stream & stream, int tag_val, int len)
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

    void server_mcs_ber_out_int8(Stream & stream, int value)
    {
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 1);
        stream.out_uint8(value);
    }

    void server_mcs_ber_out_int24(Stream & stream, int value)
    {
        this->server_mcs_ber_out_header(stream, BER_TAG_INTEGER, 3);
        stream.out_uint8(value >> 16);
        stream.out_uint8(value >> 8);
        stream.out_uint8(value);
    }

    void server_mcs_out_domain_params(Stream & stream,
                               int max_channels,
                               int max_users, int max_tokens,
                               int max_pdu_size)
    {
        this->server_mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 26);
        this->server_mcs_ber_out_int8(stream, max_channels);
        this->server_mcs_ber_out_int8(stream, max_users);
        this->server_mcs_ber_out_int8(stream, max_tokens);
        this->server_mcs_ber_out_int8(stream, 1);
        this->server_mcs_ber_out_int8(stream, 0);
        this->server_mcs_ber_out_int8(stream, 1);
        this->server_mcs_ber_out_int24(stream, max_pdu_size);
        this->server_mcs_ber_out_int8(stream, 2);
    }
};


#endif
