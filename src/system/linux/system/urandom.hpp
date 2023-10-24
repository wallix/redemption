/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/random.hpp"

#include <sys/random.h>


class URandom final : public Random
{
public:
    void random(writable_bytes_view buf) override
    {
        uint8_t* data = buf.data();
        size_t len = buf.size();
        while (len) {
#if defined(__sun) && defined(__SVR4)
            /* On Solaris, getrandom() is limited to returning up to 1024 bytes. */
            size_t len_max = 1024;
#else
            size_t len_max = LONG_MAX;
#endif
            // TODO This is basically a blocking read, we should provide timeout management and behaviour
            ssize_t n = getrandom(data, len < len_max ? len : len_max, 0);
            if (REDEMPTION_UNLIKELY(n < 0)) {
                if (errno == EINTR) {
                    continue;
                }
                // ignore errno == EAGAIN because GRND_NONBLOCK flag is not used
                LOG(LOG_ERR, "random source failed to provide random data [%s]", strerror(errno));
                throw Error(ERR_RANDOM_SOURCE_FAILED);
            }
            data += n;
            len -= n;
        }
    }
};
