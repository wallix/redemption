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

#include "acl/module_manager/enums.hpp"
#include "acl/module_manager/mm_ini.hpp"
#include "configs/config.hpp"
#include "core/RDP/gcc/userdata/cs_monitor.hpp"
#include "core/session_reactor.hpp"


Rect get_widget_rect(uint16_t width, uint16_t height, GCC::UserData::CSMonitor const & monitors)
{
    Rect widget_rect(0, 0, width - 1, height - 1);
    if (monitors.monitorCount) {
        Rect rect                 = monitors.get_rect();
        Rect primary_monitor_rect = monitors.get_primary_monitor_rect();

        widget_rect.x  = abs(rect.x);
        widget_rect.y  = abs(rect.y);
        widget_rect.cx = primary_monitor_rect.cx;
        widget_rect.cy = primary_monitor_rect.cy;
    }

    return widget_rect;
}


void MMIni::new_mod(int target_module, time_t now, AuthApi & /*unused*/, ReportMessageApi & /*unused*/)
{
    LOG(LOG_INFO, "new mod %d at time: %d\n", target_module, static_cast<int>(now));
    switch (target_module) {
    case MODULE_VNC:
    case MODULE_XUP:
    case MODULE_RDP:
        this->connected = true;
        break;
    default:
        break;
    };
}

void MMIni::invoke_close_box(
    const char * auth_error_message, BackEvent_t & signal,
    time_t now, AuthApi & authentifier, ReportMessageApi & report_message)
{
    LOG(LOG_INFO, "----------> ACL invoke_close_box <--------");
    this->last_module = true;
    if (auth_error_message) {
        this->ini.set<cfg::context::auth_error_message>(auth_error_message);
    }
    if (this->mod) {
        try {
            this->mod->disconnect(now);
        }
        catch (Error const& e) {
            LOG(LOG_INFO, "MMIni::invoke_close_box exception = %u!\n", e.id);
        }
    }

    this->remove_mod();
    if (this->ini.get<cfg::globals::enable_close_box>()) {
        this->new_mod(MODULE_INTERNAL_CLOSE, now, authentifier, report_message);
        signal = BACK_EVENT_NONE;
    }
    else {
        detail::log_proxy_logout(this->ini.get<cfg::context::auth_error_message>().c_str());
        signal = BACK_EVENT_STOP;
    }
}

int MMIni::next_module()
{
    LOG(LOG_INFO, "----------> ACL next_module <--------");
    auto & module_cstr = this->ini.get<cfg::context::module>();

    if (module_cstr == STRMODULE_LOGIN) {
        LOG(LOG_INFO, "===========> MODULE_LOGIN");
        return MODULE_INTERNAL_WIDGET_LOGIN;
    }
    if (module_cstr == STRMODULE_SELECTOR) {
        LOG(LOG_INFO, "===============> MODULE_SELECTOR");
        return MODULE_INTERNAL_WIDGET_SELECTOR;
    }
    if (module_cstr == STRMODULE_SELECTOR_LEGACY) {
        LOG(LOG_INFO, "===============> MODULE_SELECTOR_LEGACY");
        return MODULE_INTERNAL_WIDGET_SELECTOR_LEGACY;
    }
    if (module_cstr == STRMODULE_CONFIRM) {
        LOG(LOG_INFO, "===============> MODULE_DIALOG_CONFIRM");
        return MODULE_INTERNAL_DIALOG_DISPLAY_MESSAGE;
    }
    if (module_cstr == STRMODULE_CHALLENGE) {
        LOG(LOG_INFO, "===========> MODULE_DIALOG_CHALLENGE");
        return MODULE_INTERNAL_DIALOG_CHALLENGE;
    }
    if (module_cstr == STRMODULE_VALID) {
        LOG(LOG_INFO, "===========> MODULE_DIALOG_VALID");
        return MODULE_INTERNAL_DIALOG_VALID_MESSAGE;
    }
    if (module_cstr == STRMODULE_WAITINFO) {
        LOG(LOG_INFO, "===========> MODULE_WAITINFO");
        return MODULE_INTERNAL_WAIT_INFO;
    }
    if (module_cstr == STRMODULE_TARGET) {
        LOG(LOG_INFO, "===========> MODULE_INTERACTIVE_TARGET");
        return MODULE_INTERNAL_TARGET;
    }
    if (module_cstr == STRMODULE_TRANSITORY) {
        LOG(LOG_INFO, "===============> WAIT WITH CURRENT MODULE");
        return MODULE_TRANSITORY;
    }
    if (module_cstr == STRMODULE_CLOSE) {
        LOG(LOG_INFO, "===========> MODULE_INTERNAL_CLOSE (1)");
        return MODULE_INTERNAL_CLOSE;
    }
    if (module_cstr == STRMODULE_CLOSE_BACK) {
        LOG(LOG_INFO, "===========> MODULE_INTERNAL_CLOSE_BACK");
        return MODULE_INTERNAL_CLOSE_BACK;
    }
    if (this->connected &&
        (module_cstr == STRMODULE_RDP ||
            module_cstr == STRMODULE_VNC)) {
        LOG(LOG_INFO, "===========> MODULE_CLOSE");
        if (this->ini.get<cfg::context::auth_error_message>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(this->ini)));
        }
        return MODULE_INTERNAL_CLOSE;
    }
    if (module_cstr == STRMODULE_RDP) {
        LOG(LOG_INFO, "===========> MODULE_RDP");
        return MODULE_RDP;
    }
    if (module_cstr == STRMODULE_VNC) {
        LOG(LOG_INFO, "===========> MODULE_VNC");
        return MODULE_VNC;
    }
    if (module_cstr == STRMODULE_INTERNAL) {
        int res = MODULE_EXIT;
        auto & target = this->ini.get<cfg::context::target_host>();
        if (target == "bouncer2") {
            LOG(LOG_INFO, "==========> MODULE_INTERNAL bouncer2");
            res = MODULE_INTERNAL_BOUNCER2;
        }
        else if (target == "autotest") {
            LOG(LOG_INFO, "==========> MODULE_INTERNAL test");
            res = MODULE_INTERNAL_TEST;
        }
        else if (target == "widget_message") {
            LOG(LOG_INFO, "==========> MODULE_INTERNAL widget_message");
            res = MODULE_INTERNAL_WIDGET_MESSAGE;
        }
        else if (target == "widgettest") {
            LOG(LOG_INFO, "==========> MODULE_INTERNAL widgettest");
            res = MODULE_INTERNAL_WIDGETTEST;
        }
        else {
            LOG(LOG_INFO, "==========> MODULE_INTERNAL card");
            res = MODULE_INTERNAL_CARD;
        }
        return res;
    }
    LOG(LOG_INFO, "===========> UNKNOWN MODULE");
    return MODULE_INTERNAL_CLOSE;
}

void MMIni::check_module()
{
    if (this->ini.get<cfg::context::forcemodule>() && !this->is_connected()) {
        this->session_reactor.set_event_next(BACK_EVENT_NEXT);
        this->ini.set<cfg::context::forcemodule>(false);
        // Do not send back the value to sesman.
    }
}
