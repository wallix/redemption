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

   mcs layer at rdp module

*/
#if !defined(__RDP_MCS_HPP__)
#define __RDP_MCS_HPP__

#include "iso_layer.hpp"

#include <iostream>
#include <vector>
using namespace std;


/* mcs */
struct rdp_mcs {
    struct IsoLayer iso_layer;
    int userid;
    vector<struct mcs_channel_item *> channel_list;

    rdp_mcs(Transport * t)
        : iso_layer(t), userid(1)
    {
    }

    ~rdp_mcs()
    {
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_recv(Stream & stream, int& chan) throw(Error)
    {
        this->iso_layer.iso_recv(stream);
        int opcode = stream.in_uint8();
        int appid = opcode >> 2;
        if (appid != MCS_SDIN) {
            throw Error(ERR_MCS_RECV_ID_NOT_MCS_SDIN);
        }
        stream.skip_uint8(2);
        chan = stream.in_uint16_be();
        stream.skip_uint8(1);
        int len = stream.in_uint8();
        if (len & 0x80) {
            stream.skip_uint8(1);
        }
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_ber_out_header(Stream & stream, int tag_val, int len)
    {
        if (tag_val > 0xff) {
            stream.out_uint16_be( tag_val);
        } else {
            stream.out_uint8( tag_val);
        }
        if (len >= 0x80) {
            stream.out_uint8( 0x82);
            stream.out_uint16_be( len);
        } else {
            stream.out_uint8( len);
        }
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_ber_out_int16(Stream & stream, int value)
    {
        this->rdp_mcs_ber_out_header(stream, BER_TAG_INTEGER, 2);
        stream.out_uint8( (value >> 8));
        stream.out_uint8( value);
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_out_domain_params(Stream & stream, int max_channels,
                          int max_users, int max_tokens, int max_pdu_size)
    {
        this->rdp_mcs_ber_out_header(stream, MCS_TAG_DOMAIN_PARAMS, 32);
        this->rdp_mcs_ber_out_int16(stream, max_channels);
        this->rdp_mcs_ber_out_int16(stream, max_users);
        this->rdp_mcs_ber_out_int16(stream, max_tokens);
        this->rdp_mcs_ber_out_int16(stream, 1);
        this->rdp_mcs_ber_out_int16(stream, 0);
        this->rdp_mcs_ber_out_int16(stream, 1);
        this->rdp_mcs_ber_out_int16(stream, max_pdu_size);
        this->rdp_mcs_ber_out_int16(stream, 2);
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_send_connection_initial(Stream & client_mcs_data) throw(Error)
    {
        int data_len;
        int len;
        Stream stream(8192);

        data_len = client_mcs_data.end - client_mcs_data.data;
        len = 7 + 3 * 34 + 4 + data_len;
        this->iso_layer.iso_init(stream);
        this->rdp_mcs_ber_out_header(stream, MCS_CONNECT_INITIAL, len);
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* calling domain */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, 0); /* called domain */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_BOOLEAN, 1);
        stream.out_uint8(0xff); /* upward flag */
        this->rdp_mcs_out_domain_params(stream, 34, 2, 0, 0xffff); /* target params */
        this->rdp_mcs_out_domain_params(stream, 1, 1, 1, 0x420); /* min params */
        this->rdp_mcs_out_domain_params(stream, 0xffff, 0xfc17, 0xffff, 0xffff); /* max params */
        this->rdp_mcs_ber_out_header(stream, BER_TAG_OCTET_STRING, data_len);
        stream.out_copy_bytes(client_mcs_data.data, data_len);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }
    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_ber_parse_header(Stream & stream, int tag_val, int* len) throw(Error)
    {
        int tag;
        int l;
        int i;


        if (tag_val > 0xff) {
            tag = stream.in_uint16_be();
        } else {
            tag = stream.in_uint8();
        }
        if (tag != tag_val) {
            throw Error(ERR_MCS_BER_PARSE_HEADER_VAL_NOT_MATCH);}
        l = stream.in_uint8();
        if (l & 0x80) {
            l = l & ~0x80;
            *len = 0;
            while (l > 0) {
                i = stream.in_uint8();
                *len = (*len << 8) | i;
                l--;
            }
        } else {
            *len = l;
        }
        if (!stream.check()) {
            throw Error(ERR_MCS_BER_PARSE_HEADER_ERROR_CHECKING_STREAM);;
        }
    }



    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_parse_domain_params(Stream & stream) throw(Error)
    {
        int len;

        this->rdp_mcs_ber_parse_header(stream, MCS_TAG_DOMAIN_PARAMS, &len);
        stream.skip_uint8(len);
        if (!stream.check()) {
            throw Error(ERR_MCS_PARSE_DOMAIN_PARAMS_ERROR_CHECKING_STREAM);
        }
    }

    /*****************************************************************************/

    /* returns error */
    void rdp_mcs_send_edrq() throw (Error)
    {
        Stream stream(8192);

        this->iso_layer.iso_init(stream);
        stream.out_uint8( (MCS_EDRQ << 2));
        stream.out_uint16_be( 0x100); /* height */
        stream.out_uint16_be( 0x100); /* interval */
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }


    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_send_aurq() throw (Error)
    {
        Stream stream(8192);

        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_AURQ << 2));
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }

    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_recv_aucf() throw(Error)
    {
        Stream stream(8192);

        this->iso_layer.iso_recv(stream);
        int opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_AUCF) {
            throw Error(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK);
        }
        int res = stream.in_uint8();
        if (res != 0) {
            throw Error(ERR_MCS_RECV_AUCF_RES_NOT_0);
        }
        if (opcode & 2) {
            this->userid = stream.in_uint16_be();
        }
        if (!(stream.check_end())) {
            throw Error(ERR_MCS_RECV_AUCF_ERROR_CHECKING_STREAM);
        }
    }


