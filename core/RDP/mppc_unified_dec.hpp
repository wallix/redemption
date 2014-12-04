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
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_CORE_RDP_MPPC_UNIFIED_DEC_HPP
#define REDEMPTION_CORE_RDP_MPPC_UNIFIED_DEC_HPP

#include "mppc_40.hpp"
#include "mppc_50.hpp"
#include "mppc_60.hpp"
#include "mppc_61.hpp"


struct rdp_mppc_unified_dec : public rdp_mppc_dec {
protected:
    rdp_mppc_40_dec * dec_40;
    rdp_mppc_50_dec * dec_50;
    rdp_mppc_60_dec * dec_60;
    rdp_mppc_61_dec * dec_61;

public:
    /**
     * Initialize rdp_mppc_unified_dec structure
     */
    rdp_mppc_unified_dec() {
        this->dec_40 = NULL;
        this->dec_50 = NULL;
        this->dec_60 = NULL;
        this->dec_61 = NULL;
    }

    /**
     * Deinitialize rdp_mppc_unified_dec structure
     */
    virtual ~rdp_mppc_unified_dec() {
        if (this->dec_40) {
            delete this->dec_40;
        }

        if (this->dec_50) {
            delete this->dec_50;
        }

        if (this->dec_60) {
            delete this->dec_60;
        }

        if (this->dec_61) {
            delete this->dec_61;
        }
    }

    virtual void mini_dump() {
        if (this->dec_40) {
            this->dec_40->mini_dump();
        }
        else if (this->dec_50) {
            this->dec_50->mini_dump();
        }
        else if (this->dec_60) {
            this->dec_60->mini_dump();
        }
        else if (this->dec_61) {
            this->dec_61->mini_dump();
        }
    }

    virtual void dump() {
        if (this->dec_40) {
            this->dec_40->dump();
        }
        else if (this->dec_50) {
            this->dec_50->dump();
        }
        else if (this->dec_60) {
            this->dec_60->dump();
        }
        else if (this->dec_61) {
            this->dec_61->dump();
        }
    }

    int decompress(uint8_t * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen) {
        int type = ctype & 0x0f;

        switch (type) {
            case PACKET_COMPR_TYPE_8K:
                if (!this->dec_40) {
                    this->dec_40 = new rdp_mppc_40_dec();
                }
                return this->dec_40->decompress(cbuf, len, ctype, rdata, rlen);

            case PACKET_COMPR_TYPE_64K:
                if (!this->dec_50) {
                    this->dec_50 = new rdp_mppc_50_dec();
                }
                return this->dec_50->decompress(cbuf, len, ctype, rdata, rlen);

            case PACKET_COMPR_TYPE_RDP6:
                if (!this->dec_60) {
                    this->dec_60 = new rdp_mppc_60_dec();
                }
                return this->dec_60->decompress(cbuf, len, ctype, rdata, rlen);

            case PACKET_COMPR_TYPE_RDP61:
                if (!this->dec_61) {
                    this->dec_61 = new rdp_mppc_61_dec();
                }
                return this->dec_61->decompress(cbuf, len, ctype, rdata, rlen);
        }

        LOG(LOG_ERR, "rdp_mppc_unified_dec::decompress: invalid RDP compression code 0x%2.2x", type);
        return false;
    }
};  // struct rdp_mppc_unified_dec

#endif
