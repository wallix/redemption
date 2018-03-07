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
#include <new>

#include "transport/socket_transport.hpp"
#include "utils/executor.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/log.hpp"

class mod_api;
class Callback;
class Inifile;
namespace gdi
{
    class GraphicApi;
}

struct SessionReactor
{
    using PrefixArgs = jln::prefix_args<>;

    using BasicExecutor = jln::BasicExecutorImpl<PrefixArgs>;

    enum class EventType : int8_t
    {
        Timeout,
        Callback,
        Mod,
    };

    enum class EventState : bool
    {
        Alive,
        Dead,
    };

    /*
     * Layout memory
     *
     * +-----------------------+------------+
     * |    SharedDataBase     |  use_count | <._
     * |                       | shared_ptr | <-- internal
     * |                       |   deleter  | <-*
     * +-----------------------+-+--------+-|
     * |    SharedData<Base>   | |  base  | | <-- public
     * +-----------------------+-|        |-+
     * |     SharedData<T>     | |  value | | <-- private
     * +-----------------------+-+--------+-+
     */

    struct SharedDataBase
    {
        int use_count;
        void* shared_ptr = nullptr;
        void (*deleter) (SharedDataBase*) noexcept;
        void apply_deleter() noexcept
        {
            this->deleter(this);
        }
    };

    template<class T>
    struct SharedData : SharedDataBase
    {
        T value;
    };

    template<class T>
    struct SharedPtrBase
    {
        SharedPtrBase() = default;
        SharedPtrBase(SharedPtrBase const&) = delete;
        SharedPtrBase& operator=(SharedPtrBase const&) = delete;

        template<class U>
        SharedPtrBase(SharedPtrBase<U>&& ptr) noexcept
          : data(reinterpret_cast<Data*>(std::exchange(ptr.data, nullptr)))
        {
            static_assert(std::is_base_of<T, U>::value);
        }

        template<class U>
        SharedPtrBase& operator=(SharedPtrBase<U>&& other) noexcept
        {
            static_assert(std::is_base_of<T, U>::value);
            this->data = reinterpret_cast<Data*>(std::exchange(other.data, nullptr));
            return *this;
        }

        explicit operator bool () const noexcept
        {
            return bool(this->data);
        }

        T* get() const noexcept
        {
            assert(this->data);
            return &this->data->value;
        }

        T* operator->() const noexcept
        {
            assert(this->data);
            return &this->data->value;
        }

        T& operator*() const noexcept
        {
            assert(this->data);
            return this->data->value;
        }

        void set_deleter(decltype(SharedDataBase::deleter) f) noexcept
        {
            assert(this->data);
            this->data->deleter = f;
        }

        void add_use() noexcept
        {
            assert(this->data);
            ++this->data->use_count;
        }

        using Data = SharedData<T>;

        SharedPtrBase(Data* ptr) noexcept
          : data(ptr)
        {}

        Data* data = nullptr;
    };

    struct SharedPtrPrivateAccess
    {
        template<class T>
        static auto& p(T& p_) noexcept
        {
            return p_.p;
        }
    };

    template<class T>
    class SharedPtrPrivate
    {
        using Data = typename SharedPtrBase<T>::Data;

        SharedPtrBase<T> p;

        SharedPtrPrivate(Data* p) noexcept
          : p(p)
        {}

        friend SharedPtrPrivateAccess;

    public:
        SharedPtrPrivate(SharedPtrPrivate&&) = default;
        SharedPtrPrivate& operator=(SharedPtrPrivate&&) = default;

#ifndef NDEBUG
        ~SharedPtrPrivate()
        {
            assert(!this->p);
        }
#endif

        explicit operator bool () const noexcept
        {
            return bool(this->p);
        }

        T* get() const noexcept
        {
            return this->p->get();
        }

        T* operator->() const noexcept
        {
            return this->p.operator->();
        }

        T& operator*() const noexcept
        {
            return *this->p;
        }

        template<class F>
        void set_notify_delete(F f) noexcept
        {
            this->p.set_deleter(make_deleter(f));
        }

