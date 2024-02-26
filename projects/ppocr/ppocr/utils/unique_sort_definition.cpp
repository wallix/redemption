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

#include "ppocr/unique_sort_definition.hpp"
#include "ppocr/utils/image_compare.hpp"

#include <algorithm>
#include <tuple>

namespace ppocr {

void unique_sort_definitions(std::vector< Definition >& defs)
{
    if (defs.empty()) {
        return;
    }

    std::sort(defs.begin(), defs.end(), [](Definition const & a, Definition const & b) {
        {
            int const cmp = a.c.compare(b.c);
            if (cmp < 0) {
                return true;
            }
            if (cmp > 0) {
                return false;
            }
        }

        {
            int const cmp = a.font_name.compare(b.font_name);
            if (cmp < 0) {
                return true;
            }
            if (cmp > 0) {
                return false;
            }
        }

        return image_compare(a.img, b.img) < 0;
    });

    defs.erase(std::unique(defs.begin(), defs.end(), [](Definition const & a, Definition const & b) {
        return a.c == b.c && a.font_name == b.font_name && a.img == b.img;
    }), defs.end());
}

}
