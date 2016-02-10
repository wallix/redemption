/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "../modules/program_options/src/program_options.cpp"

#include "utils/apps/app_verifier.hpp"
#include "config.hpp"
#include "version.hpp"

typedef char * get_key_prototype();


extern "C" {
    __attribute__((__visibility__("default"))) 
    int do_main(int argc, char ** argv, get_key_prototype * fn) {

        printf("Running verifier with args %d %s %s\n", argc, argv[0], argv[1]);

        std::string config_filename = CFG_PATH "/" RDPPROXY_INI;

        printf("Testing callback function\n");
        printf("----> %s\n", fn());
 
        printf("Running verifier step 1\n");

        Inifile ini;

        printf("Running verifier step 2\n");

        { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), config_filename.c_str()); }

        printf("Running verifier step 3\n");

        UdevRandom rnd;
        
        printf("Running verifier step 4\n");

        CryptoContext cctx(rnd, ini, 1);

        printf("Running verifier step 5\n");

        return app_verifier(
            argc, argv
          , "ReDemPtion VERifier " VERSION ".\n"
            "Copyright (C) Wallix 2010-2015.\n"
            "Christophe Grosjean, Raphael Zhou."
          , cctx);
    }
}
