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

#include <functional>
#include "transport/transport.hpp"
#include "utils/sugar/unique_fd.hpp"


struct FileTransport : Transport
{
    explicit FileTransport(unique_fd fd, std::function<void(const Error & error)> notify_error = [](const Error&){}) noexcept /*NOLINT*/
    : file(std::move(fd))
    , _notify_error(std::move(notify_error))
    {}

    bool disconnect() override
    {
        return this->file.close();
    }

    void seek(int64_t offset, int whence) override;

    [[nodiscard]] int get_fd() const override
    {
        return this->file.fd();
    }

    void open(unique_fd fd)
    {
        this->file = std::move(fd);
    }

    [[nodiscard]] bool is_open() const
    {
        return this->file.is_open();
    }

    // alias on disconnect
    void close()
    {
        this->file.close();
    }

protected:
    void do_send(const uint8_t * data, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;
    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    void notify_error(const Error & error);

private:
    unique_fd file;
    std::function<void(const Error & error)> _notify_error;
};


struct InFileTransport : FileTransport
{
    using FileTransport::FileTransport;

private:
    void do_send(const uint8_t * buffer, size_t len) override
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }
};


struct OutFileTransport : FileTransport
{
    using FileTransport::FileTransport;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }

    size_t do_partial_read(uint8_t * buffer, size_t len) override
    {
        (void)buffer;
        (void)len;
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV);
    }
};
