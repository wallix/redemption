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

#include "ppocr/box_char/box.hpp"
#include "ppocr/image/image.hpp"

#include "ppocr/ocr2/cache.hpp"
#include "ppocr/ocr2/ambiguous.hpp"
#include "ppocr/ocr2/probabilities.hpp"
#include "ppocr/ocr2/image_context.hpp"

namespace rdp_ppocr {

struct OcrContext
{
    std::vector<unsigned> spaces;
    std::vector<ppocr::Box> boxes;

    ppocr::ocr2::Probabilities probabilities;
    ppocr::ocr2::Probabilities tmp_probabilities;

    ppocr::ocr2::ImageContext img_ctx;

    ppocr::ocr2::image_cache_type_t images_cache;

    ppocr::ocr2::ambiguous_t ambiguous;

    std::string result;

    explicit OcrContext(std::size_t datas_sz)
    : probabilities(datas_sz)
    , tmp_probabilities(datas_sz)
    {
        result.reserve(128);
        spaces.reserve(32);
        boxes.reserve(64);
    }
};

} // namespace rdp_ppocr
