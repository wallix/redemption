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

#include "ppocr/defined_loader.hpp"
#include "ppocr/spell/dictionary.hpp"
#include "ppocr/ocr2/glyphs.hpp"
#include "ppocr/ocr2/word_lines.hpp"
#include "ppocr/ocr2/words_infos.hpp"
#include "ppocr/ocr2/replacement.hpp"
#include "ppocr/ocr2/indexes_ordered.hpp"
#include "ppocr/ocr2/views_index_ordered.hpp"
#include "ppocr/ocr2/data_indexes_ordered.hpp"
#include "ppocr/ocr2/data_indexes_by_words.hpp"

#include <vector>

namespace rdp_ppocr {

namespace detail_ {
    template<class>
    struct first_type;

    template<template<class...> class Template, class T, class... Ts>
    struct first_type<Template<T, Ts...>>
    { using type = T; };
}

struct OcrDatasConstant {
    ppocr::PpOcrDatas const datas;
    ppocr::ocr2::Glyphs const glyphs;
    ppocr::spell::Dictionary const dict;
    std::vector<unsigned> const id_views;
    ppocr::ocr2::DataIndexesByWords const data_indexes_by_words;

    ppocr::ocr2::WordsInfos const words_infos;

    ppocr::ocr2::Replacements const replacements;

    using FirstLoaderStrategy = typename detail_::first_type<ppocr::PpOcrDatas>::type;
    ppocr::ocr2::DataIndexesOrdered<FirstLoaderStrategy> const first_strategy_ortered;


    OcrDatasConstant(OcrDatasConstant const &) = delete;
    OcrDatasConstant& operator=(OcrDatasConstant const &) = delete;

    OcrDatasConstant(
        ppocr::PpOcrDatas && datas_,
        ppocr::ocr2::Glyphs && glyphs_,
        ppocr::spell::Dictionary && dict_,
        ppocr::ocr2::WWordsLines const & wwords_lines_,
        ppocr::ocr2::Replacements && replacements_
    )
    : datas(std::move(datas_))
    , glyphs(std::move(glyphs_))
    , dict(std::move(dict_))
    , id_views(ppocr::ocr2::get_views_indexes_ordered(glyphs))
    , data_indexes_by_words(glyphs)
    , words_infos(glyphs, wwords_lines_)
    , replacements(std::move(replacements_))
    , first_strategy_ortered(datas)
    {}
};

}
