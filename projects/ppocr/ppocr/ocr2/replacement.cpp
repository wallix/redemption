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

#include "ppocr/ocr2/replacement.hpp"

#include <istream>

std::istream& ppocr::ocr2::operator>>(std::istream& is, Replacements& replacements) {
    using char_traits = std::char_traits<char>;
    Replacement rep;
    while (is) {
        is >> rep.pattern;
        auto c = is.rdbuf()->sgetc();
        if (!char_traits::eq_int_type(c, char_traits::eof())) {
            if (!char_traits::eq(char_traits::to_char_type(c), '\n')) {
                is >> rep.replace;
            }
            else {
                rep.replace.clear();
            }
        }
        if (is) {
            replacements.push_back(rep);
        }
    }
    return is;
}
