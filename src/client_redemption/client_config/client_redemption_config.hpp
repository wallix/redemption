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

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <dirent.h>

#include "utils/genfstat.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "main/version.hpp"
#include "utils/cli.hpp"
#include "utils/fileutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/theme.hpp"


#include "core/client_info.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/session_reactor.hpp"

#include "mod/rdp/rdp_verbose.hpp"
#include "mod/internal/client_execute.hpp"

#include "capture/cryptofile.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"

#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"
#include "client_redemption/client_input_output_api/rdp_disk_config.hpp"
#include "client_redemption/client_input_output_api/rdp_sound_config.hpp"

#include <algorithm>

#include <climits>
#include <cstdint>
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

        ModVNCParamsData(SessionReactor& session_reactor, FrontAPI & client)
          : is_apple(false)
          , exe(session_reactor, client, this->windowListCaps, false)
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

    struct RDPRemoteAppConfig {
        std::string source_of_ExeOrFile;
        std::string source_of_WorkingDir;
        std::string source_of_Arguments;
        std::string full_cmd_line;
    };

struct WindowsData {

    const std::string config_file_path;

    int form_x = 0;
    int form_y = 0;
    int screen_x = 0;
    int screen_y = 0;

    bool no_data = true;

    WindowsData(std::string config_file_path)
      : config_file_path(std::move(config_file_path))
    {}

