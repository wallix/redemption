/*
 * T his program *is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPng
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "drawable.hpp"
#include "scale.hpp"
#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestPngScale)
{
    Drawable drawable(800,600);
    Bitmap bmp(FIXTURES_PATH"/color_image.bmp");
    drawable.mem_blt(Rect(0,0,800,600), bmp, 0,0,0,1);

    const unsigned int dest_width = 260;
    const unsigned int dest_height = 160;

    char message[1024];
    uint8_t * data = (uint8_t*)malloc(dest_width * dest_height * 3);
    scale_data(data, drawable.data, dest_width, drawable.width, dest_height, drawable.height, drawable.rowsize);
    if (!check_sig(data, dest_height, dest_width * 3, message,
                   "\x56\x89\xdd\x66\x89\x3f\x74\xcf\x61\xbf"
                   "\xf1\xa8\x92\x2e\x7c\x32\x9f\xad\xfc\xfb"))
    {
        BOOST_REQUIRE_MESSAGE(false, message);
    }
    free(data);
}
