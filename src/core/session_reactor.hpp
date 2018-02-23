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

//     template<class... Ts>
//     using TopExecutor = jln::TopExecutor2<PrefixArgs, Ts...>;

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
            auto it = this->get_elem_iterator(elem);
            if (it != this->elements.end()) {
                *it = std::move(this->elements.back());
                this->elements.pop_back();
            }
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
                            assert(false && "Exit");
                        case jln::ExecutorResult::Terminate:
                            this->elements[i] = this->elements.back();
                            this->elements.pop_back();
                            elem->delete_self(jln::DeleteFrom::Observer);
                            break;
                        case jln::ExecutorResult::Nothing:
                            break;
                        case jln::ExecutorResult::NeedMoreData:
                            assert(false && "NeedMoreData");
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
    using BasicTimerPtr = jln::UniquePtrWithNotifyDelete<BasicTimer>;

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

        void info(timeval const& end_tv) {
            for (auto& timer : this->elements) {
                auto const tv = timer->time();
                LOG(LOG_DEBUG, "%p: %ld %ld %ld",
                    static_cast<void*>(timer), tv.tv_sec, tv.tv_usec, difftimeval(tv, end_tv).count());
            }
        }

        template<class... Args>
        using Elem = jln::Timer<TimerContainer&, BasicTimer::prefix_args, Args...>;
    };


    template<class Builder>
    struct NotifyDeleterBuilderWrapper : Builder
    {
        using Builder::Builder;

        template<class NotifyDeleter>
        Builder&& set_notify_delete(NotifyDeleter d) && noexcept
        {
            this->internal_value().set_notify_delete(d);
            return static_cast<Builder&&>(*this);
        }
    };


    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TimerBuilder<jln::UniquePtrEventWithUPtr<TimerContainer::Elem<Args...>>>>
    create_timer(Args&&... args)
    {
        using Timer = TimerContainer::Elem<Args...>;
        return {jln::UniquePtrEventWithUPtr<Timer>
            ::New(this->timer_events_, static_cast<Args&&>(args)...)};
    }

    using CallbackEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using CallbackEventPtr = jln::UniquePtrWithNotifyDelete<CallbackEvent>;

    struct CallbackContainer : Container<CallbackEvent>
    {
        template<class... Args>
        using Elem = jln::Action<CallbackContainer&, CallbackEvent::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::ActionBuilder<jln::UniquePtrEventWithUPtr<CallbackContainer::Elem<Args...>>>>
    create_callback_event(Args&&... args)
    {
        using Action = CallbackContainer::Elem<Args...>;
        return {jln::UniquePtrEventWithUPtr<Action>
            ::New(this->front_events_, static_cast<Args&&>(args)...)};
    }


    using GraphicEvent = jln::ActionBase<jln::prefix_args<gdi::GraphicApi&>>;
    using GraphicEventPtr = jln::UniquePtrWithNotifyDelete<GraphicEvent>;

    struct GraphicContainer : Container<GraphicEvent>
    {
        template<class... Args>
        using Elem = jln::Action<GraphicContainer&, GraphicEvent::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::ActionBuilder<jln::UniquePtrEventWithUPtr<GraphicContainer::Elem<Args...>>>>
    create_graphic_event(Args&&... args)
    {
        using Action = GraphicContainer::Elem<Args...>;
        return {jln::UniquePtrEventWithUPtr<Action>
            ::New(this->graphic_events_, static_cast<Args&&>(args)...)};
    }

    using BasicFd = jln::BasicExecutorImpl<PrefixArgs>;

    struct TopFd : BasicTimer, BasicFd
    {
        using prefix_args = typename BasicFd::prefix_args;
        using base_type = TopFd;

        static_assert(std::is_same<
            typename BasicFd::prefix_args,
            typename BasicTimer::prefix_args
        >::value);

        using BasicFd::delete_self;
        using BasicFd::deleter;
        using BasicFd::on_action;

        BasicTimer& timer() noexcept { return *this; }

        void set_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->timeout = ms;
            this->set_time(ms);
        }

        void update_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->timeout = ms;
            this->set_time(ms);
        }

        void restart_timeout()
        {
            this->set_time(this->timeout);
        }

        int fd;
        std::chrono::milliseconds timeout;
    };

    template<class PrefixArgs_, class... Ts>
    struct FdImpl : TopFd
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Args>
        FdImpl(int fd, SessionReactor& session_reactor, Args&&... args)
        : ctx{static_cast<Args&&>(args)...}
        , session_reactor(session_reactor)
        {
            this->fd = fd;
            this->session_reactor.timer_events_.attach(this->timer());
        }
        REDEMPTION_DIAGNOSTIC_POP

        ~FdImpl()
        {
            this->session_reactor.timer_events_.detach(this->timer());
            this->detach();
        }

        void detach() noexcept
        {
            this->session_reactor.fd_events_.detach(*this);
        }

        void attach() noexcept
        {
            this->session_reactor.fd_events_.attach(*this);
        }

        template<class F>
        void set_on_action(F) noexcept
        {
            this->on_action = wrap_fn<F, jln::Executor2FdContext>();
        }

        template<class F>
        void set_on_exit(F) noexcept
        {
            this->on_exit = wrap_fn<F, jln::Executor2FdContext>();
        }

        template<class F>
        void set_on_timeout(F) noexcept
        {
            this->timer().on_action = wrap_fn<F, jln::Executor2FdTimeoutContext>();
        }

    private:
        template<class F, template<class...> class Ctx>
        static auto wrap_fn() noexcept
        {
            return [](auto& e, auto... prefix_args){
                static_cast<FdImpl&>(e).restart_timeout();
                return jln::wrap_fn<F, FdImpl, Ctx>()(
                    e, static_cast<decltype(prefix_args)&&>(prefix_args)...);
            };
        }

    public:
        jln::detail::tuple<Ts...> ctx;
        SessionReactor& session_reactor;

        void *operator new(size_t n, char const*) { return ::operator new(n); }
    };

    template<class PrefixArgs_, class... Args>
    using Fd = FdImpl<PrefixArgs_, typename jln::detail::decay_and_strip<Args>::type...>;

    using TopFdPtr = jln::UniquePtrWithNotifyDelete<TopFd>;

    struct TopFdContainer : Container<TopFd>
    {
        template<class... Args>
        using Elem = Fd<TopFd::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TopFdBuilder<jln::UniquePtrEventWithUPtr<TopFdContainer::Elem<Args...>>>>
    create_fd_event(int fd, Args&&... args)
    {
        using EventFd = TopFdContainer::Elem<Args...>;
        return {jln::UniquePtrEventWithUPtr<EventFd>
            ::New(fd, *this, static_cast<Args&&>(args)...)};
    }


    //std::vector<std::unique_ptr<Context>> contexts;
    CallbackContainer front_events_;
    GraphicContainer graphic_events_;
    TimerContainer timer_events_;
    TopFdContainer fd_events_;

    std::vector<CallbackEvent*> front_events()
    {
        return this->front_events_.elements;
    }

    std::vector<GraphicEvent*> graphic_events()
    {
        return this->graphic_events_.elements;
    }

    void set_event_next(/*BackEvent_t*/int)
    {
        // assert(is not already set)
        // TODO unimplemented
    }
};
