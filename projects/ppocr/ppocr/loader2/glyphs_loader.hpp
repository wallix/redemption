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

#ifndef PPOCR_SRC_LOADER2_GLYPHS_LOADER_HPP
#define PPOCR_SRC_LOADER2_GLYPHS_LOADER_HPP

#include <string>
#include <vector>
#include <iosfwd>

#include "ppocr/image/image.hpp"

namespace ppocr { namespace loader2 {

struct View {
    std::string word;
    std::string font;
    unsigned info_line; /*ignored*/
};

inline bool operator == (View const & a, View const & b) {
    return a.word == b.word && a.font == b.font && a.info_line == b.info_line;
}

inline bool operator < (View const & a, View const & b) {
    if (int const cmp1 = a.word.compare(b.word)) {
        return cmp1 < 0;
    }
    if (int const cmp1 = a.font.compare(b.font)) {
        return cmp1 < 0;
    }
    return a.info_line < b.info_line;
}

// strong type
struct Views : std::vector<View>
{ using std::vector<View>::vector; };

struct Glyph {
    Image img;
    Views views;
};

inline bool operator == (Glyph const & a, Glyph const & b) {
    return a.img == b.img && a.views == b.views;
}
bool operator < (Glyph const & a, Glyph const & b);

// strong type
struct Glyphs : std::vector<Glyph>
{ using std::vector<Glyph>::vector; };

std::istream & operator>>(std::istream& is, Glyph & glyph);
std::ostream & operator<<(std::ostream& os, Glyph const & glyph);
std::istream & operator>>(std::istream& is, Glyphs & glyphs);
std::ostream & operator<<(std::ostream& os, Glyphs const & glyphs);

std::istream & operator>>(std::istream& is, View & view);
std::ostream & operator<<(std::ostream& os, View const & view);
std::istream & operator>>(std::istream& is, Views & views);
std::ostream & operator<<(std::ostream& os, Views const & views);

} }

#endif
