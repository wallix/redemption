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
#include "configs/type_name.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>

#include <cerrno>
#include <cstring>

class Font;

namespace cfg_generators {

namespace cpp_config_writer {

using namespace cfg_attributes;

template<class Inherit>
struct CppConfigWriterBase : ConfigSpecWriterBase<Inherit>
{
    std::ostringstream out_body_parser_;

    std::map<std::string, std::vector<std::string>> variables_by_sections;
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
    }

    template<class... Ts>
    void member(Ts const & ... args)
    {
        pack_type<Ts...> pack{args...};
        auto type = pack_get<cpp::type_>(pack);

        std::string varname = pack_get<cpp::name>(pack);
        auto const properties = value_or(pack, sesman::io::none);
        auto varname_with_section = this->section_name.empty() ? varname : this->section_name + "::" + varname;
        if (bool(/*PropertyFieldFlags::read & */properties)) {
            this->variables_acl.emplace_back(varname_with_section);
        }
        this->variables.emplace_back(varname_with_section);
        this->variables_by_sections[this->section_name].emplace_back(varname);

        this->out_ = &this->out_member_;

        apply_if_contains<desc>(pack, [this](auto desc){
            this->out() << cpp_comment(desc.value, 8);
        });
        if (bool(properties)) {
            this->tab();
            this->out() << "// AUTHID_";
            std::string str = this->section_name;
            str += '_';
            str += varname;
            for (auto & c : str) {
                c = char(std::toupper(c));
            }
            this->out() << str << "\n";
            this->authids.emplace_back(str, pack_get<sesman::name>(pack));
        }
        this->tab(); this->out() << "struct " << varname << " {\n";
        this->tab(); this->out() << "    static constexpr bool is_readable() { return " << bool(properties & sesman::io::read) << "; }\n";
        this->tab(); this->out() << "    static constexpr bool is_writable() { return " << bool(properties & sesman::io::write) << "; }\n";

        this->tab(); this->out() << "    static constexpr char const * section() { return \"" << this->section_name << "\"; }\n";
        this->tab(); this->out() << "    static constexpr char const * name() { return \"" << varname << "\"; }\n";

        if (bool(properties)) {
            this->tab(); this->out() << "    static constexpr unsigned index() { return " << this->index_authid++ << "; }\n";
        }

        this->tab(); this->out() << "    using type = ";
        this->inherit().write_type(type);
        this->out() << ";\n";

        if (bool(properties) || pack_contains<spec::attr>(pack)) {
            auto type_sesman = pack_get<sesman::type_>(pack);
            auto type_spec = pack_get<spec::type_>(pack);
            static_assert(
                std::is_same<decltype(type_spec), decltype(type_sesman)>{}
             || !std::is_same<
                    decltype(pack_contains<sesman::io>(pack)),
                    decltype(pack_contains<spec::attr>(pack))
                >{},
                "different type for sesman and spec isn't supported (go code :D)"
            );
            this->tab(); this->out() << "    using sesman_and_spec_type = ";
            this->inherit().write_type_spec(type_sesman);
            this->out() << ";\n";
        };

        if (std::is_convertible<decltype(type), type_<Font>>::value) {
            this->tab(); this->out() << "    font(char const * filename) : value(filename) {}\n";
            this->tab(); this->out() << "    type value;\n";
        }
        else {
            this->tab(); this->out() << "    type value";
            this->write_assignable_default(pack_contains<default_>(pack), type, pack);
            this->out() << ";\n";
        }
        this->tab(); this->out() << "};\n";

        this->out_ = &this->out_body_parser_;

        apply_if_contains<spec::attr>(pack, [this, &pack, &varname_with_section](auto&&){
            auto type_spec = pack_get<spec::type_>(pack);
            this->out() << "        else if (0 == strcmp(key, \"" << pack_get<spec::name>(pack) << "\")) {\n"
            "            ::configs::parse_and_log(\n"
            "                context, key, \n"
            "                static_cast<cfg::" << varname_with_section << "&>(this->variables).value,\n"
            "                ::configs::spec_type<";
            this->inherit().write_type_spec(type_spec);
            this->out() << ">{},\n"
            "                av\n"
            "            );\n"
            "        }\n";
        });
    }


    template<class E>
    enable_if_enum_t<E>
    write_value(E const & e)
    { this->out() << "{static_cast<type>(" << static_cast<unsigned long>(e) << ")}"; }

    template<class T>
    disable_if_enum_t<T>
    write_value(T const & r) { this->out() << '{' << r << '}'; }

    void write_value(const char * s) { this->out() << " = \"" << io_quoted2{s} << '"';  }
    void write_value(cpp::macro x) { this->out() << " = " << x.name; }


    template<class T, class U>
    void write_assignable_default(std::true_type, type_<T>, ref<default_<U>> const & d)
    { this->inherit().write_value(d.x.value); }

    template<unsigned N, class U>
    void write_assignable_default(std::true_type, type_<types::fixed_binary<N>>, ref<default_<U>> const & d)
    {
        static_assert(std::is_same<typename default_<U>::type, char[N+1]>::value, "not supported type");
        this->out() << "{{" << io_hexkey{d.x.value, N, "0x", ", "} << "}}";
    }

