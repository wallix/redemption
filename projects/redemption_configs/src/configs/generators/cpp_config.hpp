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
#include "configs/generators/utils/names.hpp"
#include "configs/enumeration.hpp"
#include "configs/type_name.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/file_permissions.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <memory>
#include <bitset>
#include <charconv>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

namespace cpp_config_writer
{

using namespace cfg_attributes;

namespace impl
{

template<class T>
void write_value(std::ostream& out, [[maybe_unused]] type_enumerations& enums, T const & x)
{
    if constexpr (std::is_enum_v<T>) {
        out << type_name<T>();
        type_enumeration const& e = enums.get_enum<T>();
        auto ull = static_cast<unsigned long long>(x);
        for (type_enumeration::value_type const& v : e.values) {
            if (v.val == ull) {
                out << "::" << v.name;
                return ;
            }
        }
        // probably an enum flag
        out << "(" << +static_cast<std::underlying_type_t<T>>(x) << ")";
    }
    else if constexpr (std::is_same_v<T, bool>) {
        out << (x ? "true" : "false");
    }
    else {
        out << x;
    }
}

inline void write_value(std::ostream& out, type_enumerations& /*enums*/, const char * s)
{ out << "\"" << io_quoted2{s} << "\"";  }

inline void write_value(std::ostream& out, type_enumerations& enums, std::string const & str)
{ write_value(out, enums, str.c_str()); }

inline void write_value(std::ostream& out, type_enumerations& /*enums*/, cpp::expr x)
{ out << x.value; }

template<class T, class Ratio>
void write_value(std::ostream& out, type_enumerations& /*enums*/, std::chrono::duration<T, Ratio> x)
{ out << x.count(); }

}

template<class T, class U>
void write_assignable_default(std::ostream& out, type_enumerations& enums, type_<T>, default_<U> const& d)
{ impl::write_value(out, enums, d.value); }

template<unsigned N, class U>
void write_assignable_default(std::ostream& out, type_enumerations& /*enums*/, type_<types::fixed_binary<N>>, default_<U> const& d)
{
    if (d.value.size() != N) {
        throw std::runtime_error("invalide keys size");
    }
    out << "{" << io_hexkey{d.value.c_str(), N, "0x", ", "} << "}";
}

template<class U>
void write_assignable_default(std::ostream& out, type_enumerations& /*enums*/, type_<FilePermissions>, default_<U> const& d)
{
    char octal[32]{};
    auto r = std::to_chars(std::begin(octal), std::end(octal), d.value, 8);
    if (r.ec != std::errc()) {
        throw std::runtime_error(str_concat(
            "invalide file_permission value: ", std::make_error_code(r.ec).message()));
    }
    out << "0" << octal;
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

inline void write_type(std::ostream& out, type_<types::rgb>)
{ out << "::configs::spec_types::rgb"; }

inline void write_type(std::ostream& out, type_<types::dirpath>)
{ out << "::configs::spec_types::directory_path"; }

inline void write_type(std::ostream& out, type_<types::ip_string>)
{ out << "std::string"; }

template<class T>
void write_type(std::ostream& out, type_<types::list<T>>) { out << "std::string"; }

template<class T>
void write_type(std::ostream& out, type_<T>) { out << type_name<T>(); }

inline void write_type(std::ostream& out, type_<FilePermissions>)
{ out << "FilePermissions"; }


// buffer size for assign_zbuf_from_cfg()
enum class StrBufferSize : std::size_t;

template<class TInt>
constexpr std::size_t integral_buffer_size()
{
    return std::numeric_limits<TInt>::digits10 + 1 + std::numeric_limits<TInt>::is_signed;
}

template<unsigned N>
StrBufferSize write_type_spec(std::ostream& out, type_<types::fixed_string<N>>)
{
    out << "::configs::spec_types::fixed_string";
    return StrBufferSize(0);
}

template<unsigned N>
StrBufferSize write_type_spec(std::ostream& out, type_<types::fixed_binary<N>>)
{
    out << "::configs::spec_types::fixed_binary";
    return StrBufferSize(N * 2);
}

template<class T, long min, long max>
StrBufferSize write_type_spec(std::ostream& out, type_<types::range<T, min, max>>)
{
    out << "::configs::spec_types::range<" << type_name<T>() << ", " << min << ", " << max << ">";
    return StrBufferSize(integral_buffer_size<T>());
}

template<class T>
StrBufferSize write_type_spec(std::ostream& out, type_<types::list<T>>)
{
    out << "::configs::spec_types::list<" << type_name<T>() << ">";
    return StrBufferSize(0);
}

template<class Rep, class Period>
StrBufferSize write_type_spec(std::ostream& out, type_<std::chrono::duration<Rep, Period>>)
{
    out << type_name<std::chrono::duration<Rep, Period>>();
    return StrBufferSize(integral_buffer_size<Rep>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<bool>)
{
    out << "bool";
    return StrBufferSize(5);
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::u16>)
{
    out << "uint16_t";
    return StrBufferSize(integral_buffer_size<uint16_t>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::u32>)
{
    out << "uint32_t";
    return StrBufferSize(integral_buffer_size<uint32_t>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::u64>)
{
    out << "uint64_t";
    return StrBufferSize(integral_buffer_size<uint64_t>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::unsigned_>)
{
    out << "unsigned";
    return StrBufferSize(integral_buffer_size<unsigned>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::int_>)
{
    out << "int";
    return StrBufferSize(integral_buffer_size<int>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<std::string>)
{
    out << "std::string";
    return StrBufferSize(0);
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::dirpath>)
{
    out << "::configs::spec_types::directory_path";
    return StrBufferSize(0);
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::ip_string>)
{
    out << "::configs::spec_types::ip";
    return StrBufferSize(0);
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<FilePermissions>)
{
    out << "::FilePermissions";
    return StrBufferSize(integral_buffer_size<uint16_t>());
}

inline StrBufferSize write_type_spec(std::ostream& out, type_<types::rgb>)
{
    out << "::configs::spec_types::rgb";
    return StrBufferSize(7);
}

template<class T>
StrBufferSize write_type_spec(std::ostream& out, type_<T>)
{
    if constexpr (std::is_enum_v<T>) {
        out << type_name<T>();
        return StrBufferSize(0);
    }
    else {
        static_assert(std::is_void_v<T>, "missing type");
    }
}


struct CppConfigWriterBase;

void write_max_str_buffer_size_hpp(std::ostream & out, CppConfigWriterBase& writer);
void write_ini_values(std::ostream & out, CppConfigWriterBase& writer);
void write_config_set_value(std::ostream & out_set_value, CppConfigWriterBase& writer);
void write_variables_configuration(std::ostream & out_varconf, CppConfigWriterBase& writer);
void write_variables_configuration_fwd(std::ostream & out_varconf, CppConfigWriterBase& writer);
void write_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer);
void write_str_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer);

struct FullNameMemberChecker
{
    void emplace_back(std::string const& name)
    {
        this->full_names.emplace_back(name);
    }

    std::string get_error()
    {
        std::string error;

        if (!this->full_names.empty()) {
            std::sort(this->full_names.begin(), this->full_names.end());

            auto* previous_name = &this->full_names.front();
            for (auto& name : writable_array_view{this->full_names}.from_offset(1)) {
                if (name == *previous_name) {
                    str_append(error, (error.empty() ? "duplicates member: " : ", "), name);
                }
                previous_name = &name;
            }
        }

        return error;
    }

    std::vector<std::string> full_names;
};

struct CppConfigWriterBase
{
    unsigned depth = 0;
    std::ostringstream out_body_parser_;
    std::ostringstream out_member_;

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
    std::size_t start_section_index = 0;
    std::vector<std::size_t> start_indexes;
    std::string cfg_values;
    std::string cfg_str_values;
    std::size_t max_str_buffer_size = 0;

    struct Filenames
    {
        std::string authid_hpp;
        std::string str_authid_hpp;
        std::string variable_configuration_fwd;
        std::string variable_configuration_hpp;
        std::string config_set_value;
        std::string ini_values_hpp;
        std::string max_str_buffer_size_hpp;
    };
    Filenames filenames;

    struct FullNames
    {
        FullNameMemberChecker spec;
        FullNameMemberChecker sesman;

        void check()
        {
            std::string full_err;

            struct P { char const* name; FullNameMemberChecker& checker; };
            for (P const& p : {
                P{"Ini", this->spec},
                P{"Sesman", this->sesman},
            }) {
                auto err = p.checker.get_error();
                if (not err.empty()) {
                    if (not full_err.empty()) {
                        full_err += " ; ";
                    }
                    str_append(full_err, p.name, ": ", err);
                }
            }

            if (not full_err.empty()) {
                std::runtime_error(std::move(full_err));
            }
        }
    };

    FullNames full_names;

    CppConfigWriterBase(Filenames filenames)
    : filenames(std::move(filenames))
    {}

    void do_init()
    {}

    int do_finish()
    {
        this->full_names.check();

        MultiFilenameWriter<CppConfigWriterBase> sw(*this);
        sw.then(filenames.authid_hpp, &write_authid_hpp)
          .then(filenames.str_authid_hpp, &write_str_authid_hpp)
          .then(filenames.variable_configuration_fwd, &write_variables_configuration_fwd)
          .then(filenames.variable_configuration_hpp, &write_variables_configuration)
          .then(filenames.config_set_value, &write_config_set_value)
          .then(filenames.ini_values_hpp, &write_ini_values)
          .then(filenames.max_str_buffer_size_hpp, &write_max_str_buffer_size_hpp)
        ;
        if (sw.err) {
            std::cerr << "CppConfigWriterBase: " << sw.filename << ": " << strerror(errno) << "\n";
            return sw.errnum;
        }
        return 0;
    }

    void do_start_section(Names const& section_names)
    {
        if (!section_names.cpp.empty()) {
            ++this->depth;
        }
        this->start_section_index = this->authid_policy.size();
    }

    void do_stop_section(Names const& section_names)
    {
        if (!section_names.cpp.empty()) {
            --this->depth;
        }
        this->sections.emplace_back(Section{section_names.cpp, this->out_member_.str(), std::move(this->members)});
        this->out_member_.str("");
        std::string str = this->out_body_parser_.str();
        if (!str.empty()) {
            this->sections_parser.emplace_back(section_names.cpp, str);
            this->out_body_parser_.str("");
        }
        this->start_indexes.emplace_back(this->authid_policy.size());
    }

    template<class Pack>
    void evaluate_member(Names const& section_names, Pack const & infos, type_enumerations& enums)
    {
        Names const& names = infos;
        std::string const& varname = names.cpp;

        auto type = get_t_elem<cfg_attributes::type_>(infos);
        using cpp_type_t = typename decltype(type)::type;
        this->members.push_back({varname, alignof(cpp_type_t)});

        std::string const & varname_with_section = section_names.cpp.empty()
            ? varname
            : str_concat(section_names.cpp, "::", varname);

        using sesman_io = sesman::internal::io;

        auto const properties = value_or<sesman_io_t>(infos, sesman::no_sesman).value;
        if (bool(/*PropertyFieldFlags::read & */properties)) {
            this->variables_acl.emplace_back(varname_with_section);
        }

        if constexpr (is_convertible_v<Pack, desc>) {
            this->out_member_ << cpp_doxygen_comment(cfg_attributes::desc(infos).value, 4);
        }
        std::string sesman_name;
        if (bool(properties)) {
            sesman_name = sesman_network_name(infos, section_names);
            this->authstrs.emplace_back(sesman_name);
            this->full_names.sesman.emplace_back(sesman_name);
        }
        this->out_member_ << "    /// type: "; write_type(this->out_member_, type); this->out_member_ << " <br/>\n";

        if ((properties & sesman_io::rw) == sesman_io::sesman_to_proxy) {
            if constexpr (!is_convertible_v<Pack, connection_policy_t>) {
                this->out_member_ << "    /// sesman ⇒ proxy <br/>\n";
            }
        }
        else if ((properties & sesman_io::rw) == sesman_io::proxy_to_sesman) {
            this->out_member_ << "    /// sesman ⇐ proxy <br/>\n";
        }
        else if ((properties & sesman_io::rw) == sesman_io::rw) {
            this->out_member_ << "    /// sesman ⇔ proxy <br/>\n";
        }

        if (!names.sesman.empty()) {
            this->out_member_ << "    /// sesman::name: " << sesman_name << " <br/>\n";
        }
        else if constexpr (is_convertible_v<Pack, connection_policy_t>) {
            this->out_member_ << "    /// connpolicy -> proxy";
            if (!names.connpolicy.empty() || is_convertible_v<Pack, connpolicy::section>) {
                this->out_member_ << "    [name: "
                    << value_or<connpolicy::section>(
                        infos, connpolicy::section{section_names.cpp.c_str()}).name
                    << "::" << names.connpolicy_name() << "]"
                ;
            }
            this->out_member_ << " <br/>\n";

            this->out_member_ << "    /// sesmanName: " << sesman_name << " <br/>\n";
        }

        constexpr bool has_default = is_t_convertible_v<Pack, default_>;
        this->out_member_ << "    /// default: ";
        if constexpr (has_default) {
            write_assignable_default(this->out_member_, enums, type, infos);
        }
        else if constexpr (std::is_same_v<cpp_type_t, bool>) {
            this->out_member_ << "false";
        }
        else {
            this->out_member_ << (is_convertible_v<cpp_type_t, types::integer_base> ? "0" : "{}");
        }
        this->out_member_ << " <br/>\n";
        this->out_member_ << "    struct " << varname_with_section << " {\n";
        this->out_member_ << "        static constexpr bool is_sesman_to_proxy = " << (bool(properties & sesman_io::sesman_to_proxy) ? "true" : "false") << ";\n";
        this->out_member_ << "        static constexpr bool is_proxy_to_sesman = " << (bool(properties & sesman_io::proxy_to_sesman) ? "true" : "false") << ";\n";

        if (bool(properties)) {
            this->out_member_ << "        // for old cppcheck\n";
            this->out_member_ << "        // cppcheck-suppress obsoleteFunctionsindex\n";
            this->out_member_ << "        static constexpr ::configs::authid_t index {"
                " ::configs::cfg_indexes::section" << this->sections.size() << " + "
                << (this->authid_policy.size() - this->start_section_index) << "};\n";
            this->authid_policy.emplace_back(infos);
        }

        this->out_member_ << "        using type = ";
        write_type(this->out_member_, type);
        this->out_member_ << ";\n";

        // write type
        if (bool(properties) || is_convertible_v<Pack, spec_attr_t>) {
            auto type_sesman = get_type<spec::type_>(infos);
            this->out_member_ << "        using sesman_and_spec_type = ";
            auto buf_size = write_type_spec(this->out_member_, type_sesman);
            if (bool(/*PropertyFieldFlags::read & */properties)) {
                this->max_str_buffer_size
                    = std::max(this->max_str_buffer_size, std::size_t(buf_size));
            }
            this->out_member_ << ";\n";
            this->out_member_ << "        using mapped_type = sesman_and_spec_type;\n";
        }
        else {
            this->out_member_ << "        using mapped_type = type;\n";
        }

        // write value
        this->out_member_ << "        type value { ";
        if constexpr (has_default) {
            write_assignable_default(this->out_member_, enums, type, infos);
        }
        this->out_member_ << " };\n";

        this->out_member_ << "    };\n";

        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            auto& ini_name = names.ini_name();
            this->full_names.spec.emplace_back(str_concat(section_names.cpp, ':', ini_name));
            auto type_spec = get_type<spec::type_>(infos);
            this->out_body_parser_ << "        else if (key == \"" << ini_name << "\"_zv) {\n"
            "            ::config_parse_and_log(\n"
            "                this->section_name, key.c_str(),\n"
            "                static_cast<cfg::" << varname_with_section << "&>(this->variables).value,\n"
            "                ::configs::spec_type<";
            write_type_spec(this->out_body_parser_, type_spec);
            this->out_body_parser_ << ">{},\n"
            "                value\n"
            "            );\n"
            "        }\n";
            str_append(this->cfg_values, "cfg::"_av, varname_with_section, ",\n"_av);
            str_append(this->cfg_str_values,
                "{\""_av, section_names.ini_name(), "\"_zv, \""_av,
                names.ini_name(), "\"_zv},\n"_av);
        }
    }
};


inline void write_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer)
{
    out_authid << cpp_comment(do_not_edit, 0) <<
      "\n"
      "#pragma once\n"
      "\n"
      "namespace configs\n"
      "{\n"
      "    enum class authid_t : unsigned;\n"
      "    constexpr authid_t max_authid = authid_t(" << writer.authstrs.size() << ");\n"
      "}\n"
    ;
}

inline void write_str_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer)
{
    out_authid << cpp_comment(do_not_edit, 0) <<
      "\n"
      "#pragma once\n"
      "\n"
      "#include \"utils/sugar/zstring_view.hpp\"\n"
      "#include \"configs/autogen/authid.hpp\"\n"
      "\n"
      "namespace configs\n"
      "{\n"
      "    constexpr zstring_view const authstr[] = {\n"
    ;
    for (auto & authstr : writer.authstrs) {
        out_authid << "        \"" << authstr << "\"_zv,\n";
    }
    out_authid <<
      "    };\n\n"
      "}\n"
    ;
}

inline void write_variables_configuration_fwd(std::ostream & out_varconf, CppConfigWriterBase& writer)
{
    out_varconf << cpp_comment(do_not_edit, 0) <<
        "\n"
        "#pragma once\n"
        "\n"
        "namespace cfg\n"
        "{\n"
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
    out_varconf << cpp_comment(do_not_edit, 0) <<
        "\n"
        "#pragma once\n"
        "\n"
        "#include \"configs/autogen/authid.hpp\"\n"
        "#include <cstdint>\n"
        "\n"
        "namespace configs\n"
        "{\n"
        "    template<class... Ts>\n"
        "    struct Pack\n"
        "    { static const std::size_t size = sizeof...(Ts); };\n\n"
        "    namespace cfg_indexes\n"
        "    {\n"
    ;

    {
        std::size_t i = 0;
        std::size_t previous_index = 0;
        for (auto n : writer.start_indexes) {
            out_varconf << "        ";
            if (previous_index == n) {
                out_varconf << "// ";
            }
            out_varconf
                << "inline constexpr int section" << i
                << " = " << previous_index << "; "
                   "/* " << writer.sections[i].section_name << " */\n"
            ;
            previous_index = n;
            ++i;
        }
    }

    out_varconf <<
        "    }\n"
        "}\n"
        "\n"
        "namespace cfg\n"
        "{\n"
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
    out_set_value << cpp_comment(do_not_edit, 0) <<
        "\n"
        "void Inifile::ConfigurationHolder::set_section(zstring_view section) {\n"
        "    if (0) {}\n"
    ;
    int id = 1;
    for (auto & body : writer.sections_parser) {
        out_set_value <<
            "    else if (section == \"" << body.first << "\"_zv) {\n"
            "        this->section_id = " << id << ";\n"
            "    }\n"
        ;
        ++id;
    }
    out_set_value <<
        "    else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
        "        LOG(LOG_WARNING, \"unknown section [%s]\", section);\n"
        "        this->section_id = 0;\n"
        "    }\n"
        "\n"
        "    this->section_name = section.c_str();\n"
        "}\n"
        "\n"
        "void Inifile::ConfigurationHolder::set_value(zstring_view key, zstring_view value) {\n"
        "    if (0) {}\n"
    ;
    id = 1;
    for (auto & body : writer.sections_parser) {
        out_set_value <<
            "    else if (this->section_id == " << id << ") {\n"
            "        if (0) {}\n" << body.second << "\n"
            "        else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
            "            LOG(LOG_WARNING, \"unknown parameter %s in section [%s]\",\n"
            "                key, this->section_name);\n"
            "        }\n"
            "    }\n"
        ;
        ++id;
    }
    out_set_value <<
        "    else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
        "        LOG(LOG_WARNING, \"unknown section [%s]\", this->section_name);\n"
        "    }\n"
        "}\n"
    ;
}

inline void write_ini_values(std::ostream& out, CppConfigWriterBase& writer)
{
    out << cpp_comment(do_not_edit, 0) <<
        "\n"
        "#pragma once\n"
        "\n"
        "#include \"configs/autogen/variables_configuration_fwd.hpp\"\n"
        "#include \"utils/sugar/zstring_view.hpp\"\n"
        "\n"
        "namespace configs::cfg_ini_infos {\n"
        "using IniPack = Pack<\n"
        // remove trailing comma
        << std::string_view(writer.cfg_values.data(), writer.cfg_values.size() - 2) <<
        "\n>;\n"
        "\n"
        "struct SectionAndName { zstring_view section; zstring_view name; };\n"
        "constexpr SectionAndName const ini_names[] = {\n"
        << writer.cfg_str_values <<
        "};\n"
        "}\n"
    ;
}

inline void write_max_str_buffer_size_hpp(std::ostream & out, CppConfigWriterBase& writer)
{
    out << cpp_comment(do_not_edit, 0) <<
        "\n"
        "#pragma once\n"
        "\n"
        "namespace configs {\n"
        "    inline constexpr std::size_t max_str_buffer_size = "
        << writer.max_str_buffer_size
        << ";\n}\n"
    ;
}

}

}
