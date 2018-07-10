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
   Copyright (C) Wallix 2010-2017
   Author(s): Clément Moroldo
*/

#include "main/version.hpp"

#include "front/rdpheadless.hpp"
#include "utils/netutils.hpp"
#include "utils/sugar/not_null_ptr.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"
#include "utils/cli.hpp"

#include <iomanip>

#include <signal.h>

// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user QA\\proxyuser --pwd $mdp --ip 10.10.46.88 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all

// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user admin --pwd $mdp --ip 10.10.47.54 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all


int run_mod(not_null_ptr<mod_api> /*mod_ptr*/, RDPHeadlessFront & /*front*/, bool /*quick_connection_test*/, std::chrono::milliseconds /*time_out_response*/, bool /*time_set_connection_test*/);

///////////////////////////////
// APPLICATION
int main(int argc, char** argv)
{
    //================================
    //         Default config
    //================================
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = false;
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

    bool protocol_is_VNC = false;
    std::string userName;
    std::string ip;
    std::string userPwd;
    int port(3389);
    std::string localIP;
    std::chrono::milliseconds time_out_response(RDPHeadlessFront::DEFAULT_MAX_TIMEOUT_MILISEC_RESPONSE);
    bool script_on(false);
    std::string out_path;

    int keep_alive_frequence = 100;
    int index = 0;


    std::array<unsigned char, 28> server_auto_reconnect_packet;
    std::string close_box_extra_message;
    Theme theme;
    Font font;
    ModRDPParams mod_rdp_params( ""
                               , ""
                               , ""
                               , ""
                               , 2
                               , font
                               , theme
                               , server_auto_reconnect_packet
                               , close_box_extra_message
                               , to_verbose_flags(0)
                               );
    bool quick_connection_test = true;
    bool time_set_connection_test = false;
    std::string script_file_path;
    uint32_t encryptionMethods
      = GCC::UserData::CSSecurity::_40BIT_ENCRYPTION_FLAG
      | GCC::UserData::CSSecurity::_128BIT_ENCRYPTION_FLAG;
    uint8_t input_connection_data_complete(0);

    auto options = cli::options(
        cli::helper("========= TOOLS ========="),

        cli::option('h', "help")
        .help("Show help")
        .action(cli::help),

        cli::option('v', "version")
        .help("Show version")
        .action(cli::quit([]{ std::cout << " Version 4.2.3, ReDemPtion " VERSION << "\n"; })),

        cli::option("script_help")
        .help("Show all script event commands")
        .action(cli::quit([]{ std::cout << "script help not yet implemented.\n"; })),

        cli::option("show_user_params")
        .help("Show user info parameters")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_USER_AND_TARGET_PARAMS; }),

        cli::option("show_rdp_params")
        .help("Show mod rdp parameters")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_MOD_RDP_PARAMS; }),

        cli::option("show_draw")
        .help("Show draw orders info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_DRAW_ORDERS_INFO; }),

        cli::option("show_clpbrd")
        .help("Show clipboard echange PDU info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_CLPBRD_PDU_EXCHANGE; }),

        cli::option("show_cursor")
        .help("Show cursor change")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_CURSOR_STATE_CHANGE; }),

        cli::option("show_all")
        .help("Show all log info, except PDU content")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_ALL; }),

        cli::option("show_core")
        .help("Show core server info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_CORE_SERVER_INFO; }),

        cli::option("show_security")
        .help("Show scurity server info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_SECURITY_SERVER_INFO; }),

        cli::option("show_keyboard")
        .help("Show keyboard event")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_KEYBOARD_EVENT; }),

        cli::option("show_files_sys")
        .help("Show files sytem exchange info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_FILE_SYSTEM_EXCHANGE; }),

        cli::option("show_channels")
        .help("Show all channels exchange info")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_FILE_SYSTEM_EXCHANGE
                             |  RDPHeadlessFront::SHOW_CLPBRD_PDU_EXCHANGE; }),

        cli::option("show_in_pdu")
        .help("Show received PDU content from shown channels")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_IN_PDU; }),

        cli::option("show_out_pdu")
        .help("Show sent PDU content from shown channels")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_OUT_PDU; }),

        cli::option("show_pdu")
        .help("Show both sent and received PDU content from shown channels")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_OUT_PDU
                             |  RDPHeadlessFront::SHOW_IN_PDU; }),

        cli::option("show_caps")
        .help("Show capabilities PDU exchange")
        .action([&]{ verbose |= RDPHeadlessFront::SHOW_CAPS; }),

        cli::option("script")
        .help("Show capabilities PDU exchange")
        .action(cli::arg("file_path", [&](array_view_const_char av){
            quick_connection_test = false;
            time_set_connection_test = false;
            script_file_path.assign(av.begin(), av.end());
            script_on = true;
        })),

        cli::option("persist")
        .help("Set connection to persist")
        .action([&]{
            quick_connection_test = false;
            time_set_connection_test = false;
        }),

        cli::option("timeout")
        .help("Set timeout response before to disconnect in milisecond")
        .action(cli::arg("time", [&](long time){
            time_set_connection_test = true;
            quick_connection_test = false;
            time_out_response = std::chrono::seconds(time);
        })),

        cli::helper("========= USER ========="),

        cli::option("user")
        .help("Set session user name")
        .action(cli::arg([&](std::string s){
            userName = std::move(s);
            mod_rdp_params.target_user = userName.c_str();
            input_connection_data_complete |= RDPHeadlessFront::NAME;
        })),

        cli::option("pwd")
        .help("Set session user password")
        .action(cli::arg([&](std::string s){
            userPwd = std::move(s);
            mod_rdp_params.target_password = userPwd.c_str();
            input_connection_data_complete |= RDPHeadlessFront::PWD;
        })),

        cli::option("ip")
        .help("Set target IP")
        .action(cli::arg([&](std::string s){
            ip = std::move(s);
            mod_rdp_params.target_host = ip.c_str();
            input_connection_data_complete |= RDPHeadlessFront::IP;
        })),

        cli::option("port")
        .help("Set target port")
        .action(cli::arg([&](int n){ port = n; })),

        cli::option("local_ip")
        .help("Set local IP")
        .action(cli::arg([&](std::string s){
            localIP = std::move(s);
            mod_rdp_params.client_address = localIP.c_str();
        })),

        cli::option("mon_count")
        .help("Set the number of monitor")
        .action(cli::arg([&](int count){ info.cs_monitor.monitorCount = count; })),

        cli::option("wallpaper")
        .help("Active/unactive wallpapert")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_WALLPAPER; }),

        cli::option("fullwindowdrag")
        .help("Active/unactive full window drag")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_FULLWINDOWDRAG; }),

        cli::option("menuanimations")
        .help("Active/unactive menu animations")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_MENUANIMATIONS; }),

        cli::option("keylayout")
        .help("Set decimal keylouat window id")
        .action(cli::arg("keylaout_id", [&](int id){ info.keylayout = id; })),

        cli::option("bpp")
        .help("Set bit per pixel value")
        .action(cli::arg([&](int bpp){ info.bpp = bpp; })),

        cli::option("width")
        .help("Set screen width")
        .action(cli::arg([&](int w){ info.width = w; })),

        cli::option("height")
        .help("Set screen height")
        .action(cli::arg([&](int h){ info.height = h; })),

        cli::option("encrypt_methds")/*(1, 2, 8, 16)*/
        .help("Set encryption methods as any addition of 1, 2, 8 and 16")
        .action(cli::arg("encryption", [&](int enc){ encryptionMethods = enc; })),

        cli::helper("========= CONFIG ========="),

        cli::option("tls")
        .help("Active/unactive tls")
        .action(cli::on_off_location(mod_rdp_params.enable_tls)),

        cli::option("nla")
        .help("Active/unactive nla")
        .action(cli::on_off_location(mod_rdp_params.enable_nla)),

        cli::option("fastpath")
        .help("Active/unactive fastpath")
        .action(cli::on_off_location(mod_rdp_params.enable_fastpath)),

        cli::option("mem3blt")
        .help("Active/unactive mem3blt")
        .action(cli::on_off_location(mod_rdp_params.enable_mem3blt)),

        cli::option("new_pointer")
        .help("Active/unactive new pointer")
        .action(cli::on_off_location(mod_rdp_params.enable_new_pointer)),

        cli::option("krb")
        .help("Active/unactive krb")
        .action(cli::on_off_location(mod_rdp_params.enable_krb)),

        cli::option("glph_cache")
        .help("Active/unactive glyph cache")
        .action(cli::on_off_location(mod_rdp_params.enable_glyph_cache)),

        cli::option("sess_prb")
        .help("Active/unactive session probe")
        .action(cli::on_off_location(mod_rdp_params.enable_session_probe)),

        cli::option("sess_prb_lnch_mask")
        .help("Active/unactive session probe launch mask")
        .action(cli::on_off_location(mod_rdp_params.session_probe_enable_launch_mask)),

        cli::option("disable_cb_log_sys")
        .help("Active/unactive clipboard log syslog lock")
        .action(cli::on_off([&](bool state){ mod_rdp_params.disable_clipboard_log_syslog = !state; })),

        cli::option("disable_cb_log_wrm")
        .help("Active/unactive clipboard log wrm lock")
        .action(cli::on_off([&](bool state){ mod_rdp_params.disable_clipboard_log_wrm = !state; })),

        cli::option("disable_file_syslog")
        .help("Active/unactive file system log syslog lock")
        .action(cli::on_off([&](bool state){ mod_rdp_params.disable_file_system_log_syslog = !state; })),

        cli::option("disable_file_wrm")
        .help("Active/unactive file system log wrm lock")
        .action(cli::on_off([&](bool state){ mod_rdp_params.disable_file_system_log_wrm = !state; })),

        cli::option("sess_prb_cb_based_lnch")
        .help("Active/unactive session probe use clipboard based launcher")
        .action(cli::on_off_location(mod_rdp_params.session_probe_use_clipboard_based_launcher)),

        cli::option("sess_prb_slttoal")
        .help("Active/unactive session probe start launch timeout timer only after logon")
        .action(cli::on_off_location(mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon)),

        cli::option("sess_prb_eds")
        .help("Active/unactive session probe end disconnected session")
        .action(cli::on_off_location(mod_rdp_params.session_probe_end_disconnected_session)),

        cli::option("sess_prb_custom_exe")
        .help("Active/unactive session probe customize executable name")
        .action(cli::on_off_location(mod_rdp_params.session_probe_customize_executable_name)),

        cli::option("transp_mode")
        .help("Active/unactive enable transparent mode")
        .action(cli::on_off_location(mod_rdp_params.enable_transparent_mode)),

        cli::option("ignore_auth_channel")
        .help("Active/unactive ignore auth channel")
        .action(cli::on_off_location(mod_rdp_params.ignore_auth_channel)),

        cli::option("use_client_as")
        .help("Active/unactive use client provided alternate shell")
        .action(cli::on_off_location(mod_rdp_params.use_client_provided_alternate_shell)),

        cli::option("disconn_oluc")
        .help("Active/unactive disconnect on logon user change")
        .action(cli::on_off_location(mod_rdp_params.disconnect_on_logon_user_change)),

        cli::option("cert_store")
        .help("Active/unactive enable server certifications store")
        .action(cli::on_off_location(mod_rdp_params.server_cert_store)),

        cli::option("hide_name")
        .help("Active/unactive hide client name")
        .action(cli::on_off_location(mod_rdp_params.hide_client_name)),

        cli::option("persist_bmp_cache")
        .help("Active/unactive enable persistent disk bitmap cache")
        .action(cli::on_off_location(mod_rdp_params.enable_persistent_disk_bitmap_cache)),

        cli::option("cache_wait_list")
        .help("Active/unactive enable_cache_waiting_list")
        .action(cli::on_off_location(mod_rdp_params.enable_cache_waiting_list)),

        cli::option("persist_bmp_disk")
        .help("Active/unactive persist bitmap cache on disk")
        .action(cli::on_off_location(mod_rdp_params.persist_bitmap_cache_on_disk)),

        cli::option("bogus_size")
        .help("Active/unactive bogus sc net size")
        .action(cli::on_off_location(mod_rdp_params.bogus_sc_net_size)),

        cli::option("bogus_rectc")
        .help("Active/unactive bogus refresh rect")
        .action(cli::on_off_location(mod_rdp_params.bogus_refresh_rect)),

        cli::option("multi_mon")
        .help("Active/unactive allow using multiple monitors")
        .action(cli::on_off_location(mod_rdp_params.allow_using_multiple_monitors)),

        cli::option("adj_perf_rec")
        .help("Active/unactive adjust performance flags for recording")
        .action(cli::on_off_location(mod_rdp_params.adjust_performance_flags_for_recording)),

        cli::option("outpath")
        .help("Set path where connection time will be written")
        .action(cli::arg("path", [&](std::string s){ out_path = std::move(s); })),

        cli::option("vnc")
        .help("Set protocol to VNC (default protocol is RDP)")
        .action(cli::on_off_location(protocol_is_VNC)),

        cli::option("keep_alive_frequence")
        .help("Set timeout to send keypress to keep the session alive")
        .action(cli::arg([&](int t){ keep_alive_frequence = t; })),

        cli::option("index")
        .help("Set an index to identify this client among clients logs")
        .action(cli::arg("path", [&](int i){ index = i; }))
    );

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            return 0;
        case cli::Res::Help:
            cli::print_help(options, std::cout);
            return 0;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            return 1;
    }


    struct : NullReportMessage {
        void report(const char* reason, const char* /*message*/) override
        {
            // std::cout << "report_message: " << message << "  reason:" << reason << std::endl;
            if (!strcmp(reason, "CLOSE_SESSION_SUCCESSFUL")) {
                this->is_closed = true;
            }
        }

        bool is_closed = false;
    } report_message;
    NullAuthentifier authentifier;
    RDPHeadlessFront front(info, report_message, verbose,
    RDPHeadlessFrontParams{std::move(out_path), index, keep_alive_frequence});
    int main_return = 40;

    if (input_connection_data_complete & RDPHeadlessFront::IP) {

        set_exception_handler_pretty_message();

        // Signal handler (SIGPIPE)
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
            #if REDEMPTION_COMP_CLANG >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
                REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
            #endif
            sigaction(SIGPIPE, &sa, nullptr);
            REDEMPTION_DIAGNOSTIC_POP
        }

        front.connect(ip.c_str(), userName.c_str(), userPwd.c_str(), port, protocol_is_VNC, mod_rdp_params, encryptionMethods);

        //===========================================
        //             Scripted Events
        //===========================================

