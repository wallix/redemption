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
#include <chrono>
#include <unordered_map>
#include <memory>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace cpp_config_writer {

using namespace cfg_attributes;

template<class Inherit>
struct CppConfigWriterBase : ConfigSpecWriterBase<Inherit, cpp::name>
{
    unsigned depth = 0;
    std::ostringstream out_body_parser_;
    std::ostringstream out_member_;
    std::ostream * out_ = nullptr;

    struct Section
    {
        std::string section_name;
        std::string member_struct;
        std::vector<std::string> member_names;
    };

    std::vector<std::pair<std::string, std::string>> sections_parser;
    std::vector<std::pair<std::string, std::string>> authids;
    std::vector<Section> sections;
    std::vector<std::string> member_names;
    std::vector<std::string> variables_acl;
    unsigned index_authid = 0;

    std::ostream & out() { return *this->out_; }

    void do_start_section(std::string const & section_name)
    {
        if (!section_name.empty()) {
            ++this->depth;
        }
    }

    void do_stop_section(std::string const & section_name)
    {
        if (!section_name.empty()) {
            --this->depth;
        }
        this->sections.emplace_back(Section{section_name, this->out_member_.str(), std::move(this->member_names)});
        this->out_member_.str("");
        std::string str = this->out_body_parser_.str();
        if (!str.empty()) {
            this->sections_parser.emplace_back(section_name, str);
            this->out_body_parser_.str("");
        }
    }

    void tab() { this->out() << /*std::setw(this->depth*4+4) << */"    "; }

