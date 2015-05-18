/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include "apps/app_decrypter.hpp"

#include "config.hpp"
#include "version.hpp"


int main(int argc, char ** argv) {
    return app_decrypter(
        argc, argv
      , "ReDemPtion DECrypter " VERSION ".\n"
        "Copyright (C) Wallix 2010-2015.\n"
        "Christophe Grosjean, Raphael Zhou."
      , [](CryptoContext & cctx) {
            Inifile::Inifile_crypto ini_crypto;
            memcpy(cctx.crypto_key, ini_crypto.key0, sizeof(cctx.crypto_key));
            memcpy(cctx.hmac_key,   ini_crypto.key1, sizeof(cctx.hmac_key  ));
            return 0;
        }
    );
}
