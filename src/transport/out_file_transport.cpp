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

#include "transport/out_file_transport.hpp"
#include "utils/log.hpp"

void LogReporter::operator()(char const * reason, char const * message)
{
    LOG(LOG_ERR, "%s:%s", reason, message);
}

Error ReportError::NullImpl::get_error(Error err)
{
    report_and_transform_error(err, LogReporter{});
    return err;
}

ReportError report_error_from_reporter(ReportMessageApi & reporter)
{
    return ReportError([&reporter](Error error) {
        report_and_transform_error(error, ReportMessageReporter{reporter});
        return error;
    });
}

ReportError report_error_from_reporter(ReportMessageApi * reporter)
{
    return reporter ? report_error_from_reporter(*reporter) : ReportError();
}


void OutFileTransport::seek(int64_t offset, int whence)
{
    if (lseek64(this->file.fd(), offset, whence) == static_cast<off_t>(-1)) {
        throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
    }
}

void OutFileTransport::do_send(const uint8_t * data, size_t len)
{
    size_t total_sent = 0;
    while (len > total_sent) {
        ssize_t const ret = ::write(this->file.fd(), data + total_sent, len - total_sent);
        if (ret <= 0){
            if (errno == EINTR) {
                continue;
            }
            throw this->report_error(Error(ERR_TRANSPORT_WRITE_FAILED, errno));
        }
        total_sent += ret;
    }
}
