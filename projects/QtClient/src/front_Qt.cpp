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

#define LOGPRINT
#include "utils/log.hpp"

#include <string>
#include <unistd.h>

#include "front_widget_Qt.hpp"

#pragma GCC diagnostic pop

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"

#include "configs/autogen/enums.hpp"



Front_Qt::Front_Qt(char* argv[], int argc, RDPVerbose verbose)
    : Front_Qt_API(false, false, verbose)
    , snapshoter(*this)
    , mod_bpp(24)
    , mod_palette(BGRPalette::classic_332())
    , _form(nullptr)
    , _cache(nullptr)
    , _trans_cache(nullptr)
    , _graph_capture(nullptr)
    , _clipboard_qt(nullptr)
    , _timer(0)
    , _connected(false)
    , _monitorCountNegociated(false)
    , _clipboard_channel(&(this->_to_client_sender), &(this->_to_server_sender) ,*this , [](){
        DummyAuthentifier authentifier;
        ClipboardVirtualChannel::Params params(authentifier);

        params.exchanged_data_limit = ~decltype(params.exchanged_data_limit){};
        params.verbose = to_verbose_flags(0xfffffff);

        params.clipboard_down_authorized = true;
        params.clipboard_up_authorized = true;
        params.clipboard_file_authorized = true;

        params.dont_log_data_into_syslog = true;
        params.dont_log_data_into_wrm = true;

        params.client_use_long_format_names = true;

        return params;
    }())
    , _capture(nullptr)
    , _error("error")
    , _keymap()
    , _ctrl_alt_delete(false)
    , _waiting_for_data(false)
    , _clipbrdFormatsList()
    , _cb_filesList()
    , _cb_buffers()
{
    SSL_load_error_strings();
    SSL_library_init();

    this->fileSystemData.drives[0].name[0] = 'R';
    this->fileSystemData.drives[0].name[1] = 'D';
    this->fileSystemData.drives[0].name[2] = 'P';
    this->fileSystemData.drives[0].name[3] = ' ';
    this->fileSystemData.drives[0].name[4] = 'C';
    this->fileSystemData.drives[0].name[5] = ':';
    this->fileSystemData.drives[0].ID = 1;

//     this->fileSystemData.drives[1].name[0] = 'R';
//     this->fileSystemData.drives[1].name[1] = 'D';
//     this->fileSystemData.drives[1].name[2] = 'P';
//     this->fileSystemData.drives[1].name[3] = ' ';
//     this->fileSystemData.drives[1].name[4] = 'D';
//     this->fileSystemData.drives[1].name[5] = ':';
//     this->fileSystemData.drives[1].ID = 2;

    // Windows and socket contrainer
    this->_mod_qt = new Mod_Qt(this, this->_form);
    this->_form = new Form_Qt(this);
    this->_clipboard_qt = new ClipBoard_Qt(this, this->_form);

    this->setClientInfo();

    const char * localIPtmp = "unknow_local_IP";
    this->_localIP       = localIPtmp;
    this->_nbTry         = 3;
    this->_retryDelay    = 1000;

    uint8_t commandIsValid(0);

    // TODO QCommandLineParser / program_options
    for (int i = 0; i <  argc - 1; i++) {

        std::string word(argv[i]);
        std::string arg(argv[i+1]);

        if (       word == "-n") {
            this->_userName = arg;
            commandIsValid += NAME_GOTTEN;
        } else if (word == "-w") {
            this->_pwd = arg;
            commandIsValid += PWD_GOTTEN;
        } else if (word == "-i") {
            this->_targetIP = arg;
            commandIsValid += IP_GOTTEN;
        } else if (word == "-p") {
            std::string portStr(arg);
            this->_port = std::stoi(portStr);
            commandIsValid += PORT_GOTTEN;
        }
    }

    if (this->mod_bpp == this->_info.bpp) {
        this->mod_palette = BGRPalette::classic_332();
    }

    this->_qtRDPKeymap.setKeyboardLayout(this->_info.keylayout);
    this->_keymap.init_layout(this->_info.keylayout);

    /*std::cout << "cs_monitor count negociated. MonitorCount=" << this->_monitorCount << std::endl;
                    std::cout << "width=" <<  this->_info.width <<  " " << "height=" << this->_info.height <<  std::endl;*/

    if (commandIsValid == Front_Qt::COMMAND_VALID) {
        this->connect();

    } else {
        std::cout << "Argument(s) required to connect: ";
        if (!(commandIsValid & Front_Qt::NAME_GOTTEN)) {
            std::cout << "-n [user_name] ";
        }
        if (!(commandIsValid & Front_Qt::PWD_GOTTEN)) {
            std::cout << "-w [password] ";
        }
        if (!(commandIsValid & Front_Qt::IP_GOTTEN)) {
            std::cout << "-i [ip_serveur] ";
        }
        if (!(commandIsValid & Front_Qt::PORT_GOTTEN)) {
            std::cout << "-p [port] ";
        }
        std::cout << std::endl;

        this->disconnect("");
    }
}


bool Front_Qt::setClientInfo() {

    // default config
    this->_info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
    this->_info.console_session = 0;
    this->_info.brush_cache_code = 0;
    this->_info.bpp = 24;
    this->_imageFormatRGB  = this->bpp_to_QFormat(this->_info.bpp, false);
    if (this->_info.bpp ==  32) {
        this->_imageFormatARGB = this->bpp_to_QFormat(this->_info.bpp, true);
    }
    this->_width  = 800;
    this->_height = 600;
    this->_info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    this->_fps = 30;
    this->_info.cs_monitor.monitorCount = 1;
    //this->_info.encryptionLevel = 1;

    // file config
    std::ifstream ifichier(this->USER_CONF_DIR);
    if(ifichier) {
        // get config from conf file
        std::string ligne;
        std::string delimiter = " ";

        while(std::getline(ifichier, ligne)) {
            auto pos(ligne.find(delimiter));
            std::string tag  = ligne.substr(0, pos);
            std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

            if (tag.compare(std::string("keylayout")) == 0) {
                this->_info.keylayout = std::stoi(info);
            } else
            if (tag.compare(std::string("console_session")) == 0) {
                this->_info.console_session = std::stoi(info);
            } else
            if (tag.compare(std::string("brush_cache_code")) == 0) {
                this->_info.brush_cache_code = std::stoi(info);
            } else
            if (tag.compare(std::string("bpp")) == 0) {
                this->_info.bpp = std::stoi(info);
            } else
            if (tag.compare(std::string("width")) == 0) {
                this->_width      = std::stoi(info);
            } else
            if (tag.compare(std::string("height")) == 0) {
                this->_height     = std::stoi(info);
            } else
            if (tag.compare(std::string("rdp5_performanceflags")) == 0) {
                this->_info.rdp5_performanceflags = std::stoi(info);
            } else
            if (tag.compare(std::string("fps")) == 0) {
                this->_fps = std::stoi(info);
            } else
            if (tag.compare(std::string("monitorCount")) == 0) {
                this->_info.cs_monitor.monitorCount = std::stoi(info);
                this->_monitorCount                 = std::stoi(info);
            } else
            if (tag.compare(std::string("span")) == 0) {
                if (std::stoi(info)) {
                    this->_span = true;
                }
            } else
            if (tag.compare(std::string("record")) == 0) {
                if (std::stoi(info)) {
                    this->_record = true;
                }
            } else
            if (tag.compare(std::string("tls")) == 0) {
                if (std::stoi(info)) {
                    this->_mod_qt->_modRDPParamsData.enable_tls = true;
                } else { this->_mod_qt->_modRDPParamsData.enable_tls = false; }
            } else
            if (tag.compare(std::string("nla")) == 0) {
                if (std::stoi(info)) {
                    this->_mod_qt->_modRDPParamsData.enable_nla = true;
                } else { this->_mod_qt->_modRDPParamsData.enable_nla = false; }
            } else
            if (tag.compare(std::string("delta_time")) == 0) {
                if (std::stoi(info)) {
                    this->_delta_time = std::stoi(info);
                }
            }else
            if (tag.compare(std::string("enable_shared_clipboard")) == 0) {
                if (std::stoi(info)) {
                    this->_enable_shared_clipboard = true;
                }
            }else
            if (tag.compare(std::string("enable_shared_virtual_disk")) == 0) {
                if (std::stoi(info)) {
                    this->_enable_shared_virtual_disk = true;
                }
            } else
            if (tag.compare(std::string("SHARE_DIR")) == 0) {
                this->SHARE_DIR                 = info;
            }
        }

        this->_info.width  = this->_width * this->_monitorCount;
        this->_info.height = this->_height;

        ifichier.close();

        this->_imageFormatRGB  = this->bpp_to_QFormat(this->_info.bpp, false);
        if (this->_info.bpp ==  32) {
            this->_imageFormatARGB = this->bpp_to_QFormat(this->_info.bpp, true);
        }

        return false;

    } else {

        this->_info.width  = this->_width * this->_monitorCount;
        this->_info.height = this->_height;

        return true;
    }
}

void Front_Qt::writeClientInfo() {
    std::ofstream ofichier(this->USER_CONF_DIR, std::ios::out | std::ios::trunc);
    if(ofichier) {

        ofichier << "User Info" << "\n\n";

        ofichier << "keylayout "             << this->_info.keylayout               << "\n";
        ofichier << "console_session "       << this->_info.console_session         << "\n";
        ofichier << "brush_cache_code "      << this->_info.brush_cache_code        << "\n";
        ofichier << "bpp "                   << this->mod_bpp                     << "\n";
        ofichier << "width "                 << this->_width                        << "\n";
        ofichier << "height "                << this->_height                       << "\n";
        ofichier << "rdp5_performanceflags " << this->_info.rdp5_performanceflags   << "\n";
        ofichier << "fps "                   << this->_fps                          << "\n";
        ofichier << "monitorCount "          << this->_info.cs_monitor.monitorCount << "\n";
        ofichier << "span "                  << this->_span                         << "\n";
        ofichier << "record "                << this->_record                       << "\n";
        ofichier << "tls "                   << this->_mod_qt->_modRDPParamsData.enable_tls << "\n";
        ofichier << "nla "                   << this->_mod_qt->_modRDPParamsData.enable_nla << "\n";
        ofichier << "delta_time "            << this->_delta_time << "\n";
        ofichier << "enable_shared_clipboard "    << this->_enable_shared_clipboard << "\n";
        ofichier << "enable_shared_virtual_disk " << this->_enable_shared_virtual_disk << std::endl;
        ofichier << "SHARE_DIR " << this->SHARE_DIR << std::endl;
    }
}

void Front_Qt::set_pointer(Pointer const & cursor) {

    QImage image_data(cursor.data, cursor.width, cursor.height, this->bpp_to_QFormat(24, false));
    QImage image_mask(cursor.mask, cursor.width, cursor.height, QImage::Format_Mono);

    if (cursor.mask[0x48] == 0xFF &&
        cursor.mask[0x49] == 0xFF &&
        cursor.mask[0x4A] == 0xFF &&
        cursor.mask[0x4B] == 0xFF) {

        image_mask = image_data.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        image_data.invertPixels();

    } else {
        image_mask.invertPixels();
    }

    image_data = image_data.mirrored(false, true).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    image_mask = image_mask.mirrored(false, true).convertToFormat(QImage::Format_ARGB32_Premultiplied);

    const uchar * data_data = image_data.bits();
    const uchar * mask_data = image_mask.bits();

    uint8_t data[Pointer::DATA_SIZE*4];

    for (int i = 0; i < Pointer::DATA_SIZE; i += 4) {
        data[i  ] = data_data[i+2];
        data[i+1] = data_data[i+1];
        data[i+2] = data_data[i  ];
        data[i+3] = mask_data[i+0];
    }

    if (this->_replay) {
        this->_mouse_data.cursor_image = QImage(static_cast<uchar *>(data), cursor.x, cursor.y, QImage::Format_ARGB32_Premultiplied);

    } else {
        this->_screen[this->_current_screen_index]->set_mem_cursor(static_cast<uchar *>(data), cursor.x, cursor.y);

        if (this->_record) {
            this->_graph_capture->set_pointer(cursor);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
        }
    }
}

