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

#define RED_TEST_MODULE TestBitmapPerf
#include "system/redemption_unit_tests.hpp"

#include "utils/bitmap.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/difftimeval.hpp"
#include "utils/rdtsc.hpp"
#include "utils/stream.hpp"

#include <cinttypes>


inline Bitmap compress_perf(Bitmap & bigbmp)
{
    auto sz = 2u*bigbmp.bmp_size();
    auto uptr = std::make_unique<uint8_t[]>(sz);
    OutStream out(uptr.get(), sz);

    using std::chrono::microseconds;
    microseconds usec = ustime();
    microseconds cycles = rdtsc();
    bigbmp.compress(24, out);
    microseconds elapusec = ustime() - usec;
    microseconds elapcyc = rdtsc() - cycles;
    printf("initial_size = %zu, compressed size: %zu\n"
        "elapsed time = %" PRIdLEAST64 " %" PRIdLEAST64 " %f\n",
        bigbmp.bmp_size(), out.get_offset(),
        elapusec.count(), elapcyc.count(), static_cast<double>(elapcyc.count()) / elapusec.count());

    return Bitmap(24, 24, nullptr, bigbmp.cx(), bigbmp.cy(), out.get_data(), out.get_offset(), true);
}

RED_AUTO_TEST_CASE(TestBitmapCompressPerformanceBMP)
{
    {
        // make it large enough to hold any image
        Bitmap bigbmp = bitmap_from_file(FIXTURES_PATH "/color_image.bmp");
        RED_CHECK(true);
        Bitmap bmp2 = compress_perf(bigbmp);
        RED_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        RED_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }

    {
        // make it large enough to hold any image
        Bitmap bigbmp = bitmap_from_file(FIXTURES_PATH "/logo-redemption.bmp");
        RED_CHECK(true);
        Bitmap bmp2 = compress_perf(bigbmp);
        RED_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        RED_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }
}

RED_AUTO_TEST_CASE(TestBitmapCompressPerformancePNG)
{
    {
        // make it large enough to hold any image
        Bitmap bigbmp = bitmap_from_file(FIXTURES_PATH "/color_image.png");
        RED_CHECK(true);
        Bitmap bmp2 = compress_perf(bigbmp);
        RED_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        RED_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }

    {
        // make it large enough to hold any image
        Bitmap bigbmp = bitmap_from_file(FIXTURES_PATH "/logo-redemption.png");
        RED_CHECK(true);
        Bitmap bmp2 = compress_perf(bigbmp);
        RED_CHECK_EQUAL(bmp2.bmp_size(), bigbmp.bmp_size());
        RED_CHECK(0 == memcmp(bmp2.data(), bigbmp.data(), bigbmp.bmp_size()));
    }
}
