#include "main/do_recorder.hpp"

#include <syslog.h>

// ./bin/gcc-5.4.0/release/redrec redver E38DA15E501E4F6A01EFDE6CD9B33A3F2B4172131E975B4C3954231443AE22AE 563EB6E8158F0EED2E5FB6BC2893BC15270D7E7815FA804A723EF4FB315FF4B2 -i ./tests/fixtures/verifier/recorded/toto@10.10.43.13,Administrateur@QA@cible,20160218-183009,wab-5-0-0.yourdomain,7335.mwrm -m ./tests/fixtures/verifier/recorded/ -s ./tests/fixtures/verifier/hash/ --verbose 10

int main(int argc, const char** argv) {
    openlog("redrec", LOG_PERROR, LOG_USER);
    return do_main(argc, argv, nullptr, nullptr);
}
