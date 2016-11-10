/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#pragma once

#include "utils/apps/cryptofile.hpp"

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len);

    __attribute__((__visibility__("default")))
    int do_main(int role, int argc, char ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn);
}
