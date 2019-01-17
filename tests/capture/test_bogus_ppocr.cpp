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
   Author(s): Jonathan Poelen

   Unit test to detect memory leak in OCR module
*/

#define RED_TEST_MODULE TestNewOCR
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/app_path.hpp"

#include "capture/title_extractors/ppocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"

#include "utils/drawable.hpp"
#include "utils/bitmap_from_file.hpp"

#include "capture/rdp_ppocr/get_ocr_constants.hpp"


namespace {
    template<class... Str>
    std::array<char const *, sizeof...(Str)>
    make_array(Str... str) {
        return {{str...}};
    }

    void draw_bitmap(Drawable & drawable, char const * bitmap_filename) {
        Bitmap bmp = bitmap_from_file(bitmap_filename);
        drawable.draw_bitmap({0, 0, drawable.width(), drawable.height()}, bmp);
    }

    auto & ocr_constants = rdp_ppocr::get_ocr_constants(app_path_s(AppPath::Cfg) + "/ppocr.latin-cyrillic");
}


RED_AUTO_TEST_CASE(TestNewOCR4)
{
    Drawable drawable(800, 600);

    using ocr::locale::LocaleId;
    PpOcrTitlesExtractor extractor(ocr_constants, true, 100, LocaleId{LocaleId::latin});

    OcrTitleFilter filter;
    std::vector<OcrTitle> out_titles;

    {
        draw_bitmap(drawable, FIXTURES_PATH "/win2012capture1.png");
        extractor.extract_titles(drawable, out_titles);

        auto expected = make_array(
            "a b c d e f g h ij k l m n o p q r s t u v w xyz - Bloc-notes"
        );
        RED_CHECK_EQUAL(out_titles.size(), expected.size());

        auto idx_best = filter.extract_best_title(out_titles);
        RED_CHECK_EQUAL(idx_best, 0);
        RED_CHECK_EQUAL(expected[idx_best], filter.get_title().data());

        out_titles.clear();
    }
}
