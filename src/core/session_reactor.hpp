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
     *    +-------------------------------------------------------+
     *    |  Builder<Ptr>                                         |
     *    |   Ptr ptr =                                           |
     *    |     +-----------------------+---------------------+   |
     *    |     |  SharedPtrPrivate<T>  |  SharedPtrBase<T> p |   |
     *    |     +-----------------------+---------------------+   |
     *    |                                        ||             |
     *    +-------------------------------------------------------+
     *                                             ||
     *                                            move
     *                                             ||
     *                                             \/
     *   .----> +-----------------------+------------------------+ <-- public
     *  /       |    SharedPtr<Base>    |  SharedPtrBase<Base> p |
     *  |       +-----------------------+------------------------+
     *  |    ______________________________________/
     *  |   /
     *  |   \__.+-----------------------+------------------------+
     *  |       |  SharedPtrBase<Base>  | SharedData<Base>* data |
     *  |       +-----------------------+------------------------+
     *  |       _________________________________/
     *  |      /
     *  |      \__.+--------------------+--------------+
     *  |          |   SharedDataBase   |   use_count  | <._
     *  +----------| - - - - - - - - - -|- -shared_ptr | <-- internal
     *             |                    |    deleter   | <-*
     *             +--------------------+-+----------+-|
     *             |  SharedData<Base>  | |   base   | | <-- public
     *             +--------------------+-|     ^    |-+
     *             |   SharedData<T>    | |   value  | | <-- private
     *             +--------------------+-+----------+-+
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

        bool alive() const noexcept
        {
            return bool(this->shared_ptr);
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

    struct SharedPtrAccess
    {
        template<class T>
        static auto& p(T& p_) noexcept
        {
            return p_.p;
        }
    };

    template<class T>
    class SharedPtr
    {
    protected:
        SharedPtrBase<T> p;
        friend SharedPtrAccess;

    public:
        using value_type = T;

        SharedPtr() = default;

        template<class U>
        SharedPtr(SharedPtr<U>&& other) noexcept
          : p(std::move(SharedPtrAccess::p(other)))
        {
            this->p.data->shared_ptr = &this->p;
        }

        template<class U>
        SharedPtr& operator=(SharedPtr<U>&& other) noexcept
        {
            assert(static_cast<SharedDataBase*>(SharedPtrAccess::p(other).data) != static_cast<SharedDataBase*>(this->p.data));
            this->reset();
            this->p = std::move(SharedPtrAccess::p(other));
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

    template<class T>
    class SharedPtrPrivate : public SharedPtr<T>
    {
        using Data = typename SharedPtrBase<T>::Data;

        SharedPtrPrivate(Data* data) noexcept
        {
            this->p.data = data;
            this->p.data->shared_ptr = &this->p;
        }

        friend SharedPtrAccess;

    public:
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

                static_cast<SharedPtrBase<T>*>(base->shared_ptr)->data = nullptr;
                base->shared_ptr = nullptr;
                --base->use_count;
                auto* data = static_cast<Data*>(base);
                assert(static_cast<void*>(data) == static_cast<void*>(base));
                if constexpr (!std::is_same<F, std::nullptr_t>::value) {
                    data->value.ctx.invoke(jln::make_lambda<F>());
                }
                data->value.~T();
            };
        }

        template<int Use = 1, class C, class... Args>
        static SharedPtrPrivate New(C&& c, Args&&... args)
        {
            Data* data = static_cast<Data*>(::operator new(sizeof(Data)));
            LOG(LOG_DEBUG, "new %p %s", static_cast<void*>(data), typeid(T).name());
            if constexpr (noexcept(T(static_cast<Args&&>(args)...))) {
                new(&data->value) T(static_cast<Args&&>(args)...);
            }
            else {
                bool failed = true;
                SCOPE_EXIT(if (failed) {
                    ::operator delete(data);
                });
                new(&data->value) T(static_cast<Args&&>(args)...);
                failed = false;
            }
            c.attach(data);
#ifndef NDEBUG
            data->shared_ptr = nullptr;
#endif
            data->use_count = Use + 1;
            data->deleter = SharedPtrPrivate::make_deleter(nullptr);
            return SharedPtrPrivate(data);
        }
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

    template<class Base>
    struct Container
    {
        using Data = SharedData<Base>;

        template<class U>
        void attach(SharedData<U>* data_) noexcept
        {
            static_assert(std::is_base_of<Base, U>::value);
            auto* data = reinterpret_cast<Data*>(data_);
            assert(this->elements.end() == std::find(this->elements.begin(), this->elements.end(), data));
            this->elements.emplace_back(data);
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
            auto run_element = [&](auto& elem){
                if (!pred(elem.value)) {
                    return true;
                }
                switch (elem.value.exec_action(static_cast<Args&&>(args)...)) {
                    case jln::ExecutorResult::ExitSuccess:
                    case jln::ExecutorResult::ExitFailure:
                        assert(false && "Exit");
                    case jln::ExecutorResult::Terminate:
                        assert(elem.alive());
                        LOG(LOG_DEBUG, "f = %p %d", static_cast<void*>(&elem), elem.use_count);
                        elem.apply_deleter();
                        return false;
                    case jln::ExecutorResult::NeedMoreData:
                        assert(false && "NeedMoreData");
                    case jln::ExecutorResult::Nothing:
                    case jln::ExecutorResult::Ready:
                        return true;
                }
                return true;
            };

            this->run_elements(run_element);
        }

        template<class RunElement>
        void run_elements(RunElement run_element)
        {
            auto& cont = this->elements;
            for (std::size_t i = 0; i < cont.size(); ++i) {
                auto* data_ptr = cont[i];
                if (!data_ptr->alive() || !run_element(*data_ptr)) {
                    auto alive_i = i;
                    if (--data_ptr->use_count == 0) {
                        ::operator delete(data_ptr);
                    }
                    while (++i < cont.size()) {
                        auto* data_ptr = cont[i];
                        if (!data_ptr->alive() || run_element(*data_ptr)) {
                            cont[alive_i] = data_ptr;
                            ++alive_i;
                        }
                        else if (--data_ptr->use_count == 0) {
                            ::operator delete(data_ptr);
                        }
                    }
                    LOG(LOG_DEBUG, "dead: %zu", cont.size() - alive_i);
                    cont.erase(cont.begin() + alive_i, cont.end());
                    break;
                }
            }
        }

        void clear()
        {
            for (auto* data_ptr : this->elements) {
                switch (data_ptr->use_count) {
                    case 3:
                        assert(data_ptr->alive());
                        --data_ptr->use_count;
                        break;
                    case 2:
                        if (data_ptr->alive()) {
                            data_ptr->apply_deleter();
                        }
                        else {
                            --data_ptr->use_count;
                        }
                        break;
                    case 1:
                        ::operator delete(data_ptr);
                        break;
                }
            }
            this->elements.clear();
        }

        auto get_elem_iterator(Base& base)
        {
            return std::find_if(
                this->elements.begin(), this->elements.end(),
                [&](Data* data){ return &data->value == &base; });
        }

        // TODO CountedPointer
        std::vector<Data*> elements;

        template<template<class...> class Tpl, class... Args>
        using Elem = Tpl<typename Base::prefix_args, Args...>;

        template<template<class...> class Tpl, class... Args>
        using Ptr = SharedPtrPrivate<Elem<Tpl, Args...>>;
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
        // TODO end_tv = get_current_time()
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

        template<class ReactorTimerWrapper, class... Args>
        using Elem = jln::Timer<ReactorTimerWrapper, typename Timer::prefix_args, Args...>;

        template<class ReactorTimerWrapper, class... Args>
        using Ptr = SharedPtrPrivate<Elem<ReactorTimerWrapper, Args...>>;

        template<class ReactorTimerWrapper, class... Args>
        NotifyDeleterBuilderWrapper<jln::TimerBuilder<Ptr<ReactorTimerWrapper, Args...>>>
        create_shared_ptr(ReactorTimerWrapper r, Args&&... args)
        {
            return Ptr<ReactorTimerWrapper, Args...>::New(*this, r, static_cast<Args&&>(args)...);
        }
    };


    struct ReactorWrapper
    {
        SessionReactor& get_reactor() const noexcept
        {
            return this->reactor;
        }

        SessionReactor& reactor;
    };

    template<class Base>
    struct ActionContainer : Container<Base>
    {
        template<class... Args>
        using Elem = jln::Action<ReactorWrapper, typename Base::prefix_args, Args...>;

        template<class... Args>
        using Ptr = SharedPtrPrivate<Elem<Args...>>;

        template<class... Args>
        using Builder = NotifyDeleterBuilderWrapper<jln::ActionBuilder<Ptr<Args...>>>;

        template<class... Args>
        Builder<Args...> create_shared_ptr(SessionReactor& r, Args&&... args)
        {
            return {Ptr<Args...>::New(*this, r, static_cast<Args&&>(args)...)};
        }
    };


    template<auto timers>
    struct ReactorTimerWrapper
    {
        SessionReactor& get_reactor() const noexcept
        {
            return this->reactor;
        }

        template<class Timer>
        void update_delay(Timer& timer, std::chrono::milliseconds ms)
        {
            (this->reactor.*timers).update_delay(timer, ms);
        }

        template<class Timer>
        void update_time(Timer& timer, timeval const& tv)
        {
            (this->reactor.*timers).update_time(timer, tv);
        }

        SessionReactor& reactor;
    };

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

        BasicFd(int fd) noexcept
        : fd(fd)
        {}

        const int fd;
    };

    template<class PrefixArgs_, class... Ts>
    struct FdImpl : BasicFd<PrefixArgs_>
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Args>
        FdImpl(int fd, SessionReactor& session_reactor, Args&&... args)
        noexcept(noexcept(jln::detail::tuple<Ts...>{static_cast<Args&&>(args)...}))
        : BasicFd<PrefixArgs_>(fd)
        , ctx{static_cast<Args&&>(args)...}
        , session_reactor(session_reactor)
        {}
        REDEMPTION_DIAGNOSTIC_POP

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

        SessionReactor& get_reactor() const noexcept
        {
            return this->session_reactor;
        }

    public:
        jln::detail::tuple<Ts...> ctx;

    private:
        SessionReactor& session_reactor;
    };

    template<class EventContainer, class PrefixArgs_, class... Args>
    using Fd = FdImpl<PrefixArgs_, typename jln::detail::decay_and_strip<Args>::type...>;

    template<class Base>
    struct FdContainer : Container<Base>
    {
        template<class... Args>
        using Elem = Fd<FdContainer, typename Base::prefix_args, Args...>;

        template<class... Args>
        using Ptr = SharedPtrPrivate<Elem<Args...>>;

        template<class Timers, class... Args>
        Ptr<Args...> create_shared_ptr(Timers& timers, int fd, SessionReactor& r, Args&&... args)
        {
            struct Cont
            {
                void attach(SharedData<Elem<Args...>>* p)
                {
                    this->fds.attach(p);
                    this->timers.attach(p);
                }
                FdContainer& fds;
                Timers& timers;
            };
            Cont c{*this, timers};
            return Ptr<Args...>::template New<2>(c, fd, r, static_cast<Args&&>(args)...);
        }

        template<class... Args>
        void exec(fd_set& rfds, Args&&... args)
        {
            auto run_element = [&](auto& elem){
                // LOG(LOG_DEBUG, "is set fd: %d %d", c[i]->fd, io_fd_isset(c[i]->fd, rfds));
                if (io_fd_isset(elem.value.fd, rfds)
                 && !elem.value.exec(static_cast<Args&&>(args)...)) {
                    elem.apply_deleter();
                    return false;
                }
                return true;
            };

            this->run_elements(run_element);
        }
    };

    using BasicTimer = jln::BasicTimer<jln::prefix_args<>>;
    using BasicTimerPtr = SharedPtr<BasicTimer>;

    using TimerContainer = BasicTimerContainer<BasicTimer>;

    template<class... Args>
    auto create_timer(Args&&... args)
    {
        using W = ReactorTimerWrapper<&SessionReactor::timer_events_>;
        return this->timer_events_.create_shared_ptr(W{*this}, static_cast<Args&&>(args)...);
    }


    using GraphicTimer = jln::BasicTimer<jln::prefix_args<gdi::GraphicApi&>>;
    using GraphicTimerPtr = SharedPtr<GraphicTimer>;

    using GraphicTimerContainer = BasicTimerContainer<GraphicTimer>;

    template<class... Args>
    auto create_graphic_timer(Args&&... args)
    {
        using W = ReactorTimerWrapper<&SessionReactor::graphic_timer_events_>;
        return this->graphic_timer_events_.create_shared_ptr(W{*this}, static_cast<Args&&>(args)...);
    }


    using CallbackEvent = jln::ActionBase<jln::prefix_args<Callback&>>;
    using CallbackEventPtr = SharedPtr<CallbackEvent>;

    using CallbackContainer = ActionContainer<CallbackEvent>;

    template<class... Args>
    CallbackContainer::Builder<Args...>
    create_callback_event(Args&&... args)
    {
        return this->front_events_.create_shared_ptr(*this, static_cast<Args&&>(args)...);
    }


    using GraphicEvent = jln::ActionBase<jln::prefix_args<gdi::GraphicApi&>>;
    using GraphicEventPtr = SharedPtr<GraphicEvent>;

    using GraphicContainer = ActionContainer<GraphicEvent>;

    template<class... Args>
    GraphicContainer::Builder<Args...>
    create_graphic_event(Args&&... args)
    {
        return this->graphic_events_.create_shared_ptr(*this, static_cast<Args&&>(args)...);
    }


    using SesmanEvent = jln::ActionBase<jln::prefix_args<Inifile&>>;
    using SesmanEventPtr = SharedPtr<SesmanEvent>;

    using SesmanContainer = ActionContainer<SesmanEvent>;

    template<class... Args>
    SesmanContainer::Builder<Args...>
    create_sesman_event(Args&&... args)
    {
        return this->sesman_events_.create_shared_ptr(*this, static_cast<Args&&>(args)...);
    }


    using TopFd = BasicFd<PrefixArgs>;
    using TopFdPtr = SharedPtr<TopFd>;

    using TopFdContainer = FdContainer<TopFd>;

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TopFdBuilder<TopFdContainer::Ptr<Args...>>>
    create_fd_event(int fd, Args&&... args)
    {
        return {this->fd_events_.create_shared_ptr(
            this->timer_events_, fd, *this, static_cast<Args&&>(args)...)};
    }


    using GraphicFd = BasicFd<jln::prefix_args<gdi::GraphicApi&>>;
    using GraphicFdPtr = SharedPtr<GraphicFd>;

    using GraphicFdContainer = FdContainer<GraphicFd>;

    template<class... Args>
    NotifyDeleterBuilderWrapper<jln::TopFdBuilder<GraphicFdContainer::Ptr<Args...>>>
    create_graphic_fd_event(int fd, Args&&... args)
    {
        return {this->graphic_fd_events_.create_shared_ptr(
            this->graphic_timer_events_, fd, *this, static_cast<Args&&>(args)...)};
    }


    CallbackContainer front_events_;
    GraphicContainer graphic_events_;
    SesmanContainer sesman_events_;
    TimerContainer timer_events_;
    GraphicTimerContainer graphic_timer_events_;
    TopFdContainer fd_events_;
    GraphicFdContainer graphic_fd_events_;

    timeval current_time {};

    void set_current_time(timeval const& now)
    {
        assert(now >= this->current_time);
        this->current_time = now;
    }

    timeval get_current_time() const noexcept
    {
        //assert((this->current_time.tv_sec /*> -1*/) && "current_time is uninitialized. Used set_current_time");
        return this->current_time;
    }

    struct EnableGraphics
    {
        explicit EnableGraphics(bool enable) noexcept
          : enable(enable)
        {}

        explicit operator bool () const noexcept
        {
            return this->enable;
        }

        const bool enable;
    };

    timeval get_next_timeout(EnableGraphics enable_gd)
    {
        auto tv = this->timer_events_.get_next_timeout();
        if (enable_gd) {
            auto const tv2 = this->graphic_timer_events_.get_next_timeout();
            if (tv.tv_sec < 0) {
                tv = tv2;
            }
            else if (tv2.tv_sec >= 0) {
                tv = std::min(tv, tv2);
            }
        }
        return tv;
    }

    template<class F>
    void for_each_fd(EnableGraphics enable_gd, F f)
    {
        for (auto& top_fd : this->fd_events_.elements) {
            if (top_fd->alive()) {
                assert(top_fd->value.fd != -1);
                f(top_fd->value.fd, top_fd->value);
            }
        }
        if (enable_gd) {
            for (auto& top_fd : this->graphic_fd_events_.elements) {
                if (top_fd->alive()) {
                    assert(top_fd->value.fd != -1);
                    f(top_fd->value.fd, top_fd->value);
                }
            }
        }
    }

    template<class GetGd>
    void execute_timers(EnableGraphics enable_gd, GetGd get_gd)
    {
        auto const end_tv = this->get_current_time();
        this->timer_events_.exec(end_tv);
        if (enable_gd) {
            this->graphic_timer_events_.exec(end_tv, get_gd());
        }
    }

    template<class GetGd>
    void execute_timers_at(EnableGraphics enable_gd, timeval const& end_tv, GetGd get_gd)
    {
        this->set_current_time(end_tv);
        this->execute_timers(enable_gd, get_gd);
    }

    void execute_graphics(fd_set& rfds, gdi::GraphicApi& gd)
    {
        this->graphic_events_.exec(gd);
        this->graphic_fd_events_.exec(rfds, gd);
    }

    void execute_sesman(Inifile& ini)
    {
        this->sesman_events_.exec(ini);
    }

    void execute_callbacks(Callback& callback)
    {
        this->front_events_.exec(callback);
    }

    ~SessionReactor()
    {
        front_events_.clear();
        graphic_events_.clear();
        sesman_events_.clear();
        timer_events_.clear();
        graphic_timer_events_.clear();
        fd_events_.clear();
        graphic_fd_events_.clear();
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
