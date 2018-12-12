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

#include "utils/log.hpp"
#include "utils/netutils.hpp"
#include "utils/genfstat.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "utils/sugar/algostring.hpp"

#include "main/version.hpp"
#include "utils/cli.hpp"
#include "utils/fileutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/theme.hpp"


#include "core/client_info.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/session_reactor.hpp"

#include "mod/rdp/rdp_verbose.hpp"

#include "capture/cryptofile.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"

#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"
#include "client_redemption/client_input_output_api/rdp_disk_config.hpp"
#include "client_redemption/client_input_output_api/rdp_sound_config.hpp"

#include <iostream>
#include <algorithm>

#include <cstdio>
#include <climits>
#include <cstdint>

#include <dirent.h>
#include <openssl/ssl.h>

#define CLIENT_REDEMPTION_REPLAY_PATH "/DATA/replay"
#define CLIENT_REDEMPTION_LOGINS_PATH "/DATA/config/login.config"
#define CLIENT_REDEMPTION_WINODW_CONF_PATH "/DATA/config/windows_config.config"
#define CLIENT_REDEMPTION_SHARE_PATH "/DATA/share"
#define CLIENT_REDEMPTION_CB_FILE_TEMP_PATH "/DATA/clipboard_temp"
#define CLIENT_REDEMPTION_KEY_SETTING_PATH "/DATA/config/keySetting.config"
#define CLIENT_REDEMPTION_USER_CONF_PATH "/DATA/config/userConfig.config"
#define CLIENT_REDEMPTION_SOUND_TEMP_PATH "/DATA/sound_temp"
#define CLIENT_REDEMPTION_DATA_PATH "/DATA"
#define CLIENT_REDEMPTION_DATA_CONF_PATH "/DATA/config"

#ifndef CLIENT_REDEMPTION_MAIN_PATH
#define CLIENT_REDEMPTION_MAIN_PATH ""
#endif


struct UserProfil {
    int id;
    std::string name;

    UserProfil(int id, std::string name)
        : id(id)
        , name(std::move(name)) {}
};

struct KeyCustomDefinition {
    int qtKeyID = 0;
    int scanCode = 0;
    std::string ASCII8;
    int extended = 0;
    std::string name;

    KeyCustomDefinition() = default;

    KeyCustomDefinition(int qtKeyID, int scanCode, std::string ASCII8, int extended, std::string name)
        : qtKeyID(qtKeyID)
        , scanCode(scanCode)
        , ASCII8(std::move(ASCII8))
        , extended(extended ? 0x0100 : 0)
        , name(std::move(name))
    {}
};

struct IconMovieData {
    std::string file_name;
    std::string file_path;
    std::string file_version;
    std::string file_resolution;
    std::string file_checksum;
    long int movie_len = 0;

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

// VNC mod
struct ModVNCParamsData {
    bool is_apple;
    Theme      theme;
    WindowListCaps windowListCaps;
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

    ModVNCParamsData()
        : is_apple(false)
        , vnc_encodings("5,16,0,1,-239")
    {}
};

struct ModRDPParamsData
{
    int rdp_width = 0;
    int rdp_height = 0;
    bool enable_tls   = false;
    bool enable_nla   = false;
    bool enable_sound = false;

    bool enable_shared_virtual_disk = true;
    bool enable_shared_remoteapp = false;
};

struct RDPRemoteAppConfig
{
    std::string source_of_ExeOrFile;
    std::string source_of_WorkingDir;
    std::string source_of_Arguments;
    std::string full_cmd_line;
};

struct WindowsData
{
    const std::string config_file_path;

    int form_x = 0;
    int form_y = 0;
    int screen_x = 0;
    int screen_y = 0;

    bool no_data = true;

    WindowsData(std::string config_file_path)
      : config_file_path(std::move(config_file_path))
    {}

    void writeWindowsData()
    {
        unique_fd fd(this->config_file_path.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if (fd.is_open()) {
            std::string info = str_concat(
                "form_x ", std::to_string(this->form_x), "\n"
                "form_y ", std::to_string(this->form_y), "\n"
                "screen_x ", std::to_string(this->screen_x), "\n"
                "screen_y ", std::to_string(this->screen_y), '\n');

            ::write(fd.fd(), info.c_str(), info.length());
        }
    }
};

struct AccountData
{
    std::string title;
    std::string IP;
    std::string name;
    std::string pwd;
    int port = 0;
    int options_profil = 0;
    int index = -1;
    int protocol = 0;
};


class ClientRedemptionConfig
{

public:
    std::vector<IconMovieData> icons_movie_data;

