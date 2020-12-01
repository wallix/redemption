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
#include <charconv>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

namespace python_spec_writer
{

constexpr char const* begin_raw_string = "R\"gen_config_ini(";
constexpr char const* end_raw_string = ")gen_config_ini\"";

using cfg_attributes::type_;
namespace types = cfg_attributes::types;
namespace traits = cfg_attributes::traits;

template<class T>
void write_type_info(std::ostream& /*out*/, type_<T>)
{
    if constexpr (std::is_enum_v<T>) {
    }
    else if constexpr (traits::is_integer_v<T>) {
    }
    else {
        static_assert(!sizeof(T), "missing implementation");
    }
}

//@{
template<unsigned N> void write_type_info(std::ostream&, type_<types::fixed_string<N>>) {}
inline void write_type_info(std::ostream&, type_<bool>) {}
inline void write_type_info(std::ostream&, type_<std::string>) {}
inline void write_type_info(std::ostream&, type_<types::dirpath>) {}
inline void write_type_info(std::ostream&, type_<types::ip_string>) {}
//@}

inline void write_type_info(std::ostream& out, type_<types::file_permission>)
{ out << "(in octal or symbolic mode format (as chmod Linux command))\n"; }

template<unsigned N>
void write_type_info(std::ostream& out, type_<types::fixed_binary<N>>)
{ out << "(in hexadecimal format)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::hours>)
{ out << "(in hours)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::minutes>)
{ out << "(in minutes)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::seconds>)
{ out << "(in seconds)\n"; }

inline void write_type_info(std::ostream& out, type_<std::chrono::milliseconds>)
{ out << "(in milliseconds)\n"; }

template<class T, class Ratio>
void write_type_info(std::ostream& out, type_<std::chrono::duration<T, Ratio>>)
{ out << "(in " << Ratio::num << "/" << Ratio::den << " seconds)\n"; }

template<class T, long min, long max>
void write_type_info(std::ostream& out, type_<types::range<T, min, max>>)
{ write_type_info(out, type_<T>{}); }

template<class T>
void write_type_info(std::ostream& out, type_<types::list<T>>)
{ out << "(values are comma-separated)\n"; }


template<class T>
void write_description(std::ostream& out, type_enumerations& enums, type_<T>, std::string const& d)
{
    if (!d.empty()) {
        out << d << "\n";
    }
    else if constexpr (std::is_enum_v<T>) {
        type_enumeration const& e = enums.get_enum<T>();
        if (e.desc) {
            out << e.desc << "\n";
        }
    }
}

using spec_internal_attr = cfg_attributes::spec::internal::attr;

template<class T>
spec_internal_attr attr_hex_if_enum_flag(type_<T>, type_enumerations& enums)
{
    spec_internal_attr attr{};
    if constexpr (std::is_enum_v<T>)
    {
        type_enumeration const& e = enums.get_enum<T>();
        if (type_enumeration::Category::flags == e.cat) {
            attr = spec_internal_attr::hex_in_gui;
        }
    }
    return attr;
}

inline void write_spec_attr(std::ostream& out, spec_internal_attr attr)
{
    if (bool(attr & spec_internal_attr::iptables_in_gui)) out << "_iptables\n";
    if (bool(attr & spec_internal_attr::advanced_in_gui)) out << "_advanced\n";
    if (bool(attr & spec_internal_attr::hidden_in_gui))   out << "_hidden\n";
    if (bool(attr & spec_internal_attr::hex_in_gui))      out << "_hex\n";
    if (bool(attr & spec_internal_attr::password_in_gui)) out << "_password\n";
    if (bool(attr & spec_internal_attr::image_in_gui))    out << "_image=/var/wab/images/rdp-oem-logo.png\n";
}


inline void write_member(std::ostream& out, std::string const& member_name)
{
    out << member_name << " = ";
}


inline void write_section(std::ostream& out, std::string const& section_name)
{
    if (!section_name.empty()) {
        out << "[" << section_name << "]\n\n";
    }
}


namespace impl
{
    struct exprio
    {
        const char * value;
        friend std::ostream & operator << (std::ostream & os, exprio const & eio) {
            return os << end_raw_string <<  " << (" << eio.value << ") << " << begin_raw_string;
        }
    };

    inline char const * quoted2(types::dirpath const &) { return ""; }
    inline exprio quoted2(cfg_attributes::cpp::expr e) { return {e.value}; }
    template<class T> io_quoted2 quoted2(T const & s) { return s; }
    template<class T> char const * quoted2(types::list<T>) { return ""; }
    template<unsigned n> char const * quoted2(types::fixed_string<n>) { return ""; }

    inline std::string stringize_bool(bool x) { return x ? "True" : "False"; }

    inline exprio stringize_bool(cfg_attributes::cpp::expr e) { return {e.value}; }

    inline char const * stringize_integral(bool x) = delete;

    template<class T>
    decltype(auto) stringize_integral(T const & x)
    {
        if constexpr (traits::is_integer_v<T>) {
            return "0";
        }
        else {
            return x;
        }
    }

    template<class Int, long min, long max, class T>
    T stringize_integral(types::range<Int, min, max>)
    {
        static_assert(!min, "unspecified value but 'min' isn't 0");
        return {};
    }

    inline exprio stringize_integral(cfg_attributes::cpp::expr e) { return {e.value}; }
}

template<class T>
void write_type(std::ostream& out, type_<bool>, T x)
{ out << "boolean(default=" << impl::stringize_bool(x) << ")"; }

template<class T>
void write_type(std::ostream& out, type_<std::string>, T const & s)
{ out << "string(default='" << impl::quoted2(s) << "')"; }

template<class Int, class T>
std::enable_if_t<
    traits::is_integer_v<Int>
>
write_type(std::ostream& out, type_<Int>, T i)
{
    out << "integer(";
    if (traits::is_unsigned_v<Int>) {
        out << "min=0, ";
    }
    out << "default=" << impl::stringize_integral(i) << ")";
}

template<class Int, long min, long max, class T>
void write_type(std::ostream& out, type_<types::range<Int, min, max>>, T i)
{ out << "integer(min=" << min << ", max=" << max << ", default=" << impl::stringize_integral(i) << ")"; }


template<class T, class Ratio, class U>
void write_type(std::ostream& out, type_<std::chrono::duration<T, Ratio>>, U i)
{ out << "integer(min=0, default=" << impl::stringize_integral(i) << ")"; }

template<unsigned N, class T>
void write_type(std::ostream& out, type_<types::fixed_binary<N>>, T const & s)
{
    out << "string(min=" << N*2 << ", max=" << N*2 << ", default='"
        << io_hexkey{s.c_str(), N} << "')";
}

template<unsigned N, class T>
void write_type(std::ostream& out, type_<types::fixed_string<N>>, T const & x)
{
    out << "string(max=" << N <<  ", default='" << impl::quoted2(x) << "')";
}

template<class T>
void write_type(std::ostream& out, type_<types::dirpath>, T const & x)
{
    namespace globals = cfg_attributes::globals;
    out << "string(max=" << globals::path_max <<  ", default='" << impl::quoted2(x) << "')";
}

template<class T>
void write_type(std::ostream& out, type_<types::ip_string>, T const & x)
{
    out << "ip_addr(default='" << impl::quoted2(x) << "')";
}

template<class T, class L>
void write_type(std::ostream& out, type_<types::list<T>>, L const & s)
{
    out << "string(default='" << impl::quoted2(s) << "')";
}

template<class T>
void write_type(std::ostream& out, type_<types::file_permission>, T const & x)
{
    char octal[32]{};
    (void)std::to_chars(std::begin(octal), std::end(octal), x, 8);
    out << "string(default='" << octal << "')";
}

namespace impl
{
    struct HexFlag
    {
        unsigned long long v;
        std::size_t max_element;
    };

    inline std::ostream& operator<<(std::ostream& out, HexFlag const& h)
    {
        return out << "0x"
            << std::setfill('0') << std::setw(int((h.max_element+3)/4))
            << std::hex << h.v << std::dec;
    }

    template<class T, class V>
    void write_value_(std::ostream& out, T const & name, V const & v, char const * prefix)
    {
        auto pos = out.tellp();
        out << "  " << name;
        if (v.desc) {
            out << ": ";
            int pad = int(out.tellp() - pos);
            if (prefix) {
                out << prefix << " ";
            }

            char const* desc = v.desc;
            // add padding after new line for align the description
            while (const char* nl = strchr(desc, '\n')) {
                out.write(desc, nl - desc + 1);
                out << std::setw(pad) << "";
                desc = nl+1;
            }
            out << desc;
        }
        else if constexpr (std::is_integral<T>::value || std::is_same<T, HexFlag>::value) {
            out << ": " << io_replace(v.name, '_', ' ');
        }
        out << "\n";
    }

    inline bool write_desc_value(std::ostream& out, type_enumeration const & e, char const * prefix, bool is_enum_parser)
    {
        if (is_enum_parser
         && std::none_of(begin(e.values), end(e.values),
            [](type_enumeration::value_type const & v) { return v.desc; })
        ) {
            return false;
        }

        unsigned long long total = 0;
        std::ostringstream oss;
        for (type_enumeration::value_type const & v : e.values) {
            if (v.exclude) {
            }
            else if (is_enum_parser) {
                write_value_(out, v.get_name(), v, prefix);
            }
            else switch (e.cat) {
                case type_enumeration::Category::set:
                case type_enumeration::Category::autoincrement:
                    write_value_(out, v.val, v, prefix);
                    break;

                case type_enumeration::Category::flags:{
                    auto f = v.val;
                    write_value_(out, HexFlag{f, e.values.size()}, v, prefix);
                    if (f) {
                        total |= f;
                        oss << HexFlag{f, e.values.size()} << " + ";
                    }
                }
            }
        }

        if (type_enumeration::Category::flags == e.cat) {
            auto s = oss.str();
            s[s.size() - 2] = '=';
            out << "Note: values can be added ("
                << (prefix ? prefix : "enable")
                << " all: " << s << HexFlag{total, e.values.size()} << ")";
        }

        return true;
    }
}

template<class T, class Pack>
void write_enumeration_value_description(std::ostream& out, type_enumerations& enums, type_<T>, Pack const & pack, bool is_enum_parser)
{
    using cfg_attributes::prefix_value;

    if constexpr (std::is_enum_v<T>) {
        type_enumeration const& e = enums.get_enum<T>();
        impl::write_desc_value(out, e, value_or<prefix_value>(pack, prefix_value{}).value, is_enum_parser);
        if (e.info) {
            out << e.info;
        }
    }
    else {
        (void)is_enum_parser;
        static_assert(!is_convertible_v<Pack, prefix_value>, "prefix_value only with enums type");
    }
}

template<class D>
char const * get_value_name(type_enumeration const & e, D const& x)
{
    if constexpr (std::is_enum_v<D>) {
        for (auto& v : e.values) {
            if (D(v.val) == x) {
                return v.get_name();
            }
        }
        throw std::runtime_error("unknown value");
    }
    else {
        if (!x.empty()) {
            throw std::runtime_error("is not a enum value");
        }
        auto& v = e.values[0];
        if (v.val) {
            throw std::runtime_error("add a default value");
        }
        return v.get_name();
    }
}

template<class T, class U, class D>
void write_type2(std::ostream& out, type_enumerations& enums, type_<T>, type_<U>, D const & x)
{
    if constexpr (std::is_enum_v<U>) {
        type_enumeration const& e = enums.get_enum<U>();
        auto default_value = +std::underlying_type_t<U>(x);
        switch (e.cat) {
        case type_enumeration::Category::flags:
            if constexpr (!std::is_enum_v<T>) {
                throw std::runtime_error("is not a enum value");
            }
            out << "integer(min=0, max=" << e.max() << ", default=" << default_value << ")";
            break;

        case type_enumeration::Category::autoincrement:
        case type_enumeration::Category::set:
            if constexpr (std::is_same_v<T, std::string>) {
                out << "option(";
                for (type_enumeration::value_type const & v : e.values) {
                    if (v.exclude) continue;
                    out << "'" << v.get_name() << "', ";
                }
                out << "default='" << get_value_name(e, x) << "')";
            }
            else {
                out << "option(";
                for (type_enumeration::value_type const & v : e.values) {
                    if (v.exclude) continue;
                    out << v.val << ", ";
                }
                out << "default=" << default_value << ")";
            }
            break;
        }
    }
    else {
        write_type(out, type_<T>(), x);
    }
}

template<class T, class U>
auto get_semantic_type(type_<T>, type_<U>, bool * is_enum_parser)
{
    if constexpr (std::is_same_v<T, std::string>) {
        if constexpr (std::is_enum_v<U>) {
            *is_enum_parser = true;
            return type_<U>();
        }
        else {
            (void)is_enum_parser;
            return type_<T>();
        }
    }
    else {
        (void)is_enum_parser;
        return type_<T>();
    }
}


using namespace cfg_attributes;

struct IniPythonSpecWriterBase
{
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
            << begin_raw_string
            << "## Python spec file for RDP proxy.\n\n\n"
        ;
    }

    int do_finish()
    {
        this->out_file_ << end_raw_string << "\n";

        if (!this->out_file_) {
            std::cerr << this->filename_ << ": " << strerror(errno) << "\n";
        }
        return 0;
    }

    void do_start_section(Names const& /*section_names*/)
    {}

    void do_stop_section(Names const& section_names)
    {
        auto str = this->out_member_.str();
        if (!str.empty()) {
            write_section(this->out_file_, section_names.ini_name());
            this->out_file_ << str;
            this->out_member_.str("");
        }
    }
};

struct PythonSpecWriterBase : IniPythonSpecWriterBase
{
    using IniPythonSpecWriterBase::IniPythonSpecWriterBase;

    template<class Pack>
    void evaluate_member(Names const& /*section_names*/, Pack const & infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            Names const& names = infos;
            auto type = get_type<spec::type_>(infos);
            std::string const& member_name = names.ini_name();

            bool is_enum_parser = false;
            auto semantic_type = get_semantic_type(type, infos, &is_enum_parser);

            std::stringstream comments;

            write_description(comments, enums, semantic_type, get_desc(infos));
            write_type_info(comments, type);
            write_enumeration_value_description(comments, enums, semantic_type, infos, is_enum_parser);

            this->out() << io_prefix_lines{comments.str().c_str(), "# ", "", 0};
            comments.str("");

            write_spec_attr(comments,
                spec_attr_t(infos).value
              | attr_hex_if_enum_flag(semantic_type, enums));

            this->out() << io_prefix_lines{comments.str().c_str(), "#", "", 0};

            write_member(this->out(), member_name);
            write_type2(this->out(), enums, type, semantic_type, get_default(type, infos));
            this->out() << "\n\n";
        }
    }
};

}

}
