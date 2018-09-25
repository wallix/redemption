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
   Author(s): Clément Moroldo, David Fort
*/

#pragma once


#ifndef Q_MOC_RUN

// #include <fstream>
// #include <iostream>
// #include <sstream>
// #include <cstdio>
// #include <dirent.h>
// #include <unistd.h>


#include "utils/log.hpp"
#include "mod/rdp/rdp_log.hpp"

#include "configs/config.hpp"

// #include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"

#include "mod/internal/client_execute.hpp"
#include "mod/mod_api.hpp"


#include "mod/internal/replay_mod.hpp"
#include "transport/replay_transport.hpp"

#endif




//#include "client_redemption/client_redemption_config.hpp"


class ClientRedemptionAPI : public FrontAPI
{
public:

//     ClientRedemptionConfig config;


    mod_api            * mod = nullptr;



private:
    void parse_options(int argc, char const* const argv[]);

public:

    ClientRedemptionAPI(/*SessionReactor& session_reactor, char const* argv[], int argc, RDPVerbose verbose*/)
     /* : config(session_reactor, argv, argc, verbose, *(this))*/{}

    virtual ~ClientRedemptionAPI() = default;

    virtual void send_clipboard_format() {}

    void send_to_channel( const CHANNELS::ChannelDef &  /*channel*/, uint8_t const *
                         /*data*/, std::size_t  /*length*/, std::size_t  /*chunk_size*/, int  /*flags*/) override {}

    virtual int wait_and_draw_event(timeval timeout) = 0;

    // CONTROLLER
    virtual bool connect() {return true;}
    virtual void disconnect(std::string const & /*unused*/, bool /*unused*/) {}

    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int  /*unused*/) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}

//     virtual void mouseButtonEvent(int  /*unused*/, int  /*unused*/, int /*unused*/) {}
//     virtual void wheelEvent(int  /*unused*/,  int  /*unused*/, int /*unused*/) {}
//     virtual bool mouseMouveEvent(int  /*unused*/, int  /*unused*/) {return false;}
//     virtual void send_rdp_scanCode(int /*unused*/, int /*unused*/) {}
//     virtual void send_rdp_unicode(uint16_t /*unused*/, uint16_t /*unused*/) {}
//     virtual void refreshPressed() {}
//     virtual void CtrlAltDelPressed() {}
//     virtual void CtrlAltDelReleased() {}

    virtual void update_keylayout() {}

    bool can_be_start_capture() override { return true; }


    // Replay functions
    virtual time_t get_movie_time_length(char const * /*unused*/) { return time_t{}; }
    virtual void instant_play_client(std::chrono::microseconds /*unused*/) {}
    virtual void replay(const std::string & /*unused*/, const std::string & /*unused*/) {}
    virtual bool load_replay_mod(std::string const & /*unused*/, std::string const & /*unused*/, timeval /*unused*/, timeval /*unused*/) { return false; }
    virtual timeval reload_replay_mod(int /*unused*/, timeval /*unused*/) { return timeval{}; }
    virtual bool is_replay_on() { return false; }
    virtual char const * get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void replay_set_pause(timeval /*unused*/) {}
    virtual void replay_set_sync() {}
    //virtual std::vector<IconMovieData> get_icon_movie_data() { std::vector<IconMovieData> icons; return icons; }
};




class ClientIO
{
public:
    ClientRedemptionAPI * client;

    void set_client(ClientRedemptionAPI * client) {
        this->client = client;
    }
};



#include "core/channel_list.hpp"


class ClientChannelManager
{
public:
    ClientRedemptionAPI * client;

    ClientChannelManager(ClientRedemptionAPI * client)
      : client(client) {}


    void process_client_channel_out_data(const CHANNELS::ChannelNameId & front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  const_bytes_view data, uint32_t flags){

        // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Dat

        // Virtual channel data can span multiple Virtual Channel PDUs (section 3.1.5.2.1).
        // If this is the case, the embedded length field of the channelPduHeader field
        // (the Channel PDU Header structure is specified in section 2.2.6.1.1) specifies
        // the total length of the uncompressed virtual channel data spanned across all of
        // the associated Virtual Channel PDUs. This length is referred to as totalLength.
        // For example, assume that the virtual channel chunking size specified in the Virtual
        // Channel Capability Set (section 2.2.7.1.10) is 1,000 bytes and that 2,062 bytes need
        // to be transmitted on a given virtual channel. In this example,
        // the following sequence of Virtual Channel PDUs will be sent (only relevant fields are listed):

        //    Virtual Channel PDU 1:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_FIRST
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 2:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = 0
        //    Actual virtual channel data is 1000 bytes (the chunking size).

        //    Virtual Channel PDU 3:
        //    CHANNEL_PDU_HEADER::length = 2062 bytes
        //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_LAST
        //    Actual virtual channel data is 62 bytes.

    //     // The size of the virtual channel data in the last PDU (the data in the virtualChannelData field)
        // is determined by subtracting the offset of the virtualChannelData field in the encapsulating
        // Virtual Channel PDU from the total size specified in the tpktHeader field. This length is
        // referred to as chunkLength.

        // Upon receiving each Virtual Channel PDU, the server MUST dispatch the virtual channel data to
        // the appropriate virtual channel endpoint. The sequencing of the chunk (whether it is first,
        // intermediate, or last), totalLength, chunkLength, and the virtualChannelData fields MUST
        // be dispatched to the virtual channel endpoint so that the data can be correctly reassembled.
        // If the CHANNEL_FLAG_SHOW_PROTOCOL (0x00000010) flag is specified in the Channel PDU Header,
        // then the channelPduHeader field MUST also be dispatched to the virtual channel endpoint.

        // A reassembly buffer MUST be created by the virtual channel endpoint using the size specified
        // by totalLength when the first chunk is received. After the reassembly buffer has been created
        // the first chunk MUST be copied into the front of the buffer. Subsequent chunks MUST then be
        // copied into the reassembly buffer in the order in which they are received. Upon receiving the
        // last chunk of virtual channel data, the reassembled data is processed by the virtual channel endpoint.


        if (data.size() > first_part_data_size ) {

            int real_total = data.size() - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
                out_stream_first_part.out_copy_bytes(data.data(), first_part_data_size);

                data_sent += first_part_data_size;
                InStream chunk_first(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_first
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_FIRST | flags
                                                    );

//             ::hexdump(out_stream_first_part.get_data(), out_stream_first_part.get_offset());


            for (int i = 0; i < cmpt_PDU_part; i++) {

            // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data.data() + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_data(), out_stream_next_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_next
                                                    , total_length
                                                    , flags
                                                    );

//             ::hexdump(out_stream_next_part.get_data(), out_stream_next_part.get_offset());
            }

            // Last part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
                out_stream_last_part.out_copy_bytes(data.data() + data_sent, remains_PDU);

                InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_last
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                    );

//         ::hexdump(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data.data(), data.size());
            InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

            this->client->mod->send_to_mod_channel( front_channel_name
                                                , chunk
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                                  flags
                                                );
        }
    }
};
