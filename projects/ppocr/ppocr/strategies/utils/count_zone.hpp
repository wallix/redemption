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

#ifndef PPOCR_SRC_STRATEGIES_UTILS_COUNT_ZONE_HPP
#define PPOCR_SRC_STRATEGIES_UTILS_COUNT_ZONE_HPP

#include "ppocr/image/image.hpp"

#include <vector>
#include <cassert>

namespace ppocr { namespace strategies { namespace utils
{
    struct unsigned_array_view
    {
        unsigned* data;
        unsigned len;

        unsigned* begin() const { return data; }
        unsigned* end() const { return data + len; }

        unsigned count_non_zero() const
        {
            unsigned n = 0;
            for (unsigned x : *this) {
                n += x ? 1 : 0;
            }
            return n;
        }

        unsigned& operator[](unsigned i)
        {
            assert(i < len);
            return data[i];
        }
    };

    struct ZoneInfo {
        std::unique_ptr<unsigned[]> stack;
        unsigned len = 0;
        unsigned count_zone = 1;

        unsigned_array_view top() { return {stack.get(), len}; }
        unsigned_array_view right() { return {top().end(), len}; }
        unsigned_array_view bottom() { return {right().end(), len}; }
        unsigned_array_view left() { return {bottom().end(), len}; }

        void alloc(unsigned n)
        {
            len = n;
            stack = std::make_unique<unsigned[]>(n);
        }
    };

    inline ZoneInfo count_zone(const Image& img) {
        ZoneInfo zone;
        std::vector<unsigned> mirror(img.area() * 2, 0);
        unsigned* const stack = mirror.data() + img.area();

        for (unsigned i = 0; i < img.area(); ++i) {
            if (mirror[i] || is_pix_letter(img.data()[i])) {
                continue;
            }

            unsigned idx = i;
            auto stack_it = stack;
            mirror[idx] = zone.count_zone;
            for (;;) {
                auto x = idx % img.width();
                auto y = idx / img.width();

                auto push_if = [&](unsigned idx){
                    if (!mirror[idx] && !is_pix_letter(img.data()[idx])) {
                        mirror[idx] = zone.count_zone;
                        *stack_it++ = idx;
                    }
                };

                if (y != 0) {
                    push_if(idx - img.width());
                }
                if (y + 1 != img.height()) {
                    push_if(idx + img.width());
                }
                if (x != 0) {
                    push_if(idx - 1);
                }
                if (x + 1 != img.width()) {
                    push_if(idx + 1);
                }

                if (stack_it == stack) {
                    break;
                }

                idx = *--stack_it;
            }

            zone.count_zone++;
        }

        zone.alloc(zone.count_zone - 1);

        auto insert = [&](unsigned_array_view av, unsigned x, unsigned y) {
            auto i = img.to_size_t({x, y});
            if (mirror[i]) {
                ++av[mirror[i] - 1];
            }
        };

        for (unsigned x = 0; x < img.width(); ++x) {
            insert(zone.top(), x, 0);
            insert(zone.bottom(), x, img.height()-1);
        }

        for (unsigned y = 0; y < img.height(); ++y) {
            insert(zone.left(), 0, y);
            insert(zone.right(), img.width()-1, y);
        }

        return zone;
    }

} } }

#endif
