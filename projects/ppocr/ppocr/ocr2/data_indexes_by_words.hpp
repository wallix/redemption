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

#ifndef PPOCR_SRC_OCR2_DATA_INDEXES_BY_WORDS_HPP
#define PPOCR_SRC_OCR2_DATA_INDEXES_BY_WORDS_HPP

#include "ppocr/ocr2/glyphs.hpp"

#include <vector>


namespace ppocr { namespace ocr2 {

struct DataIndexesByWords
{
    DataIndexesByWords(DataIndexesByWords &&) = default;
    DataIndexesByWords(DataIndexesByWords const &) = delete;

    DataIndexesByWords(Glyphs const & glyphs);

    std::vector<unsigned> const & operator[](std::size_t i) const noexcept {
        return this->indexes_by_words[i];
    }

private:
    std::vector<std::vector<unsigned>> indexes_by_words;
};


} }

#endif
