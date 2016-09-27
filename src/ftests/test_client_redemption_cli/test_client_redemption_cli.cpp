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


#include "test_client_redemption_cli.hpp"

// bjam -a rdpclientcli && bin/gcc-4.9.2/release/rdpclientcli --user admin --pwd admin --ip 10.10.40.22 --port 3389

void run_mod(mod_api &, TestClientCLI &, SocketTransport *, bool);
void print_help();


///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    enum : int {
        INPUT_COMPLETE = 15
      , NAME           = 1
      , PWD            = 2
      , IP             = 4
      , PORT           = 8
    };

    // Default
    int verbose = 0;

    std::string userName;
    std::string ip;
    std::string userPwd;
    int port;
    std::string localIP;
    int nbTry(3);
    int retryDelay(1000);

    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1024;
    info.height = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    bool enable_tls                      = false;
    bool enable_nla                      = false;
    bool enable_fastpath                 = false;
    bool enable_mem3blt                  = true;
    bool enable_bitmap_update            = true;
    bool enable_new_pointer              = true;
    bool server_redirection_support      = true;

    TestClientCLI front;


    uint8_t input_connection_data_complete(0);

    for (int i = 0; i <  argc; i++) {

        std::string word(argv[i]);

        if (       word == "--user") {
            userName = std::string(argv[i+1]);
            input_connection_data_complete += NAME;

        } else if (word == "--pwd") {
            userPwd = std::string(argv[i+1]);
            input_connection_data_complete += PWD;
        } else if (word == "--ip") {
            ip = std::string(argv[i+1]);
            input_connection_data_complete += IP;
        } else if (word == "--port") {
            port = std::stoi(std::string(argv[i+1]));
            input_connection_data_complete += PORT;
        } else if (word == "--iplocal") {
            localIP = std::string(argv[i+1]);
        } else if (word == "--tls") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_tls = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_tls = false;
            }
        } else if (word == "--nla") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_nla = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_nla = false;
            }
        } else if (word == "--fastparth") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_fastpath = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_fastpath = false;
            }
        } else if (word == "--mem3blt") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_mem3blt = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_mem3blt = false;
            }
        } else if (word == "--bmp_upd") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_bitmap_update = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_bitmap_update = false;
            }
        } else if (word == "--new_pointer") {
            if (       std::string(argv[i+1]) ==  "on") {
                enable_new_pointer = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                enable_new_pointer = false;
            }
        } else if (word == "--serv_red") {
            if (       std::string(argv[i+1]) ==  "on") {
                server_redirection_support = true;
            } else if (std::string(argv[i+1]) ==  "off") {
                server_redirection_support = false;
            }
        } else if (word == "--help") {
            print_help();
        } else if (word == "-h") {
            print_help();
        } else if (word == "--verbose") {
            verbose = std::stoi(std::string(argv[i+1]));
        }
    }

    front._info = info;
    front._verbose = verbose;


    if (input_connection_data_complete == INPUT_COMPLETE) {
        int sck(0);
        SocketTransport * socket;
        bool connection_succed(false);

        const char * name(userName.c_str());
        const char * targetIP(ip.c_str());
        const std::string errorMsg("Cannot connect to [" + ip +  "].");

        sck = ip_connect(targetIP, port, nbTry, retryDelay);

        if (sck > 0) {
            try {
                std::string error_message;
                socket = new SocketTransport( name
                                            , sck
                                            , targetIP
                                            , port
                                            , verbose
                                            , &error_message
                                            );
                std::cout << "Connected to [" << targetIP <<  "]." << std::endl;
                connection_succed = true;

            } catch (const std::exception & e) {
                std::string windowErrorMsg(errorMsg+" Socket error.");
                std::cout << windowErrorMsg << std::endl;
            }
        } else {
            std::string windowErrorMsg(errorMsg+" ip_connect error.");
            std::cout << windowErrorMsg << std::endl;
        }

        if (connection_succed) {

            mod_rdp * mod(nullptr);

            const char * pwd(userPwd.c_str());
            const char * local_IP(localIP.c_str());

            Inifile ini;
            //ini.set<cfg::debug::rdp>(MODRDP_LOGLEVEL_CLIPRDR);

            ModRDPParams mod_rdp_params( name
                                    , pwd
                                    , targetIP
                                    , local_IP
                                    , 2
                                    , 0
                                    );
            mod_rdp_params.device_id                       = "device_id";
            mod_rdp_params.enable_tls                      = enable_tls;
            mod_rdp_params.enable_nla                      = enable_nla;
            mod_rdp_params.enable_fastpath                 = enable_fastpath;
            mod_rdp_params.enable_mem3blt                  = enable_mem3blt;
            mod_rdp_params.enable_bitmap_update            = enable_bitmap_update;
            mod_rdp_params.enable_new_pointer              = enable_new_pointer;
            mod_rdp_params.server_redirection_support      = server_redirection_support;
            std::string allow_channels = "*";
            mod_rdp_params.allow_channels                  = &allow_channels;
            //mod_rdp_params.allow_using_multiple_monitors   = true;
            //mod_rdp_params.bogus_refresh_rect              = true;
            mod_rdp_params.verbose = MODRDP_LOGLEVEL_CLIPRDR;

            LCGRandom gen(0); // To always get the same client random, in tests
            TimeSystem timeSystem;

            try {
                mod = new mod_rdp( *socket
                                , front
                                , info
                                , ini.get_ref<cfg::mod_rdp::redir_info>()
                                , gen
                                , timeSystem
                                , mod_rdp_params
                                );

                front._to_server_sender._callback = mod;
                front._callback = mod;

                while (!mod->is_up_and_running()) {
                    mod->draw_event(time(nullptr), front);
                }


            } catch (const Error & e) {
                const std::string errorMsg("Error: connexion to [" + ip +  "] is closed.");
                std::cout << errorMsg << std::endl;
                connection_succed = false;
            }

            if (connection_succed) {
                run_mod(*mod, front, socket, true);
            }
        }
    }


    return 0;
}

