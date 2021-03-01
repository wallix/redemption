/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#include "configs/config.hpp"
#include "configs/autogen/cfg_ini_pack.hpp"
#include "utils/cfgloader.hpp"
#include "utils/cli.hpp"
#include "cxx/diagnostic.hpp"

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wunused-function")
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-member-function")
#include "configs/io.hpp"
#include "configs/autogen/enums_func_ini.tcc"
REDEMPTION_DIAGNOSTIC_POP()

#include <string_view>
#include <iostream>

#include <cstdio>

using namespace std::string_view_literals;

namespace
{
    template<class>
    struct PrinterValues;

    zstring_view assign_zbuf_from_cfg(
        writable_chars_view zbuf,
        cfg_s_type<::configs::spec_types::file_permission> /*type*/,
        uint32_t x
    ) {
        char const * digits = "01234567";
        zbuf[0] = '0';
        zbuf[1] = digits[(x & 0700) >> 6];
        zbuf[2] = digits[(x & 0070) >> 3];
        zbuf[3] = digits[(x & 0007)];
        zbuf[4] = '\0';
        return zstring_view(zstring_view::is_zero_terminated{}, zbuf.data(), 4);
    }

    template<class... Cfg>
    struct PrinterValues<configs::Pack<Cfg...>>
    {
        static void print(Inifile& ini)
        {
            int i = 0;

            auto show = [&](configs::cfg_ini_infos::SectionAndName names, zstring_view str){
                std::printf("[%s] %s = %s\n",
                    names.section.c_str(), names.name.c_str(), str.c_str());
                ++i;
            };

            Inifile::ZStringBuffer zbuffer;
            auto buffer = make_writable_array_view(zbuffer);
            (..., show(
                configs::cfg_ini_infos::ini_names[i],
                assign_zbuf_from_cfg(
                    buffer,
                    cfg_s_type<typename Cfg::sesman_and_spec_type>(),
                    ini.get<Cfg>()
                )
            ));
        }
    };
}

int main(int ac, char ** av)
{
    bool show_values = false;

    auto const options = cli::options(
        cli::option('h', "help").help("produce help message")
            .parser(cli::help()),

        cli::option('p', "print").help("display values from ini file")
            .parser(cli::on_off_location(show_values))
    );

    auto const default_ini = app_path(AppPath::CfgIni);

    auto usage = [&](std::ostream& out){
        out << "Usage: " << av[0] << " [options] [filename="
            << default_ini << "]\n\n";
        cli::print_help(options, out);
        return 1;
    };

    char const* filename = nullptr;
    int opti = 1;

    while (opti < ac) {
        auto cli_result = cli::parse(options, opti, ac, av);
        switch (cli_result.res) {
            case cli::Res::Ok:
                break;
            case cli::Res::Exit:
                return 0;
            case cli::Res::Help:
                usage(std::cout);
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
            case cli::Res::StopParsing:
                ++cli_result.opti;
                if (cli_result.opti == cli_result.argc) {
                    break;
                }
                if (cli_result.opti + 1 != cli_result.argc) {
                    return usage(std::cerr);
                }
                [[fallthrough]];
            case cli::Res::NotOption:
                if (filename) {
                    return usage(std::cerr);
                }
                filename = av[cli_result.opti];
                ++cli_result.opti;
                break;
        }
        opti = cli_result.opti;
    }

    filename = filename ? filename : default_ini;
    std::printf("filename: %s\n", filename);

    Inifile ini;
    bool const load_result = configuration_load(ini.configuration_holder(), filename);

    if (show_values) {
        PrinterValues<configs::cfg_ini_infos::IniPack>::print(ini);
    }

    return load_result ? 0 : 2;
}
