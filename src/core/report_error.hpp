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

#include "core/error.hpp"
#include "core/report_message_api.hpp"

#include <memory>
#include <utility>
#include <type_traits>

#include <cerrno>


class ReportError
{
public:
    template<class F>
    explicit ReportError(F && f) /*NOLINT*/
    : impl(new FuncImpl<typename std::decay<F>::type>{std::forward<F>(f)})
    {}

    explicit ReportError(std::nullptr_t = nullptr) /*NOLINT*/
    : impl(new NullImpl)
    {}

    ReportError(ReportError && other) noexcept
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
    struct ImplBase;
    using ImplPtr = std::unique_ptr<ImplBase>;

    struct ImplBase
    {
        virtual Error get_error(Error err) = 0;
        [[nodiscard]] virtual ImplPtr clone() const = 0;
        virtual ~ImplBase() = default;
    };

    struct NullImpl : ImplBase
    {
        Error get_error(Error err) override;
        [[nodiscard]] ImplPtr clone() const override { return ImplPtr(new NullImpl); }
    };

    template<class F>
    struct FuncImpl : ImplBase
    {
        F fun;
        template<class Fu>
        explicit FuncImpl(Fu && f) /*NOLINT*/ : fun(std::forward<Fu>(f)) {}
        Error get_error(Error err) override { return fun(err); }
        [[nodiscard]] ImplPtr clone() const override { return ImplPtr(new FuncImpl(fun)); }
    };

    ImplPtr impl;
};

template<class F>
void report_and_transform_error(Error& error, F && report)
{
    if (error.errnum == ENOSPC) {
        error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
        report("FILESYSTEM_FULL", "100|unknown");
    }
}

struct LogReporter
{
    void operator()(char const * reason, char const * message);
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

ReportError report_error_from_reporter(ReportMessageApi & reporter);
ReportError report_error_from_reporter(ReportMessageApi * reporter);
