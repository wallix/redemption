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
#include "utils/fixed_random.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/netutils.hpp"

#include "acl/auth_api.hpp"

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"

#include "mod/internal/replay_mod.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/vnc/new_mod_vnc.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/replay_transport.hpp"
#include "transport/socket_transport.hpp"

#include "capture/wrm_capture.hpp"

#include "client_redemption/client_channel_managers/client_channel_CLIPRDR_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_RDPDR_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_RDPSND_manager.hpp"
#include "client_redemption/client_channel_managers/client_channel_remoteapp_manager.hpp"

#include "client_redemption/client_config/client_redemption_config.hpp"

#include "client_redemption/client_input_output_api/client_keymap_api.hpp"
#include "client_redemption/client_input_output_api/client_socket_api.hpp"

#include "client_redemption/client_redemption_api.hpp"

#include "client_redemption/mod_wrapper/client_callback.hpp"
#include "client_redemption/mod_wrapper/client_channel_mod.hpp"

#include "configs/config.hpp"
#include "front/execute_events.hpp"



class ClientRedemption : public ClientRedemptionAPI
{

public:
    ClientRedemptionConfig config;

private:
    CryptoContext     cctx;

    std::unique_ptr<Transport> socket;
    int               client_sck;
    TimeSystem        timeSystem;
    NullAuthentifier  authentifier;
    NullReportMessage reportMessage;



public:
    ClientCallback _callback;
    ClientChannelMod channel_mod;
    SessionReactor& session_reactor;

    std::unique_ptr<Transport> _socket_in_recorder;
    std::unique_ptr<ReplayMod> replay_mod;
    // io API
    ClientOutputGraphicAPI      * impl_graphic;
    ClientIOClipboardAPI        * impl_clipboard;
    ClientOutputSoundAPI        * impl_sound;
    ClientInputSocketAPI        * impl_socket_listener;
    ClientKeyLayoutAPI          * impl_keylayout;
    ClientIODiskAPI             * impl_io_disk;


    // RDP
    CHANNELS::ChannelDefArray   cl;
    std::string          _error;
    std::string   error_message;
    std::unique_ptr<Random> gen;
    std::array<uint8_t, 28> server_auto_reconnect_packet_ref;
    Inifile ini;
    Theme theme;
    Font font;
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

    // Recorder
    Fstat fstat;

    timeval start_connection_time;                          // when socket is connected
    timeval start_wab_session_time;                         // when the first resize is received
    timeval start_win_session_time;                         // when the first memblt is received

    bool secondary_connection_finished;
    bool primary_connection_finished;

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

    struct WRMGraphicStat {

        int amount_RDPDestBlt = 0;
        long pixels_RDPDestBlt = 0;

        int amount_RDPMultiDstBlt = 0;
        long pixels_RDPMultiDstBlt = 0;

        int amount_RDPScrBlt = 0;
        long pixels_RDPScrBlt = 0;

        int amount_RDPMultiScrBlt = 0;
        long pixels_RDPMultiScrBlt = 0;

        int amount_RDPMemBlt = 0;
        long pixels_RDPMemBlt = 0;

        int amount_RDPBitmapData = 0;
        long pixels_RDPBitmapData = 0;

        int amount_RDPPatBlt= 0;
        long pixels_RDPPatBlt= 0;

        int amount_RDPMultiPatBlt = 0;
        long pixels_RDPMultiPatBlt = 0;

        int amount_RDPOpaqueRect = 0;
        long pixels_RDPOpaqueRect = 0;

        int amount_RDPMultiOpaqueRect = 0;
        long pixels_RDPMultiOpaqueRect = 0;

        int amount_RDPLineTo = 0;
        long pixels_RDPLineTo = 0;

        int amount_RDPPolygonSC = 0;
        long pixels_RDPPolygonSC = 0;

        int amount_RDPPolygonCB = 0;
        long pixels_RDPPolygonCB = 0;

        int amount_RDPPolyline = 0;
        long pixels_RDPPolyline = 0;

        int amount_RDPEllipseSC = 0;
        long pixels_RDPEllipseSC = 0;

        int amount_RDPEllipseCB = 0;
        long pixels_RDPEllipseCB = 0;

        int amount_RDPMem3Blt= 0;
        long pixels_RDPMem3Blt= 0;

        int amount_RDPGlyphIndex = 0;
        long pixels_RDPGlyphIndex = 0;

    } wrmGraphicStat;