    void writeWindowsData()  {
        std::ofstream ofile(this->config_file_path, std::ios::trunc);
        if (ofile) {
            ofile
                << "form_x " << this->form_x << "\n"
                << "form_y " << this->form_y << "\n"
                << "screen_x " << this->screen_x << "\n"
                << "screen_y " << this->screen_y << "\n"
            ;
            ofile.close();
        }
    }
};

    struct AccountData {
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


    const std::string    MAIN_DIR /*= CLIENT_REDEMPTION_MAIN_PATH*/;
    const std::string    REPLAY_DIR = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_REPLAY_PATH;
    const std::string    USER_CONF_LOG = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_LOGINS_PATH;
    const std::string    WINDOWS_CONF = CLIENT_REDEMPTION_MAIN_PATH CLIENT_REDEMPTION_WINODW_CONF_PATH;
    const std::string    CB_TEMP_DIR = MAIN_DIR + CLIENT_REDEMPTION_CB_FILE_TEMP_PATH;
    std::string          SHARE_DIR = MAIN_DIR + CLIENT_REDEMPTION_SHARE_PATH;
    const std::string    USER_CONF_DIR = MAIN_DIR + CLIENT_REDEMPTION_USER_CONF_PATH;
    const std::string    SOUND_TEMP_DIR = CLIENT_REDEMPTION_SOUND_TEMP_PATH;
    const std::string    DATA_DIR = MAIN_DIR + CLIENT_REDEMPTION_DATA_PATH;
    const std::string    DATA_CONF_DIR = MAIN_DIR + CLIENT_REDEMPTION_DATA_CONF_PATH;


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
    ModVNCParamsData vnc_conf;

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



    ClientRedemptionConfig(SessionReactor& session_reactor, char const* argv[], int argc, RDPVerbose verbose, FrontAPI &front, const std::string &MAIN_DIR )
    : MAIN_DIR(MAIN_DIR)
    , verbose(verbose)
    //, _recv_disconnect_ultimatum(false)
    , wab_diag_question(false)
    , quick_connection_test(true)
    , persist(false)
    , time_out_disconnection(5000)
    , keep_alive_freq(100)
    , windowsData(this->WINDOWS_CONF)
    , vnc_conf(session_reactor, front)
    {
        this->setDefaultConfig();

        this->info.screen_info.width  = 800;
        this->info.screen_info.height = 600;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = false;
        this->info.brush_cache_code = 0;
        this->info.screen_info.bpp = BitsPerPixel{24};
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;

        this->rDPRemoteAppConfig.source_of_ExeOrFile = "C:\\Windows\\system32\\notepad.exe";
        this->rDPRemoteAppConfig.source_of_WorkingDir = "C:\\Users\\user1";

        this->rDPRemoteAppConfig.full_cmd_line = this->rDPRemoteAppConfig.source_of_ExeOrFile + " " + this->rDPRemoteAppConfig.source_of_Arguments;


        if (!this->MAIN_DIR.empty()) {
            for (auto* pstr : {
                &this->DATA_DIR,
                &this->REPLAY_DIR,
                &this->CB_TEMP_DIR,
                &this->DATA_CONF_DIR,
                &this->SOUND_TEMP_DIR
            }) {
                if (!pstr->empty()) {
                    if (!file_exist(pstr->c_str())) {
                        LOG(LOG_INFO, "Create file \"%s\".", pstr->c_str());
                        mkdir(pstr->c_str(), 0775);
                    }
                }
            }
        }

        // Set RDP CLIPRDR config
        this->rDPClipboardConfig.arbitrary_scale = 40;
        this->rDPClipboardConfig.server_use_long_format_names = true;
        this->rDPClipboardConfig.cCapabilitiesSets = 1;
        this->rDPClipboardConfig.generalFlags = RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS;
        this->rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
        this->rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
        this->rDPClipboardConfig.add_format(RDPECLIP::CF_TEXT, {});
        this->rDPClipboardConfig.add_format(RDPECLIP::CF_METAFILEPICT, {});
        this->rDPClipboardConfig.path = this->CB_TEMP_DIR;


        // Set RDP RDPDR config
        this->rDPDiskConfig.add_drive(this->SHARE_DIR, rdpdr::RDPDR_DTYP_FILESYSTEM);
        this->rDPDiskConfig.enable_drive_type = true;
        this->rDPDiskConfig.enable_printer_type = true;


        // Set RDP SND config
        this->rDPSoundConfig.dwFlags = rdpsnd::TSSNDCAPS_ALIVE | rdpsnd::TSSNDCAPS_VOLUME;
        this->rDPSoundConfig.dwVolume = 0x7fff7fff;
        this->rDPSoundConfig.dwPitch = 0;
        this->rDPSoundConfig.wDGramPort = 0;
        this->rDPSoundConfig.wNumberOfFormats = 1;
        this->rDPSoundConfig.wVersion = 0x06;


        this->setUserProfil();
        this->setClientInfo();
        this->setCustomKeyConfig();
        this->setAccountData();

        this->openWindowsData();
        std::fill(std::begin(this->info.order_caps.orderSupport), std::end(this->info.order_caps.orderSupport), 1);
        this->info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;

//         this->parse_options(argc, argv);


        auto options = cli::options(
            cli::helper("Client ReDemPtion Help menu."),

            cli::option('h', "help").help("Show help")
            .action(cli::help),

            cli::option('v', "version").help("Show version")
            .action(cli::quit([]{ std::cout << redemption_info_version() << "\n"; })),

            cli::helper("========= Connection ========="),

            cli::option('u', "username").help("Set target session user name")
            .action(cli::arg([this](std::string s){
                this->user_name = std::move(s);

                this->connection_info_cmd_complete |= NAME_GOT;
            })),

            cli::option('p', "password").help("Set target session user password")
            .action(cli::arg([this](std::string s){
                this->user_password = std::move(s);

                this->connection_info_cmd_complete |= PWD_GOT;
            })),

            cli::option('i', "ip").help("Set target IP address")
            .action(cli::arg([this](std::string s){
                this->target_IP = std::move(s);

                this->connection_info_cmd_complete |= IP_GOT;
            })),

            cli::option('P', "port").help("Set port to use on target")
            .action(cli::arg([this](int n){
                this->port = n;
                this->connection_info_cmd_complete |= PORT_GOT;
            })),

            cli::helper("========= Verbose ========="),

            cli::option("rdpdr").help("Active rdpdr logs")
            .action(cli::on_off_bit_location<RDPVerbose::rdpdr>(this->verbose)),

            cli::option("rdpsnd").help("Active rdpsnd logs")
            .action(cli::on_off_bit_location<RDPVerbose::rdpsnd>(this->verbose)),

            cli::option("cliprdr").help("Active cliprdr logs")
            .action(cli::on_off_bit_location<RDPVerbose::cliprdr>(this->verbose)),

            cli::option("graphics").help("Active graphics logs")
            .action(cli::on_off_bit_location<RDPVerbose::graphics>(this->verbose)),

            cli::option("printer").help("Active printer logs")
            .action(cli::on_off_bit_location<RDPVerbose::printer>(this->verbose)),

            cli::option("rdpdr-dump").help("Actives rdpdr logs and dump brute rdpdr PDU")
            .action(cli::on_off_bit_location<RDPVerbose::rdpdr_dump>(this->verbose)),

            cli::option("cliprd-dump").help("Actives cliprdr logs and dump brute cliprdr PDU")
            .action(cli::on_off_bit_location<RDPVerbose::cliprdr_dump>(this->verbose)),

            cli::option("basic-trace").help("Active basic-trace logs")
            .action(cli::on_off_bit_location<RDPVerbose::basic_trace>(this->verbose)),

            cli::option("connection").help("Active connection logs")
            .action(cli::on_off_bit_location<RDPVerbose::connection>(this->verbose)),

            cli::option("rail-order").help("Active rail-order logs")
            .action(cli::on_off_bit_location<RDPVerbose::rail_order>(this->verbose)),

            cli::option("asynchronous-task").help("Active asynchronous-task logs")
            .action(cli::on_off_bit_location<RDPVerbose::asynchronous_task>(this->verbose)),

            cli::option("capabilities").help("Active capabilities logs")
            .action(cli::on_off_bit_location<RDPVerbose::capabilities>(this->verbose)),

            cli::option("rail").help("Active rail logs")
            .action(cli::on_off_bit_location<RDPVerbose::rail>(this->verbose)),

            cli::option("rail-dump").help("Actives rail logs and dump brute rail PDU")
            .action(cli::on_off_bit_location<RDPVerbose::rail_dump>(this->verbose)),


            cli::helper("========= Protocol ========="),

            cli::option("vnc").help("Set connection mod to VNC")
            .action([this](){
                this->mod_state = MOD_VNC;
                if (!bool(this->connection_info_cmd_complete & PORT_GOT)) {
                    this->port = 5900;
                }
            }),

            cli::option("rdp").help("Set connection mod to RDP (default).")
            .action([this](){
                this->mod_state = MOD_RDP;
                this->port = 3389;
            }),

            cli::option("remote-app").help("Connection as remote application.")
            .action(cli::on_off_bit_location<MOD_RDP_REMOTE_APP>(this->mod_state)),

            cli::option("remote-exe").help("Connection as remote application and set the line command.")
            .action(cli::arg("command", [this](std::string line){
                this->mod_state = MOD_RDP_REMOTE_APP;
                this->modRDPParamsData.enable_shared_remoteapp = true;
                auto pos(line.find(' '));
                if (pos == std::string::npos) {
                    this->rDPRemoteAppConfig.source_of_ExeOrFile = std::move(line);
                    this->rDPRemoteAppConfig.source_of_Arguments.clear();
                }
                else {
                    this->rDPRemoteAppConfig.source_of_ExeOrFile = line.substr(0, pos);
                    this->rDPRemoteAppConfig.source_of_Arguments = line.substr(pos + 1);
                }
            })),

            cli::option("span").help("Span the screen size on local screen")
            .action(cli::on_off_location(this->is_spanning)),

            cli::option("enable-clipboard").help("Enable clipboard sharing")
            .action(cli::on_off_location(this->enable_shared_clipboard)),

            cli::option("enable-nla").help("Entable NLA protocol")
            .action(cli::on_off_location(this->modRDPParamsData.enable_nla)),

            cli::option("enable-tls").help("Enable TLS protocol")
            .action(cli::on_off_location(this->modRDPParamsData.enable_tls)),

            cli::option("enable-sound").help("Enable sound")
            .action(cli::on_off_location(this->modRDPParamsData.enable_sound)),

            cli::option("enable-fullwindowdrag").help("Enable full window draging")
            .action(cli::on_off_bit_location<~PERF_DISABLE_FULLWINDOWDRAG>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-menuanimations").help("Enable menu animations")
            .action(cli::on_off_bit_location<~PERF_DISABLE_MENUANIMATIONS>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-theming").help("Enable theming")
            .action(cli::on_off_bit_location<~PERF_DISABLE_THEMING>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-cursor-shadow").help("Enable cursor shadow")
            .action(cli::on_off_bit_location<~PERF_DISABLE_CURSOR_SHADOW>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-cursorsettings").help("Enable cursor settings")
            .action(cli::on_off_bit_location<~PERF_DISABLE_CURSORSETTINGS>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-font-smoothing").help("Enable font smoothing")
            .action(cli::on_off_bit_location<PERF_ENABLE_FONT_SMOOTHING>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-desktop-composition").help("Enable desktop composition")
            .action(cli::on_off_bit_location<PERF_ENABLE_DESKTOP_COMPOSITION>(
                this->info.rdp5_performanceflags)),

            cli::option("vnc-applekeyboard").help("Set keyboard compatibility mod with apple VNC server")
            .action(cli::on_off_location(this->vnc_conf.is_apple)),


            cli::option("keep_alive_frequence")
            .help("Set timeout to send keypress to keep the session alive")
            .action(cli::arg([&](int t){ keep_alive_freq = t; })),


            cli::helper("========= Client ========="),

            cli::option("width").help("Set screen width")
            .action(cli::arg_location(this->rdp_width)),

            cli::option("height").help("Set screen height")
            .action(cli::arg_location(this->rdp_height)),

            cli::option("bpp").help("Set bit per pixel (8, 15, 16, 24)")
            .action(cli::arg("bit_per_pixel", [this](int x) {
                this->info.screen_info.bpp = checked_int(x);
            })),

            cli::option("keylaout").help("Set windows keylayout")
            .action(cli::arg_location(this->info.keylayout)),

            cli::option("enable-record").help("Enable session recording as .wrm movie")
            .action(cli::on_off_location(this->is_recording)),

            cli::option("persist").help("Set connection to persist")
            .action([&]{
                quick_connection_test = false;
                persist = true;
            }),

            cli::option("timeout").help("Set timeout response before to disconnect in milisecond")
            .action(cli::arg("time", [&](long time){
                quick_connection_test = false;
                time_out_disconnection = std::chrono::milliseconds(time);
            })),

            cli::option("share-dir").help("Set directory path on local disk to share with your session.")
            .action(cli::arg("directory", [this](std::string s) {
                this->modRDPParamsData.enable_shared_virtual_disk = !s.empty();
                this->SHARE_DIR = std::move(s);
            })),

            cli::option("remote-dir").help("Remote working directory")
            .action(cli::arg_location("directory", this->rDPRemoteAppConfig.source_of_WorkingDir))
        );

        auto cli_result = cli::parse(options, argc, argv);
        switch (cli_result.res) {
            case cli::Res::Ok:
                break;
            case cli::Res::Exit:
                // TODO return 0;
                break;
            case cli::Res::Help:
                cli::print_help(options, std::cout);
                // TODO return 0;
                break;
            case cli::Res::BadFormat:
            case cli::Res::BadOption:
                std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
                if (cli_result.opti < cli_result.argc) {
                    std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
                }
                std::cerr << "\n";
                // TODO return 1;
                break;
        }

        if (bool(RDPVerbose::rail & this->verbose)) {
            this->verbose = this->verbose | RDPVerbose::rail_order;
        }
    }

    ~ClientRedemptionConfig() = default;

    void set_icon_movie_data() {

        this->icons_movie_data.clear();

        DIR *dir;
        struct dirent *ent;
        std::string extension(".mwrm");

        if ((dir = opendir (this->REPLAY_DIR.c_str())) != nullptr) {
//
            try {
                while ((ent = readdir (dir)) != nullptr) {

                    std::string current_name = std::string (ent->d_name);

                    if (current_name.length() > 5) {

                        std::string end_string(current_name.substr(current_name.length()-5, current_name.length()));
                        if (end_string == extension) {

                            std::string file_path = this->REPLAY_DIR + "/" + current_name;

                            std::fstream ofile(file_path.c_str(), std::ios::in);
                            if(ofile) {
                                std::string file_name(current_name.substr(0, current_name.length()-5));
                                std::string file_version;
                                std::string file_resolution;
                                std::string file_checksum;
                                long int movie_len = this->get_movie_time_length(file_path.c_str());

                                std::getline(ofile, file_version);
                                std::getline(ofile, file_resolution);
                                std::getline(ofile, file_checksum);

                                this->icons_movie_data.emplace_back(file_name, file_path, file_version, file_resolution, file_checksum, movie_len);

                            } else {
                                LOG(LOG_INFO, "Can't open file \"%s\"", file_path);
                            }
                        }
                    }
                }
            } catch (Error & e) {
                LOG(LOG_WARNING, "readdir error: (%u) %s", e.id, e.errmsg());
            }
            closedir (dir);
        }
    }

    time_t get_movie_time_length(const char * mwrm_filename) {
        // TODO RZ: Support encrypted recorded file.

        CryptoContext cctx;
        Fstat fsats;
        InCryptoTransport trans(cctx, InCryptoTransport::EncryptionMode::NotEncrypted, fsats);
        MwrmReader mwrm_reader(trans);
        MetaLine meta_line;

        time_t start_time = 0;
        time_t stop_time = 0;

        trans.open(mwrm_filename);
        mwrm_reader.read_meta_headers();

        Transport::Read read_stat = mwrm_reader.read_meta_line(meta_line);
        if (read_stat == Transport::Read::Ok) {
            start_time = meta_line.start_time;
            stop_time = meta_line.stop_time;
            while (read_stat == Transport::Read::Ok) {
                stop_time = meta_line.stop_time;
                read_stat = mwrm_reader.read_meta_line(meta_line);
            }
        }

        return stop_time - start_time;
    }

    std::vector<IconMovieData> get_icon_movie_data() {

        this->set_icon_movie_data();

        return this->icons_movie_data;
    }

    void parse_options(int argc, char const* const argv[])
    {
        auto options = cli::options(
            cli::helper("Client ReDemPtion Help menu."),

            cli::option('h', "help").help("Show help")
            .action(cli::help),

            cli::option('v', "version").help("Show version")
            .action(cli::quit([]{ std::cout << redemption_info_version() << "\n"; })),

            cli::helper("========= Connection ========="),

            cli::option('u', "username").help("Set target session user name")
            .action(cli::arg([this](std::string s){
                this->user_name = std::move(s);
                this->connection_info_cmd_complete += NAME_GOT;
            })),

            cli::option('p', "password").help("Set target session user password")
            .action(cli::arg([this](std::string s){
                this->user_password = std::move(s);
                this->connection_info_cmd_complete += PWD_GOT;
            })),

            cli::option('i', "ip").help("Set target IP address")
            .action(cli::arg([this](std::string s){
                this->target_IP = std::move(s);
                this->connection_info_cmd_complete += IP_GOT;
            })),

            cli::option('P', "port").help("Set port to use on target")
            .action(cli::arg([this](int n){
                this->port = n;
                this->connection_info_cmd_complete += PORT_GOT;
            })),


            cli::helper("========= Verbose ========="),

            cli::option("rdpdr").help("Active rdpdr logs")
            .action(cli::on_off_bit_location<RDPVerbose::rdpdr>(this->verbose)),

            cli::option("rdpsnd").help("Active rdpsnd logs")
            .action(cli::on_off_bit_location<RDPVerbose::rdpsnd>(this->verbose)),

            cli::option("cliprdr").help("Active cliprdr logs")
            .action(cli::on_off_bit_location<RDPVerbose::cliprdr>(this->verbose)),

            cli::option("graphics").help("Active graphics logs")
            .action(cli::on_off_bit_location<RDPVerbose::graphics>(this->verbose)),

            cli::option("printer").help("Active printer logs")
            .action(cli::on_off_bit_location<RDPVerbose::printer>(this->verbose)),

            cli::option("rdpdr-dump").help("Actives rdpdr logs and dump brute rdpdr PDU")
            .action(cli::on_off_bit_location<RDPVerbose::rdpdr_dump>(this->verbose)),

            cli::option("cliprd-dump").help("Actives cliprdr logs and dump brute cliprdr PDU")
            .action(cli::on_off_bit_location<RDPVerbose::cliprdr_dump>(this->verbose)),

            cli::option("basic-trace").help("Active basic-trace logs")
            .action(cli::on_off_bit_location<RDPVerbose::basic_trace>(this->verbose)),

            cli::option("connection").help("Active connection logs")
            .action(cli::on_off_bit_location<RDPVerbose::connection>(this->verbose)),

            cli::option("rail-order").help("Active rail-order logs")
            .action(cli::on_off_bit_location<RDPVerbose::rail_order>(this->verbose)),

            cli::option("asynchronous-task").help("Active asynchronous-task logs")
            .action(cli::on_off_bit_location<RDPVerbose::asynchronous_task>(this->verbose)),

            cli::option("capabilities").help("Active capabilities logs")
            .action(cli::on_off_bit_location<RDPVerbose::capabilities>(this->verbose)),

            cli::option("rail").help("Active rail logs")
            .action(cli::on_off_bit_location<RDPVerbose::rail>(this->verbose)),

            cli::option("rail-dump").help("Actives rail logs and dump brute rail PDU")
            .action(cli::on_off_bit_location<RDPVerbose::rail_dump>(this->verbose)),


            cli::helper("========= Protocol ========="),

            cli::option("vnc").help("Set connection mod to VNC")
            .action([this](){
                this->mod_state = MOD_VNC;
                if (!bool(this->connection_info_cmd_complete & PORT_GOT)) {
                    this->port = 5900;
                }
            }),

            cli::option("rdp").help("Set connection mod to RDP (default).")
            .action([this](){ this->mod_state = MOD_VNC; }),

            cli::option("remote-app").help("Connection as remote application.")
            .action(cli::on_off_bit_location<MOD_RDP_REMOTE_APP>(this->mod_state)),

            cli::option("remote-exe").help("Connection as remote application and set the line command.")
            .action(cli::arg("command", [this](std::string line){
                this->mod_state = MOD_RDP_REMOTE_APP;
                auto pos(line.find(' '));
                if (pos == std::string::npos) {
                    this->rDPRemoteAppConfig.source_of_ExeOrFile = std::move(line);
                    this->rDPRemoteAppConfig.source_of_Arguments.clear();
                }
                else {
                    this->rDPRemoteAppConfig.source_of_ExeOrFile = line.substr(0, pos);
                    this->rDPRemoteAppConfig.source_of_Arguments = line.substr(pos + 1);
                }
            })),

            cli::option("span").help("Span the screen size on local screen")
            .action(cli::on_off_location(this->is_spanning)),

            cli::option("enable-clipboard").help("Enable clipboard sharing")
            .action(cli::on_off_location(this->enable_shared_clipboard)),

            cli::option("enable-nla").help("Entable NLA protocol")
            .action(cli::on_off_location(this->modRDPParamsData.enable_nla)),

            cli::option("enable-tls").help("Enable TLS protocol")
            .action(cli::on_off_location(this->modRDPParamsData.enable_tls)),

            cli::option("enable-sound").help("Enable sound")
            .action(cli::on_off_location(this->modRDPParamsData.enable_sound)),

            cli::option("enable-fullwindowdrag").help("Enable full window draging")
            .action(cli::on_off_bit_location<~PERF_DISABLE_FULLWINDOWDRAG>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-menuanimations").help("Enable menu animations")
            .action(cli::on_off_bit_location<~PERF_DISABLE_MENUANIMATIONS>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-theming").help("Enable theming")
            .action(cli::on_off_bit_location<~PERF_DISABLE_THEMING>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-cursor-shadow").help("Enable cursor shadow")
            .action(cli::on_off_bit_location<~PERF_DISABLE_CURSOR_SHADOW>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-cursorsettings").help("Enable cursor settings")
            .action(cli::on_off_bit_location<~PERF_DISABLE_CURSORSETTINGS>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-font-smoothing").help("Enable font smoothing")
            .action(cli::on_off_bit_location<PERF_ENABLE_FONT_SMOOTHING>(
                this->info.rdp5_performanceflags)),

            cli::option("enable-desktop-composition").help("Enable desktop composition")
            .action(cli::on_off_bit_location<PERF_ENABLE_DESKTOP_COMPOSITION>(
                this->info.rdp5_performanceflags)),

            cli::option("vnc-applekeyboard").help("Set keyboard compatibility mod with apple VNC server")
            .action(cli::on_off_location(this->vnc_conf.is_apple)),


            cli::helper("========= Client ========="),

            cli::option("width").help("Set screen width")
            .action(cli::arg_location(this->rdp_width)),

            cli::option("height").help("Set screen height")
            .action(cli::arg_location(this->rdp_height)),

            cli::option("bpp").help("Set bit per pixel (8, 15, 16, 24)")
            .action(cli::arg("bit_per_pixel", [this](int x) {
                this->info.screen_info.bpp = checked_int(x);
            })),

            cli::option("keylaout").help("Set windows keylayout")
            .action(cli::arg_location(this->info.keylayout)),

            cli::option("enable-record").help("Enable session recording as .wrm movie")
            .action(cli::on_off_location(this->is_recording)),

            cli::option("share-dir").help("Set directory path on local disk to share with your session.")
            .action(cli::arg("directory", [this](std::string s) {
                this->modRDPParamsData.enable_shared_virtual_disk = !s.empty();
                this->SHARE_DIR = std::move(s);
            })),

            cli::option("remote-dir").help("Remote directory")
            .action(cli::arg_location("directory", this->rDPRemoteAppConfig.source_of_WorkingDir)),

            cli::helper("========= Replay ========="),

            cli::option('R', "replay").help("Enable replay mode")
            .action(cli::on_off_location(this->is_full_replaying)),

            cli::option("replay-path").help("Set filename path for replay mode")
            .action(cli::arg([this](std::string s){
                this->is_full_replaying = true;
                this->full_capture_file_name = std::move(s);
            }))

        );

        auto cli_result = cli::parse(options, argc, argv);
        switch (cli_result.res) {
            case cli::Res::Ok:
                break;
            case cli::Res::Exit:
                // TODO return 0;
                break;
            case cli::Res::Help:
                cli::print_help(options, std::cout);
                // TODO return 0;
                break;
            case cli::Res::BadFormat:
            case cli::Res::BadOption:
                std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
                if (cli_result.opti < cli_result.argc) {
                    std::cerr << " (";
                    if (cli_result.res == cli::Res::BadFormat && cli_result.opti > 1) {
                        std::cerr << cli_result.argv[cli_result.opti-1] << " ";
                    }
                    std::cerr << cli_result.argv[cli_result.opti] << ")";
                }
                std::cerr << "\n";
                // TODO return 1;
                break;
        }
    }


//     std::vector<IconMovieData> get_icon_movie_data() {
//         std::vector<IconMovieData> vec;
//         return vec;
//     }



    void openWindowsData()  {
        if (std::ifstream ifile{this->WINDOWS_CONF}) {
            this->windowsData.no_data = false;

            std::string line;
            int pos = 0;

            getline(ifile, line);
            pos = line.find(' ');
            line = line.substr(pos, line.length());
            this->windowsData.form_x = std::stoi(line);

            getline(ifile, line);
            pos = line.find(' ');
            line = line.substr(pos, line.length());
            this->windowsData.form_y = std::stoi(line);

            getline(ifile, line);
            pos = line.find(' ');
            line = line.substr(pos, line.length());
            this->windowsData.screen_x = std::stoi(line);

            getline(ifile, line);
            pos = line.find(' ');
            line = line.substr(pos, line.length());
            this->windowsData.screen_y = std::stoi(line);
        }
    }

    void writeWindowsData()  {
        this->windowsData.writeWindowsData();
//         std::ofstream ofile(this->WINDOWS_CONF, std::ios::trunc);
//         if (ofile) {
//             ofile
//                 << "form_x " << this->windowsData.form_x << "\n"
//                 << "form_y " << this->windowsData.form_y << "\n"
//                 << "screen_x " << this->windowsData.screen_x << "\n"
//                 << "screen_y " << this->windowsData.screen_y << "\n"
//             ;
//             ofile.close();
//         }
    }

    void setUserProfil()  {
        std::ifstream ifichier(this->USER_CONF_DIR);
        if(ifichier) {
            std::string line;
            std::getline(ifichier, line);
            auto pos(line.find(' '));
            if (line.compare(0, pos, "current_user_profil_id") == 0) {
                this->current_user_profil = std::stoi(line.substr(pos + 1));
            }
        }
    }

    void setCustomKeyConfig()  {
        std::ifstream ifichier(this->MAIN_DIR + CLIENT_REDEMPTION_KEY_SETTING_PATH);

        if(ifichier) {
            this->keyCustomDefinitions.clear();

            std::string ligne;

            while(getline(ifichier, ligne)) {

                int pos(ligne.find(' '));

                if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {

                    ligne = ligne.substr(pos + 1, ligne.length());
                    pos = ligne.find(' ');

                    int qtKeyID  = std::stoi(ligne.substr(0, pos));

                    if (qtKeyID !=  0) {
                        ligne = ligne.substr(pos + 1, ligne.length());
                        pos = ligne.find(' ');

                        int scanCode = 0;
                        scanCode = std::stoi(ligne.substr(0, pos));
                        ligne = ligne.substr(pos + 1, ligne.length());

                        std::string ASCII8 = ligne.substr(0, 1);
                        int next_pos = 2;
                        if (ASCII8 == " ") {
                            ASCII8 = "";
                            next_pos = 1;
                        }

                        ligne = ligne.substr(next_pos, ligne.length());
                        int extended = std::stoi(ligne.substr(0, 1));

                        if (extended) {
                            extended = 1;
                        }
                        pos = ligne.find(' ');

                        std::string name = ligne.substr(pos + 1, ligne.length());

                        this->keyCustomDefinitions.emplace_back(qtKeyID, scanCode, ASCII8, extended, name);
                    }
                }
            }
        }
    }

    void writeCustomKeyConfig()  {
        auto const filename = this->MAIN_DIR + CLIENT_REDEMPTION_KEY_SETTING_PATH;
        remove(filename.c_str());

        std::ofstream ofichier(filename, std::ios::trunc);
        if(ofichier) {
            ofichier << "Key Setting" << std::endl << std::endl;

            for (KeyCustomDefinition & key : this->keyCustomDefinitions) {
                if (key.qtKeyID != 0) {
                    ofichier << "- ";
                    ofichier << key.qtKeyID  << " ";
                    ofichier << key.scanCode << " ";
                    //int key_int = key.ASCII8.data()[0];
                    ofichier << key.ASCII8 << " ";
                    ofichier << key.extended << " ";
                    ofichier << key.name << std::endl;
                }
            }
        }
    }


    void add_key_custom_definition(int qtKeyID, int scanCode, const std::string & ASCII8, int extended, const std::string & name)  {
        this->keyCustomDefinitions.emplace_back(qtKeyID, scanCode, ASCII8, extended, name);
    }



    void setClientInfo()  {

        this->userProfils.clear();
        this->userProfils.emplace_back(0, "Default");

        // file config
        std::ifstream ifichier(this->USER_CONF_DIR);
        if(ifichier) {
            // get config from conf file
            std::string line;
            int read_id(-1);

            while(std::getline(ifichier, line)) {
                auto pos(line.find(' '));
                std::string info = line.substr(pos + 1);

                if (line.compare(0, pos, "id") == 0) {
                    read_id = std::stoi(info);
                } else
                if (line.compare(0, pos, "name") == 0) {
                    if (read_id) {
                        this->userProfils.emplace_back(read_id, info);
                    }
                } else
                if (this->current_user_profil == read_id) {

                    if (line.compare(0, pos, "keylayout") == 0) {
                        this->info.keylayout = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "console_session") == 0) {
                        this->info.console_session = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "brush_cache_code") == 0) {
                        this->info.brush_cache_code = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "bpp") == 0) {
                        this->info.screen_info.bpp = checked_int(std::stoi(info));
                    } else
                    if (line.compare(0, pos, "width") == 0) {
                        this->rdp_width     = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "height") == 0) {
                        this->rdp_height     = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "monitorCount") == 0) {
                        this->info.cs_monitor.monitorCount = std::stoi(info);
//                         this->_monitorCount                 = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "span") == 0) {
                        if (std::stoi(info)) {
                            this->is_spanning = true;
                        } else {
                            this->is_spanning = false;
                        }
                    } else
                    if (line.compare(0, pos, "record") == 0) {
                        if (std::stoi(info)) {
                            this->is_recording = true;
                        } else {
                            this->is_recording = false;
                        }
                    } else
                    if (line.compare(0, pos, "tls") == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_tls = true;
                        } else { this->modRDPParamsData.enable_tls = false; }
                    } else
                    if (line.compare(0, pos, "nla") == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_nla = true;
                        } else { this->modRDPParamsData.enable_nla = false; }
                    } else
                    if (line.compare(0, pos, "sound") == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_sound = true;
                        } else { this->modRDPParamsData.enable_sound = false; }
                    } else
                    if (line.compare(0, pos, "console_mode") == 0) {
                        this->info.console_session = (std::stoi(info) > 0);
                    } else
                    if (line.compare(0, pos, "enable_shared_clipboard") == 0) {
                        if (std::stoi(info)) {
                            this->enable_shared_clipboard = true;
                        }
                    } else
                    if (line.compare(0, pos, "enable_shared_remoteapp") == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_shared_remoteapp = true;
                        }
                    } else
                    if (line.compare(0, pos, "enable_shared_virtual_disk") == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_shared_virtual_disk = true;
                        }
                    } else
                    if (line.compare(0, pos, "mod") == 0) {
                        this->mod_state = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "remote-exe") == 0) {
                         this->rDPRemoteAppConfig.full_cmd_line                = info;
                    } else
                    if (line.compare(0, pos, "remote-dir") == 0) {
                        this->rDPRemoteAppConfig.source_of_WorkingDir                = info;
                    } else
                    if (line.compare(0, pos, "rdp5_performanceflags") == 0) {
                        this->info.rdp5_performanceflags |= std::stoi(info);
                    } else

                    if (line.compare(0, pos, "vnc-applekeyboard ") == 0) {
                        if (std::stoi(info)) {
                            this->vnc_conf.is_apple = true;
                        }
                    } else
                    if (line.compare(0, pos, "share-dir") == 0) {
                        this->SHARE_DIR                 = info;
                        read_id = -1;
                    }
                }
            }
            ifichier.close();
        }
    }

    void setAccountData()  {
        this->_accountNB = 0;
        std::ifstream ifichier(this->USER_CONF_LOG, std::ios::in);

        if (ifichier) {
            int accountNB(0);
            std::string line;

            while(std::getline(ifichier, line)) {
                auto pos(line.find(' '));
                std::string info = line.substr(pos + 1);

                if (line.compare(0, pos, "save_pwd") == 0) {
                    this->_save_password_account = (info == "true");
                } else
                if (line.compare(0, pos, "last_target") == 0) {
                    this->_last_target_index = std::stoi(info);
                } else
                if (line.compare(0, pos, "title") == 0) {
                    AccountData new_account;
                    this->_accountData.push_back(new_account);
                    this->_accountData.back().title = info;
                } else
                if (line.compare(0, pos, "IP") == 0) {
                    this->_accountData.back().IP = info;
                } else
                if (line.compare(0, pos, "name") == 0) {
                    this->_accountData.back().name = info;
                } else if (line.compare(0, pos, "protocol") == 0) {
                    this->_accountData.back().protocol = std::stoi(info);
                } else
                if (line.compare(0, pos, "pwd") == 0) {
                    this->_accountData.back().pwd = info;
                } else
                if (line.compare(0, pos, "options_profil") == 0) {

                    this->_accountData.back().options_profil = std::stoi(info);
                    this->_accountData.back().index = accountNB;

                    accountNB++;
                    if (accountNB == MAX_ACCOUNT_DATA) {
                        this->_accountNB = MAX_ACCOUNT_DATA;
                        accountNB = 0;
                    }
                } else
                if (line.compare(0, pos, "port") == 0) {
                    this->_accountData.back().port = std::stoi(info);
                }
            }

            if (this->_accountNB < MAX_ACCOUNT_DATA) {
                this->_accountNB = accountNB;
            }

            if (this->_last_target_index < this->_accountData.size()) {

                this->target_IP = this->_accountData[this->_last_target_index].IP;
                this->user_name = this->_accountData[this->_last_target_index].name;
                this->user_password = this->_accountData[this->_last_target_index].pwd;
                this->port = this->_accountData[this->_last_target_index].port;
            }
        }
    }



    void writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port)  {
        if (this->connected) {
            bool alreadySet = false;

            std::string title(ip + " - " + name);

            for (int i = 0; i < this->_accountNB; i++) {
                if (this->_accountData[i].title == title) {
                    alreadySet = true;
                    this->_last_target_index = i;
                    this->_accountData[i].pwd  = pwd;
                    this->_accountData[i].port = port;
                    this->_accountData[i].options_profil  = this->current_user_profil;
                }
            }

            if (!alreadySet && (this->_accountNB < MAX_ACCOUNT_DATA)) {
                AccountData new_account;
                this->_accountData.push_back(new_account);
                this->_accountData[this->_accountNB].title = title;
                this->_accountData[this->_accountNB].IP    = ip;
                this->_accountData[this->_accountNB].name  = name;
                this->_accountData[this->_accountNB].pwd   = pwd;
                this->_accountData[this->_accountNB].port  = port;
                this->_accountData[this->_accountNB].options_profil  = this->current_user_profil;
                this->_accountData[this->_accountNB].protocol = this->mod_state;
                this->_accountNB++;

                if (this->_accountNB > MAX_ACCOUNT_DATA) {
                    this->_accountNB = MAX_ACCOUNT_DATA;
                }
                this->_last_target_index = this->_accountNB;
            }

            std::ofstream ofichier(this->USER_CONF_LOG, std::ios::out | std::ios::trunc);
            if(ofichier) {

                if (this->_save_password_account) {
                    ofichier << "save_pwd true" << "\n";
                } else {
                    ofichier << "save_pwd false" << "\n";
                }
                ofichier << "last_target " <<  this->_last_target_index << "\n";
                ofichier << "\n";

                for (int i = 0; i < this->_accountNB; i++) {
                    ofichier << "title " << this->_accountData[i].title << "\n";
                    ofichier << "IP "    << this->_accountData[i].IP    << "\n";
                    ofichier << "name "  << this->_accountData[i].name  << "\n";
                    ofichier << "protocol "  << this->_accountData[i].protocol  << "\n";
                    if (this->_save_password_account) {
                        ofichier << "pwd " << this->_accountData[i].pwd << "\n";
                    } else {
                        ofichier << "pwd " << "\n";
                    }
                    ofichier << "port " << this->_accountData[i].port << "\n";
                    ofichier << "options_profil " << this->_accountData[i].options_profil << "\n";
                    ofichier << "\n";
                }
                ofichier.close();
            }
        }
    }



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




    void set_remoteapp_cmd_line(const std::string & cmd)  {
        this->rDPRemoteAppConfig.full_cmd_line = cmd;
        int pos = cmd.find(' ');
        this->rDPRemoteAppConfig.source_of_ExeOrFile = cmd.substr(0, pos);
        this->rDPRemoteAppConfig.source_of_Arguments = cmd.substr(pos + 1);
    }

    bool is_no_win_data()  {
        return this->windowsData.no_data;
    }

