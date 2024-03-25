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

#ifndef PPOCR_SRC_STRATEGIES_HBAR_HPP
#define PPOCR_SRC_STRATEGIES_HBAR_HPP

#include "ppocr/strategies/relationship/equal.hpp"
#include "ppocr/strategies/utils/context.hpp"

namespace ppocr {

class Image;

namespace strategies {

struct hbar
{
    using value_type = unsigned;
    using relationship_type = equal_relationship<value_type>;

    using ctx_type = no_context;

    static value_type load(Image const & img, Image const & img90, ctx_type& ctx);
};

}

}

#endif
