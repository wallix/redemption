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
#include <vector>
#include <chrono>
#include <memory>
#include <bitset>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

namespace cpp_config_writer
{

using namespace cfg_attributes;

template<class T>
void write_value(std::ostream& out, T const & x)
{
    if constexpr (std::is_enum_v<T>) {
        using ll = long long;
        out << " = static_cast<type>(" << ll{static_cast<std::underlying_type_t<T>>(x)} << ")";
    }
    else if constexpr (std::is_same_v<T, bool>) {
        out << '{' << (x ? "true" : "false") << '}';
    }
    else {
        out << '{' << x << '}';
    }
}

inline void write_value(std::ostream& out, const char * s)
{ out << " = \"" << io_quoted2{s} << '"';  }

inline void write_value(std::ostream& out, std::string const & str)
{ write_value(out, str.c_str()); }

inline void write_value(std::ostream& out, cpp::expr x)
{ out << " = " << x.value; }

template<class T, class Ratio>
void write_value(std::ostream& out, std::chrono::duration<T, Ratio> x) { out << '{' << x.count() << '}'; }


namespace impl
{
    template<class T, class U>
    void write_assignable_default(std::ostream& out, type_<T>, default_<U> const& d)
    { write_value(out, d.value); }

    template<unsigned N, class U>
    void write_assignable_default(std::ostream& out, type_<types::fixed_binary<N>>, default_<U> const& d)
    {
        if (d.value.size() != N) {
            throw std::runtime_error("invalide keys size");
        }
        out << "{{" << io_hexkey{d.value.c_str(), N, "0x", ", "} << "}}";
    }
}

template<class Pack>
void write_assignable_default(std::ostream& out, Pack& infos)
{
    if constexpr (is_t_convertible_v<Pack, default_>) {
        auto t = get_type<cpp::type_>(infos);
        impl::write_assignable_default(out, t, infos);
    }
    else {
        out << "{}";
    }
}


inline void write_type(std::ostream& out, type_<types::u16>) { out << "uint16_t"; }
inline void write_type(std::ostream& out, type_<types::u32>) { out << "uint32_t"; }
inline void write_type(std::ostream& out, type_<types::u64>) { out << "uint64_t"; }

template<unsigned N>
void write_type(std::ostream& out, type_<types::fixed_binary<N>>) { out << "std::array<unsigned char, " << N << ">"; }

template<unsigned N>
void write_type(std::ostream& out, type_<types::fixed_string<N>>) { out << "char[" << N+1 << "]"; }

template<class T, long min, long max>
void write_type(std::ostream& out, type_<types::range<T, min, max>>) { out << type_name<T>(); }

inline void write_type(std::ostream& out, type_<types::dirpath>)
{ out << "::configs::spec_types::directory_path"; }

inline void write_type(std::ostream& out, type_<types::ip_string>)
{ out << "std::string"; }

template<class T>
void write_type(std::ostream& out, type_<types::list<T>>) { out << "std::string"; }

template<class T>
void write_type(std::ostream& out, type_<T>) { out << type_name<T>(); }


template<unsigned N>
void write_type_spec(std::ostream& out, type_<types::fixed_string<N>>)
{ out << "::configs::spec_types::fixed_string"; }

template<unsigned N>
void write_type_spec(std::ostream& out, type_<types::fixed_binary<N>>)
{ out << "::configs::spec_types::fixed_binary"; }

template<class T, long min, long max>
void write_type_spec(std::ostream& out, type_<types::range<T, min, max>>)
{ out << "::configs::spec_types::range<" << type_name<T>() << ", " << min << ", " << max << ">"; }

inline void write_type_spec(std::ostream& out, type_<types::ip_string>)
{ out << "::configs::spec_types::ip"; }

template<class T>
void write_type_spec(std::ostream& out, type_<types::list<T>>)
{ out << "::configs::spec_types::list<" << type_name<T>() << ">"; }

template<class T>
void write_type_spec(std::ostream& out, type_<T> t) { write_type(out, t); }

struct CppConfigWriterBase;

void write_config_set_value(std::ostream & out_set_value, CppConfigWriterBase& writer);
void write_variables_configuration(std::ostream & out_varconf, CppConfigWriterBase& writer);
void write_variables_configuration_fwd(std::ostream & out_varconf, CppConfigWriterBase& writer);
void write_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer);

