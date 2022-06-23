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
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/generators/utils/names.hpp"
#include "configs/generators/sesman_default_map.hpp"
#include "configs/enumeration.hpp"

#include "configs/generators/python_spec.hpp"

#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace connection_policy_writer {

using namespace cfg_attributes;


struct IgnoreFirstComma
{
    mutable char const* comma = "";

    friend std::ostream& operator<<(std::ostream& out, IgnoreFirstComma const& ignore_first_comma)
    {
        out << ignore_first_comma.comma;
        ignore_first_comma.comma = ",";
        return out;
    }
};

inline char const* json_bool(bool b)
{
    return b ? "true" : "false";
}

namespace impl
{
    template<class T> struct is_empty_type : std::false_type {};

    template<> struct is_empty_type<types::dirpath> : std::true_type {};
    template<class T> struct is_empty_type<types::list<T>> : std::true_type {};
    template<unsigned n> struct is_empty_type<types::fixed_string<n>> : std::true_type {};

    template<class T>
    struct unknown_type
    {};
}

inline void write_value(std::ostream& out, bool x)
{
    out << json_bool(x);
}

template<class T>
void write_value(std::ostream& out, T const& x)
{
    if constexpr (std::is_integral_v<T>) {
        out << x;
    }
    else if constexpr (std::is_enum_v<T>) {
        // TODO use type_enumerations
        out << std::underlying_type_t<T>(x);
    }
    else if constexpr (impl::is_empty_type<T>::value) {
        out << "\"\"";
    }
    else if constexpr (std::is_convertible_v<T, std::string_view>) {
        out << std::string_view(x);
    }
    else if constexpr (std::is_same_v<T, cfg_attributes::cpp::expr>) {
        out << x.value;
    }
    else {
        impl::unknown_type<T>::uncallable();
    }
}


inline void write_type(std::ostream& out, type_<types::rgb>) { out << "rgb"; }
inline void write_type(std::ostream& out, type_<types::ip_string>) { out << "IpString"; }
inline void write_type(std::ostream& out, type_<types::dirpath>) { out << "DirPath"; }
inline void write_type(std::ostream& out, type_<FilePermissions>) { out << "FilePermissions"; }

template<unsigned N>
void write_type(std::ostream& out, type_<types::fixed_binary<N>>) { out << "FixedBinary<" << N << ">"; }

template<unsigned N>
void write_type(std::ostream& out, type_<types::fixed_string<N>>) { out << "FizedString<" << N << ">"; }

template<class T, long min, long max>
void write_type(std::ostream& out, type_<types::range<T, min, max>>) { out << type_name<T>(); }

template<class T>
void write_type(std::ostream& out, type_<T>) { out << type_name<T>(); }

template<class T>
void write_type(std::ostream& out, type_<types::list<T>>)
{
    out << "List<";
    write_type(out, type_<T>());
    out << "List>";
}


struct Json
{
    Json(std::string json_filename)
    : json_filename(std::move(json_filename))
    {}

    /*
    ? = optional

    enums: {
        [name]: {
            flag_type: bool,
            min: int,
            max: int,
            description: string,
            info: string,
            values: [{
                value: int,
                name: string,
                label: string,
                description: string,
                internal: bool,
            }],
        },
    }

    options: [{
        desc: string,
        loggable: int,
        target_option: bool,
        sesman_to_proxy: bool,
        proxy_to_sesman: bool,

        cpp?: {
            section: string,
            name: string,
            type: string,
            default: string,
        },

        ini?: {
            section: string,
            name: string,
            type: string,
            default: string,
        },

        gui?: {
            section: string,
            name: string,
            type: string,
            default: string,

            input: 'text' | 'password' | 'image',
            advanced: bool,
            prefer_hex: bool,
            iptables: bool,
            external: bool,
        },

        rdp_connpolicy?: as gui,
        vnc_connpolicy?: as gui,
        jh_connpolicy?: as gui,
    }]
    */

    bool enum_is_processed = false;
    std::ostringstream output_file;

    void evaluate_enums(type_enumerations const& enums)
    {
        if (enum_is_processed) {
            return;
        }
        enum_is_processed = true;

        IgnoreFirstComma ignore_first_comma1;

        output_file << "{\"enums\":{";

        for (type_enumeration const& e : enums.enumerations_) {
            output_file << ignore_first_comma1
                << "\"" << e.name
                << "\":{\"flag_type\":" << json_bool(e.cat == type_enumeration::Category::flags)
                << ",\"min\": " << e.min()
                << ",\"max\": " << e.max()
                << ",\"description\": " << io_quoted2{e.get_desc()}
                << ",\"info\": " << io_quoted2{e.get_info()}
                << ",\"values\":["
                ;

            IgnoreFirstComma ignore_first_comma2;
            for (auto&& item : e.values) {
                output_file << ignore_first_comma2
                    << "{\"value\":" << item.val
                    << ",\"name\":" << item.name
                    << ",\"label\":" << item.get_name()
                    << ",\"description\":" << io_quoted2{item.get_desc()}
                    << ",\"internal\":" << item.exclude
                    << "}"
                    ;
            }

            output_file << "]}";
        }

        output_file << "},\"options\":[";
    }

