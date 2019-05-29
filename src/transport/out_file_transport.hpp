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

#include "core/report_error.hpp"
#include "transport/transport.hpp"
#include "utils/sugar/unique_fd.hpp"


struct OutFileTransport : Transport
{
    explicit OutFileTransport(unique_fd fd, ReportError report_error = ReportError()) noexcept /*NOLINT*/
    : file(std::move(fd))
    , report_error(std::move(report_error))
    {}

    bool disconnect() override
    {
        return this->file.close();
    }

    void seek(int64_t offset, int whence) override;

    int get_fd() const override
    {
        return this->file.fd();
    }

    ReportError & get_report_error()
    {
        return this->report_error;
    }

    void open(unique_fd fd)
    {
        this->file = std::move(fd);
    }

    bool is_open() const
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

private:
    unique_fd file;
    ReportError report_error;
};
