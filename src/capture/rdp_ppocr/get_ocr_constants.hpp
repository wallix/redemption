/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include "capture/ocr/locale/locale_id.hpp"

#include "ocr_datas_constant.hpp"

#include "ppocr/utils/read_file.hpp"

#include "utils/log.hpp"
#include "utils/strutils.hpp"

#include <cassert>


namespace rdp_ppocr
{

inline OcrDatasConstant get_ocr_constants(chars_view directory, chars_view subdirectory)
{
    try {
        auto path = [&](chars_view filename){
            return str_concat(directory, subdirectory, filename);
        };
        return OcrDatasConstant(
            ppocr::utils::load_from_file<ppocr::PpOcrDatas>(path("/datas.txt"_av).c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::Glyphs>(path("/glyphs.txt"_av).c_str()),
            ppocr::utils::load_from_file<ppocr::spell::Dictionary>(path("/dict.trie.txt"_av).c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::WWordsLines>(path("/words_lines.txt"_av).c_str()),
            ppocr::utils::load_from_file<ppocr::ocr2::Replacements>(path("/replacements.txt"_av).c_str())
        );
    }
    catch (std::exception const & e) {
        LOG(LOG_ERR, "ppocr initialization: %s", e.what());
        throw;
    }
}

} // namespace rdp_ppocr