Screen_Qt * Front_Qt::getMainScreen() {
    return this->_screen[0];
}

Front_Qt::~Front_Qt() {
    this->empty_buffer();
    delete(this->_capture);

    this->fileSystemData.paths.clear();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------
//      CONTROLLERS
//------------------------

void Front_Qt::disconnexionReleased(){
    this->_replay = false;
    this->dropScreen();
    this->disconnect("");
    this->_cache = nullptr;
    this->_trans_cache = nullptr;
    delete(this->_capture);
    this->_capture = nullptr;
    this->_graph_capture = nullptr;
}

void Front_Qt::dropScreen() {
    for (auto screen : this->_screen) {
        if (screen != nullptr) {
            screen->disConnection();
            screen->close();
            screen = nullptr;
        }
    }
}

void Front_Qt::closeFromScreen(int screen_index) {

    if (this->fileSystemData.drives_created) {
        this->removeDriveDevice(fileSystemData.drives, fileSystemData.drivesCount);
    }

    for (auto screen : this->_screen) {
        if (screen != nullptr) {
            if (screen->_screen_index != screen_index) {
                screen->disConnection();
                screen->close();
                screen = nullptr;
            }
        }
    }

    delete(this->_capture);
    this->_capture = nullptr;
    this->_graph_capture = nullptr;

    if (this->_form != nullptr && this->_connected) {
        this->_form->close();
    }
}

void Front_Qt::setScreenDimension() {
    if (!this->_span) {
        this->_screen_dimensions[0].cx = this->_info.width;
        this->_screen_dimensions[0].cy = this->_info.height;

    } else {

        QDesktopWidget* desktop = QApplication::desktop();
        int screen_count(desktop->screenCount());
        if (this->_monitorCount > screen_count) {
            this->_monitorCount = screen_count;
        }
        this->_info.width  = 0;
        this->_info.height = 0;
        this->_info.cs_monitor.monitorCount = this->_monitorCount;

        for (int i = 0; i < this->_monitorCount; i++) {
            const QRect rect = desktop->screenGeometry(i);
            this->_screen_dimensions[i].x   = this->_info.width;
            this->_info.cs_monitor.monitorDefArray[i].left   = this->_info.width;
            this->_info.width  += rect.width();

            if (this->_info.height < rect.height()) {
                this->_info.height = rect.height();
            }
            this->_info.cs_monitor.monitorDefArray[i].top    = rect.top();
            this->_info.cs_monitor.monitorDefArray[i].right  = this->_info.width + rect.width() - 1;
            this->_info.cs_monitor.monitorDefArray[i].bottom = rect.height() - 1 - 3*BUTTON_HEIGHT;

            this->_info.cs_monitor.monitorDefArray[i].flags  = 0;

            this->_screen_dimensions[i].y   = 0;
            this->_screen_dimensions[i].cx  = rect.width();
            this->_screen_dimensions[i].cy  = rect.height() - 3*BUTTON_HEIGHT;
        }
        this->_info.cs_monitor.monitorDefArray[0].flags  = GCC::UserData::CSMonitor::TS_MONITOR_PRIMARY;
        this->_info.height -= 3*BUTTON_HEIGHT;
    }
}

bool Front_Qt::connect() {

    this->setScreenDimension();

    this->_clipbrdFormatsList.index = 0;
    this->_cl.clear_channels();

    if (this->_enable_shared_clipboard) {
        CHANNELS::ChannelDef channel_cliprdr { channel_names::cliprdr
                                            , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                              GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                              GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                            , CHANNELS::CHANNEL_CHUNK_LENGTH+1
                                            };
        this->_to_client_sender._channel = channel_cliprdr;
        this->_cl.push_back(channel_cliprdr);

        this->_clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS
                                            , this->_clipbrdFormatsList.FILECONTENTS
                                            );
        this->_clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW
                                            , this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW
                                            );
        this->_clipbrdFormatsList.add_format( RDPECLIP::CF_UNICODETEXT
                                            , std::string("\0\0", 2)
                                            );
        this->_clipbrdFormatsList.add_format( RDPECLIP::CF_TEXT
                                            , std::string("\0\0", 2)
                                            );
        this->_clipbrdFormatsList.add_format( RDPECLIP::CF_METAFILEPICT
                                            , std::string("\0\0", 2)
                                            );
    }

    if (this->_enable_shared_virtual_disk) {
        CHANNELS::ChannelDef channel_rdpdr{ channel_names::rdpdr
                                        , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                          GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                        , CHANNELS::CHANNEL_CHUNK_LENGTH+2
                                        };
        this->_cl.push_back(channel_rdpdr);
    }

    if (this->_mod_qt->connect()) {

        this->_cache = new QPixmap(this->_info.width, this->_info.height);
        this->_trans_cache = new QPixmap(this->_info.width, this->_info.height);
        this->_trans_cache->fill(Qt::transparent);
        this->_screen[0] = new Screen_Qt(this, this->_cache, this->_trans_cache);
        for (int i = 1; i < this->_monitorCount; i++) {
            this->_screen[i] = new Screen_Qt(this, i, this->_cache, this->_trans_cache);
            this->_screen[i]->show();
        }

        this->_replay = false;
        this->_connected = true;
        this->_form->hide();
        this->_screen[0]->show();

        if (this->_record && !this->_replay) {
            Inifile ini;
                ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
                ini.set<cfg::video::png_limit>(0);
                ini.set<cfg::video::disable_keyboard_log>(KeyboardLogFlags::none);
                ini.set<cfg::session_log::enable_session_log>(0);
                ini.set<cfg::session_log::keyboard_input_masking_level>(KeyboardInputMaskingLevel::unmasked);
                ini.set<cfg::context::pattern_kill>("");
                ini.set<cfg::context::pattern_notify>("");
                ini.set<cfg::debug::capture>(0xfffffff);
                ini.set<cfg::video::capture_groupid>(1);
                ini.set<cfg::video::record_tmp_path>(this->REPLAY_DIR);
                ini.set<cfg::video::record_path>(this->REPLAY_DIR);
                ini.set<cfg::video::hash_path>(this->REPLAY_DIR);
                time_t now;
                time(&now);
                std::string data(ctime(&now));
                std::string data_cut(data.c_str(), data.size()-1);
                std::string name("-Replay");
                std::string movie_name(data_cut+name);
                ini.set<cfg::globals::movie_path>(movie_name.c_str());
                ini.set<cfg::globals::trace_type>(TraceType::localfile);
                ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
                ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned, std::ratio<1, 100>>(6));

            LCGRandom gen(0);
            CryptoContext cctx;
            DummyAuthentifier * authentifier = nullptr;
            struct timeval time;
            gettimeofday(&time, nullptr);
            PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, true, authentifier, ini.get<cfg::video::record_tmp_path>().c_str(), "", 1};
            FlvParams flv_params = flv_params_from_ini(this->_info.width, this->_info.height, ini);
            OcrParams ocr_params = { ini.get<cfg::ocr::version>(),
                                     static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
                                     ini.get<cfg::ocr::on_title_bar_only>(),
                                     ini.get<cfg::ocr::max_unrecog_char_rate>(),
                                     ini.get<cfg::ocr::interval>()
                                   };

            const char * record_path = "/replay";

            WrmParams wrmParams(
                  this->_info.bpp
                , TraceType::localfile
                , cctx
                , gen
                , record_path
                , ini.get<cfg::video::hash_path>().c_str()
                , ""
                , ini.get<cfg::video::capture_groupid>()
                , std::chrono::duration<unsigned int, std::ratio<1l, 100l> >{60}
                , std::chrono::seconds{1}
                , WrmCompressionAlgorithm::no_compression
                , 0
              );

            PatternCheckerParams patternCheckerParams;
            SequencedVideoParams sequenced_video_params;
            FullVideoParams full_video_params;
            MetaParams meta_params;
            KbdLogParams kbd_log_params;

            this->_capture = new Capture( true, wrmParams
                                        , false, png_params
                                        , false, patternCheckerParams
                                        , false, ocr_params
                                        , false, sequenced_video_params
                                        , false, full_video_params
                                        , false, meta_params
                                        , false, kbd_log_params
                                        , ""
                                        , time
                                        , this->_info.width
                                        , this->_info.height
                                        , this->_info.bpp
                                        , this->_info.bpp
                                        , ini.get<cfg::video::record_tmp_path>().c_str()
                                        , ini.get<cfg::video::record_tmp_path>().c_str()
                                        , 1
                                        , flv_params
                                        , false
                                        , authentifier
                                        , nullptr
                                        , ""
                                        , ""
                                        , 0xfffffff
                                        , false
                                        , false
                                        , std::chrono::duration<long int>{60}
                                        , false
                                        , false
                                        , false
                                        , false
                                        , false
                                        , false
                                        );

            this->_graph_capture = this->_capture->get_graphic_api();
        }

        return this->_mod_qt->listen();
    }

    return false;
}

void Front_Qt::replay(std::string const & movie_path_) {
    if (movie_path_.empty()) {
        return;
    }
    auto const last_delimiter_it = std::find(movie_path_.rbegin(), movie_path_.rend(), '/');
    std::string const movie_path = (last_delimiter_it == movie_path_.rend())
      ? movie_path_
      : movie_path_.substr(movie_path_.size() - (last_delimiter_it - movie_path_.rbegin()));

    this->_replay = true;
    this->setScreenDimension();
    this->_cache_replay = new QPixmap(this->_info.width, this->_info.height);
    this->_trans_cache = new QPixmap(this->_info.width, this->_info.height);
    this->_trans_cache->fill(Qt::transparent);
    this->_screen[0] = new Screen_Qt(this, this->_cache_replay, movie_path, this->_trans_cache);
    for (int i = 1; i < this->_monitorCount; i++) {
        this->_screen[i] = new Screen_Qt(this, i, this->_cache_replay, this->_trans_cache);
        this->_screen[i]->show();
    }
    this->_connected = true;
    this->_form->hide();
    this->_screen[0]->show();

    this->load_replay_mod(movie_path);
}

void Front_Qt::delete_replay_mod() {
    this->_replay_mod.reset();
}

void Front_Qt::load_replay_mod(std::string const & movie_name) {
    this->_replay_mod.reset(new ReplayMod( *this
                                         , (this->REPLAY_DIR + "/").c_str()
                                         , movie_name.c_str()
                                         , 0
                                         , 0
                                         , this->_error
                                         , this->_font
                                         , true
                                         , to_verbose_flags(0)
                                         ));

    this->_replay_mod->add_consumer(nullptr, &this->snapshoter, nullptr, nullptr, nullptr);
}


void Front_Qt::disconnect(std::string const & error) {

    if (this->fileSystemData.drives_created) {
        this->removeDriveDevice(fileSystemData.drives, fileSystemData.drivesCount);
    }

    if (this->_mod_qt != nullptr) {
        this->_mod_qt->drop_connexion();
    }

    this->_monitorCountNegociated = false;

    this->_form->set_IPField(this->_targetIP);
    this->_form->set_portField(this->_port);
    this->_form->set_PWDField(this->_pwd);
    this->_form->set_userNameField(this->_userName);
    this->_form->set_ErrorMsg(error);
    this->_form->show();

    this->_connected = false;
}

bool Front_Qt::connexionReleased(){
    this->_form->setCursor(Qt::WaitCursor);
    this->_userName = this->_form->get_userNameField();
    this->_targetIP = this->_form->get_IPField();
    this->_pwd      = this->_form->get_PWDField();
    this->_port     = this->_form->get_portField();

    bool res(false);
    if (!this->_targetIP.empty()){
        res = this->connect();
    }
    this->_form->setCursor(Qt::ArrowCursor);
    return res;
}

