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
#include "utils/sugar/iter.hpp"
#include "capture/cryptofile.hpp"

class OutCryptoTransport : public Transport
{
    bool with_checksum;
    ocrypto encrypter;
    char tmpname[2048];
    char finalname[2048];
    int fd;
public:
    explicit OutCryptoTransport(bool with_encryption,
                                bool with_checksum,
                                CryptoContext & cctx,
                                Random & rnd) noexcept
        : with_checksum(with_checksum)
        , encrypter(with_encryption, with_checksum, cctx, rnd)
        , fd(-1)
    {
        tmpname[0] = 0;
        finalname[0] = 0;
    } 

    ~OutCryptoTransport() {
        LOG(LOG_INFO, "~OutCryptoTransport()");
        if (this->fd != -1){
            uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
            uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
            this->close(qhash, fhash);
            if (this->with_checksum){
                char mes[1024]{};
                char * p = mes;
                p+= sprintf(mes, "Encrypted transport implicitely closed, hash checksums dropped :");
                auto hexdump = [&p](uint8_t (&hash)[MD_HASH::DIGEST_LENGTH]) {
                    *p++ = ' ';                // 1 octet
                    for (unsigned c : iter(hash, MD_HASH::DIGEST_LENGTH)) {
                        sprintf(p, "%02x", c); // 64 octets (hash)
                        p += 2;
                    }
                };
                hexdump(qhash);
                hexdump(fhash);
                *p++ = 0;
                LOG(LOG_INFO, "%s", mes);
            }
        }
    }

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override {
        return 0;
    }

    void open(int fd, const char * tmpname, const char * finalname)
    {
        // This should avoid double open, we do not want that
        if (this->fd != -1){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        // also ensure pathes are not to long, we will copy them in the object
        if (strlen(tmpname) >= 2047){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        if (strlen(finalname) >= 2047){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        strcpy(this->tmpname, tmpname);
        strcpy(this->finalname, finalname);
        size_t derivator_len = 0;
        const uint8_t * derivator = reinterpret_cast<const uint8_t *>(basename_len(finalname, derivator_len));
        this->fd = fd;

        ocrypto::Result res = this->encrypter.open(derivator, sizeof(derivator_len));
        this->raw_write(res.buf.data(), res.buf.size());
    }
    
    void close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        // This should avoid double closes, we do not want that
        if (this->fd == -1){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        const ocrypto::Result res = this->encrypter.close(qhash, fhash);
        this->raw_write(res.buf.data(), res.buf.size());
        if (this->tmpname[0] != 0){
            if (::rename(this->tmpname, this->finalname) < 0) {
                LOG( LOG_ERR, "Renaming file \"%s\" -> \"%s\" failed errno=%u : %s\n"
                   , this->tmpname, this->finalname, errno, strerror(errno));
                ::close(this->fd);
                this->fd = -1;
                throw Error(ERR_TRANSPORT_WRITE_FAILED);
            }
            this->tmpname[0] = 0;
        }
        ::close(this->fd);
        this->fd = -1;
    }

private:
    void do_send(const uint8_t * data, size_t len) override 
    {
        if (this->fd == -1){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        const ocrypto::Result res = this->encrypter.write(data, len);
        this->raw_write(res.buf.data(), res.buf.size());
    }
    
    void raw_write(const uint8_t * data, size_t len)
    {
        size_t total_sent = 0;
        while (len > total_sent) {
            ssize_t ret = ::write(this->fd, &data[total_sent], len-total_sent);
            if (ret <= 0){
                if (errno == EINTR){
                    continue;
                }
                throw Error(ERR_TRANSPORT_WRITE_FAILED);
            }
            total_sent += ret;
        }
    }
    
};
