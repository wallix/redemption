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

#include "configs/generators/connection_policy.hpp"
#include "configs/generators/cpp_config.hpp"
#include "configs/generators/cpp_enumeration.hpp"
#include "configs/generators/ini.hpp"
#include "configs/generators/python_spec.hpp"
#include "configs/specs/config_spec.hpp"
#include "configs/specs/config_type.hpp"

template<class... Fs>
int thens(Fs... fs)
{
    int r = 0;
    (void)(((void)(r = fs()), r) || ...);
    return r;
}

int main()
{
    return thens([]{
        struct Writer : cfg_generators::connection_policy_writer::ConnectionPolicyWriterBase<Writer>
        {
            using base_type::base_type;

            void do_init()
            {
                base_type::do_init();
                cfg_specs::config_type_definition(this->enums);
                cfg_specs::config_spec_definition(*this);
            }
        };
        char const* av[]{
            "conn_policy_python_writer",
            "../../tools/sesman/sesmanworker/sesmanconnpolicyspec.py",
            "autogen/spec/rdp.spec",
            "autogen/spec/vnc.spec",
            nullptr,
        };
        return cfg_generators::app_write_connection_policy<Writer>(std::size(av)-1, av);
    },[]{
        struct Writer : cfg_generators::connection_policy_writer::ConnectionPolicyWriterBaseJson<Writer>
        {
            using base_type::base_type;

            void do_init()
            {
                base_type::do_init();
                cfg_specs::config_type_definition(this->enums);
                cfg_specs::config_spec_definition(*this);
            }
        };
        char const* av[]{
            "conn_policy_json_writer",
            "autogen/spec/rdp.json",
            "autogen/spec/vnc.json",
            nullptr,
        };
        return cfg_generators::app_write_connection_policy_json<Writer>(std::size(av)-1, av);
    },
    []{
        struct Writer : cfg_generators::cpp_config_writer::CppConfigWriterBase<Writer>
        {
            void do_init()
            {
                cfg_specs::config_spec_definition(*this);
            }
        };
        char const* av[]{
            "cpp_writer",
            "autogen/include/configs/autogen/authid.hpp",
            "autogen/include/configs/autogen/variables_configuration_fwd.hpp",
            "autogen/include/configs/autogen/variables_configuration.hpp",
            "autogen/include/configs/autogen/set_value.tcc",
            nullptr,
        };
        return cfg_generators::app_write_cpp_config<Writer>(std::size(av)-1, av);
    },
    []{
        struct Writer : cfg_generators::ini_writer::IniWriterBase<Writer>
        {
            using base_type::base_type;

            void do_init()
            {
                base_type::do_init();
                cfg_specs::config_type_definition(this->enums);
                cfg_specs::config_spec_definition(*this);
            }
        };
        char const* av[]{
            "ini_writer",
            "autogen/include/configs/autogen/str_ini.hpp",
            nullptr,
        };
        return cfg_generators::app_write_ini<Writer>(std::size(av)-1, av);
    },
    []{
        struct Writer : cfg_generators::python_spec_writer::PythonSpecWriterBase<Writer>
        {
            using base_type::base_type;

            void do_init()
            {
                base_type::do_init();
                cfg_specs::config_type_definition(this->enums);
                cfg_specs::config_spec_definition(*this);
            }
        };
        char const* av[]{
            "python_spec_writer",
            "autogen/include/configs/autogen/str_python_spec.hpp",
            nullptr,
        };
        return cfg_generators::app_write_python_spec<Writer>(std::size(av)-1, av);
    });
}
