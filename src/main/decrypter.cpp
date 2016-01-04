/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include "utils/apps/app_decrypter.hpp"

#include "config.hpp"
#include "version.hpp"


int main(int argc, char ** argv) 
{
    std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
    Inifile ini;
    { ConfigurationLoader cfg_loader_full(ini, config_filename.c_str()); }

    UdevRandom rnd;
    CryptoContext cctx(rnd, ini);

    TODO("We don't know yet if we need the keys, we should replace that init with some internal code inside CryptoContext")
    cctx.set_crypto_key(ini.get<cfg::crypto::key0>());
    cctx.set_hmac_key(ini.get<cfg::crypto::key1>());

    return app_decrypter(
        argc, argv
      , "ReDemPtion DECrypter " VERSION ".\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Raphael Zhou."
      , cctx);
}
