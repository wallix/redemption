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
#include "configs/io.hpp"
#include "utils/translation.hpp"
#include "utils/log.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "configs/autogen/str_authid.hpp"

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wunused-function")
#include "configs/autogen/enums_func_ini.tcc"
REDEMPTION_DIAGNOSTIC_POP

namespace
{
    parse_error log_ini_parse_err(
        parse_error err, const char * context, char const* key, bytes_view av)
    {
        LOG_IF(err, LOG_WARNING,
            "parsing error with parameter '%s' in section [%s] for \"%.*s\": %s",
            key, context, int(av.size()), av.as_chars().data(), err.c_str());
        return err;
    }

    template<class T, class U>
    parse_error config_parse_and_log(
        const char * context, char const* key, T & x, U u, bytes_view av)
    {
        return log_ini_parse_err(parse_from_cfg(x, u, av), context, key, av);
    }

    parse_error log_acl_parse_err(
        parse_error err, zstring_view key, bytes_view value)
    {
        LOG_IF(err, LOG_WARNING,
            "parsing error with acl parameter '%s' for \"%.*s\": %s",
            key, int(value.size()), value.as_chars().data(), err.c_str());

        return err;
    }

    template<class T>
    struct ConfigFieldVTable
    {
        static bool parse(configs::VariablesConfiguration & variables, bytes_view value)
        {
            return !log_acl_parse_err(
                parse_from_cfg(
                    static_cast<T&>(variables).value,
                    configs::spec_type<typename T::sesman_and_spec_type>{},
                    value),
                configs::authstr[unsigned(T::index)],
                value);
        }

        static zstring_view to_zstring_view(
            configs::VariablesConfiguration const& variables,
            writable_chars_view buffer)
        {
            return assign_zbuf_from_cfg(
                buffer,
                cfg_s_type<typename T::sesman_and_spec_type>{},
                static_cast<T const&>(variables).value
            );
        }
    };

    template<class>
    struct ConfigFieldVTableMaker;

    template<class T, class... Ts>
    struct ConfigFieldVTableMaker<configs::Pack<T, Ts...>>
    {
        static constexpr auto make_parsers()
        {
            return std::array<decltype(&ConfigFieldVTable<T>::parse), sizeof...(Ts)+1>{
                &ConfigFieldVTable<T>::parse,
                &ConfigFieldVTable<Ts>::parse...
            };
        }

        static constexpr auto make_to_zstring_view()
        {
            return std::array<decltype(&ConfigFieldVTable<T>::to_zstring_view), sizeof...(Ts)+1>{
                &ConfigFieldVTable<T>::to_zstring_view,
                &ConfigFieldVTable<Ts>::to_zstring_view...
            };
        }
    };

    inline constexpr auto config_parse_value_fns
        = ConfigFieldVTableMaker<configs::VariablesAclPack>::make_parsers();

    inline constexpr auto config_to_zstring_view_fns
        = ConfigFieldVTableMaker<configs::VariablesAclPack>::make_to_zstring_view();
} // anonymous namespace

#include "configs/autogen/set_value.tcc"

zstring_view Inifile::FieldConstReference::to_zstring_view(
    Inifile::ZStringBuffer& buffer) const
{
    return config_to_zstring_view_fns[unsigned(this->id)](
        this->ini->variables, writable_chars_view(buffer));
}

zstring_view Inifile::FieldConstReference::get_acl_name() const
{
    return configs::authstr[unsigned(this->id)];
}

bool Inifile::FieldReference::set(bytes_view value)
{
    bool const ok = config_parse_value_fns[unsigned(this->id)](this->ini->variables, value);
    if (ok) {
        this->ini->asked_table.clear(this->id);
        this->ini->new_from_acl = true;
    }
    return ok;
}

Inifile::FieldReference Inifile::get_acl_field_by_name(chars_view name)
{
    using int_type = std::underlying_type_t<configs::authid_t>;
    for (int_type i = 0; i < int_type(configs::max_authid); ++i) {
        if (configs::authstr[i].size() == name.size()
         && 0 == memcmp(configs::authstr[i].data(), name.data(), name.size())
        ) {
            return {*this, authid_t(i)};
        }
    }
    return {};
}

::Language language(Inifile const & ini)
{
    return ini.get<cfg::translation::language>();
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

    this->asked_table.set(cfg::context::target_port::index);
}
