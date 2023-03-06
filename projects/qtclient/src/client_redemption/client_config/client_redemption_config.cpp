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

#include "capture/cryptofile.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "utils/cli.hpp"
#include "utils/cli_screen_info.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/split.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/finally.hpp"
#include "utils/redemption_info_version.hpp"

#include <iostream>


ClientRedemptionConfig::ClientRedemptionConfig(RDPVerbose verbose, const std::string &MAIN_DIR )
: MAIN_DIR((MAIN_DIR.empty() || MAIN_DIR == "/")
            ? MAIN_DIR
            : (MAIN_DIR.back() == '/')
            ? MAIN_DIR.substr(0, MAIN_DIR.size() - 1)
            : MAIN_DIR)
, verbose(verbose) {}



time_t ClientConfig::get_movie_time_length(const char * mwrm_filename) {
    // TODO RZ: Support encrypted recorded file.

    CryptoContext cctx;
    InCryptoTransport trans(cctx, InCryptoTransport::EncryptionMode::NotEncrypted);
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
        std::array<char, 4096> buffer;
        auto len = ::read(file.fd(), buffer.data(), buffer.size() - 1);
        if (len > 0) {
            buffer[std::size_t(len)] = 0;
            char* s = buffer.data();
            for (int* n : {
                &config.windowsData.form_x,
                &config.windowsData.form_y,
                &config.windowsData.screen_x,
                &config.windowsData.screen_y,
            }) {
                if (!(s = std::strchr(s, ' '))) return;
                *n = std::strtol(s, &s, 10);
            }
        }
    }
}

void ClientConfig::writeWindowsData(WindowsData & config)
{
    unique_fd fd(config.config_file_path.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd.is_open()) {
        std::string info = str_concat(
            "form_x ", int_to_decimal_chars(config.form_x), '\n',
            "form_y ", int_to_decimal_chars(config.form_y), '\n',
            "screen_x ", int_to_decimal_chars(config.screen_x), '\n',
            "screen_y ", int_to_decimal_chars(config.screen_y), '\n');

        ::write(fd.fd(), info.c_str(), info.length());
    }
}

static auto make_options(qtclient::Profile& config)
{
    return cli::options(
        cli::helper("Client ReDemPtion Help menu."),

        cli::option('h', "help").help("Show help")
        .parser(cli::help()),

        cli::option('v', "version").help("Show version")
        .parser(cli::quit([]{ std::cout << redemption_info_version() << "\n"; })),


        cli::helper("========= Connection ========="),

        cli::option('u', "username").help("Set target session user name")
        .parser(cli::arg_location(config.user_name)),

        cli::option('p', "password").help("Set target session user password")
        .parser(cli::arg_location(config.user_password)),

        cli::option('t', "target").help("Set target IP address")
        .parser(cli::arg_location(config.target_address)),

        cli::option('P', "port").help("Set port to use on target")
        .parser(cli::arg_location(config.target_port)),


        cli::helper("========= Protocol ========="),

        cli::option("vnc").help("Set connection mod to VNC")
        .parser(cli::trigger([&config]{
            config.is_rdp_mod = false;
            config.target_port = 5900;
        })),

        cli::option("rdp").help("Set connection mod to RDP (default).")
        .parser(cli::trigger([&config]{
            config.is_rdp_mod = true;
            config.target_port = 3389;
        })),

        cli::option("size").help("Screen size")
        .parser(cli::arg_location(config.screen_info)),

        cli::option("span").help("Span the screen size on local screen")
        .parser(cli::on_off_location(config.is_spanning)),

        cli::option("enable-clipboard").help("Enable clipboard")
        .parser(cli::on_off_location(config.enable_clipboard)),

        cli::option("tls-min-level").help("Minimal TLS protocol level")
        .parser(cli::arg_location(config.tls_min_level)),

        cli::option("tls-max-level").help("Maximal TLS protocol level allowed")
        .parser(cli::arg_location(config.tls_max_level)),

        cli::option("cipher").help("Set TLS Cipher allowed for TLS <= 1.2")
        .parser(cli::arg_location(config.cipher_string)),


        cli::helper("========= RDP ========="),

        cli::option("enable-sound").help("Enable sound")
        .parser(cli::on_off_location(config.enable_sound)),

        cli::option("rdp-performance-flags").help(
            "Set RDP performanceflags "
            "(https://learn.microsoft.com/en-us/windows/win32/termserv/imsrdpclientadvancedsettings-performanceflags)")
        .parser(cli::arg_location(config.rdp5_performance_flags)),

        cli::option("enable-nla").help("Enable NLA protocol")
        .parser(cli::on_off_location(config.enable_nla)),

        cli::option("enable-tls").help("Enable TLS protocol")
        .parser(cli::on_off_location(config.enable_tls)),

        cli::option("rdp-verbose")
        .parser(cli::arg_location(config.rdp_verbose)),


        cli::helper("========= Remote App ========="),

        cli::option("remote-app").help("Connection as remote application.")
        .parser(cli::on_off_location(config.enable_remote_app)),

        cli::option("remote-cmd").help("Set the command line of remote application.")
        .argname("command")
        .parser(cli::arg_location(config.remote_app_cmd)),

        cli::option("remote-dir").help("Remote working directory")
        .argname("directory")
        .parser(cli::arg_location(config.remote_app_working_directory)),


        cli::helper("========= Shared directory App ========="),

        cli::option("enable-drive").help("Enable shared local disk")
        .parser(cli::on_off_location(config.enable_drive)),

        cli::option("drive-dir").help("Set directory path on local disk to share with your session.")
        .argname("directory")
        .parser(cli::arg_location(config.drive_path)),

        cli::option("home-drive").help("Set $HOME to share with your session.")
        .parser(cli::trigger([&config]{
            char const* home = getenv("HOME");
            if (home) {
                config.drive_path = home;
                config.enable_drive = true;
            }
        })),


        cli::helper("========= Client ========="),

        cli::option("layout").help("Set windows keylayout")
        .parser(cli::arg_location(config.key_layout)),

        cli::option("enable-recording").help("Enable session recording as .wrm movie")
        .parser(cli::on_off_location(config.enable_recording))
    );
}