void print_help() {
    std::cout << std::endl;

    std::cout << "Command Line Interface RDP Redemption Client help:" << std::endl;
    std::cout << "-h or --help              Show help" << std::endl;
    std::cout << "--user [user_name]" << std::endl;
    std::cout << "--pwd [user_password]" << std::endl;
    std::cout << "--ip [ip]" << std::endl;
    std::cout << "--port [port]" << std::endl;
    std::cout << "--iplocal [ip_local]" << std::endl;
    std::cout << "--tls [on/off]            Active/unactive tls" << std::endl;
    std::cout << "--nla [on/off]            Active/unactive nla" << std::endl;
    std::cout << "--fastpath [on/off]       Active/unactive fastpath" << std::endl;
    std::cout << "--mem3blt [on/off]        Active/unactive mem3blt" << std::endl;
    std::cout << "--bmp_upd [on/off]        Active/unactive bitmap update" << std::endl;
    std::cout << "--new_pointer [on/off]    Active/unactive new pointer" << std::endl;
    std::cout << "--serv_red [on/off]       Active/unactive server redirection support" << std::endl;
    std::cout << "--verbose [value]         set verbose level" << std::endl;

    std::cout << std::endl;
}



void run_mod(mod_api & mod, TestClientCLI & front, SocketTransport * st_mod, bool event_from_client) {
    struct      timeval time_mark = { 0, 50000 };
    bool        run_session       = true;

    while (run_session) {
        try {
            unsigned max = 0;
            fd_set   rfds;
            fd_set   wfds;

            io_fd_zero(rfds);
            io_fd_zero(wfds);
            struct timeval timeout = time_mark;

            mod.get_event().wait_on_fd(st_mod?st_mod->sck:INVALID_SOCKET, rfds, max, timeout);

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                timeout.tv_sec  = 2;
                timeout.tv_usec = 0;
            }

            int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);

            LOG(LOG_INFO, "RDP CLIENT :: select num = %d\n", num);

            if (num < 0) {
                if (errno == EINTR) {
                    continue;
                }

                LOG(LOG_INFO, "RDP CLIENT :: errno = %d\n", errno);
                break;
            }

            if (mod.get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                LOG(LOG_INFO, "RDP CLIENT :: draw_event");
                mod.draw_event(time(nullptr), front);
            }

            if (event_from_client && front.is_running()) {
                uint32_t formatIDs[]                  = { RDPECLIP::CF_UNICODETEXT };
                std::string formatListDataShortName[] = { std::string("\0\0", 2) };
                front.send_FormatListPDU(formatIDs, formatListDataShortName, 1, true);
                event_from_client = false;
            }


        } catch (Error & e) {
            LOG(LOG_ERR, "RDP CLIENT :: Exception raised = %d!\n", e.id);
            run_session = false;
        };
    }

    return;
}
