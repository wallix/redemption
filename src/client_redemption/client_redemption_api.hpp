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
    const std::string    MAIN_DIR;
    const std::string    REPLAY_DIR;
    const std::string    USER_CONF_LOG;
    const std::string    WINDOWS_CONF;
    const std::string    CB_TEMP_DIR;
    std::string          SHARE_DIR;
    const std::string    USER_CONF_DIR;
    const std::string    SOUND_TEMP_DIR;
    const std::string    DATA_DIR;
    const std::string    DATA_CONF_DIR;

    mod_api            * mod;
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
    int  _accountNB;
    bool _save_password_account;
    int  _last_target_index;

    int current_user_profil;

    uint8_t mod_state;

    struct UserProfil {
        int id;
        std::string name;

        UserProfil(int id, std::string name)
          : id(id)
          , name(std::move(name)) {}
    };
    std::vector<UserProfil> userProfils;

    bool                 enable_shared_clipboard;
    bool                 enable_shared_virtual_disk;

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

    bool                 is_recording;
    bool                 is_spanning;

    int rdp_width;
    int rdp_height;

    bool                 is_full_capturing;
    bool                 is_full_replaying;
    std::string          full_capture_file_name;
    bool                 is_replaying;
    bool                 is_loading_replay_mod;
    bool                 connected;

    std::string _movie_name;
    std::string _movie_dir;
    std::string _movie_full_path;

    uint8_t           connection_info_cmd_complete;

    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port;
    BGRPalette           mod_palette;



    ClientRedemptionAPI(SessionReactor& session_reactor)
    : MAIN_DIR(CLIENT_REDEMPTION_MAIN_PATH)
    , REPLAY_DIR(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_REPLAY_PATH)
    , USER_CONF_LOG(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_LOGINS_PATH)
    , WINDOWS_CONF(CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_WINODW_CONF_PATH)
    , CB_TEMP_DIR(MAIN_DIR + CLIENT_REDEMPTION_CB_FILE_TEMP_PATH)
    , SHARE_DIR(MAIN_DIR + CLIENT_REDEMPTION_SHARE_PATH)
    , USER_CONF_DIR(MAIN_DIR + CLIENT_REDEMPTION_USER_CONF_PATH)
    , SOUND_TEMP_DIR(CLIENT_REDEMPTION_SOUND_TEMP_PATH)
    , DATA_DIR(MAIN_DIR + CLIENT_REDEMPTION_DATA_PATH)
    , DATA_CONF_DIR(MAIN_DIR + CLIENT_REDEMPTION_DATA_CONF_PATH)
    , mod(nullptr)
    , info()
    , _accountNB(0)
    , _save_password_account(false)
    , _last_target_index(0)
    , current_user_profil(0)
    , mod_state(MOD_RDP)
    , enable_shared_clipboard(true)
    , enable_shared_virtual_disk(true)
    , vnc_conf(session_reactor, *(this))
    , is_recording(false)
    , is_spanning(false)
    , rdp_width(0)
    , rdp_height(0)
    , is_full_capturing(false)
    , is_full_replaying(false)
    , is_replaying(false)
    , is_loading_replay_mod(false)
    , connected(false)

    , connection_info_cmd_complete(PORT_GOT)
    , port(3389)
    , mod_palette(BGRPalette::classic_332())
    {}

    private:
    void parse_options(int argc, char const* const argv[]);

public:
    virtual ~ClientRedemptionAPI() = default;

    virtual void send_clipboard_format() {}

    void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                        , std::size_t , std::size_t , int ) override {}


    // CONTROLLER
    virtual void connect() {}
    virtual void disconnect(std::string const &, bool) {}
    virtual void replay(const std::string &, const std::string &) {}
    virtual bool load_replay_mod(std::string const &, std::string const &, timeval, timeval) { return false; }
    virtual timeval reload_replay_mod(int, timeval) { return timeval{}; }
    virtual bool is_replay_on() { return false; }
    virtual char const * get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int ) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}
    virtual void replay_set_pause(timeval) {}
    virtual void replay_set_sync() {}
    virtual void mouseButtonEvent(int , int , int) {}
    virtual void wheelEvent(int ,  int , int) {}
    virtual bool mouseMouveEvent(int , int ) {return false;}
    virtual void send_rdp_scanCode(int, int) {}
    virtual void send_rdp_unicode(uint16_t, uint16_t) {}
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
    virtual void add_key_custom_definition(int, int, const std::string &, int, const std::string &) {}
    virtual void setAccountData() {}
    virtual void writeAccoundData(const std::string &, const std::string &, const std::string &, const int) {}
    virtual std::vector<IconMovieData> get_icon_movie_data() {std::vector<IconMovieData> vec; return vec;}
    virtual void set_remoteapp_cmd_line(const std::string &) {}
    virtual bool is_no_win_data() { return true; }
    virtual void deleteCurrentProtile() {}
    virtual void setDefaultConfig() {}
    virtual void writeClientInfo() {}


    virtual time_t get_movie_time_length(char const *) { return time_t{}; }
    virtual void instant_play_client(std::chrono::microseconds) {}

};


class ClientIO
{
public:
    ClientRedemptionAPI * client;

    void set_client(ClientRedemptionAPI * client) {
        this->client = client;
    }
};
