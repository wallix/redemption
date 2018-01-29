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
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#pragma once

#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"

#include <vector>
#include <type_traits>
#include <utility>
#include <functional> // std::reference_wrapper
#include <chrono>
#include <cassert>
#include <new>

namespace detail
{
    template<size_t, class T>
    struct tuple_elem
    {
        T x;
    };

    template<class Ints, class... Ts>
    struct tuple_impl;

    template<std::size_t... ints, class... Ts>
    struct tuple_impl<std::integer_sequence<size_t, ints...>, Ts...>
    : tuple_elem<ints, Ts>...
    {
        template<class F, class... Args>
        decltype(auto) invoke(F && f, Args&&... args)
        {
            return f(
                static_cast<Args&&>(args)...,
                static_cast<tuple_elem<ints, Ts>&>(*this).x...
            );
        }
    };

    template<class... Ts>
    struct tuple : tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...>
    {};

    template<class T> struct decay_and_strip { using type = T; };
    template<class T> struct decay_and_strip<T&> : decay_and_strip<T>{};
    template<class T> struct decay_and_strip<T const> : decay_and_strip<T>{};
    template<class T> struct decay_and_strip<std::reference_wrapper<T>> { using type = T&; };

    template<class... Args>
    using ctx_arg_type = detail::tuple<typename decay_and_strip<Args>::type...>;
}


// #define CXX_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define CXX_WARN_UNUSED_RESULT [[nodiscard]]

class ExecutorBase;
class Executor;
template<class Ctx>
class ExecutorActionContext;

enum class REDEMPTION_CXX_NODISCARD ExecutorResult : uint8_t
{
    Nothing,
    ExitSuccess,
    ExitFailure,
    Terminate,
};

enum class ExitStatus { Error, Success, };

#ifdef IN_IDE_PARSER
struct SubExecutorBuilderConcept_
{
    template<class F> SubExecutorBuilderConcept_ on_action(F&&) && { return *this; }
    template<class F> SubExecutorBuilderConcept_ on_exit  (F&&) && { return *this; }

    template<class T> SubExecutorBuilderConcept_(T const&) noexcept;
};

struct ExecutorActionContextConcept_
{
    ExecutorResult retry();
    ExecutorResult exit(ExitStatus status);
    ExecutorResult exit_on_error();
    ExecutorResult exit_on_success();
    template<class F> ExecutorResult next_action(F);
    template<class F> ExecutorResult exec_action(F);
    template<class F1, class F2> ExecutorResult exec_action2(F1, F2);
    template<class... Args> SubExecutorBuilderConcept_ sub_executor(Args&&... args);
    template<class F> ExecutorActionContextConcept_ set_exit_action(F f);
};
#endif


namespace
{
    template<class F>
    F make_lambda() noexcept
    {
        static_assert(
            std::is_empty<F>::value,
            "F must be an empty class or a lambda expression convertible to pointer of function");
        // big hack for a lambda not default constructible before C++20 :)
        alignas(F) char const f[sizeof(F)]{}; // same as `char f`
        return reinterpret_cast<F const&>(f);
    }
}

class BasicExecutor;

enum class ExecutorError : uint8_t
{
    no_error,
    action_error,
    terminate,
};


struct BasicExecutor
{
    using OnActionPtrFunc = ExecutorResult(*)(BasicExecutor&);
    using OnExitPtrFunc = ExecutorResult(*)(BasicExecutor&, ExecutorError error);

    OnActionPtrFunc on_action;
    OnExitPtrFunc on_exit;
    BasicExecutor* current = this;
    BasicExecutor* prev = nullptr;
    void (*deleter) (void*);

    ExecutorResult exec_action()
    {
        return this->on_action(*this);
    }

    ExecutorResult exec_exit(ExecutorError error)
    {
        return this->on_exit(*this, error);
    }

    void delete_self()
    {
        return this->deleter(this);
    }

    void terminate();

    bool exec();

    void exec_all()
    {
        while (this->exec()) {
        }
    }

protected:
    BasicExecutor() = default;

    BasicExecutor& basic_executor()
    {
        return *this;
    }
};

namespace detail
{
    struct GetExecutor
    {
        template<class T>
        auto& operator()(T& x) const
        { return x.executor; }
    };

    constexpr GetExecutor get_executor {};
}


template<class T, class U>
struct is_context_convertible;

template<class T, class U>
struct check_is_context_arg_convertible
{
    static constexpr bool value = (typename std::is_convertible<T, U>::type{} = std::true_type{});
};

