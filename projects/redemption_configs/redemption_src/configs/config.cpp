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


#include "configs/autogen/set_value.tcc"

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
