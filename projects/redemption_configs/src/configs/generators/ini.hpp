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
#include "utils/file_permissions.hpp"

#include <iostream>
#include <sstream>
#include <chrono>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace ini_writer {

using python_spec_writer::begin_raw_string;
using python_spec_writer::end_raw_string;


using namespace cfg_attributes;

template<class T>
void write_type_info(std::ostream& out, type_<T>)
{
    if constexpr (traits::is_unsigned_v<T>) {
        out << "min = 0";
    }
    else if constexpr (traits::is_signed_v<T>) {
    }
    else {
        static_assert(!sizeof(T), "missing implementation");
    }
}

//@{
template<class T>
void write_type_info(std::ostream&, type_<types::list<T>>) {}
inline void write_type_info(std::ostream&, type_<std::string>) {}
inline void write_type_info(std::ostream&, type_<types::ip_string>) {}
//@}

template<class T, class Ratio>
void write_type_info(std::ostream& out, type_<std::chrono::duration<T, Ratio>> t)
{ python_spec_writer::write_type_info(out, t); }

inline void write_type_info(std::ostream& out, type_<types::rgb> t)
{ python_spec_writer::write_type_info(out, t); }

inline void write_type_info(std::ostream& out, type_<bool>)
{ out << "value: 0 or 1"; }

template<class Int, long min, long max>
void write_type_info(std::ostream& out, type_<types::range<Int, min, max>>)
{
    out << "min = " << min << ", max = " << max << "\n";
    write_type_info(out, type_<Int>{});
}

template<unsigned N>
void write_type_info(std::ostream& out, type_<types::fixed_binary<N>>)
{ out << "(hexadecimal string of length " << N*2 << ")"; }

template<unsigned N>
void write_type_info(std::ostream& out, type_<types::fixed_string<N>>)
{ out << "maxlen = " << N; }

inline void write_type_info(std::ostream& out, type_<types::dirpath>)
{ out << "maxlen = " << globals::path_max; }

inline void write_type_info(std::ostream& out, type_<FilePermissions>)
{ out << "(is in octal or symbolic mode format (as chmod Linux command))\nmax = 777, min = 0"; }


namespace impl
{
    inline std::string stringize_bool(bool x)
    {
        return x ? "1" : "0";
    }

    inline auto stringize_bool(cpp::expr e)
    {
        return python_spec_writer::impl::stringize_bool(e);
    }

    using python_spec_writer::impl::stringize_integral;
    using python_spec_writer::impl::quoted2;

    using python_spec_writer::impl::CssColor;
}

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<bool>, T x)
{ out << impl::stringize_bool(x); }

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<std::string>, T const & s)
{ out << impl::quoted2(s); }

template<class Int, class T>
std::enable_if_t<
    traits::is_integer_v<Int>
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
void write_type(std::ostream& out, type_enumerations&, type_<types::dirpath>, T const & x)
{ out << impl::quoted2(x); }

inline void write_type(std::ostream& out, type_enumerations&, type_<types::rgb>, uint32_t x)
{ out << impl::CssColor(x); }

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

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<FilePermissions>, T const & x)
{
    char octal[32]{};
    (void)std::to_chars(std::begin(octal), std::end(octal), x, 8);
    out << octal;
}


struct IniWriterBase : python_spec_writer::IniPythonSpecWriterBase
{
    using IniPythonSpecWriterBase::IniPythonSpecWriterBase;

    void do_init()
    {
        this->out_file_ <<
            "#include \"config_variant.hpp\"\n\n"
            << begin_raw_string
            << "## Config file for RDP proxy.\n\n\n"
        ;
    }

    template<class Pack>
    void evaluate_member(Names const& /*section_names*/, Pack const & infos, type_enumerations& enums)
    {
        if constexpr (python_spec_writer::is_candidate_for_spec<Pack>) {
            Names const& names = infos;
            auto type = get_type<spec::type_>(infos);

            bool is_enum_parser = false;
            auto semantic_type = python_spec_writer::get_semantic_type(type, infos, &is_enum_parser);

            std::stringstream comments;

            using etype = typename decltype(semantic_type)::type;
            constexpr auto is_semantic_enum = std::is_enum_v<etype>;

            python_spec_writer::write_description(comments, enums, semantic_type, get_desc(infos));
            if constexpr (is_semantic_enum) {
                type_enumeration const& e = enums.get_enum<etype>();

                if (!is_enum_parser && e.cat != type_enumeration::Category::set) {
                    comments << "min = " << e.min() << ", max = " << e.max() << "\n";
                }

                if (!python_spec_writer::impl::write_desc_value(comments, e, value_or<prefix_value>(infos, prefix_value{}).value, is_enum_parser)) {
                    char const* prefix = "values: ";
                    switch (e.cat) {
                    case type_enumeration::Category::flags:
                        break;
                    case type_enumeration::Category::autoincrement:
                    case type_enumeration::Category::set:
                        for (type_enumeration::value_type const & v : e.values) {
                            if (!v.exclude) {
                                comments << prefix;
                                if (is_enum_parser) {
                                    comments << v.get_name();
                                }
                                else {
                                    comments << v.val;
                                }
                                prefix = ", ";
                            }
                        }
                        break;
                    }
                }
                if (e.info) {
                    comments << e.info;
                }
            }
            else {
                write_type_info(comments, type);
            }

            this->out() << io_prefix_lines{comments.str().c_str(), "# ", "", 0};
            comments.str("");

            python_spec_writer::write_spec_attr(comments,
                spec_attr_t(infos).value
              | python_spec_writer::attr_hex_if_enum_flag(semantic_type, enums));

            python_spec_writer::write_prefered_display_name(comments, names);

            this->out() << io_prefix_lines{comments.str().c_str(), "#", "", 0};

            python_spec_writer::write_member(this->out(), "#" + names.ini_name());
            const auto& default_value = get_default(type, infos);
            if constexpr (is_semantic_enum) {
                if constexpr (std::is_same_v<etype, typename decltype(type)::type>) {
                    this->out() << +std::underlying_type_t<etype>(default_value);
                }
                else {
                    type_enumeration const& e = enums.get_enum<etype>();
                    switch (e.cat) {
                    case type_enumeration::Category::flags:
                        throw std::runtime_error("logical error");
                    case type_enumeration::Category::autoincrement:
                    case type_enumeration::Category::set:
                        this->out() << python_spec_writer::get_value_name(e, default_value);
                        break;
                    }
                }
            }
            else {
                write_type(this->out(), enums, type, default_value);
            }

            this->out() << "\n\n";
        }
    }
};

}
}
