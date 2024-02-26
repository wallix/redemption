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

#ifndef PPOCR_SRC_OCR2_WORDS_INFOS_HPP
#define PPOCR_SRC_OCR2_WORDS_INFOS_HPP

#include "ppocr/ocr2/word_lines.hpp"
#include "ppocr/ocr2/glyphs.hpp"

#include <vector>


namespace ppocr { namespace ocr2 {

struct WordInfos {
    unsigned word;
    WordLines lines;
};

struct WordsInfos {
    WordsInfos() = default;
    WordsInfos(WordsInfos &&) = default;
    WordsInfos(WordsInfos const &) = delete;

    WordsInfos(Glyphs const & glyphs, WWordsLines const & wwords_lines)
    {
        for (auto & p : wwords_lines) {
            auto idx = glyphs.word_index_of(p.sv());
            if (idx != Glyphs::no_index) {
                this->words_infos.push_back({idx, p.wlines});
            }
        }

        std::sort(
            this->words_infos.begin(), this->words_infos.end(),
            [](WordInfos const & wi1, WordInfos const & wi2) {
                return wi1.word < wi2.word;
            }
        );
    }

    WordInfos const * get(unsigned word) const {
        auto it = std::lower_bound(
            this->words_infos.begin(), this->words_infos.end(), word,
            [](WordInfos const & info, unsigned w) {
                return info.word < w;
            }
        );
        return it != this->words_infos.end() && it->word == word ? &*it : nullptr;
    }

private:
    std::vector<WordInfos> words_infos;
};

} }

#endif
