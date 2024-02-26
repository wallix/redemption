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

#include "ppocr/ocr2/data_indexes_by_words.hpp"

#include <utility>
#include <map>

ppocr::ocr2::DataIndexesByWords::DataIndexesByWords(const ppocr::ocr2::Glyphs& glyphs)
{
    std::map<unsigned, std::vector<unsigned>> map;

    unsigned n = 0;
    for (auto & views : glyphs) {
        auto & e = map[views.front().word];
        e.emplace_back(n++);
    }

    this->indexes_by_words.resize(glyphs.size());
    for (auto & p : map) {
        if (p.second.size() > 1) {
            auto tmp = p.second.back();
            p.second.pop_back();
            indexes_by_words[tmp] = p.second;
            for (auto & i : p.second) {
                using std::swap;
                swap(i, tmp);
                indexes_by_words[tmp] = p.second;
            }
        }
    }
}