//         if (script_on) {
//             front.set_event_list(script_file_path.c_str());
//         }

        if ((input_connection_data_complete & RDPHeadlessFront::LOG_COMPLETE) || quick_connection_test) {
            try {
                main_return = run_mod(front.mod(), front, quick_connection_test, time_out_response, time_set_connection_test);
                // std::cout << "RDP Headless end." <<  std::endl;
            }
            catch (Error const& e)
            {
                if (e.id == ERR_TRANSPORT_NO_MORE_DATA) {
//                     std::cerr << e.errmsg() << std::endl;
                    report_message.is_closed = true;
                }
                if (report_message.is_closed) {
                    main_return = 0;
                }
                else {
                    std::cerr << e.errmsg() << std::endl;
                }
            }
        }

        if (!report_message.is_closed) {
            front.disconnect();
            //front.mod()->disconnect(tvtime().tv_sec);
        }
    }

    return main_return;
}


int run_mod(not_null_ptr<mod_api> mod_ptr, RDPHeadlessFront & front, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    auto & mod = *mod_ptr;

    while (front.is_connected())
    {
        if (mod.logged_on == mod_api::CLIENT_LOGGED) {
            mod.logged_on = mod_api::CLIENT_UNLOGGED;

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

        if (int err = front.wait_and_draw_event(time_mark)) {
            return err;
        }

        if (front.is_running()) {
            front.send_key_to_keep_alive();
//             al.emit();
        }
    }

    return 0;
}
