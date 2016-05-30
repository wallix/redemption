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
#include "configs/generators/utils/multi_filename_writer.hpp"
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"

#include <algorithm>
#include <iostream>
#include <locale>
#include <vector>
#include <map>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace python_spec_writer {

using namespace cfg_attributes;

template<class Inherit>
struct PythonSpecWriterBase : ConfigSpecWriterBase<Inherit>
{
    template<class... Ts>
    void do_member(Ts const & ... args)
    {
        pack_type<Ts...> pack{args...};
        this->template write_if_contains<spec::attr>(pack);
    }

    template<class Pack>
    void do_write(spec::attr attr, Pack const & pack)
    {
        auto type = pack_get<spec::type_>(pack);

        this->write_description(pack_contains<desc>(pack), type, pack);
        this->write_enumeration_value_description(pack_contains<prefix_value>(pack), type, pack);

        if (bool(attr & spec::attr::iptables)) this->out() << "\"#_iptables\\n\"\n";
        if (bool(attr & spec::attr::advanced)) this->out() << "\"#_advanced\\n\"\n";
        if (bool(attr & spec::attr::hidden))   this->out() << "\"#_hidden\\n\"\n";
        if (bool(attr & spec::attr::hex))      this->out() << "\"#_hex\\n\"\n";
        if (bool(attr & spec::attr::password)) this->out() << "\"#_password\\n\"\n";

        this->out() << "\"" << pack_get<spec::name>(pack) << " = ";
        this->inherit().write_type(type, get_default(type, pack));
        this->out() << "\\n\\n\"\n\n";
    }

    struct macroio {
        const char * name;
        friend std::ostream & operator << (std::ostream & os, macroio const & mio) {
            return os << "\" " << mio.name << " \"";
        }
    };
    macroio get_value(cpp::macro const & m) { return {m.name}; }
    int get_value(types::integer_base) { return 0; }
    int get_value(types::u32) { return 0; }
    int get_value(types::u64) { return 0; }
    template<class T> T const & get_value(T const & x) { return x; }
    template<class T> enable_if_enum_t<T, T const &> get_value(T const & x) { return x; }
    template<class Int, long min, long max, class T> T get_value(types::range<Int, min, max>)
    { static_assert(!min, "unspecified value but 'min' isn't 0"); return {}; }


    macroio quoted2(macroio m) { return m; }
    template<class T> io_quoted2 quoted2(T const & s) { return s; }
    template<class T> char const * quoted2(types::list<T> const &) { return ""; }


    io_prefix_lines comment(char const * s) {
        return io_prefix_lines{s, "\"# ", "\\n\"", 0};
    }


    template<class T, class Pack>
    void write_description(std::true_type, type_<T>, Pack const & pack)
    { this->out() << this->comment(pack_get<desc>(pack).value); }

    template<class T, class Pack>
    disable_if_enum_t<T>
    write_description(std::false_type, type_<T>, Pack const &)
    {}

    template<class T, class Pack>
    enable_if_enum_t<T>
    write_description(std::false_type, type_<T>, Pack const &)
    {
        apply_enumeration_for<T>(this->enums, [this](auto const & e) {
            if (e.desc) {
                this->out() << this->comment(e.desc);
            }
        });
    }


    template<class T, class V>
    void write_value_(T const & name, V const & v, char const * prefix)
    {
        this->out() << "\"#   " << name;
        if (v.desc) {
            this->out() << ": ";
            if (prefix) {
                this->out() << prefix << " ";
            }
            this->out() << v.desc << "\\n";
        }
        else if (std::is_integral<T>::value) {
            this->out() << ": " << io_replace(v.name, '_', ' ') << "\\n";
        }
        this->out() << "\"\n";
    }

    void write_desc_value(type_enumeration const & e, char const * prefix)
    {
        if (e.is_icase_parser) {
            if (std::none_of(begin(e.values), end(e.values), [](type_enumeration::Value const & v) {
                return v.desc;
            })) {
                return ;
            }
        }

        unsigned d = 0;
        bool const is_autoinc = e.flag == type_enumeration::autoincrement;
        for (type_enumeration::Value const & v : e.values) {
            if (e.is_icase_parser) {
                this->write_value_((v.alias ? v.alias : v.name), v, prefix);
            }
            else {
                this->write_value_((is_autoinc ? d : (1 << d >> 1)), v, prefix);
            }
            ++d;
        }
    }

