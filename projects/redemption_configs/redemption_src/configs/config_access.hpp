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
    get      = 1 << 0,
    set      = 1 << 1,
    ask      = 1 << 2,
    is_asked = 1 << 3,
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

// fix for gcc-7
template<class...> using LazyInifile = Inifile;

template<class... Cfg>
class variables
{
    LazyInifile<Cfg...> & ini;

    struct Pack : Cfg... {};

    template<class T, accessmode Mode>
    static constexpr accessmode mode(var<T, Mode> /*unused*/) {
        return Mode;
    }

    template<class T>
    static constexpr bool has_access(accessmode amode) {
        return ((mode<T>(Pack()) & amode) == amode);
    }

public:
    variables(Inifile & ini)
    : ini(ini)
    {}

    template<class T>
    typename T::type const & get() const noexcept {
        static_assert(has_access<T>(accessmode::get), "T isn't gettable");
        return this->ini.template get<T>();
    }

    template<class T, class U>
    void set(U && new_value) {
        static_assert(has_access<T>(accessmode::set), "T isn't settable");
        return this->ini.template set<T>(static_cast<U&&>(new_value));
    }

    template<class T, class U>
    void set_acl(U && new_value) {
        static_assert(has_access<T>(accessmode::set), "T isn't settable");
        return this->ini.template set_acl<T>(static_cast<U&&>(new_value));
    }

    template<class T>
    void ask() {
        static_assert(has_access<T>(accessmode::ask), "T isn't askable");
        return this->ini.template ask<T>();
    }

    template<class T>
    bool is_asked() const {
        static_assert(has_access<T>(accessmode::is_asked), "T isn't is_askable");
        return this->ini.template is_asked<T>();
    }
};

template<class... Cfg>
Translation::language_t language(variables<Cfg...> const & vars) {
    return ::language(vars.template get<cfg::translation::language>());
}

} // namespace vcfg

using vcfg::language; /*NOLINT*/

#endif
