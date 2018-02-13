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
class Callback;
namespace gdi
{
    class GraphicApi;
};

struct SessionReactor
{
    using PrefixArgs = jln::prefix_args<>;

    using BasicExecutor = jln::BasicExecutorImpl<PrefixArgs>;

    template<class... Ts>
    using TopExecutor = jln::TopExecutor2<PrefixArgs, Ts...>;

    enum class EventType : int8_t
    {
        Timeout,
        Callback,
        Mod,
    };

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

//     Context& create_socket(
//         const char * name, unique_fd sck, const char* ip_address, int port,
//         std::chrono::milliseconds recv_timeout,
//         SocketTransport::Verbose verbose, std::string* error_message)
//     {
//         return *this->contexts.emplace_back(std::unique_ptr<Context>(new Context{
//             *this,
//             {name, std::move(sck), ip_address, port, recv_timeout, verbose, error_message},
//             {},
//             {}
//         }));
//     }

    using BasicTimerPtr = jln::UniquePtr<jln::BasicTimer<PrefixArgs>>;

    struct Container
    {
        template<class T> Container(T&) {}
        template<class T> void detach(T&) {}
        template<class T> void attach(T&) {}
    };

    template<class... Args>
    jln::TimerBuilder<PrefixArgs, Args...>
    create_timer(Args&&... args)
    {
        using Timer = jln::Timer2<PrefixArgs, Args...>;
        return {jln::new_event<Timer>(this->timers, static_cast<Args&&>(args)...)};
    }

    using CallbackEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using CallbackEventPtr = jln::UniquePtr<CallbackEvent>;

    template<class... Args>
    auto create_callback_event(Args&&... args)
    {
        using Event = jln::ActionImpl<Container, jln::prefix_args<Callback&>, Args...>;
        return jln::new_event<Event>(this->front_events, static_cast<Args&&>(args)...);
    }

    using GraphicEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using GraphicEventPtr = jln::UniquePtr<GraphicEvent>;

    template<class... Args>
    auto create_graphic_event(Args&&... args)
    {
        using Event = jln::ActionImpl<Container, jln::prefix_args<gdi::GraphicApi&>, Args...>;
        return jln::new_event<Event>(this->graphic_events, static_cast<Args&&>(args)...);
    }

    std::vector<std::unique_ptr<Context>> contexts;
    std::vector<CallbackEvent*> front_events;
    std::vector<GraphicEvent*> graphic_events;
    jln::TopExecutorTimersImpl<PrefixArgs> timers;
};
