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

#ifndef PPOCR_SRC_DEFINED_LOADER_HPP
#define PPOCR_SRC_DEFINED_LOADER_HPP

#include "ppocr/loader2/datas_loader.hpp"

#include "ppocr/strategies/alternation.hpp"
// #include "ppocr/strategies/direction.hpp"
#include "ppocr/strategies/hdirection.hpp"
#include "ppocr/strategies/hdirection2.hpp"
// #include "ppocr/strategies/agravity.hpp"
// #include "ppocr/strategies/gravity.hpp"
// #include "ppocr/strategies/compass.hpp"
// #include "ppocr/strategies/dcompass.hpp"
#include "ppocr/strategies/proportionality.hpp"
// #include "ppocr/strategies/gravity2.hpp"
#include "ppocr/strategies/hgravity.hpp"
#include "ppocr/strategies/hgravity2.hpp"

#include "ppocr/strategies/dvdirection.hpp"
#include "ppocr/strategies/dvdirection2.hpp"
#include "ppocr/strategies/dvgravity.hpp"
#include "ppocr/strategies/dvgravity2.hpp"

#include "ppocr/strategies/dzdensity.hpp"

#include "ppocr/strategies/density.hpp"

#include "ppocr/strategies/zone.hpp"
#include "ppocr/strategies/proportionality_zone.hpp"

#include "ppocr/strategies/hbar.hpp"

#include <ostream>
#include <istream>

namespace ppocr {

template<class... Strategies>
struct mp_list
{
    static constexpr std::size_t size = sizeof...(Strategies);
};

namespace details_
{
    template<class L1, class L2 = mp_list<>, class L3 = mp_list<>>
    struct ppocr_to_datas;

    template<class... Strategies>
    struct DefaultDatas : loader2::Datas<Strategies...>
    {
        using loader2::Datas<Strategies...>::Datas;
    };

    template<class... T1, class... T2, class... T3>
    struct ppocr_to_datas<mp_list<T1...>, mp_list<T2...>, mp_list<T3...>>
    {
        using type = DefaultDatas<T1..., T2..., T3...>;
    };
}

#define REGISTRY(name) \
    loader2::Strategy<strategies::name, loader2::PolicyLoader::img>
#define REGISTRY2(name) \
    loader2::Strategy<strategies::name, loader2::PolicyLoader::img>, \
    loader2::Strategy<strategies::name, loader2::PolicyLoader::img90>

using PpOcrSimpleDatas = mp_list<
    loader2::Strategy<strategies::dvgravity2,   loader2::PolicyLoader::img90>,
    loader2::Strategy<strategies::dvdirection2, loader2::PolicyLoader::img90>,
    REGISTRY2(dzdensity),
    loader2::Strategy<strategies::dvdirection2, loader2::PolicyLoader::img>,
    loader2::Strategy<strategies::dvgravity2,   loader2::PolicyLoader::img>,

    REGISTRY2(hdirection),
    REGISTRY2(hdirection2),

    //REGISTRY(direction),

    //REGISTRY(agravity),
    //REGISTRY(gravity),

    REGISTRY2(hgravity),
    REGISTRY2(hgravity2),

    //REGISTRY(compass),
    //REGISTRY(dcompass),

    REGISTRY (proportionality),
    //REGISTRY(gravity2),

    REGISTRY2(dvdirection),

    REGISTRY2(dvgravity),

    REGISTRY (density)
>;

using PpOcrComplexDatas = mp_list<
    REGISTRY2(hbar),

    REGISTRY (alternations)
>;

using PpOcrExclusiveDatas = mp_list<
    REGISTRY (zone),
    REGISTRY (proportionality_zone)
>;

#undef REGISTRY2
#undef REGISTRY

using PpOcrDatas = details_::ppocr_to_datas<
    PpOcrSimpleDatas,
    PpOcrComplexDatas,
    PpOcrExclusiveDatas
>::type;


// TODO other file

namespace details_ {
    struct WriteApplyData {
        std::ostream & os_;

        template<class Data>
        void operator()(Data const & data) const {
            if (!this->os_) {
                return;
            }

            for (auto & x : data.data()) {
                this->write(x);
            }
            this->os_ << '\n';
        }

    private:
        template<class T>
        void write(T const & x) const {
            this->os_ << x << ' ';
        }

        template<class T>
        void write(std::vector<T> const & cont) const {
            this->os_ << cont.size() << ' ';
            for (auto & x : cont) {
                this->os_ << x << ' ';
            }
        }

        template<class T, std::size_t N>
        void write(std::array<T, N> const & arr) const {
            for (auto & x : arr) {
                this->os_ << x << ' ';
            }
        }
    };

    struct ReadApplyData {
        std::istream & is_;
        std::size_t data_sz_;

        void operator()() const {
            if (this->is_) {
                std::istream::sentry(this->is_);
            }
        }

        template<class Strategy, class... Strategies>
        void operator()(loader2::Data<Strategy> & data, loader2::Data<Strategies> & ... other) const {
            typename loader2::Data<Strategy>::container_type cont;

            if (this->is_) {
                cont.resize(this->data_sz_);
                for (auto & x : cont) {
                    this->read(x);
                }
            }

            data = loader2::Data<Strategy>(std::move(cont));

            (*this)(other...);
        }

    private:
        template<class T>
        void read(T & x) const {
            this->is_ >> x;
        }

        template<class T>
        void read(std::vector<T> & cont) const {
            std::size_t sz;
            if (this->is_ >> sz) {
                cont.resize(sz);
                for (auto & x : cont) {
                    this->is_ >> x;
                }
            }
        }

        template<class T, std::size_t N>
        void read(std::array<T, N> & arr) const {
            for (auto & x : arr) {
                this->is_ >> x;
            }
        }
    };
}

namespace details_ {
    template<class... Strategies>
    std::ostream & operator<<(std::ostream & os, DefaultDatas<Strategies...> const & datas) {
        os << datas.size() << '\n';
        loader2::apply_from_datas(datas, details_::WriteApplyData{os});
        return os;
    }

    template<class... Strategies>
    std::istream & operator>>(std::istream & is, DefaultDatas<Strategies...> & datas) {
        std::size_t sz;
        is >> sz;
        using loader2::Data;
        struct Tuple : Data<Strategies>... {} t;
        details_::ReadApplyData read{is, sz};
        read(static_cast<Data<Strategies>&>(t)...);
        datas = PpOcrDatas(static_cast<Data<Strategies>&&>(t)...);
        return is;
    }
}

}

#endif
