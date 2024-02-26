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

#ifndef PPOCR_SRC_OCR2_COMPUTE_IMAGE_HPP
#define PPOCR_SRC_OCR2_COMPUTE_IMAGE_HPP

#include "ppocr/ocr2/reduce_universe.hpp"
#include "ppocr/ocr2/data_strategy_loader.hpp"
#include "ppocr/ocr2/data_indexes_ordered.hpp"
#include "ppocr/ocr2/sort_probabilities.hpp"
#include "ppocr/ocr2/insert_views.hpp"
#include "ppocr/ocr2/cache.hpp"

#include "ppocr/image/image.hpp"
#include "ppocr/loader2/datas_loader.hpp"

namespace ppocr { namespace ocr2 {

#ifndef IN_IDE_PARSER
# define UNPACK(...) void(std::initializer_list<char>{(void(__VA_ARGS__), char())...})
#else
# define UNPACK(a) void(std::initializer_list<char>{(void(a), char())...})
#endif

template<template<class...> class Temp, class... Strategies, class Datas>
void reduce_complexe_universe(
    Temp<Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    Image const & img,
    Image const & img90
) {
    UNPACK(reduce_universe_and_update_probability(
        probabilities,
        datas.template get<Strategies>(),
        data_strategy_loader<Strategies>().load(img, img90),
        0.5
    ));
}

template<class Strategy>
struct data_exclusive_universe : data_strategy_loader<Strategy> {
    size_t limit;
};

template<template<class...> class Temp, class Store, class Datas>
bool has_value(Temp<>, Datas const &, unsigned, Store const &) {
    return true;
}

template<template<class...> class Temp, class Store, class Strategy, class... Strategies, class Datas>
bool has_value(
    Temp<Strategy, Strategies...>,
    Datas const & datas,
    unsigned i, Store const & store
) {
    if (datas.template get<Strategy>().get_relationship().in_dist(
        datas.template get<Strategy>()[i],
        static_cast<data_strategy_loader<Strategy> const&>(store).x,
        static_cast<data_exclusive_universe<Strategy> const&>(store).limit
    )) {
        return has_value(Temp<Strategies...>(), datas, i, store);
    }
    return false;
}

template<template<class...> class Temp, class... Strategies, class Datas>
void reduce_exclusive_universe(
    Temp<Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    Image const & img, Image const & img90,
    DataIndexesByWords const & data_indexes_by_words
) {
    if (probabilities.empty()) {
        return ;
    }

    struct
    :  data_exclusive_universe<Strategies>...
    {} store;
    UNPACK((
        static_cast<data_strategy_loader<Strategies>&>(store).load(img, img90),
        (static_cast<data_exclusive_universe<Strategies>&>(store).limit
         = datas.template get<Strategies>().count_posibilities()/2)
    ));

    reduce_universe_by_word(probabilities, data_indexes_by_words, [&](unsigned i) {
        return has_value(Temp<Strategies...>(), datas, i, store);
    });
}

// --------------
template<class Strategy, class FirstStrategyOrdered, class Datas>
void initialize_universe(
    Probabilities & probabilities,
    Datas const & datas,
    DataIndexesOrdered<FirstStrategyOrdered> const & first_strategy_ordered,
    unsigned value
) {
    probabilities.clear();
    auto & data = datas.template get<Strategy>();
    auto d = (data.count_posibilities()-1)/10u;
    for (auto idx : first_strategy_ordered.get_range(datas, value, d)) {
        probabilities.emplace_back(idx);
    }
}

template<class Data>
void initialize_probability(Probabilities & probabilities, unsigned value, Data const & data) {
    for (auto & prob : probabilities) {
        prob.prob = data.dist(data[prob.i], value);
    }
}

template<class Data>
void update_probability(Probabilities & probabilities, unsigned value, Data const & data) {
    for (auto & prob : probabilities) {
        prob.prob *= data.dist(data[prob.i], value);
    }
}

template<
    template<class...> class Temp,
    class FirstStrategyOrdered,
    class FirstStrategy,
    class... Strategies,
    class Datas
>
void compute_simple_universe(
    Temp<FirstStrategy, Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    DataIndexesOrdered<FirstStrategyOrdered> const & first_strategy_ordered,
    Image const & img,
    Image const & img90
) {
    struct
    : data_strategy_loader<FirstStrategy>
    , data_strategy_loader<Strategies>...
    {} store;

    static_assert(std::is_same<FirstStrategyOrdered, FirstStrategy>::value, "is different");

    initialize_universe<FirstStrategy>(
        probabilities,
        datas,
        first_strategy_ordered,
        static_cast<data_strategy_loader<FirstStrategy>&>(store).load(img, img90)
    );

    UNPACK(reduce_universe_with_distance(
        probabilities,
        datas.template get<Strategies>(),
        static_cast<data_strategy_loader<Strategies>&>(store).load(img, img90),
        (datas.template get<Strategies>().count_posibilities()-1)/10u
    ));

    initialize_probability(
        probabilities,
        static_cast<data_strategy_loader<FirstStrategy>&>(store).x,
        datas.template get<FirstStrategy>()
    );

    UNPACK(update_probability(
        probabilities,
        static_cast<data_strategy_loader<Strategies>&>(store).x,
        datas.template get<Strategies>()
    ));
}

template<
    class SimpleAlgos,
    class ComplexAlgos,
    class ExclusifAlgos,
    class FirstStrategyOrdered,
    class... DatasStrategies
>
view_ref_list compute_image(
    SimpleAlgos, ComplexAlgos, ExclusifAlgos,
    Probabilities & probabilities,
    Probabilities & tmp_probabilities,
    loader2::Datas<DatasStrategies...> const & datas,
    DataIndexesOrdered<FirstStrategyOrdered> const & first_strategy_ordered,
    DataIndexesByWords const & data_indexes_by_words,
    Glyphs const & glyphs,
    std::vector<unsigned> const & id_views,
    Image const & img,
    Image const & img90,
    double limit_prob_for_insert = 0.5
) {
    compute_simple_universe(SimpleAlgos{}, probabilities, datas, first_strategy_ordered, img, img90);

    view_ref_list cache_element;

    ocr2::sort_by_views(probabilities, id_views);

    if (ComplexAlgos::size || ExclusifAlgos::size) {
        ocr2::unique_copy_by_views(tmp_probabilities, probabilities, id_views);
        ocr2::sort_by_prop(tmp_probabilities);

        if (tmp_probabilities.empty()) {
            return cache_element;
        }
        else if (tmp_probabilities[0].prob >= 1./* && tmp_probabilities.size() == 1*/) {
            ocr2::insert_views(cache_element, tmp_probabilities, glyphs, 1.);
            return cache_element;
        }
        else {
            reduce_complexe_universe(ComplexAlgos(), probabilities, datas, img, img90);
            reduce_exclusive_universe(ExclusifAlgos(), probabilities, datas, img, img90, data_indexes_by_words);

            ocr2::sort_by_views(probabilities, id_views);
        }
    }

    ocr2::unique_by_views(probabilities, id_views);
    ocr2::sort_by_prop(probabilities);
    if (!probabilities.empty()) {
        ocr2::insert_views(cache_element, probabilities, glyphs, probabilities.front().prob * limit_prob_for_insert);
    }

    return cache_element;
}

#undef UNPACK

} }

#endif
