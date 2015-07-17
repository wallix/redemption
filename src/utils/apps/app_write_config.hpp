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

#ifndef REDEMPTION_SRC_UTILS_APPS_APP_WRITE_CONFIG_HPP
#define REDEMPTION_SRC_UTILS_APPS_APP_WRITE_CONFIG_HPP

#include "config_spec.hpp"
#include "multi_filename_writer.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

#include <cerrno>
#include <cstring>


namespace config_writer {

using namespace config_spec;

using config_spec::link; // for ambiguousity

template<class Inherit>
struct ConfigCppWriterBase : ConfigSpecBase<Inherit> {
    std::ostringstream out_body_ctor_;
    std::ostringstream out_body_parser_;

    std::map<std::string, std::string> sections_parser;
    std::map<std::string, std::string> authids;

    void do_tab() {
        this->out() << std::setw(this->depth*4+4) << " ";
    }

    void do_stop_section() {
        this->sections_parser[std::move(this->section_name)] += this->out_body_parser_.str();
        this->out_body_parser_.str("");
    }

    void do_sep() {
        this->out_member_ << "\n";
        this->out_body_ctor_ << "\n";
    }

    template<class... Ts>
    void member(Ts const & ... args) {
        MK_PACK(Ts) pack{args...};

        std::string varname = get_name(pack);

        this->out_ = &this->out_member_;

        this->write_if_convertible(pack, type_<todo>{});
        this->write_if_convertible(pack, type_<info>{});
        this->write_if_convertible(pack, type_<desc>{});
        this->tab();
        this->inherit().write_type(pack);
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
            "            ::configs::parse(this->" << this->section_name << "." << varname << ", value);\n"
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
    void write(macro x) { this->out() << x.name; }
    void write(null_fill x) { this->out() << "null_fill()"; }

#define WRITE_ENUM(E) \
    void write(E x) { this->out() << "static_cast<" #E ">(" << underlying_cast(x) << ")"; }
    WRITE_ENUM(Level)
    WRITE_ENUM(Language)
    WRITE_ENUM(ColorDepth)
    WRITE_ENUM(CaptureFlags)
    WRITE_ENUM(KeyboardLogFlags)
    WRITE_ENUM(ClipboardLogFlags)
    WRITE_ENUM(ClipboardEncodingType)
#undef WRITE_ENUM

    void write(todo x) { this->tab(); this->out() << "TODO(\"" << x.value << "\")\n"; }
    void write(info x) { this->inherit().write(desc{x.value}); }
    void write(desc x) { this->write_comment("//", x.value); }

    template<class T, class Pack, class U>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, default_<U> const & d)
    {
        this->out() << "    this->" << this->section_name << "." << varname << ".set(";
        this->inherit().write(d.value);
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
        this->out() << "    this->to_send_set.insert(AUTHID_";
        this->write_authid(this->get_def_authid(pack, varname), this->get_str_authid(pack, varname));
        this->out() << ");\n";
    }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack & pack, std::string const & varname, attach const & d)
    {
        this->out() << "    this->" << this->section_name << "." << varname << ".attach_ini(this, AUTHID_";
        this->write_authid(this->get_def_authid(pack, varname), this->get_str_authid(pack, varname));
        this->out() << ");\n";
    }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, ask const & d)
    { this->out() << "    this->" << this->section_name << "." << varname << ".ask();\n"; }

    template<class T, class Pack>
    enable_if_basefield<T>
    write_body_constructor(ref<type_<T>>, Pack &, std::string const & varname, use const & d)
    { this->out() << "    this->" << this->section_name << "." << varname << ".use();\n"; }

    template<class T, class Pack, class U>
    void write_body_constructor(ref<type_<T>>, Pack &, std::string const &, U const &)
    { }

    template<class T, class U>
    void check_constructible(type_<T>, default_<U> const & d)
    { static_assert((void(type_<decltype(T(d.value))>{}), 1), "incompatible type"); }

    template<class T>
    void check_constructible(type_<T>, default_<macro> const &)
    {}

    template<class T, class U>
    disable_if_basefield<T>
    write_assignable_default(ref<type_<T>>, default_<U> const & d)
    {
        this->check_constructible(type_<T>{}, d);
        this->out() << "{";
        this->inherit().write(d.value);
        this->out() << "}";
    }

