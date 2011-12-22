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

        uint8_t comp64x64RED[] = { 0xc0, 0x30, 0x00, 0x00, 0xFF, 0xf0, 0xc0, 0x0f, };
        BGRPalette palette332;
        init_palette332(palette332);
        Bitmap bloc64x64(24, &palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true );
        RDPBmpCache cmd(24, &bloc64x64, 1, 10);
        gtf.draw(cmd);
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
            struct Cache {
                class BmpCache bmp;
            } cache;
        public:
            Consumer(const Rect & screen_rect) : TestConsumer(screen_rect){}
            void check_end()
            {
                BOOST_CHECK_EQUAL(icount, 1);
            }
        private:
            virtual void draw(const RDPBmpCache & cmd)
            {
                icount++;
                this->cache.bmp.put(cmd.id, cmd.idx, cmd.bmp);
                switch (icount){
                case 1:
                {
                    BOOST_CHECK_EQUAL((uint8_t)1, cmd.id);
                    BOOST_CHECK_EQUAL((uint16_t)10, cmd.idx);
                    Bitmap * bmp = this->cache.bmp.get(cmd.id, cmd.idx);
                    BOOST_CHECK_EQUAL((uint16_t)64, bmp->cx);
                    BOOST_CHECK_EQUAL((uint16_t)64, bmp->cy);
                    BOOST_CHECK_EQUAL((uint8_t)24, bmp->original_bpp);
                    BOOST_CHECK_EQUAL((uint32_t)2984132952, bmp->get_crc());
                }
                break;
                default:
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


BOOST_AUTO_TEST_CASE(TestGraphicsToFile_ActuallyDrawAnImage)
{
    Rect screen_rect(0, 0, 800, 600);

    char tmpname[128];
    {
        sprintf(tmpname, "/tmp/test_gtf_chunk1XXXXXX");
        int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
        OutFileTransport trans(fd);
        GraphicsToFile gtf(&trans, NULL);
        BGRPalette palette332;
        init_palette332(palette332);

        gtf.draw(RDPOpaqueRect(Rect(0, 0, 800, 600), 0), screen_rect);
        gtf.draw(RDPOpaqueRect(Rect(0, 0, 800, 600), 0), Rect(10, 10, 100, 100));

        uint8_t comp64x64RED[] = { 0xc0, 0x30, 0x00, 0x00, 0xFF, 0xf0, 0xc0, 0x0f, };
        Bitmap bloc64x64(24, &palette332, 64, 64, comp64x64RED, sizeof(comp64x64RED), true );
        gtf.draw(RDPBmpCache(24, &bloc64x64, 1, 10));
        gtf.draw(RDPMemBlt(1, Rect(5, 5, 20, 20), 0xCC, 0, 0, 10), screen_rect);
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
            int width;
            int height;
            int bpp;
            unsigned long pix_len;
            uint8_t * data;
            BGRPalette palette;

            struct Cache {
                class BitmapCache {
                    Bitmap * cache[3][8192];
                    public:
                        BitmapCache(){
                            for (uint8_t cid = 0; cid < 3; cid++){
                                for (uint16_t cidx = 0; cidx < 8192; cidx++){
                                    cache[cid][cidx] = NULL;
                                }
                            }
                        }
                        ~BitmapCache(){
                            for (uint8_t cid = 0; cid < 3; cid++){
                                for (uint16_t cidx = 0; cidx < 8192; cidx++ ){
                                    if (cache[cid][cidx]){
                                        delete cache[cid][cidx];
                                        cache[cid][cidx] = NULL;
                                    }
                                }
                            }
                        }
                        void put(uint8_t id, uint16_t idx, Bitmap * bmp){
                            if (cache[id][idx]){
                                delete cache[id][idx];
                            }
                            cache[id][idx] = bmp;
                        }
                        Bitmap * get(uint8_t id, uint16_t idx){
                            return cache[id][idx];
                        }
                } bmp;
            } cache;
        public:
            Consumer(const Rect & screen_rect)
                : TestConsumer(screen_rect),
                    width(800),
                    height(600),
                    bpp(24),
                    pix_len(this->width * this->height * nbbytes(this->bpp))
            {
                init_palette332(this->palette);
                this->data = (uint8_t *)calloc(sizeof(char), this->pix_len);
            }
            ~Consumer(){
                if (this->data){
                    free(data);
                }
            }

        private:
            virtual void flush() {}
            virtual void draw(const RDPBmpCache & cmd)
            {
                this->cache.bmp.put(cmd.id, cmd.idx, cmd.bmp);
            }
            virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
            {
                const Rect trect = clip.intersect(cmd.rect);
                uint32_t color = color_decode(cmd.color, this->bpp, this->palette);

                // base adress (*3 because it has 3 color components)
                uint8_t * base = this->data + (trect.y * this->width + trect.x) * 3;
                for (int j = 0; j < trect.cy ; j++){
                    for (int i = 0; i < trect.cx ; i++){
                       uint8_t * p = base + (j * this->width + i) * 3;
                       p[0] = color >> 16; // r
                       p[1] = color >> 8;  // g
                       p[2] = color;       // b
                    }
                }
            }
            virtual void draw(const RDPMemBlt & cmd, const Rect & clip)
            {
                BOOST_CHECK(true);
                const uint8_t cache_id = cmd.cache_id & 0xFF;
                const Rect & rect = cmd.rect;
                const uint16_t srcx = cmd.srcx;
                const uint16_t srcy = cmd.srcy;
                const uint16_t cache_idx = cmd.cache_idx;
                Bitmap * bmp = this->cache.bmp.get(cache_id, cache_idx);
                const uint8_t * const bmp_data = bmp->data_co(this->bpp);
                BOOST_CHECK(true);

                // Where we draw -> target
                uint32_t px = 0;
                uint8_t r = 0;
                uint8_t g = 0;
                uint8_t b = 0;
                uint8_t * target = this->data + (rect.y * this->width + rect.x) * 3;
                BOOST_CHECK(true);
                for (int j = 0; j < rect.cy ; j++){
                    for (int i = 0; i < rect.cx ; i++){
                        if (!(clip.contains_pt(i + rect.x, j + rect.y))) {
                          continue;
                        }
                        uint32_t src_px_offset = ((rect.cy - j - srcy - 1) * align4(rect.cx) + i + srcx) * nbbytes(this->bpp);
                        px = (bmp_data[src_px_offset+2]<<16)
                           + (bmp_data[src_px_offset+1]<<8)
                           + (bmp_data[src_px_offset+0]);
                        r = (px >> 16) & 0xFF;
                        g = (px >> 8)  & 0xFF;
                        b =  px        & 0xFF;
                        // Pixel assignment (!)
                        uint8_t * pt = target + (j * this->width + i) * 3;
                        pt[0] = b;
                        pt[1] = g;
                        pt[2] = r;
                    }
                }
            }
            public:
            void dump_png(void){
                char rawImagePath[256]     = {0};
                char rawImageMetaPath[256] = {0};
                snprintf(rawImagePath,     254, "/dev/shm/%d.png", getpid());
                snprintf(rawImageMetaPath, 254, "%s.meta", rawImagePath);
                FILE * fd = fopen(rawImageMetaPath, "w");
                if (fd) {
                   fprintf(fd, "%d,%d\n", this->width, this->height);
                }
                fclose(fd);
                fd = fopen(rawImagePath, "w");
                if (fd) {
                    const uint8_t Bpp = 3;
                    png_struct * ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                    png_info * pinfo = png_create_info_struct(ppng);

                    // prepare png header
                    png_init_io(ppng, fd);
                    png_set_IHDR(ppng, pinfo, this->width, this->height, 8,
                                 PNG_COLOR_TYPE_RGB,
                                 PNG_INTERLACE_NONE,
                                 PNG_COMPRESSION_TYPE_BASE,
                                 PNG_FILTER_TYPE_BASE);
                    png_write_info(ppng, pinfo);

                    // send image buffer to file, one pixel row at once
                    for (uint32_t k = 0; k < (uint32_t)this->height; ++k ) {
                        png_write_row(ppng, this->data + k*width*Bpp);
                    }
                    png_write_end(ppng, pinfo);
                    png_destroy_write_struct(&ppng, &pinfo);
                    // commented line below it to create row capture
                    // fwrite(this->data, 3, this->width * this->height, fd);
                }
                fclose(fd);
            }

        } consumer(screen_rect);

        RDPUnserializer reader(&in_trans, &consumer, screen_rect);
        reader.next();
        reader.next();
        reader.next();
        reader.next();
        consumer.dump_png();
        // check we have read everything
        BOOST_CHECK_EQUAL(stream.end - stream.p, 0);
        ::close(fd);
    }
    ::unlink(tmpname);

}
