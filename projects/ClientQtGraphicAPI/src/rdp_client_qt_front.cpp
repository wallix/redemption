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
   Copyright (C) Wallix 2010-2016
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include "utils/log.hpp"

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "configs/autogen/enums.hpp"

#include "rdp_client_qt_widget.hpp"

#include "mod/rdp/rdp.hpp"
#include "mod/vnc/vnc.hpp"

#include "client_channel_managers/client_channel_RDPSND_manager.hpp"
#include "client_channel_managers/client_channel_CLIPRDR_manager.hpp"
#include "client_channel_managers/client_channel_RDPDR_manager.hpp"
#include "client_channel_managers/client_channel_remoteapp_manager.hpp"


#include "qt_input_output_api/qt_output_sound.hpp"
#include "qt_input_output_api/qt_input_output_clipboard.hpp"

#pragma GCC diagnostic pop






class RDPClientQtFront : public Front_RDP_Qt_API
{

    private:
    class ClipboardServerChannelDataSender : public VirtualChannelDataSender
    {
    public:
        mod_api        * _callback;

        ClipboardServerChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            //std::cout << "operator()  server " << (int)flags  << std::endl;
            InStream chunk(chunk_data, chunk_data_length);
            this->_callback->send_to_mod_channel(channel_names::cliprdr, chunk, total_length, flags);
        }
    };

    class ClipboardClientChannelDataSender : public VirtualChannelDataSender
    {
    public:
        FrontAPI            * _front;
        CHANNELS::ChannelDef  _channel;

        ClipboardClientChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            //std::cout << "operator()  client " << (int)flags  << std::endl;

            this->_front->send_to_channel(this->_channel, chunk_data, total_length, chunk_data_length, flags);
        }
    };


