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

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>



#include "utils/log.hpp"
#include "mod/rdp/rdp_log.hpp"

#include "configs/config.hpp"

#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"

#include "mod/internal/client_execute.hpp"
#include "mod/mod_api.hpp"


#endif

#define CLIENT_REDEMPTION_REPLAY_PATH "/DATA/replay"
#define CLIENT_REDEMPTION_LOGINS_PATH "/DATA/config/login.config"
#define CLIENT_REDEMPTION_WINODW_CONF_PATH "/DATA/config/windows_config.config"
#define CLIENT_REDEMPTION_SHARE_PATH "/DATA/share"
#define CLIENT_REDEMPTION_CB_FILE_TEMP_PATH "/DATA/clipboard_temp"
#define CLIENT_REDEMPTION_KEY_SETTING_PATH "/DATA/config/keySetting.config"
#define CLIENT_REDEMPTION_USER_CONF_PATH "/DATA/config/userConfig.config"
#define CLIENT_REDEMPTION_SOUND_TEMP_PATH "DATA/sound_temp"

#define CLIENT_REDEMPTION_DATA_PATH "/DATA"
#define CLIENT_REDEMPTION_DATA_CONF_PATH "/DATA/config"

#ifndef CLIENT_REDEMPTION_MAIN_PATH
//# error "undefined CLIENT_REDEMPTION_MAIN_PATH macro"
# define CLIENT_REDEMPTION_MAIN_PATH ""
#endif



class ClientRedemptionAPI : public FrontAPI
{
public:
    const std::string    MAIN_DIR = CLIENT_REDEMPTION_MAIN_PATH;
    const std::string    REPLAY_DIR = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_REPLAY_PATH;
    const std::string    USER_CONF_LOG = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_LOGINS_PATH;
    const std::string    WINDOWS_CONF = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_WINODW_CONF_PATH;
    const std::string    CB_TEMP_DIR = MAIN_DIR + CLIENT_REDEMPTION_CB_FILE_TEMP_PATH;
    std::string          SHARE_DIR = MAIN_DIR + CLIENT_REDEMPTION_SHARE_PATH;
    const std::string    USER_CONF_DIR = MAIN_DIR + CLIENT_REDEMPTION_USER_CONF_PATH;
    const std::string    SOUND_TEMP_DIR = CLIENT_REDEMPTION_SOUND_TEMP_PATH;
    const std::string    DATA_DIR = MAIN_DIR + CLIENT_REDEMPTION_DATA_PATH;
    const std::string    DATA_CONF_DIR = MAIN_DIR + CLIENT_REDEMPTION_DATA_CONF_PATH;

    mod_api            * mod = nullptr;
    ClientInfo           info;

    //  Remote App
    std::string source_of_ExeOrFile;
    std::string source_of_WorkingDir;
    std::string source_of_Arguments;
    std::string full_cmd_line;

    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    struct WindowsData {
        int form_x = 0;
        int form_y = 0;
        int screen_x = 0;
        int screen_y = 0;

        bool no_data = true;

    } windowsData;

    enum : int {
        COMMAND_VALID = 15
      , NAME_GOT      = 1
      , PWD_GOT       = 2
      , IP_GOT        = 4
      , PORT_GOT      = 8
    };

    enum : uint8_t {
        NO_PROTOCOL        = 0,
        MOD_RDP            = 1,
        MOD_VNC            = 2,
        MOD_RDP_REMOTE_APP = 3,
        MOD_RDP_REPLAY     = 4
    };

    enum : int {
        BALISED_FRAME = 15,
        MAX_ACCOUNT_DATA = 15
    };

    struct AccountData {
        std::string title;
        std::string IP;
        std::string name;
        std::string pwd;
        int port = 0;
        int options_profil = 0;
        int index = -1;
        int protocol = NO_PROTOCOL;
    }    _accountData[MAX_ACCOUNT_DATA];
    int  _accountNB = 0;
    bool _save_password_account = false;
    int  _last_target_index = 0;

