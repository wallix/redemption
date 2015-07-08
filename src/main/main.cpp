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

#include "version.hpp"

#include "parameters_holder.hpp"

#include "apps/app_proxy.hpp"

#include "program_options.hpp"

namespace po = program_options;

int main(int argc, char** argv)
{
    return app_proxy<parameters_holder>(
        argc, argv
      , "Redemption " VERSION ": A Remote Desktop Protocol proxy.\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
        "Martin Potier, Dominique Lafages, Jonathan Poelen, Raphael Zhou\n"
        "and Meng Tan."
      , extra_option_list{}, [](po::variables_map const &, bool *) { return 0; }
    );
}
