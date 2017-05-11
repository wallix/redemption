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

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/hdreg.h>
#include <unordered_map>

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "configs/autogen/enums.hpp"
#include "capture/flv_params_from_ini.hpp"

#include "rdp_client_qt_widget.hpp"


#pragma GCC diagnostic pop




class RDPClientQtFront : public Front_RDP_Qt_API
{

public:
    enum : int {
        COMMAND_VALID = 15
      , NAME_GOTTEN   = 1
      , PWD_GOTTEN    = 2
      , IP_GOTTEN     = 4
      , PORT_GOTTEN   = 8
    };

    enum : int {
       PDU_HEADER_SIZE =    8
    };

    enum : int {
        PASTE_TEXT_CONTENT_SIZE = CHANNELS::CHANNEL_CHUNK_LENGTH - PDU_HEADER_SIZE
      , PASTE_PIC_CONTENT_SIZE  = CHANNELS::CHANNEL_CHUNK_LENGTH - RDPECLIP::METAFILE_HEADERS_SIZE - PDU_HEADER_SIZE
    };


    struct MouseData {
        QImage cursor_image;
        uint16_t x = 0;
        uint16_t y = 0;
    } _mouse_data;

    // Connexion socket members
    ClipBoard_Qt       * clipboard_qt;
    bool                 _monitorCountNegociated;
    UdevRandom           gen;
    std::array<uint8_t, 28> server_auto_reconnect_packet_ref;


    // Clipboard Channel Management members
    uint32_t             _requestedFormatId = 0;
    std::string          _requestedFormatName;
    bool                 _waiting_for_data;


    struct FileSystemData {

        struct DeviceData {
            char name[8] = {0};
            uint32_t ID = 0;
            uint32_t type = 0;
        };

        bool drives_created = false;
        bool fileSystemCapacity[5] = { false };
        size_t devicesCount = 1;
        DeviceData devices[1];

        uint32_t next_file_id = 0;

        uint32_t get_file_id() {
            this->next_file_id++;
            return this->next_file_id;
        }

        std::unordered_map<int, std::string> paths;

        int writeData_to_wait = 0;
        int file_to_write_id = 0;

        enum : uint32_t {
            NEW_FILE_ATTRIBUTES =  fscc::FILE_SUPPORTS_USN_JOURNAL | fscc::FILE_SUPPORTS_OPEN_BY_FILE_ID | fscc::FILE_SUPPORTS_EXTENDED_ATTRIBUTES | fscc::FILE_SUPPORTS_HARD_LINKS | fscc::FILE_SUPPORTS_TRANSACTIONS | fscc::FILE_NAMED_STREAMS | fscc::FILE_SUPPORTS_ENCRYPTION | fscc::FILE_SUPPORTS_OBJECT_IDS | fscc::FILE_SUPPORTS_REPARSE_POINTS | fscc::FILE_SUPPORTS_SPARSE_FILES | fscc::FILE_VOLUME_QUOTAS | fscc::FILE_FILE_COMPRESSION | fscc::FILE_PERSISTENT_ACLS | fscc::FILE_UNICODE_ON_DISK | fscc::FILE_CASE_PRESERVED_NAMES | fscc::FILE_CASE_SENSITIVE_SEARCH // = 0 x03e700ff
        };

        uint32_t current_dir_id = 0;
        std::vector<std::string> elem_in_path;

    } fileSystemData;






    virtual void options() override {
        new DialogOptions_Qt(this, this->form);
    }

    unsigned WindowsTickToUnixSeconds(long long windowsTicks) {
        return unsigned((windowsTicks / _WINDOWS_TICK) - _SEC_TO_UNIX_EPOCH);
    }

    long long UnixSecondsToWindowsTick(unsigned unixSeconds) {
        return ((unixSeconds + _SEC_TO_UNIX_EPOCH) * _WINDOWS_TICK);
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

    virtual void writeClientInfo() override {
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
                new_ofile << "bpp "                   << this->mod_bpp                      << "\n";
                new_ofile << "width "                 << this->info.width                   << "\n";
                new_ofile << "height "                << this->info.height                  << "\n";
                new_ofile << "rdp5_performanceflags " << this->info.rdp5_performanceflags   << "\n";
                new_ofile << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                new_ofile << "span "                  << this->is_spanning                  << "\n";
                new_ofile << "record "                << this->is_recording                 << "\n";
                new_ofile << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                new_ofile << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
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
                ofichier << "bpp "                   << this->mod_bpp                      << "\n";
                ofichier << "width "                 << this->info.width                   << "\n";
                ofichier << "height "                << this->info.height                  << "\n";
                ofichier << "rdp5_performanceflags " << this->info.rdp5_performanceflags   << "\n";
                ofichier << "monitorCount "          << this->info.cs_monitor.monitorCount << "\n";
                ofichier << "span "                  << this->is_spanning                  << "\n";
                ofichier << "record "                << this->is_recording                 << "\n";
                ofichier << "tls "                   << this->modRDPParamsData.enable_tls  << "\n";
                ofichier << "nla "                   << this->modRDPParamsData.enable_nla  << "\n";
                ofichier << "delta_time "            << this->delta_time << "\n";
                ofichier << "enable_shared_clipboard "    << this->enable_shared_clipboard    << "\n";
                ofichier << "enable_shared_virtual_disk " << this->enable_shared_virtual_disk << "\n";
                ofichier << "SHARE_DIR "                              << this->SHARE_DIR << std::endl;

                ofichier.close();
            }
        }
    }

    virtual void deleteCurrentProtile() {
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

    virtual void setDefaultConfig() {
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


    virtual mod_api * init_mod() override {

        Inifile ini;

        ModRDPParams mod_rdp_params( this->user_name.c_str()
                                   , this->user_password.c_str()
                                   , this->target_IP.c_str()
                                   , this->local_IP.c_str()
                                   , 2
                                   , ini.get<cfg::font>()
                                   , ini.get<cfg::theme>()
                                   , this->server_auto_reconnect_packet_ref
                                   , to_verbose_flags(0)
                                   //, RDPVerbose::basic_trace4 | RDPVerbose::basic_trace3 | RDPVerbose::basic_trace7 | RDPVerbose::basic_trace
                                   );

        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = this->modRDPParamsData.enable_tls;
        mod_rdp_params.enable_nla                      = this->modRDPParamsData.enable_nla;
        mod_rdp_params.enable_fastpath                 = false;
        mod_rdp_params.enable_mem3blt                  = true;
        mod_rdp_params.enable_new_pointer              = true;
        mod_rdp_params.server_redirection_support      = true;
        mod_rdp_params.enable_new_pointer              = true;
        mod_rdp_params.enable_glyph_cache              = true;
        std::string allow_channels = "*";
        mod_rdp_params.allow_channels                  = &allow_channels;
        //mod_rdp_params.verbose = to_verbose_flags(0);


        try {
            this->mod = new mod_rdp( *(this->socket)
                                , *(this)
                                , this->info
                                , ini.get_ref<cfg::mod_rdp::redir_info>()
                                , this->gen
                                , this->timeSystem
                                , mod_rdp_params
                                , this->authentifier
                                , this->reportMessage
                                );

        } catch (const Error &) {
            return nullptr;
        }

        return this->mod;
    }






    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    RDPClientQtFront(char* argv[], int argc, RDPVerbose verbose)
        : Front_RDP_Qt_API(verbose)
        , clipboard_qt(nullptr)
        , _monitorCountNegociated(false)
        , _waiting_for_data(false)
    {
        this->clipboard_qt = new ClipBoard_Qt(this, this->form);

        this->setDefaultConfig();
        this->setUserProfil();
        this->setClientInfo();

        uint8_t commandIsValid(0);

        // TODO QCommandLineParser / program_options
        for (int i = 0; i <  argc - 1; i++) {

            std::string word(argv[i]);
            std::string arg(argv[i+1]);

            if (       word == "-n") {
                this->user_name = arg;
                commandIsValid += NAME_GOTTEN;
            } else if (word == "-w") {
                this->user_password = arg;
                commandIsValid += PWD_GOTTEN;
            } else if (word == "-i") {
                this->target_IP = arg;
                commandIsValid += IP_GOTTEN;
            } else if (word == "-p") {
                std::string portStr(arg);
                this->port = std::stoi(portStr);
                commandIsValid += PORT_GOTTEN;
            }
        }

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
                std::cout << "-i [ip_serveur] ";
            }
            if (!(commandIsValid & PORT_GOTTEN)) {
                std::cout << "-p [port] ";
            }
            std::cout << std::endl;

            this->disconnect("");
        }
    }

    ~RDPClientQtFront() {
        this->empty_buffer();
        this->fileSystemData.paths.clear();
    }




    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    virtual void connect() override {

        this->setClientInfo();

        this->setScreenDimension();

        this->clipbrdFormatsList.index = 0;
        this->cl.clear_channels();

        if (this->enable_shared_clipboard) {

            std::string tmp(this->SHARE_DIR);
            int pos(tmp.find("/"));

            while (pos != -1) {
                tmp = tmp.substr(pos+1, tmp.length());
                pos = tmp.find("/");
            }

            size_t size(tmp.size());
            if (size > 7) {
                size = 7;
            }

            for (size_t i = 0; i < size; i++) {
                this->fileSystemData.devices[0].name[i] = tmp.data()[i];
            }

            this->fileSystemData.devices[0].ID = 1;
            this->fileSystemData.devices[0].type = rdpdr::RDPDR_DTYP_FILESYSTEM;

//             this->fileSystemData.devices[1].ID = 2;
//             this->fileSystemData.devices[1].type = rdpdr::RDPDR_DTYP_PRINT;
//             std::string name_printer("printer");
//             const char * char_name_printer = name_printer.c_str();
//
//             for (size_t i = 0; i < size; i++) {
//                 this->fileSystemData.devices[1].name[i] = char_name_printer[i];
//             }
//
//             this->fileSystemData.devicesCount++;

            CHANNELS::ChannelDef channel_cliprdr { channel_names::cliprdr
                                                 , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                   GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                   GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                                 , CHANNELS::CHANNEL_CHUNK_LENGTH+1
                                                 };
            this->_to_client_sender._channel = channel_cliprdr;
            this->cl.push_back(channel_cliprdr);

            this->clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS
                                               , this->clipbrdFormatsList.FILECONTENTS
                                               );
            this->clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW
                                               , this->clipbrdFormatsList.FILEGROUPDESCRIPTORW
                                               );
            this->clipbrdFormatsList.add_format( RDPECLIP::CF_UNICODETEXT
                                               , std::string("\0\0", 2)
                                               );
            this->clipbrdFormatsList.add_format( RDPECLIP::CF_TEXT
                                               , std::string("\0\0", 2)
                                               );
            this->clipbrdFormatsList.add_format( RDPECLIP::CF_METAFILEPICT
                                               , std::string("\0\0", 2)
                                               );
        }

        if (this->enable_shared_virtual_disk) {
            CHANNELS::ChannelDef channel_rdpdr{ channel_names::rdpdr
                                              , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                              , CHANNELS::CHANNEL_CHUNK_LENGTH+2
                                              };
            this->cl.push_back(channel_rdpdr);
        }

