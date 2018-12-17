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
   Copyright (C) Wallix 2017-2018
   Author(s): Clément Moroldo
*/


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#include <csignal>

#include "utils/log.hpp"

#include "core/session_reactor.hpp"
#include "client_redemption/client_redemption.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include "client_redemption/client_input_output_api/client_headless_socket.hpp"

#pragma GCC diagnostic pop

#include <chrono>

// #include "client_redemption/client_config/client_redemption_path.hpp"



class ClientRedemptionHeadless : public ClientRedemption
{

public:

//     ClientInputSocketAPI        * socket_listener;




public:
    ClientRedemptionHeadless(SessionReactor & session_reactor,
                             ClientRedemptionConfig & config,
        //                      ClientOutputGraphicAPI * graphic_api,
        //                      ClientIOClipboardAPI * io_clipboard_api,
        //                      ClientOutputSoundAPI * output_sound_api,
                             ClientInputSocketAPI * socket_listener)
        //                      ClientKeyLayoutAPI * keylayout_api,
        //                      ClientIODiskAPI * io_disk_api)
        :ClientRedemption(session_reactor, config, nullptr, nullptr, nullptr, socket_listener, nullptr, nullptr) /*config(config)
        , client_sck(-1)
        , _callback(this, nullptr)
        , session_reactor(session_reactor)*/
//         , graphic_api(graphic_api)
//         , io_clipboard_api(io_clipboard_api)

//         , socket_listener (socket_listener)//         , output_sound_api (output_sound_api)
//         , keylayout_api(keylayout_api)
//         , io_disk_api(io_disk_api)
//         , close_box_extra_message_ref("Close")
//         , client_execute(session_reactor, *(this), this->config.info.window_list_caps, false)
//         , clientRDPSNDChannel(this->config.verbose, &(this->channel_mod), nullptr, this->config.rDPSoundConfig)
//         , clientCLIPRDRChannel(this->config.verbose, &(this->channel_mod), nullptr, this->config.rDPClipboardConfig)
//         , clientRDPDRChannel(this->config.verbose, &(this->channel_mod), nullptr, this->config.rDPDiskConfig)
//         , clientRemoteAppChannel(this->config.verbose, &(this->_callback), &(this->channel_mod), nullptr)
//         , start_win_session_time(tvtime())
//         , secondary_connection_finished(false)
//         , primary_connection_finished(false)
//         , local_IP("unknow_local_IP")
    {
//         SSL_load_error_strings();
//         SSL_library_init();

//         this->config.set_icon_movie_data();

//         if (this->io_clipboard_api) {
//             this->io_clipboard_api->set_client(this);
//             this->io_clipboard_api->set_path(this->config.CB_TEMP_DIR);
//             this->io_clipboard_api->set_manager(&(this->clientCLIPRDRChannel));
//         } else {
//             LOG(LOG_WARNING, "No clipoard IO implementation.");
//         }
//         if (this->output_sound_api) {
//             this->output_sound_api->set_path(this->config.SOUND_TEMP_DIR);
//         } else {
//             LOG(LOG_WARNING, "No sound output implementation.");
//         }
//         if (this->socket_listener) {
//             this->socket_listener->set_client(this);
//         }
//         } else {
//             LOG(LOG_WARNING, "No socket lister implementation.");
//         }
//         if (this->graphic_api) {
//             this->graphic_api->set_drawn_client(&(this->_callback), &(this->config));
//         } else {
//             LOG(LOG_WARNING, "No graphic output implementation.");
//         }

        this->cmd_launch_conn();

//         this->client_execute.set_verbose(bool( (RDPVerbose::rail & this->config.verbose) | (RDPVerbose::rail_dump & this->config.verbose) ));
//
//         if (this->config.connection_info_cmd_complete == ClientRedemptionConfig::COMMAND_VALID) {
//
//             this->connect(this->config.target_IP,
//                           this->config.user_name,
//                           this->config.user_password,
//                           this->config.port);
//
//         } else {
//             std::cout <<  "Argument(s) required for connection: ";
//             if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::NAME_GOT)) {
//                 std::cout << "-u [user_name] ";
//             }
//             if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::PWD_GOT)) {
//                 std::cout << "-p [password] ";
//             }
//             if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::IP_GOT)) {
//                 std::cout << "-i [ip_server] ";
//             }
//             if (!(this->config.connection_info_cmd_complete & ClientRedemptionConfig::PORT_GOT)) {
//                 std::cout << "-P [port] ";
//             }
//             std::cout << std::endl;

//             if (this->graphic_api) {
//                 this->graphic_api->init_form();
//                 if (this->config.help_mode) {
//                     this->graphic_api->closeFromGUI();
//                 }
//             }
//         }
    }

