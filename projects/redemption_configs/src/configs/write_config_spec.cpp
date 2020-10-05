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
#include "configs/generators/ini.hpp"
#include "configs/generators/python_spec.hpp"
#include "configs/generators/sesman_dialog.hpp"
#include "configs/generators/sesman_default_map.hpp"
#include "configs/specs/config_spec.hpp"
#include "configs/specs/config_type.hpp"

int main()
{
    auto evaluate = [](auto&&... writers){
        auto dispatch = [&](auto&& f){ return f(writers...); };
        cfg_generators::ConfigSpecWrapper<decltype(dispatch)> config{dispatch};

#ifndef IN_IDE_PARSER
        cfg_specs::config_type_definition(config.enums);
#endif

        (writers.do_init(), ...);

#ifndef IN_IDE_PARSER
        cfg_specs::config_spec_definition(config);
#endif

        int error_code = 0;
        int err;
        ((error_code = (err = writers.do_finish()) ? err : error_code), ...);
        return error_code;
    };

    using PythonSpec = cfg_generators::python_spec_writer::PythonSpecWriterBase;
    using Ini = cfg_generators::ini_writer::IniWriterBase;
    using Cpp = cfg_generators::cpp_config_writer::CppConfigWriterBase;
    using ConnPolicy = cfg_generators::connection_policy_writer::ConnectionPolicyWriterBase;
    using SesmanDialog = cfg_generators::sesman_dialog_writer::SesmanDialogWriterBase;
    using SesmanDefaultMap = cfg_generators::sesman_default_map::SesmanDefaultMapBase;

    return evaluate(
        PythonSpec("autogen/include/configs/autogen/str_python_spec.hpp"),
        Ini("autogen/include/configs/autogen/str_ini.hpp"),
        Cpp({
            "autogen/include/configs/autogen/authid.hpp",
            "autogen/include/configs/autogen/str_authid.hpp",
            "autogen/include/configs/autogen/variables_configuration_fwd.hpp",
            "autogen/include/configs/autogen/variables_configuration.hpp",
            "autogen/include/configs/autogen/set_value.tcc",
        }),
        ConnPolicy(
            "autogen/spec/", {"rdp", "vnc"},
            "../../tools/sesman/sesmanworker/sesmanconnpolicyspec.py"
        ),
        SesmanDefaultMap("../../tools/sesman/sesmanworker/sesmanbacktoselector.py"),
        SesmanDialog("autogen/doc/sesman_dialog.txt")
    );
}
