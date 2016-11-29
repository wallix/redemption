#include "main/do_recorder.hpp"

#include <iostream>
#include <cstring>


inline void usage(char const * prog) {
    std::cerr << prog << ": Usage {redrec|redver|reddec} [hex-hmac_key hex-key] -- mode-args\n";
}

static char const * g_hmac_key_str = nullptr;
static char const * g_key_str = nullptr;
// Made these globals as g_hmac, etc, because otherwiser compiler
// complains of dangling references.
static char hmac_key_str[] =
"\xe3\x8d\xa1\x5e\x50\x1e\x4f\x6a\x01\xef\xde\x6c\xd9\xb3\x3a\x3f"
"\x2b\x41\x72\x13\x1e\x97\x5b\x4c\x39\x54\x23\x14\x43\xae\x22\xae";
static char key_str[] =
"\x56\x3e\xb6\xe8\x15\x8f\x0e\xed\x2e\x5f\xb6\xbc\x28\x93\xbc\x15"
"\x27\x0d\x7e\x78\x15\xfa\x80\x4a\x72\x3e\xf4\xfb\x31\x5f\xf4\xb2";

inline int get_hmac_key_prototype_fn(char * buffer) {
    memcpy(buffer, g_hmac_key_str, 32);
    return 0;
}

inline int get_trace_key_prototype_fn(char *, int, char * buffer, unsigned oldscheme) {
    (void)oldscheme;
    memcpy(buffer, g_key_str, 32);
    return 0;
}

// ./bin/gcc-5.4.0/release/redrec redver E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2 -i ./tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -m ./tests/fixtures/verifier/recorded/ -s ./tests/fixtures/verifier/hash/ --verbose 10

int main(int argc, const char** argv) {
    // TODO: see other todo below, command line arguments should also be simplified
    g_hmac_key_str = hmac_key_str;
    g_key_str = key_str;

    // first arg=redrec,redver,reddec

    auto ka = argv + 1;
    for (auto a : {hmac_key_str, key_str}) {
        auto k = *ka;
        for (unsigned i = 0; i < 32; ++i) {
            auto char_to_hex = [](char c){
                auto in = [&c](char left, char right) { return left <= c && c <= right; };
                return
                    in('0', '9') ? c-'0'
                    : in('a', 'f') ? c-'a'
                    : in('A', 'F') ? c-'A'
                    : -1;
            };
            unsigned const c1 = char_to_hex(k[i*2]);
            unsigned const c2 = char_to_hex(k[i*2+1]);
            a[i] = static_cast<char>(c1 << 4 | c2);
        }
        ++ka;
    }

    return do_main(
        1,
        argc - 3, 
        argv + 3,
        get_hmac_key_prototype_fn,
        get_trace_key_prototype_fn
    );
}
