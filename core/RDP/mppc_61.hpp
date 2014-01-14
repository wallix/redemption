/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_MPPC_61_HPP_
#define _REDEMPTION_CORE_RDP_MPPC_61_HPP_

static const size_t RDP_61_HISTORY_BUFFER_LENGTH = 2000000;
static const size_t RDP_61_MAX_DATA_BLOCK_SIZE   = 16382;

struct rdp_mppc_61_enc : public rdp_mppc_enc {
    uint8_t * historyBuffer;    // Livel-1 history buffer.
    size_t    historyOffset;    // Livel-1 history buffer associated history offset.

    rdp_mppc_50_enc level_2_compressor;

    /**
     * Initialize rdp_mppc_61_enc structure
     */
    rdp_mppc_61_enc() {
        this->historyBuffer = static_cast<uint8_t *>(calloc(RDP_61_HISTORY_BUFFER_LENGTH, 1));
        this->historyOffset = 0;
    }

    /**
     * Deinitialize rdp_mppc_61_enc structure
     */
    virtual ~rdp_mppc_61_enc() {
        free(this->historyBuffer);
    }

    virtual bool compress_61(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size) {
        //LOG(LOG_INFO, "compress_61");

        if (uncompressed_data_size > RDP_61_MAX_DATA_BLOCK_SIZE)
            return true;

        return false;
    }

    virtual bool compress(const uint8_t * uncompressed_data, uint16_t uncompressed_data_size, uint8_t & flags, uint16_t & compressedLength) {
        return false;
    }

    virtual void get_compressed_data(Stream & stream) const {
    }

    virtual void mini_dump() {
    }

    virtual void dump() {
    }
};  // struct rdp_mppc_61_enc

#endif  // #ifndef _REDEMPTION_CORE_RDP_MPPC_61_HPP_