public:
    ClipboardServerChannelDataSender _to_server_sender;
    ClipboardClientChannelDataSender _to_client_sender;

    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   = 1
      , PWD_GOTTEN    = 2
      , IP_GOTTEN     = 4
      , PORT_GOTTEN   = 8
    };


    enum : int {
        CHANID_CLIPDRD = 1601,
        CHANID_RDPDR   = 1602,
        CHANID_WABDIAG = 1603,
        CHANID_RDPSND  = 1604,
        CHANID_RAIL    = 1605
    };


    struct MouseData {
        QImage cursor_image;
        uint16_t x = 0;
        uint16_t y = 0;
    } _mouse_data;


    // io API
    ClientIOClipboardAPI * clientIOClipboardAPI;
    ClientOutputSoundAPI * clientOutputSoundAPI;



    //  Channel managers
    ClientChannelRDPSNDManager    clientChannelRDPSNDManager;
    ClientChannelCLIPRDRManager   clientChannelCLIPRDRManager;
    ClientChannelRDPDRManager     clientChannelRDPDRManager;
    ClientChannelRemoteAppManager clientChannelRemoteAppManager;


    UdevRandom           gen;
    std::array<uint8_t, 28> server_auto_reconnect_packet_ref;

    Inifile ini;

    ClientExecute client_execute;

    std::unique_ptr<mod_api> unique_mod;

    // VNC
    bool vnc;
    bool is_apple;
    Theme      theme;
    WindowListCaps windowListCaps;
    ClientExecute exe;


    void options() override {
        new DialogOptions_Qt(this, this->form);
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

    void setClientInfo() override {

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
                    } else
                    if (tag.compare(std::string("fps")) == 0) {
                        this->fps = std::stoi(info);
                    } else
                    if (tag.compare(std::string("monitorCount")) == 0) {
                        this->info.cs_monitor.monitorCount = std::stoi(info);
                        this->_monitorCount                 = std::stoi(info);
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
                    } else
                    if (tag.compare(std::string("delta_time")) == 0) {
                        if (std::stoi(info)) {
                            this->delta_time = std::stoi(info);
                        }
                    } else
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

            this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
        }

        this->qtRDPKeymap.clearCustomKeyCode();
        this->keyCustomDefinitions.clear();

        std::ifstream iFileKeyData(this->MAIN_DIR + std::string(KEY_SETTING_PATH), std::ios::in);
        if(iFileKeyData) {

            std::string ligne;
            std::string delimiter = " ";

            while(getline(iFileKeyData, ligne)) {

                int pos(ligne.find(delimiter));

                if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {

                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int qtKeyID  = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int scanCode = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int ASCII8   = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int extended = std::stoi(ligne.substr(0, pos));

                    this->qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);
                    this->keyCustomDefinitions.push_back({qtKeyID, scanCode, ASCII8, extended});
                }
            }

            iFileKeyData.close();
        }
    }

    void writeClientInfo() override {
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
                new_ofile << "delta_time "            << this->delta_time << "\n";
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
                ofichier << "delta_time "            << this->delta_time << "\n";
                ofichier << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                ofichier << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                ofichier << "SHARE_DIR "                              << this->SHARE_DIR << std::endl;

                ofichier.close();
            }
        }
    }

    void deleteCurrentProtile() override {
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

    void setDefaultConfig() override {
        //this->current_user_profil = 0;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = 0;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
        this->mod_bpp = 24;
        this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
        this->info.width  = 800;
        this->info.height = 600;
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;
        this->is_spanning = false;
        this->is_recording = false;
        this->modRDPParamsData.enable_tls = true;
        this->modRDPParamsData.enable_nla = true;
        this->delta_time = 40;
        this->enable_shared_clipboard = true;
        this->enable_shared_virtual_disk = true;
        this->SHARE_DIR = std::string("/home");
        //this->info.encryptionLevel = 1;
    }

    void setScreenDimension() {
        if (!this->is_spanning) {
            this->_screen_dimensions[0].cx = this->info.width;
            this->_screen_dimensions[0].cy = this->info.height;

        } else {

            QDesktopWidget* desktop = QApplication::desktop();
            int screen_count(desktop->screenCount());
            if (this->_monitorCount > screen_count) {
                this->_monitorCount = screen_count;
            }
            this->info.width  = 0;
            this->info.height = 0;
            this->info.cs_monitor.monitorCount = this->_monitorCount;

            for (int i = 0; i < this->_monitorCount; i++) {
                const QRect rect = desktop->screenGeometry(i);
                this->_screen_dimensions[i].x   = this->info.width;
                this->info.cs_monitor.monitorDefArray[i].left   = this->info.width;
                this->info.width  += rect.width();

                if (this->info.height < rect.height()) {
                    this->info.height = rect.height();
                }
                this->info.cs_monitor.monitorDefArray[i].top    = rect.top();
                this->info.cs_monitor.monitorDefArray[i].right  = this->info.width + rect.width() - 1;
                this->info.cs_monitor.monitorDefArray[i].bottom = rect.height() - 1 - 3*BUTTON_HEIGHT;

                this->info.cs_monitor.monitorDefArray[i].flags  = 0;

                this->_screen_dimensions[i].y   = 0;
                this->_screen_dimensions[i].cx  = rect.width();
                this->_screen_dimensions[i].cy  = rect.height() - 3*BUTTON_HEIGHT;
            }
            this->info.cs_monitor.monitorDefArray[0].flags  = GCC::UserData::CSMonitor::TS_MONITOR_PRIMARY;
            this->info.height -= 3*BUTTON_HEIGHT;
        }
    }


    mod_api * init_mod() override {


        ModRDPParams mod_rdp_params( this->user_name.c_str()
                                   , this->user_password.c_str()
                                   , this->target_IP.c_str()
                                   , this->local_IP.c_str()
                                   , 2
                                   , ini.get<cfg::font>()
                                   , ini.get<cfg::theme>()
                                   , this->server_auto_reconnect_packet_ref
                                   , this->close_box_extra_message_ref
                                   , this->verbose
                                   //, RDPVerbose::security | RDPVerbose::cache_persister | RDPVerbose::capabilities  | RDPVerbose::channels | RDPVerbose::connection
                                   //, RDPVerbose::basic_trace | RDPVerbose::connection
                                   );

        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = this->modRDPParamsData.enable_tls;
        mod_rdp_params.enable_nla                      = this->modRDPParamsData.enable_nla;
        mod_rdp_params.enable_fastpath                 = true;
        mod_rdp_params.enable_mem3blt                  = true;
        mod_rdp_params.enable_new_pointer              = true;
        mod_rdp_params.enable_glyph_cache              = true;
        mod_rdp_params.enable_ninegrid_bitmap          = true;
        std::string allow_channels = "*";
        mod_rdp_params.allow_channels                  = &allow_channels;
        mod_rdp_params.deny_channels = nullptr;
        mod_rdp_params.enable_rdpdr_data_analysis = false;



        if (this->remoteapp) {
            this->client_execute.enable_remote_program(this->remoteapp);
            mod_rdp_params.remote_program = this->remoteapp;
            mod_rdp_params.client_execute = &(this->client_execute);
            mod_rdp_params.remote_program_enhanced = INFO_HIDEF_RAIL_SUPPORTED;
            mod_rdp_params.use_client_provided_remoteapp = this->ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();
            mod_rdp_params.use_session_probe_to_launch_remote_program = this->ini.get<cfg::context::use_session_probe_to_launch_remote_program>();

            QDesktopWidget* desktop = QApplication::desktop();
            this->info.width = desktop->width();
            this->info.height = desktop->height();
        }


        try {
            this->mod = nullptr;

            if (this->vnc) {
                 this->unique_mod.reset(new mod_vnc( *(this->socket)
                                                    , this->user_name.c_str()
                                                    , this->user_password.c_str()
                                                    , *(this)
                                                    , this->info.width
                                                    , this->info.height
                                                    , this->ini.get<cfg::font>()
                                                    , nullptr
                                                    , nullptr
                                                    , this->theme
                                                    , this->info.keylayout
                                                    , 0
                                                    , true
                                                    , true
                                                    , "0,1,-239"
                                                    , false
                                                    , true
                                                    , mod_vnc::ClipboardEncodingType::UTF8
                                                    , VncBogusClipboardInfiniteLoop::delayed
                                                    , this->reportMessage
                                                    , this->is_apple
                                                    , &(this->exe)
                                                    , to_verbose_flags(0xfffffffd)
                                                   )
                                        );
            } else {

                this->unique_mod.reset(new mod_rdp( *(this->socket)
                                            , *(this)
                                            , this->info
                                            , ini.get_ref<cfg::mod_rdp::redir_info>()
                                            , this->gen
                                            , this->timeSystem
                                            , mod_rdp_params
                                            , this->authentifier
                                            , this->reportMessage
                                            , this->ini
                                            ));

                //this->unique_mod.get()->configure_proxy_managed_drives_client(this->SHARE_DIR.c_str());

                if (this->remoteapp) {

                    std::string target_info = this->ini.get<cfg::context::target_str>();
                    target_info += ":";
                    target_info += this->ini.get<cfg::globals::primary_user_id>();

                    this->client_execute.set_target_info(target_info.c_str());
                }
            }
            this->mod = this->unique_mod.get();

            this->mod->invoke_asynchronous_graphic_task(mod_api::AsynchronousGraphicTask::none);


        } catch (const Error &) {
            return nullptr;
        }

        return this->mod;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    RDPClientQtFront(char* argv[], int argc, RDPVerbose verbose,
                    ClientOutputSoundAPI * clientOutputSoundAPI,
                    ClientIOClipboardAPI * clientIOClipboardAPI)
        : Front_RDP_Qt_API(verbose)
        , clientIOClipboardAPI(new QtInputOutputClipboard(this, this->form))
        , clientOutputSoundAPI(new QtOutputSound(this->form))
        , clientChannelRDPSNDManager(this->verbose, this, this->clientOutputSoundAPI)
        , clientChannelCLIPRDRManager(this->verbose, this, this->clientIOClipboardAPI)
        , clientChannelRDPDRManager(this->verbose, this)
        , clientChannelRemoteAppManager(this->verbose, this)
        , client_execute(*(this), this->info.window_list_caps, false)
        , vnc(false)
        , is_apple(true)
        , exe(*(this),  this->windowListCaps,  false)
    {

        this->setDefaultConfig();
        this->setUserProfil();
        this->setClientInfo();

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
                this->qtRDPKeymap._verbose = 1;
            } else if (word ==  "--rail") {
                this->verbose = RDPVerbose::rail | this->verbose;
            } else if (word ==  "--rail_dump") {
                this->verbose = RDPVerbose::rail_dump | this->verbose;
            } else if (word ==  "--vnc") {
                this->vnc = true;
            }
        }

        this->clientChannelRDPSNDManager.verbose = this->verbose;
        this->clientChannelCLIPRDRManager.verbose = this->verbose;
        this->clientChannelRDPDRManager.verbose = this->verbose;
        this->clientChannelRemoteAppManager.verbose = this->verbose;


        this->client_execute.set_verbose(bool( (RDPVerbose::rail & this->verbose) | (RDPVerbose::rail_dump & this->verbose) ));



        if (commandIsValid == COMMAND_VALID) {
            this->connect();

        } else {
            std::cout << "Argument(s) required to connect: ";
            if (!(commandIsValid & NAME_GOTTEN)) {
                std::cout << "-n [user_name] ";
            }
            if (!(commandIsValid & PWD_GOTTEN)) {
                std::cout << "-w [password] ";
            }
            if (!(commandIsValid & IP_GOTTEN)) {
                std::cout << "-i [ip_server] ";
            }
            if (!(commandIsValid & PORT_GOTTEN)) {
                std::cout << "-p [port] ";
            }
            std::cout << std::endl;

            this->disconnect("");
        }
    }

    ~RDPClientQtFront() {}



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void connect() override {

        this->clientChannelRemoteAppManager.clear();

        this->setClientInfo();

        this->setScreenDimension();


        this->cl.clear_channels();

        if (!(this->vnc)) {

            if (this->remoteapp) {
                this->info.remote_program |= INFO_RAIL;
                this->info.remote_program_enhanced |= INFO_HIDEF_RAIL_SUPPORTED;
                this->info.rail_caps.RailSupportLevel =   TS_RAIL_LEVEL_SUPPORTED
    //                                                     | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED
                                                        | TS_RAIL_LEVEL_SHELL_INTEGRATION_SUPPORTED
                                                        //| TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED
                                                        | TS_RAIL_LEVEL_SERVER_TO_CLIENT_IME_SYNC_SUPPORTED
                                                        | TS_RAIL_LEVEL_HIDE_MINIMIZED_APPS_SUPPORTED
                                                        | TS_RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED
                                                        | TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED;

                this->info.window_list_caps.WndSupportLevel = TS_WINDOW_LEVEL_SUPPORTED;
                this->info.window_list_caps.NumIconCaches = 3;  // 3;
                // 12;
                this->info.window_list_caps.NumIconCacheEntries = 12;

                CHANNELS::ChannelDef channel_rail { channel_names::rail
                                            , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                            , CHANID_RAIL
                                            };
                this->cl.push_back(channel_rail);

            } else {

                if (this->enable_shared_virtual_disk) {
                    CHANNELS::ChannelDef channel_rdpdr{ channel_names::rdpdr
                                                    , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                        GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                                    , CHANID_RDPDR
                                                    };
                    this->cl.push_back(channel_rdpdr);
                }
            }

            if (this->enable_shared_clipboard) {
                CHANNELS::ChannelDef channel_cliprdr { channel_names::cliprdr
                                                    , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                    GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                    GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                                    , CHANID_CLIPDRD
                                                    };
                this->_to_client_sender._channel = channel_cliprdr;
                this->cl.push_back(channel_cliprdr);
            }

    //         CHANNELS::ChannelDef channel_WabDiag { channel_names::wabdiag
    //                                              , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
    //                                                GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
    //                                              , CHANID_WABDIAG
    //                                              };
    //         this->cl.push_back(channel_WabDiag);

            if (modRDPParamsData.enable_sound) {
                CHANNELS::ChannelDef channel_audio_output{ channel_names::rdpsnd
                                                        , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                        GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                        GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                                        , CHANID_RDPSND
                                                        };
                this->cl.push_back(channel_audio_output);
            }

        }

        return FrontQtRDPGraphicAPI::connect();
    }




    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //      CHANNELS FUNCTIONS
    //--------------------------------

    void clipboard_callback() override {
        this->clientChannelCLIPRDRManager.send_FormatListPDU();
    }

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) override {

        const CHANNELS::ChannelDef * mod_channel = this->cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }

        InStream chunk(data, chunk_size);

        switch (channel.chanid) {

            //this->process_server_clipboard_PDU(chunk, flags);
            case CHANID_CLIPDRD: this->clientChannelCLIPRDRManager.receive(chunk, flags);
                break;

            //this->process_server_rdpdr_PDU(chunk, flags);
            case CHANID_RDPDR: this->clientChannelRDPDRManager.receive(chunk);
                break;

            //this->process_server_rdpsnd_PDU(chunk, flags);
            case CHANID_RDPSND: this->clientChannelRDPSNDManager.receive(chunk);
                break;

            //this->process_server_rail_PDU(chunk, flags);
            case CHANID_RAIL:    this->clientChannelRemoteAppManager.receive(chunk);
                break;

            case CHANID_WABDIAG:
            {
                int len = chunk.in_uint32_le();
                std::string msg(reinterpret_cast<char const *>(chunk.get_current()), len);

                if        (msg == std::string("ConfirmationPixelColor=White")) {
                    this->wab_diag_question = true;
                    this->answer_question(0xffffffff);
                    this->asked_color = 0xffffffff;
                } else if (msg == std::string("ConfirmationPixelColor=Black")) {
                    this->wab_diag_question = true;
                    this->answer_question(0xff000000);
                    this->asked_color = 0xff000000;
                } else {
                    LOG(LOG_INFO, "SERVER >> wabdiag %s", msg.c_str());
                }
            }
                break;

            default: LOG(LOG_WARNING, " send_to_channel unknow channel id: %d", channel.chanid);
                break;
        }
    }


    void draw(const RDP::RAIL::ActivelyMonitoredDesktop  & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::ActivelyMonitoredDesktop");
        //cmd.log(LOG_INFO);

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::NewOrExistingWindow");

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::DeletedWindow            & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::DeletedWindow");
        //cmd.log(LOG_INFO);
        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::WindowIcon            & ) {
        LOG(LOG_INFO, "RDP::RAIL::WindowIcon");
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::CachedIcon            & ) {
        LOG(LOG_INFO, "RDP::RAIL::CachedIcon");
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons            & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::NewOrExistingNotificationIcons");
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons            & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::DeletedNotificationIcons");
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) {
        LOG(LOG_INFO, "RDP::RAIL::NonMonitoredDesktop");
        cmd.log(LOG_INFO);
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void disconnect(std::string const & error) override {

//         if( this->remoteapp && this->rail_mod.get()) {
//             time_t  timev;
//             time(&timev);
//             this->rail_mod.get()->disconnect(timev);
//             mod_api * rail_mod_ptr = this->rail_mod.release();
//             delete (rail_mod_ptr);
//             this->rail_mod.reset(nullptr);
//             this->mod = this->rdp_mod.get();
//             this->mod_qt->_callback = this->rdp_mod.get();
//         }

        FrontQtRDPGraphicAPI::disconnect(error);
    }

    void callback() override {
        if (this->_recv_disconnect_ultimatum) {
            this->dropScreen();
            std::string labelErrorMsg("<font color='Red'>Disconnected by server</font>");
            this->disconnect(labelErrorMsg);
//             this->cache = nullptr;
//             this->trans_cache = nullptr;
            this->capture = nullptr;
            this->graph_capture = nullptr;
            this->_recv_disconnect_ultimatum = false;
        }

        FrontQtRDPGraphicAPI::callback();
    }

};


