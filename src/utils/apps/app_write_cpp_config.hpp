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

#ifndef REDEMPTION_SRC_UTILS_APPS_APP_WRITE_CPP_CONFIG_HPP
#define REDEMPTION_SRC_UTILS_APPS_APP_WRITE_CPP_CONFIG_HPP

#include "config_spec.hpp"
#include "multi_filename_writer.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

#include <cerrno>
#include <cstring>


namespace cpp_config_writer {

using namespace config_spec;

template<class Inherit>
struct CppConfigWriterBase : ConfigSpecWriterBase<Inherit> {
    std::ostringstream out_body_ctor_;
    std::ostringstream out_body_parser_;

    std::map<std::string, std::string> sections_parser;
    std::vector<std::pair<std::string, std::string>> authids;
    std::vector<std::string> variables;
    std::vector<std::string> variables_acl;
    unsigned index_authid = 0;

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
        auto const properties = this->default_or_get<PropertyFieldFlags>(PropertyFieldFlags::none, pack);
        auto varname_with_section = this->section_name.empty() ? varname : this->section_name + "::" + varname;
        if (bool(/*PropertyFieldFlags::read & */properties)) {
            this->variables_acl.emplace_back(varname_with_section);
        }
        this->variables.emplace_back(varname_with_section);

        this->out_ = &this->out_member_;

        this->write_if_convertible(pack, type_<todo>{});
        this->write_if_convertible(pack, type_<info>{});
        this->write_if_convertible(pack, type_<desc>{});
        if (bool(properties)) {
            this->tab();
            this->out() << "// AUTHID_";
            auto str = this->get_def_authid(pack, varname);
            for (auto & c : str) {
                c = char(std::toupper(c));
            }
            this->out() << str << "\n";
            this->authids.emplace_back(str, this->get_str_authid(pack, varname));
        }
        this->tab(); this->out() << "struct " << varname << " {\n";
        this->tab(); this->out() << "    static constexpr ::configs::VariableProperties properties() {\n";
        this->tab(); this->out() << "        return ";
        if (PropertyFieldFlags::none == properties) {
            this->out() << "::configs::VariableProperties::none";
        }
        if (PropertyFieldFlags::read == properties) {
            this->out() << "::configs::VariableProperties::read";
        }
        if (PropertyFieldFlags::write == properties) {
            this->out() << "::configs::VariableProperties::write";
        }
        if ((PropertyFieldFlags::read | PropertyFieldFlags::write) == properties) {
            this->out() << "::configs::VariableProperties::read | ::configs::VariableProperties::write";
        }
        this->out() << ";\n";
        this->tab(); this->out() << "    }\n";
        if (bool(properties)) {
            this->tab();
            this->out() << "    static constexpr unsigned index() { return " << this->index_authid++ << "; }\n";
        }
        this->tab();
        this->out() << "    using type = ";
        this->inherit().write_type(pack);
        this->out() << ";\n";
        this->tab();
        if (std::is_convertible<Pack, type_<Font>>::value) {
            this->out()
              << "    font(char const * filename) : value(filename) {}\n"
              << "    type value;\n"
            ;
        }
        else {
            this->out() << "    type value{";
            void(std::initializer_list<int>{
                (this->write_assignable_default(pack, args), 1)...
            });
            this->out() << "};\n";
        }
        this->tab();
        this->out() << "};\n";


        this->out_ = &this->out_body_ctor_;

        if (PropertyFieldFlags::read == properties) {
            this->out() << "    this->ask<cfg::" << varname_with_section << ">();\n";
        }
        if (PropertyFieldFlags::write == properties) {
            this->out() << "    this->insert_index<cfg::" << varname_with_section << ">(std::true_type());\n";
        }

        this->out_ = &this->out_body_parser_;

        this->write_parser(this->get_attribute(pack), pack, varname_with_section);
    }

    template<class T> T const & default_or_get(T const &, T const & x) { return x; }
    template<class T> T const & default_or_get(T const & default_, ...) { return default_; }

