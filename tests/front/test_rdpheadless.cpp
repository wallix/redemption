
/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define RED_TEST_MODULE TestRdp
#include "system/redemption_unit_tests.hpp"

// #undef RECORD_PATH
// #define RECORD_PATH "/tmp/recorded"
// #undef WRM_PATH
// #define WRM_PATH "/tmp/recorded"
// #undef HASH_PATH
// #define HASH_PATH "/tmp/hash"
// #undef RECORD_TMP_PATH
// #define RECORD_TMP_PATH "/tmp/tmp"

// Comment the code block below to generate testing data.
#define LOGNULL


#include "core/font.hpp"
#include "utils/theme.hpp"
// Uncomment the code block below to generate testing data.

// Uncomment the code block below to generate testing data.
//include "transport/socket_transport.hpp"
#include "test_only/transport/test_transport.hpp"
// #include "utils/fileutils.hpp"

#include "front/rdpheadless.hpp"


namespace dump2008 {
    #include "fixtures/dump_w2008.hpp"
}

namespace dump2008_PatBlt {
    #include "fixtures/dump_w2008_PatBlt.hpp"
}



class EventList;

inline int run_mod(mod_api * mod, RDPHeadlessFront & front, int sck, EventList & /*al*/, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    while (front.is_pipe_ok)
    {
        if (mod->logged_on == mod_api::CLIENT_LOGGED) {
            mod->logged_on = mod_api::CLIENT_UNLOGGED;

            std::cout << " RDP Session Log On." << std::endl;
            if (quick_connection_test) {
                std::cout << "quick_connection_test" <<  std::endl;
                return 0;
            }
            break;
        }

        if (time_set_connection_test) {
            if (time_stop > tvtime()) {
                //std::cerr <<  " Exit timeout (timeout = " << time_out_response.tv_sec << " sec " <<  time_out_response.tv_usec << " µsec)" << std::endl;
                return 8;
            }
        }

        if (int err = front.wait_and_draw_event(sck, mod, front, time_mark)) {
            return err;
        }

        if (front.is_running()) {
            front.send_key_to_keep_alive();
//             al.emit();
        }
    }

    return 0;
}

RED_AUTO_TEST_CASE(TestRDPHeadless) {
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER
                               | PERF_DISABLE_FULLWINDOWDRAG
                               | PERF_DISABLE_MENUANIMATIONS;
    info.cs_monitor.monitorCount = 1;
    std::fill(std::begin(info.order_caps.orderSupport), std::end(info.order_caps.orderSupport), 1);
    //info.encryptionLevel = 1;
    int verbose = 0;

//     bool protocol_is_VNC = false;
    const char * userName = "administrateur";
    const char * userPwd = "S3cur3!1nux";
    const char * ip = "10.10.47.36";
    int port(3389);
    const char * localIP;
    std::chrono::milliseconds time_out_response(RDPHeadlessFront::DEFAULT_MAX_TIMEOUT_MILISEC_RESPONSE);
    bool script_on(false);

    std::string out_path;

//     int keep_alive_frequence = 100;
    std::string index = "0";


    std::array<uint8_t, 28> server_auto_reconnect_packet_by_ref {};
    std::string close_box_extra_message_by_ref {};

    ModRDPParams mod_rdp_params( userName
                               , userPwd
                               , ip
                               , localIP
                               , 2
                               , Font{}
                               , Theme{}
                               , server_auto_reconnect_packet_by_ref
                               , close_box_extra_message_by_ref
                               , to_verbose_flags(0)
                               );
//     bool quick_connection_test = true;
//     bool time_set_connection_test = false;
//     std::string script_file_path;
//     uint32_t encryptionMethods
//       = GCC::UserData::CSSecurity::_40BIT_ENCRYPTION_FLAG
//       | GCC::UserData::CSSecurity::_128BIT_ENCRYPTION_FLAG;


    NullAuthentifier authentifier;
    NullReportMessage report_message_cli;
    RDPHeadlessFront front(info, report_message_cli, verbose);
    front.out_path = out_path;
    front.index = index;
//     int main_return = 40;
//     int sck = -42;


//     set_exception_handler_pretty_message();
//
//     // Signal handler (SIGPIPE)
//     {
//         struct sigaction sa;
//         sa.sa_flags = 0;
//         sigaddset(&sa.sa_mask, SIGPIPE);
//         sa.sa_handler = [](int sig){
//             std::cout << "got SIGPIPE(" << sig << ") : ignoring\n";
//         };
//         REDEMPTION_DIAGNOSTIC_PUSH
//         REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
//         REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
//         sigaction(SIGPIPE, &sa, nullptr);
//         REDEMPTION_DIAGNOSTIC_POP
//     }
//
//     RED_CHECK(true);
//
//     sck = front.connect(ip, userName, userPwd, port, protocol_is_VNC, mod_rdp_params, encryptionMethods);

//        RED_CHECK_EQUAL(front.client_info.width, 800);
//        RED_CHECK_EQUAL(front.client_info.height, 600);
//
//     RED_CHECK(true);
//
//     EventList eventList;
//     try {
//         main_return = run_mod(front.mod(), front, sck, eventList, quick_connection_test, time_out_response, time_set_connection_test);
//
//         RED_CHECK(true);
//         // std::cout << "RDP Headless end." <<  std::endl;
//     }
//     catch (Error const& e)
//     {
//         if (e.id == ERR_TRANSPORT_NO_MORE_DATA) {
// //                     std::cerr << e.errmsg() << std::endl;
//             front.report_message_rdp.is_closed = true;
//         }
//         if (front.report_message_rdp.is_closed) {
//             main_return = 0;
//         }
//         else {
//             std::cerr << e.errmsg() << std::endl;
//         }
//     }
//
//     if (!front.report_message_rdp.is_closed) {
//         front.disconnect();
//         //front.mod()->disconnect(tvtime().tv_sec);
//     }
}
