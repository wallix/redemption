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
#define BOOST_TEST_MODULE TestGraphicsToFile4
#include <boost/test/auto_unit_test.hpp>
#include "test_orders.hpp"

#include "RDP/RDPDrawable.hpp"
#include "GraphicToFile.hpp"
#include "constants.hpp"
#include "RDP/caches/bmpcache.hpp"
#include "ssl_calls.hpp"
#include "png.hpp"

BOOST_AUTO_TEST_CASE(TestGraphicsToFileReadBogus)
{
    class Consumer : public RDPGraphicDevice {
        RDPDrawable * data;
        size_t disable;
    public:
        Consumer()
            : RDPGraphicDevice()
            , disable(false)
        {
            this->data = new RDPDrawable(800, 600, true);
        }
        virtual ~Consumer(){
            delete this->data;
        }

    private:
        virtual void flush() {};
        virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip)
        {
//            cmd.print(clip);
            this->data->draw(cmd, clip);
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
//            cmd.print(clip);
            this->data->draw(cmd, clip, bmp);
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

        public:
        void dump_png(void){
            char tmpname[128];
            sprintf(tmpname, "/tmp/test_bug10_XXXXXX.png");
            int fd = ::mkostemps(tmpname, 4, O_WRONLY|O_CREAT);
            FILE * f = fdopen(fd, "wb");
            Drawable & d = this->data->drawable;
            ::dump_png24(f, d.data, d.width, d.height, d.rowsize);
            ::fclose(f);
        }
    } consumer;

    int fd = ::open(FIXTURES_PATH "/bug9.wrm", O_RDONLY); BOOST_CHECK(fd > 0);

    InFileTransport in_trans(fd);

    RDPUnserializer reader(&in_trans, &consumer, Rect(0, 0, 800, 600));
    size_t i = 0;
    for (i = 0; ; i++){
        if (!reader.next()){
            break;
        }
    }

    BOOST_CHECK_EQUAL(2733, i);

// to see last result file, remove unlink
// and do something like:
// eog `ls -1tr /tmp/test_* | tail -n 1`
// (or any other variation you like)

    consumer.dump_png();

    ::close(fd);

}
