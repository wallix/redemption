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

#include "utils/log.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

inline int urandom_read(unsigned char *buf, int sz)
{
    int fd = ::open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: open /dev/urandom!\n", getpid());
        return -1;
    }
    ssize_t read_ret = ::read(fd, buf, sz);
    if (read_ret == -1) {
        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: read /dev/urandom! error=%s\n", getpid(), strerror(errno));
    }
    int close_ret = ::close(fd);
    if (close_ret == -1) {
        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: close /dev/urandom! error=%s\n", getpid(), strerror(errno));
    }
    return read_ret;
}