template<class... Ts, class... Us>
struct is_context_convertible<detail::tuple<Ts...>, detail::tuple<Us...>>
{
    static constexpr bool value = (..., (check_is_context_arg_convertible<Ts, Us>::value));
};


template<class... Ts>
struct Executor2Impl;
template<class... Ts>
struct TopExecutorImpl;
template<class... Ts>
struct SubExecutor2Impl;
template<class... Ts>
struct SubAction2Impl;

class Reactor;

namespace detail { namespace
{
    enum ExecutorType
    {
        Normal,
        Sub,
        Exec
    };

    template<class Executor, ExecutorType type, int Mask = 0>
    struct REDEMPTION_CXX_NODISCARD ExecutorBuilder
    {
        friend detail::GetExecutor;

        template<int Mask2>
        decltype(auto) select_return()
        {
            if constexpr (Mask == (~Mask2 & 0b111)) {
                if constexpr (ExecutorType::Sub == type) {
                    return ExecutorResult::Nothing;
                }
                else if constexpr (ExecutorType::Exec == type) {
                    return this->executor.exec_action();
                }
                else {
                    return this->executor;
                }
            }
            else {
                return ExecutorBuilder<Executor, type, Mask | Mask2>{this->executor};
            }
        }

        template<class F>
        decltype(auto) on_action(F f) && noexcept
        {
            static_assert(!(Mask & 0b001), "on_action already set");
            this->executor.set_on_action(f);
            return select_return<0b001>();
        }

        template<class F>
        decltype(auto) on_exit(F f) && noexcept
        {
            static_assert(!(Mask & 0b010), "on_exit already set");
            this->executor.set_on_exit(f);
            return select_return<0b010>();
        }

        template<class F>
        decltype(auto) on_timeout(F f) && noexcept
        {
            static_assert(!(Mask & 0b100), "on_timeout already set");
            this->executor.set_on_timeout(f);
            return select_return<0b100>();
        }

        ExecutorBuilder(Executor& executor) noexcept
        : executor(executor)
        {}

    private:
        Executor& executor;
    };
} }

template<class... Args>
using Executor2 = Executor2Impl<typename detail::decay_and_strip<Args>::type...>;
template<class... Args>
using TopExecutor2 = TopExecutorImpl<typename detail::decay_and_strip<Args>::type...>;

template<class... Args>
using ExecutorBuilder = detail::ExecutorBuilder<Executor2<Args...>, detail::ExecutorType::Normal, 0b100>;
template<class... Args>
using TopExecutorBuilder = detail::ExecutorBuilder<TopExecutor2<Args...>, detail::ExecutorType::Normal>;
template<class... Args>
using SubExecutorBuilder = detail::ExecutorBuilder<Executor2<Args...>, detail::ExecutorType::Sub, 0b100>;
template<class... Args>
using ExecExecutorBuilder = detail::ExecutorBuilder<Executor2<Args...>, detail::ExecutorType::Exec, 0b100>;


