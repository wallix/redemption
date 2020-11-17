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
#include "utils/sugar/algostring.hpp"

#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace connection_policy_writer {

using namespace cfg_attributes;


struct ConnectionPolicyWriterBase
{
    using categories_t = std::array<std::string, 2>;

    struct PythonSpec
    {
        std::string filename;
        std::ostringstream out;

        PythonSpec(std::string filename)
        : filename(std::move(filename))
        {}
    };
    PythonSpec python_spec;

    ConnectionPolicyWriterBase(std::string directory_spec, categories_t categories, std::string sesman_map_filename)
    : python_spec{std::move(sesman_map_filename)}
    , categories(std::move(categories))
    , directory_spec(std::move(directory_spec))
    {}

    template<class Pack>
    void evaluate_member(Names const& section_names, Pack const& infos, type_enumerations& enums)
    {
        if constexpr (is_convertible_v<Pack, connection_policy_t>) {
            Names const& names = infos;
            auto type = get_type<spec::type_>(infos);
            auto& default_value = get_default<connpolicy::default_>(type, infos);

            std::string const& member_name = names.connpolicy_name();

            bool is_enum_parser = false;
            auto semantic_type = python_spec_writer::get_semantic_type(type, infos, &is_enum_parser);

            std::stringstream comments;

            python_spec_writer::write_description(comments, enums, semantic_type, get_desc(infos));
            python_spec_writer::write_type_info(comments, type);
            python_spec_writer::write_enumeration_value_description(comments, enums, semantic_type, infos, is_enum_parser);

            this->python_spec.out << io_prefix_lines{comments.str().c_str(), "# ", "", 0};
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

            this->python_spec.out << io_prefix_lines{comments.str().c_str(), "#", "", 0};

            this->python_spec.out << member_name << " = ";
            python_spec_writer::write_type2(python_spec.out, enums, type, semantic_type, default_value);
            this->python_spec.out << "\n\n";

            auto&& sections = this->file_map[connpolicy.file];
            auto const& section = value_or<connpolicy::section>(
                infos, connpolicy::section{section_names.connpolicy_name().c_str()});

            if (this->section_names.emplace(section.name).second) {
                this->ordered_section.emplace_back(section.name);
            }

            Section& sec = sections[section.name];

            auto sesman_name = sesman_network_name(infos, section_names);

            sec.python_contains += this->python_spec.out.str();

            this->python_spec.out.str("");

            auto& buf = this->python_spec.out;
            sesman_default_map::python::write_type2(buf, enums, type, semantic_type, default_value);
            update_sesman_contains(sec.sesman_contains, sesman_name, member_name, buf.str());

            buf.str("");
        }
    }

    static void update_sesman_contains(
        std::string& s,
        std::string const sesman_name,
        std::string const connpolicy_name,
        std::string const value,
        char const* extra = "")
    {
        str_append(s,
                   "        u'", sesman_name, "': (\n"
                   "            '", connpolicy_name, "', ", value, "\n"
                   "        ),", extra, '\n');
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
        std::ofstream out_sesman(this->python_spec.filename);

        out_sesman <<
          "#!/usr/bin/python -O\n"
          "# -*- coding: utf-8 -*-\n\n"
          << python_comment(do_not_edit, 0) << "\n"
          "cp_spec = {\n"
        ;

        for (auto const& cat : this->categories) {
            auto file_it = file_map.find(cat);
            if (file_it == file_map.end()) {
                std::cerr << "ConnectionPolicyWriterBase: unknown " << cat << " for connpolicy\n";
                return 1;
            }

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

            auto& section_map = file_it->second;
            for (auto& section_name : this->ordered_section) {
                auto section_it = section_map.find(section_name);
                if (section_it != section_map.end()) {
                    out_spec
                        << "[" << section_name << "]\n\n"
                        << section_it->second.python_contains
                    ;
                    out_sesman
                        << "    '" << section_name << "': {\n"
                        << section_it->second.sesman_contains << "    },\n"
                    ;
                }
            }

            if (!out_spec.flush()) {
                std::cerr << "ConnectionPolicyWriterBase: " << spec_filename << ": " << strerror(errno) << "\n";
                return 1;
            }
        }

        out_sesman << "}\n";

        if (!out_sesman) {
            std::cerr << "ConnectionPolicyWriterBase: " << this->python_spec.filename << ": " << strerror(errno) << "\n";
            return 1;
        }
        return 0;
    }

private:
    struct Section
    {
        std::string python_contains;
        std::string sesman_contains;
    };
    using data_by_section_t = std::unordered_map<std::string, Section>;
    std::unordered_map<std::string, data_by_section_t> file_map;
    std::vector<std::string> ordered_section;
    std::unordered_set<std::string> section_names;
    categories_t categories;
    std::string directory_spec;
};

}

}
