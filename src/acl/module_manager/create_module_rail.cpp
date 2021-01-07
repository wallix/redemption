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
Copyright (C) Wallix 2020
Author(s): Proxy Team
*/

#include "acl/module_manager/create_module_rail.hpp"
#include "mod/internal/rail_module_host_mod.hpp"
#include "RAIL/client_execute.hpp"
#include "utils/sugar/algostring.hpp"
#include "core/client_info.hpp"
#include "configs/config.hpp"


RailModuleHostMod* create_mod_rail(
    Inifile& ini,
    EventContainer& events,
    gdi::GraphicApi & drawable,
    FrontAPI& front,
    ClientInfo const& client_info,
    Rect const& widget_rect,
    std::unique_ptr<mod_api>&& managed_mod,
    ClientExecute& rail_client_execute,
    Font const& font,
    Theme const& theme,
    bool can_resize_hosted_desktop)
{
    // TODO: this should be external, not in mod_factory
    // Host Mod In Widget
    LOG(LOG_INFO, "Creation of internal module 'RailModuleHostMod'");

    std::string target_info = str_concat(
        ini.get<cfg::context::target_str>(),
        ':',
        ini.get<cfg::globals::primary_user_id>());

    rail_client_execute.set_target_info(target_info);

    struct RailMod final : RailModuleHostMod
    {
        using RailModuleHostMod::RailModuleHostMod;

        void set_ini(Inifile& ini)
        {
            this->ini = &ini;
            this->ini->set<cfg::context::rail_module_host_mod_is_active>(true);
        }

        ~RailMod()
        {
            this->ini->set<cfg::context::rail_module_host_mod_is_active>(false);
        }

    private:
        Inifile* ini;
    };

    auto* host_mod = new RailMod(
        events,
        drawable,
        front,
        client_info.screen_info.width,
        client_info.screen_info.height,
        widget_rect,
        std::move(managed_mod),
        rail_client_execute,
        font,
        theme,
        client_info.cs_monitor,
        can_resize_hosted_desktop
        && ini.get<cfg::remote_program::allow_resize_hosted_desktop>()
        && !ini.get<cfg::globals::is_rec>()
    );
    host_mod->init();
    host_mod->set_ini(ini);

    return host_mod;
}
