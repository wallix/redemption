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
#include "utils/sugar/algostring.hpp"

#include "acl/auth_api.hpp"
#include "acl/license_api.hpp"

#include "core/channels_authorizations.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/RDP/gcc/userdata/cs_net.hpp"

#include "gdi/graphic_api.hpp"
#include "gdi/osd_api.hpp"

#include "mod/internal/replay_mod.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/rdp/rdp_params.hpp"
#include "mod/rdp/mod_rdp_factory.hpp"
#include "mod/vnc/new_mod_vnc.hpp"

#include "transport/replay_transport.hpp"
#include "transport/socket_transport.hpp"

#include "configs/config.hpp"
#include "RAIL/client_execute.hpp"

#include "capture/capture_params.hpp"
#include "capture/wrm_capture.hpp"

#include "utils/timebase.hpp"
#include "utils/select.hpp"
#include "utils/redirection_info.hpp"
#include "core/events.hpp"

#include "client_redemption/mod_wrapper/client_callback.hpp"
#include "client_redemption/mod_wrapper/client_channel_mod.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"
#include "client_redemption/client_channels/client_rdpdr_channel.hpp"
#include "client_redemption/client_channels/client_rdpsnd_channel.hpp"
#include "client_redemption/client_channels/client_remoteapp_channel.hpp"
#include "client_redemption/client_config/client_redemption_config.hpp"
#include "client_redemption/client_redemption_api.hpp"

#include <iostream>



class ClientRedemption : public ClientRedemptionAPI, public gdi::GraphicApi
{
public:
    ClientRedemptionConfig & config;

private:
    CryptoContext     cctx;
    std::unique_ptr<Transport> socket;

public:
    int               client_sck;

private:
    NullLicenseStore  licensestore;

public:
    ClientCallback _callback;
private:
    ClientChannelMod channel_mod;
public:
    EventContainer& events;

private:
    std::unique_ptr<Transport> _socket_in_recorder;
public:
    std::unique_ptr<ReplayMod> replay_mod;

private:
    // RDP
    CHANNELS::ChannelDefArray   cl;
    std::string          _error;
    std::string   error_message;
    std::unique_ptr<Random> gen;
    std::array<uint8_t, 28> server_auto_reconnect_packet_ref;
    Inifile ini;
    NullSessionLog session_log;
    Theme theme;
    Font font;
    RedirectionInfo redir_info;
    std::string close_box_extra_message_ref;

    //  Remote App
    ClientExecute rail_client_execute;

    ModRdpFactory mod_rdp_factory;
    std::unique_ptr<mod_api> unique_mod;

    enum : int {
        CHANID_CLIPDRD = 1601,
        CHANID_RDPDR   = 1602,
        CHANID_WABDIAG = 1603,
        CHANID_RDPSND  = 1604,
        CHANID_RAIL    = 1605
    };
public:
        //  RDP Channel managers
    ClientRDPSNDChannel    clientRDPSNDChannel;
    ClientCLIPRDRChannel   clientCLIPRDRChannel;
    ClientRDPDRChannel     clientRDPDRChannel;
    ClientRemoteAppChannel clientRemoteAppChannel;

private:
    // Recorder
    Fstat fstat;

public:
    timeval start_connection_time;                          // when socket is connected
    timeval start_wab_session_time;                         // when the first resize is received
    timeval start_win_session_time;                         // when the first memblt is received

private:
    bool secondary_connection_finished;

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

        enum : uint8_t {
            RDPDstBlt,
            RDPMultiDstBlt,
            RDPScrBlt,
            RDPMultiScrBlt,
            RDPMemBlt,
            RDPBitmapData,
            RDPPatBlt,
            RDPMultiPatBlt,
            RDPOpaqueRect,
            RDPMultiOpaqueRect,
            RDPLineTo,
            RDPPolygonSC,
            RDPPolygonCB,
            RDPPolyline,
            RDPEllipseSC,
            RDPEllipseCB,
            RDPMem3Blt,
            RDPGlyphIndex,
            COUNT_FIELD
        };

