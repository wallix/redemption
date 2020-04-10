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

#include "utils/log.hpp"
#include <string_view>

enum ModuleIndex : int
{
    MODULE_EXIT,
    MODULE_VNC,
    MODULE_RDP,
    MODULE_XUP,
    MODULE_INTERNAL,
    MODULE_INTERNAL_CLOSE,
    MODULE_INTERNAL_CLOSE_BACK,
    MODULE_INTERNAL_WIDGET_LOGIN,
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
    MODULE_INTERNAL_DIALOG_CHALLENGE,
    MODULE_INTERNAL_TARGET,
    MODULE_INTERNAL_BOUNCER2,
    MODULE_INTERNAL_TEST,
    MODULE_INTERNAL_WIDGET_SELECTOR,
    MODULE_INTERNAL_WIDGETTEST,
    MODULE_INTERNAL_WAIT_INFO,
    MODULE_INTERNAL_TRANSITION,
    MODULE_TRANSITORY,
    MODULE_AUTH,

    MODULE_UNKNOWN
};

inline ModuleIndex get_internal_module_id_from_target(std::string_view target_name)
{
    struct {
        std::string_view name;
        ModuleIndex id;
    } names_id[5] = {
            {"bouncer2",           MODULE_INTERNAL_BOUNCER2},
            {"autotest",           MODULE_INTERNAL_TEST},
            {"widget_message",     MODULE_INTERNAL_WIDGET_SELECTOR},
            {"widgettest",         MODULE_INTERNAL_WIDGETTEST},
            {"card",               MODULE_INTERNAL_CARD},
    };
    ModuleIndex mi = MODULE_EXIT;
    for (auto f: names_id){
        if (f.name == target_name){
            mi = f.id;
            break;
        }
    }
    return mi;
}


inline ModuleIndex get_module_id(std::string_view module_name)
{
    struct {
        std::string_view name;
        ModuleIndex id;
    } names_id[20] = {
            {"login",              MODULE_INTERNAL_WIDGET_LOGIN},
            {"selector",           MODULE_INTERNAL_WIDGET_SELECTOR},
            {"selector_legacy",    MODULE_INTERNAL_WIDGET_SELECTOR},
            {"confirm",            MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE},
            {"challenge",          MODULE_INTERNAL_DIALOG_CHALLENGE},
            {"valid",              MODULE_INTERNAL_DIALOG_VALID_MESSAGE},
            {"transitory",         MODULE_TRANSITORY},
            {"close",              MODULE_INTERNAL_CLOSE},
            {"close_back",         MODULE_INTERNAL_CLOSE_BACK},
            {"interactive_target", MODULE_INTERNAL_TARGET},
            {"RDP",                MODULE_RDP},
            {"VNC",                MODULE_VNC},
            {"INTERNAL",           MODULE_INTERNAL},
            {"waitinfo",           MODULE_INTERNAL_WAIT_INFO},
            {"bouncer2",           MODULE_INTERNAL_BOUNCER2},
            {"autotest",           MODULE_INTERNAL_TEST},
            {"widget_message",     MODULE_INTERNAL_WIDGET_SELECTOR},
            {"widget_test",        MODULE_INTERNAL_WIDGETTEST},
            {"card",               MODULE_INTERNAL_CARD},
            {"exit",               MODULE_INTERNAL_CARD}
    };

    ModuleIndex mi = MODULE_UNKNOWN;
    for (auto f: names_id){
        if (f.name == module_name){
            mi = f.id;
            break;
        }
    }
    return mi;
}

inline const char * get_module_name(ModuleIndex module_id) noexcept
{
    switch (module_id) {
        case MODULE_EXIT:                            return "MODULE_EXIT";
        case MODULE_VNC:                             return "MODULE_VNC";
        case MODULE_RDP:                             return "MODULE_RDP";
        case MODULE_XUP:                             return "MODULE_XUP";
        case MODULE_INTERNAL:                        return "MODULE_INTERNAL";
        case MODULE_INTERNAL_CLOSE:                  return "MODULE_INTERNAL_CLOSE";
        case MODULE_INTERNAL_CLOSE_BACK:             return "MODULE_INTERNAL_CLOSE_BACK";
        case MODULE_INTERNAL_WIDGET_LOGIN:           return "MODULE_INTERNAL_WIDGET_LOGIN";
        case MODULE_INTERNAL_CARD:                   return "MODULE_INTERNAL_CARD";
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE: return "MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE";
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:   return "MODULE_INTERNAL_DIALOG_VALID_MESSAGE";
        case MODULE_INTERNAL_DIALOG_CHALLENGE:       return "MODULE_INTERNAL_DIALOG_CHALLENGE";
        case MODULE_INTERNAL_TARGET:                 return "MODULE_INTERNAL_TARGET";
        case MODULE_INTERNAL_BOUNCER2:               return "MODULE_INTERNAL_BOUNCER2";
        case MODULE_INTERNAL_TEST:                   return "MODULE_INTERNAL_TEST";
        case MODULE_INTERNAL_WIDGET_SELECTOR:        return "MODULE_INTERNAL_WIDGET_SELECTOR";
        case MODULE_INTERNAL_WIDGETTEST:             return "MODULE_INTERNAL_WIDGETTEST";
        case MODULE_INTERNAL_WAIT_INFO:              return "MODULE_INTERNAL_WAIT_INFO";
        case MODULE_INTERNAL_TRANSITION:             return "MODULE_INTERNAL_TRANSITION";
        case MODULE_TRANSITORY:                      return "MODULE_TRANSITORY";
        case MODULE_AUTH:                            return "MODULE_AUTH";
        case MODULE_UNKNOWN: break;
    }

    return "<unknown>";
}
