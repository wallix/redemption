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

//#define LOGPRINT

#include "test_client_redemption_cli.hpp"


// bjam rdpheadless && bin/gcc-4.9.2/release/rdpheadless --user QA\\proxyuser --pwd $mdp --ip 10.10.46.88 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_keyboard

// bjam rdpclientcli && bin/gcc-4.9.2/release/rdpclientcli --user admin --pwd $mdp --ip 10.10.40.22 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_keyboard



void run_mod(mod_api *, TestClientCLI &, SocketTransport *, EventList &);
void print_help(ModRDPParamsConfig *, size_t);


///////////////////////////////
// APPLICATION
int main(int argc, char** argv){


    //================================
    //      Default User config
    //================================
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 800;
    info.height = 600;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;
    info.cs_monitor.monitorCount = 1;
    //info.encryptionLevel = 1;
    int verbose(0);
    std::string userName;
    std::string ip;
    std::string userPwd;
    int port{};
    std::string localIP;
    int nbTry(3);
    int retryDelay(1000);
    //=========================================================



    std::string script_file_path;
    uint8_t input_connection_data_complete(0);
    for (int i = 0; i < argc; i++) {

        // TODO string_view
        std::string word(argv[i]);

        //================================
        //     Cmd lines User config
        //================================
        if (       word == "--user") {
            userName = std::string(argv[i+1]);
            input_connection_data_complete += TestClientCLI::NAME;

        } else if (word == "--pwd") {
            userPwd = std::string(argv[i+1]);
            input_connection_data_complete += TestClientCLI::PWD;
        } else if (word == "--ip") {
            ip = std::string(argv[i+1]);
            input_connection_data_complete += TestClientCLI::IP;
        } else if (word == "--port") {
            port = std::stoi(std::string(argv[i+1]));
            input_connection_data_complete += TestClientCLI::PORT;
        } else if (word == "--local_ip") {
            localIP = std::string(argv[i+1]);
        } else if (word == "--keylayout") {
            info.keylayout = std::stoi(std::string(argv[i+1]));
        } else if (word == "--bpp") {
            info.bpp = std::stoi(std::string(argv[i+1]));
        } else if (word == "--width") {
            info.width = std::stoi(std::string(argv[i+1]));
        } else if (word == "--height") {
            info.height = std::stoi(std::string(argv[i+1]));
        } else if (word == "--mon_count") {
            info.cs_monitor.monitorCount = std::stoi(std::string(argv[i+1]));
        } else if (word == "--wallpaper") {
            if (       std::string(argv[i+1]) ==  "off") {
                info.rdp5_performanceflags -=  PERF_DISABLE_WALLPAPER;
            }
        } else if (word == "--fullwindowdrag") {
            if (       std::string(argv[i+1]) ==  "off") {
                info.rdp5_performanceflags -=  PERF_DISABLE_FULLWINDOWDRAG;
            }
        } else if (word == "--menuanimations") {
            if (       std::string(argv[i+1]) ==  "off") {
                info.rdp5_performanceflags -=  PERF_DISABLE_MENUANIMATIONS;
            }//================================================================
        }
    }



    //===========================================
    //       Default mod_rdp_param config
    //===========================================
    const char * name_param(userName.c_str());
    const char * targetIP_param(ip.c_str());
    const char * pwd_param(userPwd.c_str());
    const char * local_IP_param(localIP.c_str());

    Inifile ini;
    ModRDPParams mod_rdp_params( name_param
                               , pwd_param
                               , targetIP_param
                               , local_IP_param
                               , 2
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , to_verbose_flags(0)
                               );

    mod_rdp_params.device_id = "device_id";

    const size_t nb_mod_rdp_params_config(34);
    std::string dscr(" Active/unactive ");

    // TODO What that ? See program_options
    ModRDPParamsConfig mod_rdp_params_config[] = {

      ModRDPParamsConfig(&(mod_rdp_params.enable_tls), false
        , "--tls", "                   "+dscr+"tls"
        , "enable_tls")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_nla), false
        , "--nla", "                   "+dscr+"nla"
        , "enable_nla")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_fastpath), false
        , "--fastpath", "              "+dscr+"fastpath"
        , "enable_fastpath")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_mem3blt) , true
        , "--mem3blt", "               "+dscr+"mem3blt"
        , "enable_mem3blt")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_bitmap_update), true
        , "--bmp_upd", "               "+dscr+"bitmap update"
        , "enable_bitmap_update")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_new_pointer), true
        , "--new_pointer", "           "+dscr+"new pointer"
        , "enable_new_pointer")
    , ModRDPParamsConfig(&(mod_rdp_params.server_redirection_support), true
        , "--serv_red", "              "+dscr+"server redirection support"
        , "server_redirection_support")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_krb)
        , "--krb", "                   "+dscr+"krb"
        , "enable_krb")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_glyph_cache)
        , "--glph_cache", "            "+dscr+"glyph cache"
        , "enable_glyph_cache")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_session_probe)
        , "--sess_prb", "              "+dscr+"session probe"
        , "enable_session_probe")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_session_probe_launch_mask)
        , "--sess_prb_lnch_mask", "    "+dscr+"session probe launch mask"
        , "enable_session_probe_launch_mask")
    , ModRDPParamsConfig(&(mod_rdp_params.disable_clipboard_log_syslog)
        , "--disable_cb_log_sys", "    "+dscr+"clipboard log syslog lock"
        , "disable_clipboard_log_syslog")
    , ModRDPParamsConfig(&(mod_rdp_params.disable_clipboard_log_wrm)
        , "--disable_cb_log_wrm", "    "+dscr+"clipboard log wrm lock"
        , "disable_clipboard_log_wrm")
    , ModRDPParamsConfig(&(mod_rdp_params.disable_file_system_log_syslog)
        , "--disable_file_syslog", "   "+dscr+"file system log syslog lock"
        , "disable_file_system_log_syslog")
    , ModRDPParamsConfig(&(mod_rdp_params.disable_file_system_log_wrm)
        , "--disable_file_wrm", "      "+dscr+"file system log wrm lock"
        , "disable_file_system_log_wrm")
    , ModRDPParamsConfig(&(mod_rdp_params.session_probe_use_clipboard_based_launcher)
        , "--sess_prb_cb_based_lnch", ""+dscr+"session probe use clipboard based launcher"
        , "session_probe_use_clipboard_based_launcher")
    , ModRDPParamsConfig(&(mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon)
        , "--sess_prb_slttoal", "      "+dscr+"session probe start launch timeout timer only after logon"
        , "session_probe_start_launch_timeout_timer_only_after_logon")
    , ModRDPParamsConfig(&(mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user)
        , "--sess_prob_oktdu", "       "+dscr+"session probe on keepalive timeout disconnect user"
        , "session_probe_on_keepalive_timeout_disconnect_user")
    , ModRDPParamsConfig(&(mod_rdp_params.session_probe_end_disconnected_session)
        , "--sess_prb_eds", "          "+dscr+"session probe end disconnected session"
        , "session_probe_end_disconnected_session")
    , ModRDPParamsConfig(&(mod_rdp_params.session_probe_customize_executable_name)
        , "--sess_prb_custom_exe", "   "+dscr+"session probe customize executable name"
        , "session_probe_cen")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_transparent_mode)
        , "--transp_mode", "           "+dscr+"enable transparent mode"
        , "enable_transparent_mode")
    , ModRDPParamsConfig(&(mod_rdp_params.ignore_auth_channel)
        , "--ignore_auth_channel", "   "+dscr+"ignore auth channel"
        , "ignore_auth_channel")
    , ModRDPParamsConfig(&(mod_rdp_params.use_client_provided_alternate_shell)
        , "--use_client_as", "         "+dscr+"use client provided alternate shell"
        , "use_client_provided_alternate_shell")
    , ModRDPParamsConfig(&(mod_rdp_params.disconnect_on_logon_user_change)
        , "--disconn_oluc", "          "+dscr+"disconnect on logon user change"
        , "disconnect_on_logon_user_change")
    , ModRDPParamsConfig(&(mod_rdp_params.server_cert_store)
        , "--cert_store", "            "+dscr+"enable server certifications store"
        , "server_cert_store")
    , ModRDPParamsConfig(&(mod_rdp_params.hide_client_name)
        , "--hide_name", "             "+dscr+"hide client name"
        , "hide_client_name")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_persistent_disk_bitmap_cache)
        , "--persist_bmp_cache", "     "+dscr+"enable persistent disk bitmap cache"
        , "enable_persistent_disk_bitmap_cache")
    , ModRDPParamsConfig(&(mod_rdp_params.enable_cache_waiting_list)
        , "--cache_wait_list", "       "+dscr+"enable_cache_waiting_list"
        , "enable_cache_waiting_list")
    , ModRDPParamsConfig(&(mod_rdp_params.persist_bitmap_cache_on_disk)
        , "--persist_bmp_disk ", "     "+dscr+"persist bitmap cache on disk"
        , "persist_bitmap_cache_on_disk")
    , ModRDPParamsConfig(&(mod_rdp_params.server_redirection_support)
        , "--serv_redir_supp", "       "+dscr+"server redirection support"
        , "server_redirection_support")
    , ModRDPParamsConfig(&(mod_rdp_params.bogus_sc_net_size)
        , "--bogus_size", "            "+dscr+"bogus sc net size"
        , "bogus_sc_net_size")
    , ModRDPParamsConfig(&(mod_rdp_params.bogus_refresh_rect)
        , "--bogus_rectc", "           "+dscr+"bogus refresh rect"
        , "bogus_refresh_rect")
    , ModRDPParamsConfig(&(mod_rdp_params.allow_using_multiple_monitors)
        , "--multi_mon", "             "+dscr+"allow using multiple monitors"
        , "allow_using_multiple_monitors")
    , ModRDPParamsConfig(&(mod_rdp_params.adjust_performance_flags_for_recording)
        , "--adj_perf_rec", "          "+dscr+"adjust performance flags for recording"
        , "adjust_performance_flags_for_recording")
    };//========================================================================================


    bool script_on(false);
    for (int i = 0; i <  argc; i++) {

        std::string word(argv[i]);
        //================================
        //            TOOLS
        //================================
        if (word == "--help") {
            print_help(mod_rdp_params_config, nb_mod_rdp_params_config);
        } else if (word == "-h") {
            print_help(mod_rdp_params_config, nb_mod_rdp_params_config);
        } else if (word == "--script_help") {
            // TODO show all script cmd
        } else if (word == "-v") {
            std::cout << "version 1.0" << "\n";
        } else if (word ==  "--script") {
            script_file_path = std::string(argv[i+1]);
            script_on = true;
        } else if (word == "--version") {
            std::cout << "version 1.0" << "\n";
        } else if (word == "--show_user_params") {
            verbose = verbose | TestClientCLI::SHOW_USER_AND_TARGET_PARAMS;
        } else if (word == "--show_rdp_params") {
            verbose = verbose | TestClientCLI::SHOW_MOD_RDP_PARAMS;
        } else if (word == "--show_draw") {
            verbose = verbose | TestClientCLI::SHOW_DRAW_ORDERS_INFO;
        } else if (word == "--show_clpbrd") {
            verbose = verbose | TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE;
        } else if (word == "--show_cursor") {
            verbose = verbose | TestClientCLI::SHOW_CURSOR_STATE_CHANGE;
        } else if (word == "--show_core") {
            verbose = verbose | TestClientCLI::SHOW_CORE_SERVER_INFO;
        } else if (word == "--show_security") {
            verbose = verbose | TestClientCLI::SHOW_SECURITY_SERVER_INFO;
        } else if (word == "--show_keyboard") {
            verbose = verbose | TestClientCLI::SHOW_KEYBOARD_EVENT;
        } else if (word == "--show_files_sys") {
            verbose = verbose | TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE;
        } else if (word == "--show_caps") {
            verbose = verbose | TestClientCLI::SHOW_CAPS;
        } else if (word == "--show_all") {
            verbose = verbose | TestClientCLI::SHOW_CURSOR_STATE_CHANGE
                      | TestClientCLI::SHOW_USER_AND_TARGET_PARAMS
                      | TestClientCLI::SHOW_MOD_RDP_PARAMS
                      | TestClientCLI::SHOW_DRAW_ORDERS_INFO
                      | TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE
                      | TestClientCLI::SHOW_CORE_SERVER_INFO
                      | TestClientCLI::SHOW_SECURITY_SERVER_INFO
                      | TestClientCLI::SHOW_KEYBOARD_EVENT
                      | TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE
                      | TestClientCLI::SHOW_CAPS
                      ;
        } else if (word == "--show_in_pdu") {
            verbose = verbose | TestClientCLI::SHOW_IN_PDU;
        } else if (word == "--show_out_pdu") {
            verbose = verbose | TestClientCLI::SHOW_OUT_PDU;
        } else if (word == "--show_pdu") {
            verbose = verbose | TestClientCLI::SHOW_OUT_PDU
                              | TestClientCLI::SHOW_IN_PDU;
        } else if (word == "--show_channels") {
            verbose = verbose | TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE
                              | TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE;
        }
    } //==============================================================



    std::cout << "\n" << "\n";
    std::cout << " ================================" << "\n";
    std::cout << " ========== Log Config ==========" << "\n";
    std::cout << " ================================" << "\n";
    std::cout << " SHOW_USER_AND_TARGET_PARAMS = " << bool(verbose & TestClientCLI::SHOW_USER_AND_TARGET_PARAMS) <<  "\n";
    std::cout << " SHOW_MOD_RDP_PARAMS         = " << bool(verbose & TestClientCLI::SHOW_MOD_RDP_PARAMS) <<  "\n";
    std::cout << " SHOW_DRAW_ORDERS            = " << bool(verbose & TestClientCLI::SHOW_DRAW_ORDERS_INFO) <<  "\n";
    std::cout << " SHOW_CLPBRD_PDU_EXCHANGE    = " << bool(verbose & TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE) <<  "\n";
    std::cout << " SHOW_CURSOR_STATE_CHANGE    = " << bool(verbose & TestClientCLI::SHOW_CURSOR_STATE_CHANGE) <<  "\n";
    std::cout << " SHOW_CORE_SERVER_INFO       = " << bool(verbose & TestClientCLI::SHOW_CORE_SERVER_INFO) <<  "\n";
    std::cout << " SHOW_SECURITY_SERVER_INFO   = " << bool(verbose & TestClientCLI::SHOW_SECURITY_SERVER_INFO) <<  "\n";
    std::cout << " SHOW_KEYBOARD_EVENT         = " << bool(verbose & TestClientCLI::SHOW_KEYBOARD_EVENT) <<  "\n";
    std::cout << " SHOW_FILE_SYSTEM_EXCHANGE   = " << bool(verbose & TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE) <<  "\n";
    std::cout << " SHOW_CAPS                   = " << bool(verbose & TestClientCLI::SHOW_CAPS) <<  "\n";
    std::cout << " SHOW_OUT_PDU                = " << bool(verbose & TestClientCLI::SHOW_OUT_PDU) <<  "\n";
    std::cout << " SHOW_IN_PDU                 = " << bool(verbose & TestClientCLI::SHOW_IN_PDU) <<  "\n";
    std::cout <<  std::endl;



    if (verbose & TestClientCLI::SHOW_USER_AND_TARGET_PARAMS) {
        std::cout <<  "\n";
        std::cout << " ================================" << "\n";
        std::cout << " == User And Target Parameters ==" << "\n";
        std::cout << " ================================" << "\n";

        std::cout << " user_name= \"" << userName << "\"" <<  "\n";
        std::cout << " user_password= \"" << userPwd << "\"" << "\n";
        std::cout << " ip= \"" << ip << "\"" << "\n";
        std::cout << " port=" << port << "\n";
        std::cout << " ip_local= \"" << localIP << "\"" << "\n";
        std::cout << " keylayout=0x" << std::hex << info.keylayout << std::dec << "\n";
        std::cout << " bpp=" << info.bpp << "\n";
        std::cout << " width=" << info.width << "\n";
        std::cout << " height=" << info.height << "\n";
        std::cout << " wallpaper_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_WALLPAPER) << "\n";
        std::cout << " full_window_drag_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_FULLWINDOWDRAG) << "\n";
        std::cout << " menu_animations_on=" << bool(info.rdp5_performanceflags & PERF_DISABLE_MENUANIMATIONS) << "\n";
        std::cout <<  std::endl;
    }


    uint32_t encryptionMethods(GCC::UserData::CSSecurity::_40BIT_ENCRYPTION_FLAG |           GCC::UserData::CSSecurity::_128BIT_ENCRYPTION_FLAG);
    //===========================================
    //       Cmd line mod_rdp_param config
    //===========================================
    for (int i = 0; i <  argc; i++) {

        std::string word(argv[i]);

        for (size_t j = 0; j < nb_mod_rdp_params_config; j++) {
            if (       word == mod_rdp_params_config[j].cmd) {
                if (       std::string(argv[i+1]) ==  "on") {
                    *(mod_rdp_params_config[j].param) = true;
                } else if (std::string(argv[i+1]) ==  "off") {
                    *(mod_rdp_params_config[j].param) = false;
                }
            }
        }

        if (word ==  "--encrpt_methds") {
            encryptionMethods = std::stoi(argv[i+1]);
        }
    }

    if (verbose & TestClientCLI::SHOW_MOD_RDP_PARAMS) {
        std::cout <<  "\n";
        std::cout << " ================================" << "\n";
        std::cout << " ======= ModRDP Parameters ======" << "\n";
        std::cout << " ================================" << "\n";
        for (size_t i = 0; i < nb_mod_rdp_params_config; i++ ) {
            std::cout << " " << mod_rdp_params_config[i].name << "=" << *(mod_rdp_params_config[i].param) << "\n";
        }
    }
    std::cout << "\n" << "\n";
    std::string allow_channels = "*";
    mod_rdp_params.allow_channels                  = &allow_channels;
    //mod_rdp_params.allow_using_multiple_monitors   = true;
    mod_rdp_params.bogus_refresh_rect              = true;
    mod_rdp_params.verbose = RDPVerbose::cliprdr;
    //======================================================================



    TestClientCLI front(info, verbose);



    if (input_connection_data_complete == TestClientCLI::INPUT_COMPLETE) {
        int sck(0);
        std::unique_ptr<SocketTransport> socket;
        std::string error_message;
        bool connection_succed(false);
        const char * name(userName.c_str());
        const char * targetIP(ip.c_str());
        const std::string errorMsg(" Cannot connect to [" + ip +  "].");

        std::cout << " ================================" << "\n";
        std::cout << " ======= Connection steps =======" << "\n";
        std::cout << " ================================" << "\n";

        sck = ip_connect(targetIP, port, nbTry, retryDelay);

        if (sck > 0) {
            try {
                socket = std::make_unique<SocketTransport>( name
                                                          , sck
                                                          , targetIP
                                                          , port
                                                          , to_verbose_flags(verbose)
                                                          , &error_message
                                                          );
                connection_succed = true;

            } catch (const std::exception &) { // TODO no throwing exception to SocketTransport ctor
                std::cout << errorMsg << " Socket error. " << std::endl;
            }
        } else {
            std::cout << errorMsg << " ip_connect error." << std::endl;
        }

        if (connection_succed) {

            mod_rdp * mod(nullptr);

            LCGRandom gen(0); // To always get the same client random, in tests
            TimeSystem timeSystem;

            try {
                mod = new mod_rdp( *(socket.get())
                                 , front
                                 , info
                                 , ini.get_ref<cfg::mod_rdp::redir_info>()
                                 , gen
                                 , timeSystem
                                 , mod_rdp_params
                                 );

                front._to_server_sender._callback = mod;
                front._callback = mod;
                GCC::UserData::CSSecurity & cs_security = mod->cs_security;
                cs_security.encryptionMethods = encryptionMethods;
                std::cout << " Connected to [" << targetIP <<  "]." << std::endl;

            } catch (const Error & e) {
                std::cout << " Error: RDP Initialization failed. " << e.errmsg() << std::endl;
                connection_succed = false;
            }

            if (connection_succed) {
                if (mod !=  nullptr) {
                    int i = 1;
                    while (!mod->is_up_and_running()) {
                        try {
                            std::cout <<  " Early negociations step " << i << "\n";
                            mod->draw_event(time(nullptr), front);
                            i++;
                         } catch (const Error & e) {
                            std::cout << " Error: Failed during RDP early negociations step " << i << ". " << e.errmsg() << "\n";
                            connection_succed = false;
                        }
                    }
                    std::cout << " Early negociations complete." <<  "\n";
                    std::cout << std::endl;
                }
            }

            if (connection_succed) {

                if (verbose & TestClientCLI::SHOW_CORE_SERVER_INFO) {
                    std::cout <<  "\n";
                    std::cout << " ================================" << "\n";
                    std::cout << " ======= Server Core Info =======" << "\n";
                    std::cout << " ================================" << "\n";
                    GCC::UserData::SCCore sc_core = mod->sc_core;
                    std::cout << " userDataType = " << int(sc_core.userDataType) << "\n";
                    std::cout << " length = " << int(sc_core.length) << "\n";
                    std::cout << " version = " << int(sc_core.version) << "\n";
                    std::cout << " clientRequestedProtocols = " << int(sc_core.clientRequestedProtocols) << "\n";
                    std::cout << " earlyCapabilityFlags = " << int(sc_core.earlyCapabilityFlags) << "\n";
                    std::cout << "\n" << std::endl;
                }

                if (verbose & TestClientCLI::SHOW_SECURITY_SERVER_INFO) {
                    std::cout << " ================================" << "\n";
                    std::cout << " ===== Server Security Info =====" << "\n";
                    std::cout << " ================================" << "\n";
                    GCC::UserData::SCSecurity & sc_sec1 = mod->sc_sec1;
                    std::cout << " userDataType = " << int(sc_sec1.userDataType) << "\n";
                    std::cout << " length = " << int(sc_sec1.length) << "\n";
                    std::cout << " encryptionMethod = " << GCC::UserData::SCSecurity::get_encryptionMethod_name(sc_sec1.encryptionMethod) << "\n";
                    std::cout << " encryptionLevel = " << GCC::UserData::SCSecurity::get_encryptionLevel_name(sc_sec1.encryptionLevel) << "\n";
                    std::cout << " serverRandomLen = " << int(sc_sec1.serverRandomLen) << "\n";
                    std::cout << " serverCertLen = " << int(sc_sec1.serverCertLen) << "\n";
                    std::cout << " dwVersion = " << int(sc_sec1.dwVersion) << "\n";
                    std::cout << " temporary = " << sc_sec1.temporary << "\n";
                    std::cout << " serverRandom : ";
                    // TODO Code duplicated (1)
                    for (size_t i = 0; i < SEC_RANDOM_SIZE; i++) {
                        if ((i % 16) == 0 && i != 0) {
                            std::cout << "\n" << "                ";
                        }
                        std::string space;
                        if (sc_sec1.serverRandom[i] < 0x10) {
                            space = "0";
                        }
                        std::cout <<"0x" << space << std::hex << int(sc_sec1.serverRandom[i]) << std::dec << " ";
                    }
                    std::cout << "\n";
                    std::cout << "\n";
                    std::cout << " pri_exp : ";
                    // TODO Code duplicated (2)
                    for (size_t i = 0; i < 64; i++) {
                        if ((i % 16) == 0 && i != 0) {
                            std::cout << "\n" << "           ";
                        }
                        std::string space;
                        if (sc_sec1.pri_exp[i] < 0x10) {
                            space = "0";
                        }
                        std::cout <<"0x" << space << std::hex << int(sc_sec1.pri_exp[i]) <<  std::dec << " ";
                    }
                    std::cout << "\n";
                    std::cout << "\n";
                    std::cout << " pub_sig : ";
                    // TODO Code duplicated (3)
                    for (size_t i = 0; i < 64; i++) {
                        if ((i % 16) == 0 && i != 0) {
                            std::cout << "\n" << "           ";
                        }
                        std::string space;
                        if (sc_sec1.pub_sig[i] < 0x10) {
                            space = "0";
                        }
                        std::cout <<"0x" << space << std::hex << int(sc_sec1.pub_sig[i]) << std::dec << " ";
                    }

                    std::cout << "\n";
                    std::cout << "\n";
                    std::cout << " proprietaryCertificate : " << "\n";
                    std::cout << "     dwSigAlgId = " << int(sc_sec1.proprietaryCertificate.dwSigAlgId) << "\n";
                    std::cout << "     dwKeyAlgId = " << int(sc_sec1.proprietaryCertificate.dwKeyAlgId) << "\n";
                    std::cout << "     wPublicKeyBlobType = " << int(sc_sec1.proprietaryCertificate.wPublicKeyBlobType) << "\n";
                    std::cout << "     wPublicKeyBlobLen = " << int(sc_sec1.proprietaryCertificate.wPublicKeyBlobLen) << "\n";
                    std::cout << "\n";
                    std::cout << "     RSAPK : " << "\n";
                    std::cout << "        magic = " << int(sc_sec1.proprietaryCertificate.RSAPK.magic) << "\n";
                    std::cout << "\n" << std::endl;

                }



                //===========================================
                //             Scripted Events
                //===========================================
                EventList eventList;
                if (script_on) {
                    std::ifstream ifichier(script_file_path);
                    if(ifichier) {
                        std::string ligne;
                        std::string delimiter = " ";

                        while(std::getline(ifichier, ligne)) {
                            auto pos(ligne.find(delimiter));
                            std::string tag  = ligne.substr(0, pos);
                            std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

                            if (       tag == "wait") {
                                eventList.wait(std::stoi(info));

                            } else if (tag == "key_press") {
                                pos = info.find(delimiter);
                                uint32_t scanCode(std::stoi(info.substr(0, pos)));
                                uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setKey_press(&front, scanCode, flag);

                            } else if (tag == "key_release") {
                                pos = info.find(delimiter);
                                uint32_t scanCode(std::stoi(info.substr(0, pos)));
                                uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setKey_release(&front, scanCode, flag);

                            } else if (tag == "mouse_press") {
                                pos = info.find(delimiter);
                                uint8_t button(std::stoi(info.substr(0, pos)));
                                info = info.substr(pos + delimiter.length(), info.length());
                                pos = info.find(delimiter);
                                uint32_t x(std::stoi(info.substr(0, pos)));
                                uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setMouse_button(&front, button, x, y, true);

                            } else if (tag == "mouse_release") {
                                pos = info.find(delimiter);
                                uint8_t button(std::stoi(info.substr(0, pos)));
                                info = info.substr(pos + delimiter.length(), info.length());
                                pos = info.find(delimiter);
                                uint32_t x(std::stoi(info.substr(0, pos)));
                                uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setMouse_button(&front, button, x, y, false);

                            } else if (tag == "clpbrd_change") {
                                // TODO dynamique data and format injection
                                uint32_t formatIDs                 = RDPECLIP::CF_TEXT;
                                std::string formatListDataLongName("\0\0", 2);

                                // TODO { formatListDataLongName, 1 } -> array_view
                                // TODO { formatIDs, 1 } -> array_view
                                eventList.setClpbrd_change(&front, &formatIDs, &formatListDataLongName, 1);

                            } else if (tag == "click") {
                                pos = info.find(delimiter);
                                uint8_t button(std::stoi(info.substr(0, pos)));
                                info = info.substr(pos + delimiter.length(), info.length());
                                pos = info.find(delimiter);
                                uint32_t x(std::stoi(info.substr(0, pos)));
                                uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setClick(&front, button, x, y);

                            } else if (tag == "double_click") {
                                pos = info.find(delimiter);
                                uint32_t x(std::stoi(info.substr(0, pos)));
                                uint32_t y(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setDouble_click(&front, x, y);

                            } else if (tag ==  "key") {
                                pos = info.find(delimiter);
                                uint32_t scanCode(std::stoi(info.substr(0, pos)));
                                uint32_t flag(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setKey(&front, scanCode, flag);

                            } else if (tag == "loop") {
                                pos = info.find(delimiter);
                                uint32_t jump_size(std::stoi(info.substr(0, pos)));
                                uint32_t count_steps(std::stoi(info.substr(pos + delimiter.length(), info.length())));

                                eventList.setLoop(jump_size, count_steps);
                            }
                        }
                    } else {
                        std::cout <<  "Can't find " << script_file_path << "\n";
                    }
                }


                run_mod(mod, front, socket.get(), eventList);

            }
        }
    }

    return 0;
}



void print_help(ModRDPParamsConfig * mod_rdp_params_config, size_t nb_mod_rdp_params_config) {
    std::cout << "\n";

    std::cout << "Command Line Interface RDP Redemption Client help:" << "\n";
    std::cout << "  ========= TOOLS =========" << "\n";
    std::cout << "  -h or --help              Show help" << "\n";
    std::cout << "  -v or --version           Show version" <<  "\n";
    std::cout << "  --script_help             Show all script event commands" << "\n";
    std::cout << "  --show_user_params        Show user info parameters" << "\n";
    std::cout << "  --show_rdp_params         Show mod rdp parameters" << "\n";
    std::cout << "  --show_draw               Show draw orders info" << "\n";
    std::cout << "  --show_clpbrd             Show clipboard echange PDU info" << "\n";
    std::cout << "  --show_cursor             Show cursor change" << "\n";
    std::cout << "  --show_all                Show all log info, except PDU content" << "\n";
    std::cout << "  --show_core               Show core server info" << "\n";
    std::cout << "  --show_security           Show scurity server info" << "\n";
    std::cout << "  --show_keyboard           Show keyboard event" << "\n";
    std::cout << "  --show_files_sys          Show files sytem exchange info" << "\n";
    std::cout << "  --show_channels           Show all channels exchange info" << "\n";
    std::cout << "  --show_in_pdu             Show received PDU content from shown channels" << "\n";
    std::cout << "  --show_out_pdu            Show sent PDU content from shown channels" << "\n";
    std::cout << "  --show_pdu                Show both sent and received PDU content from shown channels" << "\n";
    std::cout << "  --show_caps               Show capabilities PDU exchange" <<  "\n";
    std::cout << "  --script [file_path]      Set a test PDU file script" << "\n";
    std::cout << "\n";
    std::cout << "  ========= USER =========" << "\n";
    std::cout << "  --user [user_name]        Set session user name" << "\n";
    std::cout << "  --pwd [user_password]     Set sessoion user password" << "\n";
    std::cout << "  --ip [ip]                 Set target IP" << "\n";
    std::cout << "  --port [port]             Set target port" << "\n";
    std::cout << "  --local_ip [local_ip]     Set local IP" << "\n";
    std::cout << "  --mon_count [number]      Set the number of monitor" <<  "\n";
    std::cout << "  --wallpaper [on/off]      Active/unactive wallpapert" << "\n";
    std::cout << "  --fullwindowdrag [on/off] Active/unactive full window drag" << "\n";
    std::cout << "  --menuanimations [on/off] Active/unactive menu animations" << "\n";
    std::cout << "  --keylayout [keylaout_id] Set decimal keylouat window id" << "\n";
    std::cout << "  --bpp [bpp_value]         Set bit per pixel value" << "\n";
    std::cout << "  --width [width_value]     Set screen width" << "\n";
    std::cout << "  --height [height_value]   Set screen height" << "\n";
    std::cout << "  --encrpt_methds           Set encryption methods as any addition of 1, 2, 8 and 16" <<  "\n";
    std::cout << "\n";
    std::cout << "  ======== CONFIG ========" << "\n";
    for (size_t i = 0; i < nb_mod_rdp_params_config; i++) {
        std::cout << "  " << mod_rdp_params_config[i].cmd << " [on/off] " << mod_rdp_params_config[i].descrpt << "\n";
    }

    std::cout << std::endl;
}



void run_mod(mod_api * mod, TestClientCLI & front, SocketTransport * st_mod, EventList & al) {
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

            mod->get_event().wait_on_fd(st_mod?st_mod->sck:INVALID_SOCKET, rfds, max, timeout);

            if (mod->get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
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

            if (mod->get_event().is_set(st_mod?st_mod->sck:INVALID_SOCKET, rfds)) {
                LOG(LOG_INFO, "RDP CLIENT :: draw_event");
                mod->draw_event(time(nullptr), front);
            }

            if (front.is_running()) {
                al.emit();
            }

        } catch (Error & e) {
            LOG(LOG_ERR, "RDP CLIENT :: Exception raised = %d!\n", e.id);
            std::cout << "RDP CLIENT :: Exception raised = " << e.id << "\n";
            run_session = false;
        };
    }

    return;
}