    template<class Pack>
    void do_member(
        std::string const & section_name,
        std::string const & varname,
        Pack const & infos
    ) {
        this->member_names.push_back(varname);
        auto type = pack_get<cpp::type_>(infos);

        std::string const & varname_with_section = section_name.empty() ? varname : section_name + "::" + varname;

        auto const properties = value_or(infos, sesman::internal::io::none);
        if (bool(/*PropertyFieldFlags::read & */properties)) {
            this->variables_acl.emplace_back(varname_with_section);
        }

        this->out_ = &this->out_member_;

        apply_if_contains<desc>(infos, [this](auto desc){
            //this->tab();
            this->out() << cpp_doxygen_comment(desc.value, 4);
        });
        if (bool(properties)) {
            this->tab();
            this->out() << "/// AUTHID_";
            std::string str = section_name;
            str += '_';
            str += varname;
            for (auto & c : str) {
                c = char(std::toupper(c));
            }
            this->out() << str << " <br/>\n";
            this->authids.emplace_back(str, pack_get<sesman::name>(infos));
        }
        this->tab(); this->out() << "/// type: "; this->inherit().write_type(type); this->out() << " <br/>\n";
        if ((properties & sesman::internal::io::rw) == sesman::internal::io::sesman_to_proxy) {
            this->tab(); this->out() << "/// sesman -> proxy <br/>\n";
        }
        else if ((properties & sesman::internal::io::rw) == sesman::internal::io::proxy_to_sesman) {
            this->tab(); this->out() << "/// sesman <- proxy <br/>\n";
        }
        else if ((properties & sesman::internal::io::rw) == sesman::internal::io::rw) {
            this->tab(); this->out() << "/// sesman <-> proxy <br/>\n";
        }
        this->tab(); this->out() << "/// value"; this->write_assignable_default(pack_contains<default_>(infos), type, &infos); this->out() << " <br/>\n";
        this->tab(); this->out() << "struct " << varname_with_section << " {\n";
        this->tab(); this->out() << "    static constexpr bool is_sesman_to_proxy = " << (bool(properties & sesman::internal::io::sesman_to_proxy) ? "true" : "false") << ";\n";
        this->tab(); this->out() << "    static constexpr bool is_proxy_to_sesman = " << (bool(properties & sesman::internal::io::proxy_to_sesman) ? "true" : "false") << ";\n";

        this->tab(); this->out() << "    static constexpr char const * section = \"" << section_name << "\";\n";
        this->tab(); this->out() << "    static constexpr char const * name = \"" << varname << "\";\n";

        if (bool(properties)) {
            this->tab(); this->out() << "    // for old cppcheck\n";
            this->tab(); this->out() << "    // cppcheck-suppress obsoleteFunctionsindex\n";
            this->tab(); this->out() << "    static constexpr authid_t index = authid_t(" << this->index_authid++ << ");\n";
        }

        this->tab(); this->out() << "    using type = ";
        this->inherit().write_type(type);
        this->out() << ";\n";

        // write type
        if (bool(properties) || pack_contains<spec::internal::attr>(infos)) {
            auto type_sesman = pack_get<sesman::type_>(infos);
            auto type_spec = pack_get<spec::type_>(infos);
            static_assert(
                std::is_same<decltype(type_spec), decltype(type_sesman)>{}
             || !std::is_same<
                    decltype(pack_contains<sesman::internal::io>(infos)),
                    decltype(pack_contains<spec::internal::attr>(infos))
                >{},
                "different type for sesman and spec isn't supported (go code :D)"
            );
            this->tab(); this->out() << "    using sesman_and_spec_type = ";
            this->inherit().write_type_spec(type_sesman);
            this->out() << ";\n";
            this->tab(); this->out() << "    using mapped_type = sesman_and_spec_type;\n";
        }
        else {
            this->tab(); this->out() << "    using mapped_type = type;\n";
        }

        // write value
        this->tab(); this->out() << "    type value";
        this->write_assignable_default(pack_contains<default_>(infos), type, &infos);
        this->out() << ";\n";

        this->tab(); this->out() << "};\n";

        this->out_ = &this->out_body_parser_;

        apply_if_contains<spec::internal::attr>(infos, [this, &infos, &varname_with_section](auto&&){
            auto type_spec = pack_get<spec::type_>(infos);
            this->out() << "        else if (0 == strcmp(key, \"" << pack_get<spec::name>(infos) << "\")) {\n"
            "            ::configs::parse_and_log(\n"
            "                context, key,\n"
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
    { this->out() << " = static_cast<type>(" << static_cast<unsigned long>(e) << ")"; }

    template<class T>
    disable_if_enum_t<T>
    write_value(T const & r)
    {
        if (std::is_same<T, bool>{}) {
            this->out() << '{' << (r ? "true" : "false") << '}';
        }
        else {
            this->out() << '{' << r << '}';
        }
    }

    void write_value(const char * s) { this->out() << " = \"" << io_quoted2{s} << '"';  }
    void write_value(std::string const & str) { this->write_value(str.c_str()); }
    void write_value(cpp::expr x) { this->out() << " = " << x.value; }

    template<class T, class Ratio>
    void write_value(std::chrono::duration<T, Ratio> x) { this->out() << '{' << x.count() << '}'; }


    template<class T, class U>
    void write_assignable_default(std::true_type, type_<T>, val<default_<U>> const * d)
    { this->inherit().write_value(d->x.value); }

    template<unsigned N, class U>
    void write_assignable_default(std::true_type, type_<types::fixed_binary<N>>, val<default_<U>> const * d)
    {
        if (d->x.value.size() != N) {
            throw std::runtime_error("invalide keys size");
        }
        this->out() << "{{" << io_hexkey{d->x.value.c_str(), N, "0x", ", "} << "}}";
    }

    void write_assignable_default(std::false_type, ...)
    { this->out() << "{}"; }


    void write_type(type_<types::u16>) { this->out() << "uint16_t"; }
    void write_type(type_<types::u32>) { this->out() << "uint32_t"; }
    void write_type(type_<types::u64>) { this->out() << "uint64_t"; }

    template<unsigned N>
    void write_type(type_<types::fixed_binary<N>>) { this->out() << "std::array<unsigned char, " << N << ">"; }

    template<unsigned N>
    void write_type(type_<types::fixed_string<N>>) { this->out() << "char[" << N+1 << "]"; }

    template<class T, long min, long max>
    void write_type(type_<types::range<T, min, max>>) { this->out() << type_name<T>(); }

    void write_type(type_<types::dirpath>) { this->out() << "::configs::spec_types::directory_path"; }
    void write_type(type_<types::ip_string>) { this->out() << "std::string"; }

    template<class T>
    void write_type(type_<types::list<T>>) { this->out() << "std::string"; }

    template<class T>
    void write_type(type_<T>) { this->out() << type_name<T>(); }


    template<unsigned N>
    void write_type_spec(type_<types::fixed_string<N>>)
    { this->out() << "::configs::spec_types::fixed_string"; }

    template<unsigned N>
    void write_type_spec(type_<types::fixed_binary<N>>)
    { this->out() << "::configs::spec_types::fixed_binary"; }

    template<class T, long min, long max>
    void write_type_spec(type_<types::range<T, min, max>>)
    { this->out() << "::configs::spec_types::range<" << type_name<T>() << ", " << min << ", " << max << ">"; }

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
      "//\n"
      "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
      "//\n\n"
      "#pragma once\n"
      "\n"
      "enum authid_t : unsigned;\n\n"
      "inline authid_t MAX_AUTHID = authid_t(" << writer.authids.size() << ");\n\n"
      "constexpr char const * const authstr[] = {\n"
    ;
    for (auto & body : writer.authids) {
        out_authid << "    \"" << body.second << "\",\n";
    }
    out_authid << "};\n";
}

template<class ConfigCppWriter>
void write_variables_configuration_fwd(std::ostream & out_varconf, ConfigCppWriter & writer)
{
    out_varconf <<
        "//\n"
        "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
        "//\n\n"
        "#pragma once\n"
        "\n"
        "enum authid_t : unsigned;\n\n"
        "namespace cfg {\n"
    ;
    for (auto & section : writer.sections) {
        if (section.section_name.empty()) {
            for (auto & var : section.member_names) {
                out_varconf << "    struct " << var << ";\n";
            }
        }
        else {
            out_varconf << "    struct " << section.section_name << " {\n";
            for (auto & var : section.member_names) {
                out_varconf << "        struct " << var << ";\n";
            }
            out_varconf << "    };\n\n";
        }
    }

    out_varconf << "} // namespace cfg\n";
}

template<class ConfigCppWriter>
void write_variables_configuration(std::ostream & out_varconf, ConfigCppWriter & writer)
{
    out_varconf <<
        "//\n"
        "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
        "//\n\n"
        "namespace cfg {\n"
    ;
    for (auto & section : writer.sections) {
        out_varconf << section.member_struct << "\n";
    }

    auto join = [&](
        std::vector<std::string> const & cont,
        auto const & before,
        auto const & after
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
        "} // namespace cfg\n\n"
        "namespace cfg_section {\n"
    ;
    for (auto & body : writer.sections) {
       if (!body.section_name.empty()) {
           section_names.emplace_back("cfg_section::" + body.section_name);
           out_varconf << "struct " << body.section_name << "\n: ";
           join(body.member_names, "cfg::" + body.section_name + "::", "");
           out_varconf << "{ static constexpr bool is_section = true; };\n\n";
       }
    }
    out_varconf << "} // namespace cfg_section\n\n"
      "namespace configs {\n"
      "struct VariablesConfiguration\n"
      ": "
    ;
    join(section_names, "", "");
    auto it = std::find_if(begin(writer.sections), end(writer.sections), [](auto & p){ return p.section_name.empty(); });
    if (it != writer.sections.end()) {
       for (auto & s : it->member_names) {
           out_varconf << ", cfg::" << s << "\n";
       }
    }
    out_varconf <<
      "{};\n\n"
      "using VariablesAclPack = Pack<\n  "
    ;
    join(writer.variables_acl, "cfg::", "");
    out_varconf <<
      ">;\n"
      "} // namespace configs\n"
    ;
}

template<class ConfigCppWriter>
void write_config_set_value(std::ostream & out_set_value, ConfigCppWriter & writer)
{
    out_set_value <<
        "//\n"
        "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
        "//\n\n"
        "void Inifile::ConfigurationHolder::set_value(const char * context, const char * key, const char * value) {\n"
        "    array_view_const_char av {value, strlen(value)};\n"
        "    if (0) {}\n"
    ;
    for (auto & body : writer.sections_parser) {
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
        std::cerr << av[0] <<
          " out-authid.h"
          " out-variables_configuration_fwd.h"
          " out-variables_configuration.h"
          " out-config_set_value.cpp"
          "\n";
        return 1;
    }

    ConfigCppWriter writer;
    writer.evaluate();

    MultiFilenameWriter<ConfigCppWriter> sw(writer);
    sw.then(av[1], &cpp_config_writer::write_authid_hpp<ConfigCppWriter>)
      .then(av[2], &cpp_config_writer::write_variables_configuration_fwd<ConfigCppWriter>)
      .then(av[3], &cpp_config_writer::write_variables_configuration<ConfigCppWriter>)
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

}
