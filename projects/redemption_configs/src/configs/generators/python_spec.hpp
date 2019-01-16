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

#include "utils/sugar/algostring.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <unordered_map>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

namespace python_spec_writer
{

using cfg_attributes::type_;
namespace types = cfg_attributes::types;

template<class T>
void write_type_info(std::ostream& /*out*/, type_<T>)
{}

inline void write_type_info(std::ostream& out, type_<std::chrono::hours>)
{ out << "(is in hour)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::minutes>)
{ out << "(is in minute)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::seconds>)
{ out << "(is in second)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::milliseconds>)
{ out << "(is in millisecond)\n"; }

template<class T, class Ratio>
void write_type_info(std::ostream& out, type_<std::chrono::duration<T, Ratio>>)
{ out << "(is in " << Ratio::num << "/" << Ratio::den << " second)\n"; }

template<class T, class Ratio, long min, long max>
void write_type_info(std::ostream& out, type_<types::range<std::chrono::duration<T, Ratio>, min, max>>)
{ write_type_info(out, type_<std::chrono::duration<T, Ratio>>{}); }


template<class T, class Pack>
void write_description(std::ostream& out, type_enumerations& enums, type_<T>, Pack const & pack)
{
    auto& d = get_desc(pack);
    if (!d.empty()) {
        out << d.c_str() << "\n";
    }
    else if constexpr (std::is_enum_v<T>) {
        apply_enumeration_for<T>(enums, [&out](auto const & e) {
            if (e.desc) {
                out << e.desc << "\n";
            }
        });
    }
}


using spec_internal_attr = cfg_attributes::spec::internal::attr;

inline void write_spec_attr(std::ostream& out, spec_internal_attr attr)
{
    if (bool(attr & spec_internal_attr::iptables_in_gui)) out << "_iptables\n";
    if (bool(attr & spec_internal_attr::advanced_in_gui)) out << "_advanced\n";
    if (bool(attr & spec_internal_attr::hidden_in_gui))   out << "_hidden\n";
    if (bool(attr & spec_internal_attr::hex_in_gui))      out << "_hex\n";
    if (bool(attr & spec_internal_attr::password_in_gui)) out << "_password\n";
}


inline void write_member(std::ostream& out, std::string const& member_name)
{
    out << "\"" << member_name << " = ";
}


inline void write_section(std::ostream& out, std::string const& section_name)
{
    if (!section_name.empty()) {
        out << "\"[" << section_name << "]\\n\\n\"\n\n";
    }
}


namespace impl
{
    struct exprio
    {
        const char * value;
        friend std::ostream & operator << (std::ostream & os, exprio const & eio) {
            return os << "\" << (" << eio.value << ") << \"";
        }
    };

    inline exprio quoted2(cfg_attributes::cpp::expr e) { return {e.value}; }
    template<class T> static io_quoted2 quoted2(T const & s) { return s; }
    template<class T> static char const * quoted2(types::list<T> const &) { return ""; }

    inline std::string stringize_bool(bool x) { return bool(x) ? "True" : "False"; }

    inline exprio stringize_bool(cfg_attributes::cpp::expr e) { return {e.value}; }

    inline char const * stringize_integral(bool x) = delete;
    inline char const * stringize_integral(types::integer_base) { return "0"; }
    inline char const * stringize_integral(types::u16) { return "0"; }
    inline char const * stringize_integral(types::u32) { return "0"; }
    inline char const * stringize_integral(types::u64) { return "0"; }
    template<class T> T const & stringize_integral(T const & x) { return x; }

    template<class Int, long min, long max, class T>
    T stringize_integral(types::range<Int, min, max>)
    {
        static_assert(!min, "unspecified value but 'min' isn't 0");
        return {};
    }

    inline exprio stringize_integral(cfg_attributes::cpp::expr e) { return {e.value}; }
}

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<bool>, T x)
{ out << "boolean(default=" << impl::stringize_bool(x) << ")"; }

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<std::string>, T const & s)
{ out << "string(default='" << impl::quoted2(s) << "')"; }

template<class Int, class T>
std::enable_if_t<
    std::is_base_of<types::integer_base, Int>::value
    or
    std::is_integral<Int>::value
>
write_type(std::ostream& out, type_enumerations&, type_<Int>, T i)
{
    out << "integer(";
    if (std::is_unsigned<Int>::value || std::is_base_of<types::unsigned_base, Int>::value) {
        out << "min=0, ";
    }
    out << "default=" << impl::stringize_integral(i) << ")";
}

template<class Int, long min, long max, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::range<Int, min, max>>, T i)
{ out << "integer(min=" << min << ", max=" << max << ", default=" << impl::stringize_integral(i) << ")"; }


template<class T, class Ratio, class U>
void write_type(std::ostream& out, type_enumerations&, type_<std::chrono::duration<T, Ratio>>, U i)
{ out << "integer(min=0, default=" << impl::stringize_integral(i) << ")"; }

template<unsigned N, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::fixed_binary<N>>, T const & s)
{
    out << "string(min=" << N*2 << ", max=" << N*2 << ", default='"
        << io_hexkey{s.c_str(), N} << "')";
}

