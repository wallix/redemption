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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CAPTURE_CHUNKTOFILE_HPP_
#define _REDEMPTION_CAPTURE_CHUNKTOFILE_HPP_

#include "RDPChunkedDevice.hpp"
#include "compression_transport_wrapper.hpp"
#include "config.hpp"
#include "wrm_label.hpp"
#include "send_wrm_chunk.hpp"

struct ChunkToFile : public RDPChunkedDevice {
private:
    CompressionOutTransportWrapper compression_wrapper;
    Transport & trans_target;
    Transport & trans;

    const Inifile & ini;

    const uint8_t wrm_format_version;

public:
    ChunkToFile(Transport * trans

               , uint16_t info_width
               , uint16_t info_height
               , uint16_t info_bpp
               , uint16_t info_cache_0_entries
               , uint16_t info_cache_0_size
               , uint16_t info_cache_1_entries
               , uint16_t info_cache_1_size
               , uint16_t info_cache_2_entries
               , uint16_t info_cache_2_size

               , uint16_t info_number_of_cache
               , bool     info_use_waiting_list

               , bool     info_cache_0_persistent
               , bool     info_cache_1_persistent
               , bool     info_cache_2_persistent

               , uint16_t info_cache_3_entries
               , uint16_t info_cache_3_size
               , bool     info_cache_3_persistent
               , uint16_t info_cache_4_entries
               , uint16_t info_cache_4_size
               , bool     info_cache_4_persistent

               , const Inifile & ini)
    : RDPChunkedDevice()
    , compression_wrapper(*trans, ini.get<cfg::video::wrm_compression_algorithm>())
    , trans_target(*trans)
    , trans(this->compression_wrapper.get())
    , ini(ini)
    , wrm_format_version(this->compression_wrapper.get_index_algorithm() ? 4 : 3)
    {
        if (this->ini.get<cfg::video::wrm_compression_algorithm>() != this->compression_wrapper.get_index_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , this->ini.get<cfg::video::wrm_compression_algorithm>().get());
        }

        send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , info_width
          , info_height
          , info_bpp
          , info_cache_0_entries
          , info_cache_0_size
          , info_cache_1_entries
          , info_cache_1_size
          , info_cache_2_entries
          , info_cache_2_size

          , info_number_of_cache
          , info_use_waiting_list

          , info_cache_0_persistent
          , info_cache_1_persistent
          , info_cache_2_persistent

          , info_cache_3_entries
          , info_cache_3_size
          , info_cache_3_persistent
          , info_cache_4_entries
          , info_cache_4_size
          , info_cache_4_persistent

          , this->compression_wrapper.get_index_algorithm()
        );
    }

public:
    void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream stream) override {
        switch (chunk_type) {
        case META_FILE:
            {
                uint16_t info_version               = stream.in_uint16_le();
                uint16_t info_width                 = stream.in_uint16_le();
                uint16_t info_height                = stream.in_uint16_le();
                uint16_t info_bpp                   = stream.in_uint16_le();
                uint16_t info_cache_0_entries       = stream.in_uint16_le();
                uint16_t info_cache_0_size          = stream.in_uint16_le();
                uint16_t info_cache_1_entries       = stream.in_uint16_le();
                uint16_t info_cache_1_size          = stream.in_uint16_le();
                uint16_t info_cache_2_entries       = stream.in_uint16_le();
                uint16_t info_cache_2_size          = stream.in_uint16_le();

                uint16_t info_number_of_cache       = 3;
                bool     info_use_waiting_list      = false;

                bool     info_cache_0_persistent    = false;
                bool     info_cache_1_persistent    = false;
                bool     info_cache_2_persistent    = false;

                uint16_t info_cache_3_entries       = 0;
                uint16_t info_cache_3_size          = 0;
                bool     info_cache_3_persistent    = false;
                uint16_t info_cache_4_entries       = 0;
                uint16_t info_cache_4_size          = 0;
                bool     info_cache_4_persistent    = false;

                if (info_version > 3) {
                    info_number_of_cache            = stream.in_uint8();
                    info_use_waiting_list           = (stream.in_uint8() ? true : false);

                    info_cache_0_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_1_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_2_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_3_entries            = stream.in_uint16_le();
                    info_cache_3_size               = stream.in_uint16_le();
                    info_cache_3_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_4_entries            = stream.in_uint16_le();
                    info_cache_4_size               = stream.in_uint16_le();
                    info_cache_4_persistent         = (stream.in_uint8() ? true : false);

                    //uint8_t info_compression_algorithm = stream.in_uint8();
                    //REDASSERT(info_compression_algorithm < 3);
                }


                send_meta_chunk(
                    this->trans_target
                  , this->wrm_format_version

                  , info_width
                  , info_height
                  , info_bpp
                  , info_cache_0_entries
                  , info_cache_0_size
                  , info_cache_1_entries
                  , info_cache_1_size
                  , info_cache_2_entries
                  , info_cache_2_size

                  , info_number_of_cache
                  , info_use_waiting_list

                  , info_cache_0_persistent
                  , info_cache_1_persistent
                  , info_cache_2_persistent

                  , info_cache_3_entries
                  , info_cache_3_size
                  , info_cache_3_persistent
                  , info_cache_4_entries
                  , info_cache_4_size
                  , info_cache_4_persistent

                  , this->compression_wrapper.get_index_algorithm()
                );
            }
            break;

        case RESET_CHUNK:
            {
                send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
                this->trans.next();
            }
            break;

        case TIMESTAMP:
            {
                timeval record_now;
                stream.in_timeval_from_uint64le_usec(record_now);
                this->trans_target.timestamp(record_now);
            }
        default:
            {
                send_wrm_chunk(this->trans, chunk_type, stream.get_capacity(), chunk_count);
                this->trans.send(stream.get_data(), stream.get_capacity());
            }
            break;
        }
    }
};

#endif
