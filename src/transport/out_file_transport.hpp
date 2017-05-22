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
#include "core/report_message_api.hpp"
#include "transport/transport.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/make_unique.hpp"

#include <memory>

#include <cerrno>


class ReportError
{
public:
    template<class F>
    explicit ReportError(F && f)
    : impl(new FuncImpl<typename std::decay<F>::type>{std::forward<F>(f)})
    {}

    explicit ReportError(std::nullptr_t = nullptr)
    : impl(new NullImpl)
    {}

    ReportError(ReportError && other)
    : impl(std::move(other.impl))
    {}

    ReportError(ReportError const & other)
    : impl(other.impl->clone())
    {}

    ReportError(ReportError & other)
    : impl(other.impl->clone())
    {}

    ReportError & operator = (ReportError &&) = delete;
    ReportError & operator = (ReportError const &) = delete;

    Error get_error(Error err)
    {
        return this->impl->get_error(err);
    }

    Error operator()(Error err)
    {
        return this->get_error(err);
    }

private:
    struct ImplBase
    {
        virtual Error get_error(Error err) = 0;
        virtual ImplBase* clone() const = 0;
        virtual ~ImplBase() = default;
    };

    struct NullImpl : ImplBase
    {
        Error get_error(Error err) override;
        ImplBase* clone() const override { return new NullImpl; }
    };

    template<class F>
    struct FuncImpl : ImplBase
    {
        F fun;
        template<class Fu>
        FuncImpl(Fu && f) : fun(std::forward<Fu>(f)) {}
        Error get_error(Error err) override { return fun(err); }
        ImplBase* clone() const override { return new FuncImpl(fun); }
    };

    std::unique_ptr<ImplBase> impl;
};

template<class F>
void report_and_transform_error(Error& error, F && report)
{
    if (error.errnum == ENOSPC) {
        error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
        report("FILESYSTEM_FULL", "100|unknow");
    }
}

struct LogReporter
{
    void operator()(char const * reason, char const * message)
    {
        LOG(LOG_ERR, "%s:%s", reason, message);
    }
};

struct ReportMessageReporter
{
    ReportMessageReporter(ReportMessageApi & reporter)
    : reporter(reporter)
    {}

    void operator()(char const * reason, char const * message)
    {
        reporter.report(reason, message);
    }

private:
    ReportMessageApi & reporter;
};

inline Error ReportError::NullImpl::get_error(Error error)
{
    report_and_transform_error(error, LogReporter{});
    return error;
}

inline ReportError report_error_from_reporter(ReportMessageApi & reporter)
{
    return ReportError([&reporter](Error error) {
        report_and_transform_error(error, ReportMessageReporter{reporter});
        return error;
    });
}

inline ReportError report_error_from_reporter(ReportMessageApi * reporter)
{
    return reporter ? report_error_from_reporter(*reporter) : ReportError();
}

struct OutFileTransport : Transport
{
    explicit OutFileTransport(unique_fd fd, ReportError report_error = ReportError()) noexcept
    : file(std::move(fd))
    , report_error(std::move(report_error))
    {}

    bool disconnect() override
    {
        return this->file.close();
    }

    void seek(int64_t offset, int whence) override
    {
        if (lseek64(this->file.fd(), offset, whence) == static_cast<off_t>(-1)) {
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

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

private:
    void do_send(const uint8_t * data, size_t len) override
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

    unique_fd file;
    ReportError report_error;
};
