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

//#define LOGPRINT

#include "configs/config.hpp"

#include "main/rdpheadless.hpp"
#include "utils/timeval_ops.hpp"

#include "test_only/lcg_random.hpp"

// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user QA\\proxyuser --pwd $mdp --ip 10.10.46.88 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all

// bjam debug rdpheadless && bin/gcc-4.9.2/debug/rdpheadless --user admin --pwd $mdp --ip 10.10.47.54 --port 3389 --script /home/cmoroldo/Bureau/redemption/script_rdp_test.txt --show_all


int wait_and_draw_event(int sck, mod_api &, FrontAPI &, timeval timeout);
int run_mod(mod_api &, TestClientCLI &, int sck, EventList &, bool, std::chrono::milliseconds, bool);

namespace po
{
    struct DataOption
    {
        char short_name;
        char const * long_name;
        char const * help;
    };

    template<class Act>
    struct Option
    {
        DataOption d;
        Act act;

        Option(char short_name, char const * long_name, Act act = Act{})
          : d{short_name, long_name, nullptr}
          , act(act)
        {}

        Option(DataOption d, Act act)
          : d{d}
          , act(act)
        {}

        Option & help(char const * mess)
        {
            this->d.help = mess;
            return *this;
        }

        template<class NewAct>
        Option<NewAct> action(NewAct act)
        {
            return {this->d, act};
        }
    };

    struct NoAct
    {
        void operator()() const
        {}
    };

    enum class Res
    {
        Ok,
        Exit,
        Help,
        BadOption,
        BadFormat
    };

    constexpr struct Ok_ : std::integral_constant<Res, Res::Ok> {} Ok {};

    template<class T>
    T operator, (T x, Ok_)
    { return x; }

    template<class Parser, class Act>
    auto apply_option_impl(Parser & parser, Act && act, int)
    -> decltype(act(parser))
    {
        return act(parser);
    }

    template<class Parser, class Act>
    auto apply_option_impl(Parser &, Act && act, char)
    -> decltype(act())
    {
        return act();
    }

    template<class Parser, class Act>
    Res apply_option(Parser & parser, Act && act)
    {
        return apply_option_impl(parser, act, 1), Ok;
    }

    template<class Act>
    struct Quit
    {
        Act act;

        template<class Parser>
        Res operator()(Parser & parser) const
        {
            apply_option(parser, act);
            return Res::Exit;
        }
    };

    template<class Act>
    Quit<Act> quit(Act act)
    {
        return {act};
    }

    constexpr struct Help
    {
        template<class Parser>
        Res operator()(Parser const &) const
        {
            return Res::Help;
        }
    } help {};

    template<class Act>
    Res arg_parse(int*, char const * s, Act act)
    {
        return act(std::stoi(s)), Ok;
    }

    template<class Act>
    Res arg_parse(long*, char const * s, Act act)
    {
        return act(std::stol(s)), Ok;
    }

    template<class Act>
    Res arg_parse(std::string*, char const * s, Act act)
    {
        return act(s), Ok;
    }

    template<class Act>
    Res arg_parse(char const *, char const * s, Act act)
    {
        return act(s), Ok;
    }

    template<class T, class Act>
    Res arg_parse(array_view<T>*, char const * s, Act act)
    {
        return act({s, strlen(s)}), Ok;
    }

    template<class T, class Act>
    struct Arg
    {
        char const * name;
        Act act;

        template<class Parser>
        Res operator()(Parser & parser) const
        {
            auto s = parser.argv[parser.opti];
            auto r = arg_parse(static_cast<T*>(nullptr), s, act);
            if (r == Res::Ok) {
                ++parser.opti;
            }
            return r;
        }
    };

    template<class T, class Act>
    Arg<T, Act> arg(char const * name, Act act)
    {
        return {name, act};
    }

    template<class T, class Act>
    Arg<T, Act> arg(Act act)
    {
        return {nullptr, act};
    }

    template<class Mem>
    struct ParamTypeMem;

    template<class R, class M, class T>
    struct ParamTypeMem<R (M::*)(T) const>
    {
        using type = typename std::decay<T>::type;
    };

    template<class F>
    using ParamType = typename ParamTypeMem<decltype(&F::operator())>::type;

    template<class Act>
    Arg<ParamType<Act>, Act> arg(char const * name, Act act)
    {
        return {name, act};
    }

    template<class Act>
    Arg<ParamType<Act>, Act> arg(Act act)
    {
        return {nullptr, act};
    }

