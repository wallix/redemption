/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "utils/apps/app_verifier.hpp"

#include "config.hpp"
#include "version.hpp"

int main(int argc, char ** argv) {

    Inifile ini;
    ini.set<cfg::debug::config>(false);
    { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), CFG_PATH "/" RDPPROXY_INI); }

    UdevRandom rnd;
    CryptoContext cctx(rnd, ini, 1);

    try {
        return app_verifier(ini,
            argc, argv
          , "ReDemPtion VERifier " VERSION ".\n"
            "Copyright (C) Wallix 2010-2015.\n"
            "Christophe Grosjean, Raphael Zhou."
          , cctx);
    } catch (const Error & e) {
        return -1;
    }
}
