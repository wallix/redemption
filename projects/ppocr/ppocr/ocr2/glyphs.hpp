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

#ifndef PPOCR_CAPTURE_RDP_PPOCR_GLYPHS_HPP
#define PPOCR_CAPTURE_RDP_PPOCR_GLYPHS_HPP

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <istream>
#include <algorithm>

namespace ppocr { namespace ocr2 {

struct View {
    unsigned word;
    unsigned font;
};
struct Views : std::vector<View> {
    using std::vector<View>::vector;
    Views() = default;
    Views(Views &&) = default;
    Views(Views const &) = delete;
    Views & operator=(Views &&) = default;
    Views & operator=(Views const &) = delete;
};
struct Glyphs : std::vector<Views> {
    using std::vector<Views>::vector;
    Glyphs() = default;
    Glyphs(Glyphs &&) = default;
    Glyphs(Glyphs const &) = delete;
    Glyphs & operator=(Glyphs &&) = default;
    Glyphs & operator=(Glyphs const &) = delete;

    struct string {
        unsigned sz;
        char word[1];

        char const * data() const { return this->word; }
        char const * begin() const { return this->word; }
        char const * end() const { return this->word + this->sz; }
        unsigned size() const { return this->sz; }
        char front() const { return this->word[0]; }
    };

    string const & get_word(View const & v) const
    { return this->get_word_(v.word); }

    constexpr static unsigned const no_index = ~unsigned{};

    unsigned word_index_of(std::string_view s) const {
        unsigned i = 0;
        while (i < this->buf_word_sz) {
            auto r = s.compare(0, s.size(), this->get_word_(i).data(), s.size());
            if (r <= 0) {
                if (r == 0) {
                    return i;
                }
                break;
            }
            i += (this->get_word_(i).size() + sizeof(unsigned) * 2 - 1) / sizeof(unsigned);
        }
        return no_index;
    }

private:
    std::unique_ptr<unsigned[]> buf_word;
    unsigned buf_word_sz;

    string const & get_word_(unsigned i) const
    { return *reinterpret_cast<string const *>(this->buf_word.get() + i); }

    friend std::istream & operator>>(std::istream & is, Glyphs & glyphs);
};

std::ostream & operator<<(std::ostream & os, Glyphs::string const & str);

std::istream & operator>>(std::istream & is, Glyphs & glyphs);


struct EqViewWord {
    EqViewWord() {}
    bool operator()(View const & v1, View const & v2) {
        return v1.word == v2.word;
    }
};

struct EqViews {
    bool operator()(Views const & v1, Views const & v2) const {
        return v1.size() == v2.size() && std::equal(v1.begin(), v1.end(), v2.begin(), EqViewWord{});
    }
};

struct LtViews {
    bool operator()(Views const & v1, Views const & v2) const {
        if (v1.size() < v2.size()) {
            return true;
        }
        if (v1.size() > v2.size()) {
            return false;
        }
        auto const pair = std::mismatch(v1.begin(), v1.end(), v2.begin(), EqViewWord{});
        if (pair.first == v1.end()) {
            return false;
        }
        return pair.first->word < pair.second->word;
    }
};

} }

#endif
