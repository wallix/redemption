/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBitmapPerf
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL

#include "log.hpp"
#include "bitmap.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "difftimeval.hpp"
#include "rdtsc.hpp"

#ifndef FIXTURES_PATH
#define FIXTURES_PATH
#endif

BOOST_AUTO_TEST_CASE(TestBitmapCompressPerformance)
{
    {
        BOOST_CHECK(true);
        Bitmap bigbmp(FIXTURES_PATH "/color_image.bmp");

        BOOST_CHECK(true);
        // make it large enough to hold any image
        BStream out(2*bigbmp.bmp_size());
        BOOST_CHECK(true);
        uint64_t usec = ustime();
        uint64_t cycles = rdtsc();
        bigbmp.compress(24, out);
        uint64_t elapusec = ustime() - usec;
        uint64_t elapcyc = rdtsc() - cycles;
        printf("initial_size = %lu, compressed size: %lu\n",
            (long)bigbmp.bmp_size(),
            (long)(out.p - out.get_data()));
        printf("elapsed time = %lu %lu %f\n", 
            static_cast<long unsigned>(elapusec), 
            static_cast<long unsigned>(elapcyc), 
            static_cast<double>(elapcyc) / static_cast<double>(elapusec));

        Bitmap bmp2(24, 24, (BGRPalette *)NULL, bigbmp.cx(), bigbmp.cy(), out.get_data(), out.p - out.get_data(), true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        BOOST_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }

    {
        int bpp = 24;
        Bitmap bigbmp(FIXTURES_PATH "/logo-redemption.bmp");
        // make it large enough to hold any image
        BStream out(2*bigbmp.bmp_size());
        uint64_t usec = ustime();
        uint64_t cycles = rdtsc();
        bigbmp.compress(bpp, out);
        uint64_t elapusec = ustime() - usec;
        uint64_t elapcyc = rdtsc() - cycles;
        printf("initial_size = %lu, compressed size: %lu\n",
            (long)bigbmp.bmp_size(),
            (long)(out.p - out.get_data()));

        printf("elapsed time = %lu %lu %f\n", 
            static_cast<long unsigned>(elapusec), 
            static_cast<long unsigned>(elapcyc), 
            static_cast<double>(elapcyc) / static_cast<double>(elapusec));

        Bitmap bmp2(bpp, bpp, (BGRPalette *)NULL, bigbmp.cx(), bigbmp.cy(), out.get_data(), out.p - out.get_data(), true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        BOOST_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }
}

BOOST_AUTO_TEST_CASE(TestBitmapCompressPerformancePNG)
{
    {
        BOOST_CHECK(true);
        Bitmap bigbmp(FIXTURES_PATH "/color_image.png");

        BOOST_CHECK(true);
        // make it large enough to hold any image
        BStream out(2*bigbmp.bmp_size());
        BOOST_CHECK(true);
        uint64_t usec = ustime();
        uint64_t cycles = rdtsc();
        bigbmp.compress(24, out);
        uint64_t elapusec = ustime() - usec;
        uint64_t elapcyc = rdtsc() - cycles;
        printf("initial_size = %lu, compressed size: %lu\n",
            (long)bigbmp.bmp_size(),
            (long)(out.p - out.get_data()));
        printf("elapsed time = %lu %lu %f\n", 
            static_cast<long unsigned>(elapusec), 
            static_cast<long unsigned>(elapcyc), 
            static_cast<double>(elapcyc) / static_cast<double>(elapusec));

        Bitmap bmp2(24, 24, (BGRPalette *)NULL, bigbmp.cx(), bigbmp.cy(), out.get_data(), out.p - out.get_data(), true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        BOOST_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }

    {
        int bpp = 24;
        Bitmap bigbmp(FIXTURES_PATH "/logo-redemption.png");
        // make it large enough to hold any image
        BStream out(2*bigbmp.bmp_size());
        uint64_t usec = ustime();
        uint64_t cycles = rdtsc();
        bigbmp.compress(bpp, out);
        uint64_t elapusec = ustime() - usec;
        uint64_t elapcyc = rdtsc() - cycles;
        printf("initial_size = %lu, compressed size: %lu\n",
            (long)bigbmp.bmp_size(),
            (long)(out.p - out.get_data()));
        printf("elapsed time = %lu %lu %f\n", 
            static_cast<long unsigned>(elapusec), 
            static_cast<long unsigned>(elapcyc), 
            static_cast<double>(elapcyc) / static_cast<double>(elapusec));

        Bitmap bmp2(bpp, bpp, (BGRPalette *)NULL, bigbmp.cx(), bigbmp.cy(), out.get_data(), out.p - out.get_data(), true);
        BOOST_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        BOOST_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }
}
