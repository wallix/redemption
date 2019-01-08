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
   Author(s): Clément Moroldo, David Fort
*/

#include "client_redemption/client_config/client_redemption_config.hpp"



ClientRedemptionConfig::ClientRedemptionConfig(RDPVerbose verbose, const std::string &MAIN_DIR )
: MAIN_DIR((MAIN_DIR.empty() || MAIN_DIR == "/")
            ? MAIN_DIR
            : (MAIN_DIR.back() == '/')
            ? MAIN_DIR.substr(0, MAIN_DIR.size() - 1)
            : MAIN_DIR)
, verbose(verbose)
{}



time_t ClientConfig::get_movie_time_length(const char * mwrm_filename) {
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

void ClientConfig::openWindowsData(ClientRedemptionConfig & config)  {
    unique_fd file = unique_fd(config.WINDOWS_CONF.c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    if(file.is_open()) {
        config.windowsData.no_data = false;

        std::string line;
        int pos = 0;

        read_line(file.fd(), line);
        pos = line.find(' ');
        line = line.substr(pos, line.length());
        config.windowsData.form_x = std::stoi(line);

        read_line(file.fd(), line);
        pos = line.find(' ');
        line = line.substr(pos, line.length());
        config.windowsData.form_y = std::stoi(line);

        read_line(file.fd(), line);
        pos = line.find(' ');
        line = line.substr(pos, line.length());
        config.windowsData.screen_x = std::stoi(line);

        read_line(file.fd(), line);
        pos = line.find(' ');
        line = line.substr(pos, line.length());
        config.windowsData.screen_y = std::stoi(line);
    }
}

void ClientConfig::writeWindowsData(WindowsData & config)
{
    unique_fd fd(config.config_file_path.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd.is_open()) {
        std::string info = str_concat(
            "form_x ", std::to_string(config.form_x), "\n"
            "form_y ", std::to_string(config.form_y), "\n"
            "screen_x ", std::to_string(config.screen_x), "\n"
            "screen_y ", std::to_string(config.screen_y), '\n');

        ::write(fd.fd(), info.c_str(), info.length());
    }
}

void ClientConfig::parse_options(int argc, char const* const argv[], ClientRedemptionConfig & config)
{
     auto options = cli::options(
        cli::helper("Client ReDemPtion Help menu."),

        cli::option('h', "help").help("Show help")
        .action(cli::help),

        cli::option('v', "version").help("Show version")
        .action(cli::quit([]{ std::cout << redemption_info_version() << "\n"; })),

        cli::helper("========= Connection ========="),

        cli::option('u', "username").help("Set target session user name")
        .action(cli::arg([&config](std::string s) {
            config.user_name = std::move(s);
            config.connection_info_cmd_complete |= ClientRedemptionConfig::NAME_GOT;
        })),

        cli::option('p', "password").help("Set target session user password")
        .action(cli::arg([&config](std::string s) {
            config.user_password = std::move(s);
            config.connection_info_cmd_complete |= ClientRedemptionConfig::PWD_GOT;
        })),

        cli::option('i', "ip").help("Set target IP address")
        .action(cli::arg([&config](std::string s) {
            config.target_IP = std::move(s);
            config.connection_info_cmd_complete |= ClientRedemptionConfig::IP_GOT;
        })),

        cli::option('P', "port").help("Set port to use on target")
        .action(cli::arg([&config](int n) {
            config.port = n;
            config.connection_info_cmd_complete |= ClientRedemptionConfig::PORT_GOT;
        })),

        cli::helper("========= Verbose ========="),

        cli::option("rdpdr").help("Active rdpdr logs")
        .action(cli::on_off_bit_location<RDPVerbose::rdpdr>(config.verbose)),

        cli::option("rdpsnd").help("Active rdpsnd logs")
        .action(cli::on_off_bit_location<RDPVerbose::rdpsnd>(config.verbose)),

        cli::option("cliprdr").help("Active cliprdr logs")
        .action(cli::on_off_bit_location<RDPVerbose::cliprdr>(config.verbose)),

        cli::option("graphics").help("Active graphics logs")
        .action(cli::on_off_bit_location<RDPVerbose::graphics>(config.verbose)),

        cli::option("printer").help("Active printer logs")
        .action(cli::on_off_bit_location<RDPVerbose::printer>(config.verbose)),

        cli::option("rdpdr-dump").help("Actives rdpdr logs and dump brute rdpdr PDU")
        .action(cli::on_off_bit_location<RDPVerbose::rdpdr_dump>(config.verbose)),

        cli::option("cliprd-dump").help("Actives cliprdr logs and dump brute cliprdr PDU")
        .action(cli::on_off_bit_location<RDPVerbose::cliprdr_dump>(config.verbose)),

        cli::option("basic-trace").help("Active basic-trace logs")
        .action(cli::on_off_bit_location<RDPVerbose::basic_trace>(config.verbose)),

        cli::option("connection").help("Active connection logs")
        .action(cli::on_off_bit_location<RDPVerbose::connection>(config.verbose)),

        cli::option("rail-order").help("Active rail-order logs")
        .action(cli::on_off_bit_location<RDPVerbose::rail_order>(config.verbose)),

        cli::option("asynchronous-task").help("Active asynchronous-task logs")
        .action(cli::on_off_bit_location<RDPVerbose::asynchronous_task>(config.verbose)),

        cli::option("capabilities").help("Active capabilities logs")
        .action(cli::on_off_bit_location<RDPVerbose::capabilities>(config.verbose)),

        cli::option("rail").help("Active rail logs")
        .action(cli::on_off_bit_location<RDPVerbose::rail>(config.verbose)),

        cli::option("rail-dump").help("Actives rail logs and dump brute rail PDU")
        .action(cli::on_off_bit_location<RDPVerbose::rail_dump>(config.verbose)),


        cli::helper("========= Protocol ========="),

        cli::option("vnc").help("Set connection mod to VNC")
        .action([&config]() {
            config.mod_state = ClientRedemptionConfig::MOD_VNC;
            if (!bool(config.connection_info_cmd_complete & ClientRedemptionConfig::PORT_GOT)) {
                config.port = 5900;
            }
        }),

        cli::option("rdp").help("Set connection mod to RDP (default).")
        .action([&config]() {
            config.mod_state = ClientRedemptionConfig::MOD_RDP;
            config.port = 3389;
        }),

        cli::option("remote-app").help("Connection as remote application.")
        .action(cli::on_off_bit_location<ClientRedemptionConfig::MOD_RDP_REMOTE_APP>(config.mod_state)),

        cli::option("remote-exe").help("Connection as remote application and set the line command.")
        .action(cli::arg("command", [&config](std::string line) {
            config.mod_state = ClientRedemptionConfig::MOD_RDP_REMOTE_APP;
            config.modRDPParamsData.enable_shared_remoteapp = true;
            auto pos(line.find(' '));
            if (pos == std::string::npos) {
                config.rDPRemoteAppConfig.source_of_ExeOrFile = line;
                config.rDPRemoteAppConfig.source_of_Arguments.clear();
            }
            else {
                config.rDPRemoteAppConfig.source_of_ExeOrFile = line.substr(0, pos);
                config.rDPRemoteAppConfig.source_of_Arguments = line.substr(pos + 1);
            }
        })),

        cli::option("span").help("Span the screen size on local screen")
        .action(cli::on_off_location(config.is_spanning)),

        cli::option("enable-clipboard").help("Enable clipboard sharing")
        .action(cli::on_off_location(config.enable_shared_clipboard)),

        cli::option("enable-nla").help("Entable NLA protocol")
        .action(cli::on_off_location(config.modRDPParamsData.enable_nla)),

        cli::option("enable-tls").help("Enable TLS protocol")
        .action(cli::on_off_location(config.modRDPParamsData.enable_tls)),

        cli::option("enable-sound").help("Enable sound")
        .action(cli::on_off_location(config.modRDPParamsData.enable_sound)),

        cli::option("enable-fullwindowdrag").help("Enable full window draging")
        .action(cli::on_off_bit_location<~PERF_DISABLE_FULLWINDOWDRAG>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-menuanimations").help("Enable menu animations")
        .action(cli::on_off_bit_location<~PERF_DISABLE_MENUANIMATIONS>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-theming").help("Enable theming")
        .action(cli::on_off_bit_location<~PERF_DISABLE_THEMING>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-cursor-shadow").help("Enable cursor shadow")
        .action(cli::on_off_bit_location<~PERF_DISABLE_CURSOR_SHADOW>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-cursorsettings").help("Enable cursor settings")
        .action(cli::on_off_bit_location<~PERF_DISABLE_CURSORSETTINGS>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-font-smoothing").help("Enable font smoothing")
        .action(cli::on_off_bit_location<PERF_ENABLE_FONT_SMOOTHING>(
            config.info.rdp5_performanceflags)),

        cli::option("enable-desktop-composition").help("Enable desktop composition")
        .action(cli::on_off_bit_location<PERF_ENABLE_DESKTOP_COMPOSITION>(
            config.info.rdp5_performanceflags)),

        cli::option("vnc-applekeyboard").help("Set keyboard compatibility mod with apple VNC server")
        .action(cli::on_off_location(config.modVNCParamsData.is_apple)),

        cli::option("keep_alive_frequence")
        .help("Set timeout to send keypress to keep the session alive")
        .action(cli::arg([&](int t){ config.keep_alive_freq = t; })),


        cli::helper("========= Client ========="),

        cli::option("width").help("Set screen width")
        .action(cli::arg_location(config.rdp_width)),

        cli::option("height").help("Set screen height")
        .action(cli::arg_location(config.rdp_height)),

        cli::option("bpp").help("Set bit per pixel (8, 15, 16, 24)")
        .action(cli::arg("bit_per_pixel", [&config](int x) {
            config.info.screen_info.bpp = checked_int(x);
        })),

        cli::option("keylaout").help("Set windows keylayout")
        .action(cli::arg_location(config.info.keylayout)),

        cli::option("enable-record").help("Enable session recording as .wrm movie")
        .action(cli::on_off_location(config.is_recording)),

        cli::option("persist").help("Set connection to persist")
        .action([&config]() {
            config.quick_connection_test = false;
            config.persist = true;
        }),

        cli::option("timeout").help("Set timeout response before to disconnect in milisecond")
        .action(cli::arg("time", [&](long time) {
            config.quick_connection_test = false;
            config.time_out_disconnection = std::chrono::milliseconds(time);
        })),

        cli::option("share-dir").help("Set directory path on local disk to share with your session.")
        .action(cli::arg("directory", [&config](std::string s) {
            config.modRDPParamsData.enable_shared_virtual_disk = !s.empty();
            config.SHARE_DIR = std::move(s);
        })),

        cli::option("remote-dir").help("Remote working directory")
        .action(cli::arg_location("directory", config.rDPRemoteAppConfig.source_of_WorkingDir))
    );

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            // TODO return 0;
            break;
        case cli::Res::Help:
            config.help_mode = true;
            cli::print_help(options, std::cout);
            // TODO return 0;
            break;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            // TODO return 1;
            break;
    }
    if (bool(RDPVerbose::rail & config.verbose)) {
        config.verbose = config.verbose | RDPVerbose::rail_order;
    }
}


void ClientConfig::writeCustomKeyConfig(ClientRedemptionConfig & config)  {
    const std::string KEY_SETTING_PATH(config.MAIN_DIR + CLIENT_REDEMPTION_KEY_SETTING_PATH);
    unique_fd fd = unique_fd(KEY_SETTING_PATH.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    if(fd.is_open()) {
        std::string to_write = "Key Setting\n\n";

        for (KeyCustomDefinition & key : config.keyCustomDefinitions) {
            if (key.qtKeyID != 0) {
                str_append(
                    to_write,
                    "- ",
                    std::to_string(key.qtKeyID), ' ',
                    std::to_string(key.scanCode), ' ',
                    key.ASCII8, ' ',
                    std::to_string(key.extended), ' ',
                    key.name, '\n'
                );
            }
        }

        ::write(fd.fd(), to_write.c_str(), to_write.length());
    }
}



bool ClientConfig::read_line(const int fd, std::string & line) {
    line = "";
    if (fd < 0) {
        return false;
    }
    char c[2] = {'\0', '\0'};
    int size = -1;
    while (c[0] != '\n' && size !=  0) {
        size_t size = ::read(fd, c, 1);
        if (size == 1) {
            if (c[0] == '\n') {
                return true;
            } else {
                line += c[0];
            }
        } else {
            return false;
        }
    }
    return false;
}

void ClientConfig::set_config(int argc, char const* const argv[], ClientRedemptionConfig & config) {
    ClientConfig::setDefaultConfig(config);
    if (!config.MAIN_DIR.empty()) {
        ClientConfig::setUserProfil(config);
        ClientConfig::setClientInfo(config);
        ClientConfig::setCustomKeyConfig(config);
        ClientConfig::setAccountData(config);
        ClientConfig::openWindowsData(config);
    }
    ClientConfig::parse_options(argc, argv, config);
}

void ClientConfig::setAccountData(ClientRedemptionConfig & config)  {
    config._accountNB = 0;

    unique_fd fd = unique_fd(config.USER_CONF_LOG.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

    if (fd.is_open()) {
        int accountNB(0);
        std::string line;

        while(read_line(fd.fd(), line)) {
            auto pos(line.find(' '));
            std::string info = line.substr(pos + 1);

            if (line.compare(0, pos, "save_pwd") == 0) {
                config._save_password_account = (info == "true");
            } else
            if (line.compare(0, pos, "last_target") == 0) {
                config._last_target_index = std::stoi(info);
            } else
            if (line.compare(0, pos, "title") == 0) {
                AccountData new_account;
                config._accountData.push_back(new_account);
                config._accountData.back().title = info;
            } else
            if (line.compare(0, pos, "IP") == 0) {
                config._accountData.back().IP = info;
            } else
            if (line.compare(0, pos, "name") == 0) {
                config._accountData.back().name = info;
            } else if (line.compare(0, pos, "protocol") == 0) {
                config._accountData.back().protocol = std::stoi(info);
            } else
            if (line.compare(0, pos, "pwd") == 0) {
                config._accountData.back().pwd = info;
            } else
            if (line.compare(0, pos, "options_profil") == 0) {

                config._accountData.back().options_profil = std::stoi(info);
                config._accountData.back().index = accountNB;

                accountNB++;
            } else
            if (line.compare(0, pos, "port") == 0) {
                config._accountData.back().port = std::stoi(info);
            }

            line = "";
        }

        if (config._accountNB < int(config._accountData.size())) {
            config._accountNB = accountNB;
        }
    }
}




void ClientConfig::writeAccoundData(const std::string& ip, const std::string& name, const std::string& pwd, const int port, ClientRedemptionConfig & config)  {
    if (config.connected) {
        bool alreadySet = false;

        std::string title(ip + " - " + name);

        for (int i = 0; i < config._accountNB; i++) {
            if (config._accountData[i].title == title) {
                alreadySet = true;
                config._last_target_index = i;
                config._accountData[i].pwd  = pwd;
                config._accountData[i].port = port;
                config._accountData[i].options_profil  = config.current_user_profil;
            }
        }

        if (!alreadySet) {
            AccountData new_account;
            config._accountData.push_back(new_account);
            config._accountData[config._accountNB].title = title;
            config._accountData[config._accountNB].IP    = ip;
            config._accountData[config._accountNB].name  = name;
            config._accountData[config._accountNB].pwd   = pwd;
            config._accountData[config._accountNB].port  = port;
            config._accountData[config._accountNB].options_profil  = config.current_user_profil;
            config._accountData[config._accountNB].protocol = config.mod_state;
            config._accountNB++;

            config._last_target_index = config._accountNB;
        }

        unique_fd file = unique_fd(config.USER_CONF_LOG.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        if(file.is_open()) {

            std::string to_write = str_concat(
                (config._save_password_account ? "save_pwd true\n" : "save_pwd false\n"),
                "last_target ",
                std::to_string(config._last_target_index),
                "\n\n");

            for (int i = 0; i < config._accountNB; i++) {
                str_append(
                    to_write,
                    "title ", config._accountData[i].title, "\n"
                    "IP "   , config._accountData[i].IP   , "\n"
                    "name " , config._accountData[i].name , "\n"
                    "protocol ", std::to_string(config._accountData[i].protocol), '\n');

                if (config._save_password_account) {
                    str_append(to_write, "pwd ", config._accountData[i].pwd, "\n");
                } else {
                    to_write += "pwd \n";
                }

                str_append(
                    to_write,
                    "port ", std::to_string(config._accountData[i].port), "\n"
                    "options_profil ", std::to_string(config._accountData[i].options_profil), "\n"
                    "\n");
            }

            ::write(file.fd(), to_write.c_str(), to_write.length());
        }
    }
}


void ClientConfig::deleteCurrentProtile(ClientRedemptionConfig & config)  {
    unique_fd file_to_read = unique_fd(config.USER_CONF_PATH.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if(file_to_read.is_open()) {

        std::string new_file_content = "current_user_profil_id 0\n";
        int ligne_to_jump = 0;

        std::string line;

        ClientConfig::read_line(file_to_read.fd(), line);

        while(ClientConfig::read_line(file_to_read.fd(), line)) {
            if (ligne_to_jump == 0) {
                int pos = line.find(' ');
                std::string info = line.substr(pos + 1);

                if (line.compare(0, pos, "id") == 0 && std::stoi(info) == config.current_user_profil) {
                    ligne_to_jump = 18;
                } else {
                    str_append(new_file_content, line, '\n');
                }
            } else {
                ligne_to_jump--;
            }
        }

        file_to_read.close();

        unique_fd file_to_read = unique_fd(config.USER_CONF_PATH.c_str(), O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
            ::write(file_to_read.fd(), new_file_content.c_str(), new_file_content.length());
    }
}



void ClientConfig::writeClientInfo(ClientRedemptionConfig & config)  {

    unique_fd file = unique_fd(config.USER_CONF_PATH.c_str(), O_WRONLY| O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

    if(file.is_open()) {

        std::string to_write = str_concat(
            "current_user_profil_id ", std::to_string(config.current_user_profil), '\n');

        bool not_reading_current_profil = true;
        std::string ligne;
        while(read_line(file.fd(), ligne)) {
            if (!ligne.empty()) {
                std::size_t pos = ligne.find(' ');

                if (ligne.compare(pos+1, std::string::npos, "id") == 0) {
                    to_write += '\n';
                    int read_id = std::stoi(ligne);
                    not_reading_current_profil = !(read_id == config.current_user_profil);
                }

                if (not_reading_current_profil) {
                    str_append(to_write, '\n', ligne);
                }

                ligne.clear();
            }
        }

        str_append(
            to_write,
            "\nid ", std::to_string(config.userProfils[config.current_user_profil].id), "\n"
            "name ", config.userProfils[config.current_user_profil].name, "\n"
            "keylayout ", std::to_string(config.info.keylayout), "\n"
            "brush_cache_code ", std::to_string(config.info.brush_cache_code), "\n"
            "bpp ", std::to_string(static_cast<int>(config.info.screen_info.bpp)), "\n"
            "width ", std::to_string(config.rdp_width), "\n"
            "height ", std::to_string(config.rdp_height), "\n"
            "rdp5_performanceflags ", std::to_string(static_cast<int>(config.info.rdp5_performanceflags)), "\n"
            "monitorCount ", std::to_string(config.info.cs_monitor.monitorCount), "\n"
            "span ", std::to_string(config.is_spanning), "\n"
            "record ", std::to_string(config.is_recording),"\n"
            "tls ", std::to_string(config.modRDPParamsData.enable_tls), "\n"
            "nla ", std::to_string(config.modRDPParamsData.enable_nla), "\n"
            "sound ", std::to_string(config.modRDPParamsData.enable_sound), "\n"
            "console_mode ", std::to_string(config.info.console_session), "\n"
            "enable_shared_clipboard ", std::to_string(config.enable_shared_clipboard), "\n"
            "enable_shared_virtual_disk ", std::to_string(config.modRDPParamsData.enable_shared_virtual_disk), "\n"
            "enable_shared_remoteapp ", std::to_string(config.modRDPParamsData.enable_shared_remoteapp), "\n"
            "share-dir ", config.SHARE_DIR, "\n"
            "remote-exe ", config.rDPRemoteAppConfig.full_cmd_line, "\n"
            "remote-dir ", config.rDPRemoteAppConfig.source_of_WorkingDir, "\n"
            "vnc-applekeyboard ", std::to_string(config.modVNCParamsData.is_apple), "\n"
            "mod ", std::to_string(static_cast<int>(config.mod_state)) , "\n"
        );

        ::write(file.fd(), to_write.c_str(), to_write.length());
    }
}



void ClientConfig::setDefaultConfig(ClientRedemptionConfig & config)  {
    //config.current_user_profil = 0;
    config.info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
    config.info.brush_cache_code = 0;
    config.info.screen_info.bpp = BitsPerPixel{24};
    config.info.screen_info.width  = 800;
    config.info.screen_info.height = 600;
    config.info.console_session = false;
    config.info.rdp5_performanceflags = 0;               //PERF_DISABLE_WALLPAPER;
    config.info.cs_monitor.monitorCount = 1;
    config.is_spanning = false;
    config.is_recording = false;
    config.modRDPParamsData.enable_tls = true;
    config.modRDPParamsData.enable_nla = true;
    config.enable_shared_clipboard = true;
    config.modRDPParamsData.enable_shared_virtual_disk = true;
    config.SHARE_DIR = "/home";
    //config.info.encryptionLevel = 1;

    config.info.screen_info.width       = 800;
    config.info.screen_info.height      = 600;
    config.info.keylayout               = 0x040C;            // 0x40C FR, 0x409 USA
    config.info.console_session         = false;
    config.info.brush_cache_code        = 0;
    config.info.screen_info.bpp         = BitsPerPixel{24};
    config.info.rdp5_performanceflags   = PERF_DISABLE_WALLPAPER;
    config.info.cs_monitor.monitorCount = 1;

    config.rDPRemoteAppConfig.source_of_ExeOrFile  = "C:\\Windows\\system32\\notepad.exe";
    config.rDPRemoteAppConfig.source_of_WorkingDir = "C:\\Users\\user1";

    config.rDPRemoteAppConfig.full_cmd_line = config.rDPRemoteAppConfig.source_of_ExeOrFile + " " + config.rDPRemoteAppConfig.source_of_Arguments;

    if (!config.MAIN_DIR.empty()) {
        for (auto* pstr : {
            &config.DATA_DIR,
            &config.REPLAY_DIR,
            &config.CB_TEMP_DIR,
            &config.DATA_CONF_DIR,
            &config.SOUND_TEMP_DIR
//             &config.WINDOWS_CONF
        }) {
            if (!pstr->empty()) {
                if (!file_exist(pstr->c_str())) {
                    LOG(LOG_INFO, "Create file \"%s\".", pstr->c_str());
                    mkdir(pstr->c_str(), 0775);
                }
            }
        }
    }

    // Set RDP CLIPRDR config
    config.rDPClipboardConfig.arbitrary_scale = 40;
    config.rDPClipboardConfig.server_use_long_format_names = true;
    config.rDPClipboardConfig.cCapabilitiesSets = 1;
    config.rDPClipboardConfig.generalFlags = RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS;
    config.rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW, RDPECLIP::FILEGROUPDESCRIPTORW.data());
    config.rDPClipboardConfig.add_format(ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS, RDPECLIP::FILECONTENTS.data());
    config.rDPClipboardConfig.add_format(RDPECLIP::CF_TEXT, {});
    config.rDPClipboardConfig.add_format(RDPECLIP::CF_METAFILEPICT, {});
    config.rDPClipboardConfig.path = config.CB_TEMP_DIR;

    // Set RDP RDPDR config
    config.rDPDiskConfig.add_drive(config.SHARE_DIR, rdpdr::RDPDR_DTYP_FILESYSTEM);
    config.rDPDiskConfig.enable_drive_type = true;
    config.rDPDiskConfig.enable_printer_type = true;

    // Set RDP SND config
    config.rDPSoundConfig.dwFlags = rdpsnd::TSSNDCAPS_ALIVE | rdpsnd::TSSNDCAPS_VOLUME;
    config.rDPSoundConfig.dwVolume = 0x7fff7fff;
    config.rDPSoundConfig.dwPitch = 0;
    config.rDPSoundConfig.wDGramPort = 0;
    config.rDPSoundConfig.wNumberOfFormats = 1;
    config.rDPSoundConfig.wVersion = 0x06;

    config.userProfils.emplace_back(0, "Default");

    std::fill(std::begin(config.info.order_caps.orderSupport), std::end(config.info.order_caps.orderSupport), 1);
    config.info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;
}

void ClientConfig::setUserProfil(ClientRedemptionConfig & config)  {
    unique_fd fd = unique_fd(config.USER_CONF_PATH.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd.is_open()) {
        std::string line;
        read_line(fd.fd(), line);
        auto pos(line.find(' '));
        if (line.compare(0, pos, "current_user_profil_id") == 0) {
            config.current_user_profil = std::stoi(line.substr(pos + 1));
        }
    }
}

void ClientConfig::setClientInfo(ClientRedemptionConfig & config)  {
    config.windowsData.config_file_path = config.WINDOWS_CONF;

    config.userProfils.clear();
    config.userProfils.emplace_back(0, "Default");

    unique_fd fd = unique_fd(config.USER_CONF_PATH.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

    if (fd.is_open()) {

        int read_id(-1);
        std::string line;

        while(read_line(fd.fd(), line)) {

            auto pos(line.find(' '));
            std::string info = line.substr(pos + 1);
            std::string tag = line.substr(0, pos);
            if (tag == "id") {
                read_id = std::stoi(info);
            } else
            if (tag == "name") {
                if (read_id) {
                    config.userProfils.emplace_back(read_id, info);
                }
            } else
            if (config.current_user_profil == read_id) {

                if (tag == "keylayout") {
                    config.info.keylayout = std::stoi(info);
                } else
                if (tag == "console_session") {
                    config.info.console_session = std::stoi(info);
                } else
                if (tag == "brush_cache_code") {
                    config.info.brush_cache_code = std::stoi(info);
                } else
                if (tag == "bpp") {
                    config.info.screen_info.bpp = checked_int(std::stoi(info));
                } else
                if (tag == "width") {
                    config.rdp_width     = std::stoi(info);
                } else
                if (tag == "height") {
                    config.rdp_height     = std::stoi(info);
                } else
                if (tag == "monitorCount") {
                    config.info.cs_monitor.monitorCount = std::stoi(info);
                } else
                if (tag == "span") {
                    if (std::stoi(info)) {
                        config.is_spanning = true;
                    } else {
                        config.is_spanning = false;
                    }
                } else
                if (tag == "record") {
                    if (std::stoi(info)) {
                        config.is_recording = true;
                    } else {
                        config.is_recording = false;
                    }
                } else
                if (tag == "tls") {
                    config.modRDPParamsData.enable_tls = bool(std::stoi(info));
                } else
                if (tag == "nla") {
                    config.modRDPParamsData.enable_nla = bool(std::stoi(info));
                } else
                if (tag == "sound") {
                    config.modRDPParamsData.enable_sound = bool(std::stoi(info));
                } else
                if (tag == "console_mode") {
                    config.info.console_session = (std::stoi(info) > 0);
                } else
                if (tag == "enable_shared_clipboard") {
                    config.enable_shared_clipboard = bool(std::stoi(info));
                } else
                if (tag == "enable_shared_remoteapp") {
                    config.modRDPParamsData.enable_shared_remoteapp = bool(std::stoi(info));
                } else
                if (tag == "enable_shared_virtual_disk") {
                    config.modRDPParamsData.enable_shared_virtual_disk = bool(std::stoi(info));
                } else
                if (tag == "share-dir") {
                    config.SHARE_DIR                 = info;
                } else
                if (tag == "remote-exe") {
                    config.rDPRemoteAppConfig.full_cmd_line                = info;
                    auto arfs_pos(info.find(' '));
                    if (arfs_pos == 0) {
                        config.rDPRemoteAppConfig.source_of_ExeOrFile = info;
                        config.rDPRemoteAppConfig.source_of_Arguments.clear();
                    }
                    else {
                        config.rDPRemoteAppConfig.source_of_ExeOrFile = info.substr(0, arfs_pos);
                        config.rDPRemoteAppConfig.source_of_Arguments = info.substr(arfs_pos + 1);
                    }
                } else
                if (tag == "remote-dir") {
                    config.rDPRemoteAppConfig.source_of_WorkingDir                = info;
                } else
                if (tag == "rdp5_performanceflags") {
                    config.info.rdp5_performanceflags = std::stoi(info);
                } else
                if (tag == "vnc-applekeyboard ") {
                    config.modVNCParamsData.is_apple = bool(std::stoi(info));
                } else
                if (tag == "mod") {
                    config.mod_state = std::stoi(info);

                    read_id = -1;
                }
            }

            line = "";
        }
    }
}

void ClientConfig::setCustomKeyConfig(ClientRedemptionConfig & config)  {
    const std::string KEY_SETTING_PATH(config.MAIN_DIR + CLIENT_REDEMPTION_KEY_SETTING_PATH);
    unique_fd fd = unique_fd(KEY_SETTING_PATH.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

    if (fd.is_open()) {
        config.keyCustomDefinitions.clear();

        std::string ligne;

        while(read_line(fd.fd(), ligne)) {

            int pos(ligne.find(' '));

            if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {

                ligne = ligne.substr(pos + 1, ligne.length());
                pos = ligne.find(' ');

                int qtKeyID  = std::stoi(ligne.substr(0, pos));

                if (qtKeyID !=  0) {
                    ligne = ligne.substr(pos + 1, ligne.length());
                    pos = ligne.find(' ');

                    int scanCode = 0;
                    scanCode = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + 1, ligne.length());

                    std::string ASCII8 = ligne.substr(0, 1);
                    int next_pos = 2;
                    if (ASCII8 == " ") {
                        ASCII8 = "";
                        next_pos = 1;
                    }

                    ligne = ligne.substr(next_pos, ligne.length());
                    int extended = std::stoi(ligne.substr(0, 1));

                    if (extended) {
                        extended = 1;
                    }
                    pos = ligne.find(' ');

                    std::string name = ligne.substr(pos + 1, ligne.length());

                    config.keyCustomDefinitions.emplace_back(qtKeyID, scanCode, ASCII8, extended, name);
                }
            }
        }
    }
}

