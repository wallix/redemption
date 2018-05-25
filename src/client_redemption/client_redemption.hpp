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
   Author(s): Clément Moroldo, David Fort
*/


#pragma once


#include "utils/log.hpp"

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "configs/autogen/enums.hpp"
#include "core/session_reactor.hpp"

#include "capture/full_video_params.hpp"
#include "capture/video_params.hpp"
#include "capture/wrm_capture.hpp"
#include "core/RDP/RDPDrawable.hpp"

#include "mod/rdp/rdp.hpp"
#include "mod/vnc/vnc.hpp"

#include "transport/recorder_transport.hpp"
#include "transport/replay_transport.hpp"

#include "client_redemption/client_input_output_api.hpp"

#include "client_redemption/client_channel_managers/client_channel_RDPSND_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_CLIPRDR_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_RDPDR_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_remoteapp_manager.hpp"






class ClientRedemption : public ClientRedemptionIOAPI
{
    SessionReactor& session_reactor;
    SessionReactor::GraphicEventPtr clear_screen_event;

    std::unique_ptr<Transport> _socket_in_recorder;

    // io API
    ClientOutputGraphicAPI      * impl_graphic;
    ClientIOClipboardAPI        * impl_clipboard;
    ClientOutputSoundAPI        * impl_sound;
    ClientInputSocketAPI        * impl_socket_listener;
    ClientInputMouseKeyboardAPI * impl_mouse_keyboard;
    ClientIODiskAPI             * impl_io_disk;


    // RDP
    CHANNELS::ChannelDefArray   cl;
    std::string          _error;
    std::string   error_message;
    std::unique_ptr<Random> gen;
    std::array<uint8_t, 28> server_auto_reconnect_packet_ref;
    Inifile ini;
    std::string close_box_extra_message_ref;

    //  Remote App
    ClientExecute client_execute;

    std::unique_ptr<mod_api> unique_mod;

    enum : int {
        CHANID_CLIPDRD = 1601,
        CHANID_RDPDR   = 1602,
        CHANID_WABDIAG = 1603,
        CHANID_RDPSND  = 1604,
        CHANID_RAIL    = 1605
    };
        //  RDP Channel managers
    ClientChannelRDPSNDManager    clientChannelRDPSNDManager;
    ClientChannelCLIPRDRManager   clientChannelCLIPRDRManager;
    ClientChannelRDPDRManager     clientChannelRDPDRManager;
    ClientChannelRemoteAppManager clientChannelRemoteAppManager;


    // Replay Mod
    std::unique_ptr<ReplayMod> replay_mod;

    // Recorder
    Fstat fstat;

    struct Capture
    {
        RDPDrawable drawable;
        WrmCaptureImpl wrm_capture;

        Capture(
            const uint16_t width, const uint16_t height,
            CaptureParams const& capture_params, WrmParams const& wrm_params)
        : drawable(width, height)
        , wrm_capture(capture_params, wrm_params, this->drawable)
        {}
    };
    std::unique_ptr<Capture>  capture;

public:
    ClientRedemption(SessionReactor & session_reactor,
                     char* argv[], int argc, RDPVerbose verbose,
                     ClientOutputGraphicAPI * impl_graphic,
                     ClientIOClipboardAPI * impl_clipboard,
                     ClientOutputSoundAPI * impl_sound,
                     ClientInputSocketAPI * impl_socket_listener,
                     ClientInputMouseKeyboardAPI * impl_mouse_keyboard,
                     ClientIODiskAPI * impl_io_disk)
//         : ClientRedemptionIOAPI(argv, argc, verbose)
        : ClientRedemptionIOAPI(session_reactor, argv, argc, verbose)
        , session_reactor(session_reactor)
        , impl_graphic(impl_graphic)
        , impl_clipboard(impl_clipboard)
        , impl_sound (impl_sound)
        , impl_socket_listener (impl_socket_listener)
        , impl_mouse_keyboard(impl_mouse_keyboard)
        , impl_io_disk(impl_io_disk)

        , close_box_extra_message_ref("Close")
        , client_execute(session_reactor, *(this), this->info.window_list_caps, false)

