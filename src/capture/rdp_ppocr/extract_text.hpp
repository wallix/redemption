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
#include "ocr_datas_constant.hpp"
#include "ocr_context.hpp"

#include "ppocr/ocr2/compute_image.hpp"
#include "ppocr/ocr2/filter_by_lines.hpp"
#include "ppocr/ocr2/filter_by_font.hpp"
#include "ppocr/ocr2/disambiguous_with_dict.hpp"


namespace rdp_ppocr {

namespace detail_ {
    using ImageView = DrawableImageView;

    template<class ImageView, class TitlebarColor>
    ppocr::Image const &
    to_img(ppocr::ocr2::ImageContext & img_ctx, const ImageView & input, TitlebarColor const & tcolor, mln::box2d const & box)
    {
        unsigned const h = box.nrows();
        unsigned const w = box.ncols();
        return img_ctx.img({w, h}, [&](ppocr::Pixel * data) {
            const unsigned icol = box.min_col();
            for (unsigned y = 0, iy = box.min_row(); y < h; ++y, ++iy) {
                auto * pline = &data[y * w];
                for (unsigned x = 0, ix = icol; x < w; ++x, ++ix) {
                    pline[x] = tcolor.threshold_chars(input(iy, ix)) ? 'x' : '-';
                }
            }
        });
    }

    template<class ImageView, class TitlebarColor>
    bool horizontal_empty(TitlebarColor const & tcolor, unsigned char const * d, size_t w) {
        for (auto e = d+w*3; d != e; d += 3) {
            if (tcolor.threshold_chars(typename ImageView::Color{d})) {
                return false;
            }
        }
        return true;
    }

    template<class ImageView, class TitlebarColor>
    bool vertical_empty(TitlebarColor const & tcolor, unsigned char const * d, size_t w, size_t h) {
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
        ppocr::Index const & idx, ppocr::Bounds const & bnd
    ) {
        auto const W = input.width();
        size_t x = idx.x();

        auto d = input(idx.y(), x).c;
        for (; x < bnd.w(); ++x) {
            if (!vertical_empty<ImageView>(tcolor, d, W, bnd.h() - idx.y())) {
                break;
            }
            d += 3;
        }

        size_t w = x;

        while (w + 1 < bnd.w()) {
            ++w;
            if ([&](unsigned char const * d, size_t /*w*/, size_t h) -> bool {
                using Color = typename ImageView::Color;
                for (auto e = d+W*h*3; d != e; d += W*3) {
                    if (tcolor.threshold_chars(Color{d}) && (
                        (d+3 != e && tcolor.threshold_chars(Color{d+3}))
                    || (d-W*3+3 >= input(0, 0).c && tcolor.threshold_chars(Color{d-W*3+3}))
                    || (d+W*3+3 < e && tcolor.threshold_chars(Color{d+W*3+3}))
                    )) {
                        return false;
                    }
                }
                return true;
            }(d, bnd.w(), bnd.h() - idx.y())) {
                break;
            }
            d += 3;
        }
        w -= x;

        size_t y = idx.y();

        d = input(y, x).c;
        for (; y < bnd.h(); ++y) {
            if (!horizontal_empty<ImageView>(tcolor, d, w)) {
                break;
            }
            d += W * 3;
        }

        size_t h = bnd.h();

        d = input(h, x).c;
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
    void shrink_box(
        ImageView const & input, TColor const & tcolor,
        size_t & x, size_t & y, size_t & w, size_t & h
    ) {
        auto d = input(y, x).c;
        while (y < h && horizontal_empty<ImageView>(tcolor, d, w)) {
            ++y;
            d += input.width() * 3;
        }
        d = input(h, x).c - input.width() * 3;
        while (h > y && horizontal_empty<ImageView>(tcolor, d, w)) {
            d -= input.width() * 3;
            --h;
        }
    }
}

/**
 * \return unrecognized_count
 */
template<class ImageView>
unsigned extract_text(
    rdp_ppocr::OcrDatasConstant const & ocr_constant,
    rdp_ppocr::OcrContext & ocr_context,
    const ImageView & input, unsigned tid,
    mln::box2d const & box, unsigned button_col)
{
    (void)button_col;

    bool const is_win2012 = (tid == ::ocr::titlebar_color_id::WINDOWS_2012);
    bool const is_win2012_vnc = (tid == ::ocr::titlebar_color_id::WINDOWS_2012_VNC);

    /*chrono*///using resolution_clock = std::chrono::high_resolution_clock;
    /*chrono*///auto t1 = resolution_clock::now();

    size_t x = box.min_col();
    size_t y = box.min_row();
    size_t w = static_cast<unsigned>(box.max_col()+1);
    size_t h = static_cast<unsigned>(box.max_row()+1);
    if (is_win2012) {
        detail_::shrink_box(input, ::ocr::titlebar_color_windows2012_standard(), x, y, w, h);
    }
    else if (is_win2012_vnc) {
        detail_::shrink_box(input, ::ocr::titlebar_color_windows2012_vnc_standard(), x, y, w, h);
    }
    else {
        detail_::shrink_box(input, ::ocr::titlebar_colors[tid], x, y, w, h);
    }
    auto const bounds = ppocr::Bounds{w, h};

    ocr_context.ambiguous.clear();

    std::vector<ppocr::Box> & boxes = ocr_context.boxes;
    std::vector<unsigned> & spaces = ocr_context.spaces;
    boxes.clear();
    spaces.clear();

    size_t const whitespace = (is_win2012 || is_win2012_vnc) ? /*4*/5 : 3;

    unsigned i_space = 0;
    while (auto const cbox = is_win2012
        ? detail_::box_character(input, ::ocr::titlebar_color_windows2012_standard(), {x, y}, bounds)
        : is_win2012_vnc
        ? detail_::box_character(input, ::ocr::titlebar_color_windows2012_vnc_standard(), {x, y}, bounds)
        : detail_::box_character(input, ::ocr::titlebar_colors[tid], {x, y}, bounds)
    ) {
        //min_y = std::min(cbox.y(), min_y);
        //bounds_y = std::max(cbox.y() + cbox.h(), bounds_y);
        //bounds_x = cbox.x() + cbox.w();
        //std::cerr << "\nbox(" << cbox << ")\n";

        mln::box2d new_box(mln::point2d(cbox.y(), cbox.x()), mln::point2d(cbox.bottom(), cbox.right()));
        auto & img_word = is_win2012
            ? detail_::to_img(ocr_context.img_ctx, input, ::ocr::titlebar_color_windows2012_standard(), new_box)
            : is_win2012_vnc
            ? detail_::to_img(ocr_context.img_ctx, input, ::ocr::titlebar_color_windows2012_vnc_standard(), new_box)
            : detail_::to_img(ocr_context.img_ctx, input, ::ocr::titlebar_colors[tid], new_box);
        //std::cout << img_word << std::endl;

        if (x + whitespace <= cbox.x()) {
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

        x = cbox.x() + cbox.w();
    }

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

}