    std::string       local_IP;



public:
    ClientRedemption(SessionReactor & session_reactor,
                     char const* argv[], int argc, RDPVerbose verbose,
                     ClientOutputGraphicAPI * impl_graphic,
                     ClientIOClipboardAPI * impl_clipboard,
                     ClientOutputSoundAPI * impl_sound,
                     ClientInputSocketAPI * impl_socket_listener,
                     ClientKeyLayoutAPI * impl_keylayout,
                     ClientIODiskAPI * impl_io_disk)
        : config(session_reactor, argv, argc, verbose, *(this))
        , client_sck(-1)
        , _callback(this, impl_keylayout)
        , session_reactor(session_reactor)
        , impl_graphic(impl_graphic)
        , impl_clipboard(impl_clipboard)
        , impl_sound (impl_sound)
        , impl_socket_listener (impl_socket_listener)
        , impl_keylayout(impl_keylayout)
        , impl_io_disk(impl_io_disk)
        , close_box_extra_message_ref("Close")
        , client_execute(session_reactor, *(this), this->config.info.window_list_caps, false)
        , clientChannelRDPSNDManager(this->config.verbose, &(this->channel_mod), this->impl_sound, this->config.rDPSoundConfig)
        , clientChannelCLIPRDRManager(this->config.verbose, &(this->channel_mod), this->impl_clipboard, this->config.rDPClipboardConfig)
        , clientChannelRDPDRManager(this->config.verbose, &(this->channel_mod), this->impl_io_disk, this->config.rDPDiskConfig)
        , clientChannelRemoteAppManager(this->config.verbose, &(this->_callback), &(this->channel_mod), this->impl_graphic)
        , start_win_session_time(tvtime())
        , secondary_connection_finished(false)
        , primary_connection_finished(false)
        , local_IP("unknow_local_IP")
    {
        SSL_load_error_strings();
        SSL_library_init();

        this->config.set_icon_movie_data();

        if (this->impl_clipboard) {
            this->impl_clipboard->set_client(this);
            this->impl_clipboard->set_path(this->config.CB_TEMP_DIR);
            this->impl_clipboard->set_manager(&(this->clientChannelCLIPRDRManager));
        } else {
            LOG(LOG_WARNING, "No clipoard IO implementation.");
        }
        if (this->impl_sound) {
            this->impl_sound->set_path(this->config.SOUND_TEMP_DIR);
        } else {
            LOG(LOG_WARNING, "No sound output implementation.");
        }
        if (this->impl_socket_listener) {
            this->impl_socket_listener->set_client(this);
        } else {
            LOG(LOG_WARNING, "No socket lister implementation.");
        }
        if (this->impl_graphic) {
            this->impl_graphic->set_drawn_client(&(this->_callback), &(this->config));
        } else {
            LOG(LOG_WARNING, "No graphic output implementation.");
        }

        this->client_execute.set_verbose(bool( (RDPVerbose::rail & this->config.verbose) | (RDPVerbose::rail_dump & this->config.verbose) ));

        if (this->config.connection_info_cmd_complete == ClientRedemptionConfig::COMMAND_VALID) {

           this->connect();

        } else {
            std::cout <<  "Argument(s) required for connection: ";
            if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::NAME_GOT)) {
                std::cout << "-u [user_name] ";
            }
            if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::PWD_GOT)) {
                std::cout << "-p [password] ";
            }
            if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::IP_GOT)) {
                std::cout << "-i [ip_server] ";
            }
            if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::PORT_GOT)) {
                std::cout << "-P [port] ";
            }
            std::cout << std::endl;

            if (this->impl_graphic) {
                this->impl_graphic->init_form();
            }
        }
    }

    ~ClientRedemption() = default;

   virtual bool is_connected() override {
        return this->config.connected;
   }

    int wait_and_draw_event(std::chrono::milliseconds timeout) override
    {
        if (ExecuteEventsResult::Error == execute_events(
            timeout, this->session_reactor, SessionReactor::EnableGraphics{true},
            *this->_callback.get_mod(), *this
        )) {
            LOG(LOG_ERR, "RDP CLIENT :: errno = %s\n", strerror(errno));
            return 9;
        }
        return 0;
    }

    void send_key_to_keep_alive() {
        if (this->config.keep_alive_freq) {
            std::chrono::microseconds duration = difftimeval(tvtime(), this->start_win_session_time);

            if ( ((duration.count() / 1000000) % this->config.keep_alive_freq) == 0) {
                this->_callback.send_rdp_scanCode(0x1e, KBD_FLAG_UP);
                this->_callback.send_rdp_scanCode(0x1e, 0);
            }
        }
    }

    virtual void update_keylayout() override {
        if (this->impl_keylayout) {
            this->impl_keylayout->update_keylayout(this->config.info.keylayout);

            this->impl_keylayout->clearCustomKeyCode();
            for (KeyCustomDefinition& key : this->config.keyCustomDefinitions) {
                this->impl_keylayout->setCustomKeyCode(key.qtKeyID, key.scanCode, key.ASCII8, key.extended);
            }
        }

        switch (this->config.mod_state) {
            case ClientRedemptionConfig::MOD_VNC:
                this->_callback.init_layout(this->config.vnc_conf.keylayout);
                break;

            default: this->_callback.init_layout(this->config.info.keylayout);
                break;
        }
    }

    const CHANNELS::ChannelDefArray & get_channel_list() const override {
        return this->cl;
    }

    void delete_replay_mod() override {
        this->replay_mod.reset();
    }

