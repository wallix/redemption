/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "acl/module_manager/enums.hpp"
#include "core/back_event_t.hpp"
#include "mod/mod_api.hpp"
#include "mod/null/null.hpp"

class rdp_api;
class AuthApi;
class ReportMessageApi;

struct ModWrapper
{
    null_mod no_mod;
    mod_api* mod = &no_mod;

    mod_api* get_mod()
    {
        return this->mod;
    }

    bool has_mod() const {
        return (this->mod != &this->no_mod);
    }

    void remove_mod()
    {
        delete this->mod;
        this->mod = &this->no_mod;
    }

    bool is_up_and_running() const {
        return this->has_mod() && this->mod->is_up_and_running();
    }

    [[nodiscard]] mod_api const* get_mod() const
    {
        return this->mod;
    }
    
    void set_mod(mod_api* mod)
    {
        // TODO: check we are using no_mod, otherwise it is an error
        this->mod = mod;
    }
};


class MMApi
{
protected:
    ModWrapper & mw;
public:

    ModWrapper & get_mod_wrapper() 
    {
        return mw;
    }

    mod_api* get_mod()
    {
        return this->mw.get_mod();
    }

    [[nodiscard]] mod_api const* get_mod() const
    {
        return this->mw.get_mod();
    }

public:
    bool last_module{false};
    bool connected{false};

    MMApi(ModWrapper & mod_wrapper) : mw(mod_wrapper) {}
    virtual ~MMApi() = default;
    virtual void remove_mod() = 0;
    virtual void new_mod(ModuleIndex target_module, AuthApi &, ReportMessageApi &) = 0;
    virtual ModuleIndex next_module() = 0;
    // virtual int get_mod_from_protocol() = 0;
    virtual void invoke_close_box(bool /*enable_close_box*/, const char * auth_error_message, BackEvent_t & signal, AuthApi & /*unused*/, ReportMessageApi & /*unused*/) {
    
        (void)auth_error_message;
        (void)signal;
        this->last_module = true;
    }
    virtual bool is_connected() {
        return this->connected;
    }
    virtual bool is_up_and_running() {
        return this->mw.is_up_and_running();
    }
    virtual void check_module() {}

    [[nodiscard]] virtual rdp_api* get_rdp_api() const { return nullptr; }
};
