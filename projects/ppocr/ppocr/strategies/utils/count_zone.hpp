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
#include <memory>
#include <cassert>

namespace ppocr { namespace strategies { namespace utils
{
    struct MappingZoneView
    {
        MappingZoneView(unsigned* zones, unsigned len)
        : zones(zones)
        , len(len)
        {}

        unsigned& operator[](unsigned i)
        {
            assert(i < len);
            return zones[i];
        }

        unsigned operator[](unsigned i) const
        {
            assert(i < len);
            return zones[i];
        }

        unsigned* begin() const
        {
            return zones;
        }

        unsigned* end() const
        {
            return begin() + len;
        }

        unsigned count_used_zone() const
        {
            unsigned ret = 0;
            for (unsigned i : *this) {
                if (i) {
                    ++ret;
                }
            }
            return ret;
        }

    private:
        unsigned* zones;
        unsigned len;
    };

    struct ZoneInfo
    {
        ZoneInfo(unsigned count_zone)
        : _buffer(std::make_unique<unsigned[]>(count_zone * 4))
        , _count_zone(count_zone)
        {}

        unsigned count_zone() const
        {
            return _count_zone;
        }

        MappingZoneView top()
        {
            return view(0);
        }

        MappingZoneView right()
        {
            return view(1);
        }

        MappingZoneView bottom()
        {
            return view(2);
        }

        MappingZoneView left()
        {
            return view(3);
        }

        unsigned count_total_used_zone() const
        {
            unsigned used_zone = 0;
            for (unsigned i = 0; i < count_zone(); ++i) {
                if (data(0)[i]
                 || data(1)[i]
                 || data(2)[i]
                 || data(3)[i]
                ) {
                    ++used_zone;
                }
            }
            return used_zone;
        }

    private:
        std::unique_ptr<unsigned[]> _buffer;
        unsigned _count_zone;

        MappingZoneView view(unsigned d)
        {
            return MappingZoneView{data(d), _count_zone};
        }

        unsigned* data(unsigned d) const
        {
            return _buffer.get() + _count_zone * d;
        }
    };

    inline ZoneInfo count_zone(const Image& img) {
        unsigned count_zone = 1;
        std::vector<unsigned> mirror(img.area() * 2, 0);
        unsigned* const stack = mirror.data() + img.area();

        for (unsigned i = 0; i < img.area(); ++i) {
            if (mirror[i] || is_pix_letter(img.data()[i])) {
                continue;
            }

            unsigned idx = i;
            auto stack_it = stack;
            mirror[idx] = count_zone;
            for (;;) {
                auto x = idx % img.width();
                auto y = idx / img.width();

                auto push_if = [&](unsigned idx){
                    if (!mirror[idx] && !is_pix_letter(img.data()[idx])) {
                        mirror[idx] = count_zone;
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

            count_zone++;
        }

        ZoneInfo zone{count_zone - 1};

        auto insert = [&](MappingZoneView m, unsigned x, unsigned y) {
            auto i = img.to_size_t({x, y});
            if (mirror[i]) {
                ++m[mirror[i]-1];
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
