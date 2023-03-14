/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/socket.hpp"

#include <sys/ioctl.h>


size_t qtclient::SocketIO::do_partial_read(uint8_t * buffer, size_t len)
{
    size_t res = SocketTransport::do_partial_read(buffer, len);
    if (res != len && res != 0) {
        int queued;
        int error = ioctl(get_fd(), FIONREAD, &queued);
        if (error == 0 && queued > 0) {
            len -= res;
            buffer += res;
            len = std::min(len, static_cast<size_t>(queued));
            for (;;) {
                size_t res2 = SocketTransport::do_partial_read(buffer, len);
                if (res2 == 0) {
                    break;
                }
                len -= res2;
                res += res2;
                if (len == 0) {
                    break;
                }
                buffer += res2;
            }
        }
    }

    return res;
}