template<class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2ActionContext
{
    friend detail::GetExecutor;

    template<class... PreviousTs>
    Executor2ActionContext(Executor2ActionContext<PreviousTs...> const& other) noexcept
      : executor(reinterpret_cast<Executor2Impl<Ts...>&>(detail::get_executor(other)))
    {
        // TODO strip arguments support (PreviousTs=(int, int), Ts=(int))
        static_assert((..., check_is_context_arg_convertible<PreviousTs, Ts>::value));
        static_assert(sizeof(Executor2Impl<Ts...>) == sizeof(detail::get_executor(other)));
    }

    explicit Executor2ActionContext(Executor2Impl<Ts...>& executor) noexcept
      : executor{executor}
    {}

    Executor2ActionContext(Executor2ActionContext const&) = default;
    Executor2ActionContext& operator=(Executor2ActionContext const&) = delete;

    ExecutorResult retry() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult terminate() noexcept
    {
        return ExecutorResult::Terminate;
    }

    ExecutorResult exit(ExitStatus status) noexcept
    {
        return (status == ExitStatus::Success) ? this->exit_on_success() : this->exit_on_error();
    }

    ExecutorResult exit_on_error() noexcept
    {
        return ExecutorResult::ExitFailure;
    }

    ExecutorResult exit_on_success() noexcept
    {
        return ExecutorResult::ExitSuccess;
    }

    template<class... Args>
    SubExecutorBuilder<Args...> create_sub_executor(Args&&... args)
    {
        return executor.create_sub_executor(static_cast<Args&&>(args)...);
    }

    template<class... Args>
    SubExecutorBuilder<Ts..., Args...> create_nested_executor(Args&&... args)
    {
        return executor.create_nested_executor(static_cast<Args&&>(args)...);
    }

    template<class... Args>
    ExecExecutorBuilder<Args...> exec_sub_executor(Args&&... args)
    {
        auto builder = executor.create_sub_executor(static_cast<Args&&>(args)...);
        auto& sub_executor = detail::get_executor(builder);
        return {sub_executor};
    }

    template<class... Args>
    ExecExecutorBuilder<Ts..., Args...> exec_nested_executor(Args&&... args)
    {
        auto builder = executor.create_nested_executor(static_cast<Args&&>(args)...);
        auto& sub_executor = detail::get_executor(builder);
        return {sub_executor};
    }

    template<class F>
    ExecutorResult next_action(F f) noexcept
    {
        executor.set_on_action(f);
        return ExecutorResult::Nothing;
    }

    template<class F1, class F2>
    ExecutorResult exec_action2(F1 f1, F2 f2)
    {
        executor.set_on_action(f1);
        return executor.ctx.invoke(f2, Executor2ActionContext{this->executor});
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        return this->exec_action2(f, f);
    }

    template<class F>
    Executor2ActionContext set_exit_action(F f) noexcept
    {
        executor.set_on_exit(f);
        return *this;
    }

protected:
    Executor2Impl<Ts...>& executor;
};

template<class... Ts>
struct Executor2Impl : public BasicExecutor
{
    friend Executor2ActionContext<Ts...>;

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = [](BasicExecutor& executor) {
            auto& self = static_cast<Executor2Impl&>(executor);
            return self.ctx.invoke(make_lambda<F>(), Executor2ActionContext<Ts...>(self));
        };
    }

    template<class F>
    void set_on_exit(F) noexcept
    {
        this->on_exit = [](BasicExecutor& executor, ExecutorError error) {
            auto& self = static_cast<Executor2Impl&>(executor);
            // TODO ExecutorExitContext
            return self.ctx.invoke(make_lambda<F>(), Executor2ActionContext<Ts...>(self), error);
        };
    }

    template<class... Args>
    SubExecutorBuilder<Args...> create_sub_executor(Args&&... args)
    {
        auto builder = this->reactor.create_sub_executor(static_cast<Args&&>(args)...);
        auto& sub_executor = detail::get_executor(builder);
        sub_executor.current = this->current;
        sub_executor.prev = this;
        this->current->current = &sub_executor;
        return {sub_executor};
    }

    Executor2Impl(Executor2Impl const&) = delete;
    Executor2Impl& operator=(Executor2Impl const&) = delete;

    template<class... Args>
    Executor2Impl(Reactor& reactor, Args&&... args)
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
      : ctx{static_cast<Args&&>(args)...}
      , reactor(reactor)
    REDEMPTION_DIAGNOSTIC_POP
    {}

// private:
    detail::tuple<Ts...> ctx;
    Reactor& reactor;
};


template<class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2TimeoutContext : Executor2ActionContext<Ts...>
{
    using Executor2ActionContext<Ts...>::Executor2ActionContext;

    template<class F>
    Executor2ActionContext<Ts...> set_timeout_action(F f) noexcept
    {
        auto executor_action = static_cast<Executor2ActionContext<Ts...>*>(this);
        detail::get_executor(executor_action)->set_on_timeout(f);
        return *executor_action;
    }

    Executor2TimeoutContext set_timeout(std::chrono::milliseconds ms) noexcept
    {
        auto& executor = static_cast<TopExecutorImpl<Ts...>&>(this->executor);
        executor.set_timeout(ms);
        return *this;
    }
};

struct TimeoutInfo
{
    using OnTimeoutPtrFunc = ExecutorResult(*)(BasicExecutor&);
    OnTimeoutPtrFunc on_timeout;
    std::chrono::milliseconds ms;
};

template<class Executor>
struct TimoutWithExecutor
{
    TimeoutInfo timeout;
    Executor executor;

    void delete_self()
    {
        this->executor.delete_self();
    }
};

template<class... Ts>
struct TopExecutorImpl : public Executor2Impl<Ts...>
{
    using Executor2Impl<Ts...>::Executor2Impl;

    void set_timeout(std::chrono::milliseconds ms)
    {
        this->get_timeout_data(this).ms = ms;
    }

