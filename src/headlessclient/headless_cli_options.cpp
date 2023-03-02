/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_cli_options.hpp"
#include "keyboard/keylayouts.hpp"
#include "utils/cli.hpp"
#include "utils/cli_chrono.hpp"
#include "utils/cli_screen_info.hpp"
#include "utils/redemption_info_version.hpp"

#include <iostream>


HeadlessCliOptions::Result HeadlessCliOptions::parse(int argc, const char ** argv)
{
    auto positive_duration_location = [](std::chrono::milliseconds& ms){
        return cli::arg_location(ms, [](std::chrono::milliseconds& ms){
            return ms < std::chrono::milliseconds::zero() ? cli::Res::BadFormat : cli::Res::Ok;
        });
    };

    auto options = cli::options(
        cli::helper("Headless Client."),

        cli::option('h', "help").help("Show help and exit")
            .parser(cli::help()),

        cli::option('v', "version").help("Show version and exit")
            .parser(cli::quit([]{ puts(redemption_info_version()); })),

        cli::option('l', "list-keylayout").help("List keylayout and exit")
            .parser(cli::quit([]{
                for (auto&& layout : keylayouts_sorted_by_name()) {
                    printf("%8u: %s\n", safe_cast<unsigned>(layout.kbdid), layout.name.c_str());
                }
            })),

        cli::option("config-file")
            .parser(cli::arg_location(config_filename)),

        cli::option("config-print-new-section")
            .parser(cli::on_off_location(print_client_info_section)),

        cli::option('i', "interactive").help("Enter interactive mode after connection to target.")
            .parser(cli::on_off_location(interactive)),


        cli::helper("========= Connection ========="),

        cli::option('u', "username").help("Username of target")
            .parser(cli::arg_location(username)),

        cli::option('p', "password").help("Password of target")
            .parser(cli::arg_location(password)),

        cli::option('t', "target").help("IP address of target").argname("<address>")
            .parser(cli::arg_location(ip_address)),
        cli::option("ip").help("alias of --target").argname("<address>")
            .parser(cli::arg_location(ip_address)),

        cli::option('P', "port").help("Port of target")
            .parser(cli::arg_location(port)),

        cli::option("fixed-random")
            .parser(cli::on_off_location(use_fixed_random)),

        cli::option("license-store").argname("<directory>")
            .parser(cli::arg_location(license_store_path)),

        cli::option("persist").help("Keep connection open even when stdin is closed")
            .parser(cli::on_off_location(persist)),


        cli::helper("========= Client ========="),

        cli::option('W', "width").help("Screen width")
            .parser(cli::arg_location(screen_info.width)),

        cli::option('H', "height").help("Screen height")
            .parser(cli::arg_location(screen_info.height)),

        cli::option("bpp").help("Bit per pixel")
            .argname("bitPerPixel")
            .parser(cli::arg_location(screen_info.bpp)),

        cli::option('s', "size").help("Screen size")
            .parser(cli::arg_location(screen_info)),

        cli::option('k', "keylayout").help("Keyboard layout id (see --list-keylayout)")
            .argname("kbdId")
            .parser(cli::arg([&](KeyLayout::KbdId kbdid) {
                if (find_layout_by_id(kbdid)) {
                    keylayout = kbdid;
                    return cli::Res::Ok;
                }
                return cli::Res::BadOption;
            })),


        cli::helper("========= Socket ========="),

        cli::option("connection-establishment-timeout")
            .parser(positive_duration_location(connection_establishment_timeout)),

        cli::option("user-timeout")
            .parser(positive_duration_location(tcp_user_timeout)),

        cli::option("receive-timeout")
            .parser(positive_duration_location(receive_timeout)),

        cli::option("socket-verbose")
            .parser(cli::arg_location(socket_verbose)),


        cli::helper("========= Command ========="),

        cli::option("kbd-map").help("Kbd mapping for scancode command (fr or en)")
            .parser(cli::arg([&](char const* value) {
                if (value[0] == 'f' && value[1] == 'r' && value[2] == '\0') {
                    is_cmd_kbdmap_en = false;
                }
                else if (value[0] == 'e' && value[1] == 'n' && value[2] == '\0') {
                    is_cmd_kbdmap_en = true;
                }
                else {
                    return cli::Res::BadOption;
                }
                return cli::Res::Ok;
            })),

        cli::option("png-path").argname("<filepath>")
            .parser(cli::arg_location(output_png_path)),

        cli::option("wrm-path").argname("<filepath>")
            .parser(cli::arg_location(output_wrm_path)),

        cli::option("record-transport-path").argname("<filepath>")
            .parser(cli::arg_location(output_record_transport_path)),

        cli::option("enable-png-capture")
            .parser(cli::on_off_location(enable_png_capture)),

        cli::option("enable-wrm-capture")
            .parser(cli::on_off_location(enable_wrm_capture)),

        cli::option("enable-record-transport")
            .parser(cli::on_off_location(enable_record_transport_capture)),


        cli::helper("")
    );

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
    case cli::Res::Ok:
        return Result::Ok;
    case cli::Res::Exit:
        return Result::Exit;
    case cli::Res::Help:
        // std::cout.sync_with_stdio(false);
        cli::print_help(options, std::cout);
        return Result::Exit;
    case cli::Res::BadFormat:
    case cli::Res::BadOption:
    case cli::Res::NotOption:
    case cli::Res::StopParsing:
        printf("Bad %s at parameter %d",
            (cli_result.res == cli::Res::BadFormat) ? "format" : "option",
            cli_result.opti);
        if (cli_result.opti < cli_result.argc) {
            printf(" (%s)\n", cli_result.argv[cli_result.opti]);
        }
        else {
            printf("\n");
        }
        return Result::Error;
    }

    return Result::Error;
}
