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
 *   Copyright (C) Wallix 2010-2020
 */

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"

#define RED_CHECK_IMG_SIG_A(obj, expected_sig)                 \
    [&](::ut::ImgVSig const& imgvsig){                         \
        RED_TEST((void(#obj), imgvsig.hex()) == expected_sig); \
    }(::ut::ImgVSig(obj, __LINE__))

#define RED_CHECK_IMG_SIG(obj, expected_sig) \
    RED_CHECK_IMG_SIG_A(obj, expected_sig ""_av)

class ConstImageDataView;

namespace ut
{
    struct ImgSig
    {
        unsigned char sig[20];

        using value_type = unsigned char;

        operator bytes_view () const noexcept { return bytes_view(sig, sizeof(sig)); }
    };

    ImgSig img_sig(ConstImageDataView const& img);

    struct ImgVSig
    {
        int line;
        unsigned long count_error;
        ConstImageDataView const& img;
        ImgSig sig;

        ImgVSig(ConstImageDataView const& img, int line);
        ~ImgVSig();

        flagged_bytes_view hex() const noexcept { return ut::hex(sig); }
    };
}