        static std::string get_field_name(uint8_t index) {

            switch (index) {
                case RDPDstBlt:         return "RDPDstBlt         ";
                case RDPMultiDstBlt:     return "RDPMultiDstBlt     ";
                case RDPScrBlt:          return "RDPScrBlt          ";
                case RDPMultiScrBlt:     return "RDPMultiScrBlt     ";
                case RDPMemBlt:          return "RDPMemBlt          ";
                case RDPBitmapData:      return "RDPBitmapData      ";
                case RDPPatBlt:          return "RDPPatBlt          ";
                case RDPMultiPatBlt:     return "RDPMultiPatBlt     ";
                case RDPOpaqueRect:      return "RDPOpaqueRect      ";
                case RDPMultiOpaqueRect: return "RDPMultiOpaqueRect ";
                case RDPLineTo:          return "RDPLineTo          ";
                case RDPPolygonSC:       return "RDPPolygonSC       ";
                case RDPPolygonCB:       return "RDPPolygonCB       ";
                case RDPPolyline:        return "RDPPolyline        ";
                case RDPEllipseSC:       return "RDPEllipseSC       ";
                case RDPEllipseCB:       return "RDPEllipseCB       ";
                case RDPMem3Blt:         return "RDPMem3Blt         ";
                case RDPGlyphIndex:      return "RDPGlyphIndex      ";
                default: return "unknown wrm order index";
            }
        }

        struct WRMOrderStat {
            unsigned int count = 0;
            unsigned long pixels = 0;
        } wrmOrderStats[COUNT_FIELD];

        void add_wrm_order_stat(uint8_t index, unsigned long pixels) {
            this->wrmOrderStats[index].count++;
            this->wrmOrderStats[index].pixels += pixels;
        }

        unsigned int get_count(uint8_t index) {
            return this->wrmOrderStats[index].count;
        }

        unsigned long get_pixels(uint8_t index) {
            return this->wrmOrderStats[index].pixels;
        }

        void reset() {
            for (WRMOrderStat& stat : this->wrmOrderStats) {
                stat.count = 0;
                stat.pixels = 0;
            }
        }
    } wrmGraphicStat;

    std::string       local_IP;
    bool wab_diag_channel_on = false;
    gdi::NullOsd osd;

