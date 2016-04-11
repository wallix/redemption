/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include <string.h>
#include "utils/apps/app_verifier.hpp"
#include "configs/config.hpp"
#include "main/version.hpp"

extern "C" {
    __attribute__((__visibility__("default")))
    int vermemcpy(char * dest, char * source, int len)
    {
//        hexdump(dest, 32);
//        hexdump(source, 32);
        ::memcpy(dest, source, static_cast<size_t>(len));
//        hexdump(dest, 32);
        return 0;
    }


    __attribute__((__visibility__("default")))
    int do_main(int argc, char ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), CFG_PATH "/" RDPPROXY_INI); }
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        int res = -1;
        try {
            res = app_verifier(ini,
                argc, argv
              , "ReDemPtion VERifier " VERSION ".\n"
                "Copyright (C) Wallix 2010-2016.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("verify ok");
            }
            else {
                printf("verify failed\n");
            }
        } catch (const Error & e) {
            printf("verify failed: with id=%d\n", e.id);
        }
        return res;
    }
}
