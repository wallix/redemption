/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou

    rdp transparent module main header file
*/

#include <iostream>
#include <string>

#include "core/listen.hpp"
#include "core/session.hpp"
#include "transport/socket_transport.hpp"
#include "utils/invalid_socket.hpp"
#include "transport/out_file_transport.hpp"
#include "mod/internal/transparent_replay_mod.hpp"
#include "program_options/program_options.hpp"

void run_mod(mod_api & mod, Front & front, wait_obj & front_event, SocketTransport * st_mod, SocketTransport * st_front);

int main(int argc, char * argv[]) {
    openlog("transparent", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
        "\n"
        "ReDemPtion Transparent Proxy " VERSION ".\n"
        "Copyright (C) Wallix 2010-2018.\n"
        "Christophe Grosjean, Raphael Zhou.\n"
        "\n"
        ;

    std::string input_filename;
    std::string output_filename;
    std::string target_device;
    uint32_t    target_port;
    std::string username;
    std::string password;
    std::string record_filename;
    std::string play_filename;
    std::string persistent_key_list_filename;

    persistent_key_list_filename = "./PersistentKeyList.bin";
    target_port                  = 3389;

    std::string config_filename = app_path(AppPath::CfgIni);

    program_options::options_description desc({
        {'h', "help",    "produce help message"},
        {'v', "version", "show software version"},

        {'i', "input-file",    &input_filename,               "input ini file name"},
        {'o', "output-file",   &output_filename,              "output int file name"},
        {'t', "target-device", &target_device,                "target device[:port]"},
        {'u', "username",      &username,                     "username"},
        {'p', "password",      &password,                     "password"},
        {'k', "key-list-file", &persistent_key_list_filename, "persistent key list file name"},

        {'r', "record-file",   &record_filename,              "record file name"},
        {'d', "play-file",     &play_filename,                "play file name"},

        {"config-file",        &config_filename,              "used an another ini file"},
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: rdptproxy [options]\n\n";
        std::cout << desc << std::endl;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        return 0;
    }

    if (   target_device.empty()
        && play_filename.empty()) {
        std::cerr << "Missing target device or play file name: use -t target or -d filename\n\n";
        return 1;
    }

    if (   !target_device.empty()
        && !play_filename.empty()) {
        std::cerr << "Use -t target or -d filename\n\n";
        return 1;
    }

    if (   !output_filename.empty()
        && !play_filename.empty()) {
        std::cerr << "Use -o filename or -d filename\n\n";
        return 1;
    }

    if (   !record_filename.empty()
        && !play_filename.empty()) {
        std::cerr << "Use -r filename or -d filename\n\n";
        return 1;
    }

    if (   !input_filename.empty()
        && !output_filename.empty()) {
        std::cerr << "Use -i filename or -o filename\n\n";
        return 1;
    }

    if (!target_device.empty()) {
        size_t pos = target_device.find(':');
        if (pos != std::string::npos) {
            target_port = atoi(target_device.substr(pos + 1).c_str());
            target_device.resize(pos);
        }

        if (username.c_str()[0] == 0) {
            std::cerr << "Missing username : use -u username\n\n";
            return 1;
        }
    }

    if (password.empty()) {
        password = "";
    }


    // This server only support one incoming connection before closing listener
    class ServerOnce : public Server {
    public:
        int  sck;
        char ip_source[256];

        ServerOnce() : sck(0) {
           this->ip_source[0] = 0;
        }

        Server_status start(int incoming_sck, bool /*forkable*/) override {
            union {
                struct sockaddr s;
                struct sockaddr_storage ss;
                struct sockaddr_in s4;
                struct sockaddr_in6 s6;
            } u;
            unsigned int sin_size = sizeof(u);
            memset(&u, 0, sin_size);
            this->sck = accept(incoming_sck, &u.s, &sin_size);
            strcpy(this->ip_source, inet_ntoa(u.s4.sin_addr));
            LOG(LOG_INFO, "Incoming socket to %d (ip=%s)\n", this->sck, this->ip_source);
            return START_WANT_STOP;
        }
    } one_shot_server;
    Listen listener(one_shot_server, 0, 3389, true, 5);  // 25 seconds to connect, or timeout
    listener.run(true);

    Inifile ini;
    configuration_load(ini.configuration_holder(), config_filename);
    ini.set<cfg::font>(Font(app_path(AppPath::DefaultFontFile)));

    int nodelay = 1;
    if (-1 == setsockopt( one_shot_server.sck, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&nodelay)
                        , sizeof(nodelay))) {
        LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
    }
    SocketTransport front_trans( "RDP Client", unique_fd{one_shot_server.sck}, "0.0.0.0", 0, std::chrono::seconds(1)
                               , to_verbose_flags(ini.get<cfg::debug::front>()), nullptr);
    wait_obj front_event;

    // Remove existing Persistent Key List file.
    unlink(persistent_key_list_filename.c_str());

    std::unique_ptr<OutFileTransport> persistent_key_list_oft;

    if (unique_fd persistent_key_list_ofd{
        persistent_key_list_filename,
        O_CREAT | O_TRUNC | O_WRONLY,
        S_IRUSR | S_IWUSR | S_IRGRP
    }) {
        persistent_key_list_oft.reset(new OutFileTransport(std::move(persistent_key_list_ofd)));
    }
    else {
        LOG(LOG_ERR, "Failed to open Persistent Key List file to writing: name=\"%s\"",
            persistent_key_list_filename.c_str());
    }

    time_t now = time(nullptr);

    UdevRandom gen;
    TimeSystem timeobj;
    CryptoContext cctx;
    const bool fastpath_support = true;
    const bool mem3blt_support  = true;

    NullReportMessage report_message;

    SessionReactor session_reactor;
    Front front(session_reactor, front_trans, gen, ini, cctx, report_message,
        fastpath_support, mem3blt_support, now, std::move(input_filename),
        persistent_key_list_oft.get());
    null_mod no_mod;

    while (front.up_and_running == 0) {
        now = time(nullptr);
        front.incoming(no_mod, now);
    }

    LOG(LOG_INFO, "hostname=\"%s\"", front.client_info.hostname);


    try {
        if (target_device.empty()) {
            SessionReactor session_reactor;
            TransparentReplayMod mod(session_reactor, front, play_filename.c_str(),
                front.client_info.width, front.client_info.height, nullptr, ini.get<cfg::font>());

            run_mod(mod, front, front_event, nullptr, &front_trans);
        }
        else {
            std::unique_ptr<OutFileTransport> record_oft;

            if (!record_filename.empty()) {
                if (unique_fd record_fd{
                    record_filename,
                    O_CREAT | O_TRUNC | O_WRONLY,
                    S_IRUSR | S_IWUSR | S_IRGRP
                }) {
                    record_oft.reset(new OutFileTransport(std::move(record_fd)));
                }
                else {
                    LOG(LOG_ERR, "Failed to open record file to writing: name=\"%s\"",
                        record_filename.c_str());
                }
            }

            std::unique_ptr<InFileTransport> persistent_key_list_ift;

            if (unique_fd persistent_key_list_ifd{persistent_key_list_filename.c_str(), O_RDONLY}) {
                persistent_key_list_ift.reset(new InFileTransport(std::move(persistent_key_list_ifd)));
            }
            else {
                LOG(LOG_ERR, "Failed to open Persistent Key List file to reading: name=\"%s\"",
                    persistent_key_list_filename.c_str());
            }

            SocketTransport mod_trans(
                "RDP Server", ip_connect(target_device.c_str(), target_port, 3, 1000),
                target_device.c_str(), target_port, std::chrono::seconds(1),
                to_verbose_flags(ini.get<cfg::debug::mod_rdp>()),
                &ini.get_ref<cfg::context::auth_error_message>());

            ClientInfo client_info = front.client_info;

            ModRDPParams mod_rdp_params( username.c_str()
                                       , password.c_str()
                                       , target_device.c_str()
                                       , "0.0.0.0"   // client ip is silenced
                                       , front.keymap.key_flags
                                       , ini.get<cfg::font>()
                                       , ini.get<cfg::theme>()
                                       , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                                       , ini.get_ref<cfg::context::close_box_extra_message>()
                                       , to_verbose_flags(ini.get<cfg::debug::mod_rdp>())
                                       );
            //mod_rdp_params.enable_tls                          = true;
            mod_rdp_params.enable_nla                          = ini.get<cfg::mod_rdp::enable_nla>();
            mod_rdp_params.enable_krb                          = ini.get<cfg::mod_rdp::enable_kerberos>();
            //mod_rdp_params.enable_fastpath                     = true;
            //mod_rdp_params.enable_mem3blt                      = true;
            //mod_rdp_params.enable_new_pointer                  = true;
            mod_rdp_params.enable_transparent_mode             = true;
            mod_rdp_params.output_filename                     = (output_filename.empty() ? "" : output_filename.c_str());
            mod_rdp_params.persistent_key_list_transport       = persistent_key_list_ift.get();
            mod_rdp_params.transparent_recorder_transport      = record_oft.get();
            mod_rdp_params.auth_channel                        = CHANNELS::ChannelNameId(ini.get<cfg::mod_rdp::auth_channel>());
            mod_rdp_params.alternate_shell                     = ini.get<cfg::mod_rdp::alternate_shell>().c_str();
            mod_rdp_params.shell_working_dir                   = ini.get<cfg::mod_rdp::shell_working_directory>().c_str();
            mod_rdp_params.rdp_compression                     = ini.get<cfg::mod_rdp::rdp_compression>();
            mod_rdp_params.disconnect_on_logon_user_change     = ini.get<cfg::mod_rdp::disconnect_on_logon_user_change>();
            mod_rdp_params.open_session_timeout                = ini.get<cfg::mod_rdp::open_session_timeout>();
            mod_rdp_params.extra_orders                        = ini.get<cfg::mod_rdp::extra_orders>().c_str();
            mod_rdp_params.enable_persistent_disk_bitmap_cache = ini.get<cfg::mod_rdp::persistent_disk_bitmap_cache>();
            mod_rdp_params.enable_cache_waiting_list           = ini.get<cfg::mod_rdp::cache_waiting_list>();
            mod_rdp_params.password_printing_mode              = ini.get<cfg::debug::password>();
            mod_rdp_params.cache_verbose                       = to_verbose_flags(ini.get<cfg::debug::cache>());

            mod_rdp_params.allow_channels                      = &(ini.get<cfg::mod_rdp::allow_channels>());
            mod_rdp_params.deny_channels                       = &(ini.get<cfg::mod_rdp::deny_channels>());

            NullAuthentifier authentifier;
            SessionReactor session_reactor;
            mod_rdp mod(mod_trans, session_reactor, front, client_info, ini.get_ref<cfg::mod_rdp::redir_info>(),
                        gen, timeobj, mod_rdp_params, authentifier, report_message, ini);

            run_mod(mod, front, front_event, &mod_trans, &front_trans);
        }
    }   // try
    catch (Error const& e) {
        LOG(LOG_ERR, "errid = %u", e.id);
    }

    front.disconnect();

    shutdown(one_shot_server.sck, 2);
    close(one_shot_server.sck);

    LOG(LOG_INFO, "Listener closed\n");
    LOG(LOG_INFO, "Incoming socket %d (ip=%s)\n", one_shot_server.sck, one_shot_server.ip_source);

    return 0;
}

void run_mod(mod_api & mod, Front & front, wait_obj & front_event, SocketTransport * st_mod, SocketTransport * st_front) {
    struct      timeval time_mark = { 0, 50000 };
    bool        run_session       = true;
    BackEvent_t mod_event_signal  = BACK_EVENT_NONE;

    while (run_session) {
        try {
            unsigned max = 0;
            fd_set   rfds;
            fd_set   wfds;

            io_fd_zero(rfds);
            io_fd_zero(wfds);
            struct timeval timeout = time_mark;

            front_event.wait_on_fd(st_front?st_front->sck:INVALID_SOCKET, rfds, max, timeout);

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                // Socket error
                break;
            }

            if (front_event.is_set(st_front?st_front->sck:INVALID_SOCKET, rfds)) {
                time_t now = time(nullptr);

                try {
                    front.incoming(mod, now);
                }
                catch (...) {
                    run_session = false;
                    continue;
                };
            }

            if (front.up_and_running) {
// TODO        auto & event = mod.get_event();
                // TODO missing session_reactor
                LOG(LOG_ERR, "TODO: Unimplemented: %s line %d", __PRETTY_FUNCTION__, __LINE__);
                // mod.draw_event(time(nullptr), front);
                // if (session_reactor.signal == BACK_EVENT_NEXT) {
                //     run_session = false;
                // }
            }
        } catch (Error const& e) {
            LOG(LOG_ERR, "Session::Session exception = %u!\n", e.id);
            run_session = false;
        };
    }   // while (run_session)
}
