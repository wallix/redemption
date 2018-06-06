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




#include <algorithm>
// #include <string>

#include <climits>
#include <cstdint>
#include <openssl/ssl.h>


#include "utils/cli.hpp"
#include "core/RDP/clipboard.hpp"
#include "main/version.hpp"

#include "client_redemption/client_redemption_api.hpp"

#include "client_redemption/client_input_output_api/rdp_clipboard_config.hpp"




class ClientRedemptionConfig: public ClientRedemptionAPI
{

private:
    std::vector<IconMovieData> icons_movie_data;

public:
    RDPVerbose        verbose;
    //bool                _recv_disconnect_ultimatum;
    bool wab_diag_question;

    RDPClipboardConfig rDPClipboardConfig;



    ClientRedemptionConfig(SessionReactor& session_reactor, char* argv[], int argc, RDPVerbose verbose)
    : ClientRedemptionAPI(session_reactor)
    , verbose(verbose)
    //, _recv_disconnect_ultimatum(false)
    , wab_diag_question(false)
    {
        SSL_load_error_strings();
        SSL_library_init();

        this->setDefaultConfig();

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

        // Set RDP CLIPRDR config
        this->rDPClipboardConfig.channel_flags = CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL;
        this->rDPClipboardConfig.arbitrary_scale = 40;
        this->rDPClipboardConfig.total_format_list_pdu_size = 0;
        this->rDPClipboardConfig.server_use_long_format_names = true;
        this->rDPClipboardConfig.cCapabilitiesSets = 1;
        this->rDPClipboardConfig.generalFlags = RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS;
        this->rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, std::string(RDPECLIP::FILEGROUPDESCRIPTORW));
        this->rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, std::string(RDPECLIP::FILECONTENTS));
        this->rDPClipboardConfig.add_format(RDPECLIP::CF_TEXT, std::string(""));
        this->rDPClipboardConfig.add_format(RDPECLIP::CF_METAFILEPICT, std::string(""));


        this->setUserProfil();
        this->setClientInfo();
        this->setCustomKeyConfig();
        this->setAccountData();

        this->openWindowsData();
        std::fill(std::begin(this->info.order_caps.orderSupport), std::end(this->info.order_caps.orderSupport), 1);
        this->info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;
;

        //this->parse_options(argc, argv);

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

    ~ClientRedemptionConfig() = default;

    void parse_options(int argc, char const* const argv[])
    {
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
            .action(cli::arg_location("directory", this->source_of_WorkingDir)),


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




    void openWindowsData() override {
        if (std::ifstream ifile{this->WINDOWS_CONF}) {
            this->windowsData.no_data = false;

            std::string line;
            int pos = 0;

            getline(ifile, line);
            pos = line.find(" ");
            line = line.substr(pos, line.length());
            this->windowsData.form_x = std::stoi(line);

            getline(ifile, line);
            pos = line.find(" ");
            line = line.substr(pos, line.length());
            this->windowsData.form_y = std::stoi(line);

            getline(ifile, line);
            pos = line.find(" ");
            line = line.substr(pos, line.length());
            this->windowsData.screen_x = std::stoi(line);

            getline(ifile, line);
            pos = line.find(" ");
            line = line.substr(pos, line.length());
            this->windowsData.screen_y = std::stoi(line);

            ifile.close();
        }
    }

    void writeWindowsData() override {
        std::ofstream ofile(this->WINDOWS_CONF, std::ios::trunc);
        if (ofile) {
            ofile
                << "form_x " << this->windowsData.form_x << "\n"
                << "form_y " << this->windowsData.form_y << "\n"
                << "screen_x " << this->windowsData.screen_x << "\n"
                << "screen_y " << this->windowsData.screen_y << "\n"
            ;
            ofile.close();
        }
    }

    void setUserProfil() override {
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

    void setCustomKeyConfig() override {
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

    void writeCustomKeyConfig() override {
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


    void add_key_custom_definition(int qtKeyID, int scanCode, const std::string & ASCII8, int extended, const std::string & name) override {

        KeyCustomDefinition keyCustomDefinition = {qtKeyID, scanCode, ASCII8, extended, name};
        this->keyCustomDefinitions.push_back(keyCustomDefinition);
    }



    void setClientInfo() override {

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

    void setAccountData() override {
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



    void writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {
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



    std::vector<IconMovieData> get_icon_movie_data() override {

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


    void set_remoteapp_cmd_line(const std::string & cmd) override {
        this->full_cmd_line = cmd;
        int pos = cmd.find(' ');
        this->source_of_ExeOrFile = cmd.substr(0, pos);
        this->source_of_Arguments = cmd.substr(pos + 1);
    }

    bool is_no_win_data() override {
        return this->windowsData.no_data;
    }

//     void writeWindowsConf() {
//         this->windowsData.write();
//     }

    void deleteCurrentProtile() override {
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


    void setDefaultConfig() override {
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


    void writeClientInfo() override {
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
};