    template<class Act>
    struct OnOff
    {
        Act act;

        template<class Parser>
        Res operator()(Parser & parser) const
        {
            auto s = parser.argv[parser.opti];
            return this->act(!strcmp(s, "on")), Ok;
        }
    };

    template<class Act>
    OnOff<Act> on_off(Act act)
    {
        return {act};
    }

    inline Option<NoAct> option(char short_name, char const * long_name)
    {
        return {short_name, long_name};
    }

    inline Option<NoAct> option(char short_name)
    {
        return {short_name, ""};
    }

    inline Option<NoAct> option(char const * long_name)
    {
        return {0, long_name};
    }

    template<class Parser>
    Res parse_long_option(char const *, Parser &)
    {
        return Res::BadOption;
    }

    template<class Parser, class Opt, class... Opts>
    Res parse_long_option(char const * s, Parser & parser, Opt& opt, Opts&... opts)
    {
        if (!strcmp(opt.d.long_name, s)) {
            ++parser.opti;
            return apply_option(parser, opt.act);
        }
        return parse_long_option(s, parser, opts...);
    }

    template<class Parser>
    Res parse_short_option(char const *, Parser &)
    {
        return Res::BadOption;
    }

    template<class Parser, class Opt, class... Opts>
    Res parse_short_option(char const * s, Parser & parser, Opt& opt, Opts&... opts)
    {
        if (opt.d.short_name == s[0]) {
            ++parser.opti;
            return apply_option(parser, opt.act);
        }
        return parse_short_option(s, parser, opts...);
    }

    template<class Opt, class T, class Act>
    void print_action(std::ostream & out, Opt const & opt, Arg<T, Act> const & arg)
    {
        out << " [";
        if (arg.name) {
            out << arg.name;
        }
        else if (*opt.d.long_name) {
            out << opt.d.long_name;
        }
        else {
            out << opt.d.short_name;
        }
        out << ']';
    }

    template<class Opt, class Act>
    void print_action(std::ostream & out, Opt const &, OnOff<Act> const &)
    {
        out << " [on/off]";
    }

    template<class Opt, class Act>
    void print_action(std::ostream &, Opt const &, Act const &)
    {}

    template<class Opt>
    void print_help(std::ostream & out, Opt const & opt)
    {
        if (opt.d.short_name) {
            out << '-' << opt.d.short_name;
            if (*opt.d.long_name) {
                out << ", --" << opt.d.long_name;
            }
        }
        else if (*opt.d.long_name) {
            out << "--" << opt.d.long_name;
        }

        print_action(out, opt, opt.act);

        out << "    " << opt.d.help << "\n";
    }

    template<class... Opts>
    struct Parser : Opts...
    {
        Parser(Opts... opts) : Opts(opts)... {}

        Res parse(int const ac, char const * const * const av)
        {
            this->argc = ac;
            this->argv = av;
            this->opti = 1;
            while (this->opti < ac) {
                auto * s = this->argv[this->opti];
                Res r = Res::BadFormat;
                if (s[0] && s[0] == '-' && s[1]) {
                    if (s[1] == '-') {
                        if (s[2]) {
                            r = parse_long_option(s+2, *this, static_cast<Opts&>(*this)...);
                        }
                    }
                    else if (!s[2]) {
                        r = parse_short_option(s+1, *this, static_cast<Opts&>(*this)...);
                    }
                }
                if (r != Res::Ok) {
                    return r;
                }
            }
            return Res::Ok;
        }

        void help(std::ostream & out) const
        {
            (void)std::initializer_list<int>{
                (print_help(out, static_cast<Opts const &>(*this)), 0)...
            };
        }

        int opti;
        int argc;
        char const * const * argv;
    };

    template<class... Ts>
    Parser<Ts...> parser(Ts... p)
    {
        return {p...};
    }
}

