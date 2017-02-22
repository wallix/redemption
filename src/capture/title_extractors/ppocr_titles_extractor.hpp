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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "capture/ocr/extract_data.hh"
#include "capture/ocr/extract_bars.hh"
#include "capture/ocr/locale/latin_to_cyrillic.hpp"

#include "ppocr/image/image.hpp"
#include "ppocr/loader2/glyphs_loader.hpp"
#include "ppocr/box_char/box.hpp"

#include "ppocr/defined_loader.hpp"

#include "capture/rdp_ppocr/ocr_datas_constant.hpp"
#include "capture/rdp_ppocr/extract_text.hpp"
#include "capture/rdp_ppocr/ocr_context.hpp"

#include "capture/utils/drawable_image_view.hpp"

#include "ocr_title.hpp"

#include <algorithm>
/*chrono*///#include <chrono>


// namespace rdp_ppocr {
//
// template<size_t... Ints>
// struct integer_sequence
// { static constexpr std::size_t size = sizeof...(Ints); };
//
// template<class Ints>
// struct first_index;
//
// template<size_t I, size_t... Ints>
// struct first_index<integer_sequence<I, Ints...>>
// { static constexpr std::size_t value = I; };
//
// using algo_indexes = integer_sequence<16, 12, 18, 19, 11, 15, 14, 10, 13, 17, 9, 5, 4, 1, 7, 8, 3, 0, 6, 2>;
// constexpr size_t first_algo = first_index<algo_indexes>::value;
// constexpr size_t count_algo_base = algo_indexes::size;
//
// }

struct PpOcrTitlesExtractor
{
    PpOcrTitlesExtractor(
        rdp_ppocr::OcrDatasConstant const & ocr_constant,
        bool title_bar_only, uint8_t max_unrecog_char_rate, ocr::locale::LocaleId locale_id)
    : ocr_constant{ocr_constant}
    , ocr_context{this->ocr_constant.glyphs.size()}
    , locale_id(locale_id)
    , title_color_id_selected(ocr::uninitialized_titlebar_color_id)
    , title_bar_only(title_bar_only)
    , max_unrecog_char_rate(max_unrecog_char_rate)
    {}

    void reset_titlebar_color_id(unsigned bg = ocr::uninitialized_titlebar_color_id) {
        this->title_color_id_selected = bg;
    }

    unsigned selected_titlebar_color_id() const {
        return this->title_color_id_selected;
    }

    void extract_titles(Drawable const & drawable, std::vector<OcrTitle> & out_titles) {
        using ImageView = DrawableImageView;

        auto process_title = [this, &drawable, &out_titles](
            const ImageView & input, unsigned tid,
            mln::box2d const & box, unsigned button_col
        ) {
            Rect tracked_area(box.min_col(), box.min_row(), box.ncols(), box.nrows());
            if ((drawable.tracked_area != tracked_area) || drawable.tracked_area_changed)
            {
                /* TODO
                 * Change here to disable check of maximize/minimize/close icons at end of title bar
                 * It can be either BARRE_TITLE if closing buttons were detected or BARRE_OTHER
                 * drawback is that will lead to detection of non title bars containing garbage text
                 * See if we do not get too much false positives when disabling BARRE_TITLE check
                 * we should also fix these false positives using some heuristic
                 */
                bool is_title_bar = this->title_bar_only
                    ? this->is_title_bar(input, tid, box, button_col)
                    : true;

                if (is_title_bar) {
                    unsigned unrecognized_count = rdp_ppocr::extract_text(
                        this->ocr_constant, this->ocr_context,
                        input, tid, box, button_col
                    );

                    auto & result = this->ocr_context.result;

                    if (!result.empty()) {
                        //LOG(LOG_INFO, "unrecog_count=%u count=%u unrecog_rate=%u max_unrecog_rate=%d",
                        //    res.unrecognized_count,
                        //    res.character_count,
                        //    res.unrecognized_rate(),
                        //    this->max_unrecog_char_rate);

                        if (this->locale_id == ::ocr::locale::LocaleId::cyrillic) {
                            this->latin_to_cyrillic.latin_to_cyrillic(result);
                        }

                        if (!this->title_bar_only) {
                            is_title_bar = this->is_title_bar(input, tid, box, button_col);
                        }

                        uint8_t const unrecognized_rate = unrecognized_count * 100 / this->ocr_context.ambiguous.size();
                        if (unrecognized_rate <= this->max_unrecog_char_rate) {
                            tracked_area.cx = std::min<uint16_t>(input.width(), tracked_area.cx + 30);
                            out_titles.emplace_back(
                                result, tracked_area, is_title_bar, unrecognized_rate
                            );

                            if (unrecognized_rate < this->max_unrecog_char_rate / 2u + 10u && is_title_bar){
                                this->title_color_id_selected = tid;
                            }
                        }
                    }
                }
            }
            //else
            //{
            //    LOG(LOG_INFO, "Title bar remains unchanged");
            //}
        };

        this->extractor.extract_titles(ImageView(drawable), process_title, this->title_color_id_selected);
    }

private:
    bool is_title_bar(
        const DrawableImageView & input, unsigned tid,
        mln::box2d const & box, unsigned button_col) const
    {
        return ::ocr::is_title_bar(
            input, tid, box.min_row(), box.max_row(), button_col, ::ocr::bbox_max_height);
    }

    ocr::locale::latin_to_cyrillic_context latin_to_cyrillic;

    rdp_ppocr::OcrDatasConstant const & ocr_constant;
    rdp_ppocr::OcrContext ocr_context;

    ocr::ExtractTitles extractor;

    ocr::locale::LocaleId locale_id;
    unsigned title_color_id_selected;

    bool    title_bar_only;
    uint8_t max_unrecog_char_rate;
};
