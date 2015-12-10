/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_GDI_DRAWABLE_HPP
#define REDEMPTION_GDI_DRAWABLE_HPP

#include <memory>
#include <iostream>

#include "meta/meta.hpp"
#include "utils/non_null.hpp"
#include "graphic_device.hpp"
#include "proxy_gd.hpp"

#include "exchange.hpp"


namespace gdi {

class Drawable : private noncopyable_but_movable
{
    struct InterfaceBase;

public:
    struct NullFilter
    {
        template<class... Ts>
        void operator()(GraphicDevice & gd, Ts const & ... args) {
            return gd.draw(args...);
        }
    };

    using InterfacePtr = std::unique_ptr<InterfaceBase>;

    using TypeId = void const *;


    Drawable() = default;

    TypeId add_gdi(GraphicDevicePtr && pgdi) {
        if (!pgdi) {
            return {};
        }
        return this->actions().add(std::move(pgdi));
    }

    template<class FilterClass>
    TypeId add_filter(FilterClass && filter) {
        this->interface_.reset(this->new_filter(std::forward<FilterClass>(filter)));
        return this->interface_.get();
    }

    TypeId add_filter(InterfacePtr && pfilter) {
        if (!pfilter) {
            return {};
        }
        pfilter->next = std::move(this->interface_);
        this->interface_ = std::move(pfilter);
        return this->interface_.get();
    }

    GraphicDevicePtr remove_gdi(TypeId id) {
        return this->actions().remove(id);
    }

    InterfacePtr remove_filter(TypeId id);


    void draw(RDPOpaqueRect const & cmd, Rect const & clip) {
        this->interface_->draw(cmd, clip);
    }


private:
    struct InterfaceBase : GraphicDevice
    {
        InterfaceBase() = default;

        InterfaceBase(InterfacePtr && next)
        : next(std::move(next))
        {}

    private:
        friend class Drawable;
        InterfacePtr next;
    };

    template<class Delegate>
    struct ProxyInterface
    {
        template<class... Ts>
        void operator()(InterfaceBase & base, Ts && ... args) {
            this->delegate(base.next.get(), std::forward<Ts>(args)...);
        }

        Delegate delegate;
    };

    template<class Delegate>
    using ProxyClass = ProxyInterface<Delegate>;

    template<class Delegate>
    using InterfaceClass = meta::final_<ProxyGD<ProxyInterface<Delegate>, InterfaceBase>>;

    template<class TFilter>
    struct DelegtateFilter
    {
        template<class... Ts>
        void operator()(InterfaceBase & base, Ts const & ... args) {
            this->filter(*base.next, args...);
        }

        TFilter filter;
    };
    template<class TFilter>
    using Filter = ProxyGD<DelegtateFilter<TFilter>, InterfaceBase>;

    template<class FilterClass>
    InterfaceBase * new_filter(FilterClass && filter) {
        return new Filter<FilterClass>(
            DelegtateFilter<FilterClass>{std::forward<FilterClass>(filter)},
            std::move(this->interface_)
        );
    }

    struct IsId {
        TypeId id;
        bool operator()(GraphicDevice const & p) const noexcept {
            return &p == this->id;
        }
        bool operator()(GraphicDevicePtr const & p) const noexcept {
            return (*this)(*p);
        }
        bool operator()(InterfacePtr const & p) const noexcept {
            return (*this)(*p);
        }
    };

    struct DelegtateActions
    {
        template<class... Ts>
        void operator()(GraphicDevice *, Ts const & ... args) {
            for (auto & gd : this->gd_list_) {
                gd->draw(args...);
            }
        }

        TypeId add(GraphicDevicePtr && pgdi) {
            this->gd_list_.push_back(std::move(pgdi));
            return this->gd_list_.back().get();
        }

        GraphicDevicePtr remove(TypeId id) {
            auto pos = std::find_if(this->gd_list_.begin(), this->gd_list_.end(), IsId{id});
            if (pos != this->gd_list_.end()) {
                using std::swap;
                auto ret = exchange(*pos, std::move(this->gd_list_.back()));
                this->gd_list_.pop_back();
                return ret;
            }
            return GraphicDevicePtr{nullptr, no_delete};
        }

        std::vector<GraphicDevicePtr> gd_list_;
    };
    using Actions = InterfaceClass<DelegtateActions>;

    DelegtateActions & actions() const {
        return this->actions_->get_proxy().delegate;
    }

    Actions * actions_ {new Actions};
    InterfacePtr interface_ {actions_};
};

Drawable::InterfacePtr Drawable::remove_filter(Drawable::TypeId id) {
    if (this->interface_.get() == this->actions_) {
        return InterfacePtr{};
    }
    IsId pred{id};
    auto next = [](InterfacePtr * p) { return &(*p)->next; };
    auto * iprev = &this->interface_;
    auto * icurr = next(iprev);
    while (iprev->get() != this->actions_) {
        if (pred(*iprev)) {
            return exchange(*iprev, std::move(*icurr));
        }
        iprev = icurr;
        icurr = next(icurr);
    }
    return InterfacePtr{};
}


}

#endif
