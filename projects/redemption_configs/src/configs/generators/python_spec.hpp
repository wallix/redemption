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

#ifndef REDEMPTION_SRC_UTILS_APPS_APP_WRITE_PYTHON_SPEC_HPP
#define REDEMPTION_SRC_UTILS_APPS_APP_WRITE_PYTHON_SPEC_HPP

#include "configs/specs/config_spec.hpp"
#include "configs/multi_filename_writer.hpp"

#include <algorithm>
#include <iostream>
#include <locale>
#include <vector>
#include <map>

#include <cerrno>
#include <cstring>


namespace python_spec_writer {

using namespace config_spec;

template<class Inherit>
struct PythonSpecWriterBase : ConfigSpecWriterBase<Inherit>
{
    std::vector<std::string> sections_ordered;

    void do_stop_section() {
        if (std::find(
            this->sections_ordered.begin(),
            this->sections_ordered.end(),
            this->section_name
        ) == this->sections_ordered.end()) {
            this->sections_ordered.emplace_back(std::move(this->section_name));
        }
    }

    template<class Pack>
    void member_impl(std::false_type, Pack const & ) {
    }

    template<class Pack>
    void member_impl(std::true_type, Pack const & pack) {
        this->out_ = &this->out_member_;

        this->write_if_convertible(pack, type_<desc>{});
        if (bool(pack & Attribute::iptables)) this->out() << "\"#_iptables\\n\"\n";
        if (bool(pack & Attribute::advanced)) this->out() << "\"#_advanced\\n\"\n";
        if (bool(pack & Attribute::hidden)) this->out() << "\"#_hidden\\n\"\n";
        if (bool(pack & Attribute::hex)) this->out() << "\"#_hex\\n\"\n";
        if (bool(pack & Attribute::password)) this->out() << "\"#_password\\n\"\n";
        this->out() << "\"" << static_cast<char const *>(pack) << " = ";
        this->inherit().write_type(this->get_type(this->has_user_type(pack), pack), this->inherit().get_default(pack, pack));
        this->out() << "\\n\\n\"\n\n";
    }


    template<class T>
    std::true_type has_user_type(ref<user_type<T>>)
    { return {}; }

    std::false_type has_user_type(...)
    { return {}; }

    template<class T>
    type_<T> get_type(std::false_type, ref<type_<T>>)
    { return {}; }

    template<class T>
    type_<T> get_type(std::true_type, ref<user_type<T>>)
    { return {}; }


    template<class T, class U>
    U const & get_default(ref<type_<T>>, ref<default_<U>> const & d)
    { return d.x.value; }

    template<class T>
    T const & get_default(ref<type_<T>>, ...)
    { static T r; return r; }


    template<class T>
    T const & get_value(T const & x)
    { return x; }

    struct macroio {
        const char * name;
        friend std::ostream & operator << (std::ostream & os, macroio const & mio) {
            return os << "\" " << mio.name << " \"";
        }
    };
    macroio get_value(macro const & m) { return {m.name}; }
    const char * get_value(null_fill) { return ""; }
    uint32_t get_value(uint32_) { return 0; }
    uint64_t get_value(uint64_) { return 0; }


    template<class T>
    char const * get_cstr(T const & s)
    { return s.c_str(); }

    char const * get_cstr(char const * s)
    { return s; }


    template<class... Ts>
    void member(Ts const & ... args) {
        MK_PACK(Ts) pack{args...};
        this->member_impl(typename std::is_convertible<Pack, ref<Attribute>>::type(), pack);
    }

    void write(desc x) {
        char const * s = x.value;
        while (*s) {
            char const * p = s;
            while (*p && *p != '\n') {
                ++p;
            }
            this->tab();
            this->out() << "\"# ";

            for (; s != p; ++s) {
                if (*s == '\\' || *s == '"') {
                    this->out() << '\\';
                }
                this->out() << *s;
            }
            this->out() << "\\n\"\n";

            if (*s == '\n') {
                ++s;
            }
        }
        this->out() << "\n";
    }


    template<class T>
    void write_type(type_<bool>, T x) {
        this->out() << "boolean(default=" << (bool(x) ? "True" : "False") << ")";
    }

    template<class T>
    void write_type(type_<std::string>, T const & s) {
        this->out() << "string(default='" << s << "')";
    }

    template<class T>
    void write_type(type_<uint32_>, T i) {
        this->out() << "integer(min=0, default=" << this->inherit().get_value(i) << ")";
    }

