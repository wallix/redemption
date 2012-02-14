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
#define BOOST_TEST_MODULE TestGraphicsToFile3
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
    virtual void draw(const RDPMemBlt & cmd, const Rect & clip)
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
    virtual void draw(const RDPBmpCache & cmd)
    {
        BOOST_CHECK(false);
    }
    virtual void draw(const RDPGlyphCache & cmd)
    {
        BOOST_CHECK(false);
    }
};


BOOST_AUTO_TEST_CASE(TestGraphicsToFile_several_chunks)
{
    Rect screen_rect(0, 0, 800, 600);

    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL);
        gtf.draw(RDPOpaqueRect(Rect(0, 0, 800, 600), 0), screen_rect);
        gtf.timestamp();
        gtf.draw(RDPOpaqueRect(Rect(0, 0, 800, 600), 0), Rect(10, 10, 100, 100));
        gtf.flush();
        ::close(fd);
    }

    {
        // reread data from file
        int fd = ::open(tmpname, O_RDONLY);
        BOOST_CHECK(fd > 0);
        Stream stream(4096);
        InFileTransport in_trans(fd);
        class Consumer : public TestConsumer {
        public:
            Consumer(const Rect & screen_rect) : TestConsumer(screen_rect){}
            void check_end()
            {
                BOOST_CHECK_EQUAL(icount, 2);
            }
        private:
            virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
            {
                icount++;
                switch (icount){
                case 1:
                    BOOST_CHECK(cmd == RDPOpaqueRect(Rect(0, 0, 800, 600), 0));
                    BOOST_CHECK(this->screen_rect == clip);
                break;
                case 2:
                    BOOST_CHECK(cmd == RDPOpaqueRect(Rect(0, 0, 800, 600), 0));
                    BOOST_CHECK(Rect(10, 10, 100, 100) == clip);
                break;
                default:
                    BOOST_CHECK(false);
                }
            }
        } consumer(screen_rect);

        RDPUnserializer reader(&in_trans, &consumer, screen_rect);
        reader.next();
        reader.next();
        reader.next();
        consumer.check_end();
        // check we have read everything
        BOOST_CHECK_EQUAL(stream.end - stream.p, 0);
        ::close(fd);
    }
    ::unlink(tmpname);
}