template<unsigned N, class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::fixed_string<N>>, T const & x)
{
    out << "string(max=" << N <<  ", default='" << impl::quoted2(x) << "')";
}

template<class T>
void write_type(std::ostream& out, type_enumerations& enums, type_<types::dirpath>, T const & x)
{
    write_type(out, enums, type_<typename types::dirpath::fixed_type>{}, x);
}

template<class T>
void write_type(std::ostream& out, type_enumerations&, type_<types::ip_string>, T const & x)
{
    out << "ip_addr(default='" << impl::quoted2(x) << "')";
}

template<class T, class L>
void write_type(std::ostream& out, type_enumerations&, type_<types::list<T>>, L const & s)
{
    if (is_empty(s)) {
        out << "string_list(default=list())";
    }
    else {
        out << "string_list(default=list('" << impl::quoted2(s) << "'))";
    }
}

namespace impl
{
    template<class T, class V>
    void write_value_(std::ostream& out, T const & name, V const & v, char const * prefix)
    {
        out << "  " << name;
        if (v.desc) {
            out << ": ";
            if (prefix) {
                out << prefix << " ";
            }
            out << v.desc;
        }
        else if (std::is_integral<T>::value) {
            out << ": " << io_replace(v.name, '_', ' ');
        }
        out << "\n";
    }

    inline void write_desc_value(std::ostream& out, type_enumeration const & e, char const * prefix)
    {
        if (e.is_string_parser) {
            if (std::none_of(begin(e.values), end(e.values), [](type_enumeration::Value const & v) {
                return v.desc;
            })) {
                return ;
            }
        }

        unsigned d = 0;
        bool const is_autoinc = e.flag == type_enumeration::autoincrement;
        for (type_enumeration::Value const & v : e.values) {
            if (e.is_string_parser) {
                write_value_(out, (v.alias ? v.alias : v.name), v, prefix);
            }
            else {
                write_value_(out, (is_autoinc ? d : (1 << d >> 1)), v, prefix);
            }
            ++d;
        }

        if (type_enumeration::flags == e.flag) {
            out << "(note: values can be added (everyone: 1+2+4=7, mute: 0))";
        }
    }

    inline void write_desc_value(std::ostream& out, type_enumeration_set const & e, char const * prefix)
    {
        for (type_enumeration_set::Value const & v : e.values) {
            write_value_(out, v.val, v, prefix);
        }
    }
}