    template<class T>
    void write_type(type_<uint64_>, T i) {
        this->out() << "integer(min=0, default=" << this->inherit().get_value(i) << ")";
    }

    template<class T, class U>
    typename std::enable_if<std::is_integral<T>::value>::type
    write_type(type_<T>, U const & x) {
        this->out() << "integer(";
        if (std::is_unsigned<T>::value) {
            this->out() << "min=0, ";
        }
        this->out() << "default=" << this->inherit().get_value(x) << ")";
    }

    template<class T, class U>
    void write_enum(std::true_type, type_<T>, U const & x) {
        auto const & val = config_spec::enum_to_option(this->inherit().get_value(x));
        constexpr bool is_str
         = std::is_convertible<decltype(val), char const *>::value
        || std::is_convertible<decltype(val), std::string>::value;

        this->out() << "option(";
        for (auto s : config_spec::enum_option<T>::value) {
            if (is_str) {
                this->out() << "'";
            }
            this->out() << s;
            if (is_str) {
                this->out() << "'";
            }
            this->out() << ", ";
        }

        this->out() << "default=";
        if (is_str) {
            this->out() << "'";
        }
        this->out() << val;
        if (is_str) {
            this->out() << "'";
        }
        this->out() << ")";
    }

    template<class T, class U>
    decltype(T::NB) write_enum(std::false_type, type_<T>, U const & x) {
        this->out() << "integer(min=0, max=" << (underlying_cast(T::NB)-1) << ", default=" << this->inherit().get_value(x) << ")";
        return {};
    }

    template<class T, class U>
    decltype(T::FULL) write_enum(std::false_type, type_<T>, U const & x) {
        this->out() << "integer(min=0, max=" << T::FULL << ", default=" << this->inherit().get_value(x) << ")";
        return {};
    }

    template<class T, class U>
    typename std::enable_if<std::is_enum<T>::value>::type
    write_type(type_<T> t, U const & x) {
        this->write_enum(typename config_spec::enum_option<T>::type(), t, x);
    }

    template<class T, T Min, T Max, T Default, class U>
    void write_type(type_<Range<T, Min, Max, Default>>, U const & i) {
        this->out() << "integer(min=" << Min << ", max=" << Max << ", default=" << this->inherit().get_value(i) << ")";
    }

    template<class T, T Min, T Max, T Default, class U>
    void write_type(type_<SelectRange<T, Min, Max, Default>>, U const & i) {
        this->out() << "option(";
        for (auto i = Min; i <=  Max; ++i) {
            this->out() << i << ", ";
        }
        this->out() << "default=" << this->inherit().get_value(i) << ")";
    }

    template<std::size_t N, class T>
    void write_type(type_<StaticKeyString<N>>, T const & x) {
        this->out() << "string(min=" << N*2 << ", max=" << N*2 << ", default='";
        this->write_key(this->get_cstr(this->inherit().get_value(x)), N);
        this->out() << "')";
    }

    template<std::size_t N, class Copier, class T>
    void write_type(type_<StaticStringBase<N, Copier>>, T const & x) {
        this->out() << "string(max=" << N-1 <<  ", default='" << this->inherit().get_value(x) << "')";
    }

    template<class T>
    void write_type(type_<StaticIpString>, T const & x) {
        this->out() << "ip_addr(default='" << this->inherit().get_value(x) << "')";
    }

    template<class T>
    void write_type(type_<StringList>, T const & s) {
        this->out() << "string_list(default=list('" << s << "'))";
    }

    void write_type(type_<StringList>, StringList) {
        this->out() << "string_list(default=list())";
    }
};


template<class SpecWriter>
void write_spec(std::ostream & os, SpecWriter & writer) {
    os << "\"## Config file for RDP proxy.\\n\\n\\n\"\n" ;
    for (auto & section_name : writer.sections_ordered) {
        auto body = writer.sections_member.find(section_name)->second;
        if (std::none_of(begin(body), end(body), [](int c){return std::isblank(c);} )) {
            continue;
        }
        if (!section_name.empty()) {
            os << "\"[" << section_name << "]\\n\\n\"\n\n";
        }
        os << body;
    }
}

}


template<class SpecWriter>
int app_write_python_spec(int ac, char const ** av)
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


template<class SpecWriter>
int app_write_python_spec(int ac, char ** av)
{
    return app_write_python_spec<SpecWriter>(ac, const_cast<char const **>(av));
}

#endif
