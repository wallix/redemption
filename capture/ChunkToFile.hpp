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
#include "gzip_compression_transport.hpp"
//#include "lzma_compression_transport.hpp"
#include "snappy_compression_transport.hpp"

struct ChunkToFile : public RDPChunkedDevice {
private:
    Transport * trans_target;
    Transport * trans;

    const Inifile & ini;

    GZipCompressionOutTransport   gzcot;
    //LzmaCompressionOutTransport   lcot;
    SnappyCompressionOutTransport scot;

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
    , trans_target(trans)
    , trans(trans)
    , ini(ini)
    , gzcot(*trans)
    //, lcot(*trans, true, ini.debug.capture)
    , scot(*trans)
    //, wrm_format_version(((ini.video.wrm_compression_algorithm > 0) && (ini.video.wrm_compression_algorithm < 4)) ? 4 : 3)
    , wrm_format_version(((ini.video.wrm_compression_algorithm > 0) && (ini.video.wrm_compression_algorithm < 3)) ? 4 : 3)
    {
        //REDASSERT(this->ini.video.wrm_compression_algorithm < 4);
        REDASSERT(this->ini.video.wrm_compression_algorithm < 3);

        if (this->ini.video.wrm_compression_algorithm == 1) {
            this->trans = &this->gzcot;
        }
        else if (this->ini.video.wrm_compression_algorithm == 2) {
            this->trans = &this->scot;
        }
        //else if (this->ini.video.wrm_compression_algorithm == 3) {
        //    this->trans = &this->lcot;
        //}

        this->send_meta_chunk( info_width
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
                             );
    }

private:
    void send_meta_chunk( uint16_t info_width
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
                        , bool     info_cache_4_persistent) {
        BStream payload(36);
        payload.out_uint16_le(this->wrm_format_version);
        payload.out_uint16_le(info_width);
        payload.out_uint16_le(info_height);
        payload.out_uint16_le(info_bpp);

        payload.out_uint16_le(info_cache_0_entries);
        payload.out_uint16_le(info_cache_0_size);
        payload.out_uint16_le(info_cache_1_entries);
        payload.out_uint16_le(info_cache_1_size);
        payload.out_uint16_le(info_cache_2_entries);
        payload.out_uint16_le(info_cache_2_size);

        if (this->wrm_format_version > 3) {
            payload.out_uint8(info_number_of_cache);
            payload.out_uint8(info_use_waiting_list);

            payload.out_uint8(info_cache_0_persistent);
            payload.out_uint8(info_cache_1_persistent);
            payload.out_uint8(info_cache_2_persistent);

            payload.out_uint16_le(info_cache_3_entries);
            payload.out_uint16_le(info_cache_3_size);
            payload.out_uint8(info_cache_3_persistent);
            payload.out_uint16_le(info_cache_4_entries);
            payload.out_uint16_le(info_cache_4_size);
            payload.out_uint8(info_cache_4_persistent);

            //payload.out_uint8((ini.video.wrm_compression_algorithm < 4) ? this->ini.video.wrm_compression_algorithm : 0);
            payload.out_uint8((ini.video.wrm_compression_algorithm < 3) ? this->ini.video.wrm_compression_algorithm : 0);
        }
        payload.mark_end();

        BStream header(8);
        WRMChunk_Send chunk(header, META_FILE, payload.size(), 1);

        this->trans_target->send(header);
        this->trans_target->send(payload);
    }

public:
    virtual void chunk(uint16_t chunk_type, uint16_t chunk_count, const Stream & data) {
        switch (chunk_type) {
        case META_FILE:
            {
                StaticStream stream(data.p, data.size());

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

                this->send_meta_chunk( info_width
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
                                     );
            }
            break;

        case RESET_CHUNK:
            {
                BStream header(8);
                WRMChunk_Send chunk(header, RESET_CHUNK, 0, 1);

                this->trans->send(header);
                this->trans->next();
            }
            break;

        default:
            {
                FixedSizeStream payload(data.p, data.size());

                BStream header(8);
                WRMChunk_Send chunk(header, chunk_type, payload.size(), chunk_count);

                this->trans->send(header);
                this->trans->send(payload);
            }
            break;
        }
    }
};

#endif