    int current_user_profil = 0;

    uint8_t mod_state = MOD_RDP;

    struct UserProfil {
        int id;
        std::string name;

        UserProfil(int id, std::string name)
          : id(id)
          , name(std::move(name)) {}
    };
    std::vector<UserProfil> userProfils;

    bool enable_shared_clipboard = true;
    bool enable_shared_virtual_disk = true;
    bool enable_shared_remoteapp = false;

    struct KeyCustomDefinition {
        int qtKeyID;
        int scanCode;
        std::string ASCII8;
        int extended;
        std::string name;

        KeyCustomDefinition(int qtKeyID, int scanCode, std::string ASCII8, int extended, std::string name)
          : qtKeyID(qtKeyID)
          , scanCode(scanCode)
          , ASCII8(std::move(ASCII8))
          , extended(extended ? 0x0100 : 0)
          , name(std::move(name))
        {}
    };
    std::vector<KeyCustomDefinition> keyCustomDefinitions;

   // bool                 _recv_disconnect_ultimatum;


    struct IconMovieData {
        const std::string file_name;
        const std::string file_path;
        const std::string file_version;
        const std::string file_resolution;
        const std::string file_checksum;
        const long int movie_len = 0;

        IconMovieData(std::string file_name,
                      std::string file_path,
                      std::string file_version,
                      std::string file_resolution,
                      std::string file_checksum,
                      long int movie_len)
            : file_name(std::move(file_name))
            , file_path(std::move(file_path))
            , file_version(std::move(file_version))
            , file_resolution(std::move(file_resolution))
            , file_checksum(std::move(file_checksum))
            , movie_len(movie_len)
            {}
    };
//     std::vector<IconMovieData> icons_movie_data;


    // VNC mod
    struct ModVNCParamsData {
        bool is_apple;
        Theme      theme;
        WindowListCaps windowListCaps;
        ClientExecute exe;
        std::string vnc_encodings;
        int keylayout = 0x040C;
        int width = 800;
        int height = 600;

        bool enable_tls = false;
        bool enable_nla = false;
        bool enable_sound = false;
        bool enable_shared_clipboard = false;

        std::vector<UserProfil> userProfils;
        int current_user_profil = 0;

        ModVNCParamsData(SessionReactor& session_reactor, ClientRedemptionAPI & client)
          : is_apple(false)
          , exe(session_reactor, client, this->windowListCaps, false)
          , vnc_encodings("5,16,0,1,-239")
        {}
    } vnc_conf;

    struct ModRDPParamsData
    {
        int rdp_width;
        int rdp_height;
        bool enable_tls   = false;
        bool enable_nla   = false;
        bool enable_sound = false;
    } modRDPParamsData;

    bool                 is_recording = false;
    bool                 is_spanning = false;

    int rdp_width = 0;
    int rdp_height = 0;

    bool                 is_full_capturing = false;
    bool                 is_full_replaying = false;
    std::string          full_capture_file_name;
    bool                 is_replaying = false;
    bool                 is_loading_replay_mod = false;
    bool                 connected = false;

    std::string _movie_name;
    std::string _movie_dir;
    std::string _movie_full_path;

    uint8_t           connection_info_cmd_complete = PORT_GOT;

    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port = 3389;
    BGRPalette        mod_palette = BGRPalette::classic_332();



    explicit ClientRedemptionAPI(SessionReactor& session_reactor)
    : vnc_conf(session_reactor, *(this))
    {}

private:
    void parse_options(int argc, char const* const argv[]);

public:
    virtual ~ClientRedemptionAPI() = default;

    virtual void send_clipboard_format() {}

    void send_to_channel( const CHANNELS::ChannelDef &  /*channel*/, uint8_t const *
                         /*data*/, std::size_t  /*length*/, std::size_t  /*chunk_size*/, int  /*flags*/) override {}