void Front_Qt::mousePressEvent(QMouseEvent *e, int screen_shift) {
    if (this->_callback != nullptr) {
        int flag(0);
        switch (e->button()) {
            case Qt::LeftButton:  flag = MOUSE_FLAG_BUTTON1; break;
            case Qt::RightButton: flag = MOUSE_FLAG_BUTTON2; break;
            case Qt::MidButton:   flag = MOUSE_FLAG_BUTTON4; break;
            case Qt::XButton1:
            case Qt::XButton2:
            case Qt::NoButton:
            case Qt::MouseButtonMask:

            default: break;
        }
        //std::cout << "mousePressed " << e->x() << " " <<  e->y() << std::endl;
        this->_callback->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, e->x() + screen_shift, e->y(), &(this->_keymap));
    }
}

void Front_Qt::mouseReleaseEvent(QMouseEvent *e, int screen_shift) {
    if (this->_callback != nullptr) {
        int flag(0);
        switch (e->button()) {

            case Qt::LeftButton:  flag = MOUSE_FLAG_BUTTON1; break;
            case Qt::RightButton: flag = MOUSE_FLAG_BUTTON2; break;
            case Qt::MidButton:   flag = MOUSE_FLAG_BUTTON4; break;
            case Qt::XButton1:
            case Qt::XButton2:
            case Qt::NoButton:
            case Qt::MouseButtonMask:

            default: break;
        }
        //std::cout << "mouseRelease" << std::endl;
        this->_callback->rdp_input_mouse(flag, e->x() + screen_shift, e->y(), &(this->_keymap));
    }
}

void Front_Qt::keyPressEvent(QKeyEvent *e) {
    this->_qtRDPKeymap.keyEvent(0     , e);
    if (this->_qtRDPKeymap.scanCode != 0) {
        //std::cout << "keyPressed " << int(this->_qtRDPKeymap.scanCode) <<  std::endl;
        this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);

    }
}

void Front_Qt::keyReleaseEvent(QKeyEvent *e) {
    this->_qtRDPKeymap.keyEvent(KBD_FLAG_UP, e);
    if (this->_qtRDPKeymap.scanCode != 0) {
        this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);
    }
}

void Front_Qt::wheelEvent(QWheelEvent *e) {
    //std::cout << "wheel " << " delta=" << e->delta() << std::endl;
    int flag(MOUSE_FLAG_HWHEEL);
    if (e->delta() < 0) {
        flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
    }
    if (this->_callback != nullptr) {
        //this->_callback->rdp_input_mouse(flag, e->x(), e->y(), &(this->_keymap));
    }
}

bool Front_Qt::eventFilter(QObject *, QEvent *e, int screen_shift)  {
    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        //std::cout << "MouseMove " <<  mouseEvent->x() << " " <<  mouseEvent->y()<< std::endl;
        int x = mouseEvent->x() + screen_shift;
        int y = mouseEvent->y();

        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }

        if (y > this->_info.height) {
            this->_screen[this->_current_screen_index]->mouse_out = true;
        } else if (this->_screen[this->_current_screen_index]->mouse_out) {
            this->_screen[this->_current_screen_index]->update_current_cursor();
            this->_screen[this->_current_screen_index]->mouse_out = false;
        }

        if (this->_callback != nullptr) {
            this->_mouse_data.x = x;
            this->_mouse_data.y = y;
            this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->_keymap));
        }
    }
    return false;
}

void Front_Qt::connexionPressed() {}

void Front_Qt::RefreshPressed() {
    Rect rect(0, 0, this->_info.width * this->_monitorCount, this->_info.height);
    this->_callback->rdp_input_invalidate(rect);
}

void Front_Qt::CtrlAltDelPressed() {
    int flag = Keymap2::KBDFLAGS_EXTENDED;

    this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);     // ALT
    this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);     // CTRL
    this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);  // DELETE
}

void Front_Qt::CtrlAltDelReleased() {
    int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

    this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);     // ALT
    this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);     // CTRL
    this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);  // DELETE
}

void Front_Qt::disconnexionPressed() {}

void Front_Qt::send_rdp_scanCode(int keyCode, int flag) {
    Keymap2::DecodedKeys decoded_keys = this->_keymap.event(flag, keyCode, this->_ctrl_alt_delete);
    switch (decoded_keys.count)
    {
    case 2:
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[0]);
        }
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[1]);
        }
        break;
    case 1:
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[0]);
        }
        break;
    default:
    case 0:
        break;
    }
    if (this->_callback != nullptr) {
        this->_callback->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->_keymap));
    }
}

void Front_Qt::setMainScreenOnTopRelease() {
    this->_screen[0]->activateWindow();
}

void Front_Qt::recv_disconnect_provider_ultimatum() {
    LOG(LOG_INFO, "SERVER >> disconnect provider ultimatum");
    this->_recv_disconnect_ultimatum = true;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------
//   GRAPHIC FUNCTIONS (factorization)
//---------------------------------------

template<class Op>
void Front_Qt::draw_memblt_op(const Rect & drect, const Bitmap & bitmap) {
    const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
    const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

    if (mincx <= 0 || mincy <= 0) {
        return;
    }

    int rowYCoord(drect.y + drect.cy-1);

    QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
    QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
    QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

    if (bitmap.bpp() == 24) {
        srcBitmap = srcBitmap.rgbSwapped();
    }

    if (bitmap.bpp() != this->_info.bpp) {
        srcBitmap = srcBitmap.convertToFormat(this->_imageFormatRGB);
    }
    dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

    int indice(mincy-1);

    std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmap.bytesPerLine());

    for (size_t k = 0 ; k < mincy; k++) {

        const uchar * srcData = srcBitmap.constScanLine(k);
        const uchar * dstData = dstBitmap.constScanLine(indice - k);

        Op op;
        for (int i = 0; i < srcBitmap.bytesPerLine(); i++) {
             data[i] = op.op(srcData[i], dstData[i]);
        }

        QImage image(data.get(), mincx, 1, srcBitmap.format());
        QRect trect(drect.x, rowYCoord, mincx, 1);
        this->_screen[0]->paintCache().drawImage(trect, image);

        rowYCoord--;
    }
}

void Front_Qt::draw_MemBlt(const Rect & drect, const Bitmap & bitmap, bool invert, int srcx, int srcy) {
    const int16_t mincx = bitmap.cx();
    const int16_t mincy = bitmap.cy();

    if (mincx <= 0 || mincy <= 0) {
        return;
    }

    const unsigned char * row = bitmap.data();

    QImage qbitmap(row, mincx, mincy, this->bpp_to_QFormat(bitmap.bpp(), false));

    qbitmap = qbitmap.mirrored(false, true);

    qbitmap = qbitmap.copy(srcx, srcy, drect.cx, drect.cy);

    if (invert) {
        qbitmap.invertPixels();
    }

    if (bitmap.bpp() == 24) {
        qbitmap = qbitmap.rgbSwapped();
    }

    const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
    this->_screen[0]->paintCache().drawImage(trect, qbitmap);
}


void Front_Qt::draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert) {
    QImage qbitmap(this->_screen[0]->getCache()->toImage().copy(srcx, srcy, drect.cx, drect.cy));
    if (invert) {
        qbitmap.invertPixels();
    }
    const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
    this->_screen[0]->paintCache().drawImage(trect, qbitmap);
}


QColor Front_Qt::u32_to_qcolor(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    //std::cout <<  "r=" <<  int(r) <<  " g=" <<  int(g) << " b=" <<  int(b) <<  std::endl;
    return {r, g, b};
}

QColor Front_Qt::u32_to_qcolor_r(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    //std::cout <<  "r=" <<  int(r) <<  " g=" <<  int(g) << " b=" <<  int(b) <<  std::endl;
    return {b, g, r};
}


QImage::Format Front_Qt::bpp_to_QFormat(int bpp, bool alpha) {
    QImage::Format format(QImage::Format_RGB16);

    if (alpha) {

        switch (bpp) {
            case 15: format = QImage::Format_ARGB4444_Premultiplied; break;
            case 16: format = QImage::Format_ARGB4444_Premultiplied; break;
            case 24: format = QImage::Format_ARGB8565_Premultiplied; break;
            case 32: format = QImage::Format_ARGB32_Premultiplied;   break;
            default : break;
        }
    } else {

        switch (bpp) {
            case 15: format = QImage::Format_RGB555; break;
            case 16: format = QImage::Format_RGB16;  break;
            case 24: format = QImage::Format_RGB888; break;
            case 32: format = QImage::Format_RGB32;  break;
            default : break;
        }
    }

    return format;
}

void Front_Qt::draw_RDPPatBlt(const Rect & rect, const QColor color, const QPainter::CompositionMode mode, const Qt::BrushStyle style) {
    QBrush brush(color, style);
    this->_screen[0]->paintCache().setBrush(brush);
    this->_screen[0]->paintCache().setCompositionMode(mode);
    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
    this->_screen[0]->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
    this->_screen[0]->paintCache().setBrush(Qt::SolidPattern);
}