    template<std::size_t N, class U>
    void write_assignable_default(ref<type_<StaticKeyString<N>>>, default_<U> const & d)
    {
        this->check_constructible(type_<StaticKeyString<N>>{}, d);
        this->out() << "{\"";
        this->write_key(d.value, N, "\\x");
        this->out() << "\"}";
    }

    template<class T>
    void write_assignable_default(ref<type_<T>>, ...)
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
    WRITE_TYPE(ReadOnlyStringField)
    WRITE_TYPE(ClipboardLogFlagsField)
    WRITE_TYPE(ClipboardEncodingTypeField)
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
        this->inherit().write_type(type_<T>{});
        this->out() << ", " << Min << ", " << Max << ", " << Default << ">";
    }
};

}


template<class ConfigCppWriter>
void write_authid_hpp(std::ostream & out_authid, ConfigCppWriter & writer) {
    out_authid <<
      "enum authid_t {\n"
      "    AUTHID_UNKNOWN = 0,\n"
    ;
    for (auto & body : writer.authids) {
        out_authid <<
          "    AUTHID_" << body.first << ",\n"
          "#define STRAUTHID_" << body.first << " \"" << body.second << "\"\n"
        ;
    }
    out_authid <<
      "    MAX_AUTHID\n"
      "};\n"
      "constexpr char const * const authstr[] = {\n"
    ;
    for (auto & body : writer.authids) {
        out_authid << "    STRAUTHID_" << body.first << ", // AUTHID_" << body.first << "\n";
    }
    out_authid << "};\n\n";
}

template<class ConfigCppWriter>
void write_variable_configuration(std::ostream & out_varconf, ConfigCppWriter & writer) {
    out_varconf <<
        "#include \"font.hpp\"\n"
        "#include \"configs/types.hpp\"\n"
        "#include \"configs/includes.hpp\"\n\n"
        "namespace configs {\n\n"
        "struct VariablesConfiguration {\n"
        "    explicit VariablesConfiguration(char const * default_font_name)\n"
        "    : font(default_font_name)\n"
        "    {}\n\n"
    ;

    for (auto & body : writer.sections_member) {
        if (body.first.empty()) {
            out_varconf << body.second << "\n";
        }
        else {
            out_varconf <<
                "    struct Inifile_" << body.first << " {\n" <<
                         body.second <<
                "        Inifile_" << body.first << "() = default;\n"
                "    } " << body.first << ";\n\n"
            ;
        }
    }
    out_varconf << "};\n\n}\n";
}

template<class ConfigCppWriter>
void config_initialize(std::ostream & out_body, ConfigCppWriter & writer) {
    out_body <<
        "inline void Inifile::initialize() {\n" <<
        writer.out_body_ctor_.str() <<
        "}\n"
    ;
}

template<class ConfigCppWriter>
void write_config_set_value(std::ostream & out_set_value, ConfigCppWriter & writer) {
    out_set_value <<
        "inline void Inifile::set_value(const char * context, const char * key, const char * value) {\n"
        "    if (0) {}\n"
    ;
    for (auto & body : writer.sections_parser) {
        if (body.second.empty()) {
            continue;
        }
        out_set_value <<
            "    else if (0 == strcmp(context, \"" << body.first << "\")) {\n"
            "        if (0) {}\n" << body.second << "\n"
            "        else if (this->debug.config) {\n"
            "            LOG(LOG_ERR, \"unknown parameter %s in section [%s]\", key, context);\n"
            "        }\n"
            "    }\n"
        ;
    }
    out_set_value <<
        "    else if (this->debug.config) {\n"
        "        LOG(LOG_ERR, \"unknown section [%s]\", context);\n"
        "    }\n"
        "}\n"
    ;
}


template<class ConfigCppWriter>
int write_config_cpp_writer(int ac, char ** av)
{
    if (ac < 5) {
        std::cerr << av[0] << " out-authid.h out-variables_configuration.h out-config_initialize.cpp out-config_set_value.cpp";
        return 1;
    }

    ConfigCppWriter writer;

    MultiFilenameWriter<ConfigCppWriter> sw(writer);
    sw.then(av[1], &write_authid_hpp<ConfigCppWriter>)
      .then(av[2], &write_variable_configuration<ConfigCppWriter>)
      .then(av[3], &config_initialize<ConfigCppWriter>)
      .then(av[4], &write_config_set_value<ConfigCppWriter>)
    ;
    if (sw.err) {
        std::cerr << av[0] << ": " << sw.filename << ": " << strerror(errno) << "\n";
        return sw.errnum;
    }
    return 0;
}

#endif
