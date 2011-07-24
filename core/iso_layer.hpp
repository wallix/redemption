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
//        LOG(LOG_INFO, "iso_send data=%p p=%p end=%p", stream.data, stream.p, stream.end);
        t->send((char*)stream.data, stream.end - stream.data);
    }

};

#endif
