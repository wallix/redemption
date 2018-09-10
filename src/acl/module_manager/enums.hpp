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

#define STRMODULE_LOGIN            "login"
#define STRMODULE_SELECTOR         "selector"
#define STRMODULE_SELECTOR_LEGACY  "selector_legacy"
#define STRMODULE_CONFIRM          "confirm"
#define STRMODULE_CHALLENGE        "challenge"
#define STRMODULE_VALID            "valid"
#define STRMODULE_TRANSITORY       "transitory"
#define STRMODULE_CLOSE            "close"
#define STRMODULE_CLOSE_BACK       "close_back"
#define STRMODULE_CONNECTION       "connection"
#define STRMODULE_TARGET           "interactive_target"
#define STRMODULE_MESSAGE          "message"
#define STRMODULE_RDP              "RDP"
#define STRMODULE_VNC              "VNC"
#define STRMODULE_INTERNAL         "INTERNAL"
#define STRMODULE_WAITINFO         "waitinfo"

// TODO enum class
enum {
    MODULE_EXIT,
    MODULE_WAITING,
    MODULE_RUNNING,
    MODULE_REFRESH,
    MODULE_VNC,
    MODULE_RDP,
    MODULE_XUP,
    MODULE_INTERNAL,
    MODULE_INTERNAL_CLOSE,
    MODULE_INTERNAL_CLOSE_BACK,
    MODULE_INTERNAL_WIDGET_DIALOG,
    MODULE_INTERNAL_WIDGET_MESSAGE,
    MODULE_INTERNAL_WIDGET_LOGIN,
    MODULE_INTERNAL_CARD,
    MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE,
    MODULE_INTERNAL_DIALOG_VALID_MESSAGE,
    MODULE_INTERNAL_DIALOG_CHALLENGE,
    MODULE_INTERNAL_TARGET,
    MODULE_INTERNAL_BOUNCER2,
    MODULE_INTERNAL_TEST,
    MODULE_INTERNAL_WIDGET_SELECTOR,
    MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY,
    MODULE_INTERNAL_WIDGETTEST,
    MODULE_INTERNAL_WAIT_INFO,
    MODULE_EXIT_INTERNAL_CLOSE,
    MODULE_TRANSITORY,
    MODULE_AUTH,
    MODULE_CLI,

    MODULE_UNKNOWN
};

inline const char * get_module_name(int module_id) noexcept
{
    switch (module_id) {
        case MODULE_EXIT:                               return "MODULE_EXIT";
        case MODULE_WAITING:                            return "MODULE_WAITING";
        case MODULE_RUNNING:                            return "MODULE_RUNNING";
        case MODULE_REFRESH:                            return "MODULE_REFRESH";
        case MODULE_VNC:                                return "MODULE_VNC";
        case MODULE_RDP:                                return "MODULE_RDP";
        case MODULE_XUP:                                return "MODULE_XUP";
        case MODULE_INTERNAL:                           return "MODULE_INTERNAL";
        case MODULE_INTERNAL_CLOSE:                     return "MODULE_INTERNAL_CLOSE";
        case MODULE_INTERNAL_CLOSE_BACK:                return "MODULE_INTERNAL_CLOSE_BACK";
        case MODULE_INTERNAL_WIDGET_DIALOG:             return "MODULE_INTERNAL_WIDGET_DIALOG";
        case MODULE_INTERNAL_WIDGET_MESSAGE:            return "MODULE_INTERNAL_WIDGET_MESSAGE";
        case MODULE_INTERNAL_WIDGET_LOGIN:              return "MODULE_INTERNAL_WIDGET_LOGIN";
        case MODULE_INTERNAL_CARD:                      return "MODULE_INTERNAL_CARD";
        case MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE:    return "MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE";
        case MODULE_INTERNAL_DIALOG_VALID_MESSAGE:      return "MODULE_INTERNAL_DIALOG_VALID_MESSAGE";
        case MODULE_INTERNAL_DIALOG_CHALLENGE:          return "MODULE_INTERNAL_DIALOG_CHALLENGE";
        case MODULE_INTERNAL_TARGET:                    return "MODULE_INTERNAL_TARGET";
        case MODULE_INTERNAL_BOUNCER2:                  return "MODULE_INTERNAL_BOUNCER2";
        case MODULE_INTERNAL_TEST:                      return "MODULE_INTERNAL_TEST";
        case MODULE_INTERNAL_WIDGET_SELECTOR:           return "MODULE_INTERNAL_WIDGET_SELECTOR";
        case MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY:    return "MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY";
        case MODULE_INTERNAL_WIDGETTEST:                return "MODULE_INTERNAL_WIDGETTEST";
        case MODULE_INTERNAL_WAIT_INFO:                 return "MODULE_INTERNAL_WAIT_INFO";
        case MODULE_EXIT_INTERNAL_CLOSE:                return "MODULE_EXIT_INTERNAL_CLOSE";
        case MODULE_TRANSITORY:                         return "MODULE_TRANSITORY";
        case MODULE_AUTH:                               return "MODULE_AUTH";
        case MODULE_CLI:                                return "MODULE_CLI";
    }

    return "<unknown>";
}
