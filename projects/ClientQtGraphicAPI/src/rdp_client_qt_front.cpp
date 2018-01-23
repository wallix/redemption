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
#include "qt_input_output_api/qt_input_socket.hpp"

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
        CHANID_CLIPDRD = 1601,
        CHANID_RDPDR   = 1602,
        CHANID_WABDIAG = 1603,
        CHANID_RDPSND  = 1604,
        CHANID_RAIL    = 1605
    };


    // io API
    ClientIOClipboardAPI  * clientIOClipboardAPI;
    ClientOutputSoundAPI  * clientOutputSoundAPI;
    ClientInputSocketAPI * clientInputSocketAPI;


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

    std::string error_message;


    void options() override {
        new DialogOptions_Qt(this, this->form);
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

                    this->cache = new QPixmap(this->screen_max_width, this->screen_max_height);

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
                    ClientOutputSoundAPI  * clientOutputSoundAPI,
                    ClientIOClipboardAPI  * clientIOClipboardAPI,
                    ClientInputSocketAPI * clientInputSocketAPI)
        : Front_RDP_Qt_API(argv, argc, verbose)

        , clientIOClipboardAPI (new QtInputOutputClipboard(this, this->form))
        , clientOutputSoundAPI (new QtOutputSound(this->form))
        , clientInputSocketAPI(new QtInputSocket(this, this->form))

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



//         this->clientChannelRDPSNDManager.verbose = this->verbose;
//         this->clientChannelCLIPRDRManager.verbose = this->verbose;
//         this->clientChannelRDPDRManager.verbose = this->verbose;
//         this->clientChannelRemoteAppManager.verbose = this->verbose;


        this->client_execute.set_verbose(bool( (RDPVerbose::rail & this->verbose) | (RDPVerbose::rail_dump & this->verbose) ));



//         if (commandIsValid == COMMAND_VALID) {
//             this->connect();
//
//         } else {
//             std::cout << "Argument(s) required to connect: ";
//             if (!(commandIsValid & NAME_GOTTEN)) {
//                 std::cout << "-n [user_name] ";
//             }
//             if (!(commandIsValid & PWD_GOTTEN)) {
//                 std::cout << "-w [password] ";
//             }
//             if (!(commandIsValid & IP_GOTTEN)) {
//                 std::cout << "-i [ip_server] ";
//             }
//             if (!(commandIsValid & PORT_GOTTEN)) {
//                 std::cout << "-p [port] ";
//             }
//             std::cout << std::endl;
//
//             this->disconnect("");
//         }
    }

    ~RDPClientQtFront() {}