        template<class F>
        static auto make_deleter(F = nullptr) noexcept
        {
            return [](SharedDataBase* base) noexcept -> void {
                LOG(LOG_DEBUG, "dealloc %p %s", static_cast<void*>(base), typeid(T).name());
                # ifndef NDEBUG
                base->deleter = [](SharedDataBase* p) noexcept {
                    LOG(LOG_DEBUG, "dealloc %p %s already delete", static_cast<void*>(p), typeid(T).name());
                    assert(!"already delete");
                };
                # endif

                auto* data = static_cast<Data*>(base);
                assert(static_cast<void*>(data) == static_cast<void*>(base));
                static_cast<SharedPtrBase<T>*>(data->shared_ptr)->data = nullptr;
                data->shared_ptr = nullptr;
                --data->use_count;
                if constexpr (!std::is_same<F, std::nullptr_t>::value) {
                    data->value.ctx.invoke(jln::make_lambda<F>());
                }
                data->value.~T();
            };
        }

        template<int Use = 2, class C, class... Args>
        static SharedPtrPrivate New(C& c, Args&&... args)
        {
            Data* data = static_cast<Data*>(::operator new(sizeof(Data)));
            LOG(LOG_DEBUG, "new %p %s", static_cast<void*>(data), typeid(T).name());
            data->deleter = SharedPtrPrivate::make_deleter(nullptr);
            if constexpr (noexcept(T(c, static_cast<Args&&>(args)...))) {
                new(&data->value) T(c, static_cast<Args&&>(args)...);
            }
            else {
                bool failed = true;
                SCOPE_EXIT(if (failed) {
                    ::operator delete(data);
                });
                new(&data->value) T(c, static_cast<Args&&>(args)...);
                failed = false;
            }
            c.attach(data);
            data->use_count = Use;
            return SharedPtrPrivate(data);
        }
    };

    template<class T>
    class SharedPtr
    {
        SharedPtrBase<T> p;

    public:
        using value_type = T;

        SharedPtr() = default;
        SharedPtr(SharedPtr&&) = default;
        SharedPtr& operator=(SharedPtr&&) = default;

        template<class U>
        SharedPtr(SharedPtrPrivate<U>&& other) noexcept
          : p(std::move(SharedPtrPrivateAccess::p(other)))
        {
            this->p.data->shared_ptr = &this->p;
        }

        template<class U>
        SharedPtr& operator=(SharedPtrPrivate<U>&& other) noexcept
        {
            assert(static_cast<SharedDataBase*>(SharedPtrPrivateAccess::p(other).data) != static_cast<SharedDataBase*>(this->p.data));
            this->reset();
            this->p = std::move(SharedPtrPrivateAccess::p(other));
            this->p.data->shared_ptr = &this->p;
            return *this;
        }

        ~SharedPtr()
        {
            this->reset();
        }

        explicit operator bool () const noexcept
        {
            return bool(this->p);
        }

        T* operator->() const noexcept
        {
            return this->p.operator->();
        }

        T& operator*() const noexcept
        {
            return *this->p;
        }

        void reset() noexcept
        {
            if (this->p) {
                this->p.data->apply_deleter();
            }
        }
    };


    template<class Base>
    struct Container
    {
        using Data = SharedData<Base>;

        static Data* to_shared_data(Base& elem)
        {
            return reinterpret_cast<Data*>(
                reinterpret_cast<uint8_t*>(&elem) - sizeof(SharedDataBase));
        }

        void attach(Base& elem)
        {
            auto* data = to_shared_data(elem);
            assert(this->elements.end() == this->get_elem_iterator(data));
            this->elements.emplace_back(data);
        }

        template<class U>
        void attach(SharedData<U>* data_)
        {
            static_assert(std::is_base_of<Base, U>::value);
            auto* data = reinterpret_cast<Data*>(data_);
            assert(this->elements.end() == this->get_elem_iterator(data));
            this->elements.emplace_back(data);
        }

//     private:
        auto get_elem_iterator(Data* elem)
        {
            return std::find(this->elements.begin(), this->elements.end(), elem);
        }

        auto get_elem_iterator(Base& elem)
        {
            return get_elem_iterator(to_shared_data(elem));
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
            for (std::size_t i = 0; i < this->elements.size(); ++i) {
                auto* elem = this->elements[i];
                if (elem->use_count > 1 && pred(elem->value)) {
                    switch (elem->value.exec_action(static_cast<Args&&>(args)...)) {
                        case jln::ExecutorResult::ExitSuccess:
                        case jln::ExecutorResult::ExitFailure:
                            assert(false && "Exit");
                        case jln::ExecutorResult::Terminate:
                            assert(elem->use_count > 1);
                            LOG(LOG_DEBUG, "f = %p %d", static_cast<void*>(elem), elem->use_count);
                            elem->apply_deleter();
                            break;
                        case jln::ExecutorResult::NeedMoreData:
                            assert(false && "NeedMoreData");
                        case jln::ExecutorResult::Nothing:
                        case jln::ExecutorResult::Ready:
                            break;
                    }
                }
            }
        }

