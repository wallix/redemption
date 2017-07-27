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

#define LOGPRINT

#include "main/version.hpp"

#include "utils/apps/app_proxy.hpp"

#include "capture/cryptofile.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "configs/config.hpp"

#include "main/version.hpp"

int main(int argc, char** argv)
{
    CryptoContext cctx;
    UdevRandom rnd;
    Fstat fstat;

    LOG(LOG_INFO, "HASH_PATH: \"%s\"", HASH_PATH);
    LOG(LOG_INFO, "RECORD_PATH: \"%s\"", RECORD_PATH);
    LOG(LOG_INFO, "RECORD_TMP_PATH: \"%s\"", RECORD_TMP_PATH);
    LOG(LOG_INFO, "FLV_PATH: \"%s\"", FLV_PATH);
    LOG(LOG_INFO, "OCR_PATH: \"%s\"", OCR_PATH);
    LOG(LOG_INFO, "PNG_PATH: \"%s\"", PNG_PATH);
    LOG(LOG_INFO, "WRM_PATH: \"%s\"", WRM_PATH);
    LOG(LOG_INFO, "LICENSE_PATH: \"%s\"", LICENSE_PATH);
    LOG(LOG_INFO, "CERTIF_PATH: \"%s\"", CERTIF_PATH);
    LOG(LOG_INFO, "PERSISTENT_PATH: \"%s\"", PERSISTENT_PATH);
    LOG(LOG_INFO, "DRIVE_REDIRECTION_PATH: \"%s\"", DRIVE_REDIRECTION_PATH);
    LOG(LOG_INFO, "PID_PATH: \"%s\"", PID_PATH);
    LOG(LOG_INFO, "SHARE_PATH: \"%s\"", SHARE_PATH);
    LOG(LOG_INFO, "CFG_PATH: \"%s\"", CFG_PATH);
//     LOG(LOG_INFO, "CFG_THEMES_PATH: \"%s\"", CFG_THEMES_PATH);


    return app_proxy(
        argc, argv
      , "Redemption " VERSION ": A Remote Desktop Protocol proxy.\n"
        "Copyright (C) Wallix 2010-2016.\n"
        "Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
        "Martin Potier, Dominique Lafages, Jonathan Poelen, Raphael Zhou,\n"
        "Meng Tan, Clement Moroldo and Jennifer Inthavongh."
      , cctx
      , rnd
      , fstat
    );
}
