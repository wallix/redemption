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

#ifndef PPOCR_SRC_LOADER2_DATAS_LOADER_HPP
#define PPOCR_SRC_LOADER2_DATAS_LOADER_HPP

#include <array>
#include <vector>
#include <type_traits>

#include "ppocr/image/image.hpp"
#include "ppocr/strategies/utils/context.hpp"
#include <cassert>

namespace ppocr {

class Image;

namespace loader2 {

using std::size_t;

enum class PolicyLoader { img, img90 };

template<PolicyLoader Policy, class Ctx>
struct MakeRotatedCtx
{
    struct type : Ctx {};
};

template<PolicyLoader Policy>
struct MakeRotatedCtx<Policy, no_context>
{
    using type = no_context;
};

template<class Strategy_, PolicyLoader Policy>
struct Strategy
{
    using value_type = typename Strategy_::value_type;
    using relationship_type = typename Strategy_::relationship_type;
    using ctx_type = typename MakeRotatedCtx<Policy, typename Strategy_::ctx_type>::type;

    constexpr static PolicyLoader policy = Policy;

    static value_type load(Image const & img, Image const & img90, ctx_type& ctx)
    {
        if constexpr (policy == PolicyLoader::img) {
            return Strategy_::load(img, img90, ctx);
        }
        else {
            return Strategy_::load(img90, img, ctx);
        }
    }
};

namespace details_ {
    template<class T, class...>
    struct first_type
    { using type = T; };


    struct Lt { template <class T> bool operator()(T const & a, T const & b) const { return a < b; } };
    struct Eq { template <class T> bool operator()(T const & a, T const & b) const { return a == b; } };
    struct Noop { template <class T> bool operator()(T const & , T const & ) const { return false; } };

    template<class Cmp, class Cmp2, class Data>
    bool cmp_datas(size_t i1, size_t i2, Cmp cmp, Cmp2, Data const & data) {
        return cmp(data[i1], data[i2]);
    }

    template<class Cmp, class Cmp2, class Data, class... Datas>
    bool cmp_datas(size_t i1, size_t i2, Cmp cmp, Cmp2 cmp2, Data const & data, Datas const & ... others) {
        if (cmp(data[i1], data[i2]) || cmp2(data[i1], data[i2])) {
            return cmp_datas(i1, i2, cmp, cmp2, others...);
        }
        return false;
    }

    template<class Relationship, bool Is = Relationship::is_contiguous>
    constexpr std::integral_constant<bool, Is>
    is_contiguous(int) { return {}; }

    template<class Relationship>
    constexpr std::false_type
    is_contiguous(unsigned) { return {}; }
}

template<class Strategy>
typename Strategy::value_type
load(Strategy const & strategy, PolicyLoader policy, Image const & img, Image const & img90, typename Strategy::ctx_type& ctx)
{
    auto& real_ctx = static_cast<typename Strategy::ctx_type&>(ctx);
    return policy == PolicyLoader::img
        ? strategy.load(img, img90, real_ctx)
        : strategy.load(img90, img, real_ctx);
}

template<class Strategy>
struct Data
{
    using strategy_type = Strategy;
    using value_type = typename strategy_type::value_type;
    using relationship_type = typename strategy_type::relationship_type;
    using ctx_type = typename strategy_type::ctx_type;

    using container_type = std::vector<value_type>;
    using iterator = typename container_type::const_iterator;
    using const_iterator = iterator;

    using is_contiguous = decltype(details_::is_contiguous<relationship_type>(1));

    Data() = default;

    explicit Data(container_type && cont) noexcept
    : data_(std::move(cont))
    {}

    explicit Data(container_type const & cont)
    : data_(cont)
    {}

    container_type release() noexcept {
        return std::move(this->data_.values);
    }

    void load(Image const & img, Image const & img90, ctx_type& ctx) {
        this->data_.values.push_back(Strategy::load(img, img90, ctx));
    }

    value_type const & operator[](size_t i) const {
        return data_.values[i];
    }

    double dist_with(size_t i, value_type const & value) const {
        double const ret = relationship_type::dist(value, (*this)[i]);
        assert(0. <= ret && ret <= 1.);
        return ret;
    }

    bool in_dist_with(size_t i, value_type const & value, unsigned d) const
    {
        return relationship_type::in_dist((*this)[i], value, d);
    }

    unsigned count_posibilities() const {
        return relationship_type::count();
    }

    std::size_t size() const noexcept {
        return this->data_.values.size();
    }

    container_type const & data() const noexcept {
        return this->data_.values;
    }

    iterator begin() const { return this->data().begin(); }
    iterator end() const { return this->data().end(); }

private:
    struct impl
    {
        container_type values;

        template<class... Args>
        impl(Args && ... args)
        : values(std::forward<Args>(args)...)
        {}
    } data_;
};

template<class... Strategies>
struct Datas : private Data<Strategies>...
{
    Datas() = default;

    explicit Datas(Data<Strategies> && ... datas)
    : Data<Strategies>(std::move(datas))...
    {}

    template<class Strategy>
    Data<Strategy> const & get() const noexcept {
        return static_cast<Data<Strategy> const &>(*this);
    }

    template<class Strategy>
    Data<Strategy> & get() noexcept {
        return static_cast<Data<Strategy> &>(*this);
    }

    std::size_t size() const noexcept {
        return this->get<typename details_::first_type<Strategies...>::type>().size();
    }

    std::size_t release() const noexcept {
        return this->get<typename details_::first_type<Strategies...>::type>().size();
    }

    void load(Image const & img) {
        auto img90 = img.rotate90();
        ctx.reset();
        (..., static_cast<Data<Strategies>&>(*this).load(
            img, img90, static_cast<typename Strategies::ctx_type&>(ctx)
        ));
    }

    bool lt(size_t i1, size_t i2) const {
        return details_::cmp_datas(i1, i2, details_::Lt(), details_::Eq(), get<Strategies>()...);
    }

    bool eq(size_t i1, size_t i2) const {
        return details_::cmp_datas(i1, i2, details_::Eq(), details_::Noop(), get<Strategies>()...);
    }

private:
    unique_contexts_t<typename Strategies::ctx_type...> ctx;
};


template<class Strategy, class... Strategies>
Data<Strategy> const & get_data(Datas<Strategies...> const & datas) {
    return datas.template get<Strategy>();
}

template<class Fn, class... Strategies>
void apply_from_datas(Datas<Strategies...> const & datas, Fn fn) {
    (..., fn(datas.template get<Strategies>()));
}

template<class Fn, class... Strategies>
void apply_from_datas(Datas<Strategies...> & datas, Fn fn) {
    (..., fn(datas.template get<Strategies>()));
}

} }

#endif
