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


void MMIni::new_mod(ModuleIndex target_module, AuthApi & /*unused*/, ReportMessageApi & /*unused*/)
{
    LOG(LOG_INFO, "new mod %d", target_module);
    switch (target_module) {
    case MODULE_VNC:
    case MODULE_XUP:
    case MODULE_RDP:
        this->connected = true;
        break;
    default:
        break;
    }
}

void MMIni::invoke_close_box(
    const char * auth_error_message, BackEvent_t & signal,
    AuthApi & authentifier, ReportMessageApi & report_message)
{
    LOG(LOG_INFO, "----------> ACL invoke_close_box <--------");
    this->last_module = true;
    if (auth_error_message) {
        this->ini.set<cfg::context::auth_error_message>(auth_error_message);
    }
    if (this->mod) {
        try {
            this->mod->disconnect();
        }
        catch (Error const& e) {
            LOG(LOG_INFO, "MMIni::invoke_close_box exception = %u!", e.id);
        }
    }

    this->remove_mod();
    if (this->ini.get<cfg::globals::enable_close_box>()) {
        this->new_mod(MODULE_INTERNAL_CLOSE, authentifier, report_message);
        signal = BACK_EVENT_NONE;
    }
    else {
        signal = BACK_EVENT_STOP;
    }
}

ModuleIndex MMIni::next_module()
{
    auto & module_cstr = this->ini.get<cfg::context::module>();
    auto module_id = get_module_id(module_cstr);
    LOG(LOG_INFO, "----------> ACL next_module : %s %u <--------", module_cstr, unsigned(module_id));
    
    if (this->connected && ((module_id == MODULE_RDP)||(module_id == MODULE_VNC))) {
        LOG(LOG_INFO, "===========> Connection close asked by admin while connected");
        if (this->ini.get<cfg::context::auth_error_message>().empty()) {
            this->ini.set<cfg::context::auth_error_message>(TR(trkeys::end_connection, language(this->ini)));
        }
        return MODULE_INTERNAL_CLOSE;
    }
    if (module_id == MODULE_INTERNAL)
    {
        auto module_id = get_internal_module_id_from_target(this->ini.get<cfg::context::target_host>());
        LOG(LOG_INFO, "===========> %s (from target)", get_module_name(module_id));
        return module_id;
    }
    if (module_id == MODULE_UNKNOWN)
    {
        LOG(LOG_INFO, "===========> UNKNOWN MODULE (closing)");
        return MODULE_INTERNAL_CLOSE;
    }
    return module_id;
}

void MMIni::check_module()
{
    if (this->ini.get<cfg::context::forcemodule>() && !this->is_connected()) {
        this->session_reactor.set_next_event(BACK_EVENT_NEXT);
        this->ini.set<cfg::context::forcemodule>(false);
        // Do not send back the value to sesman.
    }
}
