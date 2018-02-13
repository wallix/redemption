/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2017
Author(s): Jonathan Poelen
*/

#pragma once

#include <memory>
#include <chrono>
#include <string>

#include "transport/socket_transport.hpp"
#include "utils/executor.hpp"
#include "utils/sugar/unique_fd.hpp"

class mod_api;

struct SessionReactor
{
    using PrefixArgs = jln::prefix_args<>;

    using BasicExecutor = jln::BasicExecutorImpl<PrefixArgs>;

    template<class... Ts>
    using TopExecutor = jln::TopExecutor2<PrefixArgs, Ts...>;

    struct Context
    {
        SessionReactor& session_reactor;
        SocketTransport socket_transport;
        void* first_data;
        jln::UniquePtr<BasicExecutor> executor;

        template<class... Args>
        jln::TopExecutorBuilder<PrefixArgs, Args...>
        init_executor(Args&&... args)
        {
            auto* p = TopExecutor<Args...>::New(
                this->session_reactor.timers, static_cast<Args&&>(args)...);
            this->first_data = &jln::detail::get<0>(p->ctx);
            this->executor.reset(p);
            return {*p};
        }
    };

    Context& create_socket(
        const char * name, unique_fd sck, const char* ip_address, int port,
        std::chrono::milliseconds recv_timeout,
        SocketTransport::Verbose verbose, std::string* error_message)
    {
        return *this->contexts.emplace_back(std::unique_ptr<Context>(new Context{
            *this,
            {name, std::move(sck), ip_address, port, recv_timeout, verbose, error_message},
            {},
            {}
        }));
    }

    std::vector<std::unique_ptr<Context>> contexts;
    jln::TopExecutorTimersImpl<PrefixArgs> timers;
};