    const std::string    MAIN_DIR/* = CLIENT_REDEMPTION_MAIN_PATH*/;
    const std::string    REPLAY_DIR     = MAIN_DIR + CLIENT_REDEMPTION_REPLAY_PATH;
    const std::string    USER_CONF_LOG  = MAIN_DIR + CLIENT_REDEMPTION_LOGINS_PATH;
    const std::string    WINDOWS_CONF   = MAIN_DIR + CLIENT_REDEMPTION_WINODW_CONF_PATH;
    const std::string    CB_TEMP_DIR    = MAIN_DIR + CLIENT_REDEMPTION_CB_FILE_TEMP_PATH;
    std::string          SHARE_DIR      = MAIN_DIR + CLIENT_REDEMPTION_SHARE_PATH;
    const std::string    USER_CONF_PATH = MAIN_DIR + CLIENT_REDEMPTION_USER_CONF_PATH;
    const std::string    SOUND_TEMP_DIR = MAIN_DIR + CLIENT_REDEMPTION_SOUND_TEMP_PATH;
    const std::string    DATA_DIR       = MAIN_DIR + CLIENT_REDEMPTION_DATA_PATH;
    const std::string    DATA_CONF_DIR  = MAIN_DIR + CLIENT_REDEMPTION_DATA_CONF_PATH;


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


    RDPVerbose        verbose;
    //bool                _recv_disconnect_ultimatum;
    ClientInfo           info;
    bool wab_diag_question;

    RDPClipboardConfig rDPClipboardConfig;
    RDPDiskConfig      rDPDiskConfig;
    RDPSoundConfig     rDPSoundConfig;
    RDPRemoteAppConfig rDPRemoteAppConfig;

    bool quick_connection_test;

    bool persist;

    std::chrono::milliseconds time_out_disconnection;
    int keep_alive_freq;

    WindowsData windowsData;

    std::vector<KeyCustomDefinition> keyCustomDefinitions;
    std::vector<UserProfil> userProfils;

    ModRDPParamsData modRDPParamsData;
    ModVNCParamsData modVNCParamsData;

                                                     // _accountData[MAX_ACCOUNT_DATA];
    std::vector<AccountData> _accountData;
    int  _accountNB = 0;
    bool _save_password_account = false;
    size_t  _last_target_index = 0;

    int current_user_profil = 0;

    uint8_t mod_state = MOD_RDP;

    bool enable_shared_clipboard = true;


    bool                 is_recording = false;
    bool                 is_spanning = false;

    int rdp_width = 800;
    int rdp_height = 600;

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

    bool help_mode = false;



    ClientRedemptionConfig(SessionReactor& session_reactor, char const* argv[], int argc, RDPVerbose verbose, const std::string &MAIN_DIR );

    ~ClientRedemptionConfig() = default;

    void set_icon_movie_data();

    time_t get_movie_time_length(const char * mwrm_filename);

    std::vector<IconMovieData> const& get_icon_movie_data();

    void parse_options(int argc, char const* const argv[]);

    void openWindowsData();

    void writeWindowsData();

    void setUserProfil();

    void setCustomKeyConfig();

    void writeCustomKeyConfig();

    void setAccountData();


    void add_key_custom_definition(int qtKeyID, int scanCode, const std::string & ASCII8, int extended, const std::string & name);

    void setClientInfo();

    bool read_line(const int fd, std::string & line);


    void writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port);



//         this->qtRDPKeymap.clearCustomKeyCode();
//         this->keyCustomDefinitions.clear();
//
//         std::ifstream iFileKeyData(this->MAIN_DIR + std::string(KEY_SETTING_PATH), std::ios::in);
//         if(iFileKeyData) {
//
//             std::string ligne;
//             std::string delimiter = " ";
//
//             while(getline(iFileKeyData, ligne)) {
//
//                 int pos(ligne.find(delimiter));
//
//                 if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {
//
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int qtKeyID  = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int scanCode = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int ASCII8   = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int extended = std::stoi(ligne.substr(0, pos));
//
//                     this->qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);
//                     this->keyCustomDefinitions.push_back({qtKeyID, scanCode, ASCII8, extended});
//                 }
//             }
//
//             iFileKeyData.close();
//         }




    void set_remoteapp_cmd_line(const std::string & cmd);

    bool is_no_win_data();
    void deleteCurrentProtile();


    void setDefaultConfig();


    void writeClientInfo();
};
