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

#include "../capture/GraphicToFile.hpp"
#include "../core/constants.hpp"

BOOST_AUTO_TEST_CASE(TestGraphicsToFile_one_simple_chunk)
{
    Rect clip(0, 0, 800, 600);

    char tmpname[128];
    sprintf(tmpname, "/tmp/test_transportXXXXXX");
    int fd = ::mkostemp(tmpname, O_WRONLY|O_CREAT);
    OutFileTransport trans(fd);
    GraphicsToFile gtf(&trans, NULL);
    RDPOpaqueRect cmd(Rect(0, 0, 800, 600), 0);
    gtf.send(cmd, clip);
    gtf.flush();
    ::close(fd);
    
    // reread data from file
    fd = ::open(tmpname, O_RDONLY);
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
    uint8_t control = stream.in_uint8();
    BOOST_CHECK_EQUAL(control, (uint8_t)(RDP::STANDARD|RDP::CHANGE)); 
    RDPPrimaryOrderHeader header = common.receive(stream, control);
    BOOST_CHECK_EQUAL((uint8_t)RDP::RECT, common.order);
    // the clip was not changed as encoded opaquerect was fully inside it
    // => no need to clip
    BOOST_CHECK_EQUAL(Rect(0, 0, 1, 1), common.clip);

    ::close(fd);
}