    void write_parser(Attribute a, const char * name, std::string const & varname)
    {
        if (bool(a)) {
            this->out() << "        else if (0 == strcmp(key, \"" << name << "\")) {\n"
            "            ::configs::parse(static_cast<cfg::" << varname << "&>(this->variables).value, value);\n"
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
    void write(null_fill x) { this->out() << "::configs::null_fill()"; }

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

    template<class T, class U>
    void check_constructible(type_<T>, default_<U> const & d)
    { static_assert((void(type_<decltype(T(d.value))>{}), 1), "incompatible type"); }

    template<class T>
    void check_constructible(type_<T>, default_<macro> const &)
    {}

    template<class T, class U>
    void write_assignable_default(ref<type_<T>>, default_<U> const & d)
    {
        this->check_constructible(type_<T>{}, d);
        this->inherit().write(d.value);
    }

    template<std::size_t N, class U>
    void write_assignable_default(ref<type_<StaticKeyString<N>>>, default_<U> const & d)
    {
        this->check_constructible(type_<StaticKeyString<N>>{}, d);
        this->out() << "\"";
        this->write_key(d.value, N, "\\x");
        this->out() << "\"";
    }

    template<class T>
    void write_assignable_default(ref<type_<T>>, ...)
    { }


#define WRITE_TYPE(T) void write_type(type_<T>) { this->out() << #T; }
    WRITE_TYPE(bool)
    WRITE_TYPE(int)
    WRITE_TYPE(unsigned)
    WRITE_TYPE(std::string)
    WRITE_TYPE(RedirectionInfo)
    WRITE_TYPE(Theme)
    WRITE_TYPE(Font)
    WRITE_TYPE(::configs::IniAccounts)
    WRITE_TYPE(::configs::StaticIpString)
    WRITE_TYPE(::configs::Level)
    WRITE_TYPE(::configs::Language)
    WRITE_TYPE(::configs::ColorDepth)
    WRITE_TYPE(::configs::CaptureFlags)
    WRITE_TYPE(::configs::KeyboardLogFlags)
    WRITE_TYPE(::configs::ClipboardLogFlags)
    WRITE_TYPE(::configs::ClipboardEncodingType)
#undef WRITE_TYPE

#define WRITE_STATIC_STRING(T)        \
    template<std::size_t N>           \
    void write_type(ref<type_<T<N>>>) \
    { this->out() << #T "<" << N << ">"; }

    WRITE_STATIC_STRING(::configs::StaticString)
    WRITE_STATIC_STRING(::configs::StaticNilString)
    WRITE_STATIC_STRING(::configs::StaticKeyString)
    WRITE_STATIC_STRING(::configs::StaticPath)
#undef WRITE_STATIC_STRING

    void write_type(type_<uint32_>) { this->out() << "uint32_t"; }
    void write_type(type_<uint64_>) { this->out() << "uint64_t"; }

    template<class T, T Min, T Max, T Default>
    void write_type(ref<type_<Range<T, Min, Max, Default>>>) {
        this->out() << "::configs::Range<";
        this->inherit().write_type(type_<T>{});
        this->out() << ", " << Min << ", " << Max << ", " << Default << ">";
    }
};


template<class ConfigCppWriter>
void write_authid_hpp(std::ostream & out_authid, ConfigCppWriter & writer) {
    out_authid <<
      "enum authid_t {\n"
      "    AUTHID_UNKNOWN = 0,\n"
    ;
    for (auto & body : writer.authids) {
        out_authid
          << "    AUTHID_" << body.first << ",\n"
             "#define STRAUTHID_" << body.first
          << " \"" << body.second << "\"\n"
        ;
    }
    out_authid <<
      "    MAX_AUTHID\n"
      "};\n"
      "constexpr char const * const authstr[] = {\n"
    ;
    for (auto & body : writer.authids) {
        out_authid << "    STRAUTHID_" << body.first << ",\n";
    }
    out_authid << "};\n\n";
}

template<class ConfigCppWriter>
void write_variables_configuration(std::ostream & out_varconf, ConfigCppWriter & writer) {
    out_varconf <<
        "#include \"font.hpp\"\n"
        "#include \"config_types.hpp\"\n"
        "#include \"config_includes.hpp\"\n\n"
        "namespace cfg {\n"
    ;
    for (auto & body : writer.sections_member) {
        if (body.first.empty()) {
            out_varconf << body.second << "\n";
        }
        else {
            out_varconf <<
                "    namespace " << body.first << " {\n" <<
                         body.second <<
                "    }\n\n"
            ;
        }
    }

    auto join = [&](std::vector<std::string> const & cont, char const * before = "", char const * after = "") {
        auto first = begin(cont);
        auto last = end(cont);
        if (first == last) {
            return ;
        }
        out_varconf << before << "cfg::" << *first << after << "\n";
        while (++first != last) {
            out_varconf << ", " << before << "cfg::" << *first << after << "\n";
        }
    };

    out_varconf <<
      "}\n\n"
      "namespace configs {\n"
      "namespace detail_ { struct none {}; }\n\n"
      "struct VariablesConfiguration\n"
      ": "
    ;
    join(writer.variables);
    out_varconf <<
      "{\n"
      "    explicit VariablesConfiguration(char const * default_font_name)\n"
      "    : cfg::font{default_font_name}\n"
      "    {}\n"
      "};\n\n"
      "struct Buffers\n"
      ": "
    ;
    join(writer.variables, "CBuf<", ">");
    out_varconf <<
      "{};\n\n"
      "using VariablesAclPack = Pack<\n  "
    ;
    join(writer.variables_acl);
    out_varconf <<
      ">;\n"
      "}\n"
    ;
}

template<class ConfigCppWriter>
void write_config_initialize(std::ostream & out_body, ConfigCppWriter & writer) {
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
            "        else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
            "            LOG(LOG_ERR, \"unknown parameter %s in section [%s]\", key, context);\n"
            "        }\n"
            "    }\n"
        ;
    }
    out_set_value <<
        "    else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
        "        LOG(LOG_ERR, \"unknown section [%s]\", context);\n"
        "    }\n"
        "}\n"
    ;
}

}


template<class ConfigCppWriter>
int app_write_cpp_config(int ac, char const ** av)
{
    if (ac < 5) {
        std::cerr << av[0] << " out-authid.h out-variables_configuration.h out-config_initialize.cpp out-config_set_value.cpp";
        return 1;
    }

    ConfigCppWriter writer;

    MultiFilenameWriter<ConfigCppWriter> sw(writer);
    sw.then(av[1], &cpp_config_writer::write_authid_hpp<ConfigCppWriter>)
      .then(av[2], &cpp_config_writer::write_variables_configuration<ConfigCppWriter>)
      .then(av[3], &cpp_config_writer::write_config_initialize<ConfigCppWriter>)
      .then(av[4], &cpp_config_writer::write_config_set_value<ConfigCppWriter>)
    ;
    if (sw.err) {
        std::cerr << av[0] << ": " << sw.filename << ": " << strerror(errno) << "\n";
        return sw.errnum;
    }
    return 0;
}


template<class ConfigCppWriter>
int app_write_cpp_config(int ac, char ** av)
{
    return app_write_cpp_config<ConfigCppWriter>(ac, const_cast<char const **>(av));
}

#endif
