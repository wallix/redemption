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

#include "config_spec.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <locale>
#include <vector>
#include <map>

#include <cerrno>
#include <cstring>


namespace config_writer {

using namespace config_spec;

using config_spec::link; // for ambiguousity

struct ConfigCppWriter {
    std::string section_name;
    unsigned depth = 0;
    std::ostringstream out_member_;
    std::ostream * out_;

    std::map<std::string, std::string> sections_member;
    std::vector<std::string> sections_ordered;

    std::ostream & out() {
        return *this->out_;
    }

    void tab() {
    }

    void start_section(std::string name) {
        this->out_ = &this->out_member_;
        this->section_name = std::move(name);
        if (!this->section_name.empty()) {
            ++this->depth;
        }
    }
    void stop_section() {
        this->out_ = &this->out_member_;
        if (!this->section_name.empty()) {
            --this->depth;
        }
        this->sections_member[this->section_name] += this->out_member_.str();
        this->sections_ordered.emplace_back(std::move(this->section_name));
        this->out_member_.str("");
    }

    void sep() {
    }

    template<class T>
    struct ref
    {
        T const & x;
        operator T const & () const { return x; }
    };

    template<class Pack>
    void member_impl(std::false_type, Pack const & ) {
    }

    template<class Pack>
    void member_impl(std::true_type, Pack const & pack) {
        this->out_ = &this->out_member_;

        this->write_if_convertible(pack, type_<desc>{});
        if (bool(pack & Attribute::iptables)) this->out() << "#_iptables\n";
        if (bool(pack & Attribute::advanced)) this->out() << "#_advanced\n";
        if (bool(pack & Attribute::hidden)) this->out() << "#_hidden\n";
        if (bool(pack & Attribute::hex)) this->out() << "#_hex\n";
        this->out() << static_cast<char const *>(pack) << " = ";
        this->write_type(this->get_type(this->has_user_type(pack), pack), this->get_default(pack, pack));
        this->out() << "\n";
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
    typename std::enable_if<std::is_base_of<BaseField, T>::value, decltype(std::declval<T>().get())>::type
    get_value(T const & x)
    { return x.get(); }

    template<class T>
    typename std::enable_if<!std::is_base_of<BaseField, T>::value, T const &>::type
    get_value(T const & d)
    { return d; }

    const char * get_value(macro const & m) { return m.value; }
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
        struct Pack : ref<Ts>... {
            explicit Pack(Ts const &... x)
            : ref<Ts>{x}...
            {}
        } pack{args...};
        this->member_impl(typename std::is_convertible<Pack, ref<Attribute>>::type(), pack);
    }

    void write(desc x) {
        auto s = x.value;
        auto p = s;
        while (*s) {
            while (*p && *p != '\n') {
                ++p;
            }
            if (*p == '\n') {
                ++p;
            }
            this->tab();
            this->out() << "# ";
            this->out().write(s, p-s);
            s = p;
        }
        this->out() << "\n";
    }


    template<class Pack, class To>
    typename std::enable_if<std::is_convertible<Pack, To>::value>::type
    write_if_convertible(Pack const & x, type_<To>)
    { this->write(static_cast<To const &>(x)); }

    template<class Pack, class To>
    void write_if_convertible(Pack const &, To)
    { }


    template<class T>
    void write_type(type_<bool>, T x) {
        this->out() << "boolean(default=" << bool(x) << ")\n";
    }

    template<class T>
    void write_type(type_<std::string>, T const & s) {
        this->out() << "string(default='" << s << "')\n";
    }

    template<class T>
    void write_type(type_<uint32_>, T i) {
        this->out() << "integer(min=0, default=" << this->get_value(i) << ")\n";
    }

    template<class T>
    void write_type(type_<uint64_>, T i) {
        this->out() << "integer(min=0, default=" << this->get_value(i) << ")\n";
    }

    template<class T, class U>
    typename std::enable_if<std::is_integral<T>::value>::type
    write_type(type_<T>, U const & x) {
        this->out() << "integer(";
        if (std::is_unsigned<T>::value) {
            this->out() << "min=0, ";
        }
        this->out() << "default=" << this->get_value(x) << ")\n";
    }