    void write_assignable_default(std::false_type, ...)
    { this->out() << "{}"; }


    void write_type(type_<types::u32>) { this->out() << "uint32_t"; }
    void write_type(type_<types::u64>) { this->out() << "uint64_t"; }

    template<unsigned N>
    void write_type(type_<types::fixed_binary<N>>) { this->out() << "std::array<unsigned char, " << N << ">"; }

    template<unsigned N>
    void write_type(type_<types::fixed_string<N>>) { this->out() << "char[" << N << " + 1]"; }

    void write_type(type_<types::path>) { this->out() << "::configs::spec_types::directory_path"; }
    void write_type(type_<types::ip_string>) { this->out() << "std::string"; }

    template<class T>
    void write_type(type_<types::list<T>>) { this->out() << "std::string"; }

    template<class T>
    void write_type(type_<T>) { this->out() << type_name<T>(); }


    template<unsigned N>
    void write_type_spec(type_<types::fixed_binary<N>>)
    { this->out() << "::configs::spec_types::fixed_binary<" << N << ">"; }

    void write_type_spec(type_<types::ip_string>) { this->out() << "::configs::spec_types::ip"; }

    template<class T>
    void write_type_spec(type_<types::list<T>>)
    { this->out() << "::configs::spec_types::list<" << type_name<T>() << ">"; }

    template<class T>
    void write_type_spec(type_<T> t) { this->write_type(t); }
};


template<class ConfigCppWriter>
void write_authid_hpp(std::ostream & out_authid, ConfigCppWriter & writer)
{
    out_authid <<
      "#pragma once\n"
      "\n"
      "enum authid_t {\n"
    ;
    for (auto & body : writer.authids) {
        out_authid << "    AUTHID_" << body.first << ",\n";
    }
    out_authid <<
      "    MAX_AUTHID,\n"
      "    AUTHID_UNKNOWN\n"
      "};\n"
      "constexpr char const * const authstr[] = {\n"
    ;
    for (auto & body : writer.authids) {
        out_authid << "    \"" << body.second << "\",\n";
    }
    out_authid << "};\n";
}

template<class ConfigCppWriter>
void write_variables_configuration(std::ostream & out_varconf, ConfigCppWriter & writer)
{
    out_varconf <<
        "namespace cfg {\n"
    ;
    for (auto & section_name : writer.sections_ordered) {
        auto & members = writer.sections.find(section_name)->second;
        if (section_name.empty()) {
            out_varconf << members << "\n";
        }
        else {
            out_varconf <<
                "    struct " << section_name << " {\n" <<
                         members <<
                "    };\n\n"
            ;
        }
    }

    auto join = [&](
        std::vector<std::string> const & cont,
        std::string const & before,
        std::string const & after
    ) {
        auto first = begin(cont);
        auto last = end(cont);
        if (first == last) {
            return ;
        }
        out_varconf << before << *first << after << "\n";
        while (++first != last) {
            out_varconf << ", " << before << *first << after << "\n";
        }
    };

    std::vector<std::string> section_names;

    out_varconf <<
      "}\n\n"
      "namespace cfg_section {\n"
    ;
    for (auto & body : writer.variables_by_sections) {
       if (!body.first.empty()) {
           section_names.emplace_back("cfg_section::" + body.first);
           out_varconf << "struct " << body.first << "\n: ";
           join(body.second, "cfg::" + body.first + "::", "");
           out_varconf << "{ static constexpr bool is_section = true; };\n\n";
       }
    }
    out_varconf << "}\n\n"
      "namespace configs {\n"
      "struct VariablesConfiguration\n"
      ": "
    ;
    join(section_names, "", "");
    auto it = writer.variables_by_sections.find("");
    if (it != writer.variables_by_sections.end()) {
       for (auto & s : it->second) {
           out_varconf << ", cfg::" << s << "\n";
       }
    }
    out_varconf <<
      "{\n"
      "    explicit VariablesConfiguration(char const * default_font_name)\n"
      "    : cfg::font{default_font_name}\n"
      "    {}\n"
      "};\n\n"
      "using VariablesAclPack = Pack<\n  "
    ;
    join(writer.variables_acl, "cfg::", "");
    out_varconf <<
      ">;\n"
      "}\n"
    ;
}

template<class ConfigCppWriter>
void write_config_set_value(std::ostream & out_set_value, ConfigCppWriter & writer)
{
    out_set_value <<
        "inline void Inifile::ConfigurationHolder::set_value(const char * context, const char * key, const char * value) {\n"
        "    array_view_const_char av {value, strlen(value)};\n"
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
    if (ac < 4) {
        std::cerr << av[0] << " out-authid.h out-variables_configuration.h out-config_set_value.cpp\n";
        return 1;
    }

    ConfigCppWriter writer;

    MultiFilenameWriter<ConfigCppWriter> sw(writer);
    sw.then(av[1], &cpp_config_writer::write_authid_hpp<ConfigCppWriter>)
      .then(av[2], &cpp_config_writer::write_variables_configuration<ConfigCppWriter>)
      .then(av[3], &cpp_config_writer::write_config_set_value<ConfigCppWriter>)
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

}
