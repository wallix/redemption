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
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"

#include "configs/attributes/spec.hpp"
#include "configs/generators/utils/multi_filename_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"
#include "configs/type_name.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/file_permissions.hpp"

#include "utils/file_permissions.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/bounded_array_view.hpp"
#include "utils/sugar/cast.hpp"

#include <bitset>
#include <charconv>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cerrno>
#include <cstring>


namespace cfg_generators
{

using Names = cfg_desc::names;

using namespace std::string_view_literals;
using namespace std::string_literals;

constexpr std::string_view begin_raw_string = "R\"gen_config_ini("sv;
constexpr std::string_view end_raw_string = ")gen_config_ini\""sv;

namespace types = cfg_desc::types;

using namespace std::string_literals;
using namespace cfg_desc;

// #if __has_include(<linux/limits.h>)
// # include <linux/limits.h>
// constexpr std::size_t path_max = PATH_MAX;
// #else
// constexpr std::size_t path_max = 4096;
constexpr auto path_max_as_str = "4096"_av;
// #endif

constexpr inline std::array conn_policies {
    DestSpecFile::rdp,
    DestSpecFile::jh,
    DestSpecFile::vnc,
};

constexpr bool contains_conn_policy(DestSpecFile dest)
{
    return bool(dest & ~(DestSpecFile::global_spec | DestSpecFile::ini_only));
}

constexpr std::string_view dest_file_to_filename(DestSpecFile dest)
{
    switch (dest) {
        case DestSpecFile::none: return ""sv;
        case DestSpecFile::ini_only: return ""sv;
        case DestSpecFile::global_spec: return ""sv;
        case DestSpecFile::rdp: return "rdp"sv;
        case DestSpecFile::vnc: return "vnc"sv;
        case DestSpecFile::jh: return "rdp-jumphost"sv;
    }
    return ""sv;
}


struct DataAsStrings
{
    struct ConnectionPolicy
    {
        DestSpecFile dest_file;
        std::string py;
        bool forced;
    };

    std::string py;
    std::string ini;
    std::string cpp = py;
    std::string cpp_comment = cpp;
    std::vector<ConnectionPolicy> connection_policies {};

    template<class Int>
    static DataAsStrings from_integer(Int value)
    {
        auto d = int_to_decimal_chars(value);
        return DataAsStrings::all(std::string(d.sv()));
    }

    static DataAsStrings all(std::string value)
    {
        return {
            .py = value,
            .ini = std::move(value),
        };
    }

    static DataAsStrings quoted(std::string value)
    {
        DataAsStrings r {
            .py = {},
            .ini = std::move(value),
        };

        r.py.reserve(value.size() * 2 + 2);
        r.py += '"';
        for (char c : r.ini) {
            if ('"' == c || '\\' == c) {
                r.py += '\\';
            }
            r.py += c;
        }
        r.py += '"';

        if (r.py.size() > 2) {
            r.cpp = r.py;
        }
        r.cpp_comment = r.py;

        return r;
    }

    static DataAsStrings unspaced_quote(std::string s)
    {
        return {
            .py = str_concat('"', s, '"'),
            .ini = std::move(s),
        };
    }
};

inline /*constexpr*/ DataAsStrings novalue_to_string {
    .py = "\"\"",
    .ini = {},
    .cpp = {},
    .cpp_comment = "\"\"",
};

inline DataAsStrings string_value_to_strings(types::dirpath)
{
    return novalue_to_string;
}

template<class T>
DataAsStrings string_value_to_strings(types::list<T>)
{
    return novalue_to_string;
}

template<unsigned n>
DataAsStrings string_value_to_strings(types::fixed_string<n>)
{
    return novalue_to_string;
}

inline std::string cpp_expr_to_string(cpp::expr expr)
{
    return str_concat(end_raw_string, " << ("_av, expr.value, ") << "_av, begin_raw_string);
}

inline DataAsStrings string_value_to_strings(cpp::expr expr)
{
    auto s = cpp_expr_to_string(expr);
    return {
        .py = str_concat('"', s, '"'),
        .ini = std::move(s),
        .cpp = expr.value,
    };
}

inline DataAsStrings string_value_to_strings(std::string_view str)
{
    return DataAsStrings::quoted(std::string(str));
}

template<std::size_t N>
inline DataAsStrings binary_string_value_to_strings(sized_array_view<char, N> av)
{
    char* p;

    constexpr std::size_t py_len = N*4 + 2;
    char py[py_len];
    p = py;
    *p++ = '"';
    for (char c : av) {
        *p++ = '\\';
        *p++ = 'x';
        p = int_to_fixed_hexadecimal_upper_chars(p, static_cast<unsigned char>(c));
    }
    *p++ = '"';

    constexpr std::size_t cpp_len = N*6 + 2;
    char cpp[cpp_len];
    p = cpp;
    *p++ = '{';
    for (char c : av) {
        *p++ = '0';
        *p++ = 'x';
        p = int_to_fixed_hexadecimal_upper_chars(p, static_cast<unsigned char>(c));
        *p++ = ',';
        *p++ = ' ';
    }
    *p++ = '}';

    constexpr std::size_t ini_len = N*2;
    char ini[ini_len];
    p = ini;
    for (char c : av) {
        p = int_to_fixed_hexadecimal_upper_chars(p, static_cast<unsigned char>(c));
    }

    return {
        .py = std::string(py, py_len),
        .ini = std::string(ini, ini_len),
        .cpp = std::string(cpp, cpp_len),
    };
}


inline DataAsStrings integer_value_to_strings(bool) = delete;

inline DataAsStrings integer_value_to_strings(cpp::expr expr)
{
    auto s = cpp_expr_to_string(expr);
    return {
        .py = s,
        .ini = std::move(s),
        .cpp = expr.value,
    };
}

template<class T>
inline DataAsStrings integer_value_to_strings(T const& value)
{
    if constexpr (std::is_base_of_v<impl::integer_base, T>) {
        return {
            .py = "0",
            .ini = "0",
        };
    }
    else {
        return DataAsStrings::from_integer(value);
    }
}

template<class T, class Ratio>
DataAsStrings integer_value_to_strings(std::chrono::duration<T, Ratio> value)
{
    if (value.count()) {
        return DataAsStrings::from_integer(value.count());
    }
    return {
        .py = "0",
        .ini = "0",
        .cpp = {},
        .cpp_comment = "0",
    };
}

template<class Int, long min, long max>
DataAsStrings integer_value_to_strings(types::range<Int, min, max>)
{
    static_assert(!min, "unspecified value but 'min' isn't 0");
    return integer_value_to_strings(Int());
}

inline DataAsStrings bool_value_to_strings(bool value)
{
    if (value) {
        return {
            .py = "True",
            .ini = "1",
            .cpp = "true",
        };
    }
    else {
        return {
            .py = "False",
            .ini = "0",
            .cpp = "false",
        };
    }
}

inline DataAsStrings bool_value_to_strings(cpp::expr expr)
{
    auto s = cpp_expr_to_string(expr);
    return {
        .py = s,
        .ini = std::move(s),
        .cpp = expr.value,
    };
}

inline DataAsStrings color_value_to_strings(types::rgb) = delete;

inline DataAsStrings color_value_to_strings(uint32_t color)
{
    assert(!(color >> 24));

    char data[7];
    char* p = data;
    *p++ = '#';
    auto puthex = [&](uint32_t n){ *p++ = "0123456789ABCDEF"[n & 0xf]; };
    puthex(color >> 20);
    puthex(color >> 16);
    puthex(color >> 12);
    puthex(color >> 8);
    puthex(color >> 4);
    puthex(color >> 0);

    auto s = std::string_view(data, 7);

    return {
        .py = str_concat('"', s, '"'),
        .ini = std::string(s),
        .cpp = str_concat("0x"_av, std::string_view(data+1, 6)),
    };
}

inline DataAsStrings file_permission_value_to_strings(unsigned permissions)
{
    std::array<char, 16> d;
    d[0] = '0';
    char* p = d.data() + 1;
    auto r = std::to_chars(p, d.end(), permissions, 8);
    auto len = static_cast<std::size_t>(r.ptr - p);

    return {
        .py = str_concat('"', std::string_view(p, len), '"'),
        .ini = std::string(p, len),
        .cpp = std::string(d.data(), len + 1),
    };
}


struct ValueAsStrings
{
    std::string prefix_spec_type;
    std::string cpp_type;
    std::string spec_type = cpp_type;
    std::string acl_diag_type = cpp_type;
    std::size_t spec_str_buffer_size;
    DataAsStrings values;
    std::string spec_note = {};
    std::string ini_note = spec_note;
    std::string acl_diag_note = spec_note;
    std::string cpp_note = {};
    std::size_t align_of = 0;
    const type_enumeration* enumeration = nullptr;
    bool string_parser_for_enum = false;
};

// "fix" -Wmissing-braces with clang
struct MemberNames : cfg_desc::names
{
    MemberNames(char const* str)
        : cfg_desc::names{str}
    {}