void Front_Qt::draw_RDPPatBlt(const Rect & rect, const QPainter::CompositionMode mode) {
    this->_screen[0]->paintCache().setCompositionMode(mode);
    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
    this->_screen[0]->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//-----------------------------
//       DRAW FUNCTIONS
//-----------------------------

void Front_Qt::draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    //std::cout << "RDPPatBlt " << std::hex << static_cast<int>(cmd.rop) << std::endl;
    RDPPatBlt new_cmd24 = cmd;
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
    new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
    const Rect rect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);

    QColor backColor = this->u32_to_qcolor(new_cmd24.back_color);
    QColor foreColor = this->u32_to_qcolor(new_cmd24.fore_color);

    if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) { // external

        switch (cmd.rop) {

            // +------+-------------------------------+
            // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
            // |      | RPN: DPx                      |
            // +------+-------------------------------+
            case 0x5A:
                {
                    QBrush brush(backColor, Qt::Dense4Pattern);
                    this->_screen[0]->paintCache().setBrush(brush);
                    this->_screen[0]->paintCache().setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    this->_screen[0]->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
                    this->_screen[0]->paintCache().setBrush(Qt::SolidPattern);
                }
                break;

            // +------+-------------------------------+
            // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
            // |      | RPN: P                        |
            // +------+-------------------------------+
            case 0xF0:
                {
                    QBrush brush(foreColor, Qt::Dense4Pattern);
                    this->_screen[0]->paintCache().setPen(Qt::NoPen);
                    this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                    this->_screen[0]->paintCache().setBrush(brush);
                    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    this->_screen[0]->paintCache().setBrush(Qt::SolidPattern);
                }
                break;
            default: LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", cmd.rop);
                break;
        }

    } else {
         switch (cmd.rop) {

            case 0x00: // blackness
                this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::black);
                break;
                // +------+-------------------------------+
                // | 0x05 | ROP: 0x000500A9               |
                // |      | RPN: DPon                     |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0x0F | ROP: 0x000F0001               |
                // |      | RPN: Pn                       |
                // +------+-------------------------------+
            case 0x0F:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSource);
                break;
                // +------+-------------------------------+
                // | 0x50 | ROP: 0x00500325               |
                // |      | RPN: PDna                     |
                // +------+-------------------------------+
            case 0x50:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceAndNotDestination);
                break;
                // +------+-------------------------------+
                // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
                // |      | RPN: Dn                       |
                // +------+-------------------------------+
            /*case 0x55:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotDestination);

                break;*/
                // +------+-------------------------------+
                // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
                // |      | RPN: DPx                      |
                // +------+-------------------------------+
            case 0x5A:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceXorDestination);
                break;
                // +------+-------------------------------+
                // | 0x5F | ROP: 0x005F00E9               |
                // |      | RPN: DPan                     |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xA0 | ROP: 0x00A000C9               |
                // |      | RPN: DPa                      |
                // +------+-------------------------------+
            case 0xA0:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceAndDestination);
                break;
                // +------+-------------------------------+
                // | 0xA5 | ROP: 0x00A50065               |
                // |      | RPN: PDxn                     |
                // +------+-------------------------------+
            /*case 0xA5:
                // this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceXorNotDestination);
                break;*/
                // +------+-------------------------------+
                // | 0xAA | ROP: 0x00AA0029               |
                // |      | RPN: D                        |
                // +------+-------------------------------+
            case 0xAA: // change nothing
                break;
                // +------+-------------------------------+
                // | 0xAF | ROP: 0x00AF0229               |
                // |      | RPN: DPno                     |
                // +------+-------------------------------+
            /*case 0xAF:
                //this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceOrDestination);
                break;*/
                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
            case 0xF0:
                this->_screen[0]->paintCache().setPen(Qt::NoPen);
                this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                break;
                // +------+-------------------------------+
                // | 0xF5 | ROP: 0x00F50225               |
                // |      | RPN: PDno                     |
                // +------+-------------------------------+
            //case 0xF5:
                //this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceOrNotDestination);
                //break;
                // +------+-------------------------------+
                // | 0xFA | ROP: 0x00FA0089               |
                // |      | RPN: DPo                      |
                // +------+-------------------------------+
            case 0xFA:
                this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceOrDestination);
                break;

            case 0xFF: // whiteness
                this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::white);
                break;
            default: LOG(LOG_WARNING, "RDPPatBlt rop = %x", cmd.rop);
                break;
        }
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->_info.bpp), &this->mod_palette));
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    //std::cout << "RDPOpaqueRect" << std::endl;
    RDPOpaqueRect new_cmd24 = cmd;
    new_cmd24.color = color_decode_opaquerect(cmd.color, this->_info.bpp, this->mod_palette);
    QColor qcolor(this->u32_to_qcolor(new_cmd24.color));
    Rect rect(new_cmd24.rect.intersect(clip));

    this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, qcolor);

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->_info.bpp), &this->mod_palette));
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        //bitmap_data.log(LOG_INFO, "FakeFront");
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPBitmapData" << std::endl;
    if (!bmp.is_valid()){
        return;
    }

    Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                            (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                            (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
    const Rect clipRect(0, 0, this->_info.width * this->_monitorCount, this->_info.height);
    const Rect drect = rectBmp.intersect(clipRect);

    const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
    const int16_t mincy = std::min<int16_t>(bmp.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));;

    if (mincx <= 0 || mincy <= 0) {
        return;
    }

    int rowYCoord(drect.y + drect.cy - 1);

    QImage::Format format(this->bpp_to_QFormat(bmp.bpp(), false)); //bpp
    QImage qbitmap(bmp.data(), mincx, mincy, bmp.line_size(), format);

    if (bmp.bpp() == 24) {
        qbitmap = qbitmap.rgbSwapped();
    }

    if (bmp.bpp() != this->_info.bpp) {
        qbitmap = qbitmap.convertToFormat(this->_imageFormatRGB);
    }

    for (size_t k = 0 ; k < drect.cy; k++) {

        QImage image(qbitmap.constScanLine(k), mincx, 1, qbitmap.format());
        QRect trect(drect.x, rowYCoord, mincx, 1);
        this->_screen[0]->paintCache().drawImage(trect, image);
        rowYCoord--;
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(bitmap_data, bmp);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    //std::cout << "RDPLineTo" << std::endl;
    RDPLineTo new_cmd24 = cmd;
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, 24, this->mod_palette);
    new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  24, this->mod_palette);

    // TODO clipping
    this->_screen[0]->setPenColor(this->u32_to_qcolor(new_cmd24.back_color));

    this->_screen[0]->paintCache().drawLine(new_cmd24.startx, new_cmd24.starty, new_cmd24.endx, new_cmd24.endy);

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->_info.bpp), &this->mod_palette));
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPScrBlt & cmd, Rect clip) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "RDPScrBlt" << std::endl;

    const Rect drect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);
    if (drect.isempty()) {
        return;
    }

    int srcx(drect.x + cmd.srcx - cmd.rect.x);
    int srcy(drect.y + cmd.srcy - cmd.rect.y);

    switch (cmd.rop) {

        case 0x00: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;

        case 0x55: this->draw_RDPScrBlt(srcx, srcy, drect, true);
            break;

        case 0xAA: // nothing to change
            break;

        case 0xCC: this->draw_RDPScrBlt(srcx, srcy, drect, false);
            break;

        case 0xFF:
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;
        default: LOG(LOG_WARNING, "DEFAULT: RDPScrBlt rop = %x", cmd.rop);
            break;
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPMemBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" <<  std::dec <<  std::endl;
    const Rect drect = clip.intersect(cmd.rect);
    if (drect.isempty()){
        return ;
    }

    switch (cmd.rop) {

        case 0x00: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;

        case 0x22: this->draw_memblt_op<Op_0x22>(drect, bitmap);
            break;

        case 0x33: this->draw_MemBlt(drect, bitmap, true, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
            break;

        case 0x55:
            this->draw_memblt_op<Op_0x55>(drect, bitmap);
            break;

        case 0x66: this->draw_memblt_op<Op_0x66>(drect, bitmap);
            break;

        case 0x99:  this->draw_memblt_op<Op_0x99>(drect, bitmap);
            break;

        case 0xAA:  // nothing to change
            break;

        case 0xBB: this->draw_memblt_op<Op_0xBB>(drect, bitmap);
            break;

        case 0xCC: this->draw_MemBlt(drect, bitmap, false, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
            break;

        case 0xEE: this->draw_memblt_op<Op_0xEE>(drect, bitmap);
            break;

        case 0x88: this->draw_memblt_op<Op_0x88>(drect, bitmap);
            break;

        case 0xFF: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;

        default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
            break;
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, bitmap);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    //std::cout << "RDPMem3Blt " << std::hex << int(cmd.rop) << std::dec <<  std::endl;
    const Rect drect = clip.intersect(cmd.rect);
    if (drect.isempty()){
        return ;
    }

    switch (cmd.rop) {
        case 0xB8:
            {
                const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width  - drect.x, drect.cx));
                const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

                if (mincx <= 0 || mincy <= 0) {
                    return;
                }
                uint32_t fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                const QColor fore(this->u32_to_qcolor(fore_color24));
                const uint8_t r(fore.red());
                const uint8_t g(fore.green());
                const uint8_t b(fore.blue());

                int rowYCoord(drect.y + drect.cy-1);
                const QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), true));

                QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));
                QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
                srcBitmap = srcBitmap.convertToFormat(QImage::Format_RGB888);
                dstBitmap = dstBitmap.convertToFormat(QImage::Format_RGB888);

                const size_t rowsize(srcBitmap.bytesPerLine());
                std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(rowsize);


                for (size_t k = 1 ; k < drect.cy; k++) {

                    const uchar * srcData = srcBitmap.constScanLine(k);
                    const uchar * dstData = dstBitmap.constScanLine(mincy - k);

                    for (size_t x = 0; x < rowsize-2; x += 3) {
                        data[x  ] = ((dstData[x  ] ^ r) & srcData[x  ]) ^ r;
                        data[x+1] = ((dstData[x+1] ^ g) & srcData[x+1]) ^ g;
                        data[x+2] = ((dstData[x+2] ^ b) & srcData[x+2]) ^ b;
                    }

                    QImage image(data.get(), mincx, 1, srcBitmap.format());
                    if (image.depth() != this->_info.bpp) {
                        image = image.convertToFormat(this->_imageFormatRGB);
                    }
                    QRect trect(drect.x, rowYCoord, mincx, 1);
                    this->_screen[0]->paintCache().drawImage(trect, image);
                    rowYCoord--;
                }
            }
        break;

        default: LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
        break;
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->_info.bpp), &this->mod_palette), bitmap);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}


void Front_Qt::draw(const RDPDestBlt & cmd, Rect clip) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "RDPDestBlt " <<  int(cmd.rop) <<  std::endl;

    const Rect drect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);

    switch (cmd.rop) {
        case 0x00: // blackness
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;
        case 0x55:                                         // inversion
            this->draw_RDPScrBlt(drect.x, drect.y, drect, true);
            break;
        case 0xAA: // change nothing
            break;
        case 0xFF: // whiteness
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;
        default: LOG(LOG_WARNING, "DEFAULT: RDPDestBlt rop = %x", cmd.rop);
            break;
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}

void Front_Qt::draw(const RDPMultiDstBlt & cmd, Rect clip) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    LOG(LOG_WARNING, "DEFAULT: RDPMultiDstBlt");
}

void Front_Qt::draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPMultiOpaqueRect");
}

void Front_Qt::draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPMultiPatBlt");
}

void Front_Qt::draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt");
}

void Front_Qt::draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    Rect screen_rect = clip.intersect(this->_info.width, this->_info.height);
    if (screen_rect.isempty()){
        return ;
    }

    Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
    if (clipped_glyph_fragment_rect.isempty()) {
        return;
    }
    //std::cout << "RDPGlyphIndex " << std::endl;

    // set a background color
    {
        /*Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
        if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
            ajusted.cy--;
            ajusted.intersect(screen_rect);
            this->_screen[0]->paintCache().fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, this->u32_to_qcolor(color_decode_opaquerect(cmd.fore_color, this->_info.bpp, this->mod_palette)));
        }*/
    }

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));

    const QColor color = this->u32_to_qcolor(color_decode_opaquerect(cmd.back_color, this->_info.bpp, this->mod_palette));
    const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
    const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

    uint16_t draw_pos = 0;

    InStream variable_bytes(cmd.data, cmd.data_len);

    //uint8_t const * fragment_begin_position = variable_bytes.get_current();

    while (variable_bytes.in_remain()) {
        uint8_t data = variable_bytes.in_uint8();

        if (data <= 0xFD) {
            FontChar const & fc = gly_cache.glyphs[cmd.cache_id][data].font_item;
            if (!fc)
            {
                LOG( LOG_INFO
                    , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
                    , cmd.cache_id, data);
                REDASSERT(fc);
            }

            if (has_delta_bytes)
            {
                data = variable_bytes.in_uint8();
                if (data == 0x80)
                {
                    draw_pos += variable_bytes.in_uint16_le();
                }
                else
                {
                    draw_pos += data;
                }
            }

            if (fc)
            {
                const int16_t x = draw_pos + cmd.bk.x + offset_x;
                const int16_t y = offset_y + cmd.bk.y;
                if (Rect(0,0,0,0) != clip.intersect(Rect(x, y, fc.incby, fc.height))){

                    const uint8_t * fc_data            = fc.data.get();
                    for (int yy = 0 ; yy < fc.height; yy++)
                    {
                        uint8_t   fc_bit_mask        = 128;
                        for (int xx = 0 ; xx < fc.width; xx++)
                        {
                            if (!fc_bit_mask)
                            {
                                fc_data++;
                                fc_bit_mask = 128;
                            }
                            if (clip.contains_pt(x + fc.offset + xx, y + fc.baseline + yy)
                            && (fc_bit_mask & *fc_data))
                            {
                                this->_screen[0]->paintCache().fillRect(x + fc.offset + xx, y + fc.baseline + yy, 1, 1, color);
                            }
                            fc_bit_mask >>= 1;
                        }
                        fc_data++;
                    }
                }
            }
        } else {
            LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache");
        }
    }
    //this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
        //draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
        //clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->_info.bpp), &this->mod_palette), gly_cache);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}

