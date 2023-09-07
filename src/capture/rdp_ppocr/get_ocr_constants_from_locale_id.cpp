/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "capture/rdp_ppocr/get_ocr_constants.hpp"
#include "capture/rdp_ppocr/get_ocr_constants_from_locale_id.hpp"

#include "core/app_path.hpp"

#include <cassert>


rdp_ppocr::OcrDatasConstant const &
rdp_ppocr::get_ocr_constants_from_locale_id(ocr::locale::LocaleId locale_id)
{
    if (locale_id == ocr::locale::LocaleId::cyrillic) {
        static const rdp_ppocr::OcrDatasConstant constants = [&] {
            return get_ocr_constants(app_path(AppPath::Cfg), "/ppocr.latin-cyrillic"_av);
        }();
        return constants;
    }
    else {
        static const rdp_ppocr::OcrDatasConstant constants = [&] {
            return get_ocr_constants(app_path(AppPath::Cfg), "/ppocr.latin"_av);
        }();
        return constants;
    }
}