template<class T, class Pack>
void write_enumeration_value_description(std::ostream& out, type_enumerations& enums, type_<T>, Pack const & pack)
{
    using cfg_attributes::prefix_value;

    if constexpr (std::is_enum_v<T>) {
        apply_enumeration_for<T>(enums, [&](auto const & e) {
            impl::write_desc_value(out, e, value_or<prefix_value>(pack, prefix_value{}).value);
            if (e.info) {
                out << e.info;
            }
        });
    }
    else {
        static_assert(!is_convertible_v<Pack, prefix_value>, "prefix_value only with enums type");
    }
}

namespace impl
{
    template<class T>
    void write_enum_value(std::ostream& out, type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            out << "integer(min=0, max=" << e.max() << ", default=" << default_value << ")";
        }
        else if (e.is_string_parser) {
            out << "option(";
            for (type_enumeration::Value const & v : e.values) {
                out << "'" << (v.alias ? v.alias : v.name) << "', ";
            }
            out << "default='" << e.values[default_value].name << "')";
        }
        else {
            out << "option(";
            for (unsigned i = 0; i < e.count(); ++i) {
                out << i << ", ";
            }
            out << "default=" << default_value << ")";
        }
    }

    template<class T>
    void write_enum_value(std::ostream& out, type_enumeration_set const & e, T default_value)
    {
        out << "option(";
        for (type_enumeration_set::Value const & v : e.values) {
            out << v.val << ", ";
        }
        out << "default=" << default_value << ")";
    }
}

template<class T, class E>
std::enable_if_t<std::is_enum_v<E>>
write_type(std::ostream& out, type_enumerations& enums, type_<T>, E const & x)
{
    static_assert(std::is_same<T, E>::value, "");
    using ll = long long;
    apply_enumeration_for<T>(enums, [&](auto const & e) {
        impl::write_enum_value(out, e, ll{static_cast<std::underlying_type_t<E>>(x)});
    });
}


using namespace cfg_attributes;

struct IniPythonSpecWriterBase
{
    using attribute_name_type = spec::name;

    std::string filename_;
    std::ofstream out_file_;
    std::ostringstream out_member_;

    std::ostream & out() { return this->out_member_; }

    IniPythonSpecWriterBase(std::string filename)
    : filename_(std::move(filename))
    , out_file_(this->filename_)
    {}

    void do_init()
    {
        this->out_file_ <<
            "#include \"config_variant.hpp\"\n\n"
            "\"## Python spec file for RDP proxy.\\n\\n\\n\"\n"
        ;
    }

    int do_finish()
    {
        if (!this->out_file_) {
            std::cerr << this->filename_ << ": " << strerror(errno) << "\n";
        }
        return 0;
    }

    void do_start_section(std::string const & /*section_name*/)
    {}

    void do_stop_section(std::string const & section_name)
    {
        auto str = this->out_member_.str();
        if (!str.empty()) {
            write_section(this->out_file_, section_name);
            this->out_file_ << str;
            this->out_member_.str("");
        }
    }
};

struct PythonSpecWriterBase : IniPythonSpecWriterBase
{
    using attribute_name_type = spec::name;

    using IniPythonSpecWriterBase::IniPythonSpecWriterBase;

    template<class Pack>
    void evaluate_member(std::string const & /*section_name*/, Pack const & infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            auto type = get_type<spec::type_>(infos);
            std::string const& member_name = get_name<spec::name>(infos);

            std::stringstream comments;

            write_description(comments, enums, type, infos);
            write_type_info(comments, type);
            write_enumeration_value_description(comments, enums, type, infos);

            this->out() << io_prefix_lines{comments.str().c_str(), "\"# ", "\\n\"", 0};
            comments.str("");

            write_spec_attr(comments, get_elem<spec_attr_t>(infos).value);

            this->out() << io_prefix_lines{comments.str().c_str(), "\"#", "\\n\"", 0};

            write_member(this->out(), member_name);
            write_type(this->out(), enums, type, get_default(type, infos));
            this->out() << "\\n\\n\"\n\n";
        }
    }
};

}

}
