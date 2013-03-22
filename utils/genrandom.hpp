/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean,

   Random Generators

*/

#ifndef _REDEMPTION_UTILS_GENRANDOM_HPP_
#define _REDEMPTION_UTILS_GENRANDOM_HPP_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <stdint.h>
#include "log.hpp"


class Random
{
    public:
        virtual void random(void * dest, size_t size) = 0;
};

class LCGRandom : public Random
{
    uint64_t seed;
    public:
        LCGRandom(uint32_t seed)
        : seed(seed)
        {
        }

    virtual void random(void * dest, size_t size)
    {
        for (size_t x = 0; x < size ; x++){
            ((uint32_t*)dest)[x / sizeof(uint32_t)] = this->rand32();
        }
    }

    uint32_t rand32()
    {
        return this->seed = (uint64_t)999331UL * (uint64_t)this->seed + 200560490131ULL;
    }
};

class UdevRandom : public Random
{
    public:
    UdevRandom()
    {
    }

    virtual void random(void * dest, size_t size)
    {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1) {
            fd = open("/dev/random", O_RDONLY);
            if (fd == -1) {
                LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
            }
            LOG(LOG_INFO, "using /dev/random as random source");
        }
        else {
            LOG(LOG_INFO, "using /dev/urandom as random source");
        }

        ssize_t res = read(fd, dest, size);
        if (res != (ssize_t)size) {
            if (res >= 0){
                LOG(LOG_ERR, "random source failed to provide enough random data [%u]", res);
            }
            else {
                LOG(LOG_ERR, "random source failed to provide random data [%u]", strerror(errno));
            }
            memset(dest, 0x44, size);
        }
        close(fd);
    }
};


#endif
