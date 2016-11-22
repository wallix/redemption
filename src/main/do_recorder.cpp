/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "do_recorder.hpp"
#include "main/version.hpp"

#include "utils/apps/app_verifier.hpp"
#include "utils/apps/app_recorder.hpp"
#include "utils/apps/app_decrypter.hpp"
#include "configs/config.hpp"

#include <cstdio>

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    int do_main(int role, int argc, char** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        auto config_filename = CFG_PATH "/" RDPPROXY_INI;

        UdevRandom rnd;
        CryptoContext cctx;
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        int res = -1;

        switch (role){
        case 0: // RECorder
            try {
                res = app_recorder(
                    argc, argv
                  , "ReDemPtion RECorder " VERSION ": An RDP movie converter.\n"
                    "Copyright (C) Wallix 2010-2016.\n"
                    "Christophe Grosjean, Jonathan Poelen and Raphael Zhou.\n"
                    "Compatible with any WRM file format up to 4."
                  , config_filename
                  , ini
                  , cctx
                  , rnd);
            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        case 1: // VERifier
            ini.set<cfg::debug::config>(false);
            try {
                res = app_verifier(ini,
                    argc, argv
                  , "ReDemPtion VERifier " VERSION ".\n"
                    "Copyright (C) Wallix 2010-2016.\n"
                    "Christophe Grosjean, Jonathan Poelen, Raphael Zhou."
                  , cctx);
                std::puts(res == 0 ? "verify ok\n" : "verify failed\n");
            } catch (const Error & e) {
                std::printf("verify failed: with id=%d\n", e.id);
            }
        break;
        default: // DECrypter
            try {
                res = app_decrypter(
                    argc, argv
                  , "ReDemPtion DECrypter " VERSION ".\n"
                    "Copyright (C) Wallix 2010-2016.\n"
                    "Christophe Grosjean, Jonathan Poelen, Raphael Zhou."
                  , cctx);
                std::puts(res == 0 ? "decrypt ok\n" : "decrypt failed\n");
            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        }
        return res;
    }
}