void Front_Qt::draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");

    /*RDPPolygonSC new_cmd24 = cmd;
    new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolygonCB");

    /*RDPPolygonCB new_cmd24 = cmd;
    new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
    new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
    /*RDPPolyline new_cmd24 = cmd;
    new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");

    /*RDPEllipseSC new_cmd24 = cmd;
    new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    (void) color_ctx;
    LOG(LOG_WARNING, "DEFAULT: RDPEllipseCB");
/*
    RDPEllipseCB new_cmd24 = cmd;
    new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDP::FrameMarker & order) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    if (this->_record && !this->_replay) {
        this->_graph_capture->draw(order);
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }

    LOG(LOG_INFO, "DEFAULT: FrameMarker");
}

// void Front_Qt::draw(const RDP::RAIL::NewOrExistingWindow & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_WARNING, "DEFAULT: NewOrExistingWindow");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::WindowIcon & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_WARNING, "DEFAULT: WindowIcon");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::CachedIcon & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_INFO, "DEFAULT: CachedIcon");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::DeletedWindow & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_INFO, "DEFAULT: DeletedWindow");
//
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_WARNING, "DEFAULT: NewOrExistingNotificationIcons");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::DeletedNotificationIcons & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_INFO, "DEFAULT: DeletedNotificationIcons");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_INFO, "DEFAULT: ActivelyMonitoredDesktop");
//     //this->gd.draw(order);
// }
//
// void Front_Qt::draw(const RDP::RAIL::NonMonitoredDesktop & order) {
//     if (this->verbose & RDPVerbose::graphics) {
//         LOG(LOG_INFO, "--------- FRONT ------------------------");
//         order.log(LOG_INFO);
//         LOG(LOG_INFO, "========================================\n");
//     }
//
//     LOG(LOG_WARNING, "DEFAULT: NonMonitoredDesktop");
//     //this->gd.draw(order);
// }

// void Front_Qt::draw(const RDPColCache   & cmd) {
//     LOG(LOG_WARNING, "DEFAULT: RDPColCache cacheIndex = %d", cmd.cacheIndex);
// }
//
// void Front_Qt::draw(const RDPBrushCache & brush) {
//     LOG(LOG_WARNING, "DEFAULT: RDPBrushCache cacheIndex = %d", brush.cacheIndex);
// }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------
// Serveur non drawing exchange
//------------------------------

FrontAPI::ResizeResult Front_Qt::server_resize(int width, int height, int bpp) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "resize serveur" << std::endl;
    this->mod_bpp = bpp;
    this->_info.bpp = bpp;
    this->_info.width = width;
    this->_info.height = height;

    return ResizeResult::done;
}

void Front_Qt::update_pointer_position(uint16_t xPos, uint16_t yPos) {
    //std::cout << "update_pointer_position " << int(xPos) << " " << int(yPos) << std::endl;

    if (this->_replay) {
        this->_trans_cache->fill(Qt::transparent);
        QRect nrect(xPos, yPos, this->_mouse_data.cursor_image.width(), this->_mouse_data.cursor_image.height());

        this->_screen[0]->paintTransCache().drawImage(nrect, this->_mouse_data.cursor_image);
    }
}

const CHANNELS::ChannelDefArray & Front_Qt::get_channel_list(void) const {
    return this->_cl;
}

void Front_Qt::send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) {
    if (this->verbose & RDPVerbose::graphics) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        LOG(LOG_INFO, "send_to_channel");
        LOG(LOG_INFO, "========================================\n");
    }

    const CHANNELS::ChannelDef * mod_channel = this->_cl.get_by_name(channel.name);
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
                    if (this->verbose & RDPVerbose::cliprdr) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Clipboard Capabilities PDU");
                    }
                break;

                case RDPECLIP::CB_MONITOR_READY:
                    if (this->verbose & RDPVerbose::cliprdr) {
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

                        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                            , chunk
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
                                                              |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                            );
                        if (this->verbose & RDPVerbose::cliprdr) {
                            LOG(LOG_INFO, "CLIENT >> CB Channel: Clipboard Capabilities PDU");
                        }

                        this->_monitorCount = this->_info.cs_monitor.monitorCount;

                        /*std::cout << "cs_monitor count negociated. MonitorCount=" << this->_monitorCount << std::endl;
                        std::cout << "width=" <<  this->_info.width <<  " " << "height=" << this->_info.height <<  std::endl;*/

                        //this->_info.width  = (this->_width * this->_monitorCount);

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
                        this->send_FormatListPDU(this->_clipbrdFormatsList.IDs, this->_clipbrdFormatsList.names, ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT);

                    }

                break;

                case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                    if (this->verbose & RDPVerbose::cliprdr) {
                    if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Format List Response PDU FAILED");
                    } else {
                        if (this->verbose & RDPVerbose::cliprdr) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Format List Response PDU");
                        }
                    }
                    }

                break;

                case RDPECLIP::CB_FORMAT_LIST:
                    {
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Format List PDU FAILED");
                        } else {
                            if (this->verbose & RDPVerbose::cliprdr) {
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
                                    if (this->_clipbrdFormatsList.IDs[j] == formatID) {
                                        this->_requestedFormatId = formatID;
                                        isSharedFormat = true;
                                        formatAvailable = 0;
                                    }
                                }

                                if (this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW && !isSharedFormat) {
                                    this->_requestedFormatId = formatID;
                                    isSharedFormat = true;
                                    formatAvailable = 0;
                                }
                            }

                            RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
                            StaticOutStream<256> out_stream;
                            formatListResponsePDU.emit(out_stream);
                            InStream chunk(out_stream.get_data(), out_stream.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk
                                                                , out_stream.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Format List Response PDU");
                            }


                            RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU(0);
                            StaticOutStream<32> out_stream_lock;
                            lockClipboardDataPDU.emit(out_stream_lock);
                            InStream chunk_lock(out_stream_lock.get_data(), out_stream_lock.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk_lock
                                                                , out_stream_lock.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Lock Clipboard Data PDU");
                            }

                            RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(this->_requestedFormatId);
                            StaticOutStream<256> out_streamRequest;
                            formatDataRequestPDU.emit(out_streamRequest);
                            InStream chunkRequest(out_streamRequest.get_data(), out_streamRequest.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunkRequest
                                                                , out_streamRequest.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Request PDU");
                            }
                        }
                    }
                break;

                case RDPECLIP::CB_LOCK_CLIPDATA:
                    if (this->verbose & RDPVerbose::cliprdr) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Lock Clipboard Data PDU");
                    }
                break;

                case RDPECLIP::CB_UNLOCK_CLIPDATA:
                    if (this->verbose & RDPVerbose::cliprdr) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Unlock Clipboard Data PDU");
                    }
                break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                    if(this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                        this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                    }

                    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Response PDU FAILED");
                        } else {
                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Response PDU");
                            }

                            this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList, this->_clipboard_qt);
                        }
                    }
                break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                {
                    if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                        LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Request PDU FAILED");
                    } else {
                        if (this->verbose & RDPVerbose::cliprdr) {
                            LOG(LOG_INFO, "SERVER >> CB Channel: Format Data Request PDU");
                        }

                        chunk.in_skip_bytes(4);
                        int first_part_data_size(0);
                        uint32_t total_length(this->_clipboard_qt->_cliboard_data_length + PDU_HEADER_SIZE);
                        StaticOutStream<PDU_MAX_SIZE> out_stream_first_part;

                        if (this->_clipboard_qt->_bufferTypeID == chunk.in_uint32_le()) {

                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "CLIENT >> CB Channel: Format Data Response PDU");
                            }

                            switch(this->_clipboard_qt->_bufferTypeID) {

                                case RDPECLIP::CF_METAFILEPICT:
                                {
                                    first_part_data_size = this->_clipboard_qt->_cliboard_data_length;
                                    if (first_part_data_size > PASTE_PIC_CONTENT_SIZE) {
                                        first_part_data_size = PASTE_PIC_CONTENT_SIZE;
                                    }
                                    total_length += RDPECLIP::METAFILE_HEADERS_SIZE;
                                    RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr( this->_clipboard_qt->_cliboard_data_length
                                                                                   , this->_clipboard_qt->_bufferImage->width()
                                                                                   , this->_clipboard_qt->_bufferImage->height()
                                                                                   , this->_clipboard_qt->_bufferImage->depth()
                                                                                   , this->_clipbrdFormatsList.ARBITRARY_SCALE
                                                                                   );
                                    fdr.emit(out_stream_first_part);

                                    this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                           , total_length
                                                                           , out_stream_first_part
                                                                           , first_part_data_size
                                                                           , this->_clipboard_qt->_chunk.get()
                                                                           , this->_clipboard_qt->_cliboard_data_length + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE
                                                                           , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                           );
                                }
                                break;

                                case RDPECLIP::CF_TEXT:
                                case RDPECLIP::CF_UNICODETEXT:
                                {
                                    first_part_data_size = this->_clipboard_qt->_cliboard_data_length;
                                    if (first_part_data_size > PASTE_TEXT_CONTENT_SIZE ) {
                                        first_part_data_size = PASTE_TEXT_CONTENT_SIZE;
                                    }

                                    RDPECLIP::FormatDataResponsePDU_Text fdr(this->_clipboard_qt->_cliboard_data_length);

                                    fdr.emit(out_stream_first_part);

                                    this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                           , total_length
                                                                           , out_stream_first_part
                                                                           , first_part_data_size
                                                                           , this->_clipboard_qt->_chunk.get()
                                                                           , this->_clipboard_qt->_cliboard_data_length
                                                                           , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                           );
                                }
                                break;

                                case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:
                                {
                                    int data_sent(0);
                                    first_part_data_size = PDU_HEADER_SIZE + 4;
                                    total_length = (RDPECLIP::FileDescriptor::size() * this->_clipboard_qt->_cItems) + 8 + PDU_HEADER_SIZE;
                                    int flag_first(CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                                    ClipBoard_Qt::CB_out_File * file = this->_clipboard_qt->_items_list[0];
                                    RDPECLIP::FormatDataResponsePDU_FileList fdr(this->_clipboard_qt->_cItems);
                                    fdr.emit(out_stream_first_part);

                                    RDPECLIP::FileDescriptor fdf( file->nameUTF8
                                                                , file->size
                                                                , fscc::FILE_ATTRIBUTE_ARCHIVE
                                                                );
                                    fdf.emit(out_stream_first_part);

                                    if (this->_clipboard_qt->_cItems == 1) {
                                        flag_first = flag_first | CHANNELS::CHANNEL_FLAG_LAST;
                                        out_stream_first_part.out_uint32_le(0);
                                        data_sent += 4;
                                    }
                                    InStream chunk_first_part( out_stream_first_part.get_data()
                                                             , out_stream_first_part.get_offset()
                                                             );

                                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                        , chunk_first_part
                                                                        , total_length
                                                                        , flag_first
                                                                        );
                                    data_sent += first_part_data_size + RDPECLIP::FileDescriptor::size();
                                    if (this->verbose & RDPVerbose::cliprdr) {
                                        LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%d", data_sent, total_length);
                                    }

                                    for (int i = 1; i < this->_clipboard_qt->_cItems; i++) {

                                        StaticOutStream<PDU_MAX_SIZE> out_stream_next_part;
                                        file = this->_clipboard_qt->_items_list[i];
                                        RDPECLIP::FileDescriptor fdn( file->nameUTF8
                                                                    , file->size
                                                                    , fscc::FILE_ATTRIBUTE_ARCHIVE);
                                        int flag_next(CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                                        fdn.emit(out_stream_next_part);

                                        if (i == this->_clipboard_qt->_cItems - 1) {
                                            flag_next = flag_next | CHANNELS::CHANNEL_FLAG_LAST;
                                            out_stream_next_part.out_uint32_le(0);
                                            data_sent += 4;
                                        }

                                        InStream chunk_next_part( out_stream_next_part.get_data()
                                                                , out_stream_next_part.get_offset()
                                                                );

                                        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                            , chunk_next_part
                                                                            , total_length
                                                                            , flag_next
                                                                            );

                                        data_sent += RDPECLIP::FileDescriptor::size();
                                        if (this->verbose & RDPVerbose::cliprdr) {
                                            LOG(LOG_INFO, "CLIENT >> CB Channel: Data PDU %d/%d", data_sent, total_length);
                                        }
                                    }
                                }
                                break;

                                default: LOG(LOG_WARNING, "SERVER >> CB Channel: unknow CB format ID %x", this->_clipboard_qt->_bufferTypeID);
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
                        if (this->verbose & RDPVerbose::cliprdr) {
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
                                                                            , this->_clipboard_qt->_items_list[lindex]->size);
                                fileSize.emit(out_stream);

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
                                this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                      CHANNELS::CHANNEL_FLAG_FIRST |  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                    );

                                if (this->verbose & RDPVerbose::cliprdr) {
                                    LOG(LOG_INFO, "CLIENT >> CB Channel: File Contents Response PDU SIZE");
                                }
                            }
                            break;

                            case RDPECLIP::FILECONTENTS_RANGE :
                            {
                                StaticOutStream<PDU_MAX_SIZE> out_stream_first_part;
                                RDPECLIP::FileContentsResponse_Range fileRange( streamID
                                                                              , this->_clipboard_qt->_items_list[lindex]->size);
                                this->_clipboard_qt->_cliboard_data_length = this->_clipboard_qt->_items_list[lindex]->size;
                                int total_length(this->_clipboard_qt->_items_list[lindex]->size + 12);
                                int first_part_data_size(this->_clipboard_qt->_items_list[lindex]->size);
                                first_part_data_size = this->_clipboard_qt->_items_list[lindex]->size;
                                if (first_part_data_size > PDU_MAX_SIZE - 12) {
                                    first_part_data_size = PDU_MAX_SIZE - 12;
                                }
                                fileRange.emit(out_stream_first_part);

                                this->process_client_clipboard_out_data( channel_names::cliprdr
                                                                       , total_length
                                                                       , out_stream_first_part
                                                                       , first_part_data_size
                                                                       , reinterpret_cast<uint8_t *>(
                                                                         this->_clipboard_qt->_items_list[lindex]->chunk)
                                                                       , this->_clipboard_qt->_items_list[lindex]->size
                                                                       , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                       );

                                if (this->verbose & RDPVerbose::cliprdr) {
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
                            LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU FAILED");
                        } else {
                            if (this->verbose & RDPVerbose::cliprdr) {
                                LOG(LOG_INFO, "SERVER >> CB Channel: File Contents Response PDU");
                            }

                            if(this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                                this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS;
                                this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList, this->_clipboard_qt);
                            }
                        }
                    }
                break;

                default:
                    if (this->verbose & RDPVerbose::cliprdr) {
                        LOG(LOG_INFO, "Process sever next part PDU data");
                    }
                    this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList, this->_clipboard_qt);
                break;
            }

        } else {
            if (this->verbose & RDPVerbose::cliprdr) {
                LOG(LOG_INFO, "Process sever next part PDU data");
            }
            this->process_server_clipboard_indata(flags, chunk_series, this->_cb_buffers, this->_cb_filesList, this->_clipboard_qt);
        }



    } else if (!strcmp(channel.name, channel_names::rdpdr)) {

        if (this->fileSystemData.writeData_to_wait) {

            size_t length(chunk.in_remain());

            this->fileSystemData.writeData_to_wait -= length;

            std::ofstream oFile(this->fileSystemData.paths[this->fileSystemData.file_to_write_id-1].c_str(), std::ios::out | std::ios::binary | std::ios::app);
            if (oFile.good()) {
                oFile.write(reinterpret_cast<const char *>(chunk.get_current()), length);
                oFile.close();
            }  else {
                LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", this->fileSystemData.paths[this->fileSystemData.file_to_write_id-1].c_str());
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
                        if (this->verbose & RDPVerbose::rdpdr)
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Announce Request");
                        //this->show_in_stream(0, chunk_series, chunk_size);

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

                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                            , chunk_to_send
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                            );
                        if (this->verbose & RDPVerbose::rdpdr)
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

                        rdpdr::ClientNameRequest clientNameRequest(username, 0x00000001);
                        clientNameRequest.emit(stream);

                        int total_length(stream.get_offset());
                        InStream chunk_to_send(stream.get_data(), stream.get_offset());

                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                            , chunk_to_send
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                            );
                        if (this->verbose & RDPVerbose::rdpdr)
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

                        if (this->verbose & RDPVerbose::rdpdr) {
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
                        if (this->verbose & RDPVerbose::rdpdr)
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
                        const uint32_t general_capability_version = rdpdr::GENERAL_CAPABILITY_VERSION_02;
                        out_stream.out_uint16_le(rdpdr::CAP_GENERAL_TYPE);
                        out_stream.out_uint16_le(36 + 8);
                                /*rdpdr::GeneralCapabilitySet::size(
                                    general_capability_version) +
                                8   // CapabilityType(2) + CapabilityLength(2) +
                                    //     Version(4)
                            );*/
                        out_stream.out_uint32_le(general_capability_version);

                        rdpdr::GeneralCapabilitySet general_capability_set(
                                0x2,        // osType
                                0x50001,    // osVersion
                                0x1,        // protocolMajorVersion
                                0x0002,        // protocolMinorVersion -
                                            //     RDP Client 6.0 and 6.1
                                0xFFFF,     // ioCode1
                                0x0,        // ioCode2
                                0x7,        // extendedPDU -
                                            //     RDPDR_DEVICE_REMOVE_PDUS(1) |
                                            //     RDPDR_CLIENT_DISPLAY_NAME_PDU(2) |
                                            //     RDPDR_USER_LOGGEDON_PDU(4)
                                0x1,        // extraFlags1
                                0x0,        // extraFlags2
                                0           // SpecialTypeDeviceCap
                            );

                        general_capability_set.emit(out_stream, general_capability_version);

                        rdpdr::CapabilityHeader ch1(rdpdr::CAP_PRINTER_TYPE, 8, rdpdr::PRINT_CAPABILITY_VERSION_01);
                        ch1.emit(out_stream);

                        rdpdr::CapabilityHeader ch2(rdpdr::CAP_PORT_TYPE, 8, rdpdr::PRINT_CAPABILITY_VERSION_01);
                        ch2.emit(out_stream);

                        rdpdr::CapabilityHeader ch3(rdpdr::CAP_DRIVE_TYPE, 8, rdpdr::PRINT_CAPABILITY_VERSION_01);
                        ch3.emit(out_stream);

                        rdpdr::CapabilityHeader ch4(rdpdr::CAP_SMARTCARD_TYPE, 8, rdpdr::PRINT_CAPABILITY_VERSION_01);
                        ch4.emit(out_stream);

                        int total_length(out_stream.get_offset());
                        InStream chunk_to_send(out_stream.get_data(), total_length);

                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                            , chunk_to_send
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                            );
                        if (this->verbose & RDPVerbose::rdpdr)
                            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Core Capability Response");
                        }

                        {
                        StaticOutStream<128> out_stream;
                        rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                                        , rdpdr::PacketId::PAKID_CORE_DEVICELIST_ANNOUNCE);
                        sharedHeader.emit(out_stream);

                        out_stream.out_uint32_le(this->fileSystemData.drivesCount);

                        for (size_t i = 0; i < this->fileSystemData.drivesCount; i++) {
                            out_stream.out_uint32_le(rdpdr::RDPDR_DTYP_FILESYSTEM);
                            out_stream.out_uint32_le(this->fileSystemData.drives[i].ID);
                            out_stream.out_copy_bytes(this->fileSystemData.drives[i].name, 8);
                            out_stream.out_uint32_le(0);
                        }

                        int total_length(out_stream.get_offset());
                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                            , chunk_to_send
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                            );
                        if (this->verbose & RDPVerbose::rdpdr)
                            LOG(LOG_INFO, "CLIENT >> RDPDR Channel: Client Device List Announce Request");
                        }
                        break;

                    case rdpdr::PAKID_CORE_DEVICE_REPLY:
                        {
                        int deviceID = chunk.in_uint32_le();
                        int resultCod = chunk.in_uint32_le();
                        this->fileSystemData.drives[deviceID - 1].status = resultCod;
                        if (resultCod == 0) {
                            this->fileSystemData.drives_created = true;
                        } else {
                            this->fileSystemData.drives_created = false;
                        }
                        if (this->verbose & RDPVerbose::rdpdr)
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server Device Announce Response ID=%x Hres=%x", deviceID, resultCod);
                        }
                        break;

                    case rdpdr::PAKID_CORE_USER_LOGGEDON:
                        if (this->verbose & RDPVerbose::rdpdr)
                            LOG(LOG_INFO, "SERVER >> RDPDR Channel: Server User Logged On");
                        break;

                    case rdpdr::PAKID_CORE_DEVICE_IOREQUEST:
                        {
                        rdpdr::DeviceIORequest deviceIORequest;
                        deviceIORequest.receive(chunk);

                        StaticOutStream<256> out_stream;
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

                                this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                      CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                            }
                                break;

                            case rdpdr::IRP_MJ_CREATE:
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Create Request");
                                {
                                rdpdr::DeviceCreateRequest request;
                                request.receive(chunk);

                                std::string new_path(this->SHARE_DIR + request.Path());

                                if (id == 0) {

                                    std::ifstream file(new_path.c_str());
                                    if (file.good()) {
                                        id = this->fileSystemData.get_file_id();
                                        this->fileSystemData.paths.push_back(new_path);
                                    } else {
                                        if (request.CreateDisposition() & smb2::FILE_CREATE) {

                                            id = this->fileSystemData.get_file_id();
                                            this->fileSystemData.paths.push_back(new_path);

                                            if (request.CreateOptions() & smb2::FILE_DIRECTORY_FILE) {
                                                LOG(LOG_WARNING, "new directory: \"%s\"", new_path);
                                                mkdir(new_path.c_str(), ACCESSPERMS);
                                            } else {
                                                LOG(LOG_WARNING, "new file: \"%s\"", new_path);
                                                std::ofstream oFile(new_path, std::ios::out | std::ios::binary);
                                                if (!oFile.good()) {
                                                    LOG(LOG_WARNING, "  Can't open create such file: \'%s\'.", new_path.c_str());
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                } else {
                                                    oFile.close();
                                                }
                                            }

                                        } else {
                                            LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", new_path.c_str());
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

                                this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                      CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Create Response");
                                }
                                break;

                            case rdpdr::IRP_MJ_QUERY_INFORMATION:
                                {
                                rdpdr::ServerDriveQueryInformationRequest sdqir;
                                sdqir.receive(chunk);

                                switch (sdqir.FsInformationClass()) {

                                    case rdpdr::FileBasicInformation:
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Basic Query Information Request");
                                        {

                                        std::ifstream file(this->fileSystemData.paths[id-1].c_str());
                                        if (!file.good()) {
                                            deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                            LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", this->fileSystemData.paths[id-1].c_str());
                                        }

                                        deviceIOResponse.emit(out_stream);

                                        rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_BASIC_INFORMATION_SIZE);
                                        cdqir.emit(out_stream);

                                        struct stat buff;
                                        stat(this->fileSystemData.paths[id-1].c_str(), &buff);

                                        uint64_t LastAccessTime = UnixSecondsToWindowsTick(buff.st_atime);
                                        uint64_t LastWriteTime  = UnixSecondsToWindowsTick(buff.st_mtime);
                                        uint64_t ChangeTime     = UnixSecondsToWindowsTick(buff.st_ctime);
                                        uint32_t FileAttributes = fscc::FILE_ATTRIBUTE_ARCHIVE;
                                        if (S_ISDIR(buff.st_mode)) {
                                            FileAttributes = fscc::FILE_ATTRIBUTE_DIRECTORY;
                                        }
                                        fscc::FileBasicInformation fileBasicInformation(LastWriteTime, LastAccessTime, LastWriteTime, ChangeTime, FileAttributes);

                                        fileBasicInformation.emit(out_stream);

                                        int total_length(out_stream.get_offset());
                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , total_length
                                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Basic Query Information Response");
                                        }
                                        break;

                                    case rdpdr::FileStandardInformation:
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Standard Information Request");
                                        {
                                        deviceIOResponse.emit(out_stream);

                                        rdpdr::ClientDriveQueryInformationResponse cdqir(rdpdr::FILE_STANDARD_INFORMATION_SIZE);
                                        cdqir.emit(out_stream);

                                        struct stat buff;
                                        stat(this->fileSystemData.paths[id-1].c_str(), &buff);

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

                                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Standard Information Response");
                                        }
                                        break;

                                    case rdpdr::FileAttributeTagInformation:
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query File Attribute Tag Information Request");
                                        {
                                            std::ifstream file(this->fileSystemData.paths[id-1].c_str());
                                            if (!file.good()) {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                                                LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\'.", this->fileSystemData.paths[id-1].c_str());
                                            }
                                            deviceIOResponse.emit(out_stream);

                                            struct stat buff;
                                            stat(this->fileSystemData.paths[id-1].c_str(), &buff);
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

                                            this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                                , chunk_to_send
                                                                                , out_stream.get_offset()
                                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                                  CHANNELS::CHANNEL_FLAG_FIRST
                                                                                );


                                            if (this->verbose & RDPVerbose::rdpdr)
                                                LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query File Attribute Tag Information Response");
                                        }
                                        break;

                                    default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request             unknow FsInformationClass = %x",       sdqir.FsInformationClass());
                                        break;
                                }
                                }
                                break;

                            case rdpdr::IRP_MJ_CLOSE:
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Close Request");
                                {
                                deviceIOResponse.emit(out_stream);

                                out_stream.out_uint32_le(0);

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                      CHANNELS::CHANNEL_FLAG_FIRST
                                                                    );
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Close Response");
                                }
                                break;

                            case rdpdr::IRP_MJ_READ:
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Read Request");
                                {
                                rdpdr::DeviceReadRequest drr;
                                drr.receive(chunk);

                                std::unique_ptr<uint8_t[]> ReadData;
                                int file_size(drr.Length());
                                int offset(drr.Offset());

                                std::ifstream ateFile(this->fileSystemData.paths[id-1], std::ios::binary| std::ios::ate);
                                if(ateFile.is_open()) {
                                    if (file_size > ateFile.tellg()) {
                                        file_size = ateFile.tellg();
                                    }
                                    ateFile.close();

                                    std::ifstream inFile(this->fileSystemData.paths[id-1], std::ios::in | std::ios::binary);
                                    if(inFile.is_open()) {
                                        ReadData = std::make_unique<uint8_t[]>(file_size+offset);
                                        inFile.read(reinterpret_cast<char *>(ReadData.get()), file_size+offset);
                                        inFile.close();
                                    } else {
                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", this->fileSystemData.paths[id-1].c_str());
                                    }
                                } else {
                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                    LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", this->fileSystemData.paths[id-1].c_str());
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
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Read Response");
                                }
                                break;

                            case rdpdr::IRP_MJ_DIRECTORY_CONTROL:

                                switch (deviceIORequest.MinorFunction()) {

                                    case rdpdr::IRP_MN_QUERY_DIRECTORY:
                                        if (this->verbose & RDPVerbose::rdpdr)
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
                                                LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                                            }

                                        } else {

                                            if (sdqdr.InitialQuery()) {
                                                this->fileSystemData.current_dir_id = id;
                                                this->fileSystemData.current_file_index = 0;
                                            }

                                            std::string str_dir_path;
                                            if (id <= this->fileSystemData.paths.size()) {
                                                str_dir_path = this->fileSystemData.paths[id-1];
                                            } else {
                                                LOG(LOG_WARNING, " Device I/O Query Directory Request Unknow ID (%d).", id);
                                                return;
                                            }

                                            if (str_dir_path.length() > 0) {
                                                std::string test = str_dir_path.substr(str_dir_path.length() -1, str_dir_path.length());
                                                if (test == slash) {
                                                    str_dir_path = str_dir_path.substr(0, str_dir_path.length()-1);
                                                }
                                            }

                                            DIR *dir;
                                            struct dirent *ent;
                                            std::string ignored1("..");
                                            std::string ignored2(".");

                                            if ((dir = opendir (str_dir_path.c_str())) != nullptr) {
                                                int i = 0;
                                                try {
                                                    while ((ent = readdir (dir)) != nullptr) {

                                                        std::string current_name = std::string (ent->d_name);
                                                        if (!(current_name == ignored1) && !(current_name == ignored2)) {

                                                            if (i == this->fileSystemData.current_file_index) {
                                                                this->fileSystemData.current_file_index++;
                                                                str_file_name = current_name;
                                                                break;
                                                            }
                                                            i++;
                                                        }
                                                    }
                                                } catch (Error & e) {
                                                    LOG(LOG_WARNING, "readdir error: (%d) %s", e.id, e.errmsg());
                                                }
                                                closedir (dir);
                                            } else {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_dir).", str_dir_path.c_str());
                                            }

                                            if (str_file_name.length() == 0) {
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_MORE_FILES);
                                            } else {
                                                std::string str_file_path_slash(str_dir_path + "/" + str_file_name);
                                                if (stat(str_file_path_slash.c_str(), &buff_child)) {
                                                    deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                                    LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (buff_child).", str_file_path_slash.c_str());
                                                } else {
                                                    LastAccessTime  = UnixSecondsToWindowsTick(buff_child.st_atime);
                                                    LastWriteTime   = UnixSecondsToWindowsTick(buff_child.st_mtime);
                                                    CreationTime    = LastWriteTime + 1;
                                                    EndOfFile       = buff_child.st_size;
                                                    AllocationSize  = buff_child.st_size;
                                                    if (S_ISDIR(buff_child.st_mode)) {
                                                        FileAttributes = fscc::FILE_ATTRIBUTE_DIRECTORY;
                                                    }

                                                }
                                            }
                                        }

                                        deviceIOResponse.emit(out_stream);

                                        switch (sdqdr.FsInformationClass()) {

                                            case rdpdr::FileDirectoryInformation:
                                            {
                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(
                                                    fscc::FileDirectoryInformation::MIN_SIZE
                                                  + (str_file_name.length()*2));
                                                cdqdr.emit(out_stream);

                                                fscc::FileDirectoryInformation fbdi(CreationTime,
                                                                                        LastAccessTime,
                                                                                        LastWriteTime,
                                                                                        ChangeTime,
                                                                                        EndOfFile,
                                                                                        AllocationSize,
                                                                                        FileAttributes,
                                                                                        str_file_name.c_str());
                                                fbdi.emit(out_stream);
                                            }
                                                break;
                                            case rdpdr::FileFullDirectoryInformation:
                                            {
                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(
                                                    fscc::FileFullDirectoryInformation::MIN_SIZE
                                                  + (str_file_name.length()*2));
                                                cdqdr.emit(out_stream);

                                                fscc::FileFullDirectoryInformation ffdi(CreationTime,
                                                                                        LastAccessTime,
                                                                                        LastWriteTime,
                                                                                        ChangeTime,
                                                                                        EndOfFile,
                                                                                        AllocationSize,
                                                                                        FileAttributes,
                                                                                        str_file_name.c_str());
                                                ffdi.emit(out_stream);
                                            }
                                                break;
                                            case rdpdr::FileBothDirectoryInformation:
                                            {
                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(
                                                    fscc::FileBothDirectoryInformation::MIN_SIZE
                                                  + (str_file_name.length()*2));
                                                cdqdr.emit(out_stream);

                                                fscc::FileBothDirectoryInformation fbdi(CreationTime, LastAccessTime, LastWriteTime, ChangeTime, EndOfFile, AllocationSize, FileAttributes, str_file_name.c_str());
                                                fbdi.emit(out_stream);
                                            }
                                                break;
                                            case rdpdr::FileNamesInformation:
                                            {
                                                rdpdr::ClientDriveQueryDirectoryResponse cdqdr(
                                                    fscc::FileNamesInformation::MIN_SIZE
                                                  + (str_file_name.length()*2));
                                                cdqdr.emit(out_stream);

                                                fscc::FileNamesInformation ffi(str_file_name.c_str());
                                                ffi.emit(out_stream);
                                            }
                                                break;
                                            default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: unknow  FsInformationClass = 0x%x", sdqdr.FsInformationClass());
                                                    break;
                                        }

                                        InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                        this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                            , chunk_to_send
                                                                            , out_stream.get_offset()
                                                                            , CHANNELS::CHANNEL_FLAG_LAST |
                                                                              CHANNELS::CHANNEL_FLAG_FIRST
                                                                            );
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Directory Response");
                                        }
                                        break;

                                    case rdpdr::IRP_MN_NOTIFY_CHANGE_DIRECTORY:
                                        if (this->verbose & RDPVerbose::rdpdr)
                                            LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Notify Change Directory Request");
                                        {
                                        //deviceIOResponse.emit(out_stream);
                                        }
                                        break;

                                    default: break;
                                }
                                break;

                            case rdpdr::IRP_MJ_QUERY_VOLUME_INFORMATION:
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Query Volume Information Request");
                                {
                                    rdpdr::ServerDriveQueryVolumeInformationRequest sdqvir;
                                    sdqvir.receive(chunk);

                                    uint64_t VolumeCreationTime             = 0;
                                    const char * VolumeLabel                = "";
                                    const char * FileSystemName             = "ext4";

                                    uint32_t FileSystemAttributes           = this->fileSystemData.newFileAttributes;
                                    uint32_t SectorsPerAllocationUnit       = 8;

                                    uint32_t BytesPerSector                 = 0;
                                    uint32_t MaximumComponentNameLength     = 0;
                                    uint64_t TotalAllocationUnits           = 0;
                                    uint64_t CallerAvailableAllocationUnits = 0;
                                    uint64_t AvailableAllocationUnits       = 0;
                                    uint64_t ActualAvailableAllocationUnits = 0;
                                    uint32_t VolumeSerialNumber             = 0;

                                    std::string str_path;
                                    if (id <= this->fileSystemData.paths.size()) {
                                        str_path = this->fileSystemData.paths[id-1];
                                    } else {
                                        LOG(LOG_WARNING, " Device I/O Query Volume Information Request Unknow ID (%d).", id);
                                        return;
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
                                        LOG(LOG_WARNING, "  Can't open such file or directory: \'%s\' (hd_driveid).", str_path.c_str());
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

                                    this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (this->verbose & RDPVerbose::rdpdr)
                                        LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Query Volume Information Response");

                                }
                                break;

                            case rdpdr::IRP_MJ_WRITE:
                                if (this->verbose & RDPVerbose::rdpdr)
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

                                    std::ofstream oFile(this->fileSystemData.paths[id-1].c_str(), std::ios::out | std::ios::binary);
                                    if (oFile.good()) {
                                        oFile.write(reinterpret_cast<const char *>(dwr.WriteData), WriteDataLen);
                                        oFile.close();
                                    }  else {
                                        LOG(LOG_WARNING, "  Can't open such file : \'%s\'.", this->fileSystemData.paths[id-1].c_str());
                                        deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_NO_SUCH_FILE);
                                    }

                                    deviceIOResponse.emit(out_stream);
                                    rdpdr::DeviceWriteResponse dwrp(dwr.Length);
                                    dwrp.emit(out_stream);

                                    InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());

                                    this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                            CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (this->verbose & RDPVerbose::rdpdr)
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Write Response");
                                }

                                break;

                            case rdpdr::IRP_MJ_SET_INFORMATION:
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Server Drive Set Information Request");
                                {
                                    rdpdr::ServerDriveSetInformationRequest sdsir;
                                    sdsir.receive(chunk);

                                    std::ifstream file(this->fileSystemData.paths[id-1].c_str(), std::ios::in |std::ios::binary);
                                    if (!file.good()) {
                                        LOG(LOG_WARNING, "  Can't open such file of directory : \'%s\'.", this->fileSystemData.paths[id-1].c_str());
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
                                            if (rename(this->fileSystemData.paths[id-1].c_str(), fileName.c_str()) !=  0) {
                                                LOG(LOG_WARNING, "  Can't rename such file of directory : \'%s\' to \'%s\'.", this->fileSystemData.paths[id-1].c_str(), fileName.c_str());
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

                                            if (remove(this->fileSystemData.paths[id-1].c_str()) != 0) {
                                                DeletePending = 0;
                                                deviceIOResponse.set_IoStatus(erref::NTSTATUS::STATUS_ACCESS_DENIED);
                                                LOG(LOG_WARNING, "  Can't delete such file of directory : \'%s\'.", this->fileSystemData.paths[id-1].c_str());
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

                                    this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                    if (this->verbose & RDPVerbose::rdpdr)
                                        LOG(LOG_INFO, "SERVER >> RDPDR: Device I/O Client Drive Set Information Response");
                                }

                                break;

                            case rdpdr::IRP_MJ_DEVICE_CONTROL:
                                if (this->verbose & RDPVerbose::rdpdr)
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

                                    this->_callback->send_to_mod_channel( channel_names::rdpdr
                                                                        , chunk_to_send
                                                                        , out_stream.get_offset()
                                                                        , CHANNELS::CHANNEL_FLAG_LAST |
                                                                          CHANNELS::CHANNEL_FLAG_FIRST
                                                                        );
                                }
                                if (this->verbose & RDPVerbose::rdpdr)
                                    LOG(LOG_INFO, "CLIENT >> RDPDR: Device I/O Client Drive Control Response");
                                break;

                            default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT: Device I/O Request unknow MajorFunction = %x",       deviceIORequest.MajorFunction());
                                break;
                        }

                        } break;

                    default: LOG(LOG_WARNING, "SERVER >> RDPDR Channel: DEFAULT RDPDR_CTYP_CORE unknow packetId = %x",       packetId);
                        break;
                }

            case rdpdr::Component::RDPDR_CTYP_PRT:
                //LOG(LOG_INFO, "SERVER >> RDPDR: RDPDR_CTYP_PRT");
                //this->show_in_stream(0, chunk_series, chunk_size);
                break;

            default: LOG(LOG_WARNING, "SERVER >> RDPDR: DEFAULT RDPDR unknow component = %x", component);
                break;
        }
    }
}


