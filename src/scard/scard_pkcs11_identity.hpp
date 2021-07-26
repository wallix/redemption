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
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <string>
#include <string_view>
#include <vector>


///////////////////////////////////////////////////////////////////////////////


class scard_pkcs11_identity
{
public:
    ///
    scard_pkcs11_identity(std::string_view module_name,
        std::string_view slot_id, std::string_view token_label,
        std::string_view certificate_id, std::string_view principal_name);
    
    ///
    std::string_view module_name() const;

    ///
    std::string_view slot_id() const;

    ///
    std::string_view token_label() const;

    ///
    std::string_view certificate_id() const;

    ///
    std::string_view principal_name() const;

    ///
    std::string string() const;

private:
    ///
    const std::string _module_name;

    ///
    const std::string _slot_id;

    ///
    const std::string _token_label;

    ///
    const std::string _certificate_id;

    ///
    const std::string _principal_name;
};

///
typedef std::vector<scard_pkcs11_identity> scard_pkcs11_identity_list;