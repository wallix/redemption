/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "qtclient/profile/profile.hpp"

#include "utils/cli.hpp"
#include "utils/cli_screen_info.hpp"
#include "utils/ascii.hpp"


template<>
struct cli::parsers::argname_value_traits<::qtclient::ProtocolMod>
{
    static constexpr std::string_view default_argname = "<vnc|rdp>";
};

template<>
struct cli::parsers::argname_value_traits<::qtclient::DirectoryStringPath>
{
    static constexpr std::string_view default_argname = "<directory>";
};

template<>
struct cli::arg_parsers::arg_parse_traits<::qtclient::ProtocolMod>
{
    using value_type = ::qtclient::ProtocolMod;

    static Res parse(value_type& out, std::string_view str)
    {
        auto protocol = ascii_to_limited_upper<4>(str);
        if (protocol == "RDP"_ascii_upper || protocol == "0"_ascii_upper) {
            out = value_type::RDP;
            return cli::Res::Ok;
        }

        if (protocol == "VNC"_ascii_upper || protocol == "1"_ascii_upper) {
            out = value_type::VNC;
            return cli::Res::Ok;
        }

        return cli::Res::BadValueFormat;
    }
};

// TODO add cli::arg_parsers::arg_parse_traits<::KeyLayout::KbdId> -> find by name / id

namespace qtclient
{

template<class T>
struct is_serializable_option : std::true_type
{};

template<>
struct is_serializable_option<cli::Helper> : std::false_type
{};

template<class Short, class Long, class Act, class Name>
struct is_serializable_option<cli::Option<Short, Long, cli::parsers::trigger<Act>, Name>> : std::false_type
{};

constexpr auto to_cli_options = [](auto&&... options) {
    return cli::options(static_cast<decltype(options)&&>(options)...);
};

template<class Fn = decltype(to_cli_options) const&>
auto profile_as_cli_options(qtclient::Profile& config, Fn&& fn = to_cli_options)
{
    return fn(
        cli::helper("========= Protocol ========="),

        cli::option('m', "protocol").help("Select protocol for connection").argname("<vnc|rdp>")
        .parser(cli::arg_location(config.protocol)),

        cli::option("rdp").help("Alias for --protocol=rdp")
        .parser(cli::trigger([&config]{ config.protocol = qtclient::ProtocolMod::RDP; })),

        cli::option("vnc").help("Alias for --protocol=vnc")
        .parser(cli::trigger([&config]{ config.protocol = qtclient::ProtocolMod::VNC; })),


        cli::helper("========= Connection ========="),

        cli::option('u', "username").help("Target session user name")
        .parser(cli::arg_location(config.user_name)),

        cli::option('p', "password").help("Target session user password")
        .argname("password")
        .parser(cli::arg_location(config.user_password)),

        cli::option('t', "target").help("Target address")
        .parser(cli::arg_location(config.target_address)),

        cli::option('P', "port").help("Port to use on target")
        .parser(cli::arg_location(config.target_port)),


        cli::helper("========= Client ========="),

        cli::option('s', "size").help("Screen size")
        .parser(cli::arg_location(config.screen_info)),

        cli::option("span").help("Span the screen size on local screen")
        .parser(cli::arg_location(config.is_spanning)),

        cli::option("enable-clipboard").help("Enable clipboard")
        .parser(cli::arg_location(config.enable_clipboard)),

        cli::option("tls-min-level").help("Minimal TLS protocol level")
        .parser(cli::arg_location(config.tls_min_level)),

        cli::option("tls-max-level").help("Maximal TLS protocol level allowed")
        .parser(cli::arg_location(config.tls_max_level)),

        cli::option("cipher").help("TLS Cipher allowed for TLS <= 1.2")
        .parser(cli::arg_location(config.cipher_string)),

        cli::option('r', "enable-recording").help("Enable session recording as .wrm movie")
        .parser(cli::arg_location(config.enable_recording)),

        cli::option('a', "enable-headless-script-assistance").help("Enable script assistance for headlessclient")
        .parser(cli::arg_location(config.enable_headless_script_assistance)),


        cli::helper("========= RDP ========="),

        cli::option("enable-sound").help("Enable sound")
        .parser(cli::arg_location(config.enable_sound)),

        cli::option("rdp-performance-flags").help(
            "Set RDP performanceflags "
            "(https://learn.microsoft.com/en-us/windows/win32/termserv/imsrdpclientadvancedsettings-performanceflags)")
        .parser(cli::arg_location(config.rdp5_performance_flags)),

        cli::option("enable-nla").help("Enable NLA protocol")
        .parser(cli::arg_location(config.enable_nla)),

        cli::option("enable-tls").help("Enable TLS protocol")
        .parser(cli::arg_location(config.enable_tls)),

        cli::option("layout").help("Windows keylayout")
        .parser(cli::arg_location(config.key_layout)),

        cli::option("rdp-verbose")
        .parser(cli::arg_location(config.rdp_verbose)),


        cli::helper("========= Remote App ========="),

        cli::option("enable-remote-app").help("Connection as remote application")
        .parser(cli::arg_location(config.enable_remote_app)),

        cli::option("remote-cmd").help("Command line of remote application")
        .argname("command")
        .parser(cli::arg_location(config.remote_app_cmd)),

        cli::option("remote-working-dir").help("Remote working directory")
        .argname("directory")
        .parser(cli::arg_location(config.remote_app_working_directory)),


        cli::helper("========= Shared directory ========="),

        cli::option("enable-drive").help("Enable shared local disk")
        .parser(cli::arg_location(config.enable_drive)),

        cli::option("drive-dir").help("Directory path on local disk to share with your session")
        .parser(cli::arg_location(config.drive_path)),

        cli::option('d', "home-drive").help("Use $HOME as shared drive")
        .parser(cli::arg_location(config.use_home_drive))

        // TODO VNC
    );
}

} // namespace qtclient
