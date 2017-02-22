/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen
*/


#pragma once

#include "mppc_40.hpp"
#include "mppc_50.hpp"
#include "mppc_60.hpp"
#include "mppc_61.hpp"


class rdp_mppc_unified_dec : public rdp_mppc_dec {
    rdp_mppc_dec * mppc_dec = nullptr;

public:
    rdp_mppc_unified_dec() = default;

    ~rdp_mppc_unified_dec() override {
        delete this->mppc_dec;
    }

    void mini_dump() override {
        if (this->mppc_dec) {
            this->mppc_dec->mini_dump();
        }
    }

    void dump() override {
        if (this->mppc_dec) {
            this->mppc_dec->dump();
        }
    }

    int decompress(uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) override {
        if (!this->mppc_dec) {
            const int type = ctype & 0x0f;
            switch (type) {
                case PACKET_COMPR_TYPE_8K: this->mppc_dec = new rdp_mppc_40_dec; break;
                case PACKET_COMPR_TYPE_64K: this->mppc_dec = new rdp_mppc_50_dec; break;
                case PACKET_COMPR_TYPE_RDP6: this->mppc_dec = new rdp_mppc_60_dec; break;
                case PACKET_COMPR_TYPE_RDP61: this->mppc_dec = new rdp_mppc_61_dec; break;
                default:
                    LOG(LOG_ERR, "rdp_mppc_unified_dec::decompress: invalid RDP compression code 0x%2.2x", type);
                    return false;
            }
        }

        return this->mppc_dec->decompress(cbuf, len, ctype, rdata, rlen);
    }
};  // struct rdp_mppc_unified_dec

