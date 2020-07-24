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

#include "test_only/test_framework/img_sig.hpp"
#include "test_only/test_framework/impl/test_paths.hpp"

#include "system/ssl_sha1.hpp"
#include "utils/image_data_view.hpp"
#include "utils/png.hpp"

#include <cstring>

#if !defined(REDEMPTION_UNIT_TEST_FAST_CHECK)
# define REDEMPTION_UNIT_TEST_FAST_CHECK 0
#endif

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include <charconv>
# include <iostream>
#endif


namespace
{
    template<size_t>
    struct type_n {};

    void update_img_sig(ut::ImgSig& sig, ConstImageDataView const& img)
    {
        // check ImgSig::sig size
        static_assert(sizeof(type_n<sizeof(ut::ImgSig::sig)>{} = type_n<SslSha1::DIGEST_LENGTH>{}));

        SslSha1 sha1;
        uint8_t const* p = img.data();
        size_t height = img.height();
        size_t rowsize = img.line_size();
        for (size_t y = 0; y < height; y++){
            sha1.update({p + y * rowsize, rowsize});
        }
        uint8_t buf[SslSha1::DIGEST_LENGTH];
        sha1.final(buf);

        std::memcpy(sig.sig, buf, SslSha1::DIGEST_LENGTH);
    }

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    bool do_not_save_images()
    {
        static const bool r = []{
            char const* s = std::getenv("REDEMPTION_TEST_DO_NOT_SAVE_IMAGES");
            return s && *s == '1';
        }();
        return r;
    }

    std::string previous_test_name;
    int img_counter = 0;
#endif
}

namespace ut
{
    ImgSig img_sig(ConstImageDataView const& img)
    {
        // check ImgSig::sig size
        static_assert(sizeof(type_n<sizeof(ImgSig::sig)>{} = type_n<SslSha1::DIGEST_LENGTH>{}));

        ImgSig sig;
        update_img_sig(sig, img);
        return sig;
    }

    ImgVSig::ImgVSig(ConstImageDataView const& img, int line)
    : line(line)
#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    , count_error(RED_ERROR_COUNT())
#endif
    , img(img)
    {
        update_img_sig(this->sig, img);
    }

    ImgVSig::~ImgVSig()
    {
#if !REDEMPTION_UNIT_TEST_FAST_CHECK
        if (not do_not_save_images() && count_error != RED_ERROR_COUNT()) {
            std::string const& current_test_name
                = boost::unit_test::framework::current_test_case().p_name.get();
            if (previous_test_name != current_test_name) {
                img_counter = 0;
                previous_test_name = current_test_name;
            }
            ++img_counter;

            using std::begin;
            using std::end;

            char buf1[32];
            auto r1 = std::to_chars(begin(buf1), end(buf1), this->line);

            char buf2[32];
            auto r2 = std::to_chars(begin(buf2), end(buf2), img_counter);

            std::string path = ut_impl::compute_test_path(
                "line-", array_view{buf1, r1.ptr},
                "_err-", array_view{buf2, r2.ptr},
                ".png");
            dump_png24(path.c_str(), img, false);
            std::cerr << "Image path: " << path << '"' << std::endl;
        }
#endif
    }
}