    ExecutorResult exec_timeout()
    {
        return this->get_timeout_data(*this).on_timeout(*this);
    }

    template<class F>
    void set_on_timeout(F) noexcept
    {
        this->get_timeout_data(*this).on_timeout = [](BasicExecutor& executor) {
            auto& self = static_cast<TopExecutorImpl&>(executor);
            // TODO ExecutorTimeoutContext
            return self.ctx.invoke(make_lambda<F>(), Executor2TimeoutContext<Ts...>(self));
        };
    }

    TopExecutorImpl(TopExecutorImpl const&) = delete;
    TopExecutorImpl& operator=(TopExecutorImpl const&) = delete;

    template<class... Args>
    TopExecutorImpl(Reactor& reactor, Args&&... args)
      : Executor2Impl<Ts...>(reactor, static_cast<Args&&>(args)...)
    {}

    static TimeoutInfo& get_timeout_data(TopExecutorImpl& self)
    {
        constexpr auto pad = sizeof(TimeoutInfo) % alignof(BasicExecutor);
        return *reinterpret_cast<TimeoutInfo*>(
            reinterpret_cast<uint8_t*>(&self) - pad - sizeof(TimeoutInfo));
    }

    template<class... Args>
    static TimoutWithExecutor<TopExecutorImpl>* New(Reactor& reactor, Args&&... args)
    {
        auto* p = new TimoutWithExecutor<TopExecutorImpl>{
            {}, {reactor, static_cast<Args&&>(args)...}};
        p->executor.deleter = [](void* p) {
            auto& d = get_timeout_data(*static_cast<TopExecutorImpl*>(p));
            delete reinterpret_cast<TimoutWithExecutor<TopExecutorImpl>*>(&d);
        };
        assert(static_cast<void*>(&get_timeout_data(p->executor)) == static_cast<void*>(&p->timeout));
        assert(static_cast<void*>(p) == static_cast<void*>(&p->timeout));
        return p;
    }
};

template<class... Ts>
struct TimerIdImpl;

template<class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2TimerContext
{
    friend detail::GetExecutor;

    ExecutorResult retry() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult terminate() noexcept
    {
        return ExecutorResult::Terminate;
    }

    template<class F>
    ExecutorResult next_action(F f) noexcept
    {
        executor.set_on_action(f);
        return ExecutorResult::Nothing;
    }

    template<class F1, class F2>
    ExecutorResult exec_action2(F1 f1, F2 f2)
    {
        executor.set_on_action(f1);
        return executor.ctx.invoke(f2, Executor2TimerContext{this->executor});
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        return this->exec_action2(f, f);
    }

    Executor2TimerContext set_time(std::chrono::milliseconds ms)
    {
        this->set_time(ms);
        return *this;
    }

protected:
    TimerIdImpl<Ts...>& executor;
};

struct BasicTimer
{
    using OnActionPtrFunc = ExecutorResult(*)(BasicTimer&);

    std::chrono::milliseconds ms;
    OnActionPtrFunc on_action;
    void (*deleter) (void*);

    ExecutorResult exec_action()
    {
        return this->on_action(*this);
    }

    void set_time(std::chrono::milliseconds ms)
    {
        this->ms = ms;
    }

    void delete_self()
    {
        return this->deleter(this);
    }
};

template<class... Ts>
struct TimerIdImpl : BasicTimer
{
    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = [](BasicTimer& timer) {
            auto& self = static_cast<TimerIdImpl&>(timer);
            return self.ctx.invoke(make_lambda<F>(), Executor2TimerContext<Ts...>(self));
        };
    }

    TimerIdImpl(TimerIdImpl const&) = delete;
    TimerIdImpl& operator=(TimerIdImpl const&) = delete;

    template<class... Args>
    TimerIdImpl(Reactor& reactor, std::chrono::milliseconds ms, Args&&... args)
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
      : ctx{static_cast<Args&&>(args)...}
      , reactor(reactor)
    REDEMPTION_DIAGNOSTIC_POP
    {
        this->ms = ms;
    }

// private:
    detail::tuple<Ts...> ctx;
    Reactor& reactor;
};

template<class... Args>
using TimerId = TimerIdImpl<typename detail::decay_and_strip<Args>::type...>;

namespace detail
{
    template<class Timer>
    struct REDEMPTION_CXX_NODISCARD TimerIdBuilder
    {
        template<class F>
        Timer& on_action(F f) && noexcept
        {
            this->timer.set_on_action(f);
            return this->timer;
        }

