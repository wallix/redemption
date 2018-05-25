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

#ifndef Q_MOC_RUN

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <climits>
#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <unistd.h>

#include <openssl/ssl.h>

#include "acl/auth_api.hpp"
#include "configs/config.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/channel_list.hpp"
#include "core/client_info.hpp"
#include "core/front_api.hpp"
#include "core/report_message_api.hpp"
#include "gdi/graphic_api.hpp"
#include "keyboard/keymap2.hpp"
#include "mod/internal/client_execute.hpp"
#include "mod/internal/replay_mod.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/cli.hpp"
#include "utils/bitmap.hpp"
#include "utils/genfstat.hpp"
#include "utils/genrandom.hpp"
#include "utils/netutils.hpp"
#include "utils/fileutils.hpp"
#include "main/version.hpp"

#include "program_options/program_options.hpp"

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


    ClientRedemptionAPI()
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
    {}


    virtual void send_clipboard_format() {}

    void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                        , std::size_t , std::size_t , int ) override {}


    // CONTROLLER
    virtual void connect() {}
    virtual void disconnect(std::string const &, bool) {}
    virtual void replay(const std::string &, const std::string &) {}
    virtual bool load_replay_mod(std::string const &, std::string const &, timeval, timeval) { return true; }
    virtual timeval reload_replay_mod(int, timeval) { return timeval{}; }
    virtual bool is_replay_on() { return true; }
    virtual char const * get_mwrm_filename() { return ""; }
    virtual time_t get_real_time_movie_begin() { return time_t{}; }
    virtual void delete_replay_mod() {}
    virtual void callback(bool /*is_timeout*/) {}
    virtual void draw_frame(int ) {}
    virtual void closeFromScreen() {}
    virtual void disconnexionReleased() {}
    virtual void replay_set_pause(timeval) {}
    virtual void replay_set_sync() {}

    virtual void update_keylayout() {}

    bool can_be_start_capture() override { return true; }
};


class ClientRedemptionIOAPI : public ClientRedemptionAPI
{
    // TODO Private !!!!!!!!!!!!!!!!!!!!!!!
public:
    RDPVerbose        verbose;

    CryptoContext     cctx;

    // TODO unique_ptr
    Transport    * socket;
    int                  client_sck;
    TimeSystem           timeSystem;
    NullAuthentifier   authentifier;
    NullReportMessage  reportMessage;

    Keymap2              keymap;
    StaticOutStream<256> decoded_data;    // currently not initialised

    int                  _timer;

    uint8_t commandIsValid;

    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port;
    std::string       local_IP;


    enum : int {
        COMMAND_VALID = 15
      , NAME_GOT      = 1
      , PWD_GOT       = 2
      , IP_GOT        = 4
      , PORT_GOT      = 8
    };

    enum : uint8_t {
        MOD_RDP            = 1,
        MOD_VNC            = 2,
        MOD_RDP_REMOTE_APP = 3,
        MOD_RDP_REPLAY     = 4
    };

    enum : int {
        BALISED_FRAME = 15,
        MAX_ACCOUNT_DATA = 15
    };



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

        ClientRedemptionIOAPI * front;

        WindowsData(ClientRedemptionIOAPI * front)
          : front(front)
        {}

        void open() {
            if (std::ifstream ifile{this->front->WINDOWS_CONF}) {
                 this->no_data = false;

                 std::string line;
                 int pos = 0;

                 getline(ifile, line);
                 pos = line.find(" ");
                 line = line.substr(pos, line.length());
                 this->form_x = std::stoi(line);

                 getline(ifile, line);
                 pos = line.find(" ");
                 line = line.substr(pos, line.length());
                 this->form_y = std::stoi(line);

                 getline(ifile, line);
                 pos = line.find(" ");
                 line = line.substr(pos, line.length());
                 this->screen_x = std::stoi(line);

                 getline(ifile, line);
                 pos = line.find(" ");
                 line = line.substr(pos, line.length());
                 this->screen_y = std::stoi(line);
            }
        }