//     void writeWindowsConf() {
//         this->windowsData.write();
//     }

    void deleteCurrentProtile()  {
        std::ifstream ifichier(this->USER_CONF_DIR);
        if(ifichier) {

            std::string new_file_content;
            int ligne_to_jump = 0;

            std::string line;

            std::getline(ifichier, line);

            while(std::getline(ifichier, line)) {
                if (ligne_to_jump == 0) {
                    int pos = line.find(' ');
                    std::string info = line.substr(pos + 1);

                    if (line.compare(0, pos, "id") == 0 && std::stoi(info) == this->current_user_profil) {
                        ligne_to_jump = 18;
                    } else {
                        new_file_content += line + "\n";
                    }
                } else {
                    ligne_to_jump--;
                }
            }

            ifichier.close();

            std::ofstream ofichier(this->USER_CONF_DIR, std::ios::trunc);
            ofichier << "current_user_profil_id 0" << "\n";
            ofichier << new_file_content << std::endl;
        }
    }


    void setDefaultConfig()  {
        //this->current_user_profil = 0;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.brush_cache_code = 0;
        this->info.screen_info.bpp = BitsPerPixel{24};
        this->info.screen_info.width  = 800;
        this->info.screen_info.height = 600;
        this->info.console_session = false;
        this->info.rdp5_performanceflags = 0;               //PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;
        this->is_spanning = false;
        this->is_recording = false;
        this->modRDPParamsData.enable_tls = true;
        this->modRDPParamsData.enable_nla = true;
        this->enable_shared_clipboard = true;
        this->modRDPParamsData.enable_shared_virtual_disk = true;
        this->SHARE_DIR = "/home";
        //this->info.encryptionLevel = 1;
    }


    void writeClientInfo()  {
        std::fstream ofichier(this->USER_CONF_DIR);
        if(ofichier) {

            ofichier << "current_user_profil_id " << this->current_user_profil << "\n";

            std::string ligne;

            bool new_profil = true;
            int read_id = -1;
            auto pos(ligne.find(' '));
            std::string tag  = ligne.substr(0, pos);
            std::string info = ligne.substr(pos + 1);

            while(std::getline(ofichier, ligne)) {
                pos = ligne.find(' ');
                if (ligne.compare(0, pos, "id")) {
                    info = ligne.substr(pos + 1);
                    read_id = std::stoi(info);
                    if (read_id == this->current_user_profil) {
                        new_profil = false;
                        break;
                    }
                }
            }

            if (new_profil) {
                ofichier.close();
                std::ofstream new_ofile(this->USER_CONF_DIR, std::ios::app);
                new_ofile << "\nid "     << this->userProfils[this->current_user_profil].id   << "\n";
                new_ofile << "name "   << this->userProfils[this->current_user_profil].name << "\n";
                new_ofile << "keylayout "             << this->info.keylayout               << "\n";
                new_ofile << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                new_ofile << "bpp "                   << static_cast<int>(this->info.screen_info.bpp)         << "\n";
                new_ofile << "width "                 << this->rdp_width                   << "\n";
                new_ofile << "height "                << this->rdp_height                  << "\n";
                new_ofile << "rdp5_performanceflags " << static_cast<int>(this->info.rdp5_performanceflags) << "\n";
                new_ofile << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                new_ofile << "span "                  << this->is_spanning                  << "\n";
                new_ofile << "record "                << this->is_recording                 << "\n";
                new_ofile << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                new_ofile << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                new_ofile << "sound "                 << this->modRDPParamsData.enable_sound << "\n";
                new_ofile << "console_mode "               << this->info.console_session << "\n";
//                 new_ofile << "delta_time "            << this->delta_time << "\n";
                new_ofile << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                new_ofile << "enable_shared_virtual_disk " << this->modRDPParamsData.enable_shared_virtual_disk << "\n";
                new_ofile << "enable_shared_remoteapp " << this->modRDPParamsData.enable_shared_remoteapp << "\n";
                new_ofile << "share-dir "                              << this->SHARE_DIR << std::endl;
                new_ofile << "remote-exe "                              << this->rDPRemoteAppConfig.full_cmd_line << std::endl;
                new_ofile << "remote-dir "                              << this->rDPRemoteAppConfig.source_of_WorkingDir << std::endl;
                new_ofile << "vnc- applekeyboard "                       << this->vnc_conf.is_apple << std::endl;
                new_ofile << "mod"                              << static_cast<int>(this->mod_state) << std::endl;

                new_ofile.close();

            } else {
                std::getline(ofichier, ligne);
                std::getline(ofichier, ligne);
                ofichier.seekp(ofichier.tellg());
                ofichier << "name "   << this->userProfils[this->current_user_profil].name << "\n";
                ofichier << "keylayout "             << this->info.keylayout               << "\n";
                ofichier << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                ofichier << "bpp "                   << static_cast<int>(this->info.screen_info.bpp)        << "\n";
                ofichier << "width "                 << this->rdp_width                   << "\n";
                ofichier << "height "                << this->rdp_height                  << "\n";
                ofichier << "rdp5_performanceflags " << static_cast<int>(this->info.rdp5_performanceflags)   << "\n";
                ofichier << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                ofichier << "span "                  << this->is_spanning                  << "\n";
                ofichier << "record "                << this->is_recording                 << "\n";
                ofichier << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                ofichier << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                ofichier << "console_mode "               << this->info.console_session << "\n";
                ofichier << "sound "                 << this->modRDPParamsData.enable_sound << "\n";
//                 ofichier << "delta_time "            << this->delta_time << "\n";
                ofichier << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                ofichier << "enable_shared_virtual_disk " << this->modRDPParamsData.enable_shared_virtual_disk << "\n";
                ofichier << "enable_shared_remoteapp " << this->modRDPParamsData.enable_shared_remoteapp << "\n";
                ofichier << "share-dir "                              << this->SHARE_DIR << std::endl;
                ofichier << "remote-exe "                              <<  this->rDPRemoteAppConfig.full_cmd_line << std::endl;
                ofichier << "remote-dir "                              << this->rDPRemoteAppConfig.source_of_WorkingDir << std::endl;
                ofichier << "vnc-applekeyboard "                       << this->vnc_conf.is_apple << std::endl;
                ofichier << "mod "                              << static_cast<int>(this->mod_state) << std::endl;
            }
        }
    }


};



