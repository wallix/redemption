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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <unordered_map>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace python_spec_writer {

using namespace cfg_attributes;

template<class Inherit, class AttributeName = spec::name>
struct PythonSpecWriterBase : ConfigSpecWriterBase<Inherit, AttributeName>
{
    using base_type = PythonSpecWriterBase;

    std::ofstream out_file_;
    std::ostringstream out_member_;

    std::ostream & out() { return this->out_member_; }

    PythonSpecWriterBase(char const * filename)
    : out_file_(filename)
    {}

    void do_init()
    {
        this->out_file_ <<
            "#include \"config_variant.hpp\"\n\n"
            "\"## Python spec file for RDP proxy.\\n\\n\\n\"\n"
        ;
    }

    void do_stop_section(std::string const & section_name)
    {
        auto str = this->out_member_.str();
        if (!str.empty()) {
            if (!section_name.empty()) {
                this->out_file_ << "\"[" << section_name << "]\\n\\n\"\n\n";
            }
            this->out_file_ << str;
            this->out_member_.str("");
        }
    }

    template<class Pack>
    void do_member(std::string const & /*section_name*/, Pack const & infos)
    {
        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            do_member_impl(infos, get_type<spec::type_>(infos), get_name<spec::name>(infos));
        }
    }

    template<class Type, class Pack>
    void do_member_impl(Pack const & infos, Type type, std::string const & member_name)
    {
        this->write_description(type, infos);
        this->inherit().write_type_info(type);
        this->write_enumeration_value_description(type, infos);

        using attr_t = spec::internal::attr;
        auto attr = get_elem<spec_attr_t>(infos).value;

        if (bool(attr & attr_t::iptables_in_gui)) this->out() << "\"#_iptables\\n\"\n";
        if (bool(attr & attr_t::advanced_in_gui)) this->out() << "\"#_advanced\\n\"\n";
        if (bool(attr & attr_t::hidden_in_gui))   this->out() << "\"#_hidden\\n\"\n";
        if (bool(attr & attr_t::hex_in_gui))      this->out() << "\"#_hex\\n\"\n";
        if (bool(attr & attr_t::password_in_gui)) this->out() << "\"#_password\\n\"\n";

        this->out() << "\"" << member_name << " = ";
        this->inherit().write_type(type, get_default(type, infos));
        this->out() << "\\n\\n\"\n\n";
    }


    struct exprio {
        const char * value;
        friend std::ostream & operator << (std::ostream & os, exprio const & eio) {
            return os << "\" << (" << eio.value << ") << \"";
        }
    };
    static exprio quoted2(cpp::expr e) { return {e.value}; }
    template<class T> static io_quoted2 quoted2(T const & s) { return s; }
    template<class T> static char const * quoted2(types::list<T> const &) { return ""; }


    template<class T>
    static char const * get_string(T const & s) { return s.c_str(); }


    static char const * stringize_integral(bool x) = delete;
    static char const * stringize_integral(types::integer_base) { return "0"; }
    static char const * stringize_integral(types::u16) { return "0"; }
    static char const * stringize_integral(types::u32) { return "0"; }
    static char const * stringize_integral(types::u64) { return "0"; }
    template<class T> static T const & stringize_integral(T const & x) { return x; }

    template<class Int, long min, long max, class T>
    static T stringize_integral(types::range<Int, min, max>)
    {
        static_assert(!min, "unspecified value but 'min' isn't 0");
        return {};
    }

    static exprio stringize_integral(cpp::expr e)
    {
        return {e.value};
    }


    static std::string stringize_bool(bool x)
    {
        return bool(x) ? "True" : "False";
    }

    static exprio stringize_bool(cpp::expr e)
    {
        return {e.value};
    }


    static io_prefix_lines comment(char const * s)
    {
        return io_prefix_lines{s, "\"# ", "\\n\"", 0};
    }

    template<class T, class Pack>
    void write_description(type_<T>, Pack const & pack)
    {
        auto& d = get_desc(pack);
        if (!d.empty()) {
            this->out() << this->inherit().comment(d.c_str());
        }
        else if constexpr (std::is_enum_v<T>) {
            apply_enumeration_for<T>(this->enums, [this](auto const & e) {
                if (e.desc) {
                    this->out() << this->inherit().comment(e.desc);
                }
            });
        }
    }


    template<class T>
    void write_type_info(type_<T>)
    {}

    void write_type_info(type_<std::chrono::hours>)
    { this->out() << this->inherit().comment("(is in hour)"); }

    void write_type_info(type_<std::chrono::minutes>)
    { this->out() << this->inherit().comment("(is in minute)"); }

    void write_type_info(type_<std::chrono::seconds>)
    { this->out() << this->inherit().comment("(is in second)"); }

    void write_type_info(type_<std::chrono::milliseconds>)
    { this->out() << this->inherit().comment("(is in millisecond)"); }

    template<class T, class Ratio>
    void write_type_info(type_<std::chrono::duration<T, Ratio>>)
    {
        auto prefixes = this->inherit().comment("");
        this->out() << prefixes.prefix << "(is in " << Ratio::num << "/" << Ratio::den << " second)" << prefixes.suffix << "\n";
    }

    template<class T, class Ratio, long min, long max>
    void write_type_info(type_<types::range<std::chrono::duration<T, Ratio>, min, max>>)
    { write_type_info(type_<std::chrono::duration<T, Ratio>>{}); }


    template<class T, class V>
    void write_value_(T const & name, V const & v, char const * prefix)
    {
        auto prefixes = this->inherit().comment("");
        this->out() << prefixes.prefix << "  " << name;
        if (v.desc) {
            this->out() << ": ";
            if (prefix) {
                this->out() << prefix << " ";
            }
            this->out() << v.desc;
        }
        else if (std::is_integral<T>::value) {
            this->out() << ": " << io_replace(v.name, '_', ' ');
        }
        this->out() << prefixes.suffix << "\n";
    }

    void write_desc_value(type_enumeration const & e, char const * prefix)
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
                this->write_value_((v.alias ? v.alias : v.name), v, prefix);
            }
            else {
                this->write_value_((is_autoinc ? d : (1 << d >> 1)), v, prefix);
            }
            ++d;
        }

        if (type_enumeration::flags == e.flag) {
            this->out() << this->inherit().comment("(note: values can be added (everyone: 1+2+4=7, mute: 0))");
        }
    }

    void write_desc_value(type_enumeration_set const & e, char const * prefix)
    {
        for (type_enumeration_set::Value const & v : e.values) {
            this->write_value_(v.val, v, prefix);
        }
    }

    template<class T, class Pack>
    void write_enumeration_value_description(type_<T>, Pack const & pack)
    {
        if constexpr (std::is_enum_v<T>) {
            apply_enumeration_for<T>(this->enums, [this, &pack](auto const & e) {
                this->write_desc_value(e, value_or<prefix_value>(pack, prefix_value{}).value);
                if (e.info) {
                    this->out() << this->inherit().comment(e.info);
                }
            });
        }
        else {
            static_assert(!is_convertible_v<Pack, prefix_value>, "prefix_value only with enums type");
        }
    }

    template<class T>
    void write_type(type_<bool>, T x)
    { this->out() << "boolean(default=" << stringize_bool(x) << ")"; }

    template<class T>
    void write_type(type_<std::string>, T const & s)
    { this->out() << "string(default='" << quoted2(s) << "')"; }

    template<class Int, class T>
    std::enable_if_t<
        std::is_base_of<types::integer_base, Int>::value
        or
        std::is_integral<Int>::value
    >
    write_type(type_<Int>, T i)
    {
        this->out() << "integer(";
        if (std::is_unsigned<Int>::value || std::is_base_of<types::unsigned_base, Int>::value) {
            this->out() << "min=0, ";
        }
        this->out() << "default=" << stringize_integral(i) << ")";
    }

    template<class Int, long min, long max, class T>
    void write_type(type_<types::range<Int, min, max>>, T i)
    { this->out() << "integer(min=" << min << ", max=" << max << ", default=" << stringize_integral(i) << ")"; }


    template<class T, class Ratio, class U>
    void write_type(type_<std::chrono::duration<T, Ratio>>, U i)
    { this->out() << "integer(min=0, default=" << stringize_integral(i) << ")"; }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_binary<N>>, T const & x)
    {
        this->out() << "string(min=" << N*2 << ", max=" << N*2 << ", default='"
          << io_hexkey{get_string(x), N} << "')";
    }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_string<N>>, T const & x)
    {
        this->out() << "string(max=" << N <<  ", default='" << quoted2(x) << "')";
    }

    template<class T>
    void write_type(type_<types::dirpath>, T const & x)
    {
        this->write_type(type_<typename types::dirpath::fixed_type>{}, x);
    }

    template<class T>
    void write_type(type_<types::ip_string>, T const & x)
    {
        this->out() << "ip_addr(default='" << quoted2(x) << "')";
    }

    template<class T, class L>
    void write_type(type_<types::list<T>>, L const & s)
    {
        if (is_empty(s)) {
            this->out() << "string_list(default=list())";
        }
        else {
            this->out() << "string_list(default=list('" << quoted2(s) << "'))";
        }
    }

    template<class T, class E>
    std::enable_if_t<std::is_enum_v<E>>
    write_type(type_<T>, E const & x)
    {
        static_assert(std::is_same<T, E>::value, "");
        using ll = long long;
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_value(e, ll{static_cast<std::underlying_type_t<E>>(x)});
        });
    }

    template<class T>
    void write_enum_value(type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            this->out() << "integer(min=0, max=" << e.max() << ", default=" << default_value << ")";
        }
        else if (e.is_string_parser) {
            this->out() << "option(";
            for (type_enumeration::Value const & v : e.values) {
                this->out() << "'" << (v.alias ? v.alias : v.name) << "', ";
            }
            this->out() << "default='" << e.values[default_value].name << "')";
        }
        else {
            this->out() << "option(";
            for (unsigned i = 0; i < e.count(); ++i) {
                this->out() << i << ", ";
            }
            this->out() << "default=" << default_value << ")";
        }
    }

    template<class T>
    void write_enum_value(type_enumeration_set const & e, T default_value)
    {
        this->out() << "option(";
        for (type_enumeration_set::Value const & v : e.values) {
            this->out() << v.val << ", ";
        }
        this->out() << "default=" << default_value << ")";
    }
};

}


template<class SpecWriter>
int app_write_python_spec(int ac, char const * const * av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h\n";
        return 1;
    }

    SpecWriter writer(av[1]);
    writer.evaluate();

    if (!writer.out_file_) {
        std::cerr << av[0] << ": " << av[1] << ": " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

}
