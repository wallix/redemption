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

#include <krb5.h>


///////////////////////////////////////////////////////////////////////////////


///
class scard_krb_client_handler
{
public:
    ///
    virtual ~scard_krb_client_handler() = default;
    
    ///
    virtual void handle_krb_client_result(bool success) = 0;
};

///
class scard_krb_client
{
public:
    ///
    scard_krb_client(scard_krb_client_handler *handler_ptr = nullptr);

    ///
    ~scard_krb_client();

    ///
    bool get_credentials(std::string pkcs11_identity,
        std::string principal_name, std::string security_code);

private:
    ///
    scard_krb_client_handler *_handler_ptr;

    ///
    krb5_context _context;

    ///
    static krb5_error_code prompt_password(krb5_context ctx, void *data,
        const char *name, const char *banner, int num_prompts,
        krb5_prompt prompts[]);
};