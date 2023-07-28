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
using python_spec_writer::get_desc;


template<class Pack>
auto const& get_default_value_for_connpolicy(Pack const& infos, std::string const& file)
{
    if constexpr (is_t_convertible_v<Pack, connpolicy::default_>) {
        auto& default_values = get_t_elem<connpolicy::default_>(infos);
        using pointer_type = decltype(&*default_values.values.begin());
        pointer_type all_value = nullptr;
        for (auto& value : default_values.values) {
            if (value.connpolicy_name.empty()) {
                all_value = &value;
            }
            else if (!value.always && value.connpolicy_name == file) {
                return value.value;
            }
        }

        if (all_value) {
            return all_value->value;
        }
    }

    auto type = get_type<spec::type_>(infos);
    return get_default(type, infos);
}


struct ConnectionPolicyWriterBase
{
    ConnectionPolicyWriterBase(std::string directory_spec, std::string sesman_map_filename)
    : directory_spec(std::move(directory_spec))
    , remap_filename{std::move(sesman_map_filename)}
    {}

    template<class Pack>
    void evaluate_member(Names const& section_names, Pack const& infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, connection_policy_t>) {
            Names const& names = infos;
            auto type = get_type<spec::type_>(infos);

            auto const& section = value_or<connpolicy::section>(
                infos, connpolicy::section{section_names.connpolicy_name().c_str()});

            if (this->section_names.emplace(section.name).second) {
                this->ordered_section.emplace_back(section.name);
            }

            std::string const& member_name = names.connpolicy_name();

            bool is_enum_parser = false;
            auto semantic_type = python_spec_writer::get_semantic_type(type, infos, &is_enum_parser);

            std::stringstream comments;

            python_spec_writer::write_description(comments, enums, semantic_type, get_desc(infos));
            python_spec_writer::write_type_info(comments, type);
            python_spec_writer::write_enumeration_value_description(comments, enums, semantic_type, infos, is_enum_parser);

            this->tmp_out << io_prefix_lines{python_spec_writer::htmlize(comments.str()).c_str(), "# ", "", 0};
            comments.str("");

            auto connpolicy = connection_policy_t(infos);

            using attr1_t = spec::internal::attr;
            using attr2_t = connpolicy::internal::attr;
            auto attr1 = value_or<spec_attr_t>(infos, spec_attr_t{attr1_t::no_ini_no_gui}).value;
            auto attr2 = connpolicy.spec;

            if (bool(attr1 & attr1_t::advanced_in_gui)
             || bool(attr2 & attr2_t::advanced_in_connpolicy)
            ) {
                comments << "_advanced\n";
            }
            if (bool(attr1 & attr1_t::hex_in_gui)
             || bool(attr2 & attr2_t::hex_in_connpolicy)
             || bool(python_spec_writer::attr_hex_if_enum_flag(semantic_type, enums))
            ) {
                comments << "_hex\n";
            }

            python_spec_writer::write_prefered_display_name(comments, names);

            this->tmp_out << io_prefix_lines{comments.str().c_str(), "#", "", 0};

            this->tmp_out << member_name << " = ";
            auto s = this->tmp_out.str();
            this->tmp_out.str("");

            auto sesman_name = sesman_network_name(infos, section_names);

            auto sesman_mem_key = str_concat(section.name, '/', sesman_name, '/', member_name);
            if (!this->sesman_mems.emplace(sesman_mem_key).second) {
                throw std::runtime_error(str_concat("duplicate ", section.name, ' ', member_name));
            }

            for (auto const& file : connpolicy.files) {
                auto const& value = get_default_value_for_connpolicy(infos, file);

                python_spec_writer::write_type2(tmp_out, enums, type, semantic_type, value);
                this->tmp_out << "\n\n";

                auto& content = this->spec_file_map[file][section.name];
                content += s;
                content += this->tmp_out.str();
                this->tmp_out.str("");

                if constexpr (is_convertible_v<Pack, not_external_attr_t>) {
                    sesman_default_map::python::write_type2(this->tmp_out, enums, type, semantic_type, value);
                    auto pyvalue = this->tmp_out.str();
                    str_append(this->sesman_map[file].sections[section.name],
                        "        ('", sesman_name, "', '", member_name, "', ", pyvalue, "),\n"
                    );

                    if constexpr (is_t_convertible_v<Pack, connpolicy::default_>) {
                        auto& default_values = get_t_elem<connpolicy::default_>(infos);
                        for (auto& value : default_values.values) {
                            if (value.always) {
                                this->tmp_out.str("");
                                sesman_default_map::python::write_type2(
                                    this->tmp_out, enums, type, semantic_type, value.value);
                                auto pyvalue = this->tmp_out.str();
                                str_append(this->sesman_map[value.connpolicy_name].hidden_values,
                                    "    '", sesman_name, "': ", pyvalue, ",\n");
                            }
                        }
                    }

                    this->tmp_out.str("");
                }
            }

            // check if connection_policy::set() refer an unknown name
            // or connection_policy::hidden_set() refer a known name
            if constexpr (is_t_convertible_v<Pack, connpolicy::default_>) {
                auto& default_values = get_t_elem<connpolicy::default_>(infos);
                for (auto& value : default_values.values) {
                    if (!value.connpolicy_name.empty()) {
                        auto const& names = connpolicy.files;
                        auto pos = std::find(names.begin(), names.end(), value.connpolicy_name);
                        if ((pos != names.end()) == value.always) {
                            throw std::runtime_error(str_concat(
                                "set an unknown connpolicy: ", value.connpolicy_name
                            ));
                        }
                    }
                }
            }
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

        out << python_comment(do_not_edit, 0) << "\n"
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
            std::cerr << "ConnectionPolicyWriterBase: " << this->remap_filename << ": " << strerror(errno) << "\n";
            return 1;
        }

        for (auto const& [cat, section_map] : this->spec_file_map) {
            auto const spec_filename = str_concat(this->directory_spec, '/', cat, ".spec");

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
                std::cerr << "ConnectionPolicyWriterBase: " << spec_filename << ": " << strerror(errno) << "\n";
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
    std::string directory_spec;
    std::string remap_filename;
};

}

}