///////////////////////////////
// APPLICATION
int main(int argc, char** argv)
{
    //================================
    //         Default config
    //================================
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
    //info.encryptionLevel = 1;
    int verbose = 0;

    bool protocol_is_VNC = false;
    std::string userName;
    std::string ip;
    std::string userPwd;
    int port(3389);
    std::string localIP;
    int nbTry(3);
    int retryDelay(1000);
    std::chrono::milliseconds time_out_response(TestClientCLI::DEFAULT_MAX_TIMEOUT_MILISEC_RESPONSE);
    bool script_on(false);
    std::string out_path;

    Inifile ini;
    ModRDPParams mod_rdp_params( ""
                               , ""
                               , ""
                               , ""
                               , 2
                               , ini.get<cfg::font>()
                               , ini.get<cfg::theme>()
                               , ini.get_ref<cfg::context::server_auto_reconnect_packet>()
                               , to_verbose_flags(0)
                               );
    bool quick_connection_test = true;
    bool time_set_connection_test = false;
    std::string script_file_path;
    uint32_t encryptionMethods
      = GCC::UserData::CSSecurity::_40BIT_ENCRYPTION_FLAG
      | GCC::UserData::CSSecurity::_128BIT_ENCRYPTION_FLAG;
    uint8_t input_connection_data_complete(0);


    auto parser = po::parser(
        // TOOLS
        po::option('h', "help")
        .help("Show help")
        .action(po::help),

        po::option('v', "version")
        .help("Show version")
        .action(po::quit([]{ std::cout << " Version 4.2.3" << "\n"; })),

        po::option("script_help")
        .help("Show all script event commands")
        .action(po::quit([]{ std::cout << "script help not yet implemented.\n"; })),

        po::option("show_user_params")
        .help("Show user info parameters")
        .action([&]{ verbose |= TestClientCLI::SHOW_USER_AND_TARGET_PARAMS; }),

        po::option("show_rdp_params")
        .help("Show mod rdp parameters")
        .action([&]{ verbose |= TestClientCLI::SHOW_MOD_RDP_PARAMS; }),

        po::option("show_draw")
        .help("Show draw orders info")
        .action([&]{ verbose |= TestClientCLI::SHOW_DRAW_ORDERS_INFO; }),

        po::option("show_clpbrd")
        .help("Show clipboard echange PDU info")
        .action([&]{ verbose |= TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE; }),

        po::option("show_cursor")
        .help("Show cursor change")
        .action([&]{ verbose |= TestClientCLI::SHOW_CURSOR_STATE_CHANGE; }),

        po::option("show_all")
        .help("Show all log info, except PDU content")
        .action([&]{ verbose |= TestClientCLI::SHOW_ALL; }),

        po::option("show_core")
        .help("Show core server info")
        .action([&]{ verbose |= TestClientCLI::SHOW_CORE_SERVER_INFO; }),

        po::option("show_security")
        .help("Show scurity server info")
        .action([&]{ verbose |= TestClientCLI::SHOW_SECURITY_SERVER_INFO; }),

        po::option("show_keyboard")
        .help("Show keyboard event")
        .action([&]{ verbose |= TestClientCLI::SHOW_KEYBOARD_EVENT; }),

        po::option("show_files_sys")
        .help("Show files sytem exchange info")
        .action([&]{ verbose |= TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE; }),

        po::option("show_channels")
        .help("Show all channels exchange info")
        .action([&]{ verbose |= TestClientCLI::SHOW_FILE_SYSTEM_EXCHANGE
                             |  TestClientCLI::SHOW_CLPBRD_PDU_EXCHANGE; }),

        po::option("show_in_pdu")
        .help("Show received PDU content from shown channels")
        .action([&]{ verbose |= TestClientCLI::TestClientCLI::SHOW_IN_PDU; }),

        po::option("show_out_pdu")
        .help("Show sent PDU content from shown channels")
        .action([&]{ verbose |= TestClientCLI::SHOW_OUT_PDU; }),

        po::option("show_pdu")
        .help("Show both sent and received PDU content from shown channels")
        .action([&]{ verbose |= TestClientCLI::SHOW_OUT_PDU
                             |  TestClientCLI::SHOW_IN_PDU; }),

        po::option("show_caps")
        .help("Show capabilities PDU exchange")
        .action([&]{ verbose |= TestClientCLI::SHOW_CAPS; }),

        po::option("script")
        .help("Show capabilities PDU exchange")
        .action(po::arg("file_path", [&](array_view_const_char av){
            quick_connection_test = false;
            time_set_connection_test = false;
            script_file_path.assign(av.begin(), av.end()),
            script_on = true;
        })),

        po::option("persist")
        .help("Set connection to persist")
        .action([&]{
            quick_connection_test = false;
            time_set_connection_test = false;
        }),

        po::option("timeout")
        .help("Set timeout response before to disconnect in milisecond")
        .action(po::arg("time", [&](long time){
            time_set_connection_test = true;
            quick_connection_test = false;
            time_out_response = std::chrono::seconds(time);
        })),

        po::option("vnc")
        .help("Set timeout response before to disconnect in milisecond")
        .action(po::on_off([&](bool state){ protocol_is_VNC = state; })),

        // USER
        po::option("user")
        .help("Set session user name")
        .action(po::arg([&](std::string s){
            userName = std::move(s);
            mod_rdp_params.target_user = userName.c_str();
            input_connection_data_complete |= TestClientCLI::NAME;
        })),

        po::option("pwd")
        .help("Set session user password")
        .action(po::arg([&](std::string s){
            userPwd = std::move(s);
            mod_rdp_params.target_password = userPwd.c_str();
            input_connection_data_complete |= TestClientCLI::PWD;
        })),

        po::option("ip")
        .help("Set target IP")
        .action(po::arg([&](std::string s){
            ip = std::move(s);
            mod_rdp_params.target_host = ip.c_str();
            input_connection_data_complete |= TestClientCLI::IP;
        })),

        po::option("port")
        .help("Set target port")
        .action(po::arg([&](int n){ port = n; })),

        po::option("local_ip")
        .help("Set local IP")
        .action(po::arg([&](std::string s){
            localIP = std::move(s);
            mod_rdp_params.client_address = localIP.c_str();
        })),

        po::option("mon_count")
        .help("Set the number of monitor")
        .action(po::arg([&](int count){ info.cs_monitor.monitorCount = count; })),

        po::option("wallpaper")
        .help("Active/unactive wallpapert")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_WALLPAPER; }),

        po::option("fullwindowdrag")
        .help("Active/unactive full window drag")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_FULLWINDOWDRAG; }),

        po::option("menuanimations")
        .help("Active/unactive menu animations")
        .action([&]{ info.rdp5_performanceflags -= PERF_DISABLE_MENUANIMATIONS; }),

        po::option("keylayout")
        .help("Set decimal keylouat window id")
        .action(po::arg("keylaout_id", [&](int id){ info.keylayout = id; })),

        po::option("bpp")
        .help("Set bit per pixel value")
        .action(po::arg([&](int bpp){ info.bpp = bpp; })),

        po::option("width")
        .help("Set screen width")
        .action(po::arg([&](int w){ info.width = w; })),

        po::option("height")
        .help("Set screen height")
        .action(po::arg([&](int h){ info.height = h; })),

        po::option("encrpt_methds")/*(1, 2, 8, 16)*/
        .help("Set encryption methods as any addition of 1, 2, 8 and 16")
        .action(po::arg("encryption", [&](int enc){ encryptionMethods = enc; })),

        // CONFIG
        po::option("tls")
        .help("Active/unactive tls")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_tls = state; })),

        po::option("nla")
        .help("Active/unactive nla")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_nla = true; })),

        po::option("fastpath")
        .help("Active/unactive fastpath")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_fastpath = state; })),

        po::option("mem3blt")
        .help("Active/unactive mem3blt")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_mem3blt = state; })),

        po::option("new_pointer")
        .help("Active/unactive new pointer")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_new_pointer = state; })),

        po::option("serv_red")
        .help("Active/unactive server redirection support")
        .action(po::on_off([&](bool state){ mod_rdp_params.server_redirection_support = state; })),

        po::option("krb")
        .help("Active/unactive krb")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_krb = state; })),

        po::option("glph_cache")
        .help("Active/unactive glyph cache")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_glyph_cache = state; })),

        po::option("sess_prb")
        .help("Active/unactive session probe")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_session_probe = state; })),

        po::option("sess_prb_lnch_mask")
        .help("Active/unactive session probe launch mask")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_enable_launch_mask = state; })),

        po::option("disable_cb_log_sys")
        .help("Active/unactive clipboard log syslog lock")
        .action(po::on_off([&](bool state){ mod_rdp_params.disable_clipboard_log_syslog = !state; })),

        po::option("disable_cb_log_wrm")
        .help("Active/unactive clipboard log wrm lock")
        .action(po::on_off([&](bool state){ mod_rdp_params.disable_clipboard_log_wrm = !state; })),

        po::option("disable_file_syslog")
        .help("Active/unactive file system log syslog lock")
        .action(po::on_off([&](bool state){ mod_rdp_params.disable_file_system_log_syslog = !state; })),

        po::option("disable_file_wrm")
        .help("Active/unactive file system log wrm lock")
        .action(po::on_off([&](bool state){ mod_rdp_params.disable_file_system_log_wrm = !state; })),

        po::option("sess_prb_cb_based_lnch")
        .help("Active/unactive session probe use clipboard based launcher")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_use_clipboard_based_launcher = state; })),

        po::option("sess_prb_slttoal")
        .help("Active/unactive session probe start launch timeout timer only after logon")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon = state; })),

        po::option("sess_prob_oktdu")
        .help("Active/unactive session probe on keepalive timeout disconnect user")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user = state; })),

        po::option("sess_prb_eds")
        .help("Active/unactive session probe end disconnected session")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_end_disconnected_session = state; })),

        po::option("sess_prb_custom_exe")
        .help("Active/unactive session probe customize executable name")
        .action(po::on_off([&](bool state){ mod_rdp_params.session_probe_customize_executable_name = state; })),

        po::option("transp_mode")
        .help("Active/unactive enable transparent mode")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_transparent_mode = state; })),

        po::option("ignore_auth_channel")
        .help("Active/unactive ignore auth channel")
        .action(po::on_off([&](bool state){ mod_rdp_params.ignore_auth_channel = state; })),

        po::option("use_client_as")
        .help("Active/unactive use client provided alternate shell")
        .action(po::on_off([&](bool state){ mod_rdp_params.use_client_provided_alternate_shell = state; })),

        po::option("disconn_oluc")
        .help("Active/unactive disconnect on logon user change")
        .action(po::on_off([&](bool state){ mod_rdp_params.disconnect_on_logon_user_change = state; })),

        po::option("cert_store")
        .help("Active/unactive enable server certifications store")
        .action(po::on_off([&](bool state){ mod_rdp_params.server_cert_store = state; })),

        po::option("hide_name")
        .help("Active/unactive hide client name")
        .action(po::on_off([&](bool state){ mod_rdp_params.hide_client_name = state; })),

        po::option("persist_bmp_cache")
        .help("Active/unactive enable persistent disk bitmap cache")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_persistent_disk_bitmap_cache = state; })),

        po::option("cache_wait_list")
        .help("Active/unactive enable_cache_waiting_list")
        .action(po::on_off([&](bool state){ mod_rdp_params.enable_cache_waiting_list = state; })),

        po::option("persist_bmp_disk")
        .help("Active/unactive persist bitmap cache on disk")
        .action(po::on_off([&](bool state){ mod_rdp_params.persist_bitmap_cache_on_disk = state; })),

        po::option("bogus_size")
        .help("Active/unactive bogus sc net size")
        .action(po::on_off([&](bool state){ mod_rdp_params.bogus_sc_net_size = state; })),

        po::option("bogus_rectc")
        .help("Active/unactive bogus refresh rect")
        .action(po::on_off([&](bool state){ mod_rdp_params.bogus_refresh_rect = state; })),

        po::option("multi_mon")
        .help("Active/unactive allow using multiple monitors")
        .action(po::on_off([&](bool state){ mod_rdp_params.allow_using_multiple_monitors = state; })),

        po::option("adj_perf_rec")
        .help("Active/unactive adjust performance flags for recording")
        .action(po::on_off([&](bool state){ mod_rdp_params.adjust_performance_flags_for_recording = state; })),

        po::option("outpath")
        .help("Set path where connection time will be written")
        .action(po::arg("path", [&](std::string s){ out_path = std::move(s); }))
    );

    switch (auto r = parser.parse(argc, argv)) {
        case po::Res::Ok:
            break;
        case po::Res::Exit:
            return 0;
        case po::Res::Help:
            parser.help(std::cout);
            return 0;
        case po::Res::BadFormat:
        case po::Res::BadOption:
            std::cerr << "Bad " << ( r == po::Res::BadFormat ? "format" : "option") << " at " << parser.opti;
            if (parser.opti < parser.argc) {
                std::cerr << " (" << parser.argv[parser.opti] << ")";
            }
            std::cerr << "\n";
            return 1;
    }

    if (verbose != 0) {

        std::cout << "\n";
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
    }

    if (verbose & TestClientCLI::SHOW_USER_AND_TARGET_PARAMS) {

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

    if (verbose & TestClientCLI::SHOW_MOD_RDP_PARAMS) {
        std::cout << " ================================" << "\n";
        std::cout << " ======= ModRDP Parameters ======" << "\n";
        std::cout << " ================================" << "\n";

        std::cout << " enable_tls = " << mod_rdp_params.enable_tls << "\n";
        std::cout << " enable_nla = " << mod_rdp_params.enable_nla << "\n";
        std::cout << " enable_fastpath = " << mod_rdp_params.enable_fastpath << "\n";
        std::cout << " enable_mem3blt = " << mod_rdp_params.enable_mem3blt << "\n";
        std::cout << " enable_new_pointer = " << mod_rdp_params.enable_new_pointer << "\n";
        std::cout << " server_redirection_support = " << mod_rdp_params.server_redirection_support << "\n";
        std::cout << " enable_krb = " << mod_rdp_params.enable_krb << "\n";
        std::cout << " enable_glyph_cache = " << mod_rdp_params.enable_glyph_cache << "\n";
        std::cout << " enable_session_probe = " << mod_rdp_params.enable_session_probe << "\n";
        std::cout << " session_probe_enable_launch_mask = t" << mod_rdp_params.session_probe_enable_launch_mask << "\n";
        std::cout << " disable_clipboard_log_syslog = " << mod_rdp_params.disable_clipboard_log_syslog << "\n";
        std::cout << " disable_clipboard_log_wrm = " << mod_rdp_params.disable_clipboard_log_wrm << "\n";
        std::cout << " disable_file_system_log_syslog = " << mod_rdp_params.disable_file_system_log_syslog << "\n";
        std::cout << " disable_file_system_log_wrm = " << mod_rdp_params.disable_file_system_log_wrm << "\n";
        std::cout << " session_probe_use_clipboard_based_launcher = " << mod_rdp_params.session_probe_use_clipboard_based_launcher << "\n";
        std::cout << " session_probe_start_launch_timeout_timer_only_after_logon = " << mod_rdp_params.session_probe_start_launch_timeout_timer_only_after_logon << "\n";
        std::cout << " session_probe_on_keepalive_timeout_disconnect_user = " << mod_rdp_params.session_probe_on_keepalive_timeout_disconnect_user << "\n";
        std::cout << " session_probe_end_disconnected_session = " << mod_rdp_params.session_probe_end_disconnected_session << "\n";
        std::cout << " session_probe_customize_executable_name = " << mod_rdp_params.session_probe_customize_executable_name << "\n";
        std::cout << " enable_transparent_mode = " << mod_rdp_params.enable_transparent_mode << "\n";
        std::cout << " ignore_auth_channel = " << mod_rdp_params.ignore_auth_channel << "\n";
        std::cout << " use_client_provided_alternate_shell = " << mod_rdp_params.use_client_provided_alternate_shell << "\n";
        std::cout << " disconnect_on_logon_user_change = " << mod_rdp_params.disconnect_on_logon_user_change << "\n";
        std::cout << " server_cert_store = " << mod_rdp_params.server_cert_store << "\n";
        std::cout << " hide_client_name = " << mod_rdp_params.hide_client_name << "\n";
        std::cout << " enable_persistent_disk_bitmap_cache = t" << mod_rdp_params.enable_persistent_disk_bitmap_cache << "\n";
        std::cout << " enable_cache_waiting_list = " << mod_rdp_params.enable_cache_waiting_list << "\n";
        std::cout << " persist_bitmap_cache_on_disk = " << mod_rdp_params.persist_bitmap_cache_on_disk << "\n";
        std::cout << " bogus_sc_net_size = " << mod_rdp_params.bogus_sc_net_size << "\n";
        std::cout << " bogus_refresh_rect = " << mod_rdp_params.bogus_refresh_rect << "\n";
        std::cout << " allow_using_multiple_monitors = " << mod_rdp_params.allow_using_multiple_monitors << "\n";
        std::cout << " adjust_performance_flags_for_recording = " << mod_rdp_params.adjust_performance_flags_for_recording << "\n";
        std::cout << "\n" << std::endl;
    } //======================================================================

    NullAuthentifier authentifier;
    NullReportMessage report_message;
    TestClientCLI front(info, report_message, verbose);
    front.out_path = out_path;
    int main_return = 40;

    if (input_connection_data_complete & TestClientCLI::IP) {
        // std::cout << " ================================" << "\n";
        // std::cout << " ======= Connection steps =======" << "\n";
        // std::cout << " ================================" << "\n";

        // Exception handler (pretty message)
        static std::terminate_handler old_terminate_handler =
        std::set_terminate([]{
            auto eptr = std::current_exception();
            try {
                if (eptr) {
                    std::rethrow_exception(eptr);
                }
            } catch(const Error& e) {
                std::cerr << e.errmsg() << "\n";
            } catch(...) {
            }
            old_terminate_handler();
        });

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
            sigaction(SIGPIPE, &sa, nullptr);
            REDEMPTION_DIAGNOSTIC_POP
        }


        int const sck = ip_connect(ip.c_str(), port, nbTry, retryDelay);
        if (sck <= 0) {
            std::cerr << "ip_connect: Cannot connect to [" << ip << "]." << std::endl;
            return 1;
        }

        unique_fd auto_close_sck{sck};

        std::string error_message;
        SocketTransport socket(
            userName.c_str()
          , sck
          , ip.c_str()
          , port
          , to_verbose_flags(verbose)
          , &error_message
        );

        std::cout << " Connected to [" << ip <<  "]." << std::endl;

        LCGRandom gen(0); // To always get the same client random, in tests
        TimeSystem timeSystem;

        // for VNC
        NullReportMessage reportMessage;
        Theme      theme;

        front.connection_time = tvtime();
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

        mod_api * mod;
        GCC::UserData::SCCore const original_sc_core;
        GCC::UserData::SCSecurity const original_sc_sec1;

        non_null_ptr<GCC::UserData::SCCore const> sc_core_ptr = &original_sc_core;
        non_null_ptr<GCC::UserData::SCSecurity const> sc_sec1_ptr = &original_sc_sec1;

        if (protocol_is_VNC) {

            mod = new mod_vnc( socket
                            , userName.c_str()
                            , userPwd.c_str()
                            , front
                            , info.width
                            , info.height
                            , ini.get<cfg::font>()
                            , ""
                            , ""
                            , theme
                            , info.keylayout
                            , 0
                            , true
                            , true
                            , "0,1,-239"
                            , false
                            , true
                            , mod_vnc::ClipboardEncodingType::UTF8
                            , VncBogusClipboardInfiniteLoop::delayed
                            , reportMessage
                            , false
                            , to_verbose_flags(verbose));

        } else {
            auto * rdp = new mod_rdp (
                        socket
                        , front
                        , info
                        , ini.get_ref<cfg::mod_rdp::redir_info>()
                        , gen
                        , timeSystem
                        , mod_rdp_params
                        , authentifier
                        , report_message
                        , ini
                        );
            mod = rdp;

            GCC::UserData::CSSecurity & cs_security = rdp->cs_security;
            cs_security.encryptionMethods = encryptionMethods;

            sc_core_ptr = &rdp->sc_core;
            sc_sec1_ptr = &rdp->sc_sec1;
        }

        front._to_server_sender._callback = mod;
        front._callback = mod;

        try {
            while (!mod->is_up_and_running()) {
                // std::cout << " Early negociations...\n";
                if (int err = wait_and_draw_event(sck, *mod, front, {3, 0})) {
                    return err;
                }
            }
        } catch (const Error & e) {
            std::cout << " Error: Failed during RDP early negociations step. " << e.errmsg() << "\n";
            if (error_message.size()) {
                std::cout << " Error tls: " << error_message << "\n";
            }
            return 2;
        }
        std::cout << " Early negociations completes.\n";


        if (verbose & TestClientCLI::SHOW_CORE_SERVER_INFO && !protocol_is_VNC) {
            std::cout << " ================================" << "\n";
            std::cout << " ======= Server Core Info =======" << "\n";
            std::cout << " ================================" << "\n";

            std::cout << " userDataType = " << sc_core_ptr->userDataType << "\n";
            std::cout << " length = " << sc_core_ptr->length << "\n";
            std::cout << " version = " << sc_core_ptr->version << "\n";
            std::cout << " clientRequestedProtocols = " << sc_core_ptr->clientRequestedProtocols << "\n";
            std::cout << " earlyCapabilityFlags = " << sc_core_ptr->earlyCapabilityFlags << "\n";
            std::cout << std::endl;
        }

        if (verbose & TestClientCLI::SHOW_SECURITY_SERVER_INFO && !protocol_is_VNC) {
            std::cout << " ================================" << "\n";
            std::cout << " ===== Server Security Info =====" << "\n";
            std::cout << " ================================" << "\n";

            std::cout << " userDataType = " << sc_sec1_ptr->userDataType << "\n";
            std::cout << " length = " << sc_sec1_ptr->length << "\n";
            std::cout << " encryptionMethod = " << GCC::UserData::SCSecurity::get_encryptionMethod_name(sc_sec1_ptr->encryptionMethod) << "\n";
            std::cout << " encryptionLevel = " << GCC::UserData::SCSecurity::get_encryptionLevel_name(sc_sec1_ptr->encryptionLevel) << "\n";
            std::cout << " serverRandomLen = " << sc_sec1_ptr->serverRandomLen << "\n";
            std::cout << " serverCertLen = " << sc_sec1_ptr->serverCertLen << "\n";
            std::cout << " dwVersion = " << sc_sec1_ptr->dwVersion << "\n";
            std::cout << " temporary = " << sc_sec1_ptr->temporary << "\n";

            auto print_hex_data = [&sc_sec1_ptr](array_view_const_u8 av){
                for (size_t i = 0; i < av.size(); i++) {
                    if ((i % 16) == 0 && i != 0) {
                        std::cout << "\n                ";
                    }
                    std::cout <<"0x";
                    if (av[i] < 0x10) {
                        std::cout << "0";
                    }
                    std::cout << std::hex << int(sc_sec1_ptr->serverRandom[i]) << std::dec << " ";
                }
                std::cout << "\n";
                std::cout << "\n";
            };

            std::cout << " serverRandom : "; print_hex_data(sc_sec1_ptr->serverRandom);
            std::cout << " pri_exp : "; print_hex_data(sc_sec1_ptr->pri_exp);
            std::cout << " pub_sig : "; print_hex_data(sc_sec1_ptr->pub_sig);

            std::cout << " proprietaryCertificate : " << "\n";
            std::cout << "     dwSigAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwSigAlgId << "\n";
            std::cout << "     dwKeyAlgId = " << sc_sec1_ptr->proprietaryCertificate.dwKeyAlgId << "\n";
            std::cout << "     wPublicKeyBlobType = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobType << "\n";
            std::cout << "     wPublicKeyBlobLen = " << sc_sec1_ptr->proprietaryCertificate.wPublicKeyBlobLen << "\n";
            std::cout << "\n";
            std::cout << "     RSAPK : " << "\n";
            std::cout << "        magic = " << sc_sec1_ptr->proprietaryCertificate.RSAPK.magic << "\n";
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

                    } else if (tag == "key") {
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
                std::cerr <<  "Can't find " << script_file_path << "\n";
            }
        }

        if ((input_connection_data_complete & TestClientCLI::LOG_COMPLETE) || quick_connection_test) {
            try {
                main_return = run_mod(*mod, front, sck, eventList, quick_connection_test, time_out_response, time_set_connection_test);
                // std::cout << "RDP Headless end." <<  std::endl;
            }
            catch (Error const & e)
            {
                if (report_message.is_closed) {
                    main_return = 0;
                }
                else {
                    std::cerr << e.errmsg() << std::endl;
                }
            }
        }

        front.disconnect();
        if (!report_message.is_closed) {
            mod->disconnect(tvtime().tv_sec);
        }
    }

    return main_return;
}