    void write_desc_value(type_enumeration_set const & e, char const * prefix)
    {
        for (type_enumeration_set::Value const & v : e.values) {
            this->write_value_(v.val, v, prefix);
        }
    }

    template<class Pack>
    std::nullptr_t get_prefix(std::false_type, Pack const &)
    { return nullptr; }

    template<class Pack>
    char const * get_prefix(std::true_type, Pack const & pack)
    { return pack_get<prefix_value>(pack).value; }

    template<bool HasPrefix, class T, class Pack>
    enable_if_enum_t<T>
    write_enumeration_value_description(std::integral_constant<bool, HasPrefix>, type_<T>, Pack const & pack)
    {
        apply_enumeration_for<T>(this->enums, [this, &pack](auto const & e) {
            this->write_desc_value(e, this->get_prefix(pack_contains<prefix_value>(pack), pack));
            if (e.info) {
                this->out() << this->comment(e.info);
            }
        });
    }

    template<bool HasPrefix, class T, class Pack>
    void write_enumeration_value_description(std::integral_constant<bool, HasPrefix>, T, Pack const &)
    { static_assert(!HasPrefix, "prefix_value only with enums type"); }


    template<class T>
    void write_type(type_<bool>, T x)
    { this->out() << "boolean(default=" << (bool(x) ? "True" : "False") << ")"; }

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
        this->out() << "default=" << this->get_value(i) << ")";
    }

    template<class Int, long min, long max, class T>
    void write_type(type_<types::range<Int, min, max>>, T i)
    { this->out() << "integer(min=" << min << ", max=" << max << ", default=" << this->get_value(i) << ")"; }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_binary<N>>, T const & x)
    {
        this->out() << "string(min=" << N*2 << ", max=" << N*2 << ", default='"
          << io_hexkey{this->get_value(x), N} << "')";
    }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_string<N>>, T const & x)
    {
        this->out() << "string(max=" << N <<  ", default='"
          << quoted2(this->get_value(x)) << "')";
    }

    template<class T>
    void write_type(type_<types::path>, T const & x)
    { this->write_type(type_<typename types::path::fixed_type>{}, x); }

    template<class T>
    void write_type(type_<types::ip_string>, T const & x)
    {
        this->out() << "ip_addr(default='" << this->get_value(x) << "')";
    }

    template<class T, class L>
    void write_type(type_<types::list<T>>, L const & s)
    {
        if (is_empty(s)) {
            this->out() << "string_list(default=list())";
        }
        else {
            this->out() << "string_list(default=list('" << quoted2(this->get_value(s)) << "'))";
        }
    }

    template<class T>
    void write_enum_value(type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            this->out() << "integer(min=0, max=" << e.max() << ", default=" << default_value << ")";
        }
        else if (e.is_icase_parser) {
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

    template<class T, class E>
    enable_if_enum_t<T>
    write_type(type_<T> t, E const & x)
    {
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_value(e, static_cast<std::underlying_type_t<E>>(x));
        });
    }
};


template<class SpecWriter>
void write_spec(std::ostream & os, SpecWriter & writer)
{
    os << "\"## Config file for RDP proxy.\\n\\n\\n\"\n" ;
    for (auto & section_name : writer.sections_ordered) {
        auto & members = writer.sections.find(section_name)->second;
        if (members.empty()) {
            continue;
        }
        if (!section_name.empty()) {
            os << "\"[" << section_name << "]\\n\\n\"\n\n";
        }
        os << members;
    }
}

}


template<class SpecWriter>
int app_write_python_spec(int ac, char const * const * av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h\n";
        return 1;
    }

    SpecWriter writer;

    MultiFilenameWriter<SpecWriter> sw(writer);
    sw.then(av[1], &python_spec_writer::write_spec<SpecWriter>);
    if (sw.err) {
        std::cerr << av[0] << ": " << sw.filename << ": " << strerror(errno) << "\n";
        return sw.errnum;
    }
    return 0;
}

}