        std::vector<Data*> elements;
    };

    template<class Timer>
    struct BasicTimerContainer : Container<Timer>
    {
        void update_delay(Timer& timer, std::chrono::milliseconds ms)
        {
            assert(this->elements.end() != this->get_elem_iterator(timer));
            (void)timer;
            (void)ms;
        }

        void update_time(Timer& timer, timeval const& tv)
        {
            assert(this->elements.end() != this->get_elem_iterator(timer));
            (void)timer;
            (void)tv;
        }

        timeval get_next_timeout() const noexcept
        {
            auto it = std::min_element(
                this->elements.begin(), this->elements.end(),
                [](auto& a, auto& b) { return a->value.time() < b->value.time(); });
            return it == this->elements.end() ? timeval{-1, -1} : (*it)->value.time();
        }

        template<class... Args>
        void exec(timeval const& end_tv, Args&&... args)
        {
            this->exec_impl(
                [&](Timer const& timer){ return timer.time() <= end_tv; },
                static_cast<Args&&>(args)...
            );
        }

        void info(timeval const& end_tv) {
            for (auto& timer : this->elements) {
                auto const tv = timer->value.time();
                LOG(LOG_DEBUG, "%p: %ld %ld %ld, delay=%ld",
                    static_cast<void*>(timer), tv.tv_sec, tv.tv_usec, difftimeval(tv, end_tv).count(), timer->delay.count());
            }
        }

        template<class... Args>
        using Elem = jln::Timer<BasicTimerContainer&, typename Timer::prefix_args, Args...>;
    };

    using BasicTimer = jln::BasicTimer<jln::prefix_args<>>;
    using BasicTimerPtr = SharedPtr<BasicTimer>;

    using TimerContainer = BasicTimerContainer<BasicTimer>;


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
    NotifyDeleterBuilderWrapper<jln::TimerBuilder<SharedPtrPrivate<TimerContainer::Elem<Args...>>>>
    create_timer(Args&&... args)
    {
        using Timer = TimerContainer::Elem<Args...>;
        return {SharedPtrPrivate<Timer>
            ::New(this->timer_events_, static_cast<Args&&>(args)...)};
    }


    using GraphicTimer = jln::BasicTimer<jln::prefix_args<time_t, gdi::GraphicApi&>>;
    using GraphicTimerPtr = SharedPtr<GraphicTimer>;

    using GraphicTimerContainer = BasicTimerContainer<GraphicTimer>;

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TimerBuilder<SharedPtrPrivate<GraphicTimerContainer::Elem<Args...>>>>
    create_graphic_timer(Args&&... args)
    {
        using Action = GraphicTimerContainer::Elem<Args...>;
        return {SharedPtrPrivate<Action>
            ::New(this->graphic_timer_events_, static_cast<Args&&>(args)...)};
    }


    using CallbackEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using CallbackEventPtr = SharedPtr<CallbackEvent>;

    struct CallbackContainer : Container<CallbackEvent>
    {
        template<class... Args>
        using Elem = jln::Action<CallbackContainer&, CallbackEvent::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::ActionBuilder<SharedPtrPrivate<CallbackContainer::Elem<Args...>>>>
    create_callback_event(Args&&... args)
    {
        using Action = CallbackContainer::Elem<Args...>;
        return {SharedPtrPrivate<Action>
            ::New(this->front_events_, static_cast<Args&&>(args)...)};
    }


    using GraphicEvent = jln::ActionBase<jln::prefix_args<time_t, gdi::GraphicApi&>>;
    using GraphicEventPtr = SharedPtr<GraphicEvent>;

    struct GraphicContainer : Container<GraphicEvent>
    {
        template<class... Args>
        using Elem = jln::Action<GraphicContainer&, GraphicEvent::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::ActionBuilder<SharedPtrPrivate<GraphicContainer::Elem<Args...>>>>
    create_graphic_event(Args&&... args)
    {
        using Action = GraphicContainer::Elem<Args...>;
        return {SharedPtrPrivate<Action>
            ::New(this->graphic_events_, static_cast<Args&&>(args)...)};
    }


