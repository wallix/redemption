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

#include "capture/cryptofile.hpp"
#include "utils/genrandom.hpp"
#include "configs/config.hpp"

#include "main/version.hpp"

int main(int argc, char** argv)
{
    Inifile ini;

    UdevRandom rnd;
    CryptoContext cctx;

    return app_proxy(
        argc, argv
      , "Redemption " VERSION ": A Remote Desktop Protocol proxy.\n"
        "Copyright (C) Wallix 2010-2016.\n"
        "Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
        "Martin Potier, Dominique Lafages, Jonathan Poelen, Raphael Zhou\n"
        "and Meng Tan."
      , cctx
      , rnd
    );
}