    MemberNames(std::string_view str)
        : cfg_desc::names{str}
    {}

    MemberNames(cfg_desc::names names)
        : cfg_desc::names{names}
    {}
};


enum class PrefixType : uint8_t
{
    Unspecified,
    NoPrefix,
    ForceDisable,
};

struct MemberInfo
{
    MemberNames name;
    std::string_view connpolicy_section {};
    ValueAsStrings value;
    SpecInfo spec;
    cfg_desc::TagList tags {};
    PrefixType prefix_type = PrefixType::Unspecified;
    std::string_view desc {};
};


namespace cpp_config_writer
{

using namespace cfg_desc;

struct HtmlEntitifier
{
    struct OutputData
    {
        friend std::ostream& operator<<(std::ostream& out, OutputData const& data)
        {
            for (char c : data.str) {
                if (c == '<') {
                    out << "&lt;";
                }
                else {
                    out << c;
                }
            }
            return out;
        }

        std::string str;
    };

    template<class F>
    OutputData operator()(F&& f)
    {
        out_string.str("");
        f(out_string);
        return OutputData{out_string.str()};
    }

private:
    std::ostringstream out_string;
};


struct CppConfigWriterBase;

void write_acl_and_spec_type(std::ostream & out, CppConfigWriterBase& writer);
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
    HtmlEntitifier html_entitifier;

    struct Member
    {
        std::string_view name;
        std::size_t align_of;

        friend std::ostream& operator <<(std::ostream& out, Member const& x)
        {
            return out << x.name;
        }
    };

    struct Section
    {
        std::string_view section_name;
        std::string member_struct;
        std::vector<Member> members;
    };

    struct SectionString
    {
        std::string_view section_name;
        std::string body;
    };

    std::vector<SectionString> sections_parser;
    std::vector<std::string> authstrs;
    std::vector<std::string> acl_and_connpolicy_strs;
    std::vector<Section> sections;
    std::vector<Member> members;
    std::vector<std::string> variables_acl;
    std::vector<Loggable> authid_policies;
    std::vector<Loggable> acl_and_connpolicy_policies;
    std::size_t start_section_index = 0;
    std::vector<std::size_t> start_indexes;
    std::string cfg_values;
    std::string cfg_str_values;
    std::size_t max_str_buffer_size = 0;
    std::ostringstream out_acl_and_spec_types;

    struct Filenames
    {
        std::string authid_hpp;
        std::string str_authid_hpp;
        std::string variable_configuration_fwd;
        std::string variable_configuration_hpp;
        std::string config_set_value;
        std::string ini_values_hpp;
        std::string acl_and_spec_type;
        std::string max_str_buffer_size_hpp;
    };
    Filenames filenames;

    struct FullNames
    {
        FullNameMemberChecker spec;
        FullNameMemberChecker acl;

