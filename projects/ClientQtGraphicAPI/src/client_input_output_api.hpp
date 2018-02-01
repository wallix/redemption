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
   Author(s): Clément Moroldo
*/



#pragma once



#include "utils/log.hpp"

#ifndef Q_MOC_RUN
#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "core/report_message_api.hpp"
#include "acl/auth_api.hpp"
#include "utils/genfstat.hpp"
#include "core/front_api.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "transport/socket_transport.hpp"
// #include "keymaps/qt_scancode_keymap.hpp"
#include "core/client_info.hpp"
#include "mod/internal/replay_mod.hpp"
#include "configs/config.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/netutils.hpp"
#include "utils/genrandom.hpp"
#include "keyboard/keymap2.hpp"
#include "transport/crypto_transport.hpp"

#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/channel_list.hpp"
#include "gdi/graphic_api.hpp"


#include "core/RDP/RDPDrawable.hpp"


#endif

#define REPLAY_PATH "/DATA/replay"
#define LOGINS_PATH "/DATA/config/login.config"
#define WINODW_CONF_PATH "/DATA/config/windows_config.config"
#define SHARE_PATH "/DATA/share"
#define CB_FILE_TEMP_PATH "/DATA/clipboard_temp"
#define KEY_SETTING_PATH "/DATA/config/keySetting.config"
#define USER_CONF_PATH "/DATA/config/userConfig.config"

#define DATA_PATH "/DATA"
#define DATA_CONF_PATH "/DATA/config"

#ifndef MAIN_PATH
# error "undefined MAIN_PATH macro"
# define MAIN_PATH ""
#endif






class ClientRedemptionIOAPI : public FrontAPI
{

    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   = 1
      , PWD_GOTTEN    = 2
      , IP_GOTTEN     = 4
      , PORT_GOTTEN   = 8
    };

public:
    RDPVerbose        verbose;
    ClientInfo        info;
    CryptoContext     cctx;


    mod_api            * mod;

    SocketTransport    * socket;
    TimeSystem           timeSystem;
    NullAuthentifier    authentifier;
    NullReportMessage  reportMessage;

    Keymap2              keymap;
    StaticOutStream<256> decoded_data;    // currently not initialised

    int                  _timer;

    const std::string    MAIN_DIR;
    const std::string    REPLAY_DIR;
    const std::string    USER_CONF_LOG;
    const std::string    WINDOWS_CONF;
    const std::string    CB_TEMP_DIR;
    std::string          SHARE_DIR;
    const std::string    USER_CONF_DIR;
    const std::string    DATA_DIR;
    const std::string    DATA_CONF_DIR;

    struct MouseData {
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port;
    std::string       local_IP;

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
            std::ifstream ifile(this->front->WINDOWS_CONF, std::ios::in);
            if (ifile) {
                this->no_data = false;

                std::string line;
                std::string delimiter = " ";

                while(std::getline(ifile, line)) {
                    auto pos(line.find(delimiter));
                    std::string tag  = line.substr(0, pos);
                    std::string info = line.substr(pos + delimiter.length(), line.length());

                    if (tag.compare(std::string("form_x")) == 0) {
                        this->form_x = std::stoi(info);
                    } else
                      if (tag.compare(std::string("form_y")) == 0) {
                        this->form_y = std::stoi(info);
                    } else
                    if (tag.compare(std::string("screen_x")) == 0) {
                        this->screen_x = std::stoi(info);
                    } else
                    if (tag.compare(std::string("screen_y")) == 0) {
                        this->screen_y = std::stoi(info);
                    }
                }

                ifile.close();
            }
        }

        void write() {
            std::ofstream ofile(this->front->WINDOWS_CONF, std::ios::out | std::ios::trunc);
            if (ofile) {

                ofile  << "form_x " << this->form_x <<  "\n";
                ofile  << "form_y " << this->form_y <<  "\n";
                ofile  << "screen_x " << this->screen_x <<  "\n";
                ofile  << "screen_y " << this->screen_y <<  "\n";

                ofile.close();
            }
        }

    } windowsData;


    bool remoteapp;
    bool vnc;

    enum : uint8_t {
        MOD_RDP            = 1,
        MOD_VNC            = 2,
        MOD_RDP_REMOTE_APP = 3,
        MOD_RDP_REPLAY     = 4
    };

    uint8_t mod_state;

    std::unique_ptr<ReplayMod> replay_mod;
    bool                 is_recording;
    bool                 is_replaying;
    bool                 connected;


    bool                 is_spanning;
    Fstat fstat;


    std::string _movie_name;
    std::string _movie_dir;

    bool wab_diag_question;
    int asked_color;

    int current_user_profil;

    std::string close_box_extra_message_ref;


    int client_sck;


    struct ModRDPParamsData
    {
        bool enable_tls                      = false;
        bool enable_nla                      = false;
        bool enable_sound                    = false;
    } modRDPParamsData;

    struct UserProfil {
        int id;
        std::string name;

        UserProfil(int id, const char * name)
          : id(id)
          , name(name) {}
    };
    std::vector<UserProfil> userProfils;

    bool                 enable_shared_clipboard;
    bool                 enable_shared_virtual_disk;

    struct KeyCustomDefinition {
        int qtKeyID  = 0;
        int scanCode = 0;
        int ASCII8   = 0;
        int extended = 0;

        KeyCustomDefinition(int qtKeyID, int scanCode, int ASCII8, int extended)
          : qtKeyID(qtKeyID)
          , scanCode(scanCode)
          , ASCII8(ASCII8)
          , extended(extended) {}
    };
    std::vector<KeyCustomDefinition> keyCustomDefinitions;

    bool                 _recv_disconnect_ultimatum;
    BGRPalette           mod_palette;



    ClientRedemptionIOAPI(char* argv[], int argc, RDPVerbose verbose)
    : verbose(verbose)
    , info()
    , cctx()
    , mod(nullptr)
    , socket(nullptr)
    , keymap()
    , _timer(0)
    , MAIN_DIR(MAIN_PATH)
    , REPLAY_DIR(MAIN_PATH REPLAY_PATH)
    , USER_CONF_LOG(MAIN_PATH LOGINS_PATH)
    , WINDOWS_CONF(MAIN_PATH WINODW_CONF_PATH)
    , CB_TEMP_DIR(MAIN_DIR + std::string(CB_FILE_TEMP_PATH))
    , SHARE_DIR(MAIN_DIR + std::string(SHARE_PATH))
    , USER_CONF_DIR(MAIN_DIR + std::string(USER_CONF_PATH))
    , DATA_DIR(MAIN_DIR + std::string(DATA_PATH))
    , DATA_CONF_DIR(MAIN_DIR + std::string(DATA_CONF_PATH))
    , port(0)
    , local_IP("unknow_local_IP")
    , windowsData(this)
    , mod_state(MOD_RDP)
