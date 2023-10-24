/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/random.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <cerrno>
#include <cstring>

#include <unistd.h>


class URandom final : public Random
{
    unique_fd ufd;

public:
    URandom()
    : ufd(open("/dev/urandom", O_RDONLY))
    {
        if (ufd.is_open()) {
            LOG(LOG_INFO, "using /dev/urandom as random source");
        }
        else {
            LOG(LOG_INFO, "access to /dev/urandom failed: %s", strerror(errno));

            ufd.reset(open("/dev/random", O_RDONLY));
            if (ufd.is_open()) {
                LOG(LOG_INFO, "using /dev/random as random source");
            }
            else {
                LOG(LOG_ERR, "random source failed to provide random data : couldn't open device");
                throw Error(ERR_RANDOM_SOURCE_FAILED);
            }
        }
    }

    void random(writable_bytes_view buf) override
    {
        // TODO This is basically a blocking read, we should provide timeout management and behaviour
        uint8_t* data = buf.data();
        size_t len = buf.size();
        while (len) {
            ssize_t ret = ::read(ufd.fd(), data, len);
            if (REDEMPTION_UNLIKELY(ret <= 0)) {
                if (errno == EINTR) {
                    continue;
                }
                LOG(LOG_ERR, "random source failed to provide random data [%s]", strerror(errno));
                throw Error(ERR_RANDOM_SOURCE_FAILED);
            }
            data += ret;
            len -= ret;
        }
    }
};