        void check()
        {
            std::string full_err;

            struct P { char const* name; FullNameMemberChecker& checker; };
            for (P const& p : {
                P{"Ini", this->spec},
                P{"Sesman", this->acl},
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
};


template<class Cont, class Before, class After>
inline void join_with_comma(
    std::ostream& out, Cont const& cont,
    Before const& before, After const& after)
{
    auto first = begin(cont);
    auto last = end(cont);
    if (first == last) {
        return ;
    }
    out << before << *first << after;
    while (++first != last) {
        out << ", " << before << *first << after;
    }
}

inline void write_loggable_bitsets(
    std::ostream& out, array_view<Loggable> policies,
    std::string_view loggable_name)
{
    std::vector<std::bitset<64>> loggables;
    std::vector<std::bitset<64>> unloggable_if_value_contains_passwords;
    size_t i = 0;

    for (auto log_policy : policies)
    {
        if ((i % 64) == 0) {
            i = 0;
            loggables.push_back(0);
            unloggable_if_value_contains_passwords.push_back(0);
        }

        switch (log_policy) {
            case Loggable::Yes:
                loggables.back().set(i);
                break;
            case Loggable::No:
                break;
            case Loggable::OnlyWhenContainsPasswordString:
                unloggable_if_value_contains_passwords.back().set(i);
                break;
        }
        ++i;
    }

    out << "constexpr U64BitFlags<" << loggables.size() << "> " << loggable_name << "{ {\n  ";
    join_with_comma(out, loggables, "0b", "\n");
    out << "},\n{\n  ";
    join_with_comma(out, unloggable_if_value_contains_passwords, "0b", "\n");
    out << "} };\n";
}

inline void write_authid_hpp(std::ostream & out_authid, CppConfigWriterBase& writer)
{
    out_authid << cpp_comment(do_not_edit, 0) <<
      "\n"
      "#pragma once\n"
      "\n"
      "namespace configs\n"
      "{\n"
      "    enum class authid_t : unsigned;\n"
      "    constexpr authid_t max_authid {" << writer.authstrs.size() << "};\n"
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
      "#include \"configs/loggable.hpp\"\n"
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
      "\n"
      "    // value from connpolicy but not used by the proxy\n"
      "    constexpr zstring_view const unused_connpolicy_authstr[] = {\n"
    ;
    for (auto & authstr : writer.acl_and_connpolicy_strs) {
        out_authid << "        \"" << authstr << "\"_zv,\n";
    }
    out_authid <<
      "    };\n\n"
      "\n"
    ;
    write_loggable_bitsets(
        out_authid, writer.acl_and_connpolicy_policies, "unused_connpolicy_loggable");
    out_authid <<
      "} // namespace configs\n"
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
        else if (!section.members.empty()) {
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
        "#include \"configs/loggable.hpp\"\n"
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
        "    } // namespace cfg_indexes\n"
        "} // namespace configs\n"
        "\n"
        "namespace cfg\n"
        "{\n"
    ;

    for (auto & section : writer.sections) {
        out_varconf << section.member_struct << "\n";
    }

    auto join = [&](auto const & cont, auto const & before, auto const & after) {
        join_with_comma(out_varconf, cont, before, after);
    };

    std::vector<std::string> section_names;

    out_varconf <<
        "} // namespace cfg\n\n"
        "namespace cfg_section {\n"
    ;
    using Member = typename CppConfigWriterBase::Member;
    auto has_values = [](CppConfigWriterBase::Section const& body) {
        return !body.section_name.empty() && !body.members.empty();
    };
    for (auto & body : writer.sections) {
        if (has_values(body)) {
            std::vector<std::reference_wrapper<Member>> v(body.members.begin(), body.members.end());
            std::stable_sort(v.begin(), v.end(), [](Member& a, Member& b){
                return a.align_of > b.align_of;
            });
            section_names.emplace_back(str_concat("cfg_section::"sv, body.section_name));
            out_varconf << "struct " << body.section_name << "\n: ";
            join(v, str_concat("cfg::"sv, body.section_name, "::"sv), "\n");
            out_varconf << "{ static constexpr bool is_section = true; };\n\n";
        }
    }
    out_varconf << "} // namespace cfg_section\n\n"
      "namespace configs {\n"
      "struct VariablesConfiguration\n"
      ": "
    ;
    join(section_names, "", "\n");
    auto it = std::find_if(begin(writer.sections), end(writer.sections),
        [&](CppConfigWriterBase::Section const& body) { return !has_values(body); });
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

    write_loggable_bitsets(out_varconf, writer.authid_policies, "loggable_field");

    out_varconf << "} // namespace configs\n";
}

inline void write_config_set_value(std::ostream & out_set_value, CppConfigWriterBase& writer)
{
    out_set_value << cpp_comment(do_not_edit, 0) <<
        "\n"
        "void Inifile::ConfigurationHolder::set_section(zstring_view section) {\n"
        "    if (0) {}\n"
    ;
    int id = 1;
    for (auto & section : writer.sections_parser) {
        out_set_value <<
            "    else if (section == \"" << section.section_name << "\"_zv) {\n"
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
    for (auto & section : writer.sections_parser) {
        // all members are attr::external
        out_set_value <<
            "    else if (this->section_id == " << id << ") {\n"
        ;
        if (!section.body.empty()) {
            out_set_value <<
                "        if (0) {}\n" << section.body << "\n"
                "        else if (static_cast<cfg::debug::config>(this->variables).value) {\n"
                "            LOG(LOG_WARNING, \"unknown parameter %s in section [%s]\",\n"
                "                key, this->section_name);\n"
                "        }\n"
            ;
        }
        else {
            out_set_value <<
                "        // all members are external\n"
            ;
        }
        out_set_value <<
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
        "} // namespace configs::cfg_ini_infos\n"
    ;
}

inline void write_acl_and_spec_type(std::ostream & out, CppConfigWriterBase& writer)
{
    out << cpp_comment(do_not_edit, 0) <<
        "\n"
        "#pragma once\n"
        "\n"
        "#include \"configs/autogen/variables_configuration_fwd.hpp\"\n"
        "\n"
        "namespace configs {\n"
        "template<class Cfg> struct acl_and_spec_type {};\n"
        "\n"
        << writer.out_acl_and_spec_types.str()
        << "\n}\n"
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


template<class TInt>
inline constexpr std::size_t integral_buffer_size_v
  = std::numeric_limits<std::enable_if_t<std::is_integral_v<TInt>, TInt>>::digits10
    + 1 + std::numeric_limits<TInt>::is_signed;

template<class T, class Ratio>
inline constexpr std::size_t integral_buffer_size_v<std::chrono::duration<T, Ratio>> = integral_buffer_size_v<T>;

template<>
inline constexpr std::size_t integral_buffer_size_v<types::unsigned_> = integral_buffer_size_v<unsigned>;

template<>
inline constexpr std::size_t integral_buffer_size_v<types::int_> = integral_buffer_size_v<int>;

template<>
inline constexpr std::size_t integral_buffer_size_v<types::u16> = integral_buffer_size_v<uint16_t>;

template<>
inline constexpr std::size_t integral_buffer_size_v<types::u32> = integral_buffer_size_v<uint32_t>;

template<>
inline constexpr std::size_t integral_buffer_size_v<types::u64> = integral_buffer_size_v<uint64_t>;

template<class T>
struct type_
{
    using type = T;
};

template<class T, class V>
ValueAsStrings compute_value_as_strings(type_<T>, V const& value)
{
    if constexpr (std::is_same_v<T, std::string>) {
        return {
            .prefix_spec_type = "string("s,
            .cpp_type = "std::string"s,
            .spec_str_buffer_size = 0,
            .values = string_value_to_strings(value),
        };
    }
    else if constexpr (std::is_same_v<T, bool>) {
        return {
            .prefix_spec_type = "boolean("s,
            .cpp_type = "bool"s,
            .spec_str_buffer_size = 5,
            .values = bool_value_to_strings(value),
            .ini_note = "type: boolean (0/no/false or 1/yes/true)"s,
        };
    }
    else if constexpr (std::is_same_v<T, types::dirpath>) {
        return {
            .prefix_spec_type = str_concat("string(max="_av, path_max_as_str, ", "_av),
            .cpp_type = "::configs::spec_types::directory_path"s,
            .spec_str_buffer_size = 0,
            .values = string_value_to_strings(value),
            .ini_note = str_concat("maxlen = "_av, path_max_as_str),
        };
    }
    else if constexpr (std::is_same_v<T, types::ip_string>) {
        return {
            .prefix_spec_type = "ip_addr("s,
            .cpp_type = "std::string"s,
            .spec_type = "::configs::spec_types::ip"s,
            .spec_str_buffer_size = 0,
            .values = string_value_to_strings(value),
        };
    }
    else if constexpr (std::is_same_v<T, types::rgb>) {
        return {
            .prefix_spec_type = "string("s,
            .cpp_type = "::configs::spec_types::rgb"s,
            .spec_str_buffer_size = 7,
            .values = color_value_to_strings(value),
            .spec_note = "in rgb format: hexadecimal (0x21AF21), #rgb (#2fa) or #rrggbb (#22ffaa)"s,
        };
    }
    else if constexpr (std::is_same_v<T, FilePermissions>) {
        return {
            .prefix_spec_type = "string("s,
            .cpp_type = "FilePermissions"s,
            .spec_str_buffer_size = integral_buffer_size_v<uint16_t>,
            .values = file_permission_value_to_strings(value),
            .spec_note = "in octal or symbolic mode format (as chmod Linux command)"s,
        };
    }
    else if constexpr (std::is_base_of_v<impl::unsigned_base, T>) {
        return {
            .prefix_spec_type = "integer(min=0, "s,
            .cpp_type = std::string(type_name<T>()),
            .spec_str_buffer_size = integral_buffer_size_v<T>,
            // TODO hexconverter
            .values = integer_value_to_strings(value),
            .ini_note = "min = 0"s,
        };
    }
    else if constexpr (std::is_base_of_v<impl::signed_base, T>) {
        return {
            .prefix_spec_type = "integer("s,
            .cpp_type = std::string(type_name<T>()),
            .spec_str_buffer_size = integral_buffer_size_v<T>,
            // TODO hexconverter
            .values = integer_value_to_strings(value),
        };
    }
    // TODO remove that when redirection_password_or_cookie is removed
    else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
        return {
            .prefix_spec_type = {},
            .cpp_type = "std::vector<uint8_t>"s,
            .spec_str_buffer_size = 0,
            .values = {
                .py = {},
                .ini = {},
                .cpp = {},
            },
        };
    }
    else {
        static_assert(!sizeof(T), "missing implementation");
        return {};
    }
}

template<class T, std::intmax_t Num, std::intmax_t Denom, class V>
ValueAsStrings compute_value_as_strings(
    type_<std::chrono::duration<T, std::ratio<Num, Denom>>>, V const& value)
{
    std::string_view note;

    if constexpr (Num == 1 && Denom == 1000) {
        note = "in milliseconds";
    }
    else if constexpr (Num == 1 && Denom == 100) {
        static_assert(std::is_same_v<T, unsigned>);
        note = "in 1/100 seconds";
    }
    else if constexpr (Num == 1 && Denom == 10) {
        static_assert(std::is_same_v<T, unsigned>);
        note = "in 1/10 seconds";
    }
    else if constexpr (Num == 1 && Denom == 1) {
        note = "in seconds";
    }
    else if constexpr (Num == 60 && Denom == 1) {
        note = "in minutes";
    }
    else if constexpr (Num == 3600 && Denom == 1) {
        note = "in hours";
    }
    else {
        static_assert(!Num && !Denom, "missing implementation");
    }

    return {
        .prefix_spec_type = "integer(min=0, "s,
        .cpp_type = std::string(type_name<std::chrono::duration<T, std::ratio<Num, Denom>>>()),
        .spec_str_buffer_size = integral_buffer_size_v<T>,
        .values = integer_value_to_strings(value),
        .spec_note = std::string(note),
        .acl_diag_note = {},
    };
}

template<unsigned N, class V>
ValueAsStrings compute_value_as_strings(type_<types::fixed_string<N>>, V const& value)
{
    auto d = int_to_decimal_chars(N);
    return {
        .prefix_spec_type = str_concat("string(max="_av, d, ", "_av),
        .cpp_type = str_concat("char["_av, d, "+1]"_av),
        .spec_type = "::configs::spec_types::fixed_string"s,
        .acl_diag_type = str_concat("std::string(maxlen="_av, d, ")"_av),
        .spec_str_buffer_size = 0,
        .values = string_value_to_strings(value),
        .ini_note = str_concat("maxlen = "_av, d),
    };
}

template<unsigned N, class V>
ValueAsStrings compute_value_as_strings(type_<types::fixed_binary<N>>, V const& value)
{
    auto d = int_to_decimal_chars(N);
    return {
        .prefix_spec_type = str_concat("string(min="_av, d, ", max="_av, d, ", "_av),
        .cpp_type = str_concat("std::array<unsigned char, "_av, d, '>'),
        .spec_type = "::configs::spec_types::fixed_binary"s,
        .spec_str_buffer_size = N * 2,
        .values = binary_string_value_to_strings<N>(value),
        .spec_note = "in hexadecimal format"s,
        .ini_note = str_concat("hexadecimal string of length "_av, d),
    };
}

template<class T, class V>
ValueAsStrings compute_value_as_strings(type_<types::list<T>>, V const& value)
{
    return {
        .prefix_spec_type = "string(",
        .cpp_type = "std::string",
        .spec_type = str_concat("::configs::spec_types::list<"_av, type_name<T>(), '>'),
        .spec_str_buffer_size = 0,
        .values = string_value_to_strings(value),
        .spec_note = "values are comma-separated"s
    };
}

template<class T, class V>
ValueAsStrings compute_value_as_strings(type_<types::megabytes<T>>, V const& value)
{
    auto res = compute_value_as_strings(type_<T>(), value);
    auto note = "in megabytes"sv;
    res.spec_note = std::string(note);
    res.ini_note = std::string(note);
    res.acl_diag_note = std::string(note);
    res.cpp_note = std::string(note);
    return res;
}

template<class Int, long min, long max, class V>
ValueAsStrings compute_value_as_strings(type_<types::range<Int, min, max>>, V const& value)
{
    std::string spec_note;
    chars_view sep = ""_av;

    auto smin = int_to_decimal_chars(min);
    auto smax = int_to_decimal_chars(max);

    if constexpr (!std::is_base_of_v<impl::unsigned_base, Int>) {
        spec_note = compute_value_as_strings(type_<Int>(), Int()).ini_note;
        if (!spec_note.empty()) {
            sep = " | "_av;
        }
    }

    return {
        .prefix_spec_type = str_concat("integer(min="_av, smin, ", max="_av, smax, ", "_av),
        .cpp_type = std::string(type_name<Int>()),
        .spec_type = str_concat("::configs::spec_types::range<"_av, type_name<Int>(), ", "_av, smin, ", "_av, smax, ">"_av),
        .spec_str_buffer_size = integral_buffer_size_v<Int>,
        // TODO hexconverter
        .values = integer_value_to_strings(value),
        .spec_note = spec_note,
        .ini_note = str_concat(spec_note, sep, "min = "_av, smin, ", max = "_av, smax),
    };
}


inline std::string enum_options_to_prefix_spec_type(type_enumeration const& e)
{
    if (e.cat != type_enumeration::Category::autoincrement
        && e.cat != type_enumeration::Category::set
    ) {
        throw std::runtime_error("enum parser available only with set or autoincrement enum");
    }

    std::string ret;
    ret.reserve(127);
    ret += "option("sv;
    for (type_enumeration::value_type const & v : e.values) {
        if (!v.exclude) {
            str_append(ret, '\'', v.get_name(), "', "_av);
        }
    }
    return ret;
}

inline bool enum_values_has_desc(type_enumeration const& e)
{
    return std::any_of(begin(e.values), end(e.values),
        [](type_enumeration::value_type const & v) { return !v.desc.empty(); });
}

inline std::string enum_desc_with_values_in_desc(type_enumeration const& e, bool use_disable_prefix)
{
    std::string str;
    str.reserve(128);
    for (type_enumeration::value_type const & v : e.values) {
        if (!v.exclude) {
            str += "  ";
            str += v.get_name();
            str += ": ";
            if (use_disable_prefix) {
                str += "disable ";
            }
            str += v.desc;
            str += '\n';
        }
    }
    str += e.info;
    return str;
}

inline std::string enum_as_string_ini_desc(type_enumeration const& e)
{
    std::string str;
    str.reserve(128);
    std::string_view prefix = "values: ";
    for (type_enumeration::value_type const & v : e.values) {
        if (!v.exclude) {
            str += prefix;
            str += v.get_name();
            prefix = ", ";
        }
    }
    str += e.info;
    return str;
}

inline std::string_view get_enum_value_data(uint64_t value, type_enumeration const& e)
{
    for (auto& v : e.values) {
        if (v.val == value) {
            return v.get_name();
        }
    }
    throw std::runtime_error(str_concat(
        "unknown value "_av, int_to_decimal_chars(value), " for "_av, e.name
    ));
}

inline ValueAsStrings compute_string_enum_as_strings(uint64_t value, type_enumeration const& e)
{
    auto name = get_enum_value_data(value, e);
    return {
        .prefix_spec_type = enum_options_to_prefix_spec_type(e),
        .cpp_type = std::string(e.name),
        .spec_type = "std::string"s,
        .acl_diag_type = "std::string"s,
        .spec_str_buffer_size = 0,
        .values = {
            .py = str_concat('"', name, '"'),
            .ini = std::string(name),
            .cpp = str_concat(e.name, "::"_av, name),
        },
        .enumeration = &e,
        .string_parser_for_enum = true,
    };
}


inline std::string enum_to_prefix_spec_type(type_enumeration const& e)
{
    switch (e.cat) {
        case type_enumeration::Category::flags:
            return str_concat("integer(min=0, max="_av, int_to_decimal_chars(e.max()), ", "_av);

        case type_enumeration::Category::autoincrement:
        case type_enumeration::Category::set:
            break;
    }

    std::string ret;
    ret.reserve(63);
    ret += "option(";
    for (type_enumeration::value_type const & v : e.values) {
        if (!v.exclude) {
            str_append(ret, int_to_decimal_chars(v.val), ", "_av);
        }
    }
    return ret;
}

inline std::string numeric_enum_desc(type_enumeration const& e, bool use_disable_prefix)
{
    std::string str;
    std::size_t ndigit16 = (e.cat == type_enumeration::Category::flags)
      ? static_cast<std::size_t>(64 - __builtin_clzll(e.max()) + 4 - 1) / 4
      : 0;

    auto show_values = [&](bool hexa)
    {
        for (type_enumeration::value_type const & v : e.values) {
            if (!v.exclude) {
                str += "  ";
                if (hexa) {
                    auto buf = int_to_fixed_hexadecimal_lower_chars(v.val);
                    str += "0x";
                    str += chars_view(buf).last(ndigit16).as<std::string_view>();
                }
                else {
                    str += int_to_decimal_chars(v.val).sv();
                }

                str += ": ";

                // skip "disable none" text
                if (use_disable_prefix && (!v.desc.empty() || v.val)) {
                    str += "disable ";
                }

                if (!v.desc.empty()) {
                    str += v.desc;
                }
                else {
                    for (char c : v.get_name()) {
                        str += ('_' == c ? ' ' : c);
                    }
                }

                str += '\n';
            }
        }
    };

    switch (e.cat) {
        case type_enumeration::Category::set:
        case type_enumeration::Category::autoincrement:
            show_values(false);
            break;

        case type_enumeration::Category::flags: {
            show_values(true);
            std::string note;
            uint64_t total = 0;
            for (type_enumeration::value_type const & v : e.values) {
                if (!v.exclude) {
                    if (v.val) {
                        total |= v.val;
                        note += "0x";
                        note += int_to_hexadecimal_lower_chars(v.val);
                        note += " + ";
                    }
                }
            }

            note[note.size() - 2] = '=';
            str_append(str, "\nNote: values can be added ("_av,
                (use_disable_prefix ? "disable all: "_av : "enable all: "_av),
                note, "0x"_av, int_to_hexadecimal_lower_chars(total), ')');
        }
    }

    str += e.info;
    return str;
}

inline std::string enum_to_cpp_string(
    uint64_t value, type_enumeration const& e, int_to_chars_result const& int_converted)
{
    try {
        return str_concat(e.name, "::"_av, get_enum_value_data(value, e));
    }
    catch (std::exception const&) {
        if (e.cat != type_enumeration::Category::flags) {
            throw;
        }
        // ignore error and convert to integer
    }
    return str_concat(e.name, '{', int_converted, '}');
}

inline ValueAsStrings compute_integer_enum_as_strings(uint64_t value, type_enumeration const& e)
{
    auto d = int_to_decimal_chars(value);
    return {
        .prefix_spec_type = enum_to_prefix_spec_type(e),
        .cpp_type = std::string(e.name),
        .spec_str_buffer_size = integral_buffer_size_v<uint64_t>,
        .values = {
            .py = std::string(d.sv()),
            .ini = std::string(d.sv()),
            .cpp = enum_to_cpp_string(value, e, d),
        },
        .enumeration = &e,
    };
}


template<class T>
struct ConnectionPolicyValue
{
    DestSpecFile policy_type;
    T value;
    bool forced;

    ConnectionPolicyValue<T> always()
    {
        return {policy_type, static_cast<T&&>(value), true};
    }
};

template<class T>
ConnectionPolicyValue<std::decay_t<T>> rdp_policy_value(T&& value)
{
    return {DestSpecFile::rdp, static_cast<T&&>(value), false};
}

template<class T>
ConnectionPolicyValue<std::decay_t<T>> vnc_policy_value(T&& value)
{
    return {DestSpecFile::vnc, static_cast<T&&>(value), false};
}

template<class T>
ConnectionPolicyValue<std::decay_t<T>> jh_policy_value(T&& value)
{
    return {DestSpecFile::jh, static_cast<T&&>(value), false};
}

inline void check_policy(std::initializer_list<DestSpecFile> dest_files)
{
    auto policy_type = DestSpecFile();
    for (auto dest_file : dest_files) {
        if (!contains_conn_policy(dest_file)) {
            throw std::runtime_error("connection policy value without name");
        }
        if (bool(policy_type & dest_file)) {
            std::string_view dest_name;
            switch (policy_type & dest_file) {
                case DestSpecFile::none: break;
                case DestSpecFile::ini_only: break;
                case DestSpecFile::global_spec: break;
                case DestSpecFile::rdp: dest_name = " (rdp)"; break;
                case DestSpecFile::vnc: dest_name = " (vnc)"; break;
                case DestSpecFile::jh: dest_name = " (jh)"; break;
            }
            throw std::runtime_error(
                str_concat("duplicate connection policy value", dest_name)
            );
        }
        policy_type |= dest_file;
    }
}

struct EnumAsString
{
    type_enumerations const& tenums;

    template<class Enum, class... TConnPolicy>
    ValueAsStrings operator()(Enum value, TConnPolicy&&... conn_policy_value)
    {
        static_assert(std::is_enum_v<Enum>);

        check_policy({conn_policy_value.policy_type...});

        auto& e = tenums.get_enum<Enum>();
        auto ret = compute_string_enum_as_strings(safe_int(underlying_cast(value)), e);

        (..., (ret.values.connection_policies.push_back({
            conn_policy_value.policy_type,
            compute_string_enum_as_strings(
                safe_int(underlying_cast(Enum{conn_policy_value.value})),
                e
            ).values.py,
            conn_policy_value.forced
        })));

        ret.align_of = alignof(Enum);
        return ret;
    }
};

struct ValueFromEnum
{
    type_enumerations const& tenums;

    template<class Enum, class... TConnPolicy>
    ValueAsStrings operator()(Enum value, TConnPolicy&&... conn_policy_value)
    {
        static_assert(std::is_enum_v<Enum>);

        check_policy({conn_policy_value.policy_type...});

        auto& e = tenums.get_enum<Enum>();
        auto ret = compute_integer_enum_as_strings(safe_int(underlying_cast(value)), e);

        (..., (ret.values.connection_policies.push_back({
            conn_policy_value.policy_type,
            compute_integer_enum_as_strings(
                safe_int(underlying_cast(Enum{conn_policy_value.value})),
                e
            ).values.py,
            conn_policy_value.forced
        })));

        ret.align_of = alignof(Enum);
        return ret;
    }
};

template<class T, class U = T, class... TConnPolicy>
ValueAsStrings value(U const& value = T(), TConnPolicy const&... conn_policy_value)
{
    static_assert(!std::is_enum_v<T>, "uses EnumAsString or ValueFromEnum");
    TYPE_REQUIEREMENT(T);

    check_policy({conn_policy_value.policy_type...});

    auto ret = compute_value_as_strings(type_<T>(), value);

    (..., (ret.values.connection_policies.push_back({
        conn_policy_value.policy_type,
        compute_value_as_strings(type_<T>(), conn_policy_value.value).values.py,
        conn_policy_value.forced
    })));

    ret.align_of = alignof(T);
    return ret;
}


inline constexpr std::string_view workaround_message = "⚠ The use of this feature is not recommended!\n\n";

inline std::string_view get_warning_attr(SpecAttributes attr)
{
    if (bool(attr & SpecAttributes::restart_service)) {
        return "⚠ Service will be automatically restarted and active sessions will be disconnected.\n\n";
    }

    if (bool(attr & SpecAttributes::iptables)) {
        return "⚠ IP tables rules are reloaded and active sessions will be disconnected.\n\n";
    }

    return {};
}

inline void add_attr_spec(std::string& out, std::string_view image_path, SpecAttributes attr)
{
    if (bool(attr & SpecAttributes::iptables)) out += "#_iptables\n"sv;
    if (bool(attr & SpecAttributes::advanced)) out += "#_advanced\n"sv;
    if (bool(attr & SpecAttributes::hex))      out += "#_hex\n"sv;
    if (bool(attr & SpecAttributes::password)) out += "#_password\n"sv;
    if (bool(attr & SpecAttributes::image))    str_append(out, "#_image="sv, image_path, '\n');
    if (bool(attr & SpecAttributes::restart_service)) out += "#_adminkit\n"sv;
    if (bool(attr & SpecAttributes::logged))   out += "#_logged\n"sv;
}

inline void add_prefered_display_name(std::string& out, Names const& names)
{
    if (!names.display.empty()) {
        out += "#_display_name="sv;
        out += names.display;
        out += '\n';
    }
}

inline void add_comment(std::string& out, std::string_view str, std::string_view comment)
{
    if (str.empty()) {
        return ;
    }

    out += comment;
    for (char c : str) {
        out += c;
        if (c == '\n') {
            out += comment;
        }
    }

    if (str.back() == '\n') {
        out.resize(out.size() - comment.size());
    }
    else {
        out += '\n';
    }
}

static std::string htmlize(std::string str)
{
    std::string html;
    html.reserve(str.capacity());

    // trim new lines
    while (!str.empty() && str.back() == '\n') {
        str.pop_back();
    }
    zstring_view zstr = str;
    while (zstr[0] == '\n') {
        zstr.pop_front();
    }

    // replace '&' and '<' with "&amp;" and "&lt;"
    for (char const& c : zstr) {
        if (c == '&') {
            html += "&amp;";
        }
        else if (c == '<') {
            html += "&lt;";
        }
        else {
            html += c;
        }
    }

    str.clear();
    html.swap(str);

    // replace "\n\n" with "<br/>\n"
    html.reserve(str.size());
    for (char const& c : str) {
        if (c == '\n' && (&c)[1] == '\n') {
            html += "<br/>";
        }
        else {
            html += c;
        }
    }

    str.clear();
    html.swap(str);

    // replace "  " at start line with "&nbsp; ;&nbsp; "
    if (str[0] == ' ' && str[1] == ' ') {
        html += "&nbsp; &nbsp; ";
    }
    for (char const& c : str) {
        if (c == '\n' && (&c)[1] == ' ' && (&c)[2] == ' ') {
            html += "\n&nbsp; &nbsp; ";
        }
        else {
            html += c;
        }
    }

    return html;
}

static void check_names(
    cfg_desc::names const& names,
    bool has_ini, bool has_acl, bool has_connpolicy)
{
    if (!has_ini && !names.ini.empty())
        throw std::runtime_error(str_concat("names.ini without ini for ", names.all));

    if (!has_acl && !names.acl.empty())
        throw std::runtime_error(str_concat("names.acl without acl for ", names.all));

    // if (has_acl && !names.connpolicy.empty())
    //     throw std::runtime_error(str_concat("names.connpolicy with acl for ", names.all));

    if (!has_connpolicy && !names.connpolicy.empty())
        throw std::runtime_error(str_concat("names.connpolicy without connpolicy for ", names.all));

    if (has_connpolicy && !names.acl.empty())
        throw std::runtime_error(str_concat("names.acl with connpolicy for ", names.all));

    if (!(has_ini || has_connpolicy) && !names.display.empty())
        throw std::runtime_error(str_concat("names.display without ini or connpolicy for ", names.all));
}

struct Appender
{
    std::string& str;

    template<class... T>
    void operator()(T const&... frags) const
    {
        (..., (str += frags));
    }

    void add_paragraph(std::string_view para)
    {
        if (!para.empty()) {
            str += para;
            str += '\n';
        }
    }
};

struct Marker
{
    std::string* str;
    std::size_t position;

    Marker(std::string& str)
    : str(&str)
    , position(str.size())
    {}

    std::string cut()
    {
        std::string ret(str->data() + position, str->size() - position);
        str->resize(position);
        return ret;
    }
};

struct GeneratorConfig
{
    struct Spec
    {
        std::string filename;
        std::ofstream out {filename};
    };

    struct SesmanMap
    {
        std::string filename;
        std::ofstream out {filename};
        std::vector<std::string> values_sent {};
        std::vector<std::string> values_reinit {};
    };

    struct SesmanDiag
    {
        std::string filename;
        std::ofstream out {filename};
        std::string buffer {};
    };

    struct Policy
    {
        using data_by_section_t = std::unordered_map<std::string_view, std::string>;
        struct SesmanInfo
        {
            data_by_section_t sections;
            std::string hidden_values;
        };

        std::string directory_spec;
        std::string remap_filename;
        std::unordered_map<DestSpecFile, data_by_section_t> spec_file_map {};
        std::unordered_map<DestSpecFile, SesmanInfo> acl_map {};
        std::vector<std::string> ordered_section {};
        std::unordered_set<std::string> section_names {};
        std::map<std::string, std::string> hidden_values {};
        std::unordered_set<std::string> acl_mems {};
    };

    Spec spec;
    Spec ini;
    SesmanMap acl_map;
    SesmanDiag acl_diag;
    Policy policy;
    cpp_config_writer::CppConfigWriterBase cpp;

    struct SectionData
    {
        Names names;
        std::string global_spec {};
        std::string ini {};
    };

    std::unordered_map<std::string_view, SectionData> sections;

    std::vector<std::string_view> ordered_section_names;


    GeneratorConfig(
        std::string ini_filename,
        std::string ini_spec_filename,
        std::string acl_map_filename,
        std::string acl_diag_filename,
        std::string directory_spec,
        std::string acl_remap_filename,
        cpp_config_writer::CppConfigWriterBase::Filenames filenames
    )
    : spec{std::move(ini_spec_filename)}
    , ini{std::move(ini_filename)}
    , acl_map{std::move(acl_map_filename)}
    , acl_diag{std::move(acl_diag_filename)}
    , policy{std::move(directory_spec), std::move(acl_remap_filename)}
    , cpp(std::move(filenames))
    {
        acl_map.out << python_comment(do_not_edit, 0) << "\n";

        acl_diag.buffer += do_not_edit;
        acl_diag.buffer += "\n       cpp name       |       acl / passthrough name\n\n"sv;
    }

    SectionData& get_section(Names const& section_names)
    {
        auto it = sections.find(section_names.all);
        if (it != sections.end()) {
            return it->second;
        }
        ordered_section_names.emplace_back(section_names.all);
        return sections.insert({section_names.all, SectionData{section_names}}).first->second;
    }

    int do_finish()
    {
        int err = 0;
        auto check_file = [&](std::string_view filename, std::ostream& f){
            f.flush();
            if (f) {
                return ;
            }

            int errnum = errno;
            std::cerr << filename << ": " << strerror(errnum) << "\n";
            ++err;
        };

        spec.out
            << "#include \"config_variant.hpp\"\n\n"
            << begin_raw_string
            << "## Python spec file for RDP proxy.\n\n\n"
        ;

        ini.out
            << "#include \"config_variant.hpp\"\n\n"
            << begin_raw_string
            << "## Config file for RDP proxy.\n\n\n"
        ;

        for (Spec* f : {&spec, &ini}) {
            for (std::string_view section_name : ordered_section_names) {
                auto& section = sections.find(section_name)->second;
                std::string& str = (f == &spec) ? section.global_spec : section.ini;
                if (!str.empty()) {
                    f->out << "[" << section.names.ini_name() << "]\n\n" << str;
                }
            }

            f->out << end_raw_string << "\n";
            check_file(f->filename, f->out);
        }


        std::sort(acl_map.values_sent.begin(), acl_map.values_sent.end());
        std::sort(acl_map.values_reinit.begin(), acl_map.values_reinit.end());
        auto write = [&](char const* name, std::vector<std::string> const& values){
            acl_map.out << name << " = {\n";
            for (auto&& x : values)
            {
                acl_map.out << x;
            }
            acl_map.out << "}\n";
        };
        write("back_to_selector_default_sent", acl_map.values_sent);
        write("back_to_selector_default_reinit", acl_map.values_reinit);
        check_file(acl_map.filename, acl_map.out);


        acl_diag.out << acl_diag.buffer;
        check_file(acl_diag.filename, acl_diag.out);


        std::ofstream out(policy.remap_filename);

        out << python_comment(do_not_edit, 0) << "\n"
            << "cp_spec = {\n"
        ;

        for (auto dest_file : conn_policies) {
            auto const& sections_by_file = *policy.acl_map.find(dest_file);
            auto const& sections = sections_by_file.second.sections;
            out << "'" << dest_file_to_filename(sections_by_file.first) << "': ({\n";
            for (auto const& section_name : policy.ordered_section) {
                auto section_it = sections.find(section_name);
                if (section_it != sections.end()) {
                    out
                        << "    '" << section_name << "': {\n"
                        << section_it->second << "    },\n"
                    ;
                }
            }
            out << "}, {\n" << sections_by_file.second.hidden_values << "}),\n";
        }

        out << "}\n";

        if (!out.flush()) {
            std::cerr << "ConnectionPolicyWriterBase: " << policy.remap_filename << ": " << strerror(errno) << "\n";
            return 1;
        }

        for (auto const& [cat, section_map] : policy.spec_file_map) {
            auto const spec_filename = str_concat(
                policy.directory_spec, '/', dest_file_to_filename(cat), ".spec");

            std::ofstream out_spec(spec_filename);

            out_spec << R"g([general]

# Secondary login Transformation rule
# ${LOGIN} will be replaced by login
# ${DOMAIN} (optional) will be replaced by domain if it exists.
# Empty value means no transformation rule.
transformation_rule = string(default='')

# Account Mapping password retriever
# Transformation to apply to find the correct account.
# ${USER} will be replaced by the user's login.
# ${DOMAIN} will be replaced by the user's domain (in case of LDAP mapping).
# ${USER_DOMAIN} will be replaced by the user's login + "@" + user's domain (or just user's login if there's no domain).
# ${GROUP} will be replaced by the authorization's user group.
# ${DEVICE} will be replaced by the device's name.
# A regular expression is allowed to transform a variable, with the syntax: ${USER:/regex/replacement}, groups can be captured with parentheses and used with \1, \2, ...
# For example to replace leading "A" by "B" in the username: ${USER:/^A/B}
# Empty value means no transformation rule.
vault_transformation_rule = string(default='')


)g";

            for (auto& section_name : policy.ordered_section) {
                auto section_it = section_map.find(section_name);
                if (section_it != section_map.end()) {
                    out_spec
                        << "[" << section_name << "]\n\n"
                        << section_it->second
                    ;
                }
            }

            if (!out_spec.flush()) {
                std::cerr << "ConnectionPolicyWriterBase: " << spec_filename << ": " << strerror(errno) << "\n";
                return 2;
            }
        }


        cpp.full_names.check();

        MultiFilenameWriter<cpp_config_writer::CppConfigWriterBase> sw(cpp);
        sw.then(cpp.filenames.authid_hpp, &cpp_config_writer::write_authid_hpp)
          .then(cpp.filenames.str_authid_hpp, &cpp_config_writer::write_str_authid_hpp)
          .then(cpp.filenames.variable_configuration_fwd, &cpp_config_writer::write_variables_configuration_fwd)
          .then(cpp.filenames.variable_configuration_hpp, &cpp_config_writer::write_variables_configuration)
          .then(cpp.filenames.config_set_value, &cpp_config_writer::write_config_set_value)
          .then(cpp.filenames.ini_values_hpp, &cpp_config_writer::write_ini_values)
          .then(cpp.filenames.acl_and_spec_type, &cpp_config_writer::write_acl_and_spec_type)
          .then(cpp.filenames.max_str_buffer_size_hpp, &cpp_config_writer::write_max_str_buffer_size_hpp)
        ;
        if (sw.err) {
            std::cerr << "CppConfigWriterBase: " << sw.filename << ": " << strerror(sw.errnum) << "\n";
            return sw.errnum;
        }

        return err;
    }

    void do_start_section(Names const& section_names)
    {
        get_section(section_names);

        if (!section_names.all.empty()) {
            ++cpp.depth;
        }
        cpp.start_section_index = cpp.authid_policies.size();
    }

    void do_stop_section(Names const& section_names)
    {
        acl_diag.buffer += '\n';

        if (!section_names.all.empty()) {
            --cpp.depth;
        }
        cpp.sections.emplace_back(cpp_config_writer::CppConfigWriterBase::Section{section_names.all, cpp.out_member_.str(), std::move(cpp.members)});
        cpp.out_member_.str("");
        std::string str = cpp.out_body_parser_.str();
        if (!str.empty()) {
            // all members are attr::external
            if (cpp.sections.back().members.empty()) {
                str.clear();
            }
            cpp.sections_parser.emplace_back(cpp_config_writer::CppConfigWriterBase::SectionString{section_names.all, str});
            cpp.out_body_parser_.str("");
        }
        cpp.start_indexes.emplace_back(cpp.authid_policies.size());
    }

    void evaluate_member(Names const& section_names, MemberInfo const& mem_info)
    {
        std::string spec_desc {};
        std::string ini_desc {};
        auto& section = get_section(section_names);

        Names const& names = mem_info.name;
        std::string_view desc = mem_info.desc;

        bool const has_spec = bool(mem_info.spec.dest);
        bool const has_ini = has_spec
                         && !bool(mem_info.spec.attributes & SpecAttributes::external);
        bool const has_global_spec = bool(mem_info.spec.dest & DestSpecFile::global_spec);
        bool const has_connpolicy = contains_conn_policy(mem_info.spec.dest);

        auto const acl_io = mem_info.spec.acl_io;
        bool const has_acl = bool(acl_io);

        check_names(names, has_ini, has_acl, has_connpolicy);

        std::string acl_network_name_tmp;
        std::string_view acl_network_name = has_connpolicy
            ? (acl_network_name_tmp = str_concat(section_names.all, ':', names.all))
            : names.acl_name();

        if (has_spec
         || mem_info.spec.reset_back_to_selector == ResetBackToSelector::Yes
         || !bool(mem_info.spec.attributes & SpecAttributes::external)
        ) {
            auto prefix_type = mem_info.prefix_type;
            auto* enumeration = mem_info.value.enumeration;
            if (enumeration) {
                if (desc.empty()) {
                    desc = enumeration->desc;
                }

                bool use_disable_prefix_for_enumeration = false;

                switch (prefix_type) {
                    case PrefixType::Unspecified:
                        use_disable_prefix_for_enumeration
                          = utils::starts_with(names.cpp_name(), "disable"_av);
                        break;
                    case PrefixType::ForceDisable:
                        use_disable_prefix_for_enumeration = true;
                        break;
                    case PrefixType::NoPrefix:
                        break;
                }

                if (mem_info.value.string_parser_for_enum) {
                    if (enum_values_has_desc(*enumeration)) {
                        spec_desc = enum_desc_with_values_in_desc(
                            *enumeration, use_disable_prefix_for_enumeration);
                        ini_desc = spec_desc;
                    }
                    else {
                        ini_desc = enum_as_string_ini_desc(*enumeration);
                    }
                }
                else {
                    spec_desc = numeric_enum_desc(
                        *enumeration, use_disable_prefix_for_enumeration);
                    ini_desc = spec_desc;
                }
            }
            else if (prefix_type == PrefixType::ForceDisable) {
                throw std::runtime_error("PrefixType::ForceDisable only with enums type");
            }
        }

        auto const acl_direction
                = (acl_io == SesmanIO::acl_to_proxy)
                ? " ⇐ "sv
                : (acl_io == SesmanIO::proxy_to_acl)
                ? " ⇒ "sv
                : (acl_io == SesmanIO::rw)
                ? " ⇔ "sv
                : ""sv;

        //
        // ini and spec
        //
        auto push_ini_or_spec_desc = [&](Appender appender, bool is_spec){
            auto marker = Marker{appender.str};

            if (is_spec) {
                appender(get_warning_attr(mem_info.spec.attributes));
            }

            if (bool(mem_info.tags & cfg_desc::TagList::Workaround)) {
                appender(workaround_message);
            }

            appender.add_paragraph(desc);

            appender(is_spec ? spec_desc : ini_desc);
            auto const& note = is_spec ? mem_info.value.spec_note : mem_info.value.ini_note;
            if (!note.empty()) {
                appender(is_spec ? "\n("sv : "("sv, note, ")\n"sv);
            }

            auto comment = is_spec ? htmlize(marker.cut()) : marker.cut();
            add_comment(appender.str, comment, "# "sv);

            auto spec_attr = mem_info.spec.attributes;
            if (is_spec) {
                spec_attr |= (mem_info.value.enumeration && type_enumeration::Category::flags == mem_info.value.enumeration->cat)
                    ? SpecAttributes::hex
                    : SpecAttributes();
            }
            else {
                spec_attr &= SpecAttributes::advanced;
            }

            add_attr_spec(appender.str, mem_info.spec.image_path, spec_attr);

            add_prefered_display_name(appender.str, names);
        };

        //
        // ini
        //
        if (has_ini) {
            auto appender = Appender{section.ini};
            push_ini_or_spec_desc(appender, false);
            if (has_acl) {
                appender("# (acl config: proxy"sv, acl_direction, acl_network_name, ")\n"sv);
            }
            appender('#', names.ini_name(), " = "sv,
                     mem_info.value.values.ini,  "\n\n"sv);
        }

        //
        // global spec
        //
        if (has_global_spec) {
            auto appender = Appender{section.global_spec};
            push_ini_or_spec_desc(appender, true);
            appender(names.ini_name(), " = "sv,
                     mem_info.value.prefix_spec_type,
                     "default="sv, mem_info.value.values.py, ")\n\n"sv);
        }

        //
        // acl default value
        //
        if (mem_info.spec.reset_back_to_selector == ResetBackToSelector::Yes)
        {
            auto& values = (SesmanIO::proxy_to_acl == acl_io)
                ? acl_map.values_reinit
                : acl_map.values_sent;

            values.emplace_back(str_concat(
                "    '"sv, names.acl_name(), "': "sv,
                mem_info.value.values.py, ",\n"sv
            ));
        }

        //
        // acl dialog
        //
        if (has_acl) {
            auto add_to_acl_diag = Appender{acl_diag.buffer};

            if (bool(mem_info.spec.attributes & SpecAttributes::external)) {
                add_to_acl_diag("<acl only>"sv);
            }
            else {
                add_to_acl_diag("cfg::"sv, section_names.acl_name(), "::"sv, names.all);
            }

            add_to_acl_diag(acl_direction, acl_network_name, "   ["sv);

            if (mem_info.value.string_parser_for_enum) {
                add_to_acl_diag(mem_info.value.cpp_type, acl_direction);
            }
            add_to_acl_diag(mem_info.value.acl_diag_type, "]\n"sv);

            auto marker = Marker(acl_diag.buffer);

            add_to_acl_diag.add_paragraph(desc);
            add_to_acl_diag(spec_desc);
            add_to_acl_diag(mem_info.value.acl_diag_note);
            add_comment(acl_diag.buffer, marker.cut(), "    "sv);
        }

        //
        // connection policy
        //
        if (has_connpolicy) {
            std::string_view member_name = names.connpolicy_name();

            std::string spec_str;
            spec_str.reserve(128);

            auto appender = Appender{spec_str};
            push_ini_or_spec_desc(appender, true);
            appender(member_name, " = "sv,
                     mem_info.value.prefix_spec_type,
                     "default="sv);

            std::string_view section_name = mem_info.connpolicy_section.empty()
                ? section_names.connpolicy_name()
                : mem_info.connpolicy_section;

            if (policy.section_names.emplace(section_name).second) {
                policy.ordered_section.emplace_back(section_name);
            }

            auto acl_name = acl_network_name;

            auto acl_mem_key = str_concat(section_name, '/', acl_name, '/', member_name);
            if (!policy.acl_mems.emplace(acl_mem_key).second) {
                throw std::runtime_error(str_concat("duplicate "sv, section_name, ' ', member_name));
            }

            for (auto dest_file : conn_policies) {
                if (!bool(mem_info.spec.dest & dest_file)) {
                    continue;
                }

                auto& content = policy.spec_file_map[dest_file][section_name];
                content += spec_str;

                std::string const* py_value = nullptr;
                std::string const* all_value = &mem_info.value.values.py;
                if (!mem_info.value.values.connection_policies.empty()) {
                    for (auto& connpolicy : mem_info.value.values.connection_policies) {
                        if (!bool(connpolicy.dest_file)) {
                            all_value = &connpolicy.py;
                        }
                        else if (!connpolicy.forced && connpolicy.dest_file == dest_file) {
                            py_value = &connpolicy.py;
                            break;
                        }
                    }
                }
                if (!py_value) {
                    py_value = all_value;
                }

                content += *py_value;
                content += ")\n\n"sv;

                // acl <-> proxy mapping
                if (!bool(mem_info.spec.attributes & SpecAttributes::external)) {
                    str_append(policy.acl_map[dest_file].sections[section_name],
                        "        ('", acl_name, "', '", member_name, "', ", *py_value, "),\n"
                    );

                    for (auto& connpolicy : mem_info.value.values.connection_policies) {
                        if (connpolicy.forced) {
                            str_append(policy.acl_map[connpolicy.dest_file].hidden_values,
                                "    '", acl_name, "': ", connpolicy.py, ",\n");
                        }
                    }
                }
            }
        }

        if (bool(mem_info.spec.attributes & SpecAttributes::external)) {
            if (!has_connpolicy) {
                std::string_view ini_name = names.ini_name();
                cpp.out_body_parser_ << "        else if (key == \"" << ini_name << "\"_zv) {\n"
                "            // ignored\n"
                "        }\n";
            }
            else {
                cpp.acl_and_connpolicy_policies.emplace_back(mem_info.spec.loggable);
                cpp.acl_and_connpolicy_strs.emplace_back(acl_network_name);
            }
        }
        else {
            std::string_view varname = names.all;

            cpp.members.push_back({varname, mem_info.value.align_of});

            std::string_view varname_with_section = varname;
            std::string tmp_string1;
            if (!section_names.all.empty()) {
                str_assign(tmp_string1, section_names.all, "::", varname);
                varname_with_section = tmp_string1;
            }

            if (has_acl) {
                cpp.variables_acl.emplace_back(varname_with_section);
            }

            if (!desc.empty()) {
                cpp.out_member_ << cpp_doxygen_comment(desc, 4);
            }
            if (!mem_info.value.cpp_note.empty()) {
                cpp.out_member_ << "    /// (" << mem_info.value.cpp_note << ") <br/>\n";
            }

            std::string acl_name;
            if (has_acl) {
                acl_name = acl_network_name;
                cpp.authstrs.emplace_back(acl_name);
                cpp.full_names.acl.emplace_back(acl_name);
            }
            cpp.out_member_ << "    /// type: " << cpp.html_entitifier([&](std::ostream& out){
                out << mem_info.value.cpp_type;
            }) << " <br/>\n";

            if (has_acl) {
                if (acl_io == SesmanIO::acl_to_proxy) {
                    if (!has_connpolicy) {
                        cpp.out_member_ << "    /// acl ⇒ proxy <br/>\n";
                    }
                }
                else if (acl_io == SesmanIO::proxy_to_acl) {
                    cpp.out_member_ << "    /// acl ⇐ proxy <br/>\n";
                }
                else if (acl_io == SesmanIO::rw) {
                    cpp.out_member_ << "    /// acl ⇔ proxy <br/>\n";
                }
            }

            if (!names.acl.empty()) {
                cpp.out_member_ << "    /// acl::name: " << acl_name << " <br/>\n";
            }
            else if (has_connpolicy) {
                cpp.out_member_ << "    /// connpolicy -> proxy";
                if (!names.connpolicy.empty() || !mem_info.connpolicy_section.empty()) {
                    cpp.out_member_ << "    [name: "
                        << (mem_info.connpolicy_section.empty()
                            ? section_names.all
                            : mem_info.connpolicy_section)
                        << "::" << names.connpolicy_name() << "]"
                    ;
                }
                cpp.out_member_ << " <br/>\n";

                cpp.out_member_ << "    /// aclName: " << acl_name << " <br/>\n";
            }

            if (!names.display.empty()) {
                cpp.out_member_ << "    /// displayName: " << names.display << " <br/>\n";
            }

            cpp.out_member_ << "    /// default: ";
            cpp.out_member_ << mem_info.value.values.cpp_comment;
            cpp.out_member_ << " <br/>\n";
            cpp.out_member_ << "    struct " << varname_with_section << " {\n";
            cpp.out_member_ << "        static constexpr unsigned acl_proxy_communication_flags = 0b";
            cpp.out_member_ << (bool(acl_io & SesmanIO::acl_to_proxy) ? "1" : "0");
            cpp.out_member_ << (bool(acl_io & SesmanIO::proxy_to_acl) ? "1" : "0");
            cpp.out_member_ << ";\n";

            if (has_acl) {
                cpp.out_member_ << "        // for old cppcheck\n";
                cpp.out_member_ << "        // cppcheck-suppress obsoleteFunctionsindex\n";
                cpp.out_member_ << "        static constexpr ::configs::authid_t index {"
                    " ::configs::cfg_indexes::section" << cpp.sections.size() << " + "
                    << (cpp.authid_policies.size() - cpp.start_section_index) << "};\n";
                cpp.authid_policies.emplace_back(mem_info.spec.loggable);
            }

            cpp.out_member_ << "        using type = ";
            cpp.out_member_ << mem_info.value.cpp_type;
            cpp.out_member_ << ";\n";


            // write type
            if (has_acl || has_spec) {
                cpp.out_member_ << "        using mapped_type = ";
                cpp.out_member_ << mem_info.value.spec_type;
                if (bool(/*PropertyFieldFlags::read & */acl_io)) {
                    cpp.max_str_buffer_size = std::max(cpp.max_str_buffer_size, mem_info.value.spec_str_buffer_size);
                }
                cpp.out_member_ << ";\n";

                cpp.out_acl_and_spec_types <<
                    "template<> struct acl_and_spec_type<cfg::" << varname_with_section << "> { using type = "
                ;
                cpp.out_acl_and_spec_types << mem_info.value.spec_type;
                cpp.out_acl_and_spec_types << "; };\n";
            }
            else {
                cpp.out_member_ << "        using mapped_type = type;\n";
            }


            // write value
            cpp.out_member_ << "        type value { ";
            cpp.out_member_ << mem_info.value.values.cpp;
            cpp.out_member_ << " };\n";

            cpp.out_member_ << "    };\n";

            if (has_spec) {
                std::string_view ini_name = names.ini_name();
                cpp.full_names.spec.emplace_back(str_concat(section_names.all, ':', ini_name));
                cpp.out_body_parser_ << "        else if (key == \"" << ini_name << "\"_zv) {\n"
                "            ::config_parse_and_log(\n"
                "                this->section_name, key.c_str(),\n"
                "                static_cast<cfg::" << varname_with_section << "&>(this->variables).value,\n"
                "                ::configs::spec_type<";
                cpp.out_body_parser_ << mem_info.value.spec_type;
                cpp.out_body_parser_ << ">{},\n"
                "                value\n"
                "            );\n"
                "        }\n";
                str_append(cpp.cfg_values, "cfg::"_av, varname_with_section, ",\n"_av);
                str_append(cpp.cfg_str_values,
                    "{\""_av, section_names.ini_name(), "\"_zv, \""_av,
                    names.ini_name(), "\"_zv},\n"_av);
            }
        }
    }
};


struct GeneratorConfigWrapper
{
    GeneratorConfig writer;

    void set_sections(std::initializer_list<std::string_view> l)
    {
        for (std::string_view section : l) {
            if (!section_names.emplace(section).second) {
                throw std::runtime_error(str_concat("set_sections(): duplicated section name: ", section));
            }
            section_makers.emplace_back().names.all = section;
        }
    }

    template<class Fn>
    void section(MemberNames names, Fn fn)
    {
        auto it = find_section(names);
        it->names = std::move(names);
        it->mk_section = std::unique_ptr<FuncBase>(new Func<decltype(fn)>{fn});
    }

    void build()
    {
        for (auto& makers : section_makers) {
            current_section_names = &makers.names;
            writer.do_start_section(makers.names);
            makers.mk_section->invoke();
            writer.do_stop_section(makers.names);
        }
    }

    void member(MemberInfo const& mem_info)
    {
        writer.evaluate_member(*current_section_names, mem_info);
    }

private:
    struct FuncBase
    {
        virtual void invoke() = 0;
        virtual ~FuncBase() = default;
    };

    template<class F>
    struct Func : FuncBase
    {
        F f;

        Func(F& f) : f(std::move(f)) {}

        void invoke() override
        {
            f();
        }
    };

    struct SectionInfo
    {
        Names names;
        std::unique_ptr<FuncBase> mk_section;
    };

    typename std::vector<SectionInfo>::iterator find_section(Names const& names)
    {
        assert(names.acl.empty());

        if (section_names.find(names.all) == section_names.end()) {
            throw std::runtime_error(str_concat(
                "Unknown section '", names.all, "'. Please add it in set_sections()"
            ));
        }

        auto it = std::find_if(section_makers.begin(), section_makers.end(), [&](auto const& info){
            return info.names.all == names.all;
        });

        if (it->mk_section) {
            throw std::runtime_error(str_concat("'", names.all, "' is already set"));
        }

        return it;
    }

public:
    Names const * current_section_names = nullptr;
    std::vector<SectionInfo> section_makers {};
    std::unordered_set<std::string_view> section_names {};
};

}
