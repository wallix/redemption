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

    void mcs_ber_out_int8(Stream & stream, int value)
    {
    }

    void mcs_ber_out_int16(Stream & stream, int value)
    {
        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_ber_len(2);
        stream.out_uint8((value >> 8));
        stream.out_uint8(value);
    }

    void mcs_ber_out_int24(Stream & stream, int value)
    {
        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_ber_len(3);
        stream.out_uint8(value >> 16);
        stream.out_uint8(value >> 8);
        stream.out_uint8(value);
    }

    void mcs_recv_connection_initial(Stream & data, Transport * trans)
    {
        Stream stream(8192);
        X224In(trans, stream);

        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_INITIAL) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();
        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);
        if (stream.in_uint8() != BER_TAG_BOOLEAN) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        /* make a copy of client mcs data */
        data.init(len);
        data.out_copy_bytes(stream.p, len);
        data.mark_end();
        stream.skip_uint8(len);
    }

    void mcs_send_connection_initial(Stream & client_mcs_data, Transport * trans) throw(Error)
    {
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = client_mcs_data.end - client_mcs_data.data;
        int len = 7 + 3 * 34 + 4 + data_len;
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_INITIAL);
        stream.out_ber_len(len);
        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(0); /* calling domain */
        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(0); /* called domain */
        stream.out_uint8(BER_TAG_BOOLEAN);
        stream.out_ber_len(1);
        stream.out_uint8(0xff); /* upward flag */

        // target params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(34);     // max_channels
        stream.out_ber_int16(2);      // max_users
        stream.out_ber_int16(0);      // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0xffff); // max_pdu_size
        stream.out_ber_int16(2);

        // min params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(1);     // max_channels
        stream.out_ber_int16(1);     // max_users
        stream.out_ber_int16(1);     // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0x420); // max_pdu_size
        stream.out_ber_int16(2);

        // max params
        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(32);
        stream.out_ber_int16(0xffff); // max_channels
        stream.out_ber_int16(0xfc17); // max_users
        stream.out_ber_int16(0xffff); // max_tokens
        stream.out_ber_int16(1);
        stream.out_ber_int16(0);
        stream.out_ber_int16(1);
        stream.out_ber_int16(0xffff); // max_pdu_size
        stream.out_ber_int16(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(data_len);
        stream.out_copy_bytes(client_mcs_data.data, data_len);

        tpdu.end();
        tpdu.send(trans);
    }

    void mcs_recv_connect_response(Stream & stream, Transport * trans) throw(Error)
    {
        X224In(trans, stream);
        if (stream.in_uint16_be() != BER_TAG_MCS_CONNECT_RESPONSE) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        int len = stream.in_ber_len();

        if (stream.in_uint8() != BER_TAG_RESULT) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();

        int res = stream.in_uint8();

        if (res != 0) {
            throw Error(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0);
        }
        if (stream.in_uint8() != BER_TAG_INTEGER) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len); /* connect id */

        if (stream.in_uint8() != BER_TAG_MCS_DOMAIN_PARAMS) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
        stream.skip_uint8(len);

        if (stream.in_uint8() != BER_TAG_OCTET_STRING) {
            throw Error(ERR_MCS_BER_HEADER_UNEXPECTED_TAG);
        }
        len = stream.in_ber_len();
    }

    void mcs_send_connect_response(Stream & data, Transport * trans) throw(Error)
    {
//        LOG(LOG_INFO, "server_mcs_send_connect_response");
        #warning why don't we build directly in final data buffer ? Instead of building in data and copying in stream ?
        Stream stream(8192);
        X224Out tpdu(X224Packet::DT_TPDU, stream);

        int data_len = data.end - data.data;
        stream.out_uint16_be(BER_TAG_MCS_CONNECT_RESPONSE);
        stream.out_ber_len(data_len + 38);
        stream.out_uint8(BER_TAG_RESULT);
        stream.out_ber_len(1);
        stream.out_uint8(0);
        stream.out_uint8(BER_TAG_INTEGER);
        stream.out_ber_len(1);
        stream.out_uint8(0);

        stream.out_uint8(BER_TAG_MCS_DOMAIN_PARAMS);
        stream.out_ber_len(26);
        stream.out_ber_int8(22); // max_channels
        stream.out_ber_int8(3); // max_users
        stream.out_ber_int8(0); // max_tokens
        stream.out_ber_int8(1);
        stream.out_ber_int8(0);
        stream.out_ber_int8(1);
        this->mcs_ber_out_int24(stream, 0xfff8); // max_pdu_size
        stream.out_ber_int8(2);

        stream.out_uint8(BER_TAG_OCTET_STRING);
        stream.out_ber_len(data_len);
        /* mcs data */
        stream.out_copy_bytes(data.data, data_len);

        tpdu.end();
        tpdu.send(trans);
    }

};

#endif
