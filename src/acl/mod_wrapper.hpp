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
  Copyright (C) Wallix 2019
  Author(s): Christophe Grosjean

*/
#pragma once

#include "mod/null/null.hpp"
#include "mod/mod_api.hpp"

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

