#include "main/do_recorder.hpp"

#include <syslog.h>

// ./bin/gcc-5.4.0/release/redrec redver -i ./tests/includes/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -m ./tests/includes/fixtures/verifier/recorded/ -s ./tests/fixtures/verifier/hash/ --verbose 10

int main(int argc, const char** argv) {
    openlog("redrec", LOG_PERROR, LOG_USER);
    return do_main(argc, argv, nullptr, nullptr);
}
