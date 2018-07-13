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



    ClientRedemptionAPI(SessionReactor& session_reactor)
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
