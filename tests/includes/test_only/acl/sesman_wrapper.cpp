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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include "test_only/acl/sesman_wrapper.hpp"
#include "acl/sesman.hpp"

InifileWrapper::InifileWrapper()
: ini(new Inifile) /*NOLINT*/
{}

InifileWrapper::~InifileWrapper()
{
    delete ini; /*NOLINT*/
}

struct SesmanWrapper::D
{
    Inifile ini;
    Sesman sesman;
    D() : sesman(this->ini) {}
};

SesmanWrapper::SesmanWrapper()
: d(new D) /*NOLINT*/
{}

SesmanWrapper::~SesmanWrapper()
{
    delete d; /*NOLINT*/
}

Inifile& SesmanWrapper::get_ini()
{
    return d->ini;
}

RedirectionInfo& SesmanWrapper::redir_info()
{
    return d->ini.get_mutable_ref<cfg::mod_rdp::redir_info>();
}

SesmanWrapper::operator AuthApi &() &
{
    return d->sesman;
}

Sesman& SesmanWrapper::get_sesman() &
{
    return d->sesman;
}
