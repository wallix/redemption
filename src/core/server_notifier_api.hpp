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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#pragma once

#ifndef __EMSCRIPTEN__
#  include <openssl/ossl_typ.h>
#else
  struct X509 {};
#endif

#include <string_view>
#include <string>

enum class CertificateResult { wait, valid, invalid, };

// TODO only for tls_check_certificate...
class ServerNotifier
{
public:
    enum class Status
    {
        AccessAllowed,
        CertCreate,
        CertSuccess,
        CertFailure,
        CertError,
    };

    virtual void server_cert_status(Status status, std::string_view error_msg = {}) = 0; /*NOLINT*/

    virtual CertificateResult server_cert_callback(X509& certificate, std::string* error_message, const char* ip_address, int port) = 0;

    virtual ~ServerNotifier() = default;
};

class NullServerNotifier : public ServerNotifier
{
public:
    void server_cert_status(Status status, std::string_view error_msg = {}) override /*NOLINT*/
    {
        (void)status;
        (void)error_msg;
    }

    CertificateResult server_cert_callback(X509& certificate, std::string* error_message, const char* ip_address, int port) override
    {
        (void)certificate;
        (void)ip_address;
        (void)port;
        (void)error_message;
        return CertificateResult::valid;
    }
};