    /*****************************************************************************/
    /* returns error */
    void rdp_mcs_send_cjrq(int chanid) throw(Error)
    {
        Stream stream(8192);

        this->iso_layer.iso_init(stream);
        stream.out_uint8((MCS_CJRQ << 2));
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chanid);
        stream.mark_end();
        this->iso_layer.iso_send(stream);
    }


    /*****************************************************************************/
    /* returns error : channel join confirm */
    void rdp_mcs_recv_cjcf() throw(Error)
    {
        int opcode;
        Stream stream(8192);

        this->iso_layer.iso_recv(stream);
        opcode = stream.in_uint8();
        if ((opcode >> 2) != MCS_CJCF) {
            throw Error(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF);
        }
        if (0 != stream.in_uint8()) {
            throw Error(ERR_MCS_RECV_CJCF_EMPTY);
        }
        stream.skip_uint8(4); /* mcs_userid, req_chanid */
        if (opcode & 2) {
            stream.skip_uint8(2); /* join_chanid */
        }
        if (!stream.check_end()) {
            throw Error(ERR_MCS_RECV_CJCF_ERROR_CHECKING_STREAM);
        }
    }

    /*****************************************************************************/
    /* returns error */
    int rdp_mcs_init(Stream & stream)
    {
        stream.init(8192);
        this->iso_layer.iso_init(stream);
        stream.mcs_hdr = stream.p;
        stream.p += 8;
        return 0;
    }

    /* Send an MCS transport data packet to a specific channel */
    void rdp_mcs_send_to_channel(Stream & stream, int chan_id)
    {
        int len;

        stream.p = stream.mcs_hdr;
        len = (stream.end - stream.p) - 8;
        len = len | 0x8000;
        stream.out_uint8(MCS_SDRQ << 2);
        stream.out_uint16_be(this->userid);
        stream.out_uint16_be(chan_id);
        stream.out_uint8(0x70);
        stream.out_uint16_be(len);
        this->iso_layer.iso_send(stream);
    }

    /* Send an MCS transport data packet to the global channel */
    void rdp_mcs_send(Stream & stream)
    {
        this->rdp_mcs_send_to_channel(stream, MCS_GLOBAL_CHANNEL);
    }
};

#endif