int run_mod(mod_api & mod, TestClientCLI & front, int sck, EventList & /*al*/, bool quick_connection_test, std::chrono::milliseconds time_out_response, bool time_set_connection_test) {
    const timeval time_stop = addusectimeval(time_out_response, tvtime());
    const timeval time_mark = { 0, 50000 };

    while (front.is_pipe_ok)
    {
        if (mod.logged_on == mod_api::CLIENT_LOGGED) {
            mod.logged_on = mod_api::CLIENT_UNLOGGED;

            std::cout << " RDP Session Log On." << std::endl;
            if (quick_connection_test) {
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

        if (int err = wait_and_draw_event(sck, mod, front, time_mark)) {
            return err;
        }

        // if (front.is_running()) {
        //     al.emit();
        // }
    }

    return 0;
}


int wait_and_draw_event(int sck, mod_api & mod, FrontAPI & front, timeval timeout)
{
    unsigned max = 0;
    fd_set   rfds;

    io_fd_zero(rfds);

    auto & event = mod.get_event();
    event.wait_on_fd(sck, rfds, max, timeout);

    int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);
    // std::cout << "RDP CLIENT :: select num = " <<  num << "\n";

    if (num < 0) {
        if (errno == EINTR) {
            return 0;
            //continue;
        }

        std::cerr << "RDP CLIENT :: errno = " <<  errno << "\n";
        return 9;
    }

    if (event.is_set(sck, rfds)) {
        mod.draw_event(time(nullptr), front);
    }

    return 0;
}
