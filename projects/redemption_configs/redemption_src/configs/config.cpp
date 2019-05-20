/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Jonathan Poelen, Raphael Zhou, Meng Tan

    Configuration file,
    parsing config file rdpproxy.ini
*/

#include "configs/config.hpp"
#include "utils/translation.hpp"

namespace configs
{
    template<class T, class U>
    parse_error parse_and_log(const char * context, const char * key, T & x, U u, array_view_const_char av)
    {
        auto const err = ::configs::parse(x, u, av);
        if (err) {
            LOG(
                LOG_WARNING,
                "parsing error with parameter '%s' in section [%s] for \"%.*s\": %s",
                key, context, int(av.size()), av.data(), err.c_str()
            );
        }
        return err;
    }
} // namespace configs

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wweak-template-vtables")
#include "configs/autogen/extern_template_field.tcc"
REDEMPTION_DIAGNOSTIC_POP
#include "configs/autogen/enums_func_ini.tcc"
#include "configs/autogen/set_value.tcc"


template<class T>
bool Inifile::Field<T>::parse(configs::VariablesConfiguration & variables, array_view_const_char value)
{
    return ! ::configs::parse_and_log(
        T::section, T::name,
        static_cast<T&>(variables).value,
        configs::spec_type<typename T::sesman_and_spec_type>{},
        value
    );
}

/// \return array_view_const_char::data() guarantee with null terminal
template<class T>
array_view_const_char Inifile::Field<T>::to_string_view(configs::VariablesConfiguration const & variables, Buffers & buffers) const
{
    return ::configs::assign_zbuf_from_cfg(
        static_cast<configs::zstr_buffer_from<typename T::type>&>(
            static_cast<configs::CBuf<T>&>(buffers)
        ),
        configs::cfg_s_type<typename T::sesman_and_spec_type>{},
        static_cast<T const &>(variables).value
    );
}


Translation::language_t language(Inifile const & ini)
{
    return static_cast<Translation::language_t>(
        ini.template get<cfg::translation::language>());
}

const char * Translation::translate(trkeys::TrKey_password k) const
{
    if (this->ini) {
        switch (this->lang) {
            case Translation::EN: {
                auto & s = this->ini->template get<cfg::translation::password_en>();
                if (!s.empty()) {
                    return s.c_str();
                }
            }
            break;
            case Translation::FR: {
                auto & s = this->ini->template get<cfg::translation::password_fr>();
                if (!s.empty()) {
                    return s.c_str();
                }
            }
            break;
            case Translation::MAX_LANG:
                assert(false);
                break;
        }
    }

    return k.translations[this->lang];
}

void Inifile::initialize()
{
    this->push_to_send_index<cfg::context::opt_bpp>();
    this->push_to_send_index<cfg::context::opt_width>();
    this->push_to_send_index<cfg::context::opt_height>();
    this->push_to_send_index<cfg::context::selector_current_page>();
    this->push_to_send_index<cfg::context::selector_device_filter>();
    this->push_to_send_index<cfg::context::selector_group_filter>();
    this->push_to_send_index<cfg::context::selector_proto_filter>();
    this->push_to_send_index<cfg::context::selector_lines_per_page>();
    this->ask<cfg::context::target_password>();
    this->ask<cfg::context::target_host>();
    this->ask<cfg::context::target_protocol>();
    this->ask<cfg::context::password>();
    this->push_to_send_index<cfg::context::reporting>();
    this->push_to_send_index<cfg::context::auth_channel_target>();
    this->push_to_send_index<cfg::context::accept_message>();
    this->push_to_send_index<cfg::context::display_message>();
    this->push_to_send_index<cfg::context::real_target_device>();
    this->ask<cfg::globals::auth_user>();
    this->push_to_send_index<cfg::globals::host>();
    this->push_to_send_index<cfg::globals::target>();
    this->ask<cfg::globals::target_device>();
    this->ask<cfg::globals::target_user>();

    static_cast<Field<cfg::context::target_port>&>(this->fields).asked_ = true;
}
