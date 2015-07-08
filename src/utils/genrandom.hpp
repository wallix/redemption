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
#include "noncopyable.hpp"
#include "fdbuf.hpp"

class Random : noncopyable
{
    public:
    virtual ~Random() {}
    virtual void random(void * dest, size_t size) = 0;
};

class LCGRandom : public Random
{
    uint64_t seed;
    public:
        explicit LCGRandom(uint32_t seed)
        : seed(seed)
        {
        }

    ~LCGRandom() override {}

    void random(void * dest, size_t size) override {
        for (size_t x = 0; x < size ; x++){
            ((uint32_t*)dest)[x / sizeof(uint32_t)] = this->rand32();
        }
    }

    uint32_t rand32()
    {
        return this->seed = 999331UL * this->seed + 200560490131ULL;
    }
};

class LCGRand : public Random
{
    uint64_t seed;
    public:
        explicit LCGRand(uint32_t seed)
        : seed(seed)
        {
        }

    ~LCGRand() override {}

    void random(void * dest, size_t size) override {
        for (size_t x = 0; x < size ; x++){
            ((uint32_t*)dest)[x / sizeof(uint32_t)] = this->rand32();
        }
    }

    uint32_t rand32()
    {
        return this->seed = 999331UL * this->seed + 7913UL;
    }
};

class UdevRandom : public Random
{
    public:
    UdevRandom()
    {
    }
    ~UdevRandom() override {}

    void random(void * dest, size_t size) override {
        io::posix::fdbuf file(open("/dev/urandom", O_RDONLY));
        if (!file.is_open()) {
            LOG(LOG_INFO, "using /dev/random as random source");
            file.open("/dev/random", O_RDONLY);
            if (!file.is_open()) {
                LOG(LOG_WARNING, "random source failed to provide random data : couldn't open device\n");
            }
        }
        else {
            LOG(LOG_INFO, "using /dev/urandom as random source");
        }

        ssize_t res = file.read(dest, size);
        if (res != static_cast<ssize_t>(size)) {
            if (res >= 0){
                LOG(LOG_ERR, "random source failed to provide enough random data [%u]", res);
            }
            else {
                LOG(LOG_ERR, "random source failed to provide random data [%u]", strerror(errno));
            }
            memset(dest, 0x44, size);
        }
    }
};


#endif
