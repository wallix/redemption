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

#include "ppocr/ocr2/glyphs.hpp"

#include <limits>

namespace ppocr { namespace ocr2 {

std::ostream & operator<<(std::ostream & os, Glyphs::string const & str) {
    return os.write(str.data(), str.size());
}

std::istream & operator>>(std::istream & is, Glyphs & glyphs) {
    std::map<std::string, unsigned> font_map;
    std::map<std::string, unsigned> word_map;

    std::string font;
    std::string word;
    constexpr auto limit_max = std::numeric_limits<std::streamsize>::max();

    while (is.ignore(limit_max, '\n')) {
        unsigned n;
        if (!(is >> n)) {
            break;
        }
        Views views;
        views.resize(n);

        unsigned i = 0;
        while (i < n && (is >> word >> font).ignore(limit_max, '\n')) {
            auto it_word = word_map.find(word);
            if (it_word == word_map.end()) {
                it_word = word_map.emplace(std::move(word), word_map.size()).first;
            }
            views[i].word = it_word->second;

            auto it_font = font_map.find(font);
            if (it_font == font_map.end()) {
                it_font = font_map.emplace(std::move(font), font_map.size()).first;
            }
            views[i].font = it_font->second;

            ++i;
        }

        glyphs.push_back(std::move(views));
    }

    std::unique_ptr<unsigned[]> buf_word;
    unsigned buf_word_sz = 0;
    {
        std::vector<unsigned> reindex(word_map.size());
        for (auto & p : word_map) {
            reindex[p.second] = buf_word_sz;
            buf_word_sz += (p.first.size() + sizeof(unsigned) - 1) / sizeof(unsigned) + 1;
        }
        buf_word = std::unique_ptr<unsigned[]>(new unsigned[buf_word_sz]);
        for (auto & views : glyphs) {
            for (View & v : views) {
                v.word = reindex[v.word];
            }
        }
    }
    {
        auto buf = buf_word.get();
        for (auto & p : word_map) {
            *buf = p.first.size();
            ++buf;
            std::copy(p.first.begin(), p.first.end(), reinterpret_cast<char*>(buf));
            reinterpret_cast<char*>(buf)[p.first.size()] = 0;
            buf += (p.first.size() + sizeof(unsigned) - 1) / sizeof(unsigned);
        }
    }

    glyphs.buf_word = std::move(buf_word);
    glyphs.buf_word_sz = buf_word_sz;

    return is;
}

} }