   ~ClientRedemptionHeadless() = default;

//    virtual bool is_connected() override {
//         return this->config.connected;
//    }
//
//     int wait_and_draw_event(std::chrono::milliseconds timeout) override
//     {
//         if (ExecuteEventsResult::Error == execute_events(
//             timeout, this->session_reactor, SessionReactor::EnableGraphics{true},
//             *this->_callback.get_mod(), *this
//         )) {
//             LOG(LOG_ERR, "RDP CLIENT :: errno = %s\n", strerror(errno));
//             return 9;
//         }
//         return 0;
//     }

//     virtual void update_keylayout() override {
// //         if (this->keylayout_api) {
// //             this->keylayout_api->update_keylayout(this->config.info.keylayout);
// //
// //             this->keylayout_api->clearCustomKeyCode();
// //             for (KeyCustomDefinition& key : this->config.keyCustomDefinitions) {
// //                 this->keylayout_api->setCustomKeyCode(key.qtKeyID, key.scanCode, key.ASCII8, key.extended);
// //             }
// //         }
//
//         switch (this->config.mod_state) {
//             case ClientRedemptionConfig::MOD_VNC:
//                 this->_callback.init_layout(this->config.modVNCParamsData.keylayout);
//                 break;
//
//             default: this->_callback.init_layout(this->config.info.keylayout);
//                 break;
//         }
//     }

//     const CHANNELS::ChannelDefArray & get_channel_list() const override {
//         return this->cl;
//     }
//
//     void delete_replay_mod() override {
//         this->replay_mod.reset();
//     }
//
//     void closeFromGUI() override {
// //         if (this->graphic_api) {
// //             this->graphic_api->closeFromGUI();
// //         }
//     }

//     virtual void  disconnect(std::string const & error, bool pipe_broken) override {
//
//         this->_callback.disconnect(this->timeSystem.get_time().tv_sec, pipe_broken);
//
//         if (this->socket_listener) {
//             this->socket_listener->disconnect();
//         }
//
//         if (!this->socket) {
//             this->socket.reset();
//         }
//
//         std::chrono::microseconds duration = difftimeval(tvtime(), this->start_win_session_time);
//         uint64_t movie_len = duration.count() / 1000;
//
//         time_t now;
//         time(&now);
//
//         struct tm * timeinfo;
//         char buffer [80];
//         timeinfo = localtime (&now);
//         strftime (buffer,80,"%F_%r",timeinfo);
//         std::string date(buffer);
//
//         if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REPLAY) {
// //             if (this->graphic_api) {
// //                 this->graphic_api->set_ErrorMsg(error);
// //             }
//             std::cout << "Session duration = " << movie_len << " ms" << " " << date <<  std::endl;
//             LOG(LOG_INFO, "Disconnected from [%s].", this->config.target_IP.c_str());
//         } else {
//             LOG(LOG_INFO, "Replay closed.");
//
//         }
//         this->config.set_icon_movie_data();
// //         if (this->graphic_api) {
// //             this->graphic_api->init_form();
// //         }
//     }

//     bool init_mod()  {
//
//         try {
//             this->_callback.init();
//
//             switch (this->config.mod_state) {
//             case ClientRedemptionConfig::MOD_RDP:
//             case ClientRedemptionConfig::MOD_RDP_REMOTE_APP:
//             {
//                 ModRDPParams mod_rdp_params(
//                     this->config.user_name.c_str()
//                   , this->config.user_password.c_str()
//                   , this->config.target_IP.c_str()
//                   , this->local_IP.c_str()
//                   , 2
//                   , this->font
//                   , this->theme
//                   , this->server_auto_reconnect_packet_ref
//                   , this->close_box_extra_message_ref
//                   , this->config.verbose
//                 );
//
//                 mod_rdp_params.device_id                       = "device_id";
//                 mod_rdp_params.enable_tls                      = this->config.modRDPParamsData.enable_tls;
//                 mod_rdp_params.enable_nla                      = this->config.modRDPParamsData.enable_nla;
//                 mod_rdp_params.enable_fastpath                 = true;
//                 mod_rdp_params.enable_mem3blt                  = true;
//                 mod_rdp_params.enable_new_pointer              = true;
//                 mod_rdp_params.enable_glyph_cache              = true;
//                 mod_rdp_params.enable_ninegrid_bitmap          = true;
//                 std::string allow_channels                     = "*";
//                 mod_rdp_params.allow_channels                  = &allow_channels;
//                 mod_rdp_params.deny_channels = nullptr;
//                 mod_rdp_params.enable_rdpdr_data_analysis = false;
//
//                 const bool is_remote_app = this->config.mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP;
//
//                 if (is_remote_app) {
//                     this->client_execute.enable_remote_program(true);
//                     mod_rdp_params.remote_program = true;
//                     mod_rdp_params.client_execute = &(this->client_execute);
//                     mod_rdp_params.remote_program_enhanced = INFO_HIDEF_RAIL_SUPPORTED != 0;
//                     mod_rdp_params.use_client_provided_remoteapp = this->ini.get<cfg::mod_rdp::use_client_provided_remoteapp>();
//                     mod_rdp_params.use_session_probe_to_launch_remote_program = this->ini.get<cfg::context::use_session_probe_to_launch_remote_program>();
//                     this->config.info.cs_monitor = GCC::UserData::CSMonitor{};
//
// //                     if (this->graphic_api) {
// //                         this->config.info.screen_info.width = this->graphic_api->screen_max_width;
// //                         this->config.info.screen_info.height = this->graphic_api->screen_max_height;
// //                     }
//
//                     this->clientRemoteAppChannel.set_configuration(
//                         this->config.info.screen_info.width,
//                         this->config.info.screen_info.height,
//                         this->config.rDPRemoteAppConfig);
//                 }
//
//                 this->unique_mod = new_mod_rdp(
//                     *this->socket
//                   , session_reactor
//                   , *this
//                   , this->config.info
//                   , ini.get_ref<cfg::mod_rdp::redir_info>()
//                   , *this->gen
//                   , this->timeSystem
//                   , mod_rdp_params
//                   , this->authentifier
//                   , this->reportMessage
//                   , this->ini
//                   , nullptr
//                 );
//
//                 if (is_remote_app) {
//                     std::string target_info = str_concat(
//                         this->ini.get<cfg::context::target_str>(),
//                         ':',
//                         this->ini.get<cfg::globals::primary_user_id>());
//                     this->client_execute.set_target_info(target_info);
//                 }
//
//                 break;
//             }
//
//             case ClientRedemptionConfig::MOD_VNC:
//                 this->unique_mod = new_mod_vnc(
//                     *this->socket
//                   , this->session_reactor
//                   , this->config.user_name.c_str()
//                   , this->config.user_password.c_str()
//                   , *this
//                   , this->config.modVNCParamsData.width
//                   , this->config.modVNCParamsData.height
//                   , this->config.modVNCParamsData.keylayout
//                   , 0
//                   , true
//                   , true
//                   , this->config.modVNCParamsData.vnc_encodings.c_str()
//                   , this->reportMessage
//                   , this->config.modVNCParamsData.is_apple
//                   , true                                    // alt server unix
//                   , &this->client_execute
//                   , this->ini
//                   // , to_verbose_flags(0xfffffffd)
//                   , to_verbose_flags(0)
//                   , nullptr
//                 );
//                 break;
//             }
//
//         } catch (const Error &) {
//             this->_callback.init();
//             return false;
//         }
//
//         this->_callback.set_mod(this->unique_mod.get());
//         this->channel_mod.set_mod(this->unique_mod.get());
//
//         return true;
//     }

//     bool init_socket() {
//         if (this->config.is_full_replaying) {
//             LOG(LOG_INFO, "Replay %s", this->config.full_capture_file_name);
//             auto transport = std::make_unique<ReplayTransport>(
//                 this->config.full_capture_file_name.c_str(), this->config.target_IP.c_str(), this->config.port,
//                 this->timeSystem, ReplayTransport::FdType::Timer,
//                 ReplayTransport::FirstPacket::DisableTimer,
//                 ReplayTransport::UncheckedPacket::Send);
//             this->client_sck = transport->get_fd();
//             this->socket = std::move(transport);
//             return true;
//         }
//
//         unique_fd unique_client_sck = ip_connect(this->config.target_IP.c_str(),
//                                           this->config.port,
//                                           3,                //nbTry
//                                           1000             //retryDelay
//                                           );
//
//         this->client_sck = unique_client_sck.fd();
//
//         bool has_error = false;
//         std::string has_error_string;
//
//         if (this->client_sck > 0) {
//             try {
//                 this->socket = std::make_unique<SocketTransport>(
//                     this->config.user_name.c_str(),
//                     std::move(unique_client_sck),
//                     this->config.target_IP.c_str(),
//                     this->config.port,
//                     std::chrono::seconds(1),
//                     to_verbose_flags(0x0),
//                     //SocketTransport::Verbose::dump,
//                     &this->error_message);
//
//                 if (this->config.is_full_capturing) {
//                     this->_socket_in_recorder = std::move(this->socket);
//                     this->socket = std::make_unique<RecorderTransport>(
//                         *this->_socket_in_recorder, this->timeSystem, this->config.full_capture_file_name.c_str());
//                 }
//
//                 LOG(LOG_INFO, "Connected to [%s].", this->config.target_IP.c_str());
//
//             } catch (const Error& e) {
//                 has_error = true;
//                 has_error_string = e.errmsg();
//             }
//         } else {
//             has_error = true;
//         }
//
//         if (has_error) {
//             std::string errorMsg = str_concat(
//                 "Cannot connect to [", this->config.target_IP, "]. Socket error: ", has_error_string);
//             LOG(LOG_WARNING, "%s", errorMsg);
//             this->disconnect(str_concat("<font color='Red'>", errorMsg, "</font>"), true);
//         }
//
//         return !has_error;
//     }

