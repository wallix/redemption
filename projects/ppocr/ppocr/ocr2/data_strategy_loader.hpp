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

#ifndef PPOCR_SRC_OCR2_DATA_STRATEGY_LOADER_HPP
#define PPOCR_SRC_OCR2_DATA_STRATEGY_LOADER_HPP

#include "ppocr/image/image.hpp"


namespace ppocr { namespace ocr2 {

template<class LoaderStrategy>
struct data_strategy_loader {
    using strategy_type = typename LoaderStrategy::strategy_type;
    using value_type = typename strategy_type::value_type;

    value_type x;

    value_type load(ppocr::Image const & img, ppocr::Image const & img90) {
        x = ppocr::loader2::load(strategy_type(), LoaderStrategy::policy, img, img90);
        return x;
    }
};

} }

#endif