        Timer& timer;
    };
}

template<class... Args>
using TimerIdBuilder = detail::TimerIdBuilder<TimerId<Args...>>;

template<class Base>
struct Container
{
    struct Deleter
    {
        void operator()(Base* p) const
        {
            p->delete_self();
        }
    };

    using UniquePtr = std::unique_ptr<Base, Deleter>;

    template<template<class...> class Tpl, class... Args>
    Tpl<Args...>* create_pointer(Reactor& reactor, Args&&... args)
    {
        auto* p = new Tpl<Args...>(reactor, static_cast<Args&&>(args)...);
        p->deleter = [](void* p) { delete static_cast<Tpl<Args...>*>(p); };
        return p;
    }

    template<template<class...> class Tpl, class... Args>
    Tpl<Args...>& emplace_back(Reactor& reactor, Args&&... args)
    {
        auto* p = this->create_pointer<Tpl>(reactor, static_cast<Args&&>(args)...);
        return *static_cast<Tpl<Args...>*>(this->xs.emplace_back(p).get());
    }

    template<class T>
    T& emplace_back(T* p)
    {
        return *static_cast<T*>(this->xs.emplace_back(p).get());
    }

    std::vector<UniquePtr> xs;
};

struct Reactor
{
    template<class... Args>
    TopExecutorBuilder<Args...> create_executor(int /*fd*/, Args&&... args)
    {
        auto* p = TopExecutor2<Args...>::New(*this, static_cast<Args&&>(args)...);
        this->executors.emplace_back(reinterpret_cast<TimoutWithExecutor<BasicExecutor>*>(p));
        return {p->executor};
        //return {this->executors.emplace_back<TopExecutor2>(*this, static_cast<Args&&>(args)...)};
    }

    template<class... Args>
    ExecutorBuilder<Args...> create_sub_executor(Args&&... args)
    {
        return {*this->executors.create_pointer<Executor2>(*this, static_cast<Args&&>(args)...)};
        //return {this->executors.emplace_back<Executor2>(*this, static_cast<Args&&>(args)...)};
    }

    template<class... Args>
    TimerIdBuilder<Args...> create_timer(std::chrono::milliseconds ms, Args&&... args)
    {
        return {this->timers.emplace_back<TimerId>(*this, ms, static_cast<Args&&>(args)...)};
    }

private:
    Container<TimoutWithExecutor<BasicExecutor>> executors;
    Container<BasicTimer> timers;
};


namespace detail { namespace {
    template<class... Ts>
    static ExecutorResult terminate_callee(Ts...)
    {
        assert("call a executor marked 'Terminate'");
        return ExecutorResult::Terminate;
    }
} }

bool BasicExecutor::exec()
{
    auto process_exit = [this](ExecutorError error) {
        do {
            switch (this->current->exec_exit(error)) {
                case ExecutorResult::ExitSuccess:
                    if (this->current == this) {
                        this->on_action = detail::terminate_callee;
                        this->on_exit = detail::terminate_callee;
                        return false;
                    }
                    std::exchange(this->current, this->current->prev)->delete_self();
                    error = ExecutorError::no_error;
                    break;
                case ExecutorResult::ExitFailure:
                    if (this->current == this) {
                        this->on_action = detail::terminate_callee;
                        this->on_exit = detail::terminate_callee;
                        return false;
                    }
                    std::exchange(this->current, this->current->prev)->delete_self();
                    error = ExecutorError::action_error;
                    break;
                case ExecutorResult::Terminate:
                    this->terminate();
                    return false;
                    break;
                case ExecutorResult::Nothing:
                    return true;
            }
        } while (this->current);
        return false;
    };

    switch (this->current->exec_action()) {
        case ExecutorResult::ExitSuccess:
            return process_exit(ExecutorError::no_error);
        case ExecutorResult::ExitFailure:
            return process_exit(ExecutorError::action_error);
        case ExecutorResult::Terminate:
            this->terminate();
            return false;
            break;
        case ExecutorResult::Nothing:
            break;
    }

    return this->current;
}

void BasicExecutor::terminate()
{
    while (this->current != this) {
        (void)this->current->exec_exit(ExecutorError::terminate);
        std::exchange(this->current, this->current->prev)->delete_self();
    }
    (void)this->current->exec_exit(ExecutorError::terminate);
    this->on_action = detail::terminate_callee;
    this->on_exit = detail::terminate_callee;
    //TODO this->on_timeout = detail::terminate_callee;
}
