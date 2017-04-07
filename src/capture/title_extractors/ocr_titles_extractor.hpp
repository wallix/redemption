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

#include <vector>

#include "capture/ocr/extract_data.hh"
#include "capture/ocr/extract_bars.hh"
#include "capture/ocr/extract_text_classification.hh"

#include "capture/utils/drawable_image_view.hpp"
#include "utils/drawable.hpp"

#include "ocr_title.hpp"


struct OcrTitlesExtractor
{
    OcrTitlesExtractor(bool title_bar_only, uint8_t max_unrecog_char_rate, ocr::fonts::LocaleId locale_id)
    : locale_id(locale_id)
    , font_id(-1u)
    , title_color_id_selected(ocr::uninitialized_titlebar_color_id)
    , font_id_selected(-1u)
    , title_bar_only(title_bar_only)
    , max_unrecog_char_rate(max_unrecog_char_rate)
    {
        this->reset_letters();
    }

    void set_font_id(unsigned id, unsigned bg = ocr::uninitialized_titlebar_color_id) {
        this->font_id = id;
        this->title_color_id_selected = bg;
        if (this->font_id != -1u) {
            this->extractor.set_box_height(this->font().min_height_char, this->font().max_height_char);
        }
        else {
            this->extractor.set_box_height(
                ::ocr::fonts::min_height_font[this->locale_id],
                ::ocr::fonts::max_height_font[this->locale_id]
            );
            this->reset_letters();
        }
    }

    unsigned selected_font_id() const {
        return this->font_id;
    }

    unsigned selected_titlebar_color_id() const {
        return this->title_color_id_selected;
    }

    void extract_titles(Drawable const & drawable, std::vector<OcrTitle> & out_titles)
    {
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
                    ::ocr::Classification::result_type const res = this->classification.extract_text(
                        input, tid, box, this->locale_id, this->font_id
                    );

                    // Ignores null string
                    if (res.out.empty()) {
                        return ;
                    }
                    //LOG(LOG_INFO, "unrecog_count=%u count=%u unrecog_rate=%u max_unrecog_rate=%d",
                    //    res.unrecognized_count,
                    //    res.character_count,
                    //    res.unrecognized_rate(),
                    //    this->max_unrecog_char_rate);

                    unsigned const unrecognized_rate = res.unrecognized_rate();
                    if (unrecognized_rate <= this->max_unrecog_char_rate) {
                        if (!this->title_bar_only) {
                            is_title_bar = this->is_title_bar(input, tid, box, button_col);
                        }

                        if (this->font_id == -1u
                        && is_title_bar
                        && unrecognized_rate < this->max_unrecog_char_rate / 2u + 10u) {
                            if (this->font_id_selected != res.font_id) {
                                this->font_id_selected = res.font_id;
                                this->title_color_id_selected = tid;
                                this->reset_letters();
                            }

                            char * first = this->letters_find;
                            char * last = first + sizeof(this->letters_find);
                            while (first != last && *first) {
                                ++first;
                            }
                            std::string::const_iterator sfirst = res.out.begin();
                            std::string::const_iterator slast = res.out.end();
                            for (; sfirst != slast; ++sfirst) {
                                if (std::find(this->letters_find+0, first, *sfirst) == first) {
                                    *first = *sfirst;
                                    if (++first == last) {
                                        break;
                                    }
                                }
                            }
                            if (first == last) {
                                this->font_id = res.font_id;
                                this->extractor.set_box_height(
                                    this->font().min_height_char,
                                    this->font().max_height_char
                                );
                            }
                        }

                        tracked_area.cx = std::min<uint16_t>(input.width(), tracked_area.cx + 30);
                        out_titles.emplace_back(
                            res.out, tracked_area, is_title_bar, res.unrecognized_rate()
                        );
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
    void reset_letters()
    {
        std::fill(this->letters_find + 0, this->letters_find + sizeof(this->letters_find), 0);
    }

    bool is_title_bar(
        const DrawableImageView & input, unsigned tid,
        mln::box2d const & box, unsigned button_col) const
    {
        return ::ocr::is_title_bar(
            input, tid, box.min_row(), box.max_row(), button_col,
            this->font_id == -1u ? ::ocr::bbox_max_height : this->font().max_height_char);
    }

    ocr::fonts::Font const & font() const {
        return ::ocr::fonts::fonts[this->locale_id][this->font_id];
    }

    ocr::ExtractTextClassification classification;
    ocr::ExtractTitles extractor;

    ocr::fonts::LocaleId locale_id;
    unsigned font_id;
    unsigned title_color_id_selected;
    unsigned int font_id_selected;
    char letters_find[10];

    bool    title_bar_only;
    uint8_t max_unrecog_char_rate;
};
