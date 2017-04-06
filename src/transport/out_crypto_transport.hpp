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
 *   Copyright (C) Wallix 2010-2017
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
        this->tmpname[0] = 0;
        this->finalname[0] = 0;
    } 

    const char * get_tmp(){
        return &this->tmpname[0];
    }

    ~OutCryptoTransport() {
        if (this->fd == -1){
            return;
        }
        try {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH]{};
            uint8_t fhash[MD_HASH::DIGEST_LENGTH]{};
            this->close(qhash, fhash);
            if (this->with_checksum){
                char mes[MD_HASH::DIGEST_LENGTH*4+1+128]{};
                char * p = mes;
                p+= sprintf(mes, "Encrypted transport implicitely closed, hash checksums dropped :");
                auto hexdump = [&p](uint8_t (&hash)[MD_HASH::DIGEST_LENGTH]) {
                    *p++ = ' ';                // 1 octet
                    for (unsigned c : hash) {
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
        catch (Error e){
            LOG(LOG_INFO, "Exception raised in ~OutCryptoTransport %d", e.id);
        }
    }

    // TODO: CGR: I want to remove that from Transport API
    bool disconnect() override {
        return 0;
    }

    bool is_open()
    {
        return this->fd != -1;
    }

    void open(const char * finalname, int groupid)
    {
        // This should avoid double open, we do not want that
        if (this->fd != -1){
            LOG(LOG_ERR, "OutCryptoTransport::open (double open error) %s", finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        // also ensure pathes are not to long, we will copy them in the object
        if (strlen(finalname) >= 2047-15){
            LOG(LOG_ERR, "OutCryptoTransport::open finalname oversize");
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        snprintf(this->tmpname, sizeof(this->tmpname), "%sred-XXXXXX.tmp", finalname);
        this->fd = ::mkostemps(this->tmpname, 4, O_WRONLY | O_CREAT);
        if (this->fd == -1){
            LOG(LOG_ERR, "OutCryptoTransport::open : open failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        if (chmod(this->tmpname, groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                , this->tmpname
                , groupid ? "u+r, g+r" : "u+r"
                , strerror(errno), errno);
            LOG(LOG_INFO, "OutCryptoTransport::open : chmod failed (%s -> %s)", this->tmpname, finalname);
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        strcpy(this->finalname, finalname);
        size_t derivator_len = 0;
        const uint8_t * derivator = reinterpret_cast<const uint8_t *>(basename_len(finalname, derivator_len));

        ocrypto::Result res = this->encrypter.open(derivator, derivator_len);
        this->raw_write(res.buf.data(), res.buf.size());
    }
    
    void close(uint8_t (&qhash)[MD_HASH::DIGEST_LENGTH], uint8_t (&fhash)[MD_HASH::DIGEST_LENGTH])
    {
        // This should avoid double closes, we do not want that
        if (this->fd == -1){
            LOG(LOG_ERR, "OutCryptoTransport::close error (double close error)");
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }
        const ocrypto::Result res = this->encrypter.close(qhash, fhash);
        this->raw_write(res.buf.data(), res.buf.size());
        if (this->tmpname[0] != 0){
            if (::rename(this->tmpname, this->finalname) < 0) {
                LOG(LOG_ERR, "OutCryptoTransport::close Renaming file \"%s\" -> \"%s\" failed, errno=%u : %s\n"
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
        LOG(LOG_ERR, "OutCryptoTransport::do_send failed: file not opened (%s->%s)", this->tmpname, this->finalname);
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
