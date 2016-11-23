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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

template<class E>
struct implicit_bool_flags
{
    operator bool() const { return bool(this->flags_); }

    implicit_bool_flags operator | (E y) const
    { return {this->flags_ | y}; }

    implicit_bool_flags operator & (E y) const
    { return {this->flags_ & y}; }

    implicit_bool_flags(E flags) : flags_(flags) {}

private:
    E const flags_;
};

#define REDEMPTION_VERBOSE_FLAGS(visibility, verbose_member_name)   \
    enum class VerboseFlags : uint32_t;                             \
                                                                    \
    friend VerboseFlags operator | (VerboseFlags x, VerboseFlags y) \
    { return VerboseFlags(uint32_t(x) | uint32_t(y)); }             \
    friend VerboseFlags operator & (VerboseFlags x, VerboseFlags y) \
    { return VerboseFlags(uint32_t(x) & uint32_t(y)); }             \
                                                                    \
visibility:                                                         \
    implicit_bool_flags<VerboseFlags> const verbose_member_name;    \
                                                                    \
public:                                                             \
    enum class VerboseFlags : uint32_t


namespace detail
{
    template<class Ini>
    struct ini_get_debug_fn
    {
        Ini const & ini;

        template<class T>
        uint32_t operator()(T) const
        {
            return {this->ini.template get<T>()};
        }
    };
}

#ifdef IN_IDE_PARSER
# define REDEMPTION_DEBUG_CONFIG_TO_VERBOSE_FLAGS(extractor)               \
    static VerboseFlags debug_config_to_verbose_flags(Inifile const & ini) \
    {                                                                      \
        ::detail::ini_get_debug_fn<Inifile> get;                           \
        return VerboseFlags(extractor);                                    \
    }
#else
# define REDEMPTION_DEBUG_CONFIG_TO_VERBOSE_FLAGS(extractor)               \
    template<class Inifile>                                                \
    static VerboseFlags debug_config_to_verbose_flags(Inifile const & ini) \
    {                                                                      \
        struct cfg {                                                       \
            using debug = typename Inifile::debug_section_type;            \
        };                                                                 \
        ::detail::ini_get_debug_fn<Inifile> get{ini};                      \
        return VerboseFlags(extractor);                                    \
    }
#endif
