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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
               Dominique Lafages, Raphael Zhou, Meng Tan,
               Jennifer Inthavong
    Based on xrdp Copyright (C) Jay Sorg 2004-2010

    Front object (server), used to communicate with RDP client
*/

#pragma once

#include <string>

#include "configs/config.hpp"
#include "gdi/screen_info.hpp"

struct SesmanInterface
{
    bool screen_info_sent = false;
    bool auth_info_sent = false;
    ScreenInfo screen_info;
    std::string username;
    std::string domain;
    std::string password;
    Inifile & ini;

    SesmanInterface(Inifile & ini) : ini(ini)
    {
    }

    void set_screen_info(ScreenInfo & screen_info)
    {
        this->screen_info_sent = false;
        this->screen_info = screen_info;
    }

    void set_auth_info(std::string username, std::string domain, std::string password)
    {
        this->auth_info_sent = false;
        this->username = username;
        this->domain = domain;
        this->password = password;
    }

    void set_acl_screen_info(){
        if (!this->screen_info_sent) {
            this->ini.set_acl<cfg::context::opt_width>(this->screen_info.width);
            this->ini.set_acl<cfg::context::opt_height>(this->screen_info.height);
            this->ini.set_acl<cfg::context::opt_bpp>(safe_int(screen_info.bpp));
            this->screen_info_sent = true;
        }
    }

    void set_acl_auth_info(){
        if (!this->auth_info_sent) {
            std::string username = this->username;
            if (not domain.empty()
             && (username.find('@') == std::string::npos)
             && (username.find('\\') == std::string::npos)) {
                username = username + std::string("@") + domain;
            }

            LOG(LOG_INFO, "set_acl_auth_info: auth_user=%s", username);
            this->ini.set_acl<cfg::globals::auth_user>(username);
            this->ini.ask<cfg::context::selector>();
            this->ini.ask<cfg::globals::target_user>();
            this->ini.ask<cfg::globals::target_device>();
            this->ini.ask<cfg::context::target_protocol>();
            if (!password.empty()) {
                this->ini.set_acl<cfg::context::password>(password);
            }
            this->auth_info_sent = true;
        }
    }
};