public:
    ClientRedemption(EventContainer& events,
                     ClientRedemptionConfig & config)
        : config(config)
        , client_sck(-1)
        , _callback(this)
        , events(events)
        , close_box_extra_message_ref("Close")
        , rail_client_execute(events, *this, *this, this->config.info.window_list_caps,
            bool((RDPVerbose::rail | RDPVerbose::rail_dump) & this->config.verbose))
        , clientRDPSNDChannel(this->config.verbose, &(this->channel_mod), this->config.rDPSoundConfig)
        , clientCLIPRDRChannel(this->config.verbose, &(this->channel_mod), this->config.rDPClipboardConfig)
        , clientRDPDRChannel(this->config.verbose, &(this->channel_mod), this->config.rDPDiskConfig)
        , clientRemoteAppChannel(this->config.verbose, &(this->_callback), &(this->channel_mod))
        , start_win_session_time(tvtime())
        , secondary_connection_finished(false)
        , local_IP("unknown_local_IP")
    {}

   ~ClientRedemption() = default;

    void cmd_launch_conn() {
        if (this->config.connection_info_cmd_complete == ClientRedemptionConfig::COMMAND_VALID) {

            this->connect( this->config.target_IP,
                           this->config.user_name,
                           this->config.user_password,
                           this->config.port);
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
        }
    }

    void update_keylayout() override {

        switch (this->config.mod_state) {
            case ClientRedemptionConfig::MOD_VNC:
                this->_callback.init_layout(this->config.modVNCParamsData.keylayout);
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

    void disconnect(std::string const & error, bool /*pipe_broken*/) override {

        this->config.is_replaying = false;
        this->config.connected = false;

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

        if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REPLAY) {
            this->set_error_msg(error);
            LOG(LOG_INFO, "Session duration = %" PRIu64 " ms %s ", movie_len, date);
//             std::cout << "Session duration = " << movie_len << " ms" << " " << date <<  std::endl;
            LOG(LOG_INFO, "Disconnected from [%s].", this->config.target_IP);
        } else {
            LOG(LOG_INFO, "Replay closed.");

        }
//         this->config.set_icon_movie_data();
    }

    virtual void set_error_msg(const std::string & error) {
        if (!error.empty()) {
            LOG(LOG_WARNING, "RDP Session disconnected error: %s", error);
        }
    }

    virtual bool init_mod() {

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
                );

                mod_rdp_params.device_id          = "device_id";
                mod_rdp_params.enable_tls         = this->config.modRDPParamsData.enable_tls;
                mod_rdp_params.enable_nla         = this->config.modRDPParamsData.enable_nla;
                mod_rdp_params.enable_fastpath    = true;
                mod_rdp_params.enable_new_pointer = true;
                mod_rdp_params.enable_glyph_cache = true;
                mod_rdp_params.enable_remotefx    = this->config.enable_remotefx;
                mod_rdp_params.file_system_params.enable_rdpdr_data_analysis = false;

                const bool is_remote_app = this->config.mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP;

                if (is_remote_app) {
                    this->rail_client_execute.enable_remote_program(true);
                    mod_rdp_params.remote_app_params.enable_remote_program = true;
                    mod_rdp_params.remote_app_params.rail_client_execute = &(this->rail_client_execute);
                    mod_rdp_params.remote_app_params.remote_program_enhanced = INFO_HIDEF_RAIL_SUPPORTED != 0;
                    mod_rdp_params.session_probe_params.used_to_launch_remote_program = this->ini.get<cfg::context::use_session_probe_to_launch_remote_program>();
                    this->config.info.cs_monitor = GCC::UserData::CSMonitor{};

                    this->clientRemoteAppChannel.set_configuration(
                        this->config.info.screen_info.width,
                        this->config.info.screen_info.height,
                        this->config.rDPRemoteAppConfig);
                }

                TLSClientParams tls_client_params;
                tls_client_params.tls_min_level                   = this->config.tls_client_params_data.tls_min_level;
                tls_client_params.tls_max_level                   = this->config.tls_client_params_data.tls_max_level;
                tls_client_params.cipher_string                   = this->config.tls_client_params_data.cipher_string;
                tls_client_params.show_common_cipher_list         = this->config.tls_client_params_data.show_common_cipher_list;

                const ChannelsAuthorizations channels_authorizations("*", std::string{});

                this->unique_mod = new_mod_rdp(
                    *this->socket
                  , *this
                  , this->osd
                  , this->events
                  , this->session_log
                  , *this
                  , this->config.info
                  , this->redir_info
                  , *this->gen
                  , channels_authorizations
                  , mod_rdp_params
                  , tls_client_params
                  , this->licensestore
                  , this->ini
                  , nullptr
                  , nullptr
                  , this->mod_rdp_factory
                );

                if (is_remote_app) {
                    std::string target_info = str_concat(
                        this->config.user_name,  // this->ini.get<cfg::context::target_str>(),
                        ':',
                        this->config.target_IP); // this->ini.get<cfg::globals::primary_user_id>());
                    this->rail_client_execute.set_target_info(target_info);
                }

                break;
            }

            case ClientRedemptionConfig::MOD_VNC:
                this->unique_mod = new_mod_vnc(
                    *this->socket
                  , *this
                  , this->events
                  , this->session_log
                  , this->config.user_name.c_str()
                  , this->config.user_password.c_str()
                  , *this
                  , this->config.modVNCParamsData.width
                  , this->config.modVNCParamsData.height
                  , this->config.modVNCParamsData.keylayout
                  , 0
                  , true
                  , true
                  , this->config.modVNCParamsData.vnc_encodings.c_str()
                  , this->config.modVNCParamsData.is_apple
                  , true                                    // alt server unix
                  , true                                    // support Cursor Pseudo-encoding
                  , nullptr
                  , VNCVerbose()
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
                this->events.time_base, ReplayTransport::FdType::Timer,
                ReplayTransport::FirstPacket::DisableTimer,
                ReplayTransport::UncheckedPacket::Send);
            this->client_sck = transport->get_fd();
            this->socket = std::move(transport);
            return true;
        }

        unique_fd unique_client_sck = ip_connect(
            this->config.target_IP.c_str(), this->config.port);

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
                    SocketTransport::Verbose(),
                    &this->error_message);

                if (this->config.is_full_capturing) {
                    this->_socket_in_recorder = std::move(this->socket);
                    this->socket = std::make_unique<RecorderTransport>(
                        *this->_socket_in_recorder, this->events.time_base,
                        this->config.full_capture_file_name.c_str());
                }

                LOG(LOG_INFO, "Connected to [%s].", this->config.target_IP);

            } catch (const Error& e) {
                has_error = true;
                has_error_string = e.errmsg();
            }
        } else {
            has_error = true;
        }

        if (has_error) {
            std::string errorMsg = str_concat(
                "Cannot connect to [", this->config.target_IP, "]. Socket error: ", has_error_string);
            this->set_error_msg(errorMsg);
            this->disconnect(str_concat("<font color='Red'>", errorMsg, "</font>"), true);
        }

        return !has_error;
    }



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {

        ClientConfig::writeWindowsData(this->config.windowsData);
        ClientConfig::writeClientInfo(this->config);

        this->config.port          = port;
        this->config.target_IP     = ip;
        this->config.user_name     = name;
        this->config.user_password = pwd;

        if (this->config.is_full_capturing || this->config.is_full_replaying) {
            this->gen = std::make_unique<FixedRandom>();
        } else {
            this->gen = std::make_unique<UdevRandom>();
        }

        this->clientRemoteAppChannel.clear();
        this->cl.clear_channels();

        this->config.is_replaying = false;
        if (this->config.is_recording) {
            this->set_capture();
        }

        if (this->config.mod_state != ClientRedemptionConfig::MOD_VNC) {

            if (this->config.mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {

                LOG(LOG_INFO, "ClientRedemption::connect()::MOD_RDP_REMOTE_APP");

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
            }

            if (this->config.modRDPParamsData.enable_shared_virtual_disk) {
                CHANNELS::ChannelDef channel_rdpdr { channel_names::rdpdr
                                                   , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                     GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                                   , CHANID_RDPDR
                                                   };
                this->cl.push_back(channel_rdpdr);

                this->clientRDPDRChannel.set_share_dir(this->config.SHARE_DIR);
            }

            if (this->config.enable_shared_clipboard) {
                CHANNELS::ChannelDef channel_cliprdr { channel_names::cliprdr
                                                     , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                       GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                       GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                                     , CHANID_CLIPDRD
                                                     };
                this->cl.push_back(channel_cliprdr);
            }

            if (this->wab_diag_channel_on) {
                CHANNELS::ChannelDef channel_WabDiag { channel_names::wabdiag
                                                     , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                       GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS
                                                     , CHANID_WABDIAG
                                                     };
                this->cl.push_back(channel_WabDiag);
            }

            if (this->config.modRDPParamsData.enable_sound) {
                CHANNELS::ChannelDef channel_audio_output{ channel_names::rdpsnd
                                                         , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                                           GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                                           GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                                         , CHANID_RDPSND
                                                         };
                this->cl.push_back(channel_audio_output);
            }
        }

        if (this->init_socket()) {

            this->update_keylayout();

            this->config.connected = this->init_mod();

            ClientConfig::writeAccoundData(ip, name, pwd, port, this->config);
        }
    }

     void record_connection_nego_times() {
        if (!this->secondary_connection_finished) {
            this->secondary_connection_finished = true;

            std::chrono::microseconds prim_duration = difftimeval(this->start_wab_session_time, this->start_connection_time);
            long prim_len = prim_duration.count() / 1000;
            LOG(LOG_INFO, "primary connection length = %ld ms", prim_len);

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

            LOG(LOG_INFO, "secondary connection length = %ld ms %s", sec_len, date);
        }
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
        WrmParams wrmParams{
              this->config.info.screen_info.bpp
            , is_remoteapp
            , this->cctx
            , *this->gen
            , this->fstat
            , hash_path.c_str()
            , std::chrono::duration<unsigned int, std::ratio<1, 100> >{60}
            , std::chrono::seconds(600) /* break_interval */
            , WrmCompressionAlgorithm::no_compression
            , RDPSerializerVerbose::none
            , -1u
        };

        CaptureParams captureParams;
        captureParams.now = tvtime();
        captureParams.basename = movie_name.c_str();
        captureParams.record_tmp_path = record_path.c_str();
        captureParams.record_path = record_path.c_str();
        captureParams.groupid = 0;
        captureParams.session_log = nullptr;

        this->capture = std::make_unique<Capture>(
            this->config.info.screen_info.width, this->config.info.screen_info.height,
            captureParams, wrmParams);
    }


    bool load_replay_mod(timeval begin_read, timeval end_read) override
    {
        try {
            (void)begin_read;
            (void)end_read;
            this->replay_mod = std::make_unique<ReplayMod>(
              this->events
            , *this
            , *this
            , this->config._movie_full_path.c_str()
            // , 0             //this->config.info.width
            // , 0             //this->config.info.height
            , this->_error
            , true
            // , begin_read
            // , end_read
            // , ClientRedemptionConfig::BALISED_FRAME
            , false
            , false
            , FileToGraphic::Verbose()
            );

            this->_callback.set_replay(this->replay_mod.get());

            return true;

        } catch (const Error & err) {
            LOG(LOG_ERR, "new ReplayMod error %s", err.errmsg());
        }

        if (this->replay_mod == nullptr) {
            const std::string errorMsg = str_concat("Cannot read movie \"", this->config._movie_full_path, "\".");
//             LOG(LOG_ERR, "%s", errorMsg);
            this->set_error_msg(errorMsg);
            std::string labelErrorMsg = str_concat("<font color='Red'>", errorMsg, "</font>");
            this->disconnect(labelErrorMsg, false);
        }
        return false;
    }

    void replay( const std::string & movie_path) override
    {
        auto const last_delimiter_it = std::find(movie_path.rbegin(), movie_path.rend(), '/');
//         int pos = movie_path.size() - (last_delimiter_it - movie_path.rbegin());

        std::string const movie_name = (last_delimiter_it == movie_path.rend())
        ? movie_path
        : movie_path.substr(movie_path.size() - (last_delimiter_it - movie_path.rbegin()));

        this->config.mod_state = ClientRedemptionConfig::MOD_RDP_REPLAY;
        this->config._movie_name = movie_name;
        this->config._movie_full_path = movie_path;

        if (this->config._movie_name.empty()) {
            this->set_error_msg(movie_path);
            return;
        }

        this->config.is_replaying = true;
        this->config.is_loading_replay_mod = true;

        if (this->load_replay_mod({0, 0}, {0, 0})) {

            this->config.is_loading_replay_mod = false;
            this->wrmGraphicStat.reset();
            this->print_wrm_graphic_stat(movie_path);
        }

        this->config.is_loading_replay_mod = false;
    }

    virtual void print_wrm_graphic_stat(const std::string & /*movie_path*/)
    {
        for (uint8_t i = 0; i < WRMGraphicStat::COUNT_FIELD; i++) {
            unsigned int to_count = this->wrmGraphicStat.get_count(i);
            std::string spacer("       ");

            while (to_count >=  10) {
                to_count /=  10;
                spacer = spacer.substr(0, spacer.length()-1);
            }

            LOG(LOG_INFO, "%s= %u %spixels = %lu", this->wrmGraphicStat.get_field_name(i), this->wrmGraphicStat.get_count(i), spacer, this->wrmGraphicStat.get_pixels(i));
        }
    }

    timeval reload_replay_mod(int begin, timeval now_stop) override
    {
        // timeval movie_time_start;
        //
        // switch (this->replay_mod->get_wrm_version()) {
        //     case WrmVersion::v1:
        //         if (this->load_replay_mod({0, 0}, {0, 0})) {
        //             this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));
        //             movie_time_start = tvtime();
        //             return movie_time_start;
        //         }
        //         break;
        //
        //     case WrmVersion::v2:
        //     {
        //         int last_balised = (begin / ClientRedemptionConfig::BALISED_FRAME);
        //         this->config.is_loading_replay_mod = true;
        //         if (this->load_replay_mod({last_balised * ClientRedemptionConfig::BALISED_FRAME, 0}, {0, 0})) {
        //
        //             this->config.is_loading_replay_mod = false;
        //
        //             this->instant_replay_client(begin, last_balised);
        //
        //             movie_time_start = tvtime();
        //             timeval waited_for_load = {movie_time_start.tv_sec - now_stop.tv_sec, movie_time_start.tv_usec - now_stop.tv_usec};
        //             timeval wait_duration = {movie_time_start.tv_sec - begin - waited_for_load.tv_sec, movie_time_start.tv_usec - waited_for_load.tv_usec};
        //             this->replay_mod->set_wait_after_load_client(wait_duration);
        //         }
        //         this->config.is_loading_replay_mod = false;
        //
        //         return movie_time_start;
        //     }
        // }
        //
        // return movie_time_start;

        return {};
        (void)begin;
        (void)now_stop;
    }

    virtual void instant_replay_client(int begin, int /*last_balised*/) {
        // this->replay_mod->instant_play_client(std::chrono::microseconds(begin*1000000));
        (void)begin;
    }


//     void instant_play_client(std::chrono::microseconds time) override {
//         this->replay_mod->instant_play_client(time);
//     }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //      CHANNELS FUNCTIONS
    //--------------------------------

    void send_to_channel( const CHANNELS::ChannelDef & channel, bytes_view chunk_data
                        , std::size_t /*total_length*/, int flags) override {

        const CHANNELS::ChannelDef * mod_channel = this->cl.get_by_name(channel.name);
        if (!mod_channel) {
            return;
        }

        InStream chunk(chunk_data);

        switch (channel.chanid) {

            case CHANID_CLIPDRD: this->clientCLIPRDRChannel.receive(chunk, flags);
                break;

            case CHANID_RDPDR:   this->clientRDPDRChannel.receive(chunk);
                break;

            case CHANID_RDPSND:  this->clientRDPSNDChannel.receive(chunk);
                break;

            case CHANID_RAIL:    this->clientRemoteAppChannel.receive(chunk);
                break;
/*
            case CHANID_WABDIAG:
            {
                int len = chunk.in_uint32_le();
                // TODO std::string_view
                std::string msg(char_ptr_cast(chunk.get_current()), len);

                if        (msg == "ConfirmationPixelColor=White") {
                    this->wab_diag_question = true;
                    this->answer_question(0xffffffff);
                    this->asked_color = 0xffffffff;
                } else if (msg == "ConfirmationPixelColor=Black") {
                    this->wab_diag_question = true;
                    this->answer_question(0xff000000);
                    this->asked_color = 0xff000000;
                } else {
                    LOG(LOG_INFO, "SERVER >> wabdiag %s", msg);
                }
            }
                break;

            default: LOG(LOG_WARNING, " send_to_channel unknown channel id: %d", channel.chanid);
                break;*/
        }
    }

    void draw(const RDP::RAIL::ActivelyMonitoredDesktop  & cmd) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::ActivelyMonitoredDesktop");
        //cmd.log(LOG_INFO);

        this->clientRemoteAppChannel.draw(cmd);
    }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        if (bool(this->config.verbose & RDPVerbose::rail_order)) {
            cmd.log(LOG_INFO);
//             LOG(LOG_INFO, "RDP::RAIL::NewOrExistingWindow");
        }

        this->clientRemoteAppChannel.draw(cmd);
    }

    void draw(const RDP::RAIL::DeletedWindow            & cmd) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::DeletedWindow");
        //cmd.log(LOG_INFO);
        this->clientRemoteAppChannel.draw(cmd);
    }

    void draw(const RDP::RAIL::WindowIcon            &  /*unused*/) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::WindowIcon");
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::CachedIcon            &  /*unused*/) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::CachedIcon");
//         cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NewOrExistingNotificationIcons            & cmd) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::NewOrExistingNotificationIcons");
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::DeletedNotificationIcons            & cmd) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::DeletedNotificationIcons");
        cmd.log(LOG_INFO);
    }

    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        LOG_IF(bool(this->config.verbose & RDPVerbose::rail_order), LOG_INFO,
            "RDP::RAIL::NonMonitoredDesktop");
        cmd.log(LOG_INFO);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using gdi::GraphicApi::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPPatBlt, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = cmd.rect.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPOpaqueRect, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (this->config.is_pre_loading) {
            Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                            (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                            (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
            const Rect rect = rectBmp.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPOpaqueRect, rect.cx * rect.cy);
        }
        this->draw_impl(no_log{}, bitmap_data, bmp);

        if (!this->config.is_pre_loading && !this->config.is_replaying) {
            this->record_connection_nego_times();
        }
    }

    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPLineTo, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPScrBlt, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMemBlt, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, bitmap);
        if (!this->config.is_pre_loading && !this->config.is_replaying) {
            this->record_connection_nego_times();
        }
    }

    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMem3Blt, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, color_ctx, bitmap);
        /*if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }*/
    }

    void draw(const RDPDstBlt & cmd, Rect clip) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(cmd.rect);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPDstBlt, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMultiDstBlt, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMultiOpaqueRect, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMultiPatBlt, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPMultiScrBlt, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPGlyphIndex, rect.cx * rect.cy);
        }
        this->draw_impl(with_log{}, cmd, clip, color_ctx, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPPolygonSC, rect.cx * rect.cy);
        }
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPPolygonCB, rect.cx * rect.cy);
        }
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPPolyline, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);
            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPEllipseSC, rect.cx * rect.cy);
        }
        this->draw_unimplemented(with_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (this->config.is_pre_loading) {
            const Rect rect = clip.intersect(this->replay_mod->get_dim().w, this->replay_mod->get_dim().h).intersect(clip);

            this->wrmGraphicStat.add_wrm_order_stat(WRMGraphicStat::RDPEllipseCB, rect.cx * rect.cy);
        }
        this->draw_unimplemented(no_log{}, cmd, clip, color_ctx);
    }

    void draw(const RDP::FrameMarker& order) override {
        this->draw_impl(no_log{}, order);
    }

    void set_pointer(uint16_t /*cache_idx*/, Pointer const& /*cursor*/, SetPointerMode /*mode*/) override
    {}

    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {
        //TODO: this->draw_impl(no_log{}, cmd);
    }

    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override {
        this->draw_impl(no_log{}, cmd, content);
    }

    ResizeResult server_resize(ScreenInfo screen_server) override
    {
        LOG_IF(bool(this->config.verbose & RDPVerbose::graphics), LOG_INFO,
            "server_resize to (%u, %u, %d)",
            screen_server.width, screen_server.height, screen_server.bpp);
        return ResizeResult::instant_done;
    }

    void begin_update() override {
        if ((this->config.connected || this->config.is_replaying)) {

            if (this->config.is_recording && !this->config.is_replaying) {
                this->capture->drawable.begin_update();
                this->capture->wrm_capture.begin_update();
                this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
            }
        }
    }


    void end_update() override {
        if ((this->config.connected || this->config.is_replaying)) {

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

    void must_flush_capture() override {}

private:
    using no_log = std::false_type;
    using with_log = std::true_type;

    void draw_impl(no_log /*unused*/, RDP::FrameMarker const& order)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
//             if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO);
//             }
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
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip_or_bmp);
            }
        }

        if (this->config.is_recording && !this->config.is_replaying) {
            this->capture->drawable.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.draw(order, clip_or_bmp, others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
        }
    }

    // TODO color_ctx isn't used
    template<class WithLog, class Order, class... Ts>
    void draw_impl(WithLog with_log, Order& order, Rect clip, gdi::ColorCtx /*color_ctx*/, Ts&... others)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip);
            }
        }

        if (this->config.is_recording && !this->config.is_replaying) {
            this->capture->drawable.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->config.info.screen_info.bpp), &this->config.mod_palette), others...);
            this->capture->wrm_capture.draw(order, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->config.info.screen_info.bpp), &this->config.mod_palette), others...);
            this->capture->wrm_capture.periodic_snapshot(tvtime(), this->_callback.mouse_data.x, this->_callback.mouse_data.y, false);
        }
    }

    template<class WithLog, class Order, class... Ts>
    void draw_unimplemented(WithLog with_log, Order& order, Rect clip, Ts&... /*others*/)
    {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            (void)clip;
            (void)order;
            if constexpr (with_log) { /*NOLINT*/
                order.log(LOG_INFO, clip);
            }
        }
    }
};