    void listen_to_socket(const std::string& ip, const std::string& name, const std::string& pwd, const int port) {
        if (this->config.connected) {

            if (this->socket_listener) {

                if (this->socket_listener->start_to_listen(this->client_sck, this->_callback.get_mod())) {

                    this->start_wab_session_time = tvtime();

//                     if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
//                         if (this->graphic_api) {
//                             this->graphic_api->show_screen();
//                         }
//                     }

                    this->config.writeAccoundData(ip, name, pwd, port);
                }
            }
        }
    }

    virtual void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {
        ClientRedemption::connect(ip, name, pwd, port);
        this->listen_to_socket(ip, name, pwd, port);
    }
};



using namespace std::chrono_literals;

int run_mod(ClientRedemptionAPI & front, ClientRedemptionConfig & config, ClientCallback & callback, timeval start_win_session_time);


int main(int argc, char const** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor session_reactor;

    RDPVerbose verbose = to_verbose_flags(0x0);      //to_verbose_flags(0x0);

    ClientHeadlessSocket headless_socket(session_reactor);
    ClientInputSocketAPI * headless_socket_api_obj = &headless_socket;

    {
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGPIPE);
        sa.sa_handler = [](int sig){
            std::cout << "got SIGPIPE(" << sig << ") : ignoring\n";
        };
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
        REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
        #if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
        #endif
        sigaction(SIGPIPE, &sa, nullptr);
        REDEMPTION_DIAGNOSTIC_POP
    }

    ClientRedemptionConfig config(/*session_reactor,*/ const_cast<const char**>(argv), argc, verbose, CLIENT_REDEMPTION_MAIN_PATH);

    ClientRedemptionHeadless client( session_reactor, config
//                            , nullptr
//                            , nullptr
//                            , nullptr
                           , headless_socket_api_obj
//                            , nullptr
//                            , nullptr
      );

    return run_mod(client, client.config, client._callback, client.start_win_session_time);
}


