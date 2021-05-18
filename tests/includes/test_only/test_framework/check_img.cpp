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

#include "test_only/test_framework/check_img.hpp"

#include "utils/image_view.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/png.hpp"
#include "utils/strutils.hpp"

#include <cstring>

#if !defined(REDEMPTION_UNIT_TEST_FAST_CHECK)
# define REDEMPTION_UNIT_TEST_FAST_CHECK 0
#endif

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
# include "test_only/test_framework/impl/test_paths.hpp"
# include "utils/bitmap_private_data.hpp"
# include <boost/test/results_reporter.hpp>
# include <charconv>
#endif


namespace
{
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

    bool img_equals(ImageView const& img1, ImageView const& img2)
    {
        // because unimplemented...
        assert(img1.bits_per_pixel() == BitsPerPixel(24));
        assert(img1.bits_per_pixel() == img2.bits_per_pixel());

        size_t w = img1.width();
        size_t h = img1.height();

        if (w != img2.width() || h != img2.height()) {
            return false;
        }

        uint8_t const* p1 = img1.data();
        uint8_t const* p2 = img2.data();
        size_t rowsize1 = img1.line_size();
        size_t rowsize2 = img2.line_size();
        uint8_t const* endp = p1 + h * rowsize1;

        if (img1.storage_type() == img2.storage_type()) {
            for (; p1 != endp; p1 += rowsize1, p2 += rowsize2){
                if (0 != memcmp(p1, p2, w * 3)) {
                    return false;
                }
            }
        }
        else {
            for (; p1 != endp; p2 += rowsize2){
                endp -= rowsize1;
                if (0 != memcmp(endp, p2, w * 3)) {
                    return false;
                }
            }
        }

        return true;
    }

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    Bitmap create_diff_img(ImageView const& img1, ImageView const& img2)
    {
        // because unimplemented...
        assert(img1.bits_per_pixel() == BitsPerPixel(24));
        assert(img1.bits_per_pixel() == img2.bits_per_pixel());

        const uint16_t w1 = img1.width();
        const uint16_t h1 = img1.height();
        const size_t rowsize1 = img1.line_size();
        uint8_t const* p1 = img1.data();

        const uint16_t w2 = img2.width();
        const uint16_t h2 = img2.height();
        const size_t rowsize2 = img2.line_size();
        uint8_t const* p2 = img2.data();

        const uint16_t w = std::max(w1, w2);
        const uint16_t h = std::max(h1, h2);

        const uint16_t minw = std::min(w1, w2);
        const uint16_t minh = std::min(h1, h2);

        Bitmap imgr;
        Bitmap::PrivateData::Data & data = Bitmap::PrivateData::initialize_png(imgr, w, h);

        const size_t rowsize = imgr.line_size();
        const size_t remaining = rowsize - minw * 3;
        uint8_t* p = data.get();

        auto set_pixline = [minw, remaining](uint8_t* p, uint8_t const* p1, uint8_t const* p2){
            for (auto* e = p + minw * 3; p != e; p += 3, p1 += 3, p2 += 3) {
                int is_same = (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2]);
                std::memset(p, is_same ? 0 : 0xff, 3);
            }
            std::memset(p, 0, remaining);
        };

        uint8_t* endp = p + minh * rowsize;

        if (img1.storage_type() == img2.storage_type()) {
            for (; p != endp; p += rowsize, p1 += rowsize1, p2 += rowsize2){
                set_pixline(p, p1, p2);
            }
        }
        else if (img1.storage_type() == ImageView::Storage::BottomToTop) {
            p1 += h1 * rowsize1;
            for (; p != endp; p += rowsize, p2 += rowsize2){
                p1 -= rowsize1;
                set_pixline(p, p1, p2);
            }
        }
        else {
            p2 += h2 * rowsize2;
            for (; p != endp; p += rowsize, p1 += rowsize1){
                p2 -= rowsize2;
                set_pixline(p, p2, p1);
            }
        }

        std::memset(p, 0xff, (h - minh) * rowsize);

        return imgr;
    }
#endif

    std::string check_img(ImageView const& img, char const* filedata_path, char const* img_path)
    {
        std::string err;
        std::string_view prefix_error;

        Bitmap bmp = bitmap_from_file_impl(filedata_path, BGRColor());
        if (bmp.is_valid()) {
            if (!img_equals(img, bmp)) {
                prefix_error = "\"Image differ\"";
            }
        }
        else {
            prefix_error = "\"Load bmp error\"";
        }

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
        if (prefix_error.size() && not do_not_save_images()) {
            std::string const& current_test_name
                = boost::unit_test::framework::current_test_case().p_name.get();
            if (previous_test_name != current_test_name) {
                img_counter = 0;
                previous_test_name = current_test_name;
            }
            ++img_counter;

            using std::begin;
            using std::end;

            char buf2[32];
            auto r2 = std::to_chars(begin(buf2), end(buf2), img_counter);

            std::string path = ut_impl::compute_test_path(
                "err-", array_view{buf2, r2.ptr},
                ".diff.png");

            if (bmp.is_valid()) {
                dump_png24(path.c_str(), create_diff_img(img, bmp), true);
            }

            path.erase(path.size() - 8, 5);
            if (!img_path) {
                dump_png24(path.c_str(), img, true);
            }

            chars_view msgdiff2 = "(image ref not found)"_av;
            chars_view msgdiff3;
            if (bmp.is_valid()) {
                msgdiff2 = array_view(path).drop_back(4);
                msgdiff3 = ".diff.png"_av;
            }
            err = str_concat(
                "  ", prefix_error,
                "\n  Image path: ", img_path ? std::string_view(img_path) : std::string_view(path),
                "\n  Image diff: ", msgdiff2, msgdiff3,
                "\n  Image ref: ", filedata_path,
                "\n"
            );
        }
        else
#endif

        if (prefix_error.size()) {
            err = prefix_error;
        }

        return err;
    }
}

namespace ut
{
    bool CheckImg::operator()(char const* img_path, char const* filedata_path)
    {
        Bitmap img = bitmap_from_file_impl(img_path, BGRColor());
        auto img_view = img.is_valid()
            ? img
            : ImageView{nullptr, 0, 0, 0, BitsPerPixel::BitsPP24, ImageView::Storage::BottomToTop, };
        this->err = check_img(img_view, filedata_path, img_path);
        return this->err.empty();
    }

    bool CheckImg::operator()(ImageView const& img, char const* filedata_path)
    {
        this->err = check_img(img, filedata_path, nullptr);
        return this->err.empty();
    }

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
    CheckImg::~CheckImg()
    {
        boost::unit_test::results_reporter::get_stream() << this->err;
    }
#endif
}