///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    //" -name QA\\administrateur -pwd '' -ip 10.10.46.88 -p 3389";

    // sudo python ./sesman/sesmanlink/WABRDPAuthentifier

    // sudo nano /etc/rdpproxy/rdpproxy.ini

    // /etc/rdpproxy/cert

    // bjam san -j4 rdpproxy

    // sudo bin/gcc-4.9.2/san/rdpproxy -nf

    // sudo bin/gcc-4.9.2/release/link-static/rdpproxy -nf

    //bjam -s qt=4 debug client_rdp_Qt4 && bin/gcc-4.9.2/debug/threading-multi/client_rdp_Qt4

    // sed '/usr\/include\/qt4\|threading-multi\/src\/Qt4\/\|in expansion of macro .*Q_OBJECT\|Wzero/,/\^/d' &&

    // ../../tools/c++-analyzer/bt  bin/gcc-4.9.2/debug/threading-multi/client_rdp_Qt4

    // ../packager/packager.py --version 1.0.0 --no-entry-changelog --build-package

    // sudo dpkg -i /home/qa/Desktop/redemption_0.9.740bjessie_amd64.deb


    QApplication app(argc, argv);

    // RDPVerbose::rdpdr_dump | RDPVerbose::cliprdr;
    //RDPVerbose::graphics | RDPVerbose::cliprdr | RDPVerbose::rdpdr;
    RDPVerbose verbose = to_verbose_flags(0);

    RDPClientQtFront front_qt(argv, argc, verbose, nullptr, nullptr);


    app.exec();

    // scp -P 22 -r cmoroldo@10.10.43.46:/home/cmoroldo/Bureau/redemption_test_charge/movie.wrm /home/qa/Desktop/movie_sample_data/


    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2
}