void Front_Qt::show_out_stream(int flags, OutStream & chunk, size_t length) {
    ::msgdump_c(false, false, length, flags,
                        chunk.get_data(), length);
}

void Front_Qt::show_in_stream(int flags, InStream & chunk, size_t length) {
    ::msgdump_c(false, false, length, flags,
                        chunk.get_data(), length);
}

void Front_Qt::process_server_clipboard_indata(int flags, InStream & chunk, CB_Buffers & cb_buffers, CB_FilesList & cb_filesList, ClipBoard_Qt * clipboard_qt) {

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

                this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                    , chunkRequest
                                                    , total_length_FormatDataRequestPDU
                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                      CHANNELS::CHANNEL_FLAG_FIRST |
                                                      CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                    );
                if (this->verbose & RDPVerbose::cliprdr) {
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

                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                        , chunk_unlock
                                                        , out_stream_unlock.get_offset()
                                                        , CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                        );
                    if (this->verbose & RDPVerbose::cliprdr) {
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

                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                        , chunkRequest
                                                        , total_length_FormatDataRequestPDU
                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                          CHANNELS::CHANNEL_FLAG_FIRST |
                                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                        );
                    if (this->verbose & RDPVerbose::cliprdr) {
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

void Front_Qt::removeDriveDevice(const FileSystemData::DeviceData * devices, const size_t deviceCount) {
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

    this->_callback->send_to_mod_channel( channel_names::rdpdr
                                        , chunk_to_send
                                        , total_length
                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                          CHANNELS::CHANNEL_FLAG_FIRST
                                        );
    if (this->verbose & RDPVerbose::rdpdr)
        LOG(LOG_INFO, "CLIENT >> RDPDR: Client Drive Device List Remove");
}

void Front_Qt::send_to_clipboard_Buffer(InStream & chunk) {

    // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Data

    const size_t length_of_data_to_dump(chunk.in_remain());
    const size_t sum_buffer_and_data(this->_cb_buffers.size + length_of_data_to_dump);
    const uint8_t * utf8_data = chunk.get_current();

    for (size_t i = 0; i < length_of_data_to_dump && i + this->_cb_buffers.size < this->_cb_buffers.sizeTotal; i++) {
        this->_cb_buffers.data[i + this->_cb_buffers.size] = utf8_data[i];
    }

    this->_cb_buffers.size = sum_buffer_and_data;
}

void Front_Qt::send_imageBuffer_to_clipboard() {

    QImage image(this->_cb_buffers.data.get(),
                 this->_cb_buffers.pic_width,
                 this->_cb_buffers.pic_height,
                 this->bpp_to_QFormat(this->_cb_buffers.pic_bpp, false));

    QImage imageSwapped(image.rgbSwapped().mirrored(false, true));

    this->_clipboard_qt->_local_clipboard_stream = false;
    this->_clipboard_qt->setClipboard_image(imageSwapped);
    this->_clipboard_qt->_local_clipboard_stream = true;

    this->empty_buffer();
}

void Front_Qt::send_textBuffer_to_clipboard() {
    std::unique_ptr<uint8_t[]> utf8_string = std::make_unique<uint8_t[]>(this->_cb_buffers.sizeTotal);
    size_t length_of_utf8_string = ::UTF16toUTF8(
        this->_cb_buffers.data.get(), this->_cb_buffers.sizeTotal,
        utf8_string.get(), this->_cb_buffers.sizeTotal);
    std::string str(reinterpret_cast<const char*>(utf8_string.get()), length_of_utf8_string);

    this->_clipboard_qt->_local_clipboard_stream = false;
    this->_clipboard_qt->setClipboard_text(str);
    this->_clipboard_qt->_local_clipboard_stream = true;

    this->empty_buffer();
}

void Front_Qt::empty_buffer() {
    this->_cb_buffers.pic_bpp    = 0;
    this->_cb_buffers.sizeTotal  = 0;
    this->_cb_buffers.pic_width  = 0;
    this->_cb_buffers.pic_height = 0;
    this->_cb_buffers.size       = 0;
    this->_waiting_for_data = false;
}

void Front_Qt::emptyLocalBuffer() {
    this->_clipboard_qt->emptyBuffer();
}

void Front_Qt::send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size) {

    StaticOutStream<1024> out_stream;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long(formatIDs, formatListDataShortName, formatIDs_size);
    format_list_pdu_long.emit(out_stream);
    const uint32_t total_length = out_stream.get_offset();
    InStream chunk(out_stream.get_data(), out_stream.get_offset());

    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunk
                                        , total_length
                                        , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                        );
    if (this->verbose & RDPVerbose::cliprdr) {
        LOG(LOG_INFO, "CLIENT >> CB channel: Format List PDU");
    }
}


