/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#pragma once

#include <string_view>

inline ModuleName get_internal_module_id_from_target(std::string_view target_name)
{
    if (target_name == "bouncer2") return ModuleName::bouncer2;
    if (target_name == "autotest") return ModuleName::autotest;
    if (target_name == "widget_message") return ModuleName::selector;
    if (target_name == "widgettest") return ModuleName::widgettest;
    if (target_name == "card") return ModuleName::card;
    return ModuleName::exit;
}

inline const char * get_module_name(ModuleName module_id) noexcept
{
    switch (module_id) {
        case ModuleName::INTERNAL_TRANSITION:   return "MODULE_INTERNAL_TRANSITION";
        case ModuleName::exit:                  return "MODULE_EXIT";
        case ModuleName::VNC:                   return "MODULE_VNC";
        case ModuleName::RDP:                   return "MODULE_RDP";
        case ModuleName::INTERNAL:              return "MODULE_INTERNAL";
        case ModuleName::close:                 return "MODULE_INTERNAL_CLOSE";
        case ModuleName::close_back:            return "MODULE_INTERNAL_CLOSE_BACK";
        case ModuleName::login:                 return "MODULE_INTERNAL_LOGIN";
        case ModuleName::card:                  return "MODULE_INTERNAL_CARD";
        case ModuleName::confirm:               return "MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE";
        case ModuleName::valid:                 return "MODULE_INTERNAL_DIALOG_VALID_MESSAGE";
        case ModuleName::challenge:             return "MODULE_INTERNAL_DIALOG_CHALLENGE";
        case ModuleName::interactive_target:    return "MODULE_INTERNAL_TARGET";
        case ModuleName::bouncer2:              return "MODULE_INTERNAL_BOUNCER2";
        case ModuleName::autotest:              return "MODULE_INTERNAL_TEST";
        case ModuleName::selector:              return "MODULE_INTERNAL_SELECTOR";
        case ModuleName::widgettest:            return "MODULE_INTERNAL_WIDGETTEST";
        case ModuleName::waitinfo:              return "MODULE_INTERNAL_WAIT_INFO";
        case ModuleName::transitory:            return "MODULE_TRANSITORY";
        case ModuleName::UNKNOWN: break;
    }

    return "<unknown>";
}
