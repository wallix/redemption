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

#include "utils/sugar/bytes_view.hpp"

#include "scard/scard_pkcs11_common.hpp"
#include "scard/scard_pkcs11_identity.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_pkcs11_enumerator_handler
{
public:
    ///
    virtual ~scard_pkcs11_enumerator_handler() = default;

    ///
    virtual void handle_pkcs11_enumeration_start() = 0;

    ///
    virtual std::size_t provide_pkcs11_security_code(
        writable_bytes_view buffer) = 0;

    ///
    virtual void handle_pkcs11_enumeration_end(
        const scard_pkcs11_identity_list &identities) = 0;
};

class scard_pkcs11_enumerator
{
public:
    ///
    scard_pkcs11_enumerator(std::string_view module_path,
        scard_pkcs11_enumerator_handler *handler_ptr = nullptr);

    ///
    scard_pkcs11_identity_list enumerate() const;

private:
    ///
    const std::string _module_path;

    ///
    scard_pkcs11_enumerator_handler *_handler_ptr;
};