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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include "transport/transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/fileutils.hpp"
#include "capture/cryptofile.hpp"

class OutCryptoTransport : public Transport
{
    ocrypto encrypter;
    int fd;
public:
    explicit OutCryptoTransport(bool with_encryption,
                                bool with_checksum,
                                CryptoContext & cctx,
                                Random & rnd) noexcept
        : encrypter(with_encryption, with_checksum, cctx, rnd)
        , fd(-1)
    {} 

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override {
        return 0;
    }

    void open(int fd, const char * tmpname, const char * finalname)
    {
        size_t derivator_len = 0;
        const uint8_t * derivator = reinterpret_cast<const uint8_t *>(basename_len(finalname, derivator_len));

        ocrypto::Result res = encrypter.open(derivator, sizeof(derivator_len));
        // TODO: write header data
    }
    
    void close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        // This should avoid double closes, we do not want that
        if (this->fd == -1){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        // TODO: close encryption and write data
    }

private:
    void do_send(const uint8_t * data, size_t len) override 
    {
        if (this->fd == -1){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        // TODO: send data to encrypter
        // TODO: crypt if necessary
    }
};
