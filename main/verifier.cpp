/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "apps/app_verifier.hpp"

#include "config.hpp"
#include "version.hpp"

int main(int argc, char ** argv) {
    return app_verifier(
        argc, argv
      , "ReDemPtion VERifier " VERSION ".\n"
        "Copyright (C) Wallix 2010-2014.\n"
        "Christophe Grosjean, Raphael Zhou."
      , [](CryptoContext & cctx) {
            Inifile::Inifile_crypto ini_crypto;
            memcpy(cctx.crypto_key, ini_crypto.key0, sizeof(cctx.crypto_key));
            memcpy(cctx.hmac_key,   ini_crypto.key1, sizeof(cctx.hmac_key  ));
            return 0;
        }
    );
}