// TODO PERF very inneficient. replace to append_file_contents()
bool ClientConfig::read_line(const int fd, std::string & line) {
    line.clear();
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
            }
            line += c[0];
        }
        else {
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
            if (config._accountData[i].IP == ip && config._accountData[i].name == name) {
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
                int_to_decimal_chars(config._last_target_index),
                "\n\n");

            for (int i = 0; i < config._accountNB; i++) {
                str_append(
                    to_write,
                    "title ", config._accountData[i].title, "\n"
                    "IP "   , config._accountData[i].IP   , "\n"
                    "name " , config._accountData[i].name , "\n"
                    "protocol ", int_to_decimal_chars(config._accountData[i].protocol), '\n');

                if (config._save_password_account) {
                    str_append(to_write, "pwd ", config._accountData[i].pwd, "\n");
                } else {
                    to_write += "pwd \n";
                }

                str_append(
                    to_write,
                    "port ", int_to_decimal_chars(config._accountData[i].port), "\n"
                    "options_profil ", int_to_decimal_chars(config._accountData[i].options_profil), "\n"
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
                std::string::size_type pos = line.find(' ');
                std::string info = line.substr(pos + 1u);

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


void ClientConfig::setUserProfil(ClientRedemptionConfig & config)
{
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

namespace qtclient
{

Profiles::Profiles()
: std::vector<Profile>(1)
{
    front().profile_name = "Default";
}

Profile& Profiles::add_profile(std::string_view name, bool selected)
{
    if (selected) {
        current_index = size();
    }

    auto& profile = emplace_back();
    profile.profile_name = name;
    return profile;
}

bool Profiles::choice_profile(std::size_t id) noexcept
{
    if (id < current_index) {
        current_index = id;
        return true;
    }
    return false;
}

bool Profiles::choice_profile(std::string_view name) noexcept
{
    for (auto const& profile : *this) {
        if (profile.profile_name == name) {
            current_index = checked_int(&profile - data());
            return true;
        }
    }
    return false;
}

Profile* Profiles::find(std::string_view name) noexcept
{
    for (auto& profile : *this) {
        if (profile.profile_name == name) {
            return &profile;
        }
    }
    return nullptr;
}


bool parse_options(int argc, char const* const argv[], Profile& profile)
{
    auto options = make_options(profile);

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            return true;
        case cli::Res::Exit:
            return false;
        case cli::Res::Help:
            cli::print_help(options, std::cout);
            return false;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
        case cli::Res::NotOption:
        case cli::Res::StopParsing:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
    }
    return false;
}

bool save_profiles(char const* filename, Profiles const& profiles)
{
    unique_fd file = unique_fd(filename, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    if (file.is_open()) {
        return false;
    }

    std::string str;
    str.reserve(512);

    str_append(str, "current-profile ", profiles.current_profile().profile_name, '\n');

    auto to_str = [](auto const& value) {
        using T = std::decay_t<decltype(value)>;
        /**/ if constexpr (std::is_same_v<bool, T>) return value ? '1' : '0';
        else if constexpr (std::is_integral_v<T>) return int_to_decimal_chars(value);
        else if constexpr (std::is_enum_v<T>) return int_to_decimal_chars(underlying_cast(value));
        else return value;
    };

    for (auto const& profile : profiles) {
        str_append(
            str, "\n"
            "name ", profile.profile_name, "\n"
            "is_rdp ", to_str(profile.is_rdp_mod), "\n"
            "port ", to_str(profile.target_port), "\n"
            "size ",
                to_str(profile.screen_info.width), 'x',
                to_str(profile.screen_info.height), 'x',
                to_str(profile.screen_info.bpp), "\n"
            "span ", to_str(profile.is_spanning), "\n"
            "enable-clipboard ", to_str(profile.enable_clipboard), "\n"
            "tls-min-level ", to_str(profile.tls_min_level), "\n"
            "tls-max-level ", to_str(profile.tls_max_level), "\n"
            "cipher ", profile.cipher_string, "\n"
            "enable-sound ", to_str(profile.enable_sound),"\n"
            "rdp-performance-flags ", to_str(profile.rdp5_performance_flags), "\n"
            "enable-nla ", to_str(profile.enable_nla), "\n"
            "enable-tls ", to_str(profile.enable_tls), "\n"
            "rdp-verbose ", to_str(profile.rdp_verbose), "\n"
            "remote-app ", to_str(profile.enable_remote_app), "\n"
            "remote-cmd ", profile.remote_app_cmd, "\n"
            "remote-dir ", profile.remote_app_working_directory, "\n"
            "enable-drive ", to_str(profile.enable_drive), "\n"
            "drive-dir ", profile.drive_path, "\n"
            "layout ", to_str(profile.key_layout), "\n"
            "enable-recording ", to_str(profile.enable_recording), "\n"
        );
    }

    chars_view data = str;
    ssize_t n = 0;
    while (0 < (n = ::write(file.fd(), data.data(), data.size()))) {
        data = data.drop_front(checked_int(n));
        if (data.empty()) {
            return true;
        }
    }
    return false;
}

Profiles load_profiles(char const* filename)
{
    Profiles profiles;

    auto* profile = &profiles.front();

    std::string contents;
    switch (append_file_contents(filename, contents, 0xffff)) {
        case FileContentsError::None: {
            if (contents.empty()) {
                break;
            }

            // reserve a new line for '\0' transformation
            if (contents.back() != '\n') {
                contents += '\n';
            }

            auto options = make_options(*profile);

            auto parse_option = [](std::string_view name, cli::ParseResult& pr, auto const& opt, cli::Res& res){
                if constexpr (!std::is_same_v<cli::Helper const&, decltype(opt)>) {
                    if constexpr (!std::is_same_v<cli::detail::uninit_t, decltype(opt._long_name)>) {
                        if (opt._long_name != name) {
                            return true;
                        }
                        #ifndef IN_IDE_PARSER
                        res = opt._parser(pr);
                        #endif
                        return false;
                    }
                }
                return true;
            };

            int iline = 0;
            cli::ParseResult pr {};
            std::string_view current_profile;

            for (auto line : get_lines(contents)) {
                ++iline;

                if (line.empty()) {
                    continue;
                }

                auto p = std::find(line.begin(), line.end(), ' ');

                // replace '\n'
                *line.end() = '\0';
                auto key = chars_view(line.begin(), p).as<std::string_view>();
                auto value = chars_view(p + (line.end() != p), line.end()).as<std::string_view>();

                using namespace std::string_view_literals;

                if (key == "name"sv) {
                    if (!(profile = profiles.find(value))) {
                        profile = &profiles.add_profile(value);
                    }
                    continue;
                }

                if (key == "current-profile"sv) {
                    current_profile = value;
                    continue;
                }

                using cli::Res;
                auto parser = [&](auto const&... opts) {
                    Res r = Res::Ok;
                    pr.str = value.data();
                    return (... && (r == Res::Ok && parse_option(key, pr, opts, r))) ? Res::BadOption : r;
                };

                if (options(parser) != Res::Ok) {
                    LOG(LOG_WARNING, "Parse error %s:%d: \"%s\".", filename, iline, line.data());
                }
            }

            profiles.choice_profile(current_profile);
            break;
        }

        case FileContentsError::Read:
        case FileContentsError::Open:
        case FileContentsError::Stat:
            LOG(LOG_WARNING, "Parse error %s: %s", filename, strerror(errno));
            break;
    }

    return profiles;
}

}
