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
  Copyright (C) Wallix 2010-2012
  Author(s): Jonathan Poeleb

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestMeta
#include <boost/test/auto_unit_test.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "meta_wrm.hpp"


BOOST_AUTO_TEST_CASE(TestMeta)
{
    {
        int fd = open("/tmp/test_meta.mwrm", O_WRONLY|O_CREAT, 0655);
        BOOST_CHECK(fd > 0);
        MetaWRM meta;
        meta.width = 1024;
        meta.height = 800;
        meta.bpp = 16;
        BOOST_CHECK(1);
        meta.write(fd);
        BOOST_CHECK(1);
        close(fd);
    }

    {
        int fd = open("/tmp/test_meta.mwrm", O_RDONLY);
        BOOST_CHECK(fd > 0);
        MetaWRM meta;
        BOOST_CHECK(1);
        meta.read(fd);
        BOOST_CHECK_EQUAL(1024, meta.width);
        BOOST_CHECK_EQUAL(800, meta.height);
        BOOST_CHECK_EQUAL(16, meta.bpp);
        close(fd);
    }
}
