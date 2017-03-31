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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan, Clément Moroldo
 */


#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "transport/transport.hpp"
#include "utils/genrandom.hpp"
#include "utils/fileutils.hpp"
#include "capture/cryptofile.hpp"

class InCryptoTransport : public Transport
{
    int fd;
    bool eof;
    size_t file_len;
    size_t current_len;
public:
    explicit InCryptoTransport(CryptoContext & cctx, Random & rnd) noexcept
        : fd(-1)
        , eof(true)
        , file_len(0)
        , current_len(0)
    {
    } 

    ~InCryptoTransport() {
    }

    bool is_open()
    {
        return this->fd != -1;
    }

    void open(const char * pathname)
    {
        if (this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        
        struct stat sb;
        if (stat(pathname, &sb) == -1) {
            this->file_len = sb.st_size;
        }
        
        this->fd = ::open(pathname, O_RDONLY);
        this->eof = false;
    }
    
    
    void close()
    {
        if (!this->is_open()){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        ::close(this->fd);
        this->fd = -1;
        this->eof = true;
    }

    bool is_eof() {
        return this->eof;
    }

private:
    void do_recv_new(uint8_t * buffer, size_t len) override {
        ssize_t res = -1;
        size_t remaining_len = len;
        
        while(remaining_len){
            res = ::read(this->fd, buffer + (len - remaining_len), remaining_len);
            if (res <= 0){
                
                if ((res == 0) || ((errno != EINTR) && (remaining_len != len))){
                    break;
                }
                if (errno == EINTR){
                    continue;
                }
                this->status = false;
                throw Error(ERR_TRANSPORT_READ_FAILED, res);
            }
            remaining_len -= res;
        };
        
        res = len - remaining_len;
        this->current_len += res;
        if (this->file_len <= this->current_len) {
            this->eof = true;
        }
        this->last_quantum_received += res;
        if (remaining_len != 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, errno);
        }
    }    
};