        , clientChannelRDPSNDManager(this->verbose, this, this->impl_sound)
        , clientChannelCLIPRDRManager(this->verbose, this, this->impl_clipboard)
        , clientChannelRDPDRManager(this->verbose, this, this->impl_io_disk)
        , clientChannelRemoteAppManager(this->verbose, this, this->impl_graphic, this->impl_mouse_keyboard)
    {
        if (this->impl_clipboard) {
            this->impl_clipboard->set_client(this);
        } else {
            LOG(LOG_WARNING, "No clipoard IO implementation.");
        }
        if (this->impl_sound) {
            this->impl_sound->set_client(this);
            this->impl_sound->set_path(this->SOUND_TEMP_DIR);
        } else {
            LOG(LOG_WARNING, "No sound output implementation.");
        }
        if (this->impl_socket_listener) {
            this->impl_socket_listener->set_client(this);
        } else {
            LOG(LOG_WARNING, "No socket lister implementation.");
        }
        if (this->impl_mouse_keyboard) {
            this->impl_mouse_keyboard->set_client(this);
        } else {
            LOG(LOG_WARNING, "No keyboard and mouse input implementation.");
        }
        if (this->impl_graphic) {
            this->impl_graphic->set_drawn_client(this);
        } else {
            LOG(LOG_WARNING, "No graphic output implementation.");
        }

        this->client_execute.set_verbose(bool( (RDPVerbose::rail & this->verbose) | (RDPVerbose::rail_dump & this->verbose) ));

        this->disconnect("", false);

        if (this->commandIsValid == COMMAND_VALID) {
            this->connect();

        } else {
            std::cout << "Argument(s) required to connect: ";
            if (!(this->commandIsValid & NAME_GOT)) {
                std::cout << "-u [user_name] ";
            }
            if (!(this->commandIsValid & PWD_GOT)) {
                std::cout << "-p [password] ";
            }
            if (!(this->commandIsValid & IP_GOT)) {
                std::cout << "-i [ip_server] ";
            }
            if (!(this->commandIsValid & PORT_GOT)) {
                std::cout << "-P [port] ";
            }
            std::cout << std::endl;

            this->disconnect("", false);
        }
    }

    ~ClientRedemption() {}