    // CONTROLLER
    virtual void connect() {}
    virtual void disconnect(std::string const & /*unused*/, bool /*unused*/) {}
    virtual void replay(const std::string & /*unused*/, const std::string & /*unused*/) {}
    virtual bool load_replay_mod(std::string const & /*unused*/, std::string const & /*unused*/, timeval /*unused*/, timeval /*unused*/) { return false; }
    virtual timeval reload_replay_mod(int /*unused*/, timeval /*unused*/) { return timeval{}; }
    virtual bool is_replay_on() { return false; }
    virtual char const * get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int  /*unused*/) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}
    virtual void replay_set_pause(timeval /*unused*/) {}
    virtual void replay_set_sync() {}
    virtual void mouseButtonEvent(int  /*unused*/, int  /*unused*/, int /*unused*/) {}
    virtual void wheelEvent(int  /*unused*/,  int  /*unused*/, int /*unused*/) {}
    virtual bool mouseMouveEvent(int  /*unused*/, int  /*unused*/) {return false;}
    virtual void send_rdp_scanCode(int /*unused*/, int /*unused*/) {}
    virtual void send_rdp_unicode(uint16_t /*unused*/, uint16_t /*unused*/) {}
    virtual void refreshPressed() {}
    virtual void CtrlAltDelPressed() {}
    virtual void CtrlAltDelReleased() {}

    virtual void update_keylayout() {}

    bool can_be_start_capture() override { return true; }



    // SET CONFIG FUNCTIONS
    virtual void openWindowsData() {}
    virtual void setClientInfo() {}
    virtual void writeWindowsData() {}
    virtual void setUserProfil() {}
    virtual void setCustomKeyConfig() {}
    virtual void writeCustomKeyConfig() {}
    virtual void add_key_custom_definition(int /*unused*/, int /*unused*/, const std::string & /*unused*/, int /*unused*/, const std::string & /*unused*/) {}
    virtual void setAccountData() {}
    virtual void writeAccoundData(const std::string & /*unused*/, const std::string & /*unused*/, const std::string & /*unused*/, const int /*unused*/) {}
    virtual std::vector<IconMovieData> get_icon_movie_data() {std::vector<IconMovieData> vec; return vec;}
    virtual void set_remoteapp_cmd_line(const std::string & /*unused*/) {}
    virtual bool is_no_win_data() { return true; }
    virtual void deleteCurrentProtile() {}
    virtual void setDefaultConfig() {}
    virtual void writeClientInfo() {}


    virtual time_t get_movie_time_length(char const * /*unused*/) { return time_t{}; }
    virtual void instant_play_client(std::chrono::microseconds /*unused*/) {}
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


    void process_client_channel_out_data(const CHANNELS::ChannelNameId & front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  uint8_t const * data, const size_t data_len, uint32_t flags){

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

        if (data_len > first_part_data_size ) {

            int real_total = data_len - first_part_data_size;
            const int cmpt_PDU_part(real_total  / CHANNELS::CHANNEL_CHUNK_LENGTH);
            const int remains_PDU  (real_total  % CHANNELS::CHANNEL_CHUNK_LENGTH);
            int data_sent(0);

            // First Part
                out_stream_first_part.out_copy_bytes(data, first_part_data_size);

                data_sent += first_part_data_size;
                InStream chunk_first(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_first
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_FIRST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_first_part.get_data(), out_stream_first_part.get_offset());


            for (int i = 0; i < cmpt_PDU_part; i++) {

            // Next Part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                out_stream_next_part.out_copy_bytes(data + data_sent, CHANNELS::CHANNEL_CHUNK_LENGTH);

                data_sent += CHANNELS::CHANNEL_CHUNK_LENGTH;
                InStream chunk_next(out_stream_next_part.get_data(), out_stream_next_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_next
                                                    , total_length
                                                    , flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_next_part.get_data(), out_stream_next_part.get_offset());
            }

            // Last part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
                out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);

                InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

                this->client->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_last
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_last_part.get_data(), out_stream_last_part.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data, data_len);
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
