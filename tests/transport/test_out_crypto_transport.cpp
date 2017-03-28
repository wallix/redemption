/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOutCryptoTransport
#include "system/redemption_unit_tests.hpp"

#define LOGPRINT
#include "utils/log.hpp"
#include "transport/out_crypto_transport.hpp"

BOOST_AUTO_TEST_CASE(TestOutCryptoTransport)
{
    LCGRandom rnd(0);
    Fstat fstat;
    CryptoContext cctx;
    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};

    char tmpname[128] = "/tmp/test_transportXXXXXX";
    int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
    const char * finalname = "./encrypted.txt";
    {
        OutCryptoTransport ct(true, true, cctx, rnd, fstat);
        ct.open(fd, tmpname, finalname);
        ct.send("We write, ", 10);
        ct.send("and again, ", 11);
        ct.send("and so on.", 10);
        ct.close(qhash, fhash);
    }
    ::unlink(tmpname);
    ::unlink(finalname);
}

//BOOST_AUTO_TEST_CASE(TestOutCryptoTransportAutoClose)
//{
//    LCGRandom rnd(0);
//    Fstat fstat;
//    CryptoContext cctx;
//    char tmpname[128] = "/tmp/test_transportXXXXXX";
//    int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
//    const char * finalname = "./encrypted.txt";
//    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
//    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
//    {
//        OutCryptoTransport ct(true, true, cctx, rnd, fstat);
//        ct.open(fd, tmpname, finalname);
//        ct.send("We write, ", 10);
//        ct.send("and again, ", 11);
//        ct.send("and so on.", 10);
//    }
//    // if there is no explicit close we can't get hash values
//    // but the file is correctly closed and ressources freed
//    ::unlink(tmpname);
//    ::unlink(finalname);
//}

//BOOST_AUTO_TEST_CASE(TestOutCryptoTransportMultipleFiles)
//{
//    LCGRandom rnd(0);
//    Fstat fstat;
//    CryptoContext cctx;
//    char tmpname1[128] = "/tmp/test_transportXXXXXX";
//    char tmpname2[128] = "/tmp/test_transportXXXXXX";
//    const char * finalname1 = "./encrypted001.txt";
//    const char * finalname2 = "./encrypted002.txt";
//    uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
//    uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
//    {
//        OutCryptoTransport ct(true, true, cctx, rnd, fstat);
//        int fd1 = ::mkostemp(tmpname1, O_WRONLY|O_CREAT);
//        ct.open(fd1, tmpname1, finalname1);
//        ct.send("We write, ", 10);
//        ct.send("and again, ", 11);
//        ct.send("and so on.", 10);
//        cl.close(qhash, fhash);

//        int fd2 = ::mkostemp(tmpname1, O_WRONLY|O_CREAT);
//        ct.open(fd2, tmpname2, finalname2);        
//        ct.send("We write, ", 10);
//        ct.send("and again, ", 11);
//        ct.send("and so on.", 10);
//        cl.close(qhash, fhash);
//    }
//    ::unlink(tmpname1);
//    ::unlink(finalname1);
//    ::unlink(tmpname2);
//    ::unlink(finalname2);
//}

