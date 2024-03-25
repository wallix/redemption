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
#include "ppocr/ocr2/data_indexes_ordered.hpp"
#include "ppocr/ocr2/sort_probabilities.hpp"
#include "ppocr/ocr2/insert_views.hpp"
#include "ppocr/ocr2/cache.hpp"

#include "ppocr/image/image.hpp"
#include "ppocr/loader2/datas_loader.hpp"
#include "ppocr/strategies/utils/context.hpp"

namespace ppocr { namespace ocr2 {


template<template<class...> class Tpl, class... Strategies, class Datas, class Ctx>
void reduce_complexe_universe(
    Tpl<Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    Image const & img,
    Image const & img90,
    Ctx& ctx
) {
    (..., reduce_universe_and_update_probability(
        probabilities,
        datas.template get<Strategies>(),
        Strategies::load(img, img90, ctx),
        0.5
    ));
}

template<class Strategy>
struct value_for_strategy
{
    typename Strategy::value_type value;
};

template<class... Strategy>
struct value_for_strategies : value_for_strategy<Strategy>...
{
    template<class Ctx>
    value_for_strategies(Image const & img, Image const & img90, Ctx& ctx)
      : value_for_strategy<Strategy>{Strategy::load(img, img90, ctx)}...
    {}
};

template<template<class...> class Tpl, class... Strategies, class Datas, class Ctx>
void reduce_exclusive_universe(
    Tpl<Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    Image const & img, Image const & img90, Ctx& ctx,
    DataIndexesByWords const & data_indexes_by_words
) {
    if (probabilities.empty()) {
        return ;
    }

    value_for_strategies<Strategies...> const store(img, img90, ctx);

    reduce_universe_by_word(probabilities, data_indexes_by_words, [&](unsigned i) {
        return (true && ... && datas.template get<Strategies>().in_dist_with(
            i,
            static_cast<value_for_strategy<Strategies> const&>(store).value,
            Strategies::relationship_type::count() / 2
        ));
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
    auto d = (Strategy::relationship_type::count()-1) / 10u;
    for (auto idx : first_strategy_ordered.get_range(datas, value, d)) {
        probabilities.emplace_back(idx);
    }
}

template<class Data>
void initialize_probability(Probabilities & probabilities, unsigned value, Data const & data) {
    for (auto & prob : probabilities) {
        prob.prob = Data::relationship_type::dist(data[prob.i], value);
    }
}

template<class Data>
void update_probability(Probabilities & probabilities, unsigned value, Data const & data) {
    for (auto & prob : probabilities) {
        prob.prob *= Data::relationship_type::dist(data[prob.i], value);
    }
}

template<
    template<class...> class Tpl,
    class FirstStrategyOrdered,
    class FirstStrategy,
    class... Strategies,
    class Datas,
    class Ctx
>
void compute_simple_universe(
    Tpl<FirstStrategy, Strategies...>,
    Probabilities & probabilities,
    Datas const & datas,
    DataIndexesOrdered<FirstStrategyOrdered> const & first_strategy_ordered,
    Image const & img,
    Image const & img90,
    Ctx& ctx
) {
    static_assert(std::is_same<FirstStrategyOrdered, FirstStrategy>::value, "is different");

    auto first_value = FirstStrategy::load(img, img90, ctx);

    initialize_universe<FirstStrategy>(
        probabilities,
        datas,
        first_strategy_ordered,
        first_value
    );

    value_for_strategies<Strategies...> store(img, img90, ctx);

    (..., reduce_universe_with_distance(
        probabilities,
        datas.template get<Strategies>(),
        static_cast<value_for_strategy<Strategies>&>(store).value,
        (Strategies::relationship_type::count()-1) / 10u
    ));

    initialize_probability(
        probabilities,
        first_value,
        datas.template get<FirstStrategy>()
    );

    (..., update_probability(
        probabilities,
        static_cast<value_for_strategy<Strategies>&>(store).value,
        datas.template get<Strategies>()
    ));
}


template<
    class SimpleAlgos,
    class ComplexAlgos,
    class ExclusifAlgos,
    class FirstStrategyOrdered,
    class... DatasStrategies,
    class Ctx
>
view_ref_list compute_image(
    SimpleAlgos,
    ComplexAlgos,
    ExclusifAlgos,
    Probabilities & probabilities,
    Probabilities & tmp_probabilities,
    loader2::Datas<DatasStrategies...> const & datas,
    DataIndexesOrdered<FirstStrategyOrdered> const & first_strategy_ordered,
    DataIndexesByWords const & data_indexes_by_words,
    Glyphs const & glyphs,
    std::vector<unsigned> const & id_views,
    Image const & img,
    Image const & img90,
    Ctx& ctx,
    double limit_prob_for_insert = 0.5
) {
    ctx.reset();

    compute_simple_universe(SimpleAlgos{}, probabilities, datas, first_strategy_ordered, img, img90, ctx);

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
            reduce_complexe_universe(ComplexAlgos(), probabilities, datas, img, img90, ctx);
            reduce_exclusive_universe(ExclusifAlgos(), probabilities, datas, img, img90, ctx, data_indexes_by_words);

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

} }

#endif
