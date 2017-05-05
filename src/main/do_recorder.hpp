/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#pragma once

#include "capture/cryptofile.hpp"
#include "cxx/cxx.hpp"

extern "C" {
    REDEMPTION_LIB_EXPORT
    int recmemcpy(char * dest, char * source, int len);

    REDEMPTION_LIB_EXPORT
    int do_main(int argc, char const ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn);

    void clear_files_flv_meta_png(const char * path, const char * prefix);
}
