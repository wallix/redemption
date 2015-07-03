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
    std::ostringstream out_body_parser_;
    std::ostream * out_;

    std::map<std::string, std::string> sections_member;
    std::map<std::string, std::string> sections_parser;
    std::map<std::string, std::string> authids;

    std::ostream & out() {
        return *this->out_;
    }

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
        this->sections_member[this->section_name] += this->out_member_.str();
        this->sections_parser[std::move(this->section_name)] += this->out_body_parser_.str();
        this->out_member_.str("");
        this->out_body_parser_.str("");
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

        std::string varname = get_name(pack);

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
        this->out() << ";";
        if (std::is_convertible<Pack, attach>::value) {
            this->out() << "  // AUTHID_";
            for (auto & c : this->get_def_authid(pack, varname)) {
                this->out() << char(std::toupper(c));
            }
        }
        this->out() << "\n";

        this->out_ = &this->out_body_ctor_;

        void(std::initializer_list<int>{
            (this->write_body_constructor(pack, pack, varname, args), 1)...
        });

        this->out_ = &this->out_body_parser_;

        this->write_parser(this->get_attribute(pack), pack, varname);
    }


    void write_parser(Attribute a, const char * name, std::string const & varname)
    {
        if (bool(a)) {
            this->out() << "        else if (0 == strcmp(key, \"" << name << "\")) {\n"
            "            config_detail::parse(ini." << this->section_name << "." << varname << ", value);\n"
            "        }\n";
        }
    }


    template<class Pack>
    std::string get_name(Pack const & pack)
    {
        struct RealName { static std::string impl(real_name x) { return x.name; } };
        struct Name { static std::string impl(Pack const & pack) { return static_cast<char const *>(pack); } };
        return std::conditional<
            std::is_convertible<Pack, real_name>::value,
            RealName,
            Name
        >::type::impl(pack);
    }

    template<class Pack>
    Attribute get_attribute(Pack const & pack)
    {
        struct ToAttribute { static Attribute impl(Attribute a) { return a; } };
        struct NoAttribute { static constexpr Attribute impl(...) { return Attribute::none; } };
        return std::conditional<
            std::is_convertible<Pack, ref<Attribute>>::value,
            ToAttribute,
            NoAttribute
        >::type::impl(pack);
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

    template<class T, class Pack, class U>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, default_<U> const & d)
    {
        this->out() << "        this->" << this->section_name << "." << varname << ".set(";
        this->write(d.value);
        this->out() << ");\n";
    }

    void write_authid(std::string authid, std::string s_authid)
    {
        for (auto & c : authid) {
            c = char(std::toupper(c));
        }
        this->out() << authid;
        this->authids.emplace(std::move(authid), std::move(s_authid));
    }

    std::string get_def_authid(ref<def_authid> r, std::string const & varname)
    { return r.x.name; }

    template<class T>
    std::string const & get_def_authid(ref<type_<T>>, std::string const & varname)
    { return varname; }

    const char * get_str_authid(ref<str_authid> r, std::string const & varname)
    { return r.x.name; }

    template<class T>
    std::string const & get_str_authid(ref<type_<T>>, std::string const & varname)
    { return varname; }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack & pack, std::string const & varname, link const & d)
    {
        this->out() << "        this->to_send_set.insert(AUTHID_";
        this->write_authid(this->get_def_authid(pack, varname), this->get_str_authid(pack, varname));
        this->out() << ");\n";
    }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack & pack, std::string const & varname, attach const & d)
    {
        this->out() << "        this->" << this->section_name << "." << varname << ".attach_ini(this, AUTHID_";
        this->write_authid(this->get_def_authid(pack, varname), this->get_str_authid(pack, varname));
        this->out() << ");\n";
    }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, ask const & d)
    { this->out() << "        this->" << this->section_name << "." << varname << ".ask();\n"; }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, use const & d)
    { this->out() << "        this->" << this->section_name << "." << varname << ".use();\n"; }

    template<class T, class Pack, class U>
    void write_body_constructor(ref<type_<T>>, Pack &, std::string const &, U const &)
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


#define WRITE_TYPE(T) void write_type(type_<T>) { this->out() << #T; }
    WRITE_TYPE(bool)
    WRITE_TYPE(int)
    WRITE_TYPE(unsigned)
    WRITE_TYPE(std::string)
    WRITE_TYPE(BoolField)
    WRITE_TYPE(UnsignedField)
    WRITE_TYPE(SignedField)
    WRITE_TYPE(StringField)
    WRITE_TYPE(RedirectionInfo)
    WRITE_TYPE(Theme)
    WRITE_TYPE(Font)
    WRITE_TYPE(IniAccounts)
    WRITE_TYPE(StaticIpString)
    WRITE_TYPE(Level)
    WRITE_TYPE(LevelField)
    WRITE_TYPE(LanguageField)
    WRITE_TYPE(CaptureFlags)
    WRITE_TYPE(ColorDepth)
    WRITE_TYPE(KeyboardLogFlagsField)
#undef WRITE_TYPE

#define WRITE_STATIC_STRING(T)        \
    template<std::size_t N>           \
    void write_type(ref<type_<T<N>>>) \
    { this->out() << #T "<" << N << ">"; }

    WRITE_STATIC_STRING(StaticString)
    WRITE_STATIC_STRING(StaticNilString)
    WRITE_STATIC_STRING(StaticKeyString)
    WRITE_STATIC_STRING(StaticPath)
#undef WRITE_STATIC_STRING

    void write_type(type_<uint32_>) { this->out() << "uint32_t"; }
    void write_type(type_<uint64_>) { this->out() << "uint64_t"; }

    template<class T, T Min, T Max, T Default>
    void write_type(ref<type_<Range<T, Min, Max, Default>>>) {
        this->out() << "Range<";
        this->write_type(type_<T>{});
        this->out() << ", " << Min << ", " << Max << ", " << Default << ">";
    }
};

}

BOOST_AUTO_TEST_CASE(TestConfigSpec)
{
    config_writer::ConfigCppWriter writer;
    config_spec::writer_config_spec(writer);
    std::cout <<
      "enum authid_t {\n"
      "    AUTHID_UNKNOWN = 0,\n"
    ;
    for (auto & body : writer.authids) {
        std::cout <<
          "    AUTHID_" << body.first << ",\n"
          "#define STRAUTHID_" << body.first << " \"" << body.second << "\"\n"
        ;
    }
    std::cout <<
      "    MAX_AUTHID\n"
      "};\n"
      "constexpr char * const authstr[] = {\n"
    ;
    for (auto & body : writer.authids) {
        std::cout << "    STRAUTHID_" << body.first << ", // AUTHID_" << body.first << "\n";
    }
    std::cout << "};\n\n";
    for (auto & body : writer.sections_member) {
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
    std::cout <<
      "\n" <<
      writer.out_body_ctor_.str() <<
      "void set_value(const char * context, const char * key, const char * value) {\n"
      "    if (0) {}\n"
    ;
    for (auto & body : writer.sections_parser) {
        if (body.second.empty()) {
            continue;
        }
        std::cout <<
            "    else if (0 == strcmp(context, \"" << body.first << "\")) {\n"
            "        if (0) {}\n" << body.second << "\n"
            "        else if (ini.debug.config) {\n"
            "            LOG(LOG_ERR, \"unknown parameter %s in section [%s]\", key, context);\n"
            "        }\n"
            "    }\n"
        ;
    }
    std::cout <<
        "    else if (ini.debug.config) {\n"
        "        LOG(LOG_ERR, \"unknown section [%s]\", context);\n"
        "    }\n"
        "}"
    ;
}