//     void writeWindowsConf() override {
//         this->windowsData.write();
//     }

    void closeFromGUI() override {
        if (this->impl_graphic) {
            this->impl_graphic->closeFromGUI();
        }
    }

    virtual void  disconnect(std::string const & error, bool pipe_broken) override {

        this->_callback.disconnect(this->timeSystem.get_time().tv_sec, pipe_broken);

        if (this->impl_socket_listener) {
            this->impl_socket_listener->disconnect();
        }

        if (!this->socket) {
            this->socket.reset();
        }

        std::chrono::microseconds duration = difftimeval(tvtime(), this->start_win_session_time);
        uint64_t movie_len = duration.count() / 1000;

        time_t now;
        time(&now);

        struct tm * timeinfo;
        char buffer [80];
        timeinfo = localtime (&now);
        strftime (buffer,80,"%F_%r",timeinfo);
        std::string date(buffer);

        std::cout << "Session duration = " << movie_len << " ms" << " " << date <<  std::endl;

        LOG(LOG_INFO, "Disconnected from [%s].", this->config.target_IP.c_str());

        if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REPLAY) {
            if (this->impl_graphic) {
                this->impl_graphic->set_ErrorMsg(error);
            }
        }
        this->config.set_icon_movie_data();
        if (this->impl_graphic) {
            this->impl_graphic->init_form();
        }
    }

    bool init_mod()  {

        try {
            this->_callback.init();

            switch (this->config.mod_state) {
            case ClientRedemptionConfig::MOD_RDP:
            case ClientRedemptionConfig::MOD_RDP_REMOTE_APP:
            {
                ModRDPParams mod_rdp_params(
                    this->config.user_name.c_str()
                  , this->config.user_password.c_str()
                  , this->config.target_IP.c_str()
                  , this->local_IP.c_str()
                  , 2
                  , this->font
                  , this->theme
                  , this->server_auto_reconnect_packet_ref
                  , this->close_box_extra_message_ref
                  , this->config.verbose
                  //, RDPVerbose::security | RDPVerbose::cache_persister | RDPVerbose::capabilities  | RDPVerbose::channels | RDPVerbose::connection
                  //, RDPVerbose::basic_trace | RDPVerbose::connection
                );

                mod_rdp_params.device_id                       = "device_id";
                mod_rdp_params.enable_tls                      = this->config.modRDPParamsData.enable_tls;
                mod_rdp_params.enable_nla                      = this->config.modRDPParamsData.enable_nla;
                mod_rdp_params.enable_fastpath                 = true;
                mod_rdp_params.enable_mem3blt                  = true;
                mod_rdp_params.enable_new_pointer              = true;
                mod_rdp_params.enable_glyph_cache              = true;
                mod_rdp_params.enable_ninegrid_bitmap          = true;
                std::string allow_channels                     = "*";
                mod_rdp_params.allow_channels                  = &allow_channels;
                mod_rdp_params.deny_channels = nullptr;
                mod_rdp_params.enable_rdpdr_data_analysis = false;

                const bool is_remote_app = this->config.mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP;

                if (is_remote_app) {
                    this->client_execute.enable_remote_program(true);
                    mod_rdp_params.remote_program = true;
                    mod_rdp_params.client_execute = &(this->client_execute);
                    mod_rdp_params.remote_program_enhanced = INFO_HIDEF_RAIL_SUPPORTED != 0;
                    mod_rdp_params.use_client_provided_remoteapp = this->ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();
                    mod_rdp_params.use_session_probe_to_launch_remote_program = this->ini.get<cfg::context::use_session_probe_to_launch_remote_program>();
                    this->config.info.cs_monitor = GCC::UserData::CSMonitor{};

                    if (this->impl_graphic) {
                        this->config.info.width = this->impl_graphic->screen_max_width;
                        this->config.info.height = this->impl_graphic->screen_max_height;
                    }

                    this->clientChannelRemoteAppManager.set_configuration(
                        this->config.info.width, this->config.info.height,
                        this->config.rDPRemoteAppConfig);
                }

                this->unique_mod = new_mod_rdp(
                    *this->socket
                  , session_reactor
                  , *this
                  , this->config.info
                  , ini.get_ref<cfg::mod_rdp::redir_info>()
                  , *this->gen
                  , this->timeSystem
                  , mod_rdp_params
                  , this->authentifier
                  , this->reportMessage
                  , this->ini
                  , nullptr
                );

                if (is_remote_app) {
                    std::string target_info = this->ini.get<cfg::context::target_str>();
                    target_info += ":";
                    target_info += this->ini.get<cfg::globals::primary_user_id>();

                    this->client_execute.set_target_info(target_info.c_str());
                }

                break;
            }

            case ClientRedemptionConfig::MOD_VNC:
                this->unique_mod = new_mod_vnc(
                    *this->socket
                  , this->session_reactor
                  , this->config.user_name.c_str()
                  , this->config.user_password.c_str()
                  , *this
                  , this->config.vnc_conf.width
                  , this->config.vnc_conf.height
                  , this->config.vnc_conf.keylayout
                  , 0
                  , true
                  , true
                  , this->config.vnc_conf.vnc_encodings.c_str()
                  , this->reportMessage
                  , this->config.vnc_conf.is_apple
                  , &this->config.vnc_conf.exe
                  , this->ini
                  // , to_verbose_flags(0xfffffffd)
                  , to_verbose_flags(0)
                  , nullptr
                );
                break;
            }

        } catch (const Error &) {
            this->_callback.init();
            return false;
        }

        this->_callback.set_mod(this->unique_mod.get());
        this->channel_mod.set_mod(this->unique_mod.get());

        return true;
    }

    bool init_socket() {
        if (this->config.is_full_replaying) {
            LOG(LOG_INFO, "Replay %s", this->config.full_capture_file_name);
            auto transport = std::make_unique<ReplayTransport>(
                this->config.full_capture_file_name.c_str(), this->config.target_IP.c_str(), this->config.port,
                this->timeSystem, ReplayTransport::FdType::Timer,
                ReplayTransport::FirstPacket::DisableTimer,
                ReplayTransport::UncheckedPacket::Send);
            this->client_sck = transport->get_fd();
            this->socket = std::move(transport);
            return true;
        }

        unique_fd unique_client_sck = ip_connect(this->config.target_IP.c_str(),
                                          this->config.port,
                                          3,                //nbTry
                                          1000             //retryDelay
                                          );

        this->client_sck = unique_client_sck.fd();

        bool has_error = false;
        std::string has_error_string;

        if (this->client_sck > 0) {
            try {
                this->socket = std::make_unique<SocketTransport>(
                    this->config.user_name.c_str(),
                    std::move(unique_client_sck),
                    this->config.target_IP.c_str(),
                    this->config.port,
                    std::chrono::seconds(1),
                    to_verbose_flags(0x0),
                    //SocketTransport::Verbose::dump,
                    &this->error_message);

                if (this->config.is_full_capturing) {
                    this->_socket_in_recorder = std::move(this->socket);
                    this->socket = std::make_unique<RecorderTransport>(
                        *this->_socket_in_recorder, this->timeSystem, this->config.full_capture_file_name.c_str());
                }

                LOG(LOG_INFO, "Connected to [%s].", this->config.target_IP.c_str());

            } catch (const Error& e) {
                has_error = true;
                has_error_string = e.errmsg();
            }
        } else {
            has_error = true;
        }

        if (has_error) {
            std::string errorMsg = "Cannot connect to [";
            errorMsg += this->config.target_IP;
            errorMsg += "]. Socket error: ";
            errorMsg += has_error_string;
            LOG(LOG_WARNING, "%s", errorMsg);
            this->disconnect("<font color='Red'>"+errorMsg+"</font>", true);
        }

        return !has_error;
    }



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    virtual bool connect() override {

        if (this->config.is_full_capturing || this->config.is_full_replaying) {
            this->gen = std::make_unique<FixedRandom>();
        } else {
            this->gen = std::make_unique<UdevRandom>();
        }

        this->clientChannelRemoteAppManager.clear();
        this->cl.clear_channels();

        this->config.is_replaying = false;
        if (this->config.is_recording) {
            this->set_capture();
        }

        if (this->config.mod_state != ClientRedemptionConfig::MOD_VNC) {

            if (this->config.mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {

                //this->config.info.remote_program |= INFO_RAIL;
                this->config.info.remote_program_enhanced |= INFO_HIDEF_RAIL_SUPPORTED;
                this->config.info.rail_caps.RailSupportLevel =   TS_RAIL_LEVEL_SUPPORTED
    //                                                     | TS_RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED
                                                        | TS_RAIL_LEVEL_SHELL_INTEGRATION_SUPPORTED
                                                        //| TS_RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED
                                                        | TS_RAIL_LEVEL_SERVER_TO_CLIENT_IME_SYNC_SUPPORTED
                                                        | TS_RAIL_LEVEL_HIDE_MINIMIZED_APPS_SUPPORTED
                                                        | TS_RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED
                                                        | TS_RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED;

                this->config.info.window_list_caps.WndSupportLevel = TS_WINDOW_LEVEL_SUPPORTED;
                this->config.info.window_list_caps.NumIconCaches = 3;  // 3;
                // 12;
                this->config.info.window_list_caps.NumIconCacheEntries = 12;

                CHANNELS::ChannelDef channel_rail { channel_names::rail
                                            , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                            , CHANID_RAIL
                                            };
                this->cl.push_back(channel_rail);

            } else {

                if (this->config.modRDPParamsData.enable_shared_virtual_disk && this->impl_io_disk) {
                    CHANNELS::ChannelDef channel_rdpdr{ channel_names::rdpdr
                                                    , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                        GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                                    , CHANID_RDPDR
                                                    };
                    this->cl.push_back(channel_rdpdr);

                    this->clientChannelRDPDRManager.set_share_dir(this->config.SHARE_DIR);
                }
            }

            if (this->config.enable_shared_clipboard && this->impl_clipboard) {
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

            if (this->config.modRDPParamsData.enable_sound && this->impl_sound) {
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

            if (this->config.is_spanning) {
                this->config.rdp_width  = this->impl_graphic->screen_max_width;
                this->config.rdp_height = this->impl_graphic->screen_max_height;

                this->config.vnc_conf.width = this->impl_graphic->screen_max_width;
                this->config.vnc_conf.height = this->impl_graphic->screen_max_height;
            }

            switch (this->config.mod_state) {
                case ClientRedemptionConfig::MOD_RDP:
                    this->config.info.width = this->config.rdp_width;
                    this->config.info.height = this->config.rdp_height;
                    break;

                case ClientRedemptionConfig::MOD_VNC:
                    this->config.info.width = this->config.vnc_conf.width;
                    this->config.info.height = this->config.vnc_conf.height;
                    break;

                default: break;
            }

            if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {

                this->impl_graphic->reset_cache(this->config.info.width, this->config.info.height);
                this->impl_graphic->create_screen();
            } else {
                this->impl_graphic->reset_cache(this->impl_graphic->screen_max_width, this->impl_graphic->screen_max_height);
            }
        }

        bool valid_socket_conn = this->init_socket();

        if (valid_socket_conn) {

            this->update_keylayout();

            this->config.connected = this->init_mod();

            if (this->config.connected) {

                if (this->impl_socket_listener) {

                    if (this->impl_socket_listener->start_to_listen(this->client_sck, this->_callback.get_mod())) {

                        this->start_wab_session_time = tvtime();

                        if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
                            if (this->impl_graphic) {
                                this->impl_graphic->show_screen();
                            }
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

     void record_connection_nego_times() {
        if (!this->secondary_connection_finished) {
            this->secondary_connection_finished = true;

            std::chrono::microseconds prim_duration = difftimeval(this->start_wab_session_time, this->start_connection_time);
            long prim_len = prim_duration.count() / 1000;
            std::cout << "primary connection length = " <<  prim_len << " ms\n";

            this->start_win_session_time = tvtime();

            std::chrono::microseconds sec_duration = difftimeval(this->start_win_session_time, this->start_wab_session_time);
            long sec_len = sec_duration.count() / 1000;
            time_t now;
            time(&now);

            struct tm * timeinfo;
            char buffer [80];
            timeinfo = localtime (&now);
            strftime (buffer,80,"%F_%r",timeinfo);
            std::string date(buffer);

            std::cout << "secondary connection length = " <<  sec_len << " ms " <<  date << "\n";
        }
    }


    // Replay


    void disconnexionReleased() override{
        this->config.is_replaying = false;
        this->config.connected = false;
        if (this->impl_graphic) {
            this->impl_graphic->dropScreen();
        }
        this->disconnect("", false);
    }

    virtual void set_capture() {
        std::string record_path = this->config.REPLAY_DIR + "/";
        std::string hash_path = this->config.REPLAY_DIR + "/signatures/";
        time_t now;
        time(&now);
        std::string movie_name = ctime(&now);
        movie_name.pop_back();
        movie_name += "-Replay";

        bool const is_remoteapp = false;
        WrmParams wrmParams(
              this->config.info.bpp
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
            this->config.info.width, this->config.info.height,
            captureParams, wrmParams);
    }


    bool load_replay_mod( std::string const & movie_path, timeval begin_read, timeval end_read) override {
         try {
            this->replay_mod = std::make_unique<ReplayMod>(
                this->session_reactor
              , *this
              , movie_path.c_str()
              , 0             //this->config.info.width
              , 0             //this->config.info.height
              , this->_error
              , true
              , begin_read
              , end_read
              , ClientRedemptionConfig::BALISED_FRAME
              , false
              //, FileToGraphic::Verbose::rdp_orders
              , to_verbose_flags(0)
            );

            this->_callback.set_replay(this->replay_mod.get());

            return true;

        } catch (const Error & err) {
            LOG(LOG_ERR, "new ReplayMod error %s", err.errmsg());
        }

        if (this->replay_mod == nullptr) {
            if (this->impl_graphic) {
                this->impl_graphic->dropScreen();
            }
            const std::string errorMsg("Cannot read movie \""+movie_path+ "\".");
            LOG(LOG_INFO, "%s", errorMsg.c_str());
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->disconnect(labelErrorMsg, false);
        }
        return false;
    }

    void replay( const std::string & movie_path) override {

//         this->config._movie_name = movie_name;
//         this->config._movie_dir = movie_dir;
        this->config._movie_full_path = movie_path;

        if (this->config._movie_name.empty()) {
             //this->impl_graphic->readError(movie_path_);
            return;
        }

        this->config.is_replaying = true;
        this->config.is_loading_replay_mod = true;

        //this->setScreenDimension();
        if (this->load_replay_mod(movie_path, {0, 0}, {0, 0})) {

            this->config.is_loading_replay_mod = false;

            if (impl_graphic) {
                this->impl_graphic->create_replay_screen();
                if (this->replay_mod->get_wrm_version() == WrmVersion::v2) {
                    this->impl_graphic->pre_load_movie(movie_path);
                    LOG(LOG_INFO, "amount_RDPDestBlt = %d pixels_RDPDestBlt = %ld", this->wrmGraphicStat.amount_RDPDestBlt, this->wrmGraphicStat.pixels_RDPDestBlt);
                        /*amount_RDPMultiDstBlt = %d pixels_RDPMultiDstBlt = %ld\n*/
                    LOG(LOG_INFO, "amount_RDPScrBlt = %d pixels_RDPScrBlt = %ld", this->wrmGraphicStat.amount_RDPScrBlt, this->wrmGraphicStat.pixels_RDPScrBlt);/* amount_RDPMultiScrBlt = %d pixels_RDPMultiScrBlt = %ld\n*/
                    LOG(LOG_INFO, "amount_RDPMemBlt = %d pixels_RDPMemBlt = %ld", this->wrmGraphicStat.amount_RDPMemBlt, this->wrmGraphicStat.pixels_RDPMemBlt);
                    LOG(LOG_INFO, "amount_RDPBitmapData = %d pixels_RDPBitmapData = %ld ", this->wrmGraphicStat.amount_RDPBitmapData, this->wrmGraphicStat.pixels_RDPBitmapData);
                    LOG(LOG_INFO, "amount_RDPPatBlt= %d pixels_RDPPatBlt= %ld", this->wrmGraphicStat.amount_RDPPatBlt, this->wrmGraphicStat.pixels_RDPPatBlt);
                    //LOG(LOG_INFO, "amount_RDPMultiPatBlt = %d pixels_RDPMultiPatBlt = %ld");
                    LOG(LOG_INFO, "amount_RDPOpaqueRect = %d pixels_RDPOpaqueRect = %ld", this->wrmGraphicStat.amount_RDPOpaqueRect, this->wrmGraphicStat.pixels_RDPOpaqueRect);
                    /*\n amount_RDPMultiOpaqueRect = %d pixels_RDPMultiOpaqueRect = %ld*/
                    LOG(LOG_INFO, "amount_RDPLineTo = %d pixels_RDPLineTo = %ld ", this->wrmGraphicStat.amount_RDPLineTo, this->wrmGraphicStat.pixels_RDPLineTo);
                    /*amount_RDPPolygonSC = %d pixels_RDPPolygonSC = %ld\n amount_RDPPolygonCB = %d pixels_RDPPolygonCB = %ld\n amount_RDPPolyline = %d pixels_RDPPolyline = %ld\n amount_RDPEllipseSC = %d pixels_RDPEllipseSC = %ld\n amount_RDPEllipseCB = %d pixels_RDPEllipseCB = %ld\n*/
                    LOG(LOG_INFO, "amount_RDPMem3Blt= %d pixels_RDPMem3Blt= %ld", this->wrmGraphicStat.amount_RDPMem3Blt, this->wrmGraphicStat.pixels_RDPMem3Blt);
                    LOG(LOG_INFO, "amount_RDPGlyphIndex = %d pixels_RDPGlyphIndex = %ld", this->wrmGraphicStat.amount_RDPGlyphIndex, this->wrmGraphicStat.pixels_RDPGlyphIndex);

//              this->wrmGraphicStat.amount_RDPMultiDstBlt,
//              this->wrmGraphicStat.pixels_RDPMultiDstBlt,

//              this->wrmGraphicStat.amount_RDPMultiScrBlt,
//              this->wrmGraphicStat.pixels_RDPMultiScrBlt,

//              this->wrmGraphicStat.amount_RDPMultiPatBlt,
//              this->wrmGraphicStat.pixels_RDPMultiPatBlt,

//              this->wrmGraphicStat.amount_RDPMultiOpaqueRect,
//              this->wrmGraphicStat.pixels_RDPMultiOpaqueRect,

//              this->wrmGraphicStat.amount_RDPPolygonSC,
//              this->wrmGraphicStat.pixels_RDPPolygonSC,
//
//              this->wrmGraphicStat.amount_RDPPolygonCB,
//              this->wrmGraphicStat.pixels_RDPPolygonCB,
//
//              this->wrmGraphicStat.amount_RDPPolyline,
//              this->wrmGraphicStat.pixels_RDPPolyline,
//
//              this->wrmGraphicStat.amount_RDPEllipseSC,
//              this->wrmGraphicStat.pixels_RDPEllipseSC,
//
//              this->wrmGraphicStat.amount_RDPEllipseCB,
//              this->wrmGraphicStat.pixels_RDPEllipseCB,
                }
                this->impl_graphic->show_screen();
            }
        }

        this->config.is_loading_replay_mod = false;
    }



 timeval reload_replay_mod(int begin, timeval now_stop) override {

        timeval movie_time_start;

        switch (this->replay_mod->get_wrm_version()) {

                case WrmVersion::v1:
                    if (this->load_replay_mod(this->config._movie_full_path, {0, 0}, {0, 0})) {
                        this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));
                        movie_time_start = tvtime();
                        return movie_time_start;
                    }
                    break;

                case WrmVersion::v2:
                {
                    int last_balised = (begin/ ClientRedemptionConfig::BALISED_FRAME);
                    this->config.is_loading_replay_mod = true;
                    if (this->load_replay_mod(this->config._movie_full_path, {last_balised * ClientRedemptionConfig::BALISED_FRAME, 0}, {0, 0})) {

                        this->config.is_loading_replay_mod = false;

                        if (this->impl_graphic) {
                            this->impl_graphic->draw_frame(last_balised);
                        }

                        this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));

                        if (this->impl_graphic) {
                            this->impl_graphic->update_screen();
                        }

                        movie_time_start = tvtime();
                        timeval waited_for_load = {movie_time_start.tv_sec - now_stop.tv_sec, movie_time_start.tv_usec - now_stop.tv_usec};
                        timeval wait_duration = {movie_time_start.tv_sec - begin - waited_for_load.tv_sec, movie_time_start.tv_usec - waited_for_load.tv_usec};
                        this->replay_mod->set_wait_after_load_client(wait_duration);
                    }
                    this->config.is_loading_replay_mod = false;

                    return movie_time_start;
                }
                    break;
        }

        return movie_time_start;
    }


    void instant_play_client(std::chrono::microseconds time) override {
        this->replay_mod->instant_play_client(time);
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //      CHANNELS FUNCTIONS
    //--------------------------------

    void send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t  /*unused*/, size_t chunk_size, int flags) override {

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
                std::string msg(char_ptr_cast(chunk.get_current()), len);

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
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::ActivelyMonitoredDesktop");
        }
        //cmd.log(LOG_INFO);

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            cmd.log(LOG_INFO);
//             LOG(LOG_INFO, "RDP::RAIL::NewOrExistingWindow");
        }

        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::DeletedWindow            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::DeletedWindow");
        }
        //cmd.log(LOG_INFO);
        this->clientChannelRemoteAppManager.draw(cmd);
    }

    void draw(const RDP::RAIL::WindowIcon            &  /*unused*/) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::WindowIcon");
        }
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::CachedIcon            &  /*unused*/) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::CachedIcon");
        }
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::NewOrExistingNotificationIcons");
        }
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::DeletedNotificationIcons");
        }
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            LOG(LOG_INFO, "RDP::RAIL::NonMonitoredDesktop");
        }
        cmd.log(LOG_INFO);
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    void callback(bool is_timeout) override {

//         if (this->mod != nullptr) {
            try {
                auto get_gd = [this]() -> gdi::GraphicApi& { return *this; };
                if (is_timeout) {
                    this->session_reactor.execute_timers(SessionReactor::EnableGraphics{true}, get_gd);
                } else {
                    auto is_mod_fd = [/*this*/](int /*fd*/, auto& /*e*/){
                        return true /*this->socket->get_fd() == fd*/;
                    };
                    this->session_reactor.execute_events(is_mod_fd);
                    this->session_reactor.execute_graphics(is_mod_fd, get_gd());
                }
            } catch (const Error & e) {
                if (this->impl_graphic) {
                    this->impl_graphic->dropScreen();
                }
                const std::string errorMsg("[" + this->config.target_IP +  "] lost: pipe broken");
                LOG(LOG_INFO, "%s: %s", errorMsg, e.errmsg());
                std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

                this->disconnect(labelErrorMsg, true);
            }
//         }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using ClientRedemptionAPI::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPPatBlt++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
                this->wrmGraphicStat.pixels_RDPPatBlt+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, color_ctx);
        }
    }


    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPOpaqueRect++;
                const Rect rect = cmd.rect.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
                this->wrmGraphicStat.pixels_RDPOpaqueRect+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, color_ctx);
        }
    }


    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPBitmapData++;
                Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                                    (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                                    (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
                const Rect rect = rectBmp.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
                this->wrmGraphicStat.pixels_RDPBitmapData+= rect.cx * rect.cy;
            }
            this->draw_impl(no_log{}, bitmap_data, bmp);
        }

        this->record_connection_nego_times();
    }


    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPLineTo++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
                this->wrmGraphicStat.pixels_RDPLineTo+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, color_ctx);
        }
    }


    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPScrBlt++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
                this->wrmGraphicStat.pixels_RDPScrBlt+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip);
        }
    }


    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPMemBlt++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
                this->wrmGraphicStat.pixels_RDPMemBlt+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, bitmap);
        }
        this->record_connection_nego_times();
    }


    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPMem3Blt++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
                this->wrmGraphicStat.pixels_RDPMem3Blt+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, color_ctx, bitmap);
        }
        /*if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }*/
    }

    void draw(const RDPDestBlt & cmd, Rect clip) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPDestBlt++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
                this->wrmGraphicStat.pixels_RDPDestBlt += rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPMultiDstBlt++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPMultiDstBlt+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPMultiOpaqueRect++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPMultiOpaqueRect+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPMultiPatBlt++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPMultiPatBlt+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPMultiScrBlt++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPMultiScrBlt+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        if (this->impl_graphic) {
            if (this->impl_graphic->is_pre_loading) {
                this->wrmGraphicStat.amount_RDPGlyphIndex++;
                const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
                this->wrmGraphicStat.pixels_RDPGlyphIndex+= rect.cx * rect.cy;
            }
            this->draw_impl(with_log{}, cmd, clip, color_ctx, gly_cache);
        }
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPPolygonSC++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPPolygonSC+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPPolygonCB++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPPolygonCB+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPPolyline++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPPolyline+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPEllipseSC++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPEllipseSC+= rect.cx * rect.cy;
//         }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
//         if (this->impl_graphic->is_pre_loading) {
//             this->wrmGraphicStat.amount_RDPEllipseCB++;
//             const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
//             this->wrmGraphicStat.pixels_RDPEllipseCB+= rect.cx * rect.cy;
//         }
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