//     , replay_mod(nullptr)
    , is_recording(false)
    , is_replaying(false)
    , connected(false)
    , is_spanning(false)
    , wab_diag_question(false)
    , asked_color(0)
    , current_user_profil(0)
    , close_box_extra_message_ref("Close")
    , _recv_disconnect_ultimatum(false)
    , mod_palette(BGRPalette::classic_332())
    {
        SSL_load_error_strings();
        SSL_library_init();

        this->info.width  = 800;
        this->info.height = 600;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = 0;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
//         this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
        if (this->info.bpp ==  32) {
//             this->imageFormatARGB = this->bpp_to_QFormat(this->info.bpp, true);
        }
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;


        struct stat sb;

        stat(this->DATA_DIR.c_str(), &sb);
        if (!(S_ISDIR(sb.st_mode))) {
            mkdir(this->DATA_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        stat(this->REPLAY_DIR.c_str(), &sb);
        if (!(S_ISDIR(sb.st_mode))) {
            mkdir(this->REPLAY_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        stat(this->CB_TEMP_DIR.c_str(), &sb);
        if (!(S_ISDIR(sb.st_mode))) {
            mkdir(this->CB_TEMP_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        stat(this->DATA_CONF_DIR.c_str(), &sb);
        if (!(S_ISDIR(sb.st_mode))) {
            mkdir(this->DATA_CONF_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        this->setDefaultConfig();
        this->setUserProfil();
        this->setClientInfo();
        this->keymap.init_layout(this->info.keylayout);

        this->windowsData.open();
        std::fill(std::begin(this->info.order_caps.orderSupport), std::end(this->info.order_caps.orderSupport), 1);
        this->info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;

        uint8_t commandIsValid(0);

        // TODO QCommandLineParser / program_options
        for (int i = 0; i <  argc; i++) {

            std::string word(argv[i]);

            if (       word == "-n") {
                if (i < argc-1) {
                    this->user_name = std::string(argv[i+1]);
                    commandIsValid += NAME_GOTTEN;
                }
            } else if (word == "-w") {
                if (i < argc-1) {
                    this->user_password = std::string(argv[i+1]);
                    commandIsValid += PWD_GOTTEN;
                }
            } else if (word == "-i") {
                if (i < argc-1) {
                    this->target_IP = std::string(argv[i+1]);
                    commandIsValid += IP_GOTTEN;
                }
            } else if (word == "-p") {
                if (i < argc-1) {
                    this->port = std::stoi(std::string(argv[i+1]));
                    commandIsValid += PORT_GOTTEN;
                }
            } else if (word == "--rdpdr") {
                this->verbose = RDPVerbose::rdpdr | this->verbose;
                 std::cout << "--rdpdr rdpdr verbose on";
            } else if (word == "--rdpsnd") {
                this->verbose = RDPVerbose::rdpsnd | this->verbose;
            } else if (word == "--cliprdr") {
                this->verbose = RDPVerbose::cliprdr | this->verbose;
            } else if (word == "--graphics") {
                this->verbose = RDPVerbose::graphics | this->verbose;
            } else if (word == "--printer") {
                this->verbose = RDPVerbose::printer | this->verbose;
            } else if (word == "--rdpdr_dump") {
                this->verbose = RDPVerbose::rdpdr_dump | this->verbose;
            } else if (word == "--cliprdr_dump") {
                this->verbose = RDPVerbose::cliprdr_dump | this->verbose;
            } else if (word == "--basic_trace") {
                this->verbose = RDPVerbose::basic_trace | this->verbose;
            } else if (word == "--connection") {
                this->verbose = RDPVerbose::connection | this->verbose;
            } else if (word == "--rail_order") {
                this->verbose = RDPVerbose::rail_order | this->verbose;
            } else if (word == "--asynchronous_task") {
                this->verbose = RDPVerbose::asynchronous_task | this->verbose;
            } else if (word == "--capabilities") {
                this->verbose = RDPVerbose::capabilities | this->verbose;
            } else if (word ==  "--keyboard") {
                //this->qtRDPKeymap._verbose = 1;
            } else if (word ==  "--rail") {
                this->verbose = RDPVerbose::rail | this->verbose;
            } else if (word ==  "--rail_dump") {
                this->verbose = RDPVerbose::rail_dump | this->verbose;
            } else if (word ==  "--vnc") {
                this->mod_state = MOD_VNC;
            } else if (word ==  "--remote_app") {
                this->mod_state = MOD_RDP_REMOTE_APP;
            }
        }
    }

    void setUserProfil() {
        std::ifstream ifichier(this->USER_CONF_DIR);
        if(ifichier) {
            std::string ligne;
            std::string delimiter = " ";
            std::getline(ifichier, ligne);
            auto pos(ligne.find(delimiter));
            std::string tag  = ligne.substr(0, pos);
            std::string info = ligne.substr(pos + delimiter.length(), ligne.length());
            if (tag.compare(std::string("current_user_profil_id")) == 0) {
                this->current_user_profil = std::stoi(info);
            }
        }
    }

    void setClientInfo() {

        this->userProfils.clear();
        this->userProfils.push_back({0, "Default"});

        // file config
        std::ifstream ifichier(this->USER_CONF_DIR);
        if(ifichier) {
            // get config from conf file
            std::string ligne;
            std::string delimiter = " ";
            int read_id(-1);

            while(std::getline(ifichier, ligne)) {
                auto pos(ligne.find(delimiter));
                std::string tag  = ligne.substr(0, pos);
                std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

                if (tag.compare(std::string("id")) == 0) {
                    read_id = std::stoi(info);
                } else
                if (tag.compare(std::string("name")) == 0) {
                    if (read_id) {
                        this->userProfils.push_back({read_id, info.c_str()});
                    }
                } else
                if (this->current_user_profil == read_id) {

                    if (tag.compare(std::string("keylayout")) == 0) {
                        this->info.keylayout = std::stoi(info);
                    } else
                    if (tag.compare(std::string("console_session")) == 0) {
                        this->info.console_session = std::stoi(info);
                    } else
                    if (tag.compare(std::string("brush_cache_code")) == 0) {
                        this->info.brush_cache_code = std::stoi(info);
                    } else
                    if (tag.compare(std::string("bpp")) == 0) {
                        this->info.bpp = std::stoi(info);
                    } else
                    if (tag.compare(std::string("width")) == 0) {
                        this->info.width      = std::stoi(info);
                    } else
                    if (tag.compare(std::string("height")) == 0) {
                        this->info.height     = std::stoi(info);
                    } else
                    if (tag.compare(std::string("rdp5_performanceflags")) == 0) {
                        this->info.rdp5_performanceflags = std::stoi(info);
                    } /*else
                    if (tag.compare(std::string("fps")) == 0) {
                        this->fps = std::stoi(info);
                    }*/ else
                    if (tag.compare(std::string("monitorCount")) == 0) {
                        this->info.cs_monitor.monitorCount = std::stoi(info);
//                         this->_monitorCount                 = std::stoi(info);
                    } else
                    if (tag.compare(std::string("span")) == 0) {
                        if (std::stoi(info)) {
                            this->is_spanning = true;
                        } else {
                            this->is_spanning = false;
                        }
                    } else
                    if (tag.compare(std::string("record")) == 0) {
                        if (std::stoi(info)) {
                            this->is_recording = true;
                        } else {
                            this->is_recording = false;
                        }
                    } else
                    if (tag.compare(std::string("tls")) == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_tls = true;
                        } else { this->modRDPParamsData.enable_tls = false; }
                    } else
                    if (tag.compare(std::string("nla")) == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_nla = true;
                        } else { this->modRDPParamsData.enable_nla = false; }
                    } else
                    if (tag.compare(std::string("sound")) == 0) {
                        if (std::stoi(info)) {
                            this->modRDPParamsData.enable_sound = true;
                        } else { this->modRDPParamsData.enable_sound = false; }
                    } /*else
                    if (tag.compare(std::string("delta_time")) == 0) {
                        if (std::stoi(info)) {
                            this->delta_time = std::stoi(info);
                        }
                    }*/ else
                    if (tag.compare(std::string("enable_shared_clipboard")) == 0) {
                        if (std::stoi(info)) {
                            this->enable_shared_clipboard = true;
                        }
                    } else
                    if (tag.compare(std::string("enable_shared_virtual_disk")) == 0) {
                        if (std::stoi(info)) {
                            this->enable_shared_virtual_disk = true;
                        }
                    } else
                    if (tag.compare(std::string("SHARE_DIR")) == 0) {
                        this->SHARE_DIR                 = info;
                        read_id = -1;
                    }
                }
            }

            ifichier.close();

//             this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
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
    }

    bool is_no_win_data() {
        return this->windowsData.no_data;
    }

    void writeWindowsConf() {
        this->windowsData.write();
    }

    void deleteCurrentProtile() {
        std::ifstream ifichier(this->USER_CONF_DIR, std::ios::in);
        if(ifichier) {

            std::string new_file_content;
            int ligne_to_jump = 0;

            std::string ligne;
            const std::string delimiter = " ";

            std::getline(ifichier, ligne);

            while(std::getline(ifichier, ligne)) {
                if (ligne_to_jump == 0) {
                    int pos = ligne.find(delimiter);
                    std::string tag  = ligne.substr(0, pos);
                    std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

                    if (tag == std::string("id")) {
                    }

                    if (tag == std::string("id") && std::stoi(info) == this->current_user_profil) {
                        ligne_to_jump = 18;
                    } else {
                        new_file_content += ligne + "\n";
                    }
                } else {
                    ligne_to_jump--;
                }
            }

            ifichier.close();

            std::ofstream ofichier(this->USER_CONF_DIR, std::ios::in | std::ios::trunc);
            ofichier << "current_user_profil_id 0" << "\n";
            ofichier << new_file_content << std::endl;
            ofichier.close();
        }
    }


    void setDefaultConfig() {
        //this->current_user_profil = 0;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = 0;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
//         this->mod_bpp = 24;
//         this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
        this->info.width  = 800;
        this->info.height = 600;
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;
        this->is_spanning = false;
        this->is_recording = false;
        this->modRDPParamsData.enable_tls = true;
        this->modRDPParamsData.enable_nla = true;
//         this->delta_time = 40;
        this->enable_shared_clipboard = true;
        this->enable_shared_virtual_disk = true;
        this->SHARE_DIR = std::string("/home");
        //this->info.encryptionLevel = 1;
    }

    void writeClientInfo() {
        std::fstream ofichier(this->USER_CONF_DIR);
        if(ofichier) {

            ofichier << "current_user_profil_id " << this->current_user_profil << "\n";

            std::string ligne;
            const std::string delimiter = " ";

            bool new_profil = true;
            int read_id = -1;
            auto pos(ligne.find(delimiter));
            std::string tag  = ligne.substr(0, pos);
            std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

            while(std::getline(ofichier, ligne)) {
                pos = ligne.find(delimiter);
                tag  = ligne.substr(0, pos);
                info = ligne.substr(pos + delimiter.length(), ligne.length());

                if (tag == std::string("id")) {
                    read_id = std::stoi(info);
                    if (read_id == this->current_user_profil) {
                        new_profil = false;
                        break;
                    }
                }
            }

            if (new_profil) {
                ofichier.close();
                std::ofstream new_ofile(this->USER_CONF_DIR, std::ios::app | std::ios::out);
                new_ofile << "\nid "     << this->userProfils[this->current_user_profil].id   << "\n";
                new_ofile << "name "   << this->userProfils[this->current_user_profil].name << "\n";
                new_ofile << "keylayout "             << this->info.keylayout               << "\n";
                new_ofile << "console_session "       << this->info.console_session         << "\n";
                new_ofile << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                new_ofile << "bpp "                   << this->info.bpp                     << "\n";
                new_ofile << "width "                 << this->info.width                   << "\n";
                new_ofile << "height "                << this->info.height                  << "\n";
                new_ofile << "rdp5_performanceflags " << this->info.rdp5_performanceflags   << "\n";
                new_ofile << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                new_ofile << "span "                  << this->is_spanning                  << "\n";
                new_ofile << "record "                << this->is_recording                 << "\n";
                new_ofile << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                new_ofile << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                new_ofile << "sound "                 << this->modRDPParamsData.enable_sound << "\n";
//                 new_ofile << "delta_time "            << this->delta_time << "\n";
                new_ofile << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                new_ofile << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                new_ofile << "SHARE_DIR "                              << this->SHARE_DIR << std::endl;

                new_ofile.close();

            } else {
                ofichier.seekp(ofichier.tellg());
                ofichier << "name "   << this->userProfils[this->current_user_profil].name << "\n";
                ofichier << "keylayout "             << this->info.keylayout               << "\n";
                ofichier << "console_session "       << this->info.console_session         << "\n";
                ofichier << "brush_cache_code "      << this->info.brush_cache_code        << "\n";
                ofichier << "bpp "                   << this->info.bpp                       << "\n";
                ofichier << "width "                 << this->info.width                   << "\n";
                ofichier << "height "                << this->info.height                  << "\n";
                ofichier << "rdp5_performanceflags " << this->info.rdp5_performanceflags   << "\n";
                ofichier << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                ofichier << "span "                  << this->is_spanning                  << "\n";
                ofichier << "record "                << this->is_recording                 << "\n";
                ofichier << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                ofichier << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                ofichier << "sound "                 << this->modRDPParamsData.enable_sound << "\n";
//                 ofichier << "delta_time "            << this->delta_time << "\n";
                ofichier << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                ofichier << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                ofichier << "SHARE_DIR "                              << this->SHARE_DIR << std::endl;

                ofichier.close();
            }
        }
    }


    virtual void send_clipboard_format() = 0;

    void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                        , std::size_t , std::size_t , int ) override {}

    // CONTROLLER
    virtual void connect() = 0;
//     virtual void setMainScreenOnTopRelease() = 0;
    virtual void disconnect(std::string const & txt) = 0;
//     virtual void dropScreen() = 0;
//     virtual bool is_no_win_data() = 0;
//     virtual void writeWindowsConf() = 0;
    virtual void replay(std::string const & movie_dir, std::string const & movie_path) = 0;
    virtual bool load_replay_mod(std::string const & movie_dir, std::string const & movie_name, timeval begin_read, timeval end_read) = 0;
    virtual void delete_replay_mod() = 0;
    virtual void callback() = 0;

    virtual void update_keylayout() = 0;

    bool can_be_start_capture() override { return true; }

    virtual void options() {
        LOG(LOG_WARNING, "No options window implemented yet. Virtual function \"void options()\" must be override.");
    }

//     virtual mod_api * init_mod() = 0;

    // CONTROLLER
//     virtual void connexionReleased() = 0;
    virtual void closeFromScreen() = 0;
    virtual void disconnexionReleased() = 0;

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

//     void virtual keyPressEvent(const int key, const char text) = 0;
//
//     void virtual keyReleaseEvent(const int key, const char text) = 0;

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

};



class ClientIOAPI {

public:
    ClientRedemptionIOAPI * client;

    void set_client(ClientRedemptionIOAPI * client) {
        this->client = client;
    }
};


class ClientIOClipboardAPI : public ClientIOAPI {

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
    virtual void setClipboard_text(std::string & str) = 0;
    virtual void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) = 0;
    virtual void setClipboard_files(std::string & name) = 0;

    virtual void write_clipboard_temp_file(std::string fileName, const uint8_t * data, size_t data_len) = 0;


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

        // image data
    virtual int get_image_buffer_width() = 0;
    virtual int get_image_buffer_height() = 0;
    virtual uint8_t * get_image_buffer_data() = 0;
    virtual int get_image_buffer_depth() = 0;

        // files data
    virtual std::string get_file_item_name(int index) = 0;
    virtual int get_file_item_size(int index) = 0;
    virtual char * get_file_item_data(int index) = 0;

    int get_citems_number() {
        return this->_cItems;
    }

//     virtual ~ClientIOClipboardAPI();

};



class ClientIODiskAPI : public ClientIOAPI {
//     ClientRedemptionIOAPI * client;
//
//     void set_client(ClientRedemptionIOAPI * client) {
//         this->client = client;
//     }
};



class ClientOutputSoundAPI : public ClientIODiskAPI {

public:
    uint32_t n_sample_per_sec = 0;
    uint16_t bit_per_sample = 0;
    uint16_t n_channels = 0;
    uint16_t n_block_align = 0;
    uint32_t bit_per_sec = 0;

    virtual void init(size_t raw_total_size) = 0;
    virtual void setData(const uint8_t * data, size_t size) = 0;
    virtual void play() = 0;

    virtual ~ClientOutputSoundAPI() = default;

};



class ClientInputSocketAPI : public ClientIODiskAPI {

public:
    mod_api * _callback = nullptr;

//     void set_mod(mod_api * mod) {
//         this->_callback = mod;
//     }

    virtual bool start_to_listen(int client_sck, mod_api * mod) = 0;
    virtual void disconnect() = 0;

    virtual ~ClientInputSocketAPI() = default;
};



class ClientInputMouseKeyboardAPI : public ClientIODiskAPI {


public:

    ClientInputMouseKeyboardAPI() = default;

    virtual ~ClientInputMouseKeyboardAPI() = default;


    virtual ClientRedemptionIOAPI * get_client() {
        return this->client;
    }

    virtual void update_keylayout() = 0;

    virtual void init_form() = 0;

    virtual void pre_load_movie() = 0;




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

    void virtual keyPressEvent(const int key, const char text)  = 0;
//         this->client->keyPressEvent(key, text);
//     }
//
    void virtual keyReleaseEvent(const int key, const char text)  = 0;
//         this->client->keyReleaseEvent(key, text);
//     }

    void virtual refreshPressed() {
        this->client->refreshPressed();
    }

    virtual void open_options() = 0;

};



class ClientOutputGraphicAPI {

public:
    ClientRedemptionIOAPI * drawn_client;

    const int screen_max_width;
    const int screen_max_height;

    ClientOutputGraphicAPI(int max_width, int max_height)
      : screen_max_width(max_width)
      , screen_max_height(max_height) {
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

    virtual void create_screen(std::string const & movie_dir, std::string const & movie_path) = 0;

    virtual void closeFromScreen() = 0;

    virtual void set_screen_size(int x, int y) = 0;




    // remote app

    virtual void create_remote_app_screen(uint32_t id, int w, int h, int x, int y) = 0;

    virtual void move_screen(uint32_t id, int x, int y) = 0;

    virtual void set_screen_size(uint32_t id, int x, int y) = 0;

    virtual void set_pixmap_shift(uint32_t id, int x, int y) = 0;

    virtual int get_visible_width(uint32_t id) = 0;

    virtual int get_visible_height(uint32_t id) = 0;

    virtual int get_mem_width(uint32_t id) = 0;

    virtual int get_mem_height(uint32_t id) = 0;

    virtual void set_mem_size(uint32_t id, int w, int h) = 0;

    virtual void show_screen(uint32_t id) = 0;

    virtual void dropScreen(uint32_t id) = 0;

    virtual void clear_remote_app_screen() = 0;




    virtual FrontAPI::ResizeResult server_resize(int width, int height, int bpp) = 0;

    virtual void set_pointer(Pointer      const &) {}
//     virtual void set_palette(BGRPalette   const &) {}

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
