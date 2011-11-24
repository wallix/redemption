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
#include "./test_orders.hpp"

#include "../capture/GraphicToFile.hpp"
#include "../core/constants.hpp"

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



BOOST_AUTO_TEST_CASE(TestGraphicsToFile_one_simple_chunk)
{
    Rect screen_rect(0, 0, 800, 600);

    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL);
        RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0);
        gtf.draw(cmd, screen_rect);
        gtf.flush();
        ::close(fd);
    }
    
    {    
        // reread data from file
        int fd = ::open(tmpname, O_RDONLY);
        BOOST_CHECK(fd > 0); 
        Stream stream(4096);
        InFileTransport in_trans(fd);
        in_trans.recv(&stream.end, 8);
        BOOST_CHECK_EQUAL(stream.end - stream.p, 8); 
        uint16_t chunk_type = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(chunk_type, (uint16_t)RDP_UPDATE_ORDERS); 
        uint16_t chunk_size = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(chunk_size, (uint16_t)15); 
        uint16_t order_count = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(order_count, (uint16_t)1); 
        uint16_t pad = stream.in_uint16_le();
        BOOST_CHECK_EQUAL(pad, (uint16_t)0); // really we don't care

        in_trans.recv(&stream.end, chunk_size - 8);

        // initial order and clip
        RDPOrderCommon common(RDP::PATBLT, Rect(0, 0, 1, 1));
        // initial rect state
        RDPOpaqueRect cache_rect(Rect(), 0);

        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(control, (uint8_t)(RDP::STANDARD|RDP::CHANGE)); 

        RDPPrimaryOrderHeader header = common.receive(stream, control);
        RDPOpaqueRect cmd = cache_rect;
        cmd.receive(stream, header);

        check<RDPOpaqueRect>(common, cmd,
            // the clip was not changed as encoded opaquerect was fully inside it
            // => no need to clip, we still have initial clipping 
            // (no BOUNDS in control above)
            RDPOrderCommon(RDP::RECT, Rect(0, 0, 1, 1)),
            RDPOpaqueRect(screen_rect, 0),
            "Reading back Rect 1");
        // check we have read everything
        BOOST_CHECK_EQUAL(stream.end - stream.p, 0); 
        ::close(fd);
    }
    ::unlink(tmpname);

}


BOOST_AUTO_TEST_CASE(TestGraphicsToFile_one_simple_chunk_reading_with_unserialize)
{
    Rect screen_rect(0, 0, 800, 600);

    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL);
        RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0);
        gtf.draw(cmd, screen_rect);
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
                void check_end() { BOOST_CHECK_EQUAL(this->icount, 1); }
        private:
            virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
            {
                this->icount++;
                if (this->icount == 1){
                    BOOST_CHECK(cmd == RDPOpaqueRect(Rect(0, 0, 800, 600), 0));
                    BOOST_CHECK(this->screen_rect == clip);
                }
                else {
                    BOOST_CHECK(false);
                }
            }
        } consumer(screen_rect);

        RDPUnserializer reader(&in_trans, &consumer, screen_rect);
        reader.next();
        consumer.check_end();
        // check we have read everything
        BOOST_CHECK_EQUAL(stream.end - stream.p, 0); 
        ::close(fd);
    }
    ::unlink(tmpname);

}


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
        consumer.check_end();
        // check we have read everything
        BOOST_CHECK_EQUAL(stream.end - stream.p, 0); 
        ::close(fd);
    }
    ::unlink(tmpname);

}

BOOST_AUTO_TEST_CASE(TestGraphicsToFile_SecondaryOrderCache)
{
    Rect screen_rect(0, 0, 800, 600);

    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL);

//        uint8_t comp64x64RED[] = { 0xc0, 0x30, 0x00, 0x00, 0xFF, 0xf0, 0xc0, 0x0f, };
//        BGRPalette palette332;
//        init_palette332(palette332);
//        Bitmap bloc64x64(24, &palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true );
//        const int bitmap_cache_version = 1;
//        const int use_bitmap_comp = 0;
//        const int op2 = 0;
//        RDPBmpCache cmd(24, &bloc64x64, 1, 10, bitmap_cache_version, use_bitmap_comp, op2);
//        gtf.draw(cmd);
        gtf.flush();
        ::close(fd);
    }
//    
//    {    
//        // reread data from file
//        int fd = ::open(tmpname, O_RDONLY);
//        BOOST_CHECK(fd > 0); 
//        Stream stream(4096);
//        InFileTransport in_trans(fd);
//        class Consumer : public TestConsumer {
//        public:
//            Consumer(const Rect & screen_rect) : TestConsumer(screen_rect){}
//            void check_end() 
//            {
//                BOOST_CHECK_EQUAL(icount, 2);
//            }
//        private:
//            virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
//            {
//                icount++;
//                switch (icount){
//                case 1:
//                    BOOST_CHECK(cmd == RDPOpaqueRect(Rect(0, 0, 800, 600), 0));
//                    BOOST_CHECK(this->screen_rect == clip);
//                break;
//                case 2:
//                    BOOST_CHECK(cmd == RDPOpaqueRect(Rect(0, 0, 800, 600), 0));
//                    BOOST_CHECK(Rect(10, 10, 100, 100) == clip);
//                break;
//                default:
//                    BOOST_CHECK(false);
//                }
//            }
//        } consumer(screen_rect);

//        RDPUnserializer reader(&in_trans, &consumer, screen_rect);
//        reader.next();
//        reader.next();
//        consumer.check_end();
//        // check we have read everything
//        BOOST_CHECK_EQUAL(stream.end - stream.p, 0); 
//        ::close(fd);
//    }
    ::unlink(tmpname);

}

