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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for bitmap class, compression performance

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE TestBitmapPerf
#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "bitmap.hpp"
#include "colors.hpp"
#include "config.hpp"
#include <sys/time.h>
#include "rdtsc.hpp"

long long ustime() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (long long)now.tv_sec*1000000LL + (long long)now.tv_usec;
}

BOOST_AUTO_TEST_CASE(TestBitmapCompressPerformance)
{
    {
        int bpp = 16;
        Bitmap bigbmp(FIXTURES_PATH "/color_image.bmp");
        // make it large enough to hold any image
        Stream out(2*bigbmp.bmp_size(bpp));
        unsigned long long usec = ustime();
        unsigned long long cycles = rdtsc();
        bigbmp.compress(bpp, out);
        unsigned long long elapusec = ustime() - usec;
        unsigned long long elapcyc = rdtsc() - cycles;
        printf("initial_size = %llu, compressed size: %llu\n",
            (long long)bigbmp.bmp_size(bpp),
            (long long)(out.p - out.data));
        printf("elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);

        Bitmap bmp2(bpp, (RGBPalette *)NULL, bigbmp.cx, bigbmp.cy, out.data, out.p - out.data, true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(bpp), bigbmp.bmp_size(bpp));
        BOOST_CHECK(0 == memcmp(bmp2.data_co(bpp), bigbmp.data_co(bpp), bigbmp.bmp_size(bpp)));
    }

    {
        int bpp = 24;
        Bitmap bigbmp(FIXTURES_PATH "/logo-redemption.bmp");
        // make it large enough to hold any image
        Stream out(2*bigbmp.bmp_size(bpp));
        unsigned long long usec = ustime();
        unsigned long long cycles = rdtsc();
        bigbmp.compress(bpp, out);
        unsigned long long elapusec = ustime() - usec;
        unsigned long long elapcyc = rdtsc() - cycles;
        printf("initial_size = %llu, compressed size: %llu\n",
            (long long)bigbmp.bmp_size(bpp),
            (long long)(out.p - out.data));
        printf("elapsed time = %llu %llu %f\n", elapusec, elapcyc, (double)elapcyc / (double)elapusec);

        Bitmap bmp2(bpp, (RGBPalette *)NULL, bigbmp.cx, bigbmp.cy, out.data, out.p - out.data, true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(bpp), bigbmp.bmp_size(bpp));
        BOOST_CHECK(0 == memcmp(bmp2.data_co(bpp), bigbmp.data_co(bpp), bigbmp.bmp_size(bpp)));
    }
}