//         CHANNELS::ChannelDef channel_audio_output{ channel_names::rdpsnd
//                                                  , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
//                                                    GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
//                                                    GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
//                                                  , CHANNELS::CHANNEL_CHUNK_LENGTH+3
//                                                  };
//         this->cl.push_back(channel_audio_output);

        return FrontQtRDPGraphicAPI::connect();
    }




    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //      CHANNELS FUNCTIONS
    //--------------------------------

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) override {
//         if (bool(this->verbose & RDPVerbose::graphics)) {
//             LOG(LOG_INFO, "--------- FRONT ------------------------");
//             LOG(LOG_INFO, "send_to_channel");
//             LOG(LOG_INFO, "========================================\n");
//         }

        const CHANNELS::ChannelDef * mod_channel = this->cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }

        InStream chunk(data, chunk_size);

        InStream chunk_series = chunk.clone();

        if (!strcmp(channel.name, channel_names::cliprdr)) {
            //std::unique_ptr<AsynchronousTask> out_asynchronous_task;

            if (!chunk.in_check_rem(2  /*msgType(2)*/ )) {
                LOG(LOG_ERR,
                    "ClipboardVirtualChannel::process_client_message: "
                        "Truncated msgType, need=2 remains=%zu",
                    chunk.in_remain());
                throw Error(ERR_RDP_DATA_TRUNCATED);
            }

            const uint16_t server_message_type = chunk.in_uint16_le();

            if (!this->_waiting_for_data) {
                switch (server_message_type) {
                    case RDPECLIP::CB_CLIP_CAPS:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Clipboard Capabilities PDU");
                        }
                    break;

                    case RDPECLIP::CB_MONITOR_READY:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Monitor Ready PDU");
                        }

                        {
                            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1, RDPECLIP::GeneralCapabilitySet::size());
                            RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_USE_LONG_FORMAT_NAMES | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);
                            StaticOutStream<1024> out_stream;
                            clipboard_caps_pdu.emit(out_stream);
                            general_cap_set.emit(out_stream);

                            const uint32_t total_length = out_stream.get_offset();
                            InStream chunk(out_stream.get_data(), total_length);

                            this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk
                                                                , total_length
                                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
                                                                |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Clipboard Capabilities PDU");
                            }

                            this->_monitorCount = this->info.cs_monitor.monitorCount;

                            /*std::cout << "cs_monitor count negociated. MonitorCount=" << this->_monitorCount << std::endl;
                            std::cout << "width=" <<  this->info.width <<  " " << "height=" << this->info.height <<  std::endl;*/

                            //this->info.width  = (this->_width * this->_monitorCount);

                            /*if (!this->_monitorCountNegociated) {
                                for (int i = this->_monitorCount - 1; i >= 1; i--) {
                                    this->_screen[i] = new Screen_Qt(this, i);
                                    this->_screen[i]->show();
                                }
                                this->_screen[0]->activateWindow();
                                this->_monitorCountNegociated = true;

                            }*/
                            this->_monitorCountNegociated = true;
                        }
                        {
                            this->send_FormatListPDU(this->clipbrdFormatsList.IDs, this->clipbrdFormatsList.names, ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT);

                        }

                    break;

                    case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_WARNING, "SERVER >> CB Channel: Format List Response PDU FAILED");
                        } else {
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: Format List Response PDU");
                            }
                        }
                        }

                    break;

                    case RDPECLIP::CB_FORMAT_LIST:
                        {
                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: Format List PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: Format List PDU");
                                }

                                int formatAvailable = chunk.in_uint32_le();

                                bool isSharedFormat = false;

                                while (formatAvailable > 0) {
                                    uint32_t formatID = chunk.in_uint32_le();
                                    formatAvailable -=  4;

                                    uint16_t utf16_string[120];
                                    int k(0);
                                    bool isEndString = false;
                                    while (!isEndString) {
                                        u_int16_t bit(chunk.in_uint16_le());
                                        if (bit == 0) {
                                            isEndString = true;
                                        }
                                        utf16_string[k] = bit;
                                        k++;
                                        formatAvailable -=  2;
                                    }
                                    this->_requestedFormatName = std::string(reinterpret_cast<const char*>(utf16_string), k*2);

                                    for (int j = 0; j < ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT && !isSharedFormat; j++) {
                                        if (this->clipbrdFormatsList.IDs[j] == formatID) {
                                            this->_requestedFormatId = formatID;
                                            isSharedFormat = true;
                                            formatAvailable = 0;
                                        }
                                    }

                                    if (this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW && !isSharedFormat) {
                                        this->_requestedFormatId = formatID;
                                        isSharedFormat = true;
                                        formatAvailable = 0;
                                    }
                                }

                                RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
                                StaticOutStream<256> out_stream;
                                formatListResponsePDU.emit(out_stream);
                                InStream chunk(out_stream.get_data(), out_stream.get_offset());

                                this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                    , chunk
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                    );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format List Response PDU");
                                }


                                RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
                                StaticOutStream<32> out_stream_lock;
                                lockClipboardDataPDU.emit(out_stream_lock);
                                InStream chunk_lock(out_stream_lock.get_data(), out_stream_lock.get_offset());

                                this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                    , chunk_lock
                                                                    , out_stream_lock.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                    );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Lock Clipboard Data PDU");
                                }

                                RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(this->_requestedFormatId);
                                StaticOutStream<256> out_streamRequest;
                                formatDataRequestPDU.emit(out_streamRequest);
                                InStream chunkRequest(out_streamRequest.get_data(), out_streamRequest.get_offset());

                                this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                    , chunkRequest
                                                                    , out_streamRequest.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                    CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                    );
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Request PDU");
                                }
                            }
                        }
                    break;

                    case RDPECLIP::CB_LOCK_CLIPDATA:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Lock Clipboard Data PDU");
                        }
                    break;

                    case RDPECLIP::CB_UNLOCK_CLIPDATA:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Unlock Clipboard Data PDU");
                        }
                    break;

                    case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                        if(this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                            this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                        }

                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Response PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Response PDU");
                                }

                                this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList, this->clipboard_qt);
                            }
                        }
                    break;

                    case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                    {
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Request PDU FAILED");
                        } else {
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Request PDU");
                            }

                            chunk.in_skip_bytes(4);
                            int first_part_data_size(0);
                            uint32_t total_length(this->clipboard_qt->_cliboard_data_length + PDU_HEADER_SIZE);
                            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;

                            if (this->clipboard_qt->_bufferTypeID == chunk.in_uint32_le()) {

                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Response PDU");
                                }

                                switch(this->clipboard_qt->_bufferTypeID) {

                                    case RDPECLIP::CF_METAFILEPICT:
                                    {
                                        first_part_data_size = this->clipboard_qt->_cliboard_data_length;
                                        if (first_part_data_size > PASTE_PIC_CONTENT_SIZE) {
                                            first_part_data_size = PASTE_PIC_CONTENT_SIZE;
                                        }
                                        total_length += RDPECLIP::METAFILE_HEADERS_SIZE;
                                        RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr( this->clipboard_qt->_cliboard_data_length
                                                                                    , this->clipboard_qt->_bufferImage->width()
                                                                                    , this->clipboard_qt->_bufferImage->height()
                                                                                    , this->clipboard_qt->_bufferImage->depth()
                                                                                    , this->clipbrdFormatsList.ARBITRARY_SCALE
                                                                                    );
                                        fdr.emit(out_stream_first_part);

                                        this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                            , total_length
                                                                            , out_stream_first_part
                                                                            , first_part_data_size
                                                                            , this->clipboard_qt->_chunk.get()
                                                                            , this->clipboard_qt->_cliboard_data_length + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE
                                                                            , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                            );
                                    }
                                    break;

                                    case RDPECLIP::CF_TEXT:
                                    case RDPECLIP::CF_UNICODETEXT:
                                    {
                                        first_part_data_size = this->clipboard_qt->_cliboard_data_length;
                                        if (first_part_data_size > PASTE_TEXT_CONTENT_SIZE ) {
                                            first_part_data_size = PASTE_TEXT_CONTENT_SIZE;
                                        }

                                        RDPECLIP::FormatDataResponsePDU_Text fdr(this->clipboard_qt->_cliboard_data_length);

                                        fdr.emit(out_stream_first_part);

                                        this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                            , total_length
                                                                            , out_stream_first_part
                                                                            , first_part_data_size
                                                                            , this->clipboard_qt->_chunk.get()
                                                                            , this->clipboard_qt->_cliboard_data_length
                                                                            , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                            );
                                    }
                                    break;

                                    case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:
                                    {
                                        int data_sent(0);
                                        first_part_data_size = PDU_HEADER_SIZE + 4;
                                        total_length = (RDPECLIP::FileDescriptor::size() * this->clipboard_qt->_cItems) + 8 + PDU_HEADER_SIZE;
                                        int flag_first(CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                                        ClipBoard_Qt::CB_out_File * file = this->clipboard_qt->_items_list[0];
                                        RDPECLIP::FormatDataResponsePDU_FileList fdr(this->clipboard_qt->_cItems);
                                        fdr.emit(out_stream_first_part);

                                        RDPECLIP::FileDescriptor fdf( file->nameUTF8
                                                                    , file->size
                                                                    , fscc::FILE_ATTRIBUTE_ARCHIVE
                                                                    );
                                        fdf.emit(out_stream_first_part);

                                        if (this->clipboard_qt->_cItems == 1) {
                                            flag_first = flag_first | CHANNELS::CHANNEL_FLAG_LAST;
                                            out_stream_first_part.out_uint32_le(0);
                                            data_sent += 4;
                                        }
                                        InStream chunk_first_part( out_stream_first_part.get_data()
                                                                , out_stream_first_part.get_offset()
                                                                );

                                        this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                            , chunk_first_part
                                                                            , total_length
                                                                            , flag_first
                                                                            );
                                        data_sent += first_part_data_size + RDPECLIP::FileDescriptor::size();
                                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                            LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%d", data_sent, total_length);
                                        }

                                        for (int i = 1; i < this->clipboard_qt->_cItems; i++) {

                                            StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_next_part;
                                            file = this->clipboard_qt->_items_list[i];
                                            RDPECLIP::FileDescriptor fdn( file->nameUTF8
                                                                        , file->size
                                                                        , fscc::FILE_ATTRIBUTE_ARCHIVE);
                                            int flag_next(CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                                            fdn.emit(out_stream_next_part);

                                            if (i == this->clipboard_qt->_cItems - 1) {
                                                flag_next = flag_next | CHANNELS::CHANNEL_FLAG_LAST;
                                                out_stream_next_part.out_uint32_le(0);
                                                data_sent += 4;
                                            }

                                            InStream chunk_next_part( out_stream_next_part.get_data()
                                                                    , out_stream_next_part.get_offset()
                                                                    );

                                            this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                                , chunk_next_part
                                                                                , total_length
                                                                                , flag_next
                                                                                );

                                            data_sent += RDPECLIP::FileDescriptor::size();
                                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                                LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%d", data_sent, total_length);
                                            }
                                        }
                                    }
                                    break;

                                    default: LOG(LOG_WARNING, "SERVER >> CB Channel: unknow CB format ID %x", this->clipboard_qt->_bufferTypeID);
                                    break;
                                }
                            }
                        }
                    }
                    break;

                    case RDPECLIP::CB_FILECONTENTS_REQUEST:
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Resquest PDU FAIL");
                        } else {
                            if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Resquest PDU");
                            }

                            chunk.in_skip_bytes(4);                 // data_len
                            int streamID(chunk.in_uint32_le());
                            int lindex(chunk.in_uint32_le());

                            switch (chunk.in_uint32_le()) {         // flag

                                case RDPECLIP::FILECONTENTS_SIZE :
                                {
                                    StaticOutStream<32> out_stream;
                                    RDPECLIP::FileContentsResponse_Size fileSize( streamID
                                                                                , this->clipboard_qt->_items_list[lindex]->size);
                                    fileSize.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
                                    this->mod->send_to_mod_channel( channel_names::cliprdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                        CHANNELS::CHANNEL_FLAG_FIRST |  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                        );

                                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                        LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU SIZE");
                                    }
                                }
                                break;

                                case RDPECLIP::FILECONTENTS_RANGE :
                                {
                                    StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_first_part;
                                    RDPECLIP::FileContentsResponse_Range fileRange( streamID
                                                                                , this->clipboard_qt->_items_list[lindex]->size);
                                    this->clipboard_qt->_cliboard_data_length = this->clipboard_qt->_items_list[lindex]->size;
                                    int total_length(this->clipboard_qt->_items_list[lindex]->size + 12);
                                    int first_part_data_size(this->clipboard_qt->_items_list[lindex]->size);
                                    first_part_data_size = this->clipboard_qt->_items_list[lindex]->size;
                                    if (first_part_data_size > CHANNELS::CHANNEL_CHUNK_LENGTH - 12) {
                                        first_part_data_size = CHANNELS::CHANNEL_CHUNK_LENGTH - 12;
                                    }
                                    fileRange.emit(out_stream_first_part);

                                    this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                        , total_length
                                                                        , out_stream_first_part
                                                                        , first_part_data_size
                                                                        , reinterpret_cast<uint8_t *>(
                                                                            this->clipboard_qt->_items_list[lindex]->chunk)
                                                                        , this->clipboard_qt->_items_list[lindex]->size
                                                                        , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                        );

                                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                        LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU RANGE");
                                    }
                                }
                                break;
                            }
                        }
                    break;

                    case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                            if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                                LOG(LOG_WARNING, "SERVER >> CB Channel: File Contents Response PDU FAILED");
                            } else {
                                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                                    LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU");
                                }

                                if(this->_requestedFormatName == this->clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                                    this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS;
                                    this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList, this->clipboard_qt);
                                }
                            }
                        }
                    break;

                    default:
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "Process sever next part PDU data");
                        }
                        this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList, this->clipboard_qt);
                    break;
                }

            } else {
                if (bool(this->verbose & RDPVerbose::cliprdr)) {
                    LOG(LOG_INFO, "Process sever next part PDU data");
                }
                this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList, this->clipboard_qt);
            }



        } else if (!strcmp(channel.name, channel_names::rdpdr)) {

            if (this->fileSystemData.writeData_to_wait) {

                size_t length(chunk.in_remain());

                this->fileSystemData.writeData_to_wait -= length;

                std::string file_to_write = this->fileSystemData.paths.at(this->fileSystemData.file_to_write_id);

                std::ofstream oFile(file_to_write.c_str(), std::ios::out | std::ios::binary | std::ios::app);
                if (oFile.good()) {
                    oFile.write(reinterpret_cast<const char *>(chunk.get_current()), length);
                    oFile.close();
                }  else {
                    LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_write.c_str());
                }

                return;
            }


            uint16_t component = chunk.in_uint16_le();
            uint16_t packetId  = chunk.in_uint16_le();

            switch (component) {

                case rdpdr::Component::RDPDR_CTYP_CORE:

                    switch (packetId) {
                        case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                            {
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Announce Request");

                            uint16_t versionMajor = chunk.in_uint16_le();
                            uint16_t versionMinor = chunk.in_uint16_le();
                            uint32_t clientID = chunk.in_uint32_le();

                            StaticOutStream<32> stream;

                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
                            sharedHeader.emit(stream);

                            rdpdr::ClientAnnounceReply clientAnnounceReply( versionMajor
                                                                        , versionMinor
                                                                        , clientID);
                            clientAnnounceReply.emit(stream);

                            int total_length(stream.get_offset());
                            InStream chunk_to_send(stream.get_data(), stream.get_offset());

                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                , chunk_to_send
                                                                , total_length
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                );
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Announce Reply");
                            }

                            {
                            StaticOutStream<32> stream;

                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_CLIENT_NAME);
                            sharedHeader.emit(stream);
                            char username[LOGIN_NAME_MAX];
                            gethostname(username, LOGIN_NAME_MAX);
                            std::string str_username(username);

                            rdpdr::ClientNameRequest clientNameRequest(username, rdpdr::UNICODE_CHAR);
                            clientNameRequest.emit(stream);

                            int total_length(stream.get_offset());
                            InStream chunk_to_send(stream.get_data(), stream.get_offset());

                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                , chunk_to_send
                                                                , total_length
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                );
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Name Request");
                            }
                            break;

                        case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                            {
                            uint16_t capa  = chunk.in_uint16_le();
                            chunk.in_skip_bytes(2);
                            bool driveEnable = false;
                            for (int i = 0; i < capa; i++) {
                                uint16_t type = chunk.in_uint16_le();
                                uint16_t size = chunk.in_uint16_le() - 4;
                                chunk.in_skip_bytes(size);
                                this->fileSystemData.fileSystemCapacity[type] = true;
                                if (type == 0x4) {
                                    driveEnable = true;
                                }
                            }

                            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                if (driveEnable) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Core Capability Request - Drive Capability Enable");
                                    //this->show_in_stream(0, chunk_series, chunk_size);
                                } else {
                                    LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Core Capability Request - Drive Not Allowed");
                                    //this->show_in_stream(0, chunk_series, chunk_size);
                                }
                            }
                            }

                            break;

                        case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Client ID Confirm");
                            //this->show_in_stream(0, chunk_series, chunk_size);
                            {
                            StaticOutStream<1024> out_stream;
                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_CLIENT_CAPABILITY);
                            sharedHeader.emit(out_stream);

                            out_stream.out_uint16_le(5);    // 5 capabilities.
                            out_stream.out_clear_bytes(2);  // Padding(2)

                            rdpdr::GeneralCapabilitySet gcs();

                            // General capability set
                            out_stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
                            out_stream.out_uint16_le(36 + 8);
                                    /*rdpdr::GeneralCapabilitySet::size(
                                        general_capability_version) +
                                    8   // CapabilityType(2) + CapabilityLength(2) +
                                        //     Version(4)
                                );*/
                            out_stream.out_uint32_le(rdpdr::GENERAL_CAPABILITY_VERSION_02);

                            rdpdr::GeneralCapabilitySet general_capability_set(
                                    0x2,        // osType
                                    rdpdr::MINOR_VERSION_2,        // protocolMinorVersion -
                                    rdpdr::SUPPORT_ALL_REQUEST,     // ioCode1
                                    rdpdr::RDPDR_DEVICE_REMOVE_PDUS |           // extendedPDU -
                                        rdpdr::RDPDR_CLIENT_DISPLAY_NAME_PDU  |
                                        rdpdr::RDPDR_USER_LOGGEDON_PDU,
                                    rdpdr::ENABLE_ASYNCIO,        // extraFlags1
                                    0,                          // SpecialTypeDeviceCap
                                    rdpdr::GENERAL_CAPABILITY_VERSION_02
                                );

                            general_capability_set.emit(out_stream);

                            rdpdr::CapabilityHeader ch1(rdpdr::CAP_PRINTER_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch1.emit(out_stream);

                            rdpdr::CapabilityHeader ch2(rdpdr::CAP_PORT_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch2.emit(out_stream);

                            rdpdr::CapabilityHeader ch3(rdpdr::CAP_DRIVE_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch3.emit(out_stream);

                            rdpdr::CapabilityHeader ch4(rdpdr::CAP_SMARTCARD_TYPE, rdpdr::PRINT_CAPABILITY_VERSION_01);
                            ch4.emit(out_stream);

                            int total_length(out_stream.get_offset());
                            InStream chunk_to_send(out_stream.get_data(), total_length);

                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                , chunk_to_send
                                                                , total_length
                                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                );
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Core Capability Response");
                            }

                            {
                            StaticOutStream<128> out_stream;
                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);
                            sharedHeader.emit(out_stream);

                            rdpdr::ClientDeviceListAnnounceRequest cdlar(this->fileSystemData.devicesCount);
                            cdlar.emit(out_stream);

                            for (size_t i = 0; i < this->fileSystemData.devicesCount; i++) {

                                if (this->fileSystemData.devices[i].type == rdpdr::RDPDR_DTYP_PRINT) {

//                                     rdpdr::DeviceAnnounceHeaderPrinterSpecificData dahp(
//                                                     this->fileSystemData.devices[i].type
//                                                   , this->fileSystemData.devices[i].ID
//                                                   , this->fileSystemData.devices[i].name
//                                                   , rdpdr::RDPDR_PRINTER_ANNOUNCE_FLAG_ASCII |

//                                                   ,
//                                       );
//                                     dahp.emit(out_stream);

                                } else {

                                    rdpdr::DeviceAnnounceHeader dah( this->fileSystemData.devices[i].type
                                                                   , this->fileSystemData.devices[i].ID
                                                                   , this->fileSystemData.devices[i].name
                                                                   , nullptr, 0);
                                    dah.emit(out_stream);

                                }
                            }

                            int total_length(out_stream.get_offset());
                            InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                          , chunk_to_send
                                                          , total_length
                                                          , CHANNELS::CHANNEL_FLAG_LAST  |
                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                          );
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Device List Announce Request");
                            }
                            break;

                        case rdpdr::PAKID_CORE_DEVICE_REPLY:
                            {
                            rdpdr::ServerDeviceAnnounceResponse sdar;
                            sdar.receive(chunk);

                            if (sdar.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS) {
                                this->fileSystemData.drives_created = true;
                            } else {
                                this->fileSystemData.drives_created = false;
                                LOG(LOG_WARNING, "SERVER >> RDPDR Channel: Can't create virtual disk ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                            }
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Device Announce Response ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                            }
                            break;

                        case rdpdr::PAKID_CORE_USER_LOGGEDON:
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server User Logged On");
                            break;

                        case rdpdr::PAKID_CORE_DEVICE_IOREQUEST:
                            {
                            rdpdr::DeviceIORequest deviceIORequest;
                            deviceIORequest.receive(chunk);

                            StaticOutStream<1024> out_stream;
                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                            sharedHeader.emit(out_stream);

                            uint32_t id = deviceIORequest.FileId();

                            rdpdr::DeviceIOResponse deviceIOResponse( deviceIORequest.DeviceId()
                                                                    , deviceIORequest.CompletionId()
                                                                    , erref::NTSTATUS::STATUS_SUCCESS);

                            switch (deviceIORequest.MajorFunction()) {

                                case rdpdr::IRP_MJ_LOCK_CONTROL:
                                {
                                    deviceIOResponse.emit(out_stream);

                                    rdpdr::ClientDriveLockControlResponse cdlcr;
                                    cdlcr.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                }
                                    break;

                                case rdpdr::IRP_MJ_CREATE:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Create Request");
                                    {
                                    rdpdr::DeviceCreateRequest request;
                                    request.receive(chunk);

                                    std::string new_path(this->SHARE_DIR + request.Path());

                                    if (id == 0) {

                                        std::ifstream file(new_path.c_str());
                                        if (file.good()) {
                                            id = this->fileSystemData.get_file_id();
                                            this->fileSystemData.paths.emplace(id, new_path);
                                        } else {
                                            if (request.CreateDisposition() & smb2::FILE_CREATE) {

                                                id = this->fileSystemData.get_file_id();
                                                this->fileSystemData.paths.emplace(id, new_path);

                                                if (request.CreateOptions() & smb2::FILE_DIRECTORY_FILE) {
                                                    LOG(LOG_WARNING, "new directory: \"%s\"", new_path);
                                                    mkdir(new_path.c_str(), ACCESSPERMS);
                                                } else {
                                                    //LOG(LOG_WARNING, "new file: \"%s\"", new_path);
                                                    std::ofstream oFile(new_path, std::ios::out | std::ios::binary);
                                                    if (!oFile.good()) {
                                                        LOG(LOG_WARNING, "  Can't open create such file: \'%s\'.", new_path.c_str());
                                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                    } else {
                                                        oFile.close();
                                                    }
                                                }

                                            } else {
                                                //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", new_path.c_str());
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            }
                                        }
                                    }

                                    uint8_t Information(rdpdr::FILE_SUPERSEDED);
                                    if (request.CreateDisposition() & smb2::FILE_OPEN_IF) {
                                        Information = rdpdr::FILE_OPENED;
                                    }

                                    rdpdr::DeviceCreateResponse deviceCreateResponse( id
                                                                                    , Information);

                                    deviceIOResponse.emit(out_stream);
                                    deviceCreateResponse.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                        CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );

                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Create Response");
                                    }
                                    break;

                                case rdpdr::IRP_MJ_QUERY_INFORMATION:
                                    {
                                    rdpdr::ServerDriveQueryInformationRequest sdqir;
                                    sdqir.receive(chunk);

                                    switch (sdqir.FsInformationClass()) {

                                        case rdpdr::FileBasicInformation:
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Basic Query Information Request");
                                            {

                                            std::string file_to_request = this->fileSystemData.paths.at(id);

                                            std::ifstream file(file_to_request.c_str());
                                            if (!file.good()) {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", file_to_request.c_str());
                                            }

                                            deviceIOResponse.emit(out_stream);

                                            rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_BASIC_INFORMATION_SIZE);
                                            cdqir.emit(out_stream);

                                            struct stat buff;
                                            stat(file_to_request.c_str(), &buff);

                                            uint64_t LastAccessTime = UnixSecondsToWindowsTick(buff.st_atime);
                                            uint64_t LastWriteTime  = UnixSecondsToWindowsTick(buff.st_mtime);
                                            uint64_t ChangeTime     = UnixSecondsToWindowsTick(buff.st_ctime);
                                            uint32_t FileAttributes = fscc::FILE_ATTRIBUTE_ARCHIVE;
                                            if (S_ISDIR(buff.st_mode)) {
                                                FileAttributes = fscc::FILE_ATTRIBUTE_DIRECTORY;
                                            }
                                            fscc::FileBasicInformation fileBasicInformation(LastWriteTime, LastAccessTime, LastWriteTime, ChangeTime, FileAttributes);

                                            fileBasicInformation.emit(out_stream);

                                            InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                                , chunk_to_send
                                                                                , out_stream.get_offset()
                                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                                );
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Basic Query Information Response");
                                            }
                                            break;

                                        case rdpdr::FileStandardInformation:
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Standard Information Request");
                                            {
                                            deviceIOResponse.emit(out_stream);

                                            rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_STANDARD_INFORMATION_SIZE);
                                            cdqir.emit(out_stream);

                                            struct stat buff;
                                            stat(this->fileSystemData.paths.at(id).c_str(), &buff);

                                            int64_t  AllocationSize = buff.st_size;;
                                            int64_t  EndOfFile      = buff.st_size;
                                            uint32_t NumberOfLinks  = buff.st_nlink;
                                            uint8_t  DeletePending  = 1;
                                            uint8_t  Directory      = 0;

                                            if (S_ISDIR(buff.st_mode)) {
                                                Directory = 1;
                                            }

                                            fscc::FileStandardInformation fsi( AllocationSize
                                                                            , EndOfFile
                                                                            , NumberOfLinks
                                                                            , DeletePending
                                                                            , Directory);
                                            fsi.emit(out_stream);

                                            InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                                , chunk_to_send
                                                                                , out_stream.get_offset()
                                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                                );
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Standard Information Response");
                                            }
                                            break;

                                        case rdpdr::FileAttributeTagInformation:
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query File Attribute Tag Information Request");
                                            {
                                                std::string file_to_request = this->fileSystemData.paths.at(id);

                                                std::ifstream file(file_to_request.c_str());
                                                if (!file.good()) {
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                                                    //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", file_to_request.c_str());
                                                }
                                                deviceIOResponse.emit(out_stream);

                                                struct stat buff;
                                                stat(file_to_request.c_str(), &buff);
                                                uint32_t fileAttributes(0);
                                                if (!S_ISDIR(buff.st_mode)) {
                                                    fileAttributes = fscc::FILE_ATTRIBUTE_ARCHIVE;
                                                }

                                                rdpdr::ClientDriveQueryInformationResponse cdqir(8);
                                                cdqir.emit(out_stream);

                                                fscc::FileAttributeTagInformation fati( fileAttributes
                                                                                    , 0);
                                                fati.emit(out_stream);

                                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                                this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                                    , chunk_to_send
                                                                                    , out_stream.get_offset()
                                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                                      CHANNELS::CHANNEL_FLAG_FIRST
                                                                                    );


                                                if (bool(this->verbose & RDPVerbose::rdpdr))
                                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query File Attribute Tag Information Response");
                                            }
                                            break;

                                        default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request             unknow FsInformationClass = %x",       sdqir.FsInformationClass());
                                            break;
                                    }

                                    }
                                    break;

                                case rdpdr::IRP_MJ_CLOSE:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Close Request");
                                    {

                                    this->fileSystemData.paths.erase(id);

                                    deviceIOResponse.emit(out_stream);

                                    out_stream.out_uint32_le(0);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Close Response");
                                    }
                                    break;

                                case rdpdr::IRP_MJ_READ:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Read Request");
                                    {
                                    rdpdr::DeviceReadRequest drr;
                                    drr.receive(chunk);

                                    std::unique_ptr<uint8_t[]> ReadData;
                                    int file_size(drr.Length());
                                    int offset(drr.Offset());

                                    std::string file_to_tread = this->fileSystemData.paths.at(id);

                                    std::ifstream ateFile(file_to_tread, std::ios::binary| std::ios::ate);
                                    if(ateFile.is_open()) {
                                        if (file_size > ateFile.tellg()) {
                                            file_size = ateFile.tellg();
                                        }
                                        ateFile.close();

                                        std::ifstream inFile(file_to_tread, std::ios::in | std::ios::binary);
                                        if(inFile.is_open()) {
                                            ReadData = std::make_unique<uint8_t[]>(file_size+offset);
                                            inFile.read(reinterpret_cast<char *>(ReadData.get()), file_size+offset);
                                            inFile.close();
                                        } else {
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_tread.c_str());
                                        }
                                    } else {
                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_tread.c_str());
                                    }

                                    deviceIOResponse.emit(out_stream);
                                    rdpdr::DeviceReadResponse deviceReadResponse(file_size);
                                    deviceReadResponse.emit(out_stream);

                                    this->process_client_clipboard_out_data( channel_names::rdpdr
                                                                        , 20 + file_size
                                                                        , out_stream
                                                                        , out_stream.get_capacity() - 20
                                                                        , ReadData.get() + offset
                                                                        , file_size
                                                                        , 0);
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Read Response");
                                    }
                                    break;

                                case rdpdr::IRP_MJ_DIRECTORY_CONTROL:

                                    switch (deviceIORequest.MinorFunction()) {

                                        case rdpdr::IRP_MN_QUERY_DIRECTORY:
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Directory Request");
                                            {
                                            std::string slash("/");
                                            std::string asterix("*");

                                            rdpdr::ServerDriveQueryDirectoryRequest sdqdr;
                                            sdqdr.receive(chunk);

                                            uint64_t LastAccessTime  = 0;
                                            uint64_t LastWriteTime   = 0;
                                            uint64_t ChangeTime      = 0;
                                            uint64_t CreationTime    = 0;
                                            int64_t  EndOfFile       = 0;
                                            int64_t  AllocationSize  = 0;
                                            uint32_t FileAttributes  = fscc::FILE_ATTRIBUTE_ARCHIVE;

                                            std::string path = sdqdr.Path();
                                            std::string endPath;
                                            if (path.length() > 0) {
                                                endPath = path.substr(path.length() -1, path.length());
                                            }

                                            struct stat buff_child;
                                            std::string str_file_name;

                                            if (sdqdr.InitialQuery() && endPath != asterix) {

                                                std::string tmp_path = path;
                                                int tmp_path_index = tmp_path.find("/");
                                                while (tmp_path_index != -1) {
                                                    tmp_path = tmp_path.substr(tmp_path_index+1, tmp_path.length());
                                                    tmp_path_index = tmp_path.find("/");
                                                }
                                                str_file_name = tmp_path;

                                                std::string str_file_path_slash(this->SHARE_DIR + path);
                                                if (stat(str_file_path_slash.c_str(), &buff_child)) {
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                    //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                                                }

                                            } else {

                                                std::string str_dir_path;
                                                if (this->fileSystemData.paths.end() != this->fileSystemData.paths.find(id)) {
                                                    str_dir_path = this->fileSystemData.paths.at(id);
                                                } else {
                                                    LOG(LOG_WARNING, " Device I/O Query Directory Request Unknow ID (%d).", id);
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                }

                                                if (str_dir_path.length() > 0) {
                                                    std::string test = str_dir_path.substr(str_dir_path.length() -1, str_dir_path.length());
                                                    if (test == slash) {
                                                        str_dir_path = str_dir_path.substr(0, str_dir_path.length()-1);
                                                    }
                                                }


                                                if (sdqdr.InitialQuery()) {
                                                    this->fileSystemData.current_dir_id = id;
                                                    this->fileSystemData.elem_in_path.clear();

                                                    DIR *dir;
                                                    struct dirent *ent;
                                                    std::string ignored1("..");
                                                    std::string ignored2(".");

                                                    if ((dir = opendir (str_dir_path.c_str())) != nullptr) {

                                                        try {
                                                            while ((ent = readdir (dir)) != nullptr) {

                                                                std::string current_name = std::string (ent->d_name);

                                                                if (!(current_name == ignored1) && !(current_name == ignored2)) {
                                                                    this->fileSystemData.elem_in_path.push_back(current_name);
                                                                }
                                                            }
                                                        } catch (Error & e) {
                                                            LOG(LOG_WARNING, "readdir error: (%d) %s", e.id, e.errmsg());
                                                        }
                                                        closedir (dir);

                                                    } else {
                                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                        //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_dir).", str_dir_path.c_str());
                                                    }
                                                }

                                                if (this->fileSystemData.elem_in_path.size() == 0) {
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_MORE_FILES);
                                                } else {
                                                    str_file_name = this->fileSystemData.elem_in_path[0];
                                                    this->fileSystemData.elem_in_path.erase(this->fileSystemData.elem_in_path.begin());

                                                    std::string str_file_path_slash(str_dir_path + "/" + str_file_name);
                                                    if (stat(str_file_path_slash.c_str(), &buff_child)) {
                                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                        //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                                                    } else {
                                                        LastAccessTime  = UnixSecondsToWindowsTick(buff_child.st_atime);
                                                        LastWriteTime   = UnixSecondsToWindowsTick(buff_child.st_mtime);
                                                        CreationTime    = LastWriteTime - 1;
                                                        EndOfFile       = buff_child.st_size;
                                                        AllocationSize  = buff_child.st_size;
                                                        if (S_ISDIR(buff_child.st_mode)) {
                                                            FileAttributes = fscc::FILE_ATTRIBUTE_DIRECTORY;
                                                            EndOfFile       = 0;
                                                            AllocationSize  = 0;
                                                        }
                                                    }
                                                }
                                            }

                                            deviceIOResponse.emit(out_stream);

                                            switch (sdqdr.FsInformationClass()) {

                                                case rdpdr::FileDirectoryInformation:
                                                {
                                                    fscc::FileDirectoryInformation fbdi(CreationTime,
                                                                                            LastAccessTime,
                                                                                            LastWriteTime,
                                                                                            ChangeTime,
                                                                                            EndOfFile,
                                                                                            AllocationSize,
                                                                                            FileAttributes,
                                                                                            str_file_name.c_str());

                                                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(fbdi.total_size());
                                                    cdqdr.emit(out_stream);

                                                    fbdi.emit(out_stream);
                                                }
                                                    break;
                                                case rdpdr::FileFullDirectoryInformation:
                                                {
                                                    fscc::FileFullDirectoryInformation ffdi(CreationTime,
                                                                                            LastAccessTime,
                                                                                            LastWriteTime,
                                                                                            ChangeTime,
                                                                                            EndOfFile,
                                                                                            AllocationSize,
                                                                                            FileAttributes,
                                                                                            str_file_name.c_str());

                                                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffdi.total_size());
                                                    cdqdr.emit(out_stream);

                                                    ffdi.emit(out_stream);
                                                }
                                                    break;
                                                case rdpdr::FileBothDirectoryInformation:
                                                {
                                                    fscc::FileBothDirectoryInformation fbdi(CreationTime, LastAccessTime, LastWriteTime, ChangeTime, EndOfFile, AllocationSize, FileAttributes, str_file_name.c_str());

                                                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(fbdi.total_size());
                                                    cdqdr.emit(out_stream);

                                                    fbdi.emit(out_stream);
                                                }
                                                    break;
                                                case rdpdr::FileNamesInformation:
                                                {
                                                    fscc::FileNamesInformation ffi(str_file_name.c_str());

                                                    rdpdr::ClientDriveQueryDirectoryResponse cdqdr(ffi.total_size());
                                                    cdqdr.emit(out_stream);

                                                    ffi.emit(out_stream);
                                                }
                                                    break;
                                                default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow  FsInformationClass = 0x%x", sdqdr.FsInformationClass());
                                                        break;
                                            }

                                            InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                            this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                                , chunk_to_send
                                                                                , out_stream.get_offset()
                                                                                , CHANNELS::CHANNEL_FLAG_LAST |
                                                                                  CHANNELS::CHANNEL_FLAG_FIRST
                                                                                );
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Directory Response");
                                            }
                                            break;

                                        case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                                LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Notify Change Directory Request");
                                            {
                                                rdpdr::ServerDriveNotifyChangeDirectoryRequest sdncdr;
                                                sdncdr.receive(chunk);

                                                if (sdncdr.WatchTree) {

    //                                                 deviceIOResponse.emit(out_stream);
    //
    //                                                 fscc::FileNotifyInformation fni();
    //                                                 fni.emit(out_stream);
    //
    //                                                 InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
    //
    //                                                 this->mod->send_to_mod_channel( channel_names::rdpdr
    //                                                                                     , chunk_to_send
    //                                                                                     , out_stream.get_offset()
    //                                                                                     , CHANNELS::CHANNEL_FLAG_LAST |
    //                                                                                       CHANNELS::CHANNEL_FLAG_FIRST
    //                                                                                     );

                                                    //if (bool(this->verbose & RDPVerbose::rdpdr))
                                                    LOG(LOG_WARNING, "CLIENT >> RDPDR: Device I/O Must Send Notify Change Directory Response");
                                                }


                                            //
                                            }
                                            break;

                                        default: break;
                                    }
                                    break;

                                case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Volume Information Request");
                                    {
                                        rdpdr::ServerDriveQueryVolumeInformationRequest sdqvir;
                                        sdqvir.receive(chunk);

                                        uint64_t VolumeCreationTime             = 0;
                                        const char * VolumeLabel                = "";
                                        const char * FileSystemName             = "ext4";

                                        uint32_t FileSystemAttributes           = FileSystemData::NEW_FILE_ATTRIBUTES;
                                        uint32_t SectorsPerAllocationUnit       = 8;

                                        uint32_t BytesPerSector                 = 0;
                                        uint32_t MaximumComponentNameLength     = 0;
                                        uint64_t TotalAllocationUnits           = 0;
                                        uint64_t CallerAvailableAllocationUnits = 0;
                                        uint64_t AvailableAllocationUnits       = 0;
                                        uint64_t ActualAvailableAllocationUnits = 0;
                                        uint32_t VolumeSerialNumber             = 0;

                                        std::string str_path;

                                        if (this->fileSystemData.paths.end() != this->fileSystemData.paths.find(id)) {
                                            str_path = this->fileSystemData.paths.at(id);
                                        } else {
                                            LOG(LOG_WARNING, " Device I/O Query Volume Information Request Unknow ID (%d).", id);
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                        }

                                        struct statvfs buffvfs;
                                        if (statvfs(str_path.c_str(), &buffvfs)) {
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buffvfs).", str_path.c_str());
                                        } else {
                                            uint64_t freespace(buffvfs.f_bfree * buffvfs.f_bsize);

                                            TotalAllocationUnits           = freespace + 0x1000000;
                                            CallerAvailableAllocationUnits = freespace;
                                            AvailableAllocationUnits       = freespace;
                                            ActualAvailableAllocationUnits = freespace;

                                            BytesPerSector                 = buffvfs.f_bsize;
                                            MaximumComponentNameLength     = buffvfs.f_namemax;
                                        }

                                        static struct hd_driveid hd;
                                        int device = open(str_path.c_str(), O_RDONLY | O_NONBLOCK);
                                        if (device < 0) {
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            //LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (hd_driveid).", str_path.c_str());
                                        } else {
                                            ioctl(device, HDIO_GET_IDENTITY, &hd);
                                            VolumeSerialNumber = this->string_to_hex32(hd.serial_no);
                                        }

                                        deviceIOResponse.emit(out_stream);

                                        if (deviceIOResponse.IoStatus() == erref::NTSTATUS::STATUS_SUCCESS) {
                                            switch (sdqvir.FsInformationClass()) {
                                                case rdpdr::FileFsVolumeInformation:
                                                {
                                                    fscc::FileFsVolumeInformation ffvi(VolumeCreationTime, VolumeSerialNumber, 0, VolumeLabel);

                                                    rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffvi.size());
                                                    cdqvir.emit(out_stream);

                                                    ffvi.emit(out_stream);
                                                }
                                                    break;

                                                case rdpdr::FileFsSizeInformation:
                                                {
                                                    fscc::FileFsSizeInformation ffsi(TotalAllocationUnits, AvailableAllocationUnits, SectorsPerAllocationUnit, BytesPerSector);

                                                    rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffsi.size());
                                                    cdqvir.emit(out_stream);

                                                    ffsi.emit(out_stream);
                                                }
                                                    break;

                                                case rdpdr::FileFsAttributeInformation:
                                                {
                                                    fscc::FileFsAttributeInformation ffai(FileSystemAttributes, MaximumComponentNameLength, FileSystemName);

                                                    rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffai.size());
                                                    cdqvir.emit(out_stream);

                                                    ffai.emit(out_stream);
                                                }
                                                    break;
                                                case rdpdr::FileFsFullSizeInformation:
                                                {
                                                    fscc::FileFsFullSizeInformation fffsi(TotalAllocationUnits, CallerAvailableAllocationUnits, ActualAvailableAllocationUnits, SectorsPerAllocationUnit, BytesPerSector);

                                                    rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(fffsi.size());
                                                    cdqvir.emit(out_stream);

                                                    fffsi.emit(out_stream);
                                                }
                                                    break;

                                                case rdpdr::FileFsDeviceInformation:
                                                {
                                                    fscc::FileFsDeviceInformation ffdi(fscc::FILE_DEVICE_DISK, fscc::FILE_REMOTE_DEVICE | fscc::FILE_DEVICE_IS_MOUNTED);

                                                    rdpdr::ClientDriveQueryVolumeInformationResponse cdqvir(ffdi.size());
                                                    cdqvir.emit(out_stream);

                                                    ffdi.emit(out_stream);
                                                }
                                                    break;

                                                default:
                                                    LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow FsInformationClass = 0x%x", sdqvir.FsInformationClass());
                                                    break;
                                            }
                                        }

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (bool(this->verbose & RDPVerbose::rdpdr))
                                            LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Volume Information Response");

                                    }
                                    break;

                                case rdpdr::IRP_MJ_WRITE:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Request");
                                    {
                                        rdpdr::DeviceWriteRequest dwr;
                                        dwr.receive(chunk);

                                        size_t WriteDataLen(dwr.Length);

                                        if (dwr.Length > CHANNELS::CHANNEL_CHUNK_LENGTH) {

                                            this->fileSystemData.writeData_to_wait = dwr.Length - rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
                                            this->fileSystemData.file_to_write_id = id;
                                            WriteDataLen = rdpdr::DeviceWriteRequest::FISRT_PART_DATA_MAX_LEN;
                                        }

                                        std::string file_to_write = this->fileSystemData.paths.at(id);

                                        std::ofstream oFile(file_to_write.c_str(), std::ios::out | std::ios::binary);
                                        if (oFile.good()) {
                                            oFile.write(reinterpret_cast<const char *>(dwr.WriteData), WriteDataLen);
                                            oFile.close();
                                        }  else {
                                            LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", file_to_write.c_str());
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                        }

                                        deviceIOResponse.emit(out_stream);
                                        rdpdr::DeviceWriteResponse dwrp(dwr.Length);
                                        dwrp.emit(out_stream);

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                                CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (bool(this->verbose & RDPVerbose::rdpdr))
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Response");
                                    }

                                    break;

                                case rdpdr::IRP_MJ_SET_INFORMATION:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Server Drive Set Information Request");
                                    {
                                        rdpdr::ServerDriveSetInformationRequest sdsir;
                                        sdsir.receive(chunk);

                                        std::string file_to_request = this->fileSystemData.paths.at(id);

                                        std::ifstream file(file_to_request.c_str(), std::ios::in |std::ios::binary);
                                        if (!file.good()) {
                                            LOG(LOG_WARNING, "  Can't open such file of directory : \'%s\'.", file_to_request.c_str());
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            file.close();
                                        }


                                        rdpdr::ClientDriveSetInformationResponse cdsir(sdsir.Length());


                                        switch (sdsir.FsInformationClass()) {

                                            case rdpdr::FileRenameInformation:
                                            {
                                                rdpdr::RDPFileRenameInformation rdpfri;
                                                rdpfri.receive(chunk);

                                                std::string fileName(this->SHARE_DIR + rdpfri.FileName());
                                                if (rename(file_to_request.c_str(), fileName.c_str()) !=  0) {
                                                    LOG(LOG_WARNING, "  Can't rename such file of directory : \'%s\' to \'%s\'.", file_to_request.c_str(), fileName.c_str());
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_OBJECT_NAME_INVALID);
                                                }

                                                deviceIOResponse.emit(out_stream);
                                                cdsir.emit(out_stream);
                                            }
                                                break;

                                            case rdpdr::FileAllocationInformation :
                                                deviceIOResponse.emit(out_stream);
                                                cdsir.emit(out_stream);
                                                break;

                                            case rdpdr::FileEndOfFileInformation:
                                                deviceIOResponse.emit(out_stream);
                                                cdsir.emit(out_stream);
                                                break;

                                            case rdpdr::FileDispositionInformation:
                                            {
                                                uint8_t DeletePending = 1;

                                                std::string file_to_request = this->fileSystemData.paths.at(id);

                                                if (remove(file_to_request.c_str()) != 0) {
                                                    DeletePending = 0;
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                                                    LOG(LOG_WARNING, "  Can't delete such file of directory : \'%s\'.", file_to_request.c_str());
                                                }

                                                deviceIOResponse.emit(out_stream);
                                                cdsir.emit(out_stream);
                                                fscc::FileDispositionInformation fdi(DeletePending);
                                                fdi.emit(out_stream);
                                            }
                                                break;

                                            case rdpdr::FileBasicInformation:
                                            {
                                                deviceIOResponse.emit(out_stream);
                                                cdsir.emit(out_stream);
                                            }
                                                break;

                                            default:  LOG(LOG_INFO, "SERVER >> RDPDR: unknow FsInformationClass = 0x%x", sdsir.FsInformationClass());

                                                break;
                                        }

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (bool(this->verbose & RDPVerbose::rdpdr))
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Set Information Response");
                                    }

                                    break;

                                case rdpdr::IRP_MJ_DEVICE_CONTROL:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Control Response");
                                    {
                                        rdpdr::DeviceControlRequest dcr;
                                        dcr.receive(chunk);

                                        deviceIOResponse.emit(out_stream);

                                        switch (dcr.IoControlCode()) {
                                            case fscc::FSCTL_CREATE_OR_GET_OBJECT_ID :
                                            {
                                                rdpdr::ClientDriveControlResponse cdcr(64);
                                                cdcr.emit(out_stream);

                                                uint8_t ObjectId[16] =  { 0 };
                                                ObjectId[0] = 1;
                                                uint8_t BirthVolumeId[16] =  { 0 };
                                                BirthVolumeId[15] = 1;
                                                uint8_t BirthObjectId[16] =  { 0 };
                                                BirthObjectId[15] = 1;

                                                fscc::FileObjectBuffer_Type1 rgdb(ObjectId, BirthVolumeId, BirthObjectId);
                                                rgdb.emit(out_stream);
                                            }
                                            break;

                                            case fscc::FSCTL_GET_OBJECT_ID :
                                            {
                                                rdpdr::ClientDriveControlResponse cdcr(64);
                                                cdcr.emit(out_stream);

                                                uint8_t ObjectId[16] =  { 0 };
                                                ObjectId[0] = 1;
                                                uint8_t BirthVolumeId[16] =  { 0 };
                                                uint8_t BirthObjectId[16] =  { 0 };

                                                fscc::FileObjectBuffer_Type1 rgdb(ObjectId, BirthVolumeId, BirthObjectId);
                                                rgdb.emit(out_stream);

                                            }
                                                break;

                                            default: LOG(LOG_INFO, "     Device Controle UnLogged IO Control Data: Code = 0x%08x", dcr.IoControlCode());
                                                break;
                                        }

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->mod->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                    }
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Client Drive Control Response");
                                    break;

                                default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request unknow MajorFunction = %x",       deviceIORequest.MajorFunction());
                                    break;
                            }

                            } break;

                        default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT RDPDR_CTYP_CORE unknow packetId = %x",       packetId);
                            break;
                    }
                            break;

                case rdpdr::Component::RDPDR_CTYP_PRT:
                {
                    //hexdump_c(chunk_series.get_data(), chunk_size);
                    chunk.in_skip_bytes(4);

                    switch (packetId) {
                        case rdpdr::PacketId::PAKID_CORE_SERVER_ANNOUNCE:
                        {
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Announce Request ");
                        }
                            break;

                        case rdpdr::PacketId::PAKID_CORE_SERVER_CAPABILITY:
                        {
                            uint16_t capa  = chunk.in_uint16_le();
                            chunk.in_skip_bytes(2);
                            bool driveEnable = false;
                            for (int i = 0; i < capa; i++) {
                                uint16_t type = chunk.in_uint16_le();
                                uint16_t size = chunk.in_uint16_le() - 4;
                                chunk.in_skip_bytes(size);
                                this->fileSystemData.fileSystemCapacity[type] = true;
                                if (type == 0x4) {
                                    driveEnable = true;
                                }
                            }

                            if (bool(this->verbose & RDPVerbose::rdpdr)) {
                                if (driveEnable) {
                                    LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Core Capability Request - Drive Capability Enable");
                                } else {
                                    LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Core Capability Request - Drive Not Allowed");
                                }
                            }
                        }
                            break;

                        case rdpdr::PacketId::PAKID_CORE_USER_LOGGEDON:
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server User Logged On");
                            break;

                        case rdpdr::PacketId::PAKID_CORE_DEVICE_REPLY:
                        {
                            rdpdr::ServerDeviceAnnounceResponse sdar;
                            sdar.receive(chunk);

                            if (sdar.ResultCode() == erref::NTSTATUS::STATUS_SUCCESS) {
                                this->fileSystemData.drives_created = true;
                            } else {
                                this->fileSystemData.drives_created = false;
                                LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: Can't create virtual disk ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                            }
                            if (bool(this->verbose & RDPVerbose::rdpdr))
                                LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Device Announce Response ID=%x Hres=%x", sdar.DeviceId(), sdar.ResultCode());
                        }
                            break;

                        case rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM:
                        {
                            LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Server Client ID Confirm");
                        }
                            break;

                        case rdpdr::PacketId::PAKID_CORE_DEVICE_IOREQUEST:
                        {
                            rdpdr::DeviceIORequest deviceIORequest;
                            deviceIORequest.receive(chunk);


                            StaticOutStream<1024> out_stream;
                            rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                            , rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
                            sharedHeader.emit(out_stream);

                            rdpdr::DeviceIOResponse deviceIOResponse( deviceIORequest.DeviceId()
                                                                    , deviceIORequest.CompletionId()
                                                                    , erref::NTSTATUS::STATUS_SUCCESS);

                            switch (deviceIORequest.MajorFunction()) {

                                case rdpdr::IRP_MJ_CREATE:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Create Request");
                                        break;

                                case rdpdr::IRP_MJ_READ:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Read Request");
                                        break;

                                case rdpdr::IRP_MJ_CLOSE:
                                    if (bool(this->verbose & RDPVerbose::rdpdr))
                                        LOG(LOG_INFO, "SERVER >> RDPDR PRINTER: Device I/O Close Request");
                                        break;

                                    hexdump_c(chunk_series.get_data(), chunk_size);
                                        break;


                                default:
                                    LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: DEFAULT PRINTER unknow MajorFunction = %x", deviceIORequest.MajorFunction());
                                    //hexdump_c(chunk_series.get_data(), chunk_size);
                                    break;
                            }
                        }
                            break;


                        default :
                            LOG(LOG_WARNING, "SERVER >> RDPDR PRINTER: DEFAULT PRINTER unknow packetId = %x", packetId);
                            break;
                    }
                }
                    break;

                default: LOG(LOG_WARNING, "SERVER >> RDPDR: DEFAULT RDPDR unknow component = %x", component);
                    break;
            }
        } else  /*if (!strcmp(channel.name, channel_names::rdpsnd))*/ {
            LOG(LOG_INFO, "SERVER >> RDPEA: Server Audio Formats and Version PDU");
        }
    }


    void process_client_clipboard_out_data(const char * const front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  uint8_t const * data, const size_t data_len, uint32_t flags){

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

                this->mod->send_to_mod_channel( front_channel_name
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

                this->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_next
                                                    , total_length
                                                    , flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_next_part.get_data(), out_stream_next_part.get_offset());
            }

            // Last part
                StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream_last_part;
                out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);

                data_sent += remains_PDU;
                InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

                this->mod->send_to_mod_channel( front_channel_name
                                                    , chunk_last
                                                    , total_length
                                                    , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                    );

    //             msgdump_c(false, false, total_length, 0, out_stream_last_part.get_data(), out_stream_last_part.get_offset());

        } else {

            out_stream_first_part.out_copy_bytes(data, data_len);
            InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

            this->mod->send_to_mod_channel( front_channel_name
                                                , chunk
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                                flags
                                                );
        }
    }

    void process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList, ClipBoard_Qt * clipboard_qt) {

        // 3.1.5.2.2 Processing of Virtual Channel PDU

        // The Virtual Channel PDU is received by both the client and the server. Its structure
        // and fields are specified in section 2.2.6.1.

        // If Enhanced RDP Security (section 5.4) is in effect, the External Security Protocol (section 5.4.5)
        // being used to secure the connection MUST be used to decrypt and verify the integrity of the entire
        // PDU prior to any processing taking place.

        // The embedded length fields within the tpktHeader ([T123] section 8) and mcsPdu ([T125] section 7, parts
        // 7 and 10) fields MUST be examined for consistency with the received data. If there is any discrepancy,
        // the connection SHOULD be dropped.

        // The mcsPdu field encapsulates either an MCS Send Data Request PDU (if the PDU is being sent from client
        // to server) or an MCS Send Data Indication PDU (if the PDU is being sent from server to client). In both
        // of these cases, the embedded channelId field MUST contain the server-assigned virtual channel ID. This
        // ID MUST be used to route the data in the virtualChannelData field to the appropriate virtual channel
        // endpoint after decryption of the PDU and any necessary decompression of the payload has been conducted.

        // The conditions mandating the presence of the securityHeader field, as well as the type of Security
        // Header structure present in this field, are explained in section 2.2.6.1. If the securityHeader field is
        // present, the embedded flags field MUST be examined for the presence of the SEC_ENCRYPT (0x0008) flag
        // (section 2.2.8.1.1.2.1), and, if it is present, the data following the securityHeader field MUST be
        // verified and decrypted using the methods and techniques specified in section 5.3.6. If the MAC signature
        // is incorrect, or the data cannot be decrypted correctly, the connection SHOULD be dropped.

        // If the data in the virtualChannelData field is compressed, then the data MUST be decompressed using
        // the techniques detailed in section 3.1.8.3 (the Virtual Channel PDU compression flags are specified
        // in section 2.2.6.1.1).

        // If the embedded flags field of the channelPduHeader field (the Channel PDU Header structure is specified
        // in section 2.2.6.1.1) does not contain the CHANNEL_FLAG_FIRST (0x00000001) flag or CHANNEL_FLAG_LAST
        // (0x00000002) flag, and the data is not part of a chunked sequence (that is, a start chunk has not been
        // received), then the data in the virtualChannelData field can be dispatched to the appropriate virtual
        // channel endpoint (no reassembly is required by the endpoint). If the CHANNEL_FLAG_SHOW_PROTOCOL
        // (0x00000010) flag is specified in the Channel PDU Header, then the channelPduHeader field MUST also
        // be dispatched to the virtual channel endpoint.

        // If the virtual channel data is part of a sequence of chunks, then the instructions in section 3.1.5.2.2.1
        //MUST be followed to reassemble the stream.

        if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->_waiting_for_data = true;
            cb_buffers.sizeTotal = chunk.in_uint32_le();
            cb_buffers.data = std::make_unique<uint8_t[]>(cb_buffers.sizeTotal);
        }

        switch (this->_requestedFormatId) {

            case RDPECLIP::CF_UNICODETEXT:
            case RDPECLIP::CF_TEXT:
                this->send_to_clipboard_Buffer(chunk);
                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard();
                }
            break;

            case RDPECLIP::CF_METAFILEPICT:

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                    RDPECLIP::MetaFilePicDescriptor mfpd;
                    mfpd.receive(chunk);

                    cb_buffers.pic_height = mfpd.height;
                    cb_buffers.pic_width  = mfpd.width;
                    cb_buffers.pic_bpp    = mfpd.bpp;
                    cb_buffers.sizeTotal  = mfpd.imageSize;
                    cb_buffers.data       = std::make_unique<uint8_t[]>(cb_buffers.sizeTotal);
                }

                this->send_to_clipboard_Buffer(chunk);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_imageBuffer_to_clipboard();
                }
            break;

            case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    cb_buffers.sizeTotal -= 4;
                    cb_filesList.cItems= chunk.in_uint32_le();
                    cb_filesList.lindexToRequest= 0;
                    clipboard_qt->emptyBuffer();
                    cb_filesList.itemslist.clear();
                }

                this->send_to_clipboard_Buffer(chunk);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    InStream stream(cb_buffers.data.get(), cb_buffers.sizeTotal);

                    RDPECLIP::FileDescriptor fd;

                    for (uint32_t i = 0; i < cb_filesList.cItems; i++) {
                        fd.receive(stream);
                        CB_FilesList::CB_in_Files file;
                        file.size = fd.file_size();
                        file.name = fd.fileName();
                        cb_filesList.itemslist.push_back(file);
                    }

                    RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                        , RDPECLIP::FILECONTENTS_RANGE
                                                                        , cb_filesList.lindexToRequest
                                                                        , cb_filesList.itemslist[cb_filesList.lindexToRequest].size);
                    StaticOutStream<32> out_streamRequest;
                    fileContentsRequest.emit(out_streamRequest);
                    const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                    InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                    this->mod->send_to_mod_channel( channel_names::cliprdr
                                                        , chunkRequest
                                                        , total_length_FormatDataRequestPDU
                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                        CHANNELS::CHANNEL_FLAG_FIRST |
                                                        CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                        );
                    if (bool(this->verbose & RDPVerbose::cliprdr)) {
                        LOG(LOG_INFO, "CLIENT >> CB channel: File Contents Resquest PDU FILECONTENTS_RANGE");
                    }

                    this->empty_buffer();
                }
            break;

            case ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS:

                if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                    cb_filesList.streamIDToRequest = chunk.in_uint32_le();
                }

                if (cb_filesList.lindexToRequest == cb_filesList.itemslist.size()) {
                    cb_filesList.lindexToRequest--;
                }

                clipboard_qt->write_clipboard_temp_file( cb_filesList.itemslist[cb_filesList.lindexToRequest].name
                                                    , chunk.get_current()
                                                    , chunk.in_remain()
                                                    );

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->_waiting_for_data = false;

                    cb_filesList.lindexToRequest++;

                    if (cb_filesList.lindexToRequest >= cb_filesList.cItems) {

                        clipboard_qt->_local_clipboard_stream = false;
                        clipboard_qt->setClipboard_files(cb_filesList.itemslist);
                        clipboard_qt->_local_clipboard_stream = true;

                        RDPECLIP::UnlockClipboardDataPDU unlockClipboardDataPDU(0);
                        StaticOutStream<32> out_stream_unlock;
                        unlockClipboardDataPDU.emit(out_stream_unlock);
                        InStream chunk_unlock(out_stream_unlock.get_data(), out_stream_unlock.get_offset());

                        this->mod->send_to_mod_channel( channel_names::cliprdr
                                                            , chunk_unlock
                                                            , out_stream_unlock.get_offset()
                                                            , CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                            CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                            );
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "CLIENT >> CB channel: Unlock Clipboard Data PDU");
                        }

                    } else {
                        cb_filesList.streamIDToRequest++;

                        StaticOutStream<32> out_streamRequest;
                        RDPECLIP::FileContentsRequestPDU fileContentsRequest( cb_filesList.streamIDToRequest
                                                                            , RDPECLIP::FILECONTENTS_RANGE
                                                                            , cb_filesList.lindexToRequest
                                                                            ,   cb_filesList.itemslist[cb_filesList.lindexToRequest].size);
                        fileContentsRequest.emit(out_streamRequest);
                        const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                        InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                        this->mod->send_to_mod_channel( channel_names::cliprdr
                                                            , chunkRequest
                                                            , total_length_FormatDataRequestPDU
                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                            CHANNELS::CHANNEL_FLAG_FIRST |
                                                            CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                            );
                        if (bool(this->verbose & RDPVerbose::cliprdr)) {
                            LOG(LOG_INFO, "CLIENT >> CB channel: File Contents Resquest PDU FILECONTENTS_RANGE");
                        }
                    }

                    this->empty_buffer();
                }
            break;

            default:
                if (strcmp(this->_requestedFormatName.c_str(), RDPECLIP::get_format_short_name(RDPECLIP::SF_TEXT_HTML)) == 0) {
                    this->send_to_clipboard_Buffer(chunk);

                    if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                        this->send_textBuffer_to_clipboard();
                    }

                }  else {
                    LOG(LOG_WARNING, "SERVER >> CB channel: unknow CB Format = %x", this->_requestedFormatId);
                }
            break;
        }
    }

    void FremoveDriveDevice(const FileSystemData::DeviceData * devices, const size_t deviceCount) {
        StaticOutStream<1024> out_stream;

        rdpdr::SharedHeader sharedHeader( rdpdr::RDPDR_CTYP_CORE
                                        , rdpdr::PAKID_CORE_DEVICELIST_REMOVE);
        sharedHeader.emit(out_stream);
        out_stream.out_uint32_le(deviceCount);
        for (size_t i = 0; i < deviceCount; i++) {
            out_stream.out_uint32_le(devices[i].ID);
        }

        int total_length(out_stream.get_offset());
        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

        this->mod->send_to_mod_channel( channel_names::rdpdr
                                            , chunk_to_send
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                            CHANNELS::CHANNEL_FLAG_FIRST
                                            );
        if (bool(this->verbose & RDPVerbose::rdpdr))
            LOG(LOG_INFO, "CLIENT >> RDPDR: Client Drive Device List Remove");
    }

    void send_textBuffer_to_clipboard() {
        std::unique_ptr<uint8_t[]> utf8_string = std::make_unique<uint8_t[]>(this->_cb_buffers.sizeTotal);
        size_t length_of_utf8_string = ::UTF16toUTF8(
            this->_cb_buffers.data.get(), this->_cb_buffers.sizeTotal,
            utf8_string.get(), this->_cb_buffers.sizeTotal);
        std::string str(reinterpret_cast<const char*>(utf8_string.get()), length_of_utf8_string);

        this->clipboard_qt->_local_clipboard_stream = false;
        this->clipboard_qt->setClipboard_text(str);
        this->clipboard_qt->_local_clipboard_stream = true;

        this->empty_buffer();
    }

    void send_to_clipboard_Buffer(InStream & chunk) {

        const size_t length_of_data_to_dump(chunk.in_remain());
        const size_t sum_buffer_and_data(this->_cb_buffers.size + length_of_data_to_dump);
        const uint8_t * utf8_data = chunk.get_current();

        for (size_t i = 0; i < length_of_data_to_dump && i + this->_cb_buffers.size < this->_cb_buffers.sizeTotal; i++) {
            this->_cb_buffers.data[i + this->_cb_buffers.size] = utf8_data[i];
        }

        this->_cb_buffers.size = sum_buffer_and_data;
    }

    void send_imageBuffer_to_clipboard() {

        QImage image(this->_cb_buffers.data.get(),
                    this->_cb_buffers.pic_width,
                    this->_cb_buffers.pic_height,
                    this->bpp_to_QFormat(this->_cb_buffers.pic_bpp, false));

        QImage imageSwapped(image.rgbSwapped().mirrored(false, true));

        this->clipboard_qt->_local_clipboard_stream = false;
        this->clipboard_qt->setClipboard_image(imageSwapped);
        this->clipboard_qt->_local_clipboard_stream = true;

        this->empty_buffer();
    }

    virtual void empty_buffer() override {
        this->_cb_buffers.pic_bpp    = 0;
        this->_cb_buffers.sizeTotal  = 0;
        this->_cb_buffers.pic_width  = 0;
        this->_cb_buffers.pic_height = 0;
        this->_cb_buffers.size       = 0;
        this->_waiting_for_data = false;
    }

    void emptyLocalBuffer() override {
        this->clipboard_qt->emptyBuffer();
    }

    void send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size) override {

        StaticOutStream<1024> out_stream;
        RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListDataShortName, formatIDs_size);
        format_list_pdu_long.emit(out_stream);
        const uint32_t total_length = out_stream.get_offset();
        InStream chunk(out_stream.get_data(), out_stream.get_offset());

        this->mod->send_to_mod_channel( channel_names::cliprdr
                                            , chunk
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                            CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                            );
        if (bool(this->verbose & RDPVerbose::cliprdr)) {
            LOG(LOG_INFO, "CLIENT >> CB channel: Format List PDU");
        }
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    virtual void callback() override {
        if (this->_recv_disconnect_ultimatum) {
            this->dropScreen();
            std::string labelErrorMsg("<font color='Red'>Disconnected by server</font>");
            this->disconnect(labelErrorMsg);
            this->cache = nullptr;
            this->trans_cache = nullptr;
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

    // bjam san -j4 rdpproxy

    // sudo bin/gcc-4.9.2/san/rdpproxy -nf

    //bjam debug client_rdp_Qt4 && bin/gcc-4.9.2/debug/threading-multi/client_rdp_Qt4 -n admin -w $mdp -i 10.10.40.22 -p 3389

    // sed '/usr\/include\/qt4\|threading-multi\/src\/Qt4\/\|in expansion of macro .*Q_OBJECT\|Wzero/,/\^/d' &&

    QApplication app(argc, argv);

    // RDPVerbose::rdpdr_dump | RDPVerbose::cliprdr;
    RDPVerbose verbose = RDPVerbose::none;                  // RDPVerbose::graphics | RDPVerbose::cliprdr | RDPVerbose::rdpdr;

    RDPClientQtFront front_qt(argv, argc, verbose);


    app.exec();

    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2
}