struct CppConfigWriterBase
{
    using attribute_name_type = cfg_attributes::cpp::name;

    unsigned depth = 0;
    std::ostringstream out_body_parser_;
    std::ostringstream out_member_;
    std::ostream * out_ = nullptr;

    struct Member
    {
        std::string name;
        std::size_t align_of;

        friend std::ostream& operator <<(std::ostream& out, Member const& x)
        {
            return out << x.name;
        }
    };

    struct Section
    {
        std::string section_name;
        std::string member_struct;
        std::vector<Member> members;
    };

    std::vector<std::pair<std::string, std::string>> sections_parser;
    std::vector<std::string> authstrs;
    std::vector<Section> sections;
    std::vector<Member> members;
    std::vector<std::string> variables_acl;
    std::vector<log_policy_t> authid_policy;

    struct Filenames
    {
        std::string authid_hpp;
        std::string variable_configuration_fwd;
        std::string variable_configuration_hpp;
        std::string congig_set_value;
    };
    Filenames filenames;

    CppConfigWriterBase(Filenames filenames)
    : filenames(std::move(filenames))
    {}

    std::ostream & out() { return *this->out_; }

    void do_init()
    {}

    int do_finish()
    {
        MultiFilenameWriter<CppConfigWriterBase> sw(*this);
        sw.then(filenames.authid_hpp, &write_authid_hpp)
          .then(filenames.variable_configuration_fwd, &write_variables_configuration_fwd)
          .then(filenames.variable_configuration_hpp, &write_variables_configuration)
          .then(filenames.congig_set_value, &write_config_set_value)
        ;
        if (sw.err) {
            std::cerr << "CppConfigWriterBase: " << sw.filename << ": " << strerror(errno) << "\n";
            return sw.errnum;
        }
        return 0;
    }

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
        this->sections.emplace_back(Section{section_name, this->out_member_.str(), std::move(this->members)});
        this->out_member_.str("");
        std::string str = this->out_body_parser_.str();
        if (!str.empty()) {
            this->sections_parser.emplace_back(section_name, str);
            this->out_body_parser_.str("");
        }
    }

    void tab() { this->out() << /*std::setw(this->depth*4+4) << */"    "; }

    template<class Pack>
    void evaluate_member(std::string const & section_name, Pack const & infos, type_enumerations& /*enums*/)
    {
        std::string const& varname = get_name<cpp::name>(infos);

        auto type = get_type<cpp::type_>(infos);
        this->members.push_back({varname, alignof(typename decltype(type)::type)});

        std::string const & varname_with_section = section_name.empty() ? varname : section_name + "::" + varname;

        using sesman_io = sesman::internal::io;

        auto const properties = [&]{
            if constexpr (is_convertible_v<Pack, sesman_io_t>) {
                return get_elem<sesman_io_t>(infos).value;
            }
            else {
                return sesman_io::none;
            }
        }();
        if (bool(/*PropertyFieldFlags::read & */properties)) {
            this->variables_acl.emplace_back(varname_with_section);
        }

        this->out_ = &this->out_member_;

        if constexpr (is_convertible_v<Pack, desc>) {
            //this->tab();
            this->out() << cpp_doxygen_comment(get_elem<cfg_attributes::desc>(infos).value, 4);
        };
        if (bool(properties)) {
            this->authstrs.emplace_back(get_name<sesman::name>(infos));
        }
        this->tab(); this->out() << "/// type: "; write_type(this->out(), type); this->out() << " <br/>\n";
        if ((properties & sesman_io::rw) == sesman_io::sesman_to_proxy) {
            this->tab(); this->out() << "/// sesman -> proxy <br/>\n";
        }
        else if ((properties & sesman_io::rw) == sesman_io::proxy_to_sesman) {
            this->tab(); this->out() << "/// sesman <- proxy <br/>\n";
        }
        else if ((properties & sesman_io::rw) == sesman_io::rw) {
            this->tab(); this->out() << "/// sesman <-> proxy <br/>\n";
        }
        this->tab(); this->out() << "/// value"; write_assignable_default(this->out(), infos); this->out() << " <br/>\n";
        this->tab(); this->out() << "struct " << varname_with_section << " {\n";
        this->tab(); this->out() << "    static constexpr bool is_sesman_to_proxy = " << (bool(properties & sesman_io::sesman_to_proxy) ? "true" : "false") << ";\n";
        this->tab(); this->out() << "    static constexpr bool is_proxy_to_sesman = " << (bool(properties & sesman_io::proxy_to_sesman) ? "true" : "false") << ";\n";
        this->tab(); this->out() << "    static constexpr char const * section = \"" << section_name << "\";\n";
        this->tab(); this->out() << "    static constexpr char const * name = \"" << varname << "\";\n";

        if (bool(properties)) {
            this->tab(); this->out() << "    // for old cppcheck\n";
            this->tab(); this->out() << "    // cppcheck-suppress obsoleteFunctionsindex\n";
            this->tab(); this->out() << "    static constexpr authid_t index = authid_t(" << this->authid_policy.size() << ");\n";
            this->authid_policy.emplace_back(infos);
        }

        this->tab(); this->out() << "    using type = ";
        write_type(this->out(), type);
        this->out() << ";\n";

        // write type
        if (bool(properties) || is_convertible_v<Pack, spec_attr_t>) {
            auto type_sesman = get_type<sesman::type_>(infos);
            auto type_spec = get_type<spec::type_>(infos);
            static_assert(
                std::is_same_v<decltype(type_spec), decltype(type_sesman)>
             || is_convertible_v<Pack, sesman_io_t> != is_convertible_v<Pack, spec_attr_t>,
                "different type for sesman and spec isn't supported (go code :D)"
            );
            this->tab(); this->out() << "    using sesman_and_spec_type = ";
            write_type_spec(this->out(), type_sesman);
            this->out() << ";\n";
            this->tab(); this->out() << "    using mapped_type = sesman_and_spec_type;\n";
        }
        else {
            this->tab(); this->out() << "    using mapped_type = type;\n";
        }

        // write value
        this->tab(); this->out() << "    type value";
        write_assignable_default(this->out(), infos);
        this->out() << ";\n";

        this->tab(); this->out() << "};\n";

        this->out_ = &this->out_body_parser_;

        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            auto type_spec = get_type<spec::type_>(infos);
            this->out() << "        else if (0 == strcmp(key, \"" << get_name<spec::name>(infos) << "\")) {\n"
            "            ::configs::parse_and_log(\n"
            "                context, key,\n"
            "                static_cast<cfg::" << varname_with_section << "&>(this->variables).value,\n"
            "                ::configs::spec_type<";
            write_type_spec(this->out(), type_spec);
            this->out() << ">{},\n"
            "                av\n"
            "            );\n"
            "        }\n";
        }
    }
};


