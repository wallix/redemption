/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#pragma once

#include <cstdio>
#include <cstring>

#include "system/ssl_sha1.hpp"
#include "utils/image_data_view.hpp"
#include "utils/sugar/bytes_view.hpp"

// fix tests with:
// bjam test_flat_wab_close | grep ^tests/ | while IFS='()"' read f l e n E s ; do sed $l,$(($l+1))'s/RED_CHECK_SIG(drawable.gd, .*/RED_CHECK_SIG(drawable.gd, "'${s//x/\\\\x}');/' -i "$f" ; done

#define RED_CHECK_SIG(obj, expected_sig)                           \
    do {                                                           \
        uint8_t obj_sig__[SslSha1::DIGEST_LENGTH];                 \
        ::redemption_unit_test__::compute_obj_sig(obj_sig__, obj); \
        RED_CHECK_MEM_AC(obj_sig__, expected_sig);                 \
    } while (0)

#define RED_CHECK_SIG_FROM(obj, array_for_sig)                     \
    do {                                                           \
        uint8_t obj_sig__[SslSha1::DIGEST_LENGTH];                 \
        uint8_t expected_sig__[SslSha1::DIGEST_LENGTH];            \
        ::redemption_unit_test__::compute_av_sig(expected_sig__,   \
            make_array_view(array_for_sig));                       \
        ::redemption_unit_test__::compute_obj_sig(obj_sig__, obj); \
        RED_CHECK_MEM_AA(obj_sig__, expected_sig__);               \
    } while (0)

class InStream;
class OutStream;

namespace redemption_unit_test__
{
    using SigArray = uint8_t[SslSha1::DIGEST_LENGTH];

    inline void compute_av_sig(SigArray& sig, const_bytes_view av)
    {
        SslSha1 sha1;
        sha1.update(av);
        sha1.final(sig);
    }

    inline void compute_av_sig2(SigArray& sig, uint8_t const* p, size_t height, size_t rowsize)
    {
        SslSha1 sha1;
        for (size_t y = 0; y < height; y++){
            sha1.update({p + y * rowsize, rowsize});
        }
        sha1.final(sig);
    }

    // authorize incomplete type in compute_obj_sig
    template<class Stream>
    inline void compute_stream_sig(SigArray& sig, Stream const& stream)
    {
        ::redemption_unit_test__::compute_av_sig(sig, {stream.get_data(), stream.get_offset()});
    }

    template<class T>
    void compute_obj_sig(SigArray& sig, T const& obj)
    {
        if constexpr (std::is_convertible<T const&, ConstImageDataView>::value)
        {
            ConstImageDataView image = obj;
            ::redemption_unit_test__::compute_av_sig2(
                sig, image.data(), image.height(), image.line_size());
        }
        else if constexpr (std::is_convertible<T const&, InStream const&>::value
                        || std::is_convertible<T const&, OutStream const&>::value)
        {
          ::redemption_unit_test__::compute_stream_sig(sig, obj);
        }
        else
        {
            const_bytes_view ar = obj;
            ::redemption_unit_test__::compute_av_sig2(sig, ar.data(), 1, ar.size());
        }
    }
}
