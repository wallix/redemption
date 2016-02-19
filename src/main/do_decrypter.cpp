/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include "utils/apps/app_decrypter.hpp"

#include "config.hpp"
#include "version.hpp"

extern "C" {
    __attribute__((__visibility__("default"))) 
    int do_main(int argc, char ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn) 
    {
        std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
        Inifile ini;
        { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), config_filename.c_str()); }

        UdevRandom rnd;
        CryptoContext cctx(rnd, ini, 1);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        return app_decrypter(
            argc, argv
          , "ReDemPtion DECrypter " VERSION ".\n"
            "Copyright (C) Wallix 2010-2015.\n"
            "Christophe Grosjean, Raphael Zhou."
          , cctx);
    }
}

