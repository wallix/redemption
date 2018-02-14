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

//     struct Context
//     {
//         SessionReactor& session_reactor;
//         SocketTransport socket_transport;
//         void* first_data;
//         jln::UniquePtr<BasicExecutor> executor;
//
//         template<class... Args>
//         jln::TopExecutorBuilder<PrefixArgs, Args...>
//         init_executor(Args&&... args)
//         {
//             auto* p = TopExecutor<Args...>::New(
//                 this->session_reactor.timers, static_cast<Args&&>(args)...);
//             this->first_data = &jln::detail::get<0>(p->ctx);
//             this->executor.reset(p);
//             return {*p};
//         }
//     };

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

    template<class Base>
    struct Container
    {
        void attach(Base& elem)
        {
            assert(this->elements.end() == this->get_elem_iterator(elem));
            this->elements.emplace_back(&elem);
        }

        void detach(Base& elem)
        {
            this->elements.erase(this->get_elem_iterator(elem), this->elements.end());
        }

//     private:
        auto get_elem_iterator(Base& elem)
        {
            return std::find(this->elements.begin(), this->elements.end(), &elem);
        }

        template<class... Args>
        void exec(Args&&... args)
        {
            this->exec_impl(
                [](auto&){ return std::true_type{}; },
                static_cast<Args&&>(args)...
            );
        }

        template<class Predicate, class... Args>
        void exec_impl(Predicate pred, Args&&... args)
        {
            for (std::size_t i = 0; i < this->elements.size(); ) {
                auto* elem = this->elements[i];
                if (pred(*elem)) {
                    switch (elem->exec_action(static_cast<Args&&>(args)...)) {
                        case jln::ExecutorResult::ExitSuccess:
                        case jln::ExecutorResult::ExitFailure:
                            assert(false);
                        case jln::ExecutorResult::Terminate:
                            this->elements.erase(this->elements.begin() + i);
                            break;
                        case jln::ExecutorResult::Nothing:
                        case jln::ExecutorResult::NeedMoreData:
                            assert(false);
                        case jln::ExecutorResult::Ready:
                            ++i;
                            break;
                    }
                }
                else {
                    ++i;
                }
            }
        }

        std::vector<Base*> elements;
    };

    using BasicTimer = jln::BasicTimer<jln::prefix_args<>>;
    using BasicTimerPtr = jln::UniquePtr<BasicTimer>;

    struct TimerContainer : Container<BasicTimer>
    {
        void update_time(BasicTimer& timer, std::chrono::milliseconds ms)
        {
            assert(this->elements.end() != this->get_elem_iterator(timer));
            (void)timer;
            (void)ms;
        }

        timeval get_next_timeout() const noexcept
        {
            auto it = std::min_element(
                this->elements.begin(), this->elements.end(),
                [](auto& a, auto& b) { return a->time() < b->time(); });
            return it == this->elements.end() ? timeval{-1, -1} : (*it)->time();
        }

        template<class... Args>
        void exec(timeval const& end_tv, Args&&... args)
        {
            this->exec_impl(
                [&](BasicTimer const& timer){ return timer.time() <= end_tv; },
                static_cast<Args&&>(args)...
            );
        }

        template<class... Args>
        using Timer = jln::Timer<TimerContainer&, jln::prefix_args<>, Args...>;
    };

    template<class T>
    using OwnerPtr = std::unique_ptr<T, jln::DeleteSelf<typename T::base_type>>;

    template<class... Args>
    jln::TimerBuilder<OwnerPtr<TimerContainer::Timer<Args...>>>
    create_timer(Args&&... args)
    {
        using Timer = TimerContainer::Timer<Args...>;
        return {jln::new_event<Timer>(this->timers_events_, static_cast<Args&&>(args)...)};
    }

    using CallbackEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using CallbackEventPtr = jln::UniquePtr<CallbackEvent>;

    template<class... Args>
    auto create_callback_event(Args&&... args)
    {
        using Event = jln::ActionImpl<decltype((this->front_events_)), jln::prefix_args<Callback&>, Args...>;
        return jln::new_event<Event>(this->front_events_, static_cast<Args&&>(args)...);
    }

    using GraphicEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using GraphicEventPtr = jln::UniquePtr<GraphicEvent>;

    template<class... Args>
    auto create_graphic_event(Args&&... args)
    {
        using Event = jln::ActionImpl<decltype((this->graphic_events_)), jln::prefix_args<gdi::GraphicApi&>, Args...>;
        return jln::new_event<Event>(this->graphic_events_, static_cast<Args&&>(args)...);
    }

    //std::vector<std::unique_ptr<Context>> contexts;
    Container<CallbackEvent> front_events_;
    Container<GraphicEvent> graphic_events_;
    TimerContainer timers_events_;

    std::vector<CallbackEvent*> front_events()
    {
        return this->front_events_.elements;
    }

    std::vector<GraphicEvent*> graphic_events()
    {
        return this->graphic_events_.elements;
    }
};