    virtual void update_keylayout() override {
        if (this->impl_mouse_keyboard) {
            this->impl_mouse_keyboard->update_keylayout();
        }

        switch (this->mod_state) {
            case ClientRedemptionIOAPI::MOD_VNC:
                this->keymap.init_layout(this->vnc_conf.keylayout);
                break;

            default: this->keymap.init_layout(this->info.keylayout);
                break;

        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->cl;
    }

    void delete_replay_mod() override {
        this->replay_mod.reset();
    }

//     void writeWindowsConf() override {
//         this->windowsData.write();
//     }

    void closeFromScreen() override {
        if (this->impl_graphic) {
            this->impl_graphic->closeFromScreen();
        }
    }

    bool load_replay_mod(std::string const & movie_dir, std::string const & movie_name, timeval begin_read, timeval end_read) override {
         try {
            this->replay_mod.reset(new ReplayMod( session_reactor
                                                , *this
                                                , movie_dir.c_str() //(this->REPLAY_DIR + "/").c_str()
                                                , movie_name.c_str()
                                                , 0             //this->info.width
                                                , 0             //this->info.height
                                                , this->_error
                                                , true
                                                , begin_read
                                                , end_read
                                                , BALISED_FRAME
                                                , false
                                                //, FileToGraphic::Verbose::rdp_orders
                                                , to_verbose_flags(0)
                                                ));

            return true;

        } catch (const Error & err) {
            LOG(LOG_ERR, "new ReplayMod error %s", err.errmsg());
        }

        if (this->replay_mod.get() == nullptr) {
            if (impl_graphic) {
                this->impl_graphic->dropScreen();
            }
            const std::string errorMsg("Cannot read movie \""+movie_name+ "\".");
            LOG(LOG_INFO, "%s", errorMsg.c_str());
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->disconnect(labelErrorMsg, false);
        }
        return false;
    }

    void replay(const std::string & movie_name, const std::string & movie_dir) override {

        this->_movie_name = movie_name;
        this->_movie_dir = movie_dir;

        if (this->_movie_name.empty()) {
             //this->impl_graphic->readError(movie_path_);
            return;
        }

        this->is_replaying = true;
        this->is_loading_replay_mod = true;
        //this->setScreenDimension();
        if (this->load_replay_mod(this->_movie_dir, this->_movie_name, {0, 0}, {0, 0})) {

            this->is_loading_replay_mod = false;

            if (impl_graphic) {
                //LOG(LOG_INFO, "", this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
                //this->impl_graphic->reset_cache(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
                this->impl_graphic->create_screen(this->_movie_dir, this->_movie_name);
                if (this->replay_mod->get_wrm_version() == WrmVersion::v2) {
                    if (this->impl_mouse_keyboard) {
                        this->impl_mouse_keyboard->pre_load_movie();
                    }
                }
            }

            if (impl_graphic) {
                this->impl_graphic->show_screen();
            }
        }

        this->is_loading_replay_mod = false;
    }

    virtual void  disconnect(std::string const & error, bool pipe_broken) override {
        if (this->mod != nullptr) {
            if (!pipe_broken) {
                TimeSystem timeobj;
                this->mod->disconnect(timeobj.get_time().tv_sec);
            }
            this->mod = nullptr;
        }

        if (this->impl_socket_listener) {
            this->impl_socket_listener->disconnect();
        }

        if (this->socket != nullptr) {
            delete (this->socket);
            this->socket = nullptr;
            LOG(LOG_INFO, "Disconnected from [%s].", this->target_IP.c_str());
        }

        if (this->mod_state != MOD_RDP_REPLAY) {
            if (this->impl_graphic) {
                this->impl_graphic->set_ErrorMsg(error);
            }
        }
        if (this->impl_mouse_keyboard) {
            this->impl_mouse_keyboard->init_form();
        }
    }

    bool init_mod()  {

        try {
            this->mod = nullptr;

            switch (this->mod_state) {
                case MOD_RDP:
                {
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

                    this->unique_mod.reset(new mod_rdp( *this->socket
                                                , session_reactor
                                                , *this
                                                , this->info
                                                , ini.get_ref<cfg::mod_rdp::redir_info>()
                                                , *this->gen
                                                , this->timeSystem
                                                , mod_rdp_params
                                                , this->authentifier
                                                , this->reportMessage
                                                , this->ini
                                                ));
                }
                    break;

                case MOD_RDP_REMOTE_APP:
                {
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

                    this->client_execute.enable_remote_program(true);
                    mod_rdp_params.remote_program = true;
                    mod_rdp_params.client_execute = &(this->client_execute);
                    mod_rdp_params.remote_program_enhanced = INFO_HIDEF_RAIL_SUPPORTED != 0;
                    mod_rdp_params.use_client_provided_remoteapp = this->ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();
                    mod_rdp_params.use_session_probe_to_launch_remote_program = this->ini.get<cfg::context::use_session_probe_to_launch_remote_program>();

                    if (this->impl_graphic) {
                        this->info.width = this->impl_graphic->screen_max_width;
                        this->info.height = this->impl_graphic->screen_max_height;
                    }

                    this->unique_mod.reset(new mod_rdp( *(this->socket)
                                                , session_reactor
                                                , *(this)
                                                , this->info
                                                , ini.get_ref<cfg::mod_rdp::redir_info>()
                                                , *this->gen
                                                , this->timeSystem
                                                , mod_rdp_params
                                                , this->authentifier
                                                , this->reportMessage
                                                , this->ini
                                                ));

                    std::string target_info = this->ini.get<cfg::context::target_str>();
                    target_info += ":";
                    target_info += this->ini.get<cfg::globals::primary_user_id>();

                    this->client_execute.set_target_info(target_info.c_str());
                }
                    break;

            case MOD_VNC:
            {
                 this->unique_mod.reset(new mod_vnc( *this->socket
                                                    , session_reactor
                                                    , this->user_name.c_str()
                                                    , this->user_password.c_str()
                                                    , *this
                                                    , this->vnc_conf.width
                                                    , this->vnc_conf.height
                                                    , this->vnc_conf.keylayout
                                                    , 0
                                                    , true
                                                    , true
                                                    , this->vnc_conf.vnc_encodings.c_str()
                                                    , mod_vnc::ClipboardEncodingType::UTF8
                                                    , VncBogusClipboardInfiniteLoop::delayed
                                                    , this->reportMessage
                                                    , this->vnc_conf.is_apple
                                                    , &this->vnc_conf.exe
//                                                    , to_verbose_flags(0xfffffffd)
                                                    , to_verbose_flags(0)
                                                   )
                                        );
            }
                break;

            }

            this->mod = this->unique_mod.get();

            this->clear_screen_event = this->session_reactor.create_graphic_event(std::ref(*this))
            .on_action(jln::one_shot([](gdi::GraphicApi& gd, ClientRedemption& self){
                gdi_clear_screen(gd, self.mod->get_dim());
            }));
        } catch (const Error &) {
            this->mod = nullptr;
            return false;
        }

         return true;
    }

    bool init_socket() {
        if (this->is_full_replaying) {
            ReplayTransport *transport = new ReplayTransport(
                user_name.c_str(), full_capture_file_name,
                this->target_IP.c_str(), this->port,
                std::chrono::seconds(1), true);
            this->socket = transport;
            this->client_sck = transport->get_fd();
            return true;
        }

        unique_fd client_sck = ip_connect(this->target_IP.c_str(),
                                          this->port,
                                          3,                //nbTry
                                          1000             //retryDelay
                                          );
        this->client_sck = client_sck.fd();

        if (this->client_sck > 0) {
            try {
                this->socket = new SocketTransport( this->user_name.c_str()
                                            , std::move(client_sck)
                                            , this->target_IP.c_str()
                                            , this->port
                                            , std::chrono::milliseconds(1000)
                                            , to_verbose_flags(0)
                                            //, SocketTransport::Verbose::dump
                                            , &this->error_message
                                            );

                if (this->is_full_capturing) {
                    this->_socket_in_recorder.reset(this->socket);
                    this->socket = new RecorderTransport(
                        *this->socket, this->full_capture_file_name.c_str());
                }

                LOG(LOG_INFO, "Connected to [%s].", this->target_IP.c_str());

            } catch (const std::exception &) {
                const std::string errorMsg("Cannot connect to [" + target_IP +  "].");
                std::string windowErrorMsg(errorMsg+" Socket error.");
                LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
                this->disconnect("<font color='Red'>"+windowErrorMsg+"</font>", true);
                return false;
            }

        } else {
            const std::string errorMsg("Cannot connect to [" + target_IP +  "].");
            std::string windowErrorMsg(errorMsg+" Invalid IP or port.");
            LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
            this->disconnect("<font color='Red'>"+windowErrorMsg+"</font>", true);
            return false;
        }

        return true;
    }



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    virtual void connect() override {

        if (this->is_full_capturing || this->is_full_replaying) {
            gen.reset(new FixedRandom());
        } else {
            gen.reset(new UdevRandom());
        }

        this->clientChannelRemoteAppManager.clear();
        this->cl.clear_channels();

        this->is_replaying = false;
        if (this->is_recording) {
            this->set_capture();
        }

        if (this->mod_state != MOD_VNC) {

            if (this->mod_state == MOD_RDP_REMOTE_APP) {
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
//                 this->_to_client_sender._channel = channel_cliprdr;
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



        if (this->impl_graphic) {

            if (this->is_spanning) {
                this->rdp_width  = this->impl_graphic->screen_max_width;
                this->rdp_height = this->impl_graphic->screen_max_height;

                this->vnc_conf.width = this->impl_graphic->screen_max_width;
                this->vnc_conf.height = this->impl_graphic->screen_max_height;
            }

            switch (this->mod_state) {
                case MOD_RDP:
                    this->info.width = this->rdp_width;
                    this->info.height = this->rdp_height;
                    break;

                case MOD_VNC:
                    this->info.width = this->vnc_conf.width;
                    this->info.height = this->vnc_conf.height;
                    break;

                default: break;
            }

            if (this->mod_state != MOD_RDP_REMOTE_APP) {

                this->impl_graphic->reset_cache(this->info.width, this->info.height);
                this->impl_graphic->create_screen();
            } else {
                this->impl_graphic->reset_cache(this->impl_graphic->screen_max_width, this->impl_graphic->screen_max_height);
            }
        }

        if (this->init_socket()) {

            this->update_keylayout();

            if (this->init_mod()) {
                this->connected = true;

                if (this->impl_socket_listener) {
                    if (this->impl_socket_listener->start_to_listen(this->client_sck, this->mod)) {

                        if (mod_state != MOD_RDP_REMOTE_APP) {
                            if (this->impl_graphic) {
                                this->impl_graphic->show_screen();
                            }
                        }

                        return;
                    }
                }
            }

            const std::string errorMsgfail("Error: Mod Initialization failed.");
            std::string labelErrorMsg("<font color='Red'>"+errorMsgfail+"</font>");
            if (this->impl_graphic) {
                this->impl_graphic->dropScreen();
            }
            this->disconnect(labelErrorMsg, false);
        }
    }

    timeval reload_replay_mod(int begin, timeval now_stop) override {

        timeval movie_time_start;

        switch (this->replay_mod->get_wrm_version()) {

                case WrmVersion::v1:
                    if (this->load_replay_mod(this->_movie_dir, this->_movie_name, {0, 0}, {0, 0})) {
                        this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));
                        movie_time_start = tvtime();
                        return movie_time_start;
                    }
                    break;

                case WrmVersion::v2:
                {
                    int last_balised = (begin/ ClientRedemptionIOAPI::BALISED_FRAME);
                    this->is_loading_replay_mod = true;
                    if (this->load_replay_mod(this->_movie_dir, this->_movie_name, {last_balised * ClientRedemptionIOAPI::BALISED_FRAME, 0}, {0, 0})) {

                        this->is_loading_replay_mod = false;

                        this->draw_frame(last_balised);

                        this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));

                        if (this->impl_graphic) {
                            this->impl_graphic->update_screen();
                        }

                        movie_time_start = tvtime();
                        timeval waited_for_load = {movie_time_start.tv_sec - now_stop.tv_sec, movie_time_start.tv_usec - now_stop.tv_usec};
                        timeval wait_duration = {movie_time_start.tv_sec - begin - waited_for_load.tv_sec, movie_time_start.tv_usec - waited_for_load.tv_usec};
                        this->replay_mod->set_wait_after_load_client(wait_duration);
                    }
                    this->is_loading_replay_mod = false;

                    return movie_time_start;
                }
                    break;
        }

