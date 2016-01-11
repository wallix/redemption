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

    std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
    Inifile ini;
    { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), config_filename.c_str()); }

    UdevRandom rnd;
    CryptoContext cctx(rnd, ini, 1);

    TODO("We don't know yet if we need the keys, we should replace that init with some internal code inside CryptoContext")
    cctx.get_crypto_key();
    cctx.set_hmac_key(ini.get<cfg::crypto::key1>());

    return app_verifier(
        argc, argv
      , "ReDemPtion VERifier " VERSION ".\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Raphael Zhou."
      , cctx);
}
