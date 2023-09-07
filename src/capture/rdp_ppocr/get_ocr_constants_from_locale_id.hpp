/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "capture/ocr/locale/locale_id.hpp"

namespace rdp_ppocr
{

class OcrDatasConstant;

OcrDatasConstant const & get_ocr_constants_from_locale_id(ocr::locale::LocaleId locale_id);

} // namespace rdp_ppocr
