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

#ifndef REDEMPTION_GDI_UTILS_SERVICE_PROVIDER_HPP
#define REDEMPTION_GDI_UTILS_SERVICE_PROVIDER_HPP

#include <memory>
#include <iostream>

#include "gdi/meta/meta.hpp"
#include "non_null.hpp"

#include "noncopyable.hpp"
#include "exchange.hpp"


namespace gdi { namespace utils {

template<
    class ServiceInterface,
    class Delegate,
    template<class Proxy, class InterfaceBase> class ProxyInterface,
    class ServicePtr = ServiceInterface*
>
class service_provider : private noncopyable_but_movable
{
    struct InterfaceBase;
    using interface_pointer = std::unique_ptr<InterfaceBase>;

public:
    using service_interface = ServiceInterface;
    using service_pointer = ServicePtr;

    using filter_pointer = interface_pointer;

    using delegate_filter_base = Delegate;

    struct barrier_filter_base
    {
        template<class... Ts>
        void operator()(ServiceInterface &, Ts const & ...) {
        }
    };

    using type_id = void const *;


    service_provider() = default;

    type_id add_class(ServicePtr && pservice) {
        if (!pservice) {
            return {};
        }
        return this->actions().add(std::move(pservice));
    }

    template<class FilterClass>
    type_id add_filter(FilterClass && filter) {
        this->interface_.reset(this->new_filter(std::forward<FilterClass>(filter)));
        return this->interface_.get();
    }

    type_id add_filter(filter_pointer && pfilter) {
        if (!pfilter) {
            return {};
        }
        pfilter->next = std::move(this->interface_);
        this->interface_ = std::move(pfilter);
        return this->interface_.get();
    }

    ServicePtr remove_class(type_id id) {
        return this->actions().remove(id);
    }

    filter_pointer remove_filter(type_id id);

    template<class... Ts>
    void operator()(Ts && ... args) {
        Delegate()(*this->interface_, std::forward<Ts>(args)...);
    }


private:
    struct InterfaceBase : ServiceInterface
    {
        InterfaceBase() = default;

        InterfaceBase(interface_pointer && next)
        : next(std::move(next))
        {}

    private:
        friend service_provider;
        interface_pointer next;
    };

    template<class TFilter>
    struct DelegtateFilter
    {
        template<class... Ts>
        void operator()(InterfaceBase & base, Ts && ... args) {
            assert(bool(base.next));
            this->filter(*base.next, std::forward<Ts>(args)...);
        }

        TFilter filter;
    };
    template<class TFilter>
    using Filter = meta::final_<ProxyInterface<DelegtateFilter<TFilter>, InterfaceBase>>;

    template<class FilterClass>
    InterfaceBase * new_filter(FilterClass && filter) {
        return new Filter<FilterClass>(
            DelegtateFilter<FilterClass>{std::forward<FilterClass>(filter)},
            std::move(this->interface_)
        );
    }

    struct IsId {
        type_id id;
        bool operator()(ServiceInterface const & p) const noexcept {
            return &p == this->id;
        }
        bool operator()(ServicePtr const & p) const noexcept {
            return (*this)(*p);
        }
        bool operator()(interface_pointer const & p) const noexcept {
            return (*this)(*p);
        }
    };

    struct DelegtateActions
    {
        template<class... Ts>
        void operator()(InterfaceBase &, Ts && ... args) {
            for (auto & gd : this->gd_list_) {
                Delegate()(*gd, std::forward<Ts>(args)...);
            }
        }

        type_id add(ServicePtr && pservice) {
            this->gd_list_.push_back(std::move(pservice));
            return this->gd_list_.back().get();
        }

        ServicePtr remove(type_id id) {
            auto pos = std::find_if(this->gd_list_.begin(), this->gd_list_.end(), IsId{id});
            if (pos != this->gd_list_.end()) {
                using std::swap;
                auto ret = exchange(*pos, std::move(this->gd_list_.back()));
                this->gd_list_.pop_back();
                return ret;
            }
            return ServicePtr{nullptr};
        }

        std::vector<ServicePtr> gd_list_;
    };
    using Actions = meta::final_<ProxyInterface<DelegtateActions, InterfaceBase>>;

    DelegtateActions & actions() const {
        return this->actions_->get_proxy();
    }

    Actions * actions_ {new Actions};
    interface_pointer interface_ {actions_};
};

template<class ServiceIface, class Delegate, template<class, class> class ProxyIface, class ServicePtr>
typename service_provider<ServiceIface, Delegate, ProxyIface, ServicePtr>::interface_pointer
service_provider<ServiceIface, Delegate, ProxyIface, ServicePtr>::remove_filter(service_provider::type_id id) {
    if (this->interface_.get() == this->actions_) {
        return interface_pointer{};
    }
    IsId pred{id};
    auto next = [](interface_pointer * p) { return &(*p)->next; };
    auto * iprev = &this->interface_;
    auto * icurr = next(iprev);
    while (iprev->get() != this->actions_) {
        if (pred(*iprev)) {
            return exchange(*iprev, std::move(*icurr));
        }
        iprev = icurr;
        icurr = next(icurr);
    }
    return interface_pointer{};
}

} }

#endif
