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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once


struct RedIO {
    ~RedIO() = delete;
};

struct RedIn : public RedIO {
    virtual ~RedIn() {};
    virtual int read(uint8_t * buffer, size_t len) = 0;
};

struct RedOut : public RedIO {
    virtual ~RedOut() {};
    virtual int write(const uint8_t * buffer, size_t len) = 0;
};

struct RedSeekOut : public RedOut {
    virtual ~RedSeekOut() {};
    // seek man for lseek
    virtual off_t lseek(off_t offset, int whence) = 0;
};


// TODO: use this to replace class ofile_buf_out

class RedOutFd : public RedOut {
    int fd;

public:
    RedOutFd(const char * filename, mode_t mode) : fd(-1) {
        this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
    }

    bool is_open(){
        return this->fd != -1;
    }

    virtual ~RedOut() { if (this->is_open()) { ::close(this->fd); } };
    virtual int write(const uint8_t * buffer, size_t len)
    {
        return ::write(this->fd, buffer, len);
    }
};


