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

#include "core/client_info.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "mod/rdp/rdp_verbose.hpp"
#include "utils/theme.hpp"

#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"
#include "client_redemption/client_input_output_api/rdp_disk_config.hpp"
#include "client_redemption/client_input_output_api/rdp_sound_config.hpp"

#include <string>
#include <vector>


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

namespace qtclient
{

struct Profile
{
    std::string profile_name;

    std::string user_name;
    std::string user_password;
    std::string target_address;
    uint16_t target_port = 3389;
    bool is_rdp_mod = true;

    RDPVerbose rdp_verbose = RDPVerbose();

    bool is_spanning = false;
    bool enable_recording = false;
    bool enable_clipboard = true;

    bool enable_nla = false;
    bool enable_tls = true;
    bool enable_sound = false;

    bool enable_remote_app = true;
    std::string remote_app_cmd;
    std::string remote_app_working_directory;

    bool enable_drive= true;
    std::string drive_path;

    uint32_t tls_min_level = 0;
    uint32_t tls_max_level = 0;
    std::string cipher_string;

    uint32_t rdp5_performance_flags = 0x00000040;

    ScreenInfo screen_info = {800, 600, BitsPerPixel::BitsPP16};
    KeyLayout::KbdId key_layout = KeyLayout::KbdId();
};

struct Profiles : std::vector<Profile>
{
    Profiles();

    Profile& add_profile(std::string_view name, bool selected = false);

    bool choice_profile(std::size_t id) noexcept;
    bool choice_profile(std::string_view name) noexcept;

    Profile* find(std::string_view name) noexcept;

    Profile& current_profile() noexcept
    {
        return operator[](current_index);
    }

    Profile const& current_profile() const noexcept
    {
        return operator[](current_index);
    }

    std::size_t current_index = 0;
};

Profiles load_profiles(char const* filename);
bool save_profiles(char const* filename, Profiles const& profiles);
bool parse_options(int argc, char const* const argv[], Profile& profile);

} // namespace qtclient


struct UserProfil {
    int id;
    std::string name;

    UserProfil(int id, std::string name)
        : id(id)
        , name(std::move(name)) {}
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
struct ModVNCParamsData
{
    Theme      theme;
    WindowListCaps windowListCaps;
    std::string vnc_encodings = "5,16,0,1,-239";
    std::vector<UserProfil> userProfils;
    KeyLayout::KbdId keylayout {0x040C};
    int width = 800;
    int height = 600;
    int current_user_profil = 0;

    bool enable_tls = false;
    bool enable_nla = false;
    bool enable_sound = false;
    bool enable_shared_clipboard = false;
    bool is_apple = false;

    ModVNCParamsData() = default;
};

struct TLSClientParamsData
{
    uint32_t tls_min_level = 0;
    uint32_t tls_max_level = 0;
    bool show_common_cipher_list = false;
    std::string cipher_string;
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
    std::string config_file_path;

    int form_x = 0;
    int form_y = 0;
    int screen_x = 0;
    int screen_y = 0;

    bool no_data = true;

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


struct ClientRedemptionConfig
{
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
    };


    RDPVerbose        verbose;
    //bool                _recv_disconnect_ultimatum;
    ClientInfo           info;
    bool wab_diag_question = false;

    RDPClipboardConfig rDPClipboardConfig;
    RDPDiskConfig      rDPDiskConfig;
    RDPSoundConfig     rDPSoundConfig;
    RDPRemoteAppConfig rDPRemoteAppConfig;

    int keep_alive_freq = 100;

    WindowsData windowsData;

    std::string profilName;

    ModRDPParamsData modRDPParamsData;
    TLSClientParamsData tls_client_params_data;
    ModVNCParamsData modVNCParamsData;


    std::vector<AccountData> _accountData;
    size_t  _last_target_index = 0;
    int  _accountNB = 0;
    bool _save_password_account = false;

    int current_user_profil = 0;

    uint8_t mod_state = MOD_RDP;

    std::string full_capture_file_name;
    bool        is_full_capturing = false;
    bool        is_full_replaying = false;
    bool        is_replaying = false;
    bool        is_loading_replay_mod = false;
    bool        connected = false;

    bool enable_shared_clipboard = true;
    bool enable_remotefx = false;

    bool is_pre_loading = false;
    bool is_recording = false;
    bool is_spanning = false;

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



    ClientRedemptionConfig(RDPVerbose verbose, const std::string &MAIN_DIR );

};


namespace ClientConfig {

void setUserProfil(ClientRedemptionConfig & config);
void setAccountData(ClientRedemptionConfig & config);
void openWindowsData(ClientRedemptionConfig & config);

void set_config(int argc, char const* const argv[], ClientRedemptionConfig & config);

void writeWindowsData(WindowsData & config);
void writeClientInfo(ClientRedemptionConfig & config);
void deleteCurrentProtile(ClientRedemptionConfig & config);
void writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port, ClientRedemptionConfig & config);

bool read_line(const int fd, std::string & line);
time_t get_movie_time_length(const char * mwrm_filename);

} // namespace ClientConfig