int run_mod(ClientRedemptionAPI & front, ClientRedemptionConfig & config, ClientCallback & callback, timeval start_win_session_time) {
    const timeval time_stop = addusectimeval(config.time_out_disconnection, tvtime());
    const std::chrono::milliseconds time_mark = 50ms;

    if (callback.get_mod()) {
        auto & mod = *(callback.get_mod());

        bool logged = false;

        while (true)
        {
            if (mod.logged_on == mod_api::CLIENT_LOGGED && !logged) {
                //mod.logged_on = mod_api::CLIENT_UNLOGGED;
                logged = true;

                std::cout << "RDP Session Log On.\n";
                if (config.quick_connection_test) {

                    std::cout << "quick_connection_test\n";
                    front.disconnect("", false);
                    return 0;
                }
            }

            if (time_stop < tvtime() && !config.persist) {
                std::cerr <<  " Exit timeout (timeout = " << config.time_out_disconnection.count() << " ms)" << std::endl;
                front.disconnect("", false);
                return 8;
            }

            if (int err = front.wait_and_draw_event(time_mark)) {
                return err;
            }

            // send key to keep alive
            if (config.keep_alive_freq) {
                std::chrono::microseconds duration = difftimeval(tvtime(), start_win_session_time);

                if ( ((duration.count() / 1000000) % config.keep_alive_freq) == 0) {
                    callback.send_rdp_scanCode(0x1e, KBD_FLAG_UP);
                    callback.send_rdp_scanCode(0x1e, 0);
                }
            }
        }
    }

    return 0;
}
