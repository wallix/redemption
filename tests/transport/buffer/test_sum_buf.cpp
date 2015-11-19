/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2013
 * Author(s): Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestSumBuf
#include <boost/test/auto_unit_test.hpp>


#include "buffer/sum_buf.hpp"
#include "buffer/null_buf.hpp"

#include <fstream>
#include <sstream>

template<size_t N>
long write(transbuf::osum_buf<transbuf::null_buf> & buf, char const (&s)[N]) {
    return buf.write(s, N-1);
}

template<size_t N>
long write(transbuf::osum_and_mini_sum_buf<transbuf::null_buf> & buf, char const (&s)[N]) {
    return buf.write(s, N-1);
}

BOOST_AUTO_TEST_CASE(TestOSumBuf)
{
    using osum_buf = transbuf::osum_buf<transbuf::null_buf>;

    osum_buf buf;
    BOOST_CHECK_EQUAL(write(buf, "ab"), 2);
    BOOST_CHECK_EQUAL(write(buf, "cde"), 3);

    BOOST_CHECK_EQUAL(buf.sum(), 'a'+'b'+'c'+'d'+'e');
}

BOOST_AUTO_TEST_CASE(TestOSumAndMiniSumBuf)
{
    {
        using osum_buf = transbuf::osum_and_mini_sum_buf<transbuf::null_buf>;
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{10});
        BOOST_CHECK_EQUAL(write(buf, "ab"), 2);
        BOOST_CHECK_EQUAL(write(buf, "cde"), 3);

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c'+'d'+'e');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum());
    }

    using osum_buf = transbuf::osum_and_mini_sum_buf<transbuf::null_buf>;

    {
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{5});
        write(buf, "abcde");

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c'+'d'+'e');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum());
    }

    {
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{4});
        write(buf, "abcde");

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c'+'d');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum()+'e');
    }

    {
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{5});
        write(buf, "ab");
        write(buf, "cd");
        write(buf, "e");

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c'+'d'+'e');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum());
    }

    {
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{5});
        write(buf, "ab");
        write(buf, "cd");
        write(buf, "ef");

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c'+'d'+'e');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum()+'f');
    }

    {
        osum_buf buf(transbuf::sum_and_mini_sum_buf_param<>{3});
        write(buf, "ab");
        write(buf, "cd");
        write(buf, "e");

        BOOST_CHECK_EQUAL(buf.mini_sum(), 'a'+'b'+'c');
        BOOST_CHECK_EQUAL(buf.sum(), buf.mini_sum()+'d'+'e');
    }
}