inline void write_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer)
{
    out_authid <<
      "//\n"
      "// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"
      "//\n\n"
      "#pragma once\n"
      "\n"
      "#include \"utils/sugar/array_view.hpp\"\n"
      "\n"
      "enum authid_t : unsigned;\n\n"
      "constexpr authid_t MAX_AUTHID = authid_t(" << writer.authstrs.size() << ");\n\n"
      "constexpr array_view_const_char const authstr[] = {\n"
    ;
    for (auto & authstr : writer.authstrs) {
        out_authid << "    \"" << authstr << "\"_av,\n";
    }
    out_authid << "};\n";
}

inline void write_variables_configuration_fwd(std::ostream & out_varconf, CppConfigWriterBase& writer)
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
            for (auto & var : section.members) {
                out_varconf << "    struct " << var << ";\n";
            }
        }
        else {
            out_varconf << "    struct " << section.section_name << " {\n";
            for (auto & var : section.members) {
                out_varconf << "        struct " << var << ";\n";
            }
            out_varconf << "    };\n\n";
        }
    }

    out_varconf << "} // namespace cfg\n";
}

inline void write_variables_configuration(std::ostream & out_varconf, CppConfigWriterBase& writer)
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
        auto const & cont,
        auto const & before,
        auto const & after
    ) {
        auto first = begin(cont);
        auto last = end(cont);
        if (first == last) {
            return ;
        }
        out_varconf << before << *first << after;
        while (++first != last) {
            out_varconf << ", " << before << *first << after;
        }
    };

    std::vector<std::string> section_names;

    out_varconf <<
        "} // namespace cfg\n\n"
        "namespace cfg_section {\n"
    ;
    using Member = typename CppConfigWriterBase::Member;
    for (auto & body : writer.sections) {
        if (!body.section_name.empty()) {
            std::vector<std::reference_wrapper<Member>> v(body.members.begin(), body.members.end());
            std::stable_sort(v.begin(), v.end(), [](Member& a, Member& b){
                return a.align_of > b.align_of;
            });
            section_names.emplace_back("cfg_section::" + body.section_name);
            out_varconf << "struct " << body.section_name << "\n: ";
            join(v, "cfg::" + body.section_name + "::", "\n");
            out_varconf << "{ static constexpr bool is_section = true; };\n\n";
        }
    }
    out_varconf << "} // namespace cfg_section\n\n"
      "namespace configs {\n"
      "struct VariablesConfiguration\n"
      ": "
    ;
    join(section_names, "", "\n");
    auto it = std::find_if(begin(writer.sections), end(writer.sections), [](auto & p){ return p.section_name.empty(); });
    if (it != writer.sections.end()) {
        for (Member& mem : it->members) {
            out_varconf << ", cfg::" << mem.name << "\n";
        }
    }
    out_varconf <<
      "{};\n\n"
      "using VariablesAclPack = Pack<\n  "
    ;
    join(writer.variables_acl, "cfg::", "\n");
    out_varconf <<
      ">;\n\n\n"
    ;

    std::vector<std::bitset<64>> loggables;
    std::vector<std::bitset<64>> unloggable_if_value_contains_passwords;
    int i = 0;

    for (auto log_policy : writer.authid_policy)
    {
        if ((i % 64) == 0) {
            i = 0;
            loggables.push_back(0);
            unloggable_if_value_contains_passwords.push_back(0);
        }

        switch (log_policy.value) {
            case spec::log_policy::loggable:
                loggables.back().set(i);
                break;
            case spec::log_policy::unloggable:
                break;
            case spec::log_policy::unloggable_if_value_contains_password:
                unloggable_if_value_contains_passwords.back().set(i);
                break;
        }
        ++i;
    }

    out_varconf <<
      "struct BitFlags {\n"
      "  uint64_t bits_[" << loggables.size() << "];\n"
      "  bool operator()(unsigned i) const noexcept { return bits_[i/64] & (uint64_t{1} << (i%64)); }\n"
      "};\n\n"
    ;

    out_varconf << "constexpr BitFlags is_loggable{{\n  ";
    join(loggables, "0b", "\n");
    out_varconf << "}};\nconstexpr BitFlags is_unloggable_if_value_with_password{{\n  ";
    join(unloggable_if_value_contains_passwords, "0b", "\n");
    out_varconf <<
      "}};\n"
      "} // namespace configs\n"
    ;
}

inline void write_config_set_value(std::ostream & out_set_value, CppConfigWriterBase& writer)
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

}
