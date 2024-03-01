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
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "capture/utils/drawable_image_view.hpp"
#include "capture/ocr/extract_bars.hh"
#include "ocr_datas_constant.hpp"
#include "ocr_context.hpp"

#include "ppocr/ocr2/compute_image.hpp"
#include "ppocr/ocr2/filter_by_lines.hpp"
#include "ppocr/ocr2/filter_by_font.hpp"
#include "ppocr/ocr2/disambiguous_with_dict.hpp"

#include <mln/image/image2d.hh>


namespace rdp_ppocr {

namespace detail_ {
    using ImageView = DrawableImageView;

    template<class ImageView, class TitlebarColor>
    ppocr::Image const &
    to_img(ppocr::ocr2::ImageContext & img_ctx, const ImageView & input, TitlebarColor const & tcolor, ppocr::Box const & box)
    {
        unsigned const h = box.height();
        unsigned const w = box.width();
        return img_ctx.img({w, h}, [&](ppocr::Pixel * data) {
            const unsigned icol = box.x();
            for (unsigned y = 0, iy = box.y(); y < h; ++y, ++iy) {
                auto * pline = &data[y * w];
                for (unsigned x = 0, ix = icol; x < w; ++x, ++ix) {
                    pline[x] = tcolor.threshold_chars(input[{ix, iy}]) ? 'x' : '-';
                }
            }
        });
    }

    template<class ImageView, class TitlebarColor>
    bool horizontal_empty(TitlebarColor const & tcolor, unsigned char const * d, unsigned w) {
        for (auto e = d+w*3; d != e; d += 3) {
            if (tcolor.threshold_chars(typename ImageView::Color{d})) {
                return false;
            }
        }
        return true;
    }

    template<class ImageView, class TitlebarColor>
    bool vertical_empty(TitlebarColor const & tcolor, unsigned char const * d, unsigned w, unsigned h) {
        for (auto e = d + w * h * 3; d != e; d += w * 3) {
            if (tcolor.threshold_chars(typename ImageView::Color{d})) {
                return false;
            }
        }
        return true;
    }

    template<class ImageView, class TitlebarColor>
    ppocr::Box box_character(
        ImageView const & input, TitlebarColor const & tcolor,
        unsigned x1, unsigned y1, unsigned x2, unsigned y2
    ) {
        auto const W = input.width();
        unsigned x = x1;

        auto d = input[{x, y1}].c;
        for (; x < x2; ++x) {
            if (!vertical_empty<ImageView>(tcolor, d, W, y2 - y1)) {
                break;
            }
            d += 3;
        }

        unsigned w = x;

        while (w + 1 < x2) {
            ++w;
            if ([&](unsigned char const * d, unsigned /*w*/, unsigned h) -> bool {
                using Color = typename ImageView::Color;
                for (auto e = d+W*h*3; d != e; d += W*3) {
                    if (tcolor.threshold_chars(Color{d}) && (
                        (d+3 != e && tcolor.threshold_chars(Color{d+3}))
                    || (d-W*3+3 >= input[{0, 0}].c && tcolor.threshold_chars(Color{d-W*3+3}))
                    || (d+W*3+3 < e && tcolor.threshold_chars(Color{d+W*3+3}))
                    )) {
                        return false;
                    }
                }
                return true;
            }(d, x2, y2 - y1)) {
                break;
            }
            d += 3;
        }
        w -= x;

        unsigned y = y1;

        d = input[{x, y}].c;
        for (; y < y2; ++y) {
            if (!horizontal_empty<ImageView>(tcolor, d, w)) {
                break;
            }
            d += W * 3;
        }

        unsigned h = y2;

        d = input[{x, h}].c;
        while (--h > y) {
            d -= W * 3;
            if (!horizontal_empty<ImageView>(tcolor, d, w)) {
                break;
            }
        }
        h -= y;

        ++h;

        return {{x, y}, {w, h}};
    }

