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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   Transport layer abstraction, socket implementation with TLS support
*/


#pragma once

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

namespace
{
    bool cert_to_escaped_string(X509& cert, std::string& output)
    {
        // TODO unique_ptr<BIO, BIO_delete>
        BIO* bio = BIO_new(BIO_s_mem());
        if (!bio) {
            return false;
        }

        if (!PEM_write_bio_X509(bio, &cert)) {
            BIO_free(bio);
            return false;
        }

        std::size_t pem_len = BIO_number_written(bio);
        output.resize(pem_len);
        std::fill(output.data(), output.data() + pem_len, 0);

        BIO_read(bio, output.data(), pem_len);
        BIO_free(bio);

        std::replace(output.begin(), output.end(), '\n', '\x01');

        return true;
    }
} // anonymous namespace