    using SesmanEvent = jln::ActionBase<jln::prefix_args<Inifile&>>;
    using SesmanEventPtr = SharedPtr<SesmanEvent>;

    struct SesmanContainer : Container<SesmanEvent>
    {
        template<class... Args>
        using Elem = jln::Action<SesmanContainer&, SesmanEvent::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::ActionBuilder<SharedPtrPrivate<SesmanContainer::Elem<Args...>>>>
    create_sesman_event(Args&&... args)
    {
        using Action = SesmanContainer::Elem<Args...>;
        return {SharedPtrPrivate<Action>
            ::New(this->sesman_events_, static_cast<Args&&>(args)...)};
    }

    template<class PrefixArgs_>
    struct BasicFd : jln::BasicTimer<PrefixArgs_>, jln::BasicExecutorImpl<PrefixArgs_>
    {
        using prefix_args = PrefixArgs_;
        using base_type = BasicFd;

        // using jln::BasicExecutorImpl<PrefixArgs_>::delete_self;
        // using jln::BasicExecutorImpl<PrefixArgs_>::deleter;
        using jln::BasicExecutorImpl<PrefixArgs_>::on_action;

        jln::BasicTimer<PrefixArgs_>& timer() noexcept { return *this; }

        void restart_timeout()
        {
            assert(this->delay.count() > 0);
            // TODO tvtime -> reactor.time()
            this->tv = addusectimeval(this->delay, tvtime());
        }

        int fd;
    };

    template<class PrefixArgs_, class... Ts>
    struct FdImpl : BasicFd<PrefixArgs_>
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class C, class... Args>
        FdImpl(C&, int fd, SessionReactor& session_reactor, Args&&... args)
        : ctx{static_cast<Args&&>(args)...}
        , session_reactor(session_reactor)
        {
            this->fd = fd;
            if constexpr (std::is_same<PrefixArgs, PrefixArgs_>::value) {
                this->session_reactor.timer_events_.attach(this->timer());
            }
            else {
                this->session_reactor.graphic_timer_events_.attach(this->timer());
            }
        }
        REDEMPTION_DIAGNOSTIC_POP

        ~FdImpl()
        {
            // if constexpr (std::is_same<PrefixArgs, PrefixArgs_>::value) {
            //     this->session_reactor.timer_events_.detach(this->timer());
            // }
            // else {
            //     this->session_reactor.graphic_timer_events_.detach(this->timer());
            // }
        }

        void detach() noexcept
        {
            // if constexpr (std::is_same<PrefixArgs, PrefixArgs_>::value) {
            //     this->session_reactor.fd_events_.detach(*this);
            // }
            // else {
            //     this->session_reactor.graphic_fd_events_.detach(*this);
            // }
        }

        void attach() noexcept
        {
            // if constexpr (std::is_same<PrefixArgs, PrefixArgs_>::value) {
            //     this->session_reactor.fd_events_.attach(*this);
            // }
            // else {
            //     this->session_reactor.graphic_fd_events_.attach(*this);
            // }
        }

        template<class F>
        void set_on_action(F) noexcept
        {
            this->on_action = jln::wrap_fn<F, FdImpl, jln::Executor2FdContext>();
        }

        template<class F>
        void set_on_exit(F) noexcept
        {
            this->on_exit = jln::wrap_fn<F, FdImpl, jln::Executor2FdContext>();
        }

        template<class F>
        void set_on_timeout(F) noexcept
        {
            this->timer().on_action = jln::wrap_fn<F, FdImpl, jln::Executor2FdTimeoutContext>();
        }

        void set_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->set_delay(ms);
            this->set_time(addusectimeval(this->delay, tvtime()));
            if constexpr (std::is_same<PrefixArgs, PrefixArgs_>::value) {
                this->session_reactor.timer_events_.update_delay(*this, ms);
            }
            else {
                this->session_reactor.graphic_timer_events_.update_delay(*this, ms);
            }
        }

        void update_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->set_timeout(ms);
        }

    public:
        jln::detail::tuple<Ts...> ctx;
        SessionReactor& session_reactor;
    };