    template<class ImageView, class TColor>
    void shrink_hbox(
        ImageView const & input, TColor const & tcolor,
        unsigned x1, unsigned & y1, unsigned x2, unsigned & y2
    ) {
        auto d = input[{x1, y1}].c;
        while (y1 < y2 && horizontal_empty<ImageView>(tcolor, d, x2)) {
            ++y1;
            d += input.width() * 3;
        }

        if (y2 > y1) {
            d = input[{x1, y2}].c - input.width() * 3;
            while (y2 > y1 && horizontal_empty<ImageView>(tcolor, d, x2)) {
                d -= input.width() * 3;
                --y2;
            }
        }
    }
} // namespace detail_

/**
 * \return unrecognized_count
 */
template<class ImageView>
unsigned extract_text(
    rdp_ppocr::OcrDatasConstant const & ocr_constant,
    rdp_ppocr::OcrContext & ocr_context,
    const ImageView & input, unsigned tid,
    ppocr::Box const & box, unsigned button_col)
{
    (void)button_col;

    ::ocr::dispatch_title_color(tid, [&](auto const& tcolor, bool is_win2012){
        /*chrono*///using resolution_clock = std::chrono::high_resolution_clock;
        /*chrono*///auto t1 = resolution_clock::now();

        unsigned x1 = box.x();
        unsigned y1 = box.y();
        unsigned x2 = x1 + box.width();
        unsigned y2 = y1 + box.height();
        detail_::shrink_hbox(input, tcolor, x1, y1, x2, y2);

        ocr_context.ambiguous.clear();

        std::vector<ppocr::Box> & boxes = ocr_context.boxes;
        std::vector<unsigned> & spaces = ocr_context.spaces;
        boxes.clear();
        spaces.clear();

        unsigned const whitespace = (is_win2012 ? /*4*/5 : 3);

        unsigned i_space = 0;
        while (auto const cbox = detail_::box_character(input, tcolor, x1, y1, x2, y2)) {
            //min_y = std::min(cbox.y(), min_y);
            //bounds_y = std::max(cbox.y() + cbox.height(), bounds_y);
            //bounds_x = cbox.x() + cbox.width();
            //std::cerr << "\nbox(" << cbox << ")\n";
            auto & img_word = detail_::to_img(ocr_context.img_ctx, input, tcolor, cbox);
            //std::cout << img_word << std::endl;

            if (x1 + whitespace <= cbox.x()) {
                spaces.push_back(i_space);
            }
            ++i_space;

            auto it = ocr_context.images_cache.find(img_word);
            if (it != ocr_context.images_cache.end()) {
                ocr_context.ambiguous.emplace_back(it->second);
            }
            else {
                auto it = ocr_context.images_cache.emplace(
                    img_word.clone(),
                    ppocr::ocr2::compute_image(
                        ppocr::PpOcrSimpleDatas{},
                        ppocr::PpOcrComplexDatas{},
                        ppocr::PpOcrExclusiveDatas{},
                        ocr_context.probabilities,
                        ocr_context.tmp_probabilities,
                        ocr_constant.datas,
                        ocr_constant.first_strategy_ortered,
                        ocr_constant.data_indexes_by_words,
                        ocr_constant.glyphs,
                        ocr_constant.id_views,
                        img_word,
                        ocr_context.img_ctx.img90()
                    )
                ).first;
                ocr_context.ambiguous.emplace_back(it->second);
            }

            boxes.push_back(cbox);

            x1 = cbox.x() + cbox.width();
        }
    });

    ppocr::ocr2::filter_by_lines(
        /*ocr_context.filter_by_lines_context,*/
        ocr_context.ambiguous,
        ocr_constant.words_infos,
        ocr_context.boxes
    );

    ppocr::ocr2::filter_by_font(
        /*ocr_context.filter_by_font_context,*/
        ocr_context.ambiguous
    );

    /*chrono*///auto t2 = resolution_clock::now();
    /*chrono*///std::cerr << "t2: " << std::chrono::duration<double>(t2-t1).count() << "s\n";

    std::string & result = ocr_context.result;
    ocr_context.spaces.push_back(ocr_context.ambiguous.size());
    unsigned unrecognized_count = ppocr::ocr2::disambiguous_with_dict(
        ocr_context.ambiguous,
        ocr_constant.glyphs,
        ocr_constant.dict,
        ocr_context.spaces.cbegin(),
        result
    );

    /*chrono*///auto t3 = resolution_clock::now();
    /*chrono*///std::cerr << "t3: " << std::chrono::duration<double>(t3-t1).count() << "s\n";

    ppocr::ocr2::replace_words(
        ocr_context.result,
        ocr_constant.replacements
    );
    // TODO recall dict ?

    //std::cout << " ## result3: " << (result3) << "\n";

    /*chrono*///auto t4 = resolution_clock::now();
    /*chrono*///std::cerr << "t4: " << std::chrono::duration<double>(t4-t1).count() << "s\n";

    return unrecognized_count;
}

} // namespace rdp_ppocr