//     void draw_frame(int frame_index) override {
//         if (this->impl_graphic) {
//             this->impl_graphic->draw_frame(frame_index);
//         }
//     }

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
        if ((this->config.connected || this->config.is_replaying)) {

            if (this->impl_graphic) {
                this->impl_graphic->begin_update();
            }

            if (this->config.is_recording && !this->config.is_replaying) {
                this->capture->drawable.begin_update();
                this->capture->wrm_capture.begin_update();
                this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
            }
        }
    }


    void end_update() override {
        if ((this->config.connected || this->config.is_replaying)) {

            if (this->impl_graphic) {
                this->impl_graphic->end_update();
            }

            if (this->config.is_recording && !this->config.is_replaying) {
                this->capture->drawable.end_update();
                this->capture->wrm_capture.end_update();
                this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
            }
        }
    }

    bool must_be_stop_capture() override {
        return false;
    }

private:
    using no_log = std::false_type;
    using with_log = std::true_type;

    void draw_impl(no_log /*unused*/, RDP::FrameMarker const& order)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->config.is_recording && !this->config.is_replaying) {
            this->capture->drawable.draw(order);
            this->capture->wrm_capture.draw(order);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class T, class... Ts>
    void draw_impl(WithLog with_log, Order& order, T& clip_or_bmp, Ts&... others)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip_or_bmp);
            }
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->impl_graphic) {
            this->impl_graphic->draw(order, clip_or_bmp, others...);
        }

        if (this->config.is_recording && !this->config.is_replaying) {
            this->capture->drawable.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class... Ts>
    void draw_impl(WithLog with_log, Order& order, Rect clip, gdi::ColorCtx color_ctx, Ts&... others)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip);
            }
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->impl_graphic) {
            this->impl_graphic->draw(order, clip, color_ctx, others...);
        }

        if (this->config.is_recording && !this->config.is_replaying) {
            this->capture->drawable.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->config.info.bpp), &this->config.mod_palette), others...);
            this->capture->wrm_capture.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->config.info.bpp), &this->config.mod_palette), others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class... Ts>
    void draw_unimplemented(WithLog with_log, Order& order, Rect clip, Ts&... /*others*/)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            (void)clip;
            (void)order;
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip);
            }
            LOG(LOG_INFO, "========================================\n");
        }
    }
};
