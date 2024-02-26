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

#ifndef PPOCR_CAPTURE_RDP_PPOCR_REPLACEMENT_HPP
#define PPOCR_CAPTURE_RDP_PPOCR_REPLACEMENT_HPP

#include <iosfwd>
#include <string>
#include <vector>

namespace ppocr { namespace ocr2 {

struct Replacement {
    std::string pattern;
    std::string replace;
};

struct Replacements : std::vector<Replacement>
{ using std::vector<Replacement>::vector; };

std::istream & operator >> (std::istream & is, Replacements & replacements);


inline void replace_words(std::string & result, Replacements const & replacements) {
    for (Replacement const & rep : replacements) {
        std::string::size_type pos = 0;
        while ((pos = result.find(rep.pattern, pos)) != std::string::npos) {
            result.replace(pos, rep.pattern.size(), rep.replace);
        }
    }
}

} }

#endif