void Front_Qt::process_client_clipboard_out_data(const char * const front_channel_name, const uint64_t total_length, OutStream & out_stream_first_part, const size_t first_part_data_size,  uint8_t const * data, const size_t data_len, uint32_t flags){

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

            this->_callback->send_to_mod_channel( front_channel_name
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

            this->_callback->send_to_mod_channel( front_channel_name
                                                , chunk_next
                                                , total_length
                                                , flags
                                                );

//             msgdump_c(false, false, total_length, 0, out_stream_next_part.get_data(), out_stream_next_part.get_offset());
        }

        // Last part
            StaticOutStream<PDU_MAX_SIZE> out_stream_last_part;
            out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);

            data_sent += remains_PDU;
            InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

            this->_callback->send_to_mod_channel( front_channel_name
                                                , chunk_last
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | flags
                                                );

//             msgdump_c(false, false, total_length, 0, out_stream_last_part.get_data(), out_stream_last_part.get_offset());

    } else {

        out_stream_first_part.out_copy_bytes(data, data_len);
        InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

        this->_callback->send_to_mod_channel( front_channel_name
                                            , chunk
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                              flags
                                            );
    }
}

void Front_Qt::begin_update() {
    //if (this->verbose & RDPVerbose::graphics) {
    //    LOG(LOG_INFO, "--------- FRONT ------------------------");
    //    LOG(LOG_INFO, "begin_update");
    //    LOG(LOG_INFO, "========================================\n");
    //}
}