        void write() {
            std::ofstream ofile(this->front->WINDOWS_CONF, std::ios::trunc);
            if (ofile) {
                ofile
                  << "form_x " << this->form_x << "\n"
                  << "form_y " << this->form_y << "\n"
                  << "screen_x " << this->screen_x << "\n"
                  << "screen_y " << this->screen_y << "\n"
                ;
            }
        }

    } windowsData;


    struct AccountData {
        std::string title;
        std::string IP;
        std::string name;
        std::string pwd;
        int port = 0;
        int options_profil = 0;
        int index = -1;
        int protocol = MOD_RDP;
    }    _accountData[MAX_ACCOUNT_DATA];
    int  _accountNB;
    bool _save_password_account;
    int  _last_target_index;


    uint8_t mod_state;

    bool                 is_recording;
    bool                 is_replaying;
    bool                 is_loading_replay_mod;
    bool				 is_full_capturing;
    bool				 is_full_replaying;
    std::string 		 full_capture_file_name;
    bool                 connected;
    bool                 is_spanning;
//


    std::string _movie_name;
    std::string _movie_dir;
    std::string _movie_full_path;

    int rdp_width;
    int rdp_height;

//     bool wab_diag_question;
//     int asked_color;

    int current_user_profil;


    struct ModRDPParamsData
    {
        int rdp_width;
        int rdp_height;
        bool enable_tls   = false;
        bool enable_nla   = false;
        bool enable_sound = false;
    } modRDPParamsData;

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
        int qtKeyID  = 0;
        int scanCode = 0;
        std::string ASCII8;
        int extended = 0;
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

    bool                 _recv_disconnect_ultimatum;
    BGRPalette           mod_palette;


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
    std::vector<IconMovieData> icons_movie_data;


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

        // TODO ClientRedemptionIOAPI& client
        ModVNCParamsData(SessionReactor& session_reactor, ClientRedemptionIOAPI * client)
          : is_apple(false)
          , exe(session_reactor, *client, this->windowListCaps, false)
          , vnc_encodings("5,16,0,1,-239")
        {}
    } vnc_conf;
    bool wab_diag_question;



    ClientRedemptionIOAPI(SessionReactor& session_reactor, char* argv[], int argc, RDPVerbose verbose)
    : ClientRedemptionAPI()
    , verbose(verbose)
    , cctx()
    , socket(nullptr)
    , client_sck(-1)
    , keymap()
    , _timer(0)
    , commandIsValid(PORT_GOT)
    , port(3389)
    , local_IP("unknow_local_IP")
    , windowsData(this)
    , _accountNB(0)
    , _save_password_account(false)
    , _last_target_index(0)
    , mod_state(MOD_RDP)
    , is_recording(false)
    , is_replaying(false)
    , is_loading_replay_mod(false)
    , is_full_capturing(false)
    , is_full_replaying(false)
    , connected(false)
    , is_spanning(false)