        return movie_time_start;
    }

    void replay_set_pause(timeval pause_duration) override {
        this->replay_mod->set_pause(pause_duration);
    }

    void replay_set_sync() override {
        this->replay_mod->set_sync();
    }

    bool is_replay_on() override {
        if (!this->replay_mod->get_break_privplay_client()) {
            if (!this->replay_mod->play_client()) {
                return true;
            }
        }
        return false;
    }

    time_t get_real_time_movie_begin() override {
        return this->replay_mod->get_real_time_movie_begin();
    }

    char const * get_mwrm_filename() override {
        this->_movie_full_path = this->_movie_dir + this->_movie_name;
        return _movie_full_path.c_str();
    }

    time_t get_movie_time_length(char const * mwrm_filename) override  {
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

    void instant_play_client(std::chrono::microseconds time) override {
        this->replay_mod.get()->instant_play_client(time);
    }

    void disconnexionReleased() override{
        this->is_replaying = false;
        this->connected = false;
        if (this->impl_graphic) {
            this->impl_graphic->dropScreen();
        }
        this->disconnect("", false);
    }

    virtual void set_capture() {
        std::string record_path = this->REPLAY_DIR + "/";
        std::string hash_path = this->REPLAY_DIR + "/signatures/";
        time_t now;
        time(&now);
        std::string movie_name = ctime(&now);
        movie_name.pop_back();
        movie_name += "-Replay";

        bool const is_remoteapp = false;
        WrmParams wrmParams(
              this->info.bpp
            , is_remoteapp
            , this->cctx
            , *this->gen
            , this->fstat
            , hash_path.c_str()
            , std::chrono::duration<unsigned int, std::ratio<1l, 100l> >{60}
            , std::chrono::seconds(600) /* break_interval */
            , WrmCompressionAlgorithm::no_compression
            , 0
        );

        CaptureParams captureParams;
        captureParams.now = tvtime();
        captureParams.basename = movie_name.c_str();
        captureParams.record_tmp_path = record_path.c_str();
        captureParams.record_path = record_path.c_str();
        captureParams.groupid = 0;
        captureParams.report_message = nullptr;

        this->capture = std::make_unique<Capture>(
            this->info.width, this->info.height,
            captureParams, wrmParams);

        //this->capture->gd_drawable->width();
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

            case CHANID_CLIPDRD: this->clientChannelCLIPRDRManager.receive(chunk, flags);
                break;

            case CHANID_RDPDR: this->clientChannelRDPDRManager.receive(chunk);
                break;

            case CHANID_RDPSND: this->clientChannelRDPSNDManager.receive(chunk);
                break;

            case CHANID_RAIL:    this->clientChannelRemoteAppManager.receive(chunk);
                break;
/*
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
                break;*/
        }
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop  & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::ActivelyMonitoredDesktop");
        }
        //cmd.log(LOG_INFO);

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::NewOrExistingWindow");
        }

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::DeletedWindow            & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::DeletedWindow");
        }
        //cmd.log(LOG_INFO);
        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::WindowIcon            & ) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::WindowIcon");
        }
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::CachedIcon            & ) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::CachedIcon");
        }
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons            & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::NewOrExistingNotificationIcons");
        }
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons            & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::DeletedNotificationIcons");
        }
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        if (bool(this->verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::NonMonitoredDesktop");
        }
        cmd.log(LOG_INFO);
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void callback(bool is_timeout) override {

//         LOG(LOG_INFO, "Socket Event callback");
//         if (this->_recv_disconnect_ultimatum) {
//             if (this->impl_graphic) {
//                 this->impl_graphic->dropScreen();
//             }
//             std::string labelErrorMsg("<font color='Red'>Disconnected by server</font>");
//             this->disconnect(labelErrorMsg, false);
//             this->capture = nullptr;
//             this->_recv_disconnect_ultimatum = false;
//         }

        if (this->mod != nullptr) {
            try {
                auto get_gd = [this]() -> gdi::GraphicApi& { return *this; };
                if (is_timeout) {
                    session_reactor.execute_timers(SessionReactor::EnableGraphics{true}, get_gd);
                } else {
                    auto is_mod_fd = [/*this*/](int /*fd*/, auto& /*e*/){
                        return true /*this->socket->get_fd() == fd*/;
                    };
                    session_reactor.execute_events(is_mod_fd);
                    session_reactor.execute_graphics(is_mod_fd, get_gd());
                }
            } catch (const Error & e) {
                if (this->impl_graphic) {
                    this->impl_graphic->dropScreen();
                }
                const std::string errorMsg("[" + this->target_IP +  "] lost: pipe broken");
                LOG(LOG_INFO, "%s: %s", errorMsg, e.errmsg());
                std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

                this->disconnect(labelErrorMsg, true);
            }
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using ClientRedemptionIOAPI::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }


    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }


    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        this->draw_impl(no_log{}, bitmap_data, bmp);
    }


    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }


    void draw(const RDPScrBlt & cmd, Rect clip) override {
        this->draw_impl(with_log{}, cmd, clip);
    }


    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        this->draw_impl(with_log{}, cmd, clip, bitmap);
    }


    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        this->draw_impl(with_log{}, cmd, clip, color_ctx, bitmap);
        /*if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }*/
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        this->draw_impl(with_log{}, cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        this->draw_impl(with_log{}, cmd, clip, color_ctx, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::FrameMarker& order) override {
        this->draw_impl(no_log{}, order);
    }

    void draw(RDPNineGrid const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        (void) cmd;
        (void) clip;
        (void) color_ctx;
        (void) bmp;
    }

    void set_pointer(Pointer const & cursor) override {
        if (this->impl_graphic) {
            this->impl_graphic->set_pointer(cursor);
        }
    }

    void draw_frame(int frame_index) override {
        this->impl_graphic->draw_frame(frame_index);
    }

//     void update_pointer_position(uint16_t xPos, uint16_t yPos) override {
//
//         if (this->is_replaying) {
// //             this->screen->_trans_cache.fill(Qt::transparent);
//             QRect nrect(xPos, yPos, this->cursor_image.width(), this->cursor_image.height());
//
// //             this->screen->paintTransCache().drawImage(nrect, this->cursor_image);
//         }
//     }

    ResizeResult server_resize(int width, int height, int bpp) override {
        LOG(LOG_INFO, "server_resize to (%d, %d, %d)", width, height, bpp);
        if (this->impl_graphic) {
            return this->impl_graphic->server_resize(width, height, bpp);
        }
        return ResizeResult::instant_done;
    }

    void begin_update() override {
        if ((this->connected || this->is_replaying)) {

            if (this->impl_graphic) {
                this->impl_graphic->begin_update();
            }

            if (this->is_recording && !this->is_replaying) {
                this->capture->drawable.begin_update();
                this->capture->wrm_capture.begin_update();
                this->capture->wrm_capture.periodic_snapshot(tvtime(), this->mouse_data.x, this->mouse_data.y, false);
            }
        }
    }


    void end_update() override {
        if ((this->connected || this->is_replaying)) {

            if (this->impl_graphic) {
                this->impl_graphic->end_update();
            }

            if (this->is_recording && !this->is_replaying) {
                this->capture->drawable.end_update();
                this->capture->wrm_capture.end_update();
                this->capture->wrm_capture.periodic_snapshot(tvtime(), this->mouse_data.x, this->mouse_data.y, false);
            }
        }
    }

    bool must_be_stop_capture() override {
        return false;
    }

private:
    using no_log = std::false_type;
    using with_log = std::true_type;

    void draw_impl(no_log, RDP::FrameMarker const& order)
    {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->is_recording && !this->is_replaying) {
            this->capture->drawable.draw(order);
            this->capture->wrm_capture.draw(order);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->mouse_data.x, this->mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class T, class... Ts>
    void draw_impl(WithLog with_log, Order& order, T& clip_or_bmp, Ts&... others)
    {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            if constexpr (with_log) {
                order.log(LOG_INFO, clip_or_bmp);
            }
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->impl_graphic) {
            this->impl_graphic->draw(order, clip_or_bmp, others...);
        }

        if (this->is_recording && !this->is_replaying) {
            this->capture->drawable.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->mouse_data.x, this->mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class... Ts>
    void draw_impl(WithLog with_log, Order& order, Rect clip, gdi::ColorCtx color_ctx, Ts&... others)
    {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            if constexpr (with_log) {
                order.log(LOG_INFO, clip);
            }
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->impl_graphic) {
            this->impl_graphic->draw(order, clip, color_ctx, others...);
        }

        if (this->is_recording && !this->is_replaying) {
            this->capture->drawable.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette), others...);
            this->capture->wrm_capture.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette), others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->mouse_data.x, this->mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class... Ts>
    void draw_unimplemented(WithLog with_log, Order& order, Rect clip, Ts&... /*others*/)
    {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            if constexpr (with_log) {
                order.log(LOG_INFO, clip);
            }
            LOG(LOG_INFO, "========================================\n");
        }
    }
};



    //" -name QA\\administrateur -pwd '' -ip 10.10.46.88 -p 3389";

    // sudo python ./sesman/sesmanlink/WABRDPAuthentifier

    // sudo nano /etc/rdpproxy/rdpproxy.ini

    // /etc/rdpproxy/cert

    // bjam san -j4 rdpproxy

    // sudo bin/gcc-4.9.2/san/rdpproxy -nf

    // sudo bin/gcc-4.9.2/release/link-static/rdpproxy -nf

    //bjam -s qt=4 debug client_rdp && bin/gcc-4.9.2/debug/threading-multi/client_rdp

    // sed '/usr\/include\/qt4\|threading-multi\/src\/Qt4\/\|in expansion of macro .*Q_OBJECT\|Wzero/,/\^/d' &&

    // ../../tools/c++-analyzer/bt  bin/gcc-4.9.2/debug/threading-multi/client_rdp

    // ../packager/packager.py --version 1.0.0 --no-entry-changelog --build-package

    // sudo dpkg -i /home/qa/Desktop/redemption_0.9.740bjessie_amd64.deb


// cxxflags=-DREDEMPTION_NO_FFMPEG


    // scp -P 22 -r cmoroldo@10.10.43.46:/home/cmoroldo/Bureau/redemption_test_charge/movie.wrm /home/qa/Desktop/movie_sample_data/


    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2

