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

   Iso layer at core_module

*/

#if !defined(__ISO_LAYER_HPP__)
#define __ISO_LAYER_HPP__

#include "transport.hpp"

/* iso */
struct IsoLayer {

    uint8_t * iso_hdr;

    IsoLayer(){}

    IsoLayer(Stream & stream) {
        this->iso_hdr = stream.p;
        stream.p += 7;
    }

    ~IsoLayer(){
    }

public:

    public:
    void iso_recv(Transport * t, Stream & stream) throw (Error)
    {
        stream.init(4);
        t->recv((char**)(&(stream.end)), 4);
        int version = stream.in_uint8();
        if (3 != version) {
            throw Error(ERR_ISO_RECV_MSG_VER_NOT_3);
        }
        stream.skip_uint8(1);
        const uint16_t len = stream.in_uint16_be();

        stream.init(len - 4);
        t->recv((char**)(&(stream.end)), len - 4);

        uint8_t LI = stream.in_uint8();
        int code = stream.in_uint8();

        if (LI != ((code == ISO_PDU_DT)?2:6)){
            LOG(LOG_ERR, "Bad TPDU header header length=%u expected length=%u",
                LI, ((code == ISO_PDU_DT)?2:6));
        }
        assert( LI == ((code == ISO_PDU_DT)?2:6) ) ;
        stream.skip_uint8(LI-1);

        if (code != ISO_PDU_DT) {
            LOG(LOG_ERR, "code =%d not ISO_PDU_DT", code);
            throw Error(ERR_ISO_RECV_CODE_NOT_PDU_DT);
        }
    }

    // iso_TPDU_DT_init
    void iso_init(Stream & stream) throw (Error)
    {
        stream.p += 7;
//        LOG(LOG_INFO, "iso_init data=%p iso_hdr=%p p=%p end=%p", stream.data, this->iso_hdr, stream.p, stream.end);
    }

    // iso_TPDU_DT_send
    void iso_send(Transport * t, Stream & stream) throw (Error)
    {
        stream.p = stream.data;
        int len = stream.end - stream.p;

        // tpktHeader
        stream.out_uint8(3);  /* version */
        stream.out_uint8(0);  /* reserved */
        stream.out_uint16_be(len); /* length */

        // x224 ? 2 F0 EOT
        stream.out_uint8(2);
        stream.out_uint8(ISO_PDU_DT);
        stream.out_uint8(0x80); // EOT ?
//        LOG(LOG_INFO, "iso_send data=%p iso_hdr=%p p=%p end=%p", stream.data, this->iso_hdr, stream.p, stream.end);
        t->send((char*)stream.data, stream.end - stream.data);
    }

};

#endif