    template<class PrefixArgs_, class... Args>
    using Fd = FdImpl<PrefixArgs_, typename jln::detail::decay_and_strip<Args>::type...>;

    using TopFd = BasicFd<PrefixArgs>;
    using TopFdPtr = SharedPtr<TopFd>;

    struct TopFdContainer : Container<TopFd>
    {
        template<class... Args>
        using Elem = Fd<TopFd::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TopFdBuilder<SharedPtrPrivate<TopFdContainer::Elem<Args...>>>>
    create_fd_event(int fd, Args&&... args)
    {
        using EventFd = TopFdContainer::Elem<Args...>;
        return {SharedPtrPrivate<EventFd>
            ::template New<3>(this->fd_events_, fd, *this, static_cast<Args&&>(args)...)};
    }


    using GraphicFd = BasicFd<jln::prefix_args<time_t, gdi::GraphicApi&>>;
    using GraphicFdPtr = SharedPtr<GraphicFd>;

    struct GraphicFdContainer : Container<GraphicFd>
    {
        template<class... Args>
        using Elem = Fd<GraphicFd::prefix_args, Args...>;
    };

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TopFdBuilder<SharedPtrPrivate<GraphicFdContainer::Elem<Args...>>>>
    create_graphic_fd_event(int fd, Args&&... args)
    {
        using EventFd = GraphicFdContainer::Elem<Args...>;
        return {SharedPtrPrivate<EventFd>
            ::template New<3>(this->graphic_fd_events_, fd, *this, static_cast<Args&&>(args)...)};
    }


    //std::vector<std::unique_ptr<Context>> contexts;
    CallbackContainer front_events_;
    GraphicContainer graphic_events_;
    SesmanContainer sesman_events_;
    TimerContainer timer_events_;
    GraphicTimerContainer graphic_timer_events_;
    TopFdContainer fd_events_;
    GraphicFdContainer graphic_fd_events_;

    void clear()
    {
        auto remove_dead_state = [](auto& c){
            c.elements.erase(std::remove_if(
                c.elements.begin(), c.elements.end(),
                [](auto& p){
                    if (!p->shared_ptr && p->use_count == 2) {
                        LOG(LOG_DEBUG, "detach %p", static_cast<void*>(p));
                        --p->use_count;
                        return true;
                    }
                    return false;
                }
            ), c.elements.end());
            auto e = c.elements.end();
            auto it = std::find_if(
                c.elements.begin(), e,
                [](auto& p){ return p->use_count == 1; });
            if (it != e) {
                auto itprev = it;
                ::operator delete(*it);
                LOG(LOG_DEBUG, "delete %p", static_cast<void*>(*it));
                ++it;
                while (it != e) {
                    if ((*it)->use_count == 1) {
                        ::operator delete(*it);
                        LOG(LOG_DEBUG, "delete %p", static_cast<void*>(*it));
                    }
                    else {
                        *itprev = *it;
                        ++itprev;
                    }
                    ++it;
                }
                c.elements.erase(itprev, e);
            }
        };

        remove_dead_state(front_events_);
        remove_dead_state(graphic_events_);
        remove_dead_state(sesman_events_);
        remove_dead_state(timer_events_);
        remove_dead_state(graphic_timer_events_);
        remove_dead_state(fd_events_);
        remove_dead_state(graphic_fd_events_);
    }

    ~SessionReactor()
    {
        this->clear();
        auto remove_dead_state = [](auto& c){
            for (auto& p : c.elements) {
                p->deleter(p);
            }
        };
        remove_dead_state(front_events_);
        remove_dead_state(graphic_events_);
        remove_dead_state(sesman_events_);
        remove_dead_state(timer_events_);
        remove_dead_state(graphic_timer_events_);
        remove_dead_state(fd_events_);
        remove_dead_state(graphic_fd_events_);
        this->clear();
    }

    auto& front_events()
    {
        return this->front_events_.elements;
    }

    auto& graphic_events()
    {
        return this->graphic_events_.elements;
    }

    int signal = 0;
    void set_next_event(/*BackEvent_t*/int signal)
    {
        LOG(LOG_DEBUG, "SessionReactor::set_next_event %d", signal);
        assert(!this->signal || this->signal == signal);
        this->signal = signal;
        // assert(is not already set)
        // TODO unimplemented
    }
    void set_event_next(/*BackEvent_t*/int signal)
    {
        this->set_next_event(signal);
    }
};
