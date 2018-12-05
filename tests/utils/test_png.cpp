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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Martin Potier

   Unit test to write / read a "movie" from a file
   Using lib boost functions for testing
*/


#define RED_TEST_MODULE TestPng
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/png.hpp"
#include "utils/image_data_view.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/fileutils.hpp"
#include "transport/transport.hpp"


ConstImageDataView const img(
    byte_ptr_cast("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"),
    3, 3, 4, BytesPerPixel{3}, ConstImageDataView::Storage::TopToBottom);

RED_AUTO_TEST_CASE(TestPngWriteFile)
{
    char const* filename = "/tmp/dump_png24_1.png";
    dump_png24(filename, img, false);
    RED_CHECK_EQ(filesize(filename), 68);
    unlink(filename);
}

RED_AUTO_TEST_CASE(TestPngTransError)
{
    struct : Transport
    {
        int i = 0;
        void do_send(const uint8_t * /*buffer*/, size_t /*len*/) override
        {
            ++i;
        }

        void flush() override
        {
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
    } trans;

    RED_CHECK_EXCEPTION_ERROR_ID(dump_png24(trans, img, false), ERR_TRANSPORT_WRITE_FAILED);
    RED_CHECK_EQ(trans.i, 9);
}