    template<class T, class U>
    void write_enum(std::true_type, type_<T>, U const & x) {
        auto const & val = config_spec::enum_to_option(this->get_value(x));
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
        this->out() << ")\n";
    }

    template<class T, class U>
    decltype(T::NB) write_enum(std::false_type, type_<T>, U const & x) {
        this->out() << "integer(min=0, max=" << (underlying_cast(T::NB)-1) << ", default=" << this->get_value(x) << ")\n";
        return {};
    }

    template<class T, class U>
    decltype(T::FULL) write_enum(std::false_type, type_<T>, U const & x) {
        this->out() << "integer(min=0, max=" << T::FULL << ", default=" << this->get_value(x) << ")\n";
        return {};
    }

    template<class T, class U>
    typename std::enable_if<std::is_enum<T>::value>::type
    write_type(type_<T> t, U const & x) {
        this->write_enum(typename config_spec::enum_option<T>::type(), t, x);
    }

    template<class T, class U>
    typename std::enable_if<std::is_base_of<BaseField, T>::value>::type
    write_type(type_<T>, U const & x) {
        using type = type_<typename std::remove_cv<typename std::remove_reference<
            decltype(std::declval<T>().get())
        >::type>::type>;
        this->write_type(type{}, this->get_value(x));
    }

    template<class T, T Min, T Max, T Default, class U>
    void write_type(type_<Range<T, Min, Max, Default>>, U const & i) {
        this->out() << "integer(min=" << Min << ", max=" << Max << ", default=" << this->get_value(i) << ")\n";
    }

    template<std::size_t N, class T>
    void write_type(type_<StaticKeyString<N>>, T const & x) {
        this->out() << "string(min=" << N*2 << ", max=" << N*2 << ", default='";
        const char * k = this->get_cstr(this->get_value(x));
        int c;
        for (const char * e = k + N; k != e; ++k) {
            c = (*k >> 4);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
            c = (*k & 0xf);
            c += (c > 9) ? 'A' - 10 : '0';
            this->out() << char(c);
        }
        this->out() << "')\n";
    }

    template<std::size_t N, class Copier, bool NullableString, class T>
    void write_type(type_<StaticStringBase<N, Copier, NullableString>>, T const & x) {
        this->out() << "string(max=" << N-1 <<  ", default='" << this->get_value(x) << "')\n";
    }

    template<class T>
    void write_type(type_<StaticIpString>, T const & x) {
        this->out() << "ip_addr(default='" << this->get_value(x) << "')\n";
    }
};

}


void write_spec(std::ostream & os, config_writer::ConfigCppWriter & writer) {
    os << "## Config file for RDP proxy.\n\n\n";
    for (auto & section_name : writer.sections_ordered) {
        auto body = writer.sections_member.find(section_name)->second;
        if (std::none_of(begin(body), end(body), std::isblank)) {
            continue;
        }
        if (!section_name.empty()) {
            os << "[" << section_name << "]\n\n";
        }
        os << body;
    }
}

struct SuitableWrite {
    explicit SuitableWrite(config_writer::ConfigCppWriter & writer, int errnum = 1)
    : writer(writer)
    , errnum(errnum)
    {}

    template<class Fn>
    SuitableWrite & then(const char * new_filename, Fn fn) {
        if (!err) {
            const char * filename = this->filename;
            if (new_filename && *new_filename && strcmp(this->filename, new_filename)) {
                this->of.close();
                this->of.open(new_filename);
                filename = new_filename;
            }

            fn(this->of, this->writer);
            this->err = !bool(this->of);
            this->filename = filename;
            ++this->errnum;
        }
        return *this;
    }

    char const * filename = "";
    config_writer::ConfigCppWriter & writer;
    std::ofstream of;
    int errnum;
    int err = 0;
};

int main(int ac, char ** av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h";
        return 1;
    }

    config_writer::ConfigCppWriter writer;
    config_spec::writer_config_spec(writer);

    SuitableWrite sw(writer);
    sw.then(av[1], &write_spec);
    if (sw.err) {
        std::cerr << av[0] << ": " << sw.filename << ": " << strerror(errno) << "\n";
        return sw.errnum;
    }
    return 0;
}
