/*
* Copyright (C) 2016 Wallix
* 
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
* 
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
* 
* You should have received a copy of the GNU Lesser General Public License along
* with this library; if not, write to the Free Software Foundation, Inc., 59
* Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "ppocr/ocr2/word_lines.hpp"

#include "ppocr/utils/read_file.hpp"

#include <istream>

std::istream& ppocr::ocr2::operator>>(std::istream& is, WordLines & wlines) {
    unsigned ascentline;
    unsigned capline;
    unsigned meanline;
    unsigned baseline;

    if (is >> ascentline >> capline >> meanline >> baseline) {
        wlines.ascentline = ascentline;
        wlines.capline = capline;
        wlines.meanline = meanline;
        wlines.baseline = baseline;
    }
    return is;
}

std::istream& ppocr::ocr2::operator>>(std::istream& is, WWordsLines& wwords_lines)
{
    WWordLines wwl;
    std::string s;
    while (is >> s >> wwl.wlines) {
        if (s.size() <= 8) {
            memcpy(wwl.str_data, s.data(), s.size());
            wwl.str_len = static_cast<uint16_t>(s.size());
            wwords_lines.push_back(wwl);
        }
    }
    return is;
}
