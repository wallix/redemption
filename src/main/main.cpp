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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Olivier Hervieu, Martin Potier, Raphael Zhou
              and Meng Tan

   main program
*/

#include "main/version.hpp"

#include "utils/apps/app_proxy.hpp"

#include "program_options/program_options.hpp"

#include "utils/apps/cryptofile.hpp"

namespace po = program_options;

int main(int argc, char** argv)
{
    std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
    Inifile ini;
    { ConfigurationLoader cfg_loader(ini.configuration_holder(), config_filename.c_str()); }

    UdevRandom rnd;
    CryptoContext cctx(rnd, ini);

    static constexpr char const * opt_print_spec = "print-config-spec";
    static constexpr char const * opt_print_ini = "print-default-ini";

    return app_proxy(
        argc, argv
      , "Redemption " VERSION ": A Remote Desktop Protocol proxy.\n"
        "Copyright (C) Wallix 2010-2016.\n"
        "Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
        "Martin Potier, Dominique Lafages, Jonathan Poelen, Raphael Zhou\n"
        "and Meng Tan."
      , cctx
      , extra_option_list{
          {opt_print_spec, "Configuration file spec for rdpproxy.ini"},
          {opt_print_ini, "rdpproxy.ini by default"}
      }
      , [argv](po::variables_map const & options, bool * quit) {
            if (options.count(opt_print_spec)) {
                *quit = true;
                std::cout <<
                  #include "configs/autogen/str_python_spec.hpp"
                ;
            }
            if (options.count(opt_print_ini)) {
                *quit = true;
                std::cout <<
                  #include "configs/autogen/str_ini.hpp"
                ;
            }
            return 0;
        }
    );
}