    IgnoreFirstComma ignore_first_comma;

    template<class Pack>
    void evaluate_member(Names const& section_names, Pack const& infos, type_enumerations const& enums)
    {
        this->evaluate_enums(enums);

        auto sesman_io = value_or<sesman_io_t>(infos, sesman_io_t{sesman::internal::io::none}).value;

        output_file << ignore_first_comma
            << "{\"desc\":\"" << io_quoted2{get_desc(infos)}
            << "\",\"loggable\":" << unsigned(value_or<log_policy_t>(infos, log_policy_t{}).value)
            << ",\"target_option\":" << json_bool(is_convertible_v<Pack, decltype(sesman::constants::reset_back_to_selector)>)
            << ",\"sesman_to_proxy\":" << json_bool(sesman_io & sesman::internal::io::sesman_to_proxy)
            << ",\"proxy_to_sesman\":" << json_bool(sesman_io & sesman::internal::io::proxy_to_sesman)
            ;

        Names const& names = infos;

        auto type = get_t_elem<cfg_attributes::type_>(infos);

        // cpp option
        if constexpr (!std::is_convertible_v<Pack, is_external_attr_t>) {
            output_file << ",\"cpp\":"
                "{\"section\":\"" << section_names.cpp_name() << "\""
                ",\"name\":\"" << names.cpp_name() << "\""
                ",\"type\":\"" << write_type(type) << "\""
                ",\"default\":" << write_value(get_default(type, infos)) <<
                "}"
                ;
        }

        // ini option
        if constexpr (python_spec_writer::is_candidate_for_spec<Pack>) {
            auto spec_type = get_type<spec::type_>(infos);
            output_file << ",\"ini\":"
                "{\"section\":\"" << section_names.ini_name() << "\""
                ",\"name\":\"" << names.ini_name() << "\""
                ",\"type\":\"" << write_type(spec_type) << "\""
                ",\"default\":" << write_value(get_default(spec_type, infos)) <<
                "}"
                ;
        }

        // gui option
        if constexpr (!std::is_convertible_v<Pack, is_external_attr_t>) {
            auto type = get_type<spec::type_>(infos);
            output_file << ",\"ini\":"
                "{\"section\":\"" << section_names.ini_name() << "\""
                ",\"name\":\"" << names.ini_name() << "\""
                ",\"type\":\"" << write_type(type) << "\""
                ",\"default\":" << write_value(get_default(type, infos)) <<
                "}"
                ;
        }
    }

    void do_start_section(Names const& /*names*/)
    {
    }

    void do_stop_section(Names const& /*names*/)
    {
    }

    void do_init()
    {}

    int do_finish()
    {
        std::ofstream out(this->remap_filename);

        out <<
            "# -*- coding: utf-8 -*-\n#\n"
            << python_comment(do_not_edit, 0) << "\n"
            << "cp_spec = {\n"
        ;

        using StringPtr = std::string const*;
        std::unique_ptr<StringPtr[]> sorted_connpolicy_names(
            new StringPtr[this->sesman_map.size() + 1]
        );
        {
            auto it = sorted_connpolicy_names.get();
            for (auto const& sections_by_file : this->sesman_map) {
                *it++ = &sections_by_file.first;
            }
            *it = nullptr;
            std::sort(
                sorted_connpolicy_names.get(), it,
                [](auto* a, auto* b) { return *a < *b; }
            );
        }

        for (auto it = sorted_connpolicy_names.get(); *it; ++it) {
            auto const& sections_by_file = *this->sesman_map.find(**it);
            auto const& sections = sections_by_file.second.sections;
            out << "'" << sections_by_file.first << "': ({\n";
            for (auto const& section_name : this->ordered_section) {
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
            std::cerr << "Json: " << this->remap_filename << ": " << strerror(errno) << "\n";
            return 1;
        }

        for (auto const& [cat, section_map] : this->spec_file_map) {
            auto const spec_filename = str_concat(this->json_filename, '/', cat, ".spec");

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

            for (auto& section_name : this->ordered_section) {
                auto section_it = section_map.find(section_name);
                if (section_it != section_map.end()) {
                    out_spec
                        << "[" << section_name << "]\n\n"
                        << section_it->second
                    ;
                }
            }

            if (!out_spec.flush()) {
                std::cerr << "Json: " << spec_filename << ": " << strerror(errno) << "\n";
                return 2;
            }
        }

        return 0;
    }

private:
    std::ostringstream tmp_out;

    using data_by_section_t = std::unordered_map<std::string, std::string>;
    std::unordered_map<std::string, data_by_section_t> spec_file_map;
    struct SesmanInfo
    {
        data_by_section_t sections;
        std::string hidden_values;
    };
    std::unordered_map<std::string, SesmanInfo> sesman_map;
    std::vector<std::string> ordered_section;
    std::unordered_set<std::string> section_names;
    std::map<std::string, std::string> hidden_values;
    std::unordered_set<std::string> sesman_mems;
    std::string json_filename;
    std::string remap_filename;
};

}

}