//     virtual void connect() override {
//
//     }



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    virtual void connect() override {

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

        this->is_pipe_ok = true;

        const char * name(this->user_name.c_str());
        const char * targetIP(this->target_IP.c_str());
        const std::string errorMsg("Cannot connect to [" + target_IP +  "].");

        unique_fd client_sck = ip_connect(targetIP,
                                          this->port,
                                          3,                //nbTry
                                          1000             //retryDelay
                                          );
        this->client_sck = client_sck.fd();

        if (this->client_sck > 0) {
            try {

                this->socket = new SocketTransport( name
                                                , std::move(client_sck)
                                                , targetIP
                                                , this->port
                                                , std::chrono::milliseconds(1000)
                                                , to_verbose_flags(0)
                                                //, SocketTransport::Verbose::dump
                                                , &this->error_message
                                                );

                LOG(LOG_INFO, "Connected to [%s].", targetIP);

            } catch (const std::exception &) {
                std::string windowErrorMsg(errorMsg+" Socket error.");
                LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
                this->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
                return;
            }

        } else {
            std::string windowErrorMsg(errorMsg+" Invalid ip or port.");
            LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
            this->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
            return;
        }

        this->qtRDPKeymap.setKeyboardLayout(this->info.keylayout);
//             this->cache = new QPixmap(this->info.width, this->info.height);
//             this->trans_cache = new QPixmap(this->info.width, this->info.height);
//             this->trans_cache->fill(Qt::transparent);

//         if (this->cache !=  nullptr) {
//             delete(this->cache);
//         }

        this->cache = new QPixmap(this->info.width, this->info.height);

        this->screen = new Screen_Qt(this, this->cache);

        this->is_replaying = false;
        if (this->is_recording && !this->is_replaying) {

//                 this->start_capture();

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
                ini.set<cfg::video::hash_path>(this->REPLAY_DIR+std::string("/signatures"));
                time_t now;
                time(&now);
                std::string data(ctime(&now));
                std::string data_cut(data.c_str(), data.size()-1);
                std::string name("-Replay");
                std::string movie_name(data_cut+name);
                ini.set<cfg::globals::movie_path>(movie_name.c_str());
                ini.set<cfg::globals::trace_type>(TraceType::localfile);
                ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
                ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned, std::ratio<1, 100>>(1));
                ini.set<cfg::video::break_interval>(std::chrono::seconds(600));

            UdevRandom gen;

            //NullReportMessage * reportMessage  = nullptr;
            struct timeval time;
            gettimeofday(&time, nullptr);
            PngParams png_params = {0, 0, ini.get<cfg::video::png_interval>(), 100, 0, true, this->info.remote_program, ini.get<cfg::video::rt_display>()};
            VideoParams videoParams = {Level::high, this->info.width, this->info.height, 0, 0, 0, std::string(""), true, true, false, ini.get<cfg::video::break_interval>(), 0};
            OcrParams ocr_params = { ini.get<cfg::ocr::version>(),
                                        static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
                                        ini.get<cfg::ocr::on_title_bar_only>(),
                                        ini.get<cfg::ocr::max_unrecog_char_rate>(),
                                        ini.get<cfg::ocr::interval>(),
                                        0
                                    };

            std::string record_path = this->REPLAY_DIR.c_str() + std::string("/");



            WrmParams wrmParams(
                    this->info.bpp
                , this->cctx
                , gen
                , this->fstat
                , ini.get<cfg::video::hash_path>().c_str()
                , std::chrono::duration<unsigned int, std::ratio<1l, 100l> >{60}
                , ini.get<cfg::video::break_interval>()
                , WrmCompressionAlgorithm::no_compression
                , 0
            );

            PatternParams patternCheckerParams {"", "", 0};
            SequencedVideoParams sequenced_video_params {};
            FullVideoParams full_video_params = { false };
            MetaParams meta_params {
                MetaParams::EnableSessionLog::No,
                MetaParams::HideNonPrintable::No
            };
            KbdLogParams kbd_log_params {false, false, false, false};

            CaptureParams captureParams;
            captureParams.now = tvtime();
            captureParams.basename = movie_name.c_str();
            captureParams.record_tmp_path = record_path.c_str();
            captureParams.record_path = record_path.c_str();
            captureParams.groupid = 0;
            captureParams.report_message = nullptr;

            DrawableParams drawableParams;
            drawableParams.width  = this->info.width;
            drawableParams.height = this->info.height;
            drawableParams.rdp_drawable = nullptr;

            this->capture = std::make_unique<Capture>(captureParams
                                            , drawableParams
                                            , true, wrmParams
                                            , false, png_params
                                            , false, patternCheckerParams
                                            , false, ocr_params
                                            , false, sequenced_video_params
                                            , false, full_video_params
                                            , false, meta_params
                                            , false, kbd_log_params
                                            , videoParams
                                            , nullptr
                                            , Rect(0, 0, 0, 0)
                                            );

            this->capture.get()->gd_drawable->width();

            this->graph_capture = this->capture.get()->get_graphic_api();
        }

        if (this->clientInputSocketAPI->start_to_listen(this->client_sck)) {
            this->form->hide();
            this->screen->show();
            this->connected = true;

        } else {
            this->connected = false;
        }
    }




    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //      CHANNELS FUNCTIONS
    //--------------------------------

    void send_clipboard_format() override {
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

//     virtual void disconnect(std::string const & error) override {
//
//
//         LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!! 1");
//
//         if (this->clientInputSocketAPI != nullptr) {
//             this->clientInputSocketAPI->disconnect();
//         }
//
//         if (this->mod != nullptr) {
//             TimeSystem timeobj;
//             if (this->is_pipe_ok) {
//                 this->mod->disconnect(timeobj.get_time().tv_sec);
//             };
//             this->mod = nullptr;
//         }
//
//         if (this->socket != nullptr) {
//             delete (this->socket);
//             this->socket = nullptr;
//             LOG(LOG_INFO, "Disconnected from [%s].", this->target_IP.c_str());
//         }
//
//         this->form->set_IPField(this->target_IP);
//         this->form->set_portField(this->port);
//         this->form->set_PWDField(this->user_password);
//         this->form->set_userNameField(this->user_name);
//         this->form->set_ErrorMsg(error);
//         this->form->show();
//
//         LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!! 4");
//
//         this->connected = false;
//     }

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

    RDPClientQtFront front_qt(argv, argc, verbose, nullptr, nullptr, nullptr);


    app.exec();

    // scp -P 22 -r cmoroldo@10.10.43.46:/home/cmoroldo/Bureau/redemption_test_charge/movie.wrm /home/qa/Desktop/movie_sample_data/


    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2
}
