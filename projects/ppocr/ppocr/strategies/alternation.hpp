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

#ifndef PPOCR_STRATEGIES_ALTERNATION_HPP
#define PPOCR_STRATEGIES_ALTERNATION_HPP

#include <array>
#include <iosfwd>

#include "ppocr/strategies/relationship/array_compare.hpp"

namespace ppocr {

class Image;

namespace strategies
{
    /**
     *                             ::::::::::::
     *                             :xx        :
     *                             :xx        :
     *                             :xx        :
     *                             :xx        :
     *    Hl1, Hl2,                :xx        :
     *    Hm1, Hm2,                :xxxxxxx   :
     *    Vl1,                     :xxxxxxxx  :
     *    Vm1, Vm2                 :xx     xxx:
     *                             :xx      xx:
     *                             :xx     xxx:
     *                             :xxxxxxxxx :
     *                             :xxxxxxx   :
     *                             ::::::::::::
     *
     *   ::::::::::::
     *   :xx        :                                           ----------
     *   :xx        :                                          :xx        :
     *   :xx        :                                          :xx        : Hm1 = 1, 0
     *    ----------                                           :xx        : h = (img.h/3)
     *   |xx        | Hl1 = 1, 0 ; y = ((img.h-2)/3)           :xx        :
     *    ----------                                            ----------
     *   :xx        :                                          :xx        :
     *   :xxxxxxx   :                                          :xxxxxxx   :
     *   :xxxxxxxx  :                                          :xxxxxxxx  :
     *    ----------                                           :xx     xxx:
     *   |xx     xxx| Hl2 = 1, 0, 1 ; y = ((img.h*2-1)/3)       ----------
     *    ----------                                           :xx      xx:
     *   :xx      xx:                                          :xx     xxx: Hm2 = 1
     *   :xx     xxx:                                          :xxxxxxxxx : h = (img.h/3)
     *   :xxxxxxxxx :                                          :xxxxxxx   :
     *   :xxxxxxx   :                                           ----------
     *   ::::::::::::
     *
     *
     *  Vl1 = 0, 1, 0, 1 ; x = (img.w-1)/2      Vm1 = 1 ; w = img.w/3 ; Vm2 = 0, 1, 0
     *           ::::::_:::::::                        :___::::::___:
     *           :xx  | |     :                        |xx |    |   |
     *           :xx  | |     :                        |xx |    |   |
     *           :xx  | |     :                        |xx |    |   |
     *           :xx  | |     :                        |xx |    |   |
     *           :xx  | |     :                        |xx |    |   |
     *           :xxxx|x|xx   :                        |xxx|xxxx|   |
     *           :xxxx|x|xxx  :                        |xxx|xxxx|x  |
     *           :xx  | |  xxx:                        |xx |    |xxx|
     *           :xx  | |   xx:                        |xx |    | xx|
     *           :xx  | |  xxx:                        |xx |    |xxx|
     *           :xxxx|x|xxxx :                        |xxx|xxxx|xx |
     *           :xxxx|x|xx   :                        |xxx|xxxx|   |
     *           ::::::-:::::::                        :---::::::---:
     */
struct alternations
{
    struct alternations_type {
        bool start_contains_letter;
        unsigned count;

        bool operator==(alternations_type const & other) const noexcept
        { return start_contains_letter == other.start_contains_letter && count == other.count; }

        bool operator<(alternations_type const & other) const noexcept
        {
            return count < other.count
                || (count == other.count && start_contains_letter < other.start_contains_letter);
        }
    };

    using relationship_type = array_compare_relationship<alternations_type, 7>;
    using value_type = relationship_type::value_type;

    static constexpr bool one_axis = false;

    value_type load(Image const & img, Image const & /*img90*/) const;

    constexpr relationship_type relationship() const { return {}; }
    unsigned best_difference() const;
};

std::ostream & operator<<(std::ostream &, alternations::alternations_type const &);
std::istream & operator>>(std::istream &, alternations::alternations_type &);

}

}

#endif