//     , asked_color(0)
    , current_user_profil(0)
    , _recv_disconnect_ultimatum(false)
    , mod_palette(BGRPalette::classic_332())
    , vnc_conf(session_reactor, this)
    , wab_diag_question(false)
    {
        SSL_load_error_strings();
        SSL_library_init();

        this->info.width  = rdp_width;
        this->info.height = rdp_height;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = false;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;

        this->source_of_ExeOrFile = "C:\\Windows\\system32\\notepad.exe";
        this->source_of_WorkingDir = "C:\\Users\\user1";

        this->full_cmd_line = this->source_of_ExeOrFile + " " + this->source_of_Arguments;

        for (auto* pstr : {
            &this->DATA_DIR,
            &this->REPLAY_DIR,
            &this->CB_TEMP_DIR,
            &this->DATA_CONF_DIR,
            &this->SOUND_TEMP_DIR
        }) {
            if (!file_exist(pstr->c_str())) {
                LOG(LOG_INFO, "Create file \"%s\".", pstr->c_str());
                mkdir(pstr->c_str(), 0775);
            }
        }

        this->setDefaultConfig();
        this->setUserProfil();
        this->setClientInfo();
//         this->rdp_width = 1920;
//         this->rdp_height = 1080;
//         this->info.width  = 1920;
//         this->info.height = 1080;
        this->keymap.init_layout(this->info.keylayout);
        this->setCustomKeyConfig();

        this->windowsData.open();
        std::fill(std::begin(this->info.order_caps.orderSupport), std::end(this->info.order_caps.orderSupport), 1);
        this->info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;

        auto options = cli::options(
            cli::helper("Client ReDemPtion Help menu."),

            cli::option('h', "help").help("Show help")
            .action(cli::help),

            cli::option('v', "version").help("Show version")
            .action(cli::quit([]{ std::cout << " ReDemPtion Version " VERSION << "\n"; })),

            cli::helper("========= Connection ========="),

            cli::option('u', "username").help("Set target session user name")
            .action(cli::arg([this](std::string s){
                this->user_name = std::move(s);
                this->commandIsValid += NAME_GOT;
            })),

            cli::option('p', "password").help("Set target session user password")
            .action(cli::arg([this](std::string s){
                this->user_password = std::move(s);
                this->commandIsValid += PWD_GOT;
            })),

            cli::option('i', "ip").help("Set target IP address")
            .action(cli::arg([this](std::string s){
                this->target_IP = std::move(s);
                this->commandIsValid += IP_GOT;
            })),

            cli::option('P', "port").help("Set port to use on target")
            .action(cli::arg([this](int n){
                this->port = n;
                this->commandIsValid += PORT_GOT;
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
                if (!bool(this->commandIsValid & PORT_GOT)) {
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
                    this->source_of_ExeOrFile = std::move(line);
                    this->source_of_Arguments.clear();
                }
                else {
                    this->source_of_ExeOrFile = line.substr(0, pos);
                    this->source_of_Arguments = line.substr(pos + 1);
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
            .action(cli::arg_location("bit_per_pixel", this->info.bpp)),

            cli::option("keylaout").help("Set windows keylayout")
            .action(cli::arg_location(this->info.keylayout)),

            cli::option("enable-record").help("Enable session recording as .wrm movie")
            .action(cli::on_off_location(this->is_recording)),

            cli::option("share-dir").help("Set directory path on local disk to share with your session.")
            .action(cli::arg("directory", [this](std::string s) {
                this->enable_shared_virtual_disk = !s.empty();
                this->SHARE_DIR = std::move(s);
            })),

            cli::option("remote-dir").help("Remote directory")
            .action(cli::arg_location("directory", this->source_of_WorkingDir))
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
    }

    void setUserProfil() {
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

    void setCustomKeyConfig() {
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
                        pos = ligne.find(' ');

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


                        KeyCustomDefinition keyCustomDefinition = {qtKeyID, scanCode, ASCII8, extended, name};

                        this->keyCustomDefinitions.push_back(keyCustomDefinition);
                    }
                }
            }
        }
    }

    void writeCustomKeyConfig() {
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


    void add_key_custom_definition(int qtKeyID, int scanCode, const std::string & ASCII8, int extended, const std::string & name) {

        KeyCustomDefinition keyCustomDefinition = {qtKeyID, scanCode, ASCII8, extended, name};
        this->keyCustomDefinitions.push_back(keyCustomDefinition);

        const ClientRedemptionIOAPI::KeyCustomDefinition & key = this->keyCustomDefinitions[this->keyCustomDefinitions.size() - 1];
    }



    void setClientInfo() {

        this->userProfils.clear();
        this->userProfils.push_back({0, "Default"});

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
                        this->userProfils.push_back({read_id, info.c_str()});
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
                        this->info.bpp = std::stoi(info);
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
                    if (line.compare(0, pos, "enable_shared_virtual_disk") == 0) {
                        if (std::stoi(info)) {
                            this->enable_shared_virtual_disk = true;
                        }
                    } else
                    if (line.compare(0, pos, "mod") == 0) {
                        this->mod_state = std::stoi(info);
                    } else
                    if (line.compare(0, pos, "remote-exe") == 0) {
                        this->full_cmd_line                = info;
                    } else
                    if (line.compare(0, pos, "remote-dir") == 0) {
                        this->source_of_WorkingDir                = info;
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

    void setAccountData() {
        this->_accountNB = 0;
        std::ifstream ifichier(this->USER_CONF_LOG, std::ios::in);

        if (ifichier) {
            int accountNB(0);
            std::string line;

            while(std::getline(ifichier, line)) {
                auto pos(line.find(' '));
                std::string info = line.substr(pos + 1);

                if (line.compare(0, pos, "save_pwd") == 0) {
                    if (info.compare(std::string("true")) == 0) {
                        this->_save_password_account = true;
                    } else {
                        this->_save_password_account = false;
                    }
                } else
                if (line.compare(0, pos, "last_target") == 0) {
                    this->_last_target_index = std::stoi(info);
                } else
                if (line.compare(0, pos, "title") == 0) {
                    this->_accountData[accountNB].title = info;
                } else
                if (line.compare(0, pos, "IP") == 0) {
                    this->_accountData[accountNB].IP = info;
                } else
                if (line.compare(0, pos, "name") == 0) {
                    this->_accountData[accountNB].name = info;
                } else if (line.compare(0, pos, "protocol") == 0) {
                    this->_accountData[accountNB].protocol = std::stoi(info);
                } else
                if (line.compare(0, pos, "pwd") == 0) {
                    this->_accountData[accountNB].pwd = info;
                } else
                if (line.compare(0, pos, "options_profil") == 0) {
                    this->_accountData[accountNB].options_profil = std::stoi(info);
                    this->_accountData[accountNB].index = accountNB+1;
                    accountNB++;
                    if (accountNB == MAX_ACCOUNT_DATA) {
                        this->_accountNB = MAX_ACCOUNT_DATA;
                        accountNB = 0;
                    }
                } else
                if (line.compare(0, pos, "port") == 0) {
                    this->_accountData[accountNB].port = std::stoi(info);
                }
            }

            if (this->_accountNB < MAX_ACCOUNT_DATA) {
                this->_accountNB = accountNB;
            }

            this->target_IP = this->_accountData[this->_last_target_index].IP;
            this->user_name = this->_accountData[this->_last_target_index].name;
            this->user_password = this->_accountData[this->_last_target_index].pwd;
            this->port = this->_accountData[this->_last_target_index].port;
        }
    }



    void writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port) {
        if (this->connected && this->mod !=  nullptr) {
            bool alreadySet = false;

            std::string title(ip + " - " + name);

            for (int i = 0; i < this->_accountNB; i++) {
                if (this->_accountData[i].title.compare(title) == 0) {
                    alreadySet = true;
                    this->_last_target_index = i;
                    this->_accountData[i].pwd  = pwd;
                    this->_accountData[i].port = port;
                    this->_accountData[i].options_profil  = this->current_user_profil;
                }
            }

            if (!alreadySet && (this->_accountNB < MAX_ACCOUNT_DATA)) {
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



    std::vector<IconMovieData> get_icon_movie_data() {

        this->icons_movie_data.clear();

        DIR *dir;
        struct dirent *ent;
        std::string extension(".mwrm");

        if ((dir = opendir (this->REPLAY_DIR.c_str())) != nullptr) {

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

                                IconMovieData iconData = {file_name, file_path, file_version, file_resolution, file_checksum, movie_len};

                                this->icons_movie_data.push_back(iconData);

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

        return this->icons_movie_data;
    }


    void set_remoteapp_cmd_line(const std::string & cmd) {
        this->full_cmd_line = cmd;
        int pos = cmd.find(' ');
        this->source_of_ExeOrFile = cmd.substr(0, pos);
        this->source_of_Arguments = cmd.substr(pos + 1);
    }

    bool is_no_win_data() {
        return this->windowsData.no_data;
    }

    void writeWindowsConf() {
        this->windowsData.write();
    }

    void deleteCurrentProtile() {
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


    void setDefaultConfig() {
        //this->current_user_profil = 0;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = 0;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
        this->info.width  = 800;
        this->info.height = 600;
        this->info.console_session = false;
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;
        this->is_spanning = false;
        this->is_recording = false;
        this->modRDPParamsData.enable_tls = true;
        this->modRDPParamsData.enable_nla = true;
        this->enable_shared_clipboard = true;
        this->enable_shared_virtual_disk = true;
        this->SHARE_DIR = "/home";
        //this->info.encryptionLevel = 1;
    }


    void writeClientInfo() {
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
                new_ofile << "console_session "       << this->info.console_session         << "\n";
                new_ofile << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                new_ofile << "bpp "                   << this->info.bpp                     << "\n";
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
                new_ofile << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                new_ofile << "share-dir "                              << this->SHARE_DIR << std::endl;
                new_ofile << "remote-exe "                              << this->full_cmd_line << std::endl;
                new_ofile << "remote-dir "                              << this->source_of_WorkingDir << std::endl;
                new_ofile << "vnc- applekeyboard "                       << this->vnc_conf.is_apple << std::endl;
                new_ofile << "mod"                              << static_cast<int>(this->mod_state) << std::endl;

                new_ofile.close();

            } else {
                std::getline(ofichier, ligne);
                std::getline(ofichier, ligne);
                ofichier.seekp(ofichier.tellg());
                ofichier << "name "   << this->userProfils[this->current_user_profil].name << "\n";
                ofichier << "keylayout "             << this->info.keylayout               << "\n";
                ofichier << "console_session "       << this->info.console_session         << "\n";
                ofichier << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                ofichier << "bpp "                   << this->info.bpp                       << "\n";
                ofichier << "width "                 << this->rdp_width                   << "\n";
                ofichier << "height "                << this->rdp_height                  << "\n";
                ofichier << "rdp5_performanceflags " << this->info.rdp5_performanceflags   << "\n";
                ofichier << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                ofichier << "span "                  << this->is_spanning                  << "\n";
                ofichier << "record "                << this->is_recording                 << "\n";
                ofichier << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                ofichier << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                ofichier << "console_mode "               << this->info.console_session << "\n";
                ofichier << "sound "                 << this->modRDPParamsData.enable_sound << "\n";
//                 ofichier << "delta_time "            << this->delta_time << "\n";
                ofichier << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                ofichier << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                ofichier << "share-dir "                              << this->SHARE_DIR << std::endl;
                ofichier << "remote-exe "                              << this->full_cmd_line << std::endl;
                ofichier << "remote-dir "                              << this->source_of_WorkingDir << std::endl;
                ofichier << "vnc-applekeyboard "                       << this->vnc_conf.is_apple << std::endl;
                ofichier << "mod "                              << static_cast<int>(this->mod_state) << std::endl;
            }
        }
    }


    virtual void refreshPressed() {
        if (this->mod != nullptr) {
            Rect rect(0, 0, this->info.width, this->info.height);
            this->mod->rdp_input_invalidate(rect);
        }
    }

    void CtrlAltDelPressed() {
        int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void CtrlAltDelReleased() {
        int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    virtual void mouseButtonEvent(int x, int y, int flag) {
        if (this->mod != nullptr) {
            this->mod->rdp_input_mouse(flag, x, y, &(this->keymap));
        }
    }

    virtual void wheelEvent(int ,  int , int delta) {
        int flag(MOUSE_FLAG_HWHEEL);
        if (delta < 0) {
            flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
        }
        if (this->mod != nullptr) {
            //this->mod->rdp_input_mouse(flag, e->x(), e->y(), &(this->keymap));
        }
    }

    virtual bool mouseMouveEvent(int x, int y) {

        if (this->mod != nullptr && y < this->info.height) {
            this->mouse_data.x = x;
            this->mouse_data.y = y;
            this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y, &(this->keymap));
        }

        return false;
    }

    void send_rdp_scanCode(int keyCode, int flag) {
        bool tsk_switch_shortcuts = false;
        Keymap2::DecodedKeys decoded_keys = this->keymap.event(flag, keyCode, tsk_switch_shortcuts);
        switch (decoded_keys.count)
        {
        case 2:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[1]);
            }
            break;
        case 1:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            break;
        default:
        case 0:
            break;
        }
        if (this->mod != nullptr) {
            this->mod->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->keymap));
        }
    }

    void send_rdp_unicode(uint16_t unicode, uint16_t flag) {
        this->mod->rdp_input_unicode(unicode, flag);
    }

    virtual time_t get_movie_time_length(char const * mwrm_filename) = 0;

    virtual void instant_play_client(std::chrono::microseconds time) = 0;

};



class ClientIO
{
public:
    ClientRedemptionIOAPI * client;

    void set_client(ClientRedemptionIOAPI * client) {
        this->client = client;
    }
};


class ClientIOClipboardAPI : public ClientIO {

public:
    enum : int {
        FILEGROUPDESCRIPTORW_BUFFER_TYPE = 0,
        IMAGE_BUFFER_TYPE                = 1,
        TEXT_BUFFER_TYPE                 = 2
    };

    uint16_t    _bufferTypeID;
    int         _bufferTypeNameIndex;
    bool        _local_clipboard_stream;
    size_t      _cliboard_data_length;
    int         _cItems;

public:
    ClientIOClipboardAPI()
      : _bufferTypeID(0)
      , _bufferTypeNameIndex(0)
      , _local_clipboard_stream(true)
      , _cliboard_data_length(0)
      , _cItems(0)
      {}

    virtual ~ClientIOClipboardAPI() = default;

    // control state
    virtual void emptyBuffer() = 0;

    void set_local_clipboard_stream(bool val) {
        this->_local_clipboard_stream = val;
    }

    bool get_local_clipboard_stream() {
        return this->_local_clipboard_stream;
    }

    //  set distant clipboard data
    virtual void setClipboard_text(std::string const& str) = 0;
    virtual void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) = 0;
    virtual void setClipboard_files(std::string const& name) = 0;
    virtual void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) = 0;


    //  get local clipboard data
    uint16_t get_buffer_type_id() {
        return this->_bufferTypeID;
    }

    size_t get_cliboard_data_length() {
        return _cliboard_data_length;
    }

    int get_buffer_type_long_name() {
        return this->_bufferTypeNameIndex;
    }

    virtual ConstImageDataView get_image()
    {
        return ConstImageDataView(
            reinterpret_cast<uint8_t const*>(""),
            0, 0, 0, ConstImageDataView::BitsPerPixel{},
            ConstImageDataView::Storage::TopToBottom
        );
    }

    // files data (file index to identify a file among a files group descriptor)
    virtual std::string get_file_item_name(int index) {(void)index; return {};}

    // TODO should be `array_view_const_char get_file_item_size(int index)`
    virtual  int get_file_item_size(int index) {(void) index; return 0;}
    virtual char * get_file_item_data(int index) {(void) index; /*TODO char const/string_view*/ return const_cast<char*>("");}

    int get_citems_number() {
        return this->_cItems;
    }

//     virtual ~ClientIOClipboardAPI();

};



#include "core/FSCC/FileInformation.hpp"

constexpr long long WINDOWS_TICK = 10000000;
constexpr long long SEC_TO_UNIX_EPOCH = 11644473600LL;

class ClientIODiskAPI : public ClientIO {


public:
    virtual ~ClientIODiskAPI() = default;

    struct FileStat {

        uint64_t LastAccessTime = 0;
        uint64_t LastWriteTime  = 0;
        uint64_t CreationTime   = 0;
        uint64_t ChangeTime     = 0;
        uint32_t FileAttributes = 0;

        int64_t  AllocationSize = 0;
        int64_t  EndOfFile      = 0;
        uint32_t NumberOfLinks  = 0;
        uint8_t  DeletePending  = 0;
        uint8_t  Directory      = 0;
    };

    struct FileStatvfs {

        uint64_t VolumeCreationTime             = 0;
        const char * VolumeLabel                = "";
        const char * FileSystemName             = "ext4";

        uint32_t FileSystemAttributes           = fscc::NEW_FILE_ATTRIBUTES;
        uint32_t SectorsPerAllocationUnit       = 8;

        uint32_t BytesPerSector                 = 0;
        uint32_t MaximumComponentNameLength     = 0;
        uint64_t TotalAllocationUnits           = 0;
        uint64_t CallerAvailableAllocationUnits = 0;
        uint64_t AvailableAllocationUnits       = 0;
        uint64_t ActualAvailableAllocationUnits = 0;
    };




    unsigned WindowsTickToUnixSeconds(long long windowsTicks) {
        return unsigned((windowsTicks / WINDOWS_TICK) - SEC_TO_UNIX_EPOCH);
    }

    long long UnixSecondsToWindowsTick(unsigned unixSeconds) {
        return ((unixSeconds + SEC_TO_UNIX_EPOCH) * WINDOWS_TICK);
    }

    uint32_t string_to_hex32(unsigned char * str) {
        size_t size = sizeof(str);
        uint32_t hex32(0);
        for (size_t i = 0; i < size; i++) {
            int s = str[i];
            if(s > 47 && s < 58) {                      //this covers 0-9
                hex32 += (s - 48) << (size - i - 1);
            } else if (s > 64 && s < 71) {              // this covers A-F
                hex32 += (s - 55) << (size - i - 1);
            } else if (s > 'a'-1 && s < 'f'+1) {        // this covers a-f
                hex32 += (s - 'a') << (size - i - 1);
            }
        }
        return hex32;
    }

    virtual bool ifile_good(const char * new_path) = 0;

    virtual bool ofile_good(const char * new_path) = 0;

    virtual bool dir_good(const char * new_path) = 0;

    virtual void marke_dir(const char * new_path) = 0;

    virtual FileStat get_file_stat(const char * file_to_request) = 0;

    virtual FileStatvfs get_file_statvfs(const char * file_to_request) = 0;

    // TODO `log_error_on` is suspecious
    virtual erref::NTSTATUS read_data(
        std::string const& file_to_tread, int offset, byte_array data,
        bool log_error_on) = 0;

    virtual bool set_elem_from_dir(std::vector<std::string> & elem_list, const std::string & str_dir_path) = 0;

    virtual int get_device(const char * file_path) = 0;

    virtual uint32_t get_volume_serial_number(int device) = 0;

    virtual bool write_file(const char * file_to_write, const char * data, int data_len) = 0;

    virtual bool remove_file(const char * file_to_remove) = 0;

    virtual bool rename_file(const char * file_to_rename,  const char * new_name) = 0;
};



class ClientOutputSoundAPI : public ClientIO {

public:
    uint32_t n_sample_per_sec = 0;
    uint16_t bit_per_sample = 0;
    uint16_t n_channels = 0;
    uint16_t n_block_align = 0;
    uint32_t bit_per_sec = 0;

    std::string path;

    void set_path(const std::string & path) {
        this->path = path;
    }

    virtual void init(size_t raw_total_size) = 0;
    virtual void setData(const uint8_t * data, size_t size) = 0;
    virtual void play() = 0;

    virtual ~ClientOutputSoundAPI() = default;

};



class ClientInputSocketAPI : public ClientIO {

public:
    mod_api * _callback = nullptr;

    virtual bool start_to_listen(int client_sck, mod_api * mod) = 0;
    virtual void disconnect() = 0;

    virtual ~ClientInputSocketAPI() = default;
};



class ClientInputMouseKeyboardAPI : public ClientIO {


public:

    ClientInputMouseKeyboardAPI() = default;

    virtual ~ClientInputMouseKeyboardAPI() = default;


    virtual ClientRedemptionIOAPI * get_client() {
        return this->client;
    }

    virtual void update_keylayout() = 0;

    virtual void init_form() = 0;

    virtual void pre_load_movie() {}


    // CONTROLLER
    virtual void connexionReleased() {
        this->client->connect();
    }

    virtual void disconnexionReleased() {
        this->client->disconnexionReleased();
    }

    void CtrlAltDelPressed() {
        this->client->CtrlAltDelPressed();
    }

    void CtrlAltDelReleased() {
        this->client->CtrlAltDelReleased();
    }

    virtual void mouseButtonEvent(int x, int y, int flag) {
        this->client->mouseButtonEvent(x, y, flag);
    }

    virtual void wheelEvent(int x,  int y, int delta) {
        this->client->wheelEvent(x, y, delta);
    }

    virtual bool mouseMouveEvent(int x, int y) {
        return this->client->mouseMouveEvent(x, y);
    }

    // TODO string_view
    void virtual keyPressEvent(const int key, std::string const& text)  = 0;

    // TODO string_view
    void virtual keyReleaseEvent(const int key, std::string const& text)  = 0;

    void virtual refreshPressed() {
        this->client->refreshPressed();
    }

    virtual void open_options() {}

    // TODO string_view
    virtual ClientRedemptionIOAPI::KeyCustomDefinition get_key_info(int, std::string const&) {
        return ClientRedemptionIOAPI::KeyCustomDefinition(0, 0, "", 0, "");
    }

};



class ClientOutputGraphicAPI {

public:
    ClientRedemptionIOAPI * drawn_client;

    const int screen_max_width;
    const int screen_max_height;

    bool is_pre_loading;

    ClientOutputGraphicAPI(int max_width, int max_height)
      : drawn_client(nullptr),
		screen_max_width(max_width)
      , screen_max_height(max_height)
      , is_pre_loading(false) {
    }

    virtual ~ClientOutputGraphicAPI() = default;

    virtual void set_drawn_client(ClientRedemptionIOAPI * client) {
        this->drawn_client = client;
    }

    virtual void set_ErrorMsg(std::string const & movie_path) = 0;

    virtual void dropScreen() = 0;

    virtual void show_screen() = 0;

    virtual void reset_cache(int w,  int h) = 0;

    virtual void create_screen() = 0;

    virtual void closeFromScreen() = 0;

    virtual void set_screen_size(int x, int y) = 0;

    virtual void update_screen() = 0;


    // replay mod

    virtual void create_screen(std::string const & , std::string const & ) {}

    virtual void draw_frame(int ) {}


    // remote app

    virtual void create_remote_app_screen(uint32_t , int , int , int , int ) {}

    virtual void move_screen(uint32_t , int , int ) {}

    virtual void set_screen_size(uint32_t , int , int ) {}

    virtual void set_pixmap_shift(uint32_t , int , int ) {}

    virtual int get_visible_width(uint32_t ) {return 0;}

    virtual int get_visible_height(uint32_t ) {return 0;}

    virtual int get_mem_width(uint32_t ) {return 0;}

    virtual int get_mem_height(uint32_t ) {return 0;}

    virtual void set_mem_size(uint32_t , int , int ) {}

    virtual void show_screen(uint32_t ) {}

    virtual void dropScreen(uint32_t ) {}

    virtual void clear_remote_app_screen() {}




    virtual FrontAPI::ResizeResult server_resize(int width, int height, int bpp) = 0;

    virtual void set_pointer(Pointer      const &) {}

    virtual void draw(RDP::FrameMarker    const & cmd) = 0;
    virtual void draw(RDPNineGrid const & , Rect , gdi::ColorCtx , Bitmap const & ) = 0;
    virtual void draw(RDPDestBlt          const & cmd, Rect clip) = 0;
    virtual void draw(RDPMultiDstBlt      const & cmd, Rect clip) = 0;
    virtual void draw(RDPScrBlt           const & cmd, Rect clip) = 0;
    virtual void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) = 0;
    virtual void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) = 0;
    virtual void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) = 0;

    virtual void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) = 0;
    virtual void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) = 0;
    virtual void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) = 0;


    // TODO The 2 methods below should not exist and cache access be done before calling drawing orders
//     virtual void draw(RDPColCache   const &) {}
//     virtual void draw(RDPBrushCache const &) {}

    virtual void begin_update() {}
    virtual void end_update() {}
};
