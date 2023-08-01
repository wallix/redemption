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

#include "configs/generators/config.hpp"
#include "configs/specs/config_spec.hpp"
#include "configs/specs/config_type.hpp"

int main()
{
    cfg_generators::GeneratorConfigWrapper generator{
        cfg_generators::GeneratorConfig(
            "autogen/include/configs/autogen/str_ini.hpp",

            "autogen/include/configs/autogen/str_python_spec.hpp",

            "../../tools/sesman/sesmanworker/sesmanbacktoselector.py",

            "autogen/doc/acl_dialog.txt",

            "autogen/spec/",
            "../../tools/sesman/sesmanworker/sesmanconnpolicyspec.py",

            {
                "autogen/include/configs/autogen/authid.hpp",
                "autogen/include/configs/autogen/str_authid.hpp",
                "autogen/include/configs/autogen/variables_configuration_fwd.hpp",
                "autogen/include/configs/autogen/variables_configuration.hpp",
                "autogen/include/configs/autogen/set_value.tcc",
                "autogen/include/configs/autogen/cfg_ini_pack.hpp",
                "autogen/include/configs/autogen/acl_and_spec_type.hpp",
                "autogen/include/configs/autogen/max_str_buffer_size.hpp",
            }
        )
    };

    type_enumerations enums;

    cfg_specs::config_type_definition(enums);
    cfg_specs::config_spec_definition(generator, enums);

    int err = generator.writer.do_finish();
    return err ? err : 0;
}
