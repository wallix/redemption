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

#ifndef REDEMPTION_SRC_CORE_CONFIG_ACCESS_HPP
#define REDEMPTION_SRC_CORE_CONFIG_ACCESS_HPP

#include "configs/autogen/variables_configuration_fwd.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/translation.hpp"

class Inifile;


namespace vcfg {

enum class accessmode {
    get             = 1 << 0,
    set             = 1 << 1,
    ask             = 1 << 2,
    is_asked        = 1 << 3,
    get_mutable_ref = 1 << 4,
};

constexpr inline accessmode operator | (accessmode x, accessmode y) {
    return static_cast<accessmode>(underlying_cast(x) | underlying_cast(y));
}

constexpr inline accessmode operator & (accessmode x, accessmode y) {
    return static_cast<accessmode>(underlying_cast(x) & underlying_cast(y));
}

template<class T, accessmode Mode>
struct var
{};

namespace detail
{
    template<class T, accessmode Mode>
    constexpr bool has_vcfg_access(var<T, Mode> /*unused*/, accessmode amode)
    {
        return ((Mode & amode) == amode);
    }

    struct GetPrivate
    {
        template<class V>
        static Inifile& ini(V const& vars)
        {
            return vars.ini;
        }

        template<class V>
        static typename V::Pack pack()
        {
            return typename V::Pack();
        }
    };

    template<class Cfg>
    struct CheckCompatibility;

    template<class T, accessmode Mode>
    struct CheckCompatibility<var<T, Mode>>
    {
        template<accessmode Mode2>
        static void check(var<T, Mode2> /*unused*/)
        {
            static_assert((Mode & Mode2) == Mode, "incompatible restriction");
        }
    };
}

// fix for gcc-7
template<class...> using LazyInifile = Inifile;

template<class... Cfg>
class variables
{
    LazyInifile<Cfg...> & ini;

    struct Pack : Cfg... {};

    friend class detail::GetPrivate;

public:
    variables(Inifile & ini)
    : ini(ini)
    {}

    variables(variables const& other) = default;

    template<class... CfgOther>
    variables(variables<CfgOther...> const& other)
    : ini(detail::GetPrivate::ini(other))
    {
        auto pack2 = detail::GetPrivate::pack<variables<CfgOther...>>();
        (..., detail::CheckCompatibility<Cfg>::check(pack2));
    }

    template<class T>
    [[nodiscard]] typename T::type const & get() const noexcept
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::get), "T isn't gettable");
        return this->ini.template get<T>();
    }

    template<class T>
    [[nodiscard]] typename T::type& get_mutable_ref() const noexcept
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::get_mutable_ref), "get_ref isn't enabled");
        return this->ini.template get_mutable_ref<T>();
    }

    template<class T, class U>
    void set(U && new_value)
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::set), "T isn't settable");
        return this->ini.template set<T>(static_cast<U&&>(new_value));
    }

    template<class T, class U>
    void set_acl(U && new_value)
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::set), "T isn't settable");
        return this->ini.template set_acl<T>(static_cast<U&&>(new_value));
    }

    template<class T>
    void ask()
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::ask), "T isn't askable");
        return this->ini.template ask<T>();
    }

    template<class T>
    [[nodiscard]] bool is_asked() const
    {
        static_assert(detail::has_vcfg_access<T>(Pack(), accessmode::is_asked), "T isn't is_askable");
        return this->ini.template is_asked<T>();
    }
};

template<class... Cfg>
::Language language(variables<Cfg...> const & vars)
{
    return vars.template get<cfg::translation::language>();
}

template<class... Cfg>
::Language login_language(variables<Cfg...> const & vars)
{
    switch (vars.template get<cfg::translation::login_language>())
    {
        case LoginLanguage::Auto: return language(vars);
        case LoginLanguage::EN: return ::Language::en;
        case LoginLanguage::FR: return ::Language::fr;
    }

    assert("Unknown LoginLanguage value");
    return ::Language::en;
}

} // namespace vcfg

using vcfg::language; /*NOLINT*/

#endif
