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

   Unit test to writing RDP orders to file and rereading them

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGraphicsToFile
#include <boost/test/auto_unit_test.hpp>
#include "test_orders.hpp"

#include "GraphicToFile.hpp"
#include "constants.hpp"
#include "bmpcache.hpp"
#include <png.h>

class TestConsumer : public RDPGraphicDevice {
protected:
    unsigned icount;
    const Rect screen_rect;
public:
        TestConsumer(const Rect & screen_rect)
        : icount(0), screen_rect(screen_rect) {}
        void check_end() { BOOST_CHECK(false); }
private:
    virtual void flush() {};
    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPScrBlt & cmd, const Rect &clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPDestBlt & cmd, const Rect &clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPPatBlt & cmd, const Rect &clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bmp)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPLineTo& cmd, const Rect & clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPBrushCache & cmd)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPColCache & cmd)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPGlyphCache & cmd)
    {
        BOOST_CHECK(false);
    }
};



BOOST_AUTO_TEST_CASE(TestGraphicsToFile_one_simple_chunk)
{
    Rect screen_rect(0, 0, 800, 600);

    timeval now = { 212, 10 };
    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        BOOST_CHECK(fd > 0);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL, 24, 8192, 768, 8192, 3072, 8192, 12288, now);
        now.tv_sec += 5;
        now.tv_usec += 1100;
        gtf.timestamp(now);
        gtf.flush();
        ::close(fd);
    }

    {
        // reread data from file
        int fd = ::open(tmpname, O_RDONLY);
        BOOST_CHECK(fd > 0);
        Stream stream(4096);

        InFileTransport in_trans(fd);

        in_trans.recv(&stream.end, 16);
        BOOST_CHECK_EQUAL(stream.end - stream.p, 16);
        uint16_t chunk_type = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(chunk_type, (uint16_t)GraphicsToFile::TIMESTAMP);
        uint16_t chunk_size = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(chunk_size, (uint16_t)16);
        uint16_t order_count = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(order_count, (uint16_t)1);
        uint16_t pad = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(pad, (uint16_t)0); // really we don't care

        uint64_t micro_sec;
        stream.in_copy_bytes((uint8_t*)&micro_sec, sizeof(micro_sec));
        BOOST_CHECK_EQUAL(micro_sec, uint64_t(5ull * 1000000ull + 1100ull));

        BOOST_CHECK_EQUAL(stream.p, stream.end);

        ::close(fd);
    }
    ::unlink(tmpname);

}
