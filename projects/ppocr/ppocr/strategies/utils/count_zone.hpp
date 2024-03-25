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

#include <memory>
#include <cassert>
#include <cstring>

namespace ppocr { namespace strategies { namespace utils
{
    struct count_zone_fn;

    template<class T>
    struct Buffer
    {
        T* alloc_and_init_to_zero(std::size_t n)
        {
            if (n > _capacity) {
                _buffer.reset(new T[n * 2]);
                _capacity = n * 2;
            }
            std::memset(data(), 0, n * sizeof(T));
            return data();
        }

        T* data() const
        {
            return _buffer.get();
        }

    private:
        std::unique_ptr<T[]> _buffer;
        std::size_t _capacity = 0;
    };

    struct MappingZoneView
    {
        MappingZoneView(unsigned* zones, unsigned len)
        : zones(zones)
        , len(len)
        {}

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

        friend class count_zone_fn;

        unsigned& writable_get(unsigned i)
        {
            assert(i < len);
            return zones[i];
        }
    };

    struct ZoneInfo
    {
        ZoneInfo() = default;

        void alloc(unsigned count_zone)
        {
            _buffer.alloc_and_init_to_zero(count_zone * 4);
            _count_zone = count_zone;
        }

        unsigned count_zone() const
        {
            return _count_zone;
        }

        MappingZoneView top() const
        {
            return view(0);
        }

        MappingZoneView right() const
        {
            return view(1);
        }

        MappingZoneView bottom() const
        {
            return view(2);
        }

        MappingZoneView left() const
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
        Buffer<unsigned> _buffer;
        unsigned _count_zone = 0;

        MappingZoneView view(unsigned d) const
        {
            return MappingZoneView{data(d), _count_zone};
        }

        unsigned* data(unsigned d) const
        {
            return _buffer.data() + _count_zone * d;
        }
    };

    struct count_zone_fn
    {
        ZoneInfo const& value() const
        {
            return zone;
        }

        void compute(const Image& img);

    private:
        ZoneInfo zone;
        Buffer<unsigned> _buffer;
    };

    inline void count_zone_fn::compute(const Image& img)
    {
        unsigned* mirror = _buffer.alloc_and_init_to_zero(img.area() * 2 + 2);
        unsigned* const stack = mirror + img.area();

        unsigned count_zone = 1;

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

        zone.alloc(count_zone - 1);

        auto insert = [&](MappingZoneView m, unsigned x, unsigned y) {
            auto i = img.to_size_t({x, y});
            if (mirror[i]) {
                ++m.writable_get(mirror[i]-1);
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
    }

} } }

#endif
