/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include "utils/apps/app_decrypter.hpp"

#include "configs/config.hpp"
#include "main/version.hpp"

extern "C" {
    __attribute__((__visibility__("default")))
    inline int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    inline int do_main(int argc, char ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        Inifile ini;
        { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), CFG_PATH "/" RDPPROXY_INI); }

        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        int res = -1;
        try {
            res = app_decrypter(
                argc, argv
              , "ReDemPtion DECrypter " VERSION ".\n"
                "Copyright (C) Wallix 2010-2015.\n"
                "Christophe Grosjean, Raphael Zhou."
              , cctx);
            if (res == 0){
                printf("decrypt ok");
            }
            else {
                printf("decrypt failed\n");
            }
        } catch (const Error & e) {
            printf("decrypt failed: with id=%d\n", e.id);
        }
        return res;
    }
}

