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

            // TODO add --profile=name

            // TODO add --config=filename

            static_cast<decltype(options)&&>(options)...
        );
    });
}

qtclient::CliResult qtclient::cli_parse_profile(int argc, char const* const argv[], Profile& profile)
{
    auto options = make_options(profile);
    switch (cli::check_result(options, cli::parse(options, argc, argv), std::cout, std::cerr))
    {
        case cli::CheckResult::Ok: return CliResult::Ok;
        case cli::CheckResult::Exit: return CliResult::Exit;
        case cli::CheckResult::Error:;
    }

    return CliResult::Error;
}
