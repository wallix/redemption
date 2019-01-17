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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Implementation of Microsoft Point to Point Compression (MPPC) protocol

   Based on code by Laxmikant Rashinkar & Jiten Pathy from FreeRDP project
   Copyright 2011 Laxmikant Rashinkar <LK.Rashinkar@gmail.com>
   Copyright 2012 Jiten Pathy
*/

#include "core/RDP/mppc.hpp"
#include "core/RDP/mppc/mppc_40.hpp"
#include "core/RDP/mppc/mppc_50.hpp"
#include "core/RDP/mppc/mppc_60.hpp"
#include "core/RDP/mppc/mppc_61.hpp"

rdp_mppc_enc::rdp_mppc_enc(uint32_t max_data_block_size, bool verbose)
    : total_uncompressed_data_size(0)
    , total_compressed_data_size(0)
    , max_data_block_size(max_data_block_size)
    , verbose(verbose)
{
}

void rdp_mppc_enc::compress(
    const uint8_t * uncompressed_data, uint16_t uncompressed_data_size,
    uint8_t & compressedType, uint16_t & compressed_data_size,
    uint16_t max_compressed_data_size)
{
    this->total_uncompressed_data_size += uncompressed_data_size;
    this->do_compress(uncompressed_data, uncompressed_data_size,
        compressedType, compressed_data_size, max_compressed_data_size);

    this->total_compressed_data_size +=
        ((compressedType & PACKET_COMPRESSED) ? compressed_data_size :
            uncompressed_data_size);

    if (this->verbose) {
        LOG(LOG_INFO, "compressedType=0x%02X", static_cast<unsigned>(compressedType));
        LOG(LOG_INFO, "uncompressed_data_size=%" PRIu16 " compressed_data_size=%" PRIu16 " rate=%.2f",
            uncompressed_data_size, compressed_data_size,
            compressed_data_size * 100.0 / uncompressed_data_size);
        LOG(LOG_INFO, "total_uncompressed_data_size=%" PRIu64 " total_compressed_data_size=%" PRIu64 " total_rate=%.2Lf",
            total_uncompressed_data_size, total_compressed_data_size,
            static_cast<long double>(total_compressed_data_size) * 100
            / static_cast<long double>(total_uncompressed_data_size));
    }
}

uint32_t rdp_mppc_enc::get_max_data_block_size() const
{
    return this->max_data_block_size;
}


rdp_mppc_unified_dec::~rdp_mppc_unified_dec()
{
    delete this->mppc_dec;
}

int rdp_mppc_unified_dec::decompress(
    uint8_t const * cbuf, int len, int ctype, const uint8_t *& rdata, uint32_t & rlen)
{
    if (!this->mppc_dec) {
        const int type = ctype & 0x0f;
        switch (type) {
            case PACKET_COMPR_TYPE_8K: this->mppc_dec = new rdp_mppc_40_dec; break;
            case PACKET_COMPR_TYPE_64K: this->mppc_dec = new rdp_mppc_50_dec; break;
            case PACKET_COMPR_TYPE_RDP6: this->mppc_dec = new rdp_mppc_60_dec; break;
            case PACKET_COMPR_TYPE_RDP61: this->mppc_dec = new rdp_mppc_61_dec; break;
            default:
                LOG(LOG_ERR, "rdp_mppc_unified_dec::decompress: invalid RDP compression code 0x%2.2x", unsigned(type));
                return false;
        }
    }

    return this->mppc_dec->decompress(cbuf, len, ctype, rdata, rlen);
}

std::unique_ptr<rdp_mppc_enc>
rdp_mppc_load_compressor(bool log_trace, char const * trace, int type, bool verbose)
{
    switch (type) {
        case PACKET_COMPR_TYPE_8K:
            if (log_trace) {
                LOG(LOG_INFO, "%s: Use RDP 4.0 Bulk compression", trace);
            }
            return std::make_unique<rdp_mppc_40_enc>(verbose);
        case PACKET_COMPR_TYPE_64K:
            if (log_trace) {
                LOG(LOG_INFO, "%s: Use RDP 5.0 Bulk compression", trace);
            }
            return std::make_unique<rdp_mppc_50_enc>(verbose);
        case PACKET_COMPR_TYPE_RDP6:
            if (log_trace) {
                LOG(LOG_INFO, "%s: Use RDP 6.0 Bulk compression", trace);
            }
            return std::make_unique<rdp_mppc_60_enc>(verbose);
        case PACKET_COMPR_TYPE_RDP61:
            if (log_trace) {
                LOG(LOG_INFO, "%s: Use RDP 6.1 Bulk compression", trace);
            }
            return std::make_unique<rdp_mppc_61_enc_hash_based>(verbose);
        default:
            LOG(LOG_ERR, "%s: invalid RDP compression code 0x%2.2x", trace, unsigned(type));
            return {};
    }
}
