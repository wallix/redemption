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

#include "utils/fdbuf.hpp"
#include "transport/transport.hpp"
#include "utils/sugar/local_fd.hpp"
#include "utils/sugar/make_unique.hpp"

#include <memory>

class TransportError : noncopyable_but_movable
{
public:
    template<class F>
    static TransportError mk(F && f)
    {
        return TransportError(new FuncImpl<typename std::decay<F>::type>{std::forward<F>(f)});
    }

    static TransportError mk(std::nullptr_t = nullptr)
    {
        // disable allocation/deallocation
        struct NullImpl : ImplBase
        {
            void operator delete(void *) {}
            Error get_error(Error err) override { return err; }
        };
        static NullImpl null_impl;
        return TransportError(&null_impl);
    }

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
        virtual ~ImplBase() = default;
    };

    template<class F>
    struct FuncImpl : ImplBase
    {
        F fun;
        template<class Fu>
        FuncImpl(Fu && f) : fun(std::forward<Fu>(f)) {}
        Error get_error(Error err) override { return fun(err); }
    };

    template<class F, class Fu>
    static TransportError dispath_mk(Fu && f, std::false_type = typename std::is_pointer<F>::type{})
    {
        return TransportError{{new FuncImpl<F>{std::forward<Fu>(f)}}};
    }

    template<class F, class Fu>
    static TransportError dispath_mk(Fu && f, std::true_type = typename std::is_pointer<F>::type{})
    {
        return f ? TransportError(new FuncImpl<F>{f}) : mk(nullptr);
    }

    TransportError(ImplBase* p)
    : impl(p)
    {}

    std::unique_ptr<ImplBase> impl;
};

inline TransportError auth_report_error(auth_api& auth)
{
    return TransportError::mk([&auth](Error error) {
        if (error.id == ENOSPC) {
            auth.report("FILESYSTEM_FULL", "100|unknow");
            error.id = ERR_TRANSPORT_WRITE_NO_ROOM;
        }
        return error;
    });
}

inline TransportError auth_report_error(auth_api* auth)
{
    return auth ? auth_report_error(*auth) : TransportError::mk();
}

struct OutFileTransport : Transport
{
    explicit OutFileTransport(local_fd fd, TransportError report_error = TransportError::mk()) noexcept
    : file(fd.release())
    , report(std::move(report_error))
    {}

    bool disconnect() override {
        return !this->file.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override
    {
        const ssize_t res = this->file.write(data, len);
        if (res < 0) {
            this->status = false;
            throw this->report(Error(ERR_TRANSPORT_WRITE_FAILED, errno));
        }
        this->last_quantum_sent += res;
    }

    io::posix::fdbuf file;
    TransportError report;
};
