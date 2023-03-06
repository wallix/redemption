/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/profile/cli_parse_profile.hpp"
#include "qtclient/profile/profile_as_cli_options.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/split.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/finally.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/log.hpp"

#include <iostream>


static auto make_options(qtclient::Profile& config)
{
    return qtclient::profile_as_cli_options(config, [](auto&&... options){
        return cli::options(
            cli::helper("Client ReDemPtion."),

            cli::option('h', "help").help("Show help")
            .parser(cli::help()),

            cli::option('v', "version").help("Show version")
            .parser(cli::quit([]{ std::cout << redemption_info_version() << "\n"; })),

            // TODO add profile=name

            static_cast<decltype(options)&&>(options)...
        );
    });
}

bool qtclient::cli_parse_profile(char const* const argv[], int argc, Profile& profile)
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
