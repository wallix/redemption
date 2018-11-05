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
#include "configs/enumeration.hpp"

#include "configs/generators/python_spec.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <locale>
#include <vector>
#include <unordered_map>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace connection_policy_writer {

using namespace cfg_attributes;

template<class Inherit>
struct ConnectionPolicyWriterBase : python_spec_writer::PythonSpecWriterBase<Inherit, connpolicy::name>
{
    using base_type = ConnectionPolicyWriterBase;
    using base_type_ = python_spec_writer::PythonSpecWriterBase<Inherit, connpolicy::name>;
    using base_type_::base_type_;

    std::unordered_map<std::string, std::string> rebind_sections;

    template<class Pack>
    void do_member(
        std::string const & section_name,
        std::string const & member_name,
        Pack const & infos
    ) {
        apply_if_contains<sesman::internal::io>(infos, [&, this](auto connpolicy, auto&& infos) {
            if (sesman::internal::io::connection_policy == connpolicy) {
                auto section = value_or<connpolicy::section>(infos,
                    connpolicy::section{section_name.c_str()});

                auto type = pack_get<spec::type_>(infos);

                this->write_description(pack_contains<desc>(infos), type, infos);
                this->inherit().write_type_info(type);
                this->write_enumeration_value_description(pack_contains<prefix_value>(infos), type, infos);

                auto spec_attr = value_or<spec::internal::attr>(infos, spec::internal::attr::no_ini_no_gui);

                if (bool(spec_attr & spec::internal::attr::advanced_in_gui)) this->out() << "\"#_advanced\\n\"\n";
                if (bool(spec_attr & spec::internal::attr::hex_in_gui))      this->out() << "\"#_hex\\n\"\n";

                this->out() << "\"" << member_name << " = ";
                this->inherit().write_type(type, get_default(type, infos));
                this->out() << "\\n\\n\"\n\n";

                if (section.name == section_name) {
                    this->sections[section.name] += this->out_member_.str();
                }
                else {
                    auto it = this->sections.find(section.name);
                    if (it == this->sections.end()) {
                        rebind_sections[section.name] += this->out_member_.str();
                    }
                    else {
                        it->second += this->out_member_.str();
                    }
                }

                this->out_member_.str("");
            }
        }, infos);
    }

    void do_init()
    {
    }

    void do_finish()
    {
        for (auto&& [section_name, contains] : sections) {
            this->out_file_ << "\"[" << section_name << "]\\n\\n\"\n\n" << contains;
        }

        for (auto&& [section_name, contains] : rebind_sections) {
            this->out_file_ << "\"[" << section_name << "]\\n\\n\"\n\n" << contains;
        }
    }

private:
    std::unordered_map<std::string, std::string> sections;
};

}


template<class SpecWriter>
int app_write_connection_policy(int ac, char const * const * av)
{
    if (ac < 2) {
        std::cerr << av[0] << " out-spec.h\n";
        return 1;
    }

    SpecWriter writer(av[1]);
    writer.evaluate();

    if (!writer.out_file_) {
        std::cerr << av[0] << ": " << av[1] << ": " << strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

}
