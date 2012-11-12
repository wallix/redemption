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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include <sys/time.h>
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"

void readline(int fd, char ** begin, char **end)
{
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *p = 0;
            *begin = p+1;
            return;
        }
    }
    ssize_t rcvd = ::read(fd, *end, 2048);
    *end += rcvd;
    printf("received %u chars\n", rcvd);
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *p = 0;
            *begin = p+1;
            return;
        }
    }
    return;
}


BOOST_AUTO_TEST_CASE(TestChainedMWRM)
{
    const char * input_filename = "./tests/fixtures/sample.mwrm";
    char buffer[2048];
    char * begin = buffer;
    char * end = buffer;
    int fd = ::open(input_filename, O_RDONLY);
    if (fd == -1){
        LOG(LOG_ERR, "Open failed with error %s", strerror(errno));
        throw Error(ERR_TRANSPORT);
    }
    char * oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line1=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line2=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line3=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line4=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line5=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line6=%s\n", oldbeg);
    oldbeg = begin;
    readline(fd, &begin, &end);
    printf("line7=%s\n", oldbeg);
    ::close(fd);
//    InByMetaTransport in_wrm_trans(input_filename);
}
