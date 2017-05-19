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

#include "capture/rdp_ppocr/get_ocr_constants.hpp"
#include "title_extractor_api.hpp"
#include "ocr_titles_extractor.hpp"
#include "ppocr_titles_extractor.hpp"
#include "ocr_title_filter.hpp"
#include "core/defines.hpp"
#include "configs/autogen/enums.hpp"


template<class Extractor>
class DrawableTitleExtractor : public TitleExtractorApi
{
    Drawable const & drawable;
    bool enable_title_log;
    std::vector<OcrTitle> titles;
    OcrTitleFilter title_filter;
    Extractor extractor;

public:
    template<class... ExtractorArgs>
    DrawableTitleExtractor(
        Drawable const & drawable,
        bool enable_title_log,
        ExtractorArgs && ... extractor_args
    )
    : drawable(drawable)
    , enable_title_log(enable_title_log)
    , extractor(std::forward<ExtractorArgs>(extractor_args)...)
    {}

    /// \return unchanged title = {nullptr, 0} ; no title = {x, 0}
    array_view_const_char extract_title() override
    {
        this->titles.clear();

        try {
            this->extractor.extract_titles(this->drawable, titles);
        } catch (const std::exception &e) {
            LOG(LOG_ERR, "exception caught in flush (ocr): %s", e.what());
            throw Error(ERR_RECORDER_SNAPSHOT_FAILED);
        }

        auto const best_index = this->title_filter.extract_best_title(this->titles);

        if (this->title_filter.has_title_changed()) {
            OcrTitle const & title = this->titles[best_index];
            if (this->enable_title_log) {
                LOG(LOG_INFO, "Title rect: x=%u y=%u cx=%u cy=%u title=%s",
                    title.rect.x, title.rect.y, title.rect.cx, title.rect.cy, title.text.c_str());
            }
            const_cast<Drawable&>(this->drawable).tracked_area         = title.rect;
            const_cast<Drawable&>(this->drawable).tracked_area_changed = false;

            //return this->titles[best_index].text;
            return this->title_filter.get_title();
        }

        return {};
    }
};

using OcrDrawableTitleExtractor = DrawableTitleExtractor<OcrTitlesExtractor>;
using PpOcrDrawableTitleExtractor = DrawableTitleExtractor<PpOcrTitlesExtractor>;

class OcrTitleExtractorBuilder
{
    bool is_ocr2;
    union Variant {
        PpOcrDrawableTitleExtractor v2;
        OcrDrawableTitleExtractor v1;
        struct {} dummy;

        Variant() : dummy() {}
        ~Variant() {}
    } variant;

public:
    OcrTitleExtractorBuilder(
        Drawable const & drawable, bool enable_title_log,
        OcrVersion ocr_version, ocr::locale::LocaleId locale_id,
        bool title_bar_only, uint8_t max_unrecog_char_rate)
    : is_ocr2(ocr_version == OcrVersion::v2)
    {
        if (is_ocr2) {
            new (&this->variant.v2) PpOcrDrawableTitleExtractor(
                drawable, enable_title_log,
                rdp_ppocr::get_ocr_constants(CFG_PATH, locale_id)
                , title_bar_only, max_unrecog_char_rate, locale_id
            );
        }
        else {
            new (&this->variant.v1) OcrDrawableTitleExtractor(
                drawable, enable_title_log,
                title_bar_only, max_unrecog_char_rate, locale_id
            );
        }
    }

    ~OcrTitleExtractorBuilder() {
        if (this->is_ocr2) {
            this->variant.v2.~PpOcrDrawableTitleExtractor();
        }
        else {
            this->variant.v1.~OcrDrawableTitleExtractor();
        }
    }

    TitleExtractorApi & get_title_extractor() {
        if (this->is_ocr2) {
            return this->variant.v2;
        }
        return this->variant.v1;
    }
};
