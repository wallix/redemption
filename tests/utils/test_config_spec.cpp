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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestBase64
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "config_spec.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <map>


namespace config_writer {

using namespace config_spec;

using config_spec::link;

struct ConfigCppWriter {
    std::string section_name;
    unsigned depth = 0;
    std::ostringstream out_member_;
    std::ostringstream out_body_ctor_;
    std::ostream * out_;

    std::map<std::string, std::string> sections;
    std::string body_ctor;

    std::ostream & out() { return *this->out_; }

    void tab() {
        this->out() << std::setw(this->depth*4+4) << " ";
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
        this->sections[std::move(this->section_name)] += this->out_member_.str();
        this->body_ctor += this->out_body_ctor_.str();
        this->out_member_.str("");
        this->out_body_ctor_.str("");
    }

    void sep() {
        this->out_member_ << "\n";
        this->out_body_ctor_ << "\n";
    }

    template<class T>
    struct ref
    {
        T const & x;
        operator T const & () const { return x; }
    };

    template<class... Ts>
    void field(Ts const & ... args) {
        struct Pack : ref<Ts>... {
            Pack(Ts const &... x)
            : ref<Ts>{x}...
            {}
        } pack{args...};

        std::string varname = static_cast<char const *>(pack);

        this->out_ = &this->out_member_;

        this->write_if_convertible(pack, type_<todo>{});
        this->write_if_convertible(pack, type_<info>{});
        this->write_if_convertible(pack, type_<desc>{});
        this->tab();
        this->write_type(pack);
        this->out() << " " << varname;
        void(std::initializer_list<int>{
            (this->write_assignable_default(pack, args), 1)...
        });
        this->out() << ";\n";

        this->out_ = &this->out_body_ctor_;

        void(std::initializer_list<int>{
            (this->write_body_constructor(pack, varname, args), 1)...
        });
    }


    template<class T>
    void write(T const & r) { this->out() << r; }
    void write(const char * s) { this->out() << '"' << s << '"'; }
    void write(expr x) { this->out() << x.value; }
    void write(null_fill x) { this->out() << "null_fill()"; }

    void write(todo x) { this->tab(); this->out() << "TODO(\"" << x.value << "\")\n"; }
    void write(info x) { this->write(desc{x.value}); }
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
            this->out() << "// ";
            this->out().write(s, p-s);
            s = p;
        }
        this->out() << "\n";
    }


    template<class T>
    using enable_if_basefield = typename std::enable_if<std::is_base_of<BaseField, T>::value>::type;

    template<class T, class U>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, std::string const & varname, default_<U> const & d)
    {
        this->out() << "        this->" << this->section_name << "." << varname << ".set(";
        this->write(d.value);
        this->out() << ");\n";
    }

    void write_upper(std::string const & s)
    {
        for (auto c : s) {
            this->out() << char(std::toupper(c));
        }
    }

    template<class T>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, std::string const & varname, link const & d)
    {
        this->out() << "        this->to_send_set.insert(AUTHID_";
        this->write_upper(varname);
        this->out() << ");\n";
    }

    template<class T>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, std::string const & varname, attach const & d)
    {
        this->out() << "        this->" << this->section_name << "." << varname << ".attach_ini(this, AUTHID_";
        this->write_upper(varname);
        this->out() << ");\n";
    }

    template<class T>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, std::string const & varname, ask const & d)
    { this->out() << "        this->" << this->section_name << "." << varname << ".ask();\n"; }

    template<class T>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, std::string const & varname, use const & d)
    { this->out() << "        this->" << this->section_name << "." << varname << ".use();\n"; }

    template<class T, class U>
    void write_body_constructor(ref<type_<T>>, std::string const &, U const &)
    { }


    template<class T, class U>
    typename std::enable_if<!std::is_base_of<BaseField, T>::value>::type
    write_assignable_default(ref<type_<T>>, default_<U> const & d)
    { this->out() << " = "; this->write(d.value); }

    template<class T, class U>
    void write_assignable_default(ref<type_<T>>, U const &)
    { }


    template<class Pack, class To>
    typename std::enable_if<std::is_convertible<Pack, To>::value>::type
    write_if_convertible(Pack const & x, type_<To>)
    { this->write(static_cast<To const &>(x)); }

    template<class Pack, class To>
    void write_if_convertible(Pack const &, To)
    { }


    void write_type(type_<bool_>) { this->out() << "bool"; }
    void write_type(type_<int_>) { this->out() << "int"; }
    void write_type(type_<uint32_>) { this->out() << "uint32_t"; }
    void write_type(type_<uint64_>) { this->out() << "uint64_t"; }
    void write_type(type_<uint_>) { this->out() << "unsigned"; }
    void write_type(type_<std::string>) { this->out() << "std::string"; }
    void write_type(type_<StringField>) { this->out() << "StringField"; }
    void write_type(type_<UnsignedField>) { this->out() << "UnsignedField"; }
    void write_type(type_<SignedField>) { this->out() << "SignedField"; }
    void write_type(type_<BoolField>) { this->out() << "BoolField"; }
    void write_type(type_<SetField>) { this->out() << "SetField"; }
    void write_type(type_<RedirectionInfo>) { this->out() << "RedirectionInfo"; }
    void write_type(type_<Theme>) { this->out() << "Theme"; }
    void write_type(type_<Font>) { this->out() << "Font"; }
    void write_type(type_<IniAccount>) { this->out() << "IniAccount"; }
    void write_type(type_<Level>) { this->out() << "Level"; }
    void write_type(type_<StaticIpString>) { this->out() << "StaticIpString"; }

    template<std::size_t N>
    void write_type(ref<type_<StaticString<N>>>)
    { this->out() << "StaticString<" << N << ">"; }

    template<std::size_t N>
    void write_type(ref<type_<StaticNilString<N>>>)
    { this->out() << "StaticNilString<" << N << ">"; }

    template<std::size_t N>
    void write_type(ref<type_<StaticKeyString<N>>>)
    { this->out() << "StaticKeyString<" << N << ">"; }

    template<std::size_t N>
    void write_type(ref<type_<StaticPath<N>>>)
    { this->out() << "StaticPath<" << N << ">"; }
};

}

BOOST_AUTO_TEST_CASE(TestConfigSpec)
{
    config_writer::ConfigCppWriter writer;
    config_spec::writer_config_spec(writer);
    for (auto & body : writer.sections) {
        if (body.first.empty()) {
            std::cout << body.second << "\n";
        }
        else {
            std::cout
                << "    struct Inifile_" << body.first << " {\n"
                <<          body.second
                << "    } " << body.first << ";\n\n"
            ;
        }
    }
    std::cout << "\n";
    std::cout << writer.body_ctor;
}