void Front_Qt::end_update() {
    for (size_t i = 0; i < this->_info.cs_monitor.monitorCount; i++) {
        this->_screen[i]->update_view();
    }
    //if (this->verbose & RDPVerbose::graphics) {
    //    LOG(LOG_INFO, "--------- FRONT ------------------------");
    //    LOG(LOG_INFO, "end_update");
    //    LOG(LOG_INFO, "========================================\n");
    //}
    if (this->_record && !this->_replay) {
        this->_graph_capture->end_update();
        struct timeval time;
        gettimeofday(&time, nullptr);
        this->_capture->periodic_snapshot(time, this->_mouse_data.x, this->_mouse_data.y, false);
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------
//    SOCKET EVENTS FUNCTIONS
//--------------------------------

void Front_Qt::call_Draw() {
    if (this->_recv_disconnect_ultimatum) {
        this->dropScreen();
        std::string labelErrorMsg("<font color='Red'>Disconnected by server</font>");
        this->disconnect(labelErrorMsg);
        this->_cache = nullptr;
        this->_trans_cache = nullptr;
        delete(this->_capture);
        this->_capture = nullptr;
        this->_graph_capture = nullptr;
        this->_recv_disconnect_ultimatum = false;
    }
    if (this->_callback != nullptr && this->_cache != nullptr) {
        try {
            this->_callback->draw_event(time(nullptr), *(this));
        } catch (const Error &) {
            this->dropScreen();
            const std::string errorMsg("Error: connexion to [" + this->_targetIP +  "] is closed.");
            LOG(LOG_INFO, "%s", errorMsg.c_str());
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

            this->disconnect(labelErrorMsg);
        }
    }
}

/*
void log() {
        LOG(LOG_INFO, "     File Fs Volume Information:");
        LOG(LOG_INFO, "          * VolumeCreationTime = %" PRIx64 " (8 bytes)", this->VolumeCreationTime);
        LOG(LOG_INFO, "          * VolumeSerialNumber = %08x (4 bytes)", this->VolumeSerialNumber);
        LOG(LOG_INFO, "          * VolumeLabelLength = %d (4 bytes)", int(this->volume_label.size()));
        LOG(LOG_INFO, "          * SupportsObjects = %02x (1 byte)", this->SupportsObjects);
        LOG(LOG_INFO, "          * Padding - (1 byte) NOT USED");
        LOG(LOG_INFO, "          * VolumeLabel = \"%s\"", this->volume_label.c_str());
    }

hexdump_c(this->WriteData,  this->Length);
    void log() {

        LOG(LOG_INFO, "     File Directory Information:");
        LOG(LOG_INFO, "          * NextEntryOffset = 0x%08x (4 bytes)", this->NextEntryOffset);
        LOG(LOG_INFO, "          * FileIndex       = 0x%08x (4 bytes)", this->FileIndex);
        LOG(LOG_INFO, "          * CreationTime    = 0x%" PRIx64 " (8 bytes)", this->CreationTime);
        LOG(LOG_INFO, "          * LastAccessTime  = 0x%" PRIx64 " (8 bytes)", this->LastAccessTime_);
        LOG(LOG_INFO, "          * LastWriteTime   = 0x%" PRIx64 " (8 bytes)", this->LastWriteTime_);
        LOG(LOG_INFO, "          * ChangeTime      = 0x%" PRIx64 " (8 bytes)", this->ChangeTime);
        LOG(LOG_INFO, "          * FileAttributes  = 0x%08x (4 bytes)", this->FileAttributes_);
        LOG(LOG_INFO, "          * FileNameLength  = %d (4 bytes)", int(this->FileName.size()));
        LOG(LOG_INFO, "          * FileName        = \"%s\"", this->FileName.c_str());
    }

    const unsigned expected = 32;  // Padding(32)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated DeviceCloseRequest: expected=%u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }

            DeviceWriteResponse

    struct FileDispositionInformation {

    uint8_t DeletePending = 0;

    FileDispositionInformation() = default;

    FileDispositionInformation( uint64_t DeletePending)
      : DeletePending(DeletePending)
      {}

    void emit(OutStream & stream) {
        stream.out_uint8(this->DeletePending);
    }

    void receive(InStream & stream) {
        this->DeletePending = stream.in_uint8();
    }

    void log() {
        LOG(LOG_INFO, "     File Disposition Information:");
        LOG(LOG_INFO, "          * DeletePending = %02x (1 byte)", this->DeletePending);
    }
};
*/
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

    RDPVerbose verbose = RDPVerbose::none;       // RDPVerbose::rdpdr_dump | RDPVerbose::cliprdr;

    Front_Qt front_qt(argv, argc, verbose);


    app.exec();

    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2
}

