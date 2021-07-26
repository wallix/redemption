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
    Copyright (C) Wallix 2020
    Author(s): 
*/

#include <sstream>

#include "scard/scard_pkcs11_identity.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pkcs11_identity::scard_pkcs11_identity(std::string_view module_name,
    std::string_view slot_id, std::string_view token_label,
    std::string_view certificate_id, std::string_view principal_name)
    :
    _module_name(module_name),
    _slot_id(slot_id),
    _token_label(token_label),
    _certificate_id(certificate_id),
    _principal_name(principal_name)
{
}

std::string_view scard_pkcs11_identity::module_name() const
{
    return _module_name;
}

std::string_view scard_pkcs11_identity::slot_id() const
{
    return _slot_id;
}

std::string_view scard_pkcs11_identity::token_label() const
{
    return _token_label;
}

std::string_view scard_pkcs11_identity::certificate_id() const
{
    return _certificate_id;
}

std::string_view scard_pkcs11_identity::principal_name() const
{
    return _principal_name;
}

std::string scard_pkcs11_identity::string() const
{
    std::ostringstream stream;

    stream
        << "PKCS11:module_name=" << _module_name << ":"
        << "slotid=" << _slot_id << ":"
        << "token=" << _token_label << ":"
        << "certid=" << _certificate_id
    ;

    return stream.str();
}