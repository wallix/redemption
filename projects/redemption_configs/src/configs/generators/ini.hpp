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

#pragma once

#include "configs/attributes/spec.hpp"
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"

#include "configs/generators/python_spec.hpp"

#include <iostream>
#include <chrono>
#include <vector>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace ini_writer {

using namespace cfg_attributes;

namespace impl
{
    inline void write_enum_info(std::ostream& out, type_enumeration const & e)
    {
        if (e.is_string_parser) {
            out << "values: ";
            for (type_enumeration::Value const & v : e.values) {
                out << "'" << (v.alias ? v.alias : v.name) << "', ";
            }
            out << "\n";
        }
        else {
            out << "min = 0, max = " << e.count() << "\n";
        }
    }

    inline void write_enum_info(std::ostream& /*out*/, type_enumeration_set const & /*e*/)
    {
        // out << "values: ";
        // for (type_enumeration_set::Value const & v : e.values) {
        //     out << v.val << ", ";
        // }
        // out << "\n";
    }
}

template<class T>
void write_type_info(std::ostream& out, type_enumerations& enums, type_<T>)
{
    if constexpr (std::is_enum_v<T>) {
        apply_enumeration_for<T>(enums, [&](auto const & e) {
            impl::write_enum_info(out, e);
        });
    }
    else if (std::is_unsigned<T>::value || std::is_base_of<types::unsigned_base, T>::value) {
        out << "min = 0";
    }
}

template<class T, class Ratio>
void write_type_info(std::ostream& out, type_enumerations&, type_<std::chrono::duration<T, Ratio>> t)
{ python_spec_writer::write_type_info(out, t); }

// template<class T, class Ratio>
// void write_type_info(std::ostream& out, type_enumerations&, type_<std::chrono::duration<T, Ratio>>)
// { out << "min = 0\n"; }

inline void write_type_info(std::ostream& out, type_enumerations&, type_<bool>)
{ out << "value: 0 or 1"; }

template<class Int, long min, long max>
void write_type_info(std::ostream& out, type_enumerations& enums, type_<types::range<Int, min, max>>)
{
    out << "min = " << min << ", max = " << max << "\n";
    write_type_info(out, enums, type_<Int>{});
}

template<unsigned N, class T>
void write_type_info(std::ostream& out, type_enumerations&, type_<types::fixed_binary<N>>)
{ out << "(hexadecimal string) size = " << N*2 << "\n"; }

template<unsigned N>
void write_type_info(std::ostream& out, type_enumerations&, type_<types::fixed_string<N>>)
{ out << "maxlen = " << N << "\n"; }

inline void write_type_info(std::ostream& out, type_enumerations& enums, type_<types::dirpath>)
{ write_type_info(out, enums, type_<typename types::dirpath::fixed_type>{}); }


namespace impl
{
    inline std::string stringize_bool(bool x)
    {
        return bool(x) ? "1" : "0";
    }

    inline auto stringize_bool(cpp::expr e)
    {
        return python_spec_writer::impl::stringize_bool(e);
    }

    using python_spec_writer::impl::stringize_integral;
    using python_spec_writer::impl::quoted2;
}

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<bool>, T x)
{ out << impl::stringize_bool(x); }

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<std::string>, T const & s)
{ out << impl::quoted2(s); }

template<class Int, class T>
std::enable_if_t<
    std::is_base_of<types::integer_base, Int>::value
    or
    std::is_integral<Int>::value
>
write_type(std::ostream& out, type_enumerations&, type_<Int>, T i)
{ out << impl::stringize_integral(i); }

template<class Int, long min, long max, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::range<Int, min, max>>, T i)
{ out << impl::stringize_integral(i); }


template<class T, class Ratio, class U>
void write_type(std::ostream& out, type_enumerations&, type_<std::chrono::duration<T, Ratio>>, U i)
{ out << impl::stringize_integral(i); }

template<unsigned N, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::fixed_binary<N>>, T const & x)
{ out << io_hexkey{x.c_str(), N}; }

template<unsigned N, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::fixed_string<N>>, T const & x)
{ out << impl::quoted2(x); }

template<class T>
void write_type(std::ostream& out, type_enumerations& enums, type_<types::dirpath>, T const & x)
{ write_type(out, enums, type_<typename types::dirpath::fixed_type>{}, x); }

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::ip_string>, T const & x)
{ out << impl::quoted2(x); }

template<class T, class L>
void write_type(std::ostream& out, type_enumerations&, type_<types::list<T>>, L const & s)
{
    if (!is_empty(s)) {
        out << impl::quoted2(s);
    }
}

namespace impl
{
    template<class T>
    void write_enum_value(std::ostream& out, type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            out << default_value;
        }
        else if (e.is_string_parser) {
            out << e.values[default_value].name;
        }
        else {
            out << default_value;
        }
    }

    template<class T>
    void write_enum_value(std::ostream& out, type_enumeration_set const &, T default_value)
    { out << default_value; }
}

template<class T, class E>
std::enable_if_t<std::is_enum_v<T>>
write_type(std::ostream& out, type_enumerations& enums, type_<T>, E const & x)
{
    using ll = long long;
    static_assert(std::is_same<T, E>::value, "");
    apply_enumeration_for<T>(enums, [&](auto const & e) {
        impl::write_enum_value(out, e, ll{static_cast<std::underlying_type_t<E>>(x)});
    });
}


struct IniWriterBase : python_spec_writer::IniPythonSpecWriterBase
{
    using attribute_name_type = spec::name;

    using IniPythonSpecWriterBase::IniPythonSpecWriterBase;

    void do_init()
    {
        this->out_file_ <<
            "#include \"config_variant.hpp\"\n\n"
            "\"## Config file for RDP proxy.\\n\\n\\n\"\n"
        ;
    }

    template<class Pack>
    void evaluate_member(std::string const & /*section_name*/, Pack const & infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            auto type = get_type<spec::type_>(infos);
            std::string const& member_name = get_name<spec::name>(infos);

            std::stringstream comments;

            python_spec_writer::write_description(comments, enums, type, infos);
            write_type_info(comments, enums, type);
            python_spec_writer::write_enumeration_value_description(comments, enums, type, infos);

            this->out() << io_prefix_lines{comments.str().c_str(), "\"# ", "\\n\"", 0};
            comments.str("");

            python_spec_writer::write_spec_attr(comments, get_elem<spec_attr_t>(infos).value);

            this->out() << io_prefix_lines{comments.str().c_str(), "\"#", "\\n\"", 0};

            python_spec_writer::write_member(this->out(), "#" + member_name);
            write_type(this->out(), enums, type, get_default(type, infos));
            this->out() << "\\n\\n\"\n\n";
        }
    }
};

}
}
