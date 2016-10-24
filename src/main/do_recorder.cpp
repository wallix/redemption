/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include <string.h>
#include "main/version.hpp"
#include "capture/capture.hpp"

#include "utils/apps/app_verifier.hpp"
#include "utils/apps/app_recorder.hpp"
#include "utils/apps/app_decrypter.hpp"
#include "configs/config.hpp"
#include "program_options/program_options.hpp"

namespace po = program_options;

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len);

    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    int do_main(int role, int argc, char ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn);

    __attribute__((__visibility__("default")))
    int do_main(int role, int argc, char** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        Inifile ini;
        ini.set<cfg::debug::config>(false);
        std::string config_filename = CFG_PATH "/" RDPPROXY_INI;
        { ConfigurationLoader cfg_loader_full(ini.configuration_holder(), config_filename.c_str()); }
        UdevRandom rnd;
        CryptoContext cctx(rnd, ini);
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        int res = -1;

        switch (role){
        case 0: // RECorder
            {
                try {
                    res = app_recorder<Capture>(
                        argc, argv
                      , "ReDemPtion RECorder " VERSION ": An RDP movie converter.\n"
                        "Copyright (C) Wallix 2010-2015.\n"
                        "Christophe Grosjean, Jonathan Poelen and Raphael Zhou."
                      , [](po::options_description const &){}
                      , [](Inifile const & ini, po::variables_map const &, std::string const & output_filename) -> int {
                            if (   output_filename.length()
                                && !(
                                    bool(ini.get<cfg::video::capture_flags>()
                                        & (CaptureFlags::png | CaptureFlags::wrm)
                                    ) | ini.get<cfg::globals::capture_chunk>()
                                )
                            ) {
                                std::cerr << "Missing target format : need --png or --wrm\n" << std::endl;
                                return -1;
                            }
                            return 0;
                      }
                      // TODO: now that we have cctx the lambda is useless
                      , config_filename, ini, cctx, rnd
                      , [](Inifile const &) { return false; }/*has_extra_capture*/
                      , false // full_video
                    );
                } catch (const Error & e) {
                    printf("decrypt failed: with id=%d\n", e.id);
                }
            }
        break;
        case 1: // VERIFIER
            {
                try {
                    res = app_verifier(ini,
                        argc, argv
                      , "ReDemPtion VERifier " VERSION ".\n"
                        "Copyright (C) Wallix 2010-2016.\n"
                        "Christophe Grosjean, Raphael Zhou."
                      , cctx);
                    puts(res == 0 ? "verify ok" : "verify failed");
                } catch (const Error & e) {
                    printf("verify failed: with id=%d\n", e.id);
                }
            }
        break;
        default:
            try {
                res = app_decrypter(
                    argc, argv
                  , "ReDemPtion DECrypter " VERSION ".\n"
                    "Copyright (C) Wallix 2010-2015.\n"
                    "Christophe Grosjean, Raphael Zhou."
                  , cctx);
                puts(res == 0 ? "decrypt ok" : "decrypt failed");
            } catch (const Error & e) {
                printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        }
        return res;
    }
}
