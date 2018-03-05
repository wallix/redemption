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
#include "utils/sugar/std_stream_proto.hpp"
#include "utils/difftimeval.hpp"

#include <algorithm>
#include <vector>
#include <type_traits>
#include <utility>
#include <functional> // std::reference_wrapper
#include <chrono>
#include <cassert>
#include <memory>

namespace jln
{

namespace detail
{
    template<class... Ts>
    struct tuple;

    template<class Ints, class... Ts>
    struct tuple_impl;

    template<class T, class... Ts>
    struct emplace_type
    {
        tuple<Ts...> t;

        template<class... Us>
        auto operator()(Us&&... xs) const
        {
            static_assert(0 == sizeof...(Ts));
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
            return emplace_type<T, Us&&...>{{xs...}};
            REDEMPTION_DIAGNOSTIC_POP
        }
    };

    template<size_t, class T>
    struct tuple_elem
    {
        T x;

        template<std::size_t... ints, class... Ts>
        constexpr tuple_elem(int, tuple_impl<std::integer_sequence<size_t, ints...>, Ts...>& t)
          : x{static_cast<Ts&&>(static_cast<tuple_elem<ints, Ts>&>(t).x)...}
        {}

        template<class... Ts>
        constexpr tuple_elem(emplace_type<T, Ts...> e)
          : tuple_elem(1, e.t)
        {}

        template<class U>
        constexpr tuple_elem(U&& x)
          : x(static_cast<U&&>(x))
        {}
    };

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

    template<std::size_t i, class T>
    T& get(tuple_elem<i, T>& e) noexcept
    {
        return e.x;
    }

    template<std::size_t i, class T>
    T&& get(tuple_elem<i, T>&& e) noexcept
    {
        return static_cast<T&&>(e.x);
    }

    template<std::size_t i, class T>
    T const& get(tuple_elem<i, T> const& e) noexcept
    {
        return e.x;
    }

    template<class T> struct tuple_size;
    template<class... Ts> struct tuple_size<tuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)>
    {};

    template<class T> struct decay_and_strip { using type = T; };
    template<class T> struct decay_and_strip<T&> : decay_and_strip<T>{};
    template<class T> struct decay_and_strip<T const> : decay_and_strip<T>{};
    template<class T> struct decay_and_strip<std::reference_wrapper<T>> { using type = T&; };
    template<class T, class... Ts> struct decay_and_strip<emplace_type<T, Ts...>> { using type = T; };
}

template<class T>
constexpr auto emplace = detail::emplace_type<T>{};

// TODO namespace detail
template<class...>
struct prefix_args;

enum class REDEMPTION_CXX_NODISCARD ExecutorResult : int8_t
{
    Nothing,
    NeedMoreData,
    Ready,
    ExitSuccess,
    ExitFailure,
    Terminate,
};

enum class ExitStatus : bool {
    Error,
    Success,
};

enum class ExecutorError : int8_t
{
    NoError,
    ActionError,
    Terminate,
    ExternalExit,
};

#ifdef IN_IDE_PARSER
struct BasicExecutorConcept_
{
    template<class... Args> ExecutorResult exec_action(Args&&... args);
    template<class... Args> ExecutorResult exec_exit(ExecutorError error, Args&&... args);
    void delete_self();
};

struct TopExecutorBuilderConcept_
{
    template<class F> TopExecutorBuilderConcept_ on_action (F&&) &&;
    template<class F> TopExecutorBuilderConcept_ on_exit   (F&&) &&;
    template<class F> TopExecutorBuilderConcept_ on_timeout(std::chrono::milliseconds, F&&) &&;

    template<class T> TopExecutorBuilderConcept_(T const&) noexcept;
};

struct SubExecutorBuilderConcept_
{
    template<class F> SubExecutorBuilderConcept_ on_action(F&&) &&;
    template<class F> SubExecutorBuilderConcept_ on_exit  (F&&) &&;

    template<class T> SubExecutorBuilderConcept_(T const&) noexcept;
};

struct ExecutorActionContextConcept_
{
    ExecutorResult ready();
    ExecutorResult need_more_data();
    ExecutorResult terminate();
    ExecutorResult exit(ExitStatus status);
    ExecutorResult exit_on_error();
    ExecutorResult exit_on_success();
    template<class F> ExecutorResult next_action(F);
    template<class F> ExecutorResult exec_action(F);
    template<class F1, class F2> ExecutorResult exec_action2(F1, F2);

    template<class... Args> auto create_timer(Args&&...);
    template<class... Args> SubExecutorBuilderConcept_ create_sub_executor(Args&&...);
    BasicExecutorConcept_& get_basic_executor();
    template<class... Args> SubExecutorBuilderConcept_ create_nested_executor(Args&&...);
    template<class... Args> SubExecutorBuilderConcept_ exec_sub_executor(Args&&...);
    template<class... Args> SubExecutorBuilderConcept_ exec_nested_executor(Args&&...);
    template<class F> ExecutorActionContextConcept_ set_exit_action(F);
    template<class... Args> SubExecutorBuilderConcept_ sub_executor(Args&&...);
};

struct ExecutorTimeoutContextConcept_ : ExecutorActionContextConcept_
{
    template<class F> ExecutorActionContextConcept_ set_timeout_action(F);
    ExecutorTimeoutContextConcept_ set_timeout(std::chrono::milliseconds ms);
};

struct ExecutorTimerContextConcept_
{
    ExecutorResult terminate();
    ExecutorResult exit(ExitStatus status);
    ExecutorResult exit_on_error();
    ExecutorResult exit_on_success();
    template<class F> ExecutorResult next_action(F f);
    template<class F> ExecutorResult exec_action(F f);
    template<class F1, class F2> ExecutorResult exec_action2(F1 f1, F2 f2);

    ExecutorResult detach_timer();
    ExecutorResult retry();
    ExecutorResult retry_until(std::chrono::milliseconds ms);
    ExecutorTimerContextConcept_ set_time(std::chrono::milliseconds ms);
};
#endif


namespace detail
{
    template<class BaseType, class PrefixArgs, class...Ts>
    struct make_func_ptr;

    template<class BaseType, class...Ts, class...Us>
    struct make_func_ptr<BaseType, prefix_args<Ts...>, Us...>
    {
        using type = ExecutorResult(*)(BaseType&, Us..., Ts...);
    };
}

template<class BaseType, class PrefixArgs, class...Ts>
using MakeFuncPtr = typename detail::make_func_ptr<BaseType, PrefixArgs, Ts...>::type;

constexpr auto default_action_function() noexcept
{
    return []([[maybe_unused]] auto... args){
        return ExecutorResult::Nothing;
    };
}

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


template<class PrefixArgs>
struct ActionBase
{
    using prefix_args = PrefixArgs;
    using base_type = ActionBase;

    void delete_self() noexcept
    {
        return this->deleter(this);
    }

    template<class... Args>
    ExecutorResult exec_action(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    using OnEventPtrFunc = MakeFuncPtr<ActionBase, PrefixArgs>;

    OnEventPtrFunc on_action = default_action_function();
    void  (*deleter) (ActionBase*) noexcept = [](ActionBase*) noexcept {};
    ActionBase() = default;
};

template<class PrefixArgs>
struct BasicTimer
{
    using prefix_args = PrefixArgs;
    using base_type = BasicTimer;

    void delete_self() noexcept
    {
        return this->deleter(this);
    }

    void reset_time() noexcept
    {
        this->tv = addusectimeval(this->delay, tvtime());
    }

    timeval time() const noexcept
    {
        return this->tv;
    }

    // TODO set_delay
    void set_time(std::chrono::milliseconds ms) noexcept
    {
        this->delay = std::chrono::duration_cast<std::chrono::microseconds>(ms);
        this->tv = addusectimeval(this->delay, tvtime());
    }

    void set_time(timeval const& tv) noexcept
    {
        this->delay = std::chrono::microseconds(-1);
        this->tv = tv;
    }

    template<class... Args>
    ExecutorResult exec_timer(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    template<class... Args>
    ExecutorResult exec_action(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    using OnTimerPtrFunc = MakeFuncPtr<BasicTimer&, PrefixArgs>;

    timeval tv {};
    OnTimerPtrFunc on_action = default_action_function();
    void (*deleter) (BasicTimer*) noexcept = [](BasicTimer*) noexcept {};
    std::chrono::microseconds delay;

    BasicTimer() = default;
};

template<class PrefixArgs>
struct BasicExecutorImpl
{
    using prefix_args = PrefixArgs;
    using base_type = BasicExecutorImpl;

    template<class... Args>
    ExecutorResult exec_action(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    template<class... Args>
    ExecutorResult exec_exit(ExecutorError error, Args&&... args)
    {
        return this->on_exit(*this, error, static_cast<Args&&>(args)...);
    }

    void delete_self()
    {
        return this->deleter(this);
    }

    template<class... Args>
    void terminate(Args&&... args);

    template<class... Args>
    bool exec(Args&&... args);

    template<class... Args>
    void exec_all(Args&&... args)
    {
        while (this->exec(static_cast<Args&&>(args)...)) {
        }
    }

    template<class... Args>
    bool exit_with(ExecutorError, Args&&... args);

// protected:
    using OnActionPtrFunc = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs>;
    using OnExitPtrFunc = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs, ExecutorError>;

    OnActionPtrFunc on_action = default_action_function();
    OnExitPtrFunc on_exit = default_action_function();
    BasicExecutorImpl* current = this;
    BasicExecutorImpl* prev = nullptr;
    void (*deleter) (BasicExecutorImpl*) noexcept = [](BasicExecutorImpl*) noexcept {};

    void set_next_executor(BasicExecutorImpl& other) noexcept
    {
        other.current = this->current;
        other.prev = this;
        this->current->current = &other;
    }

    BasicExecutorImpl() = default;
};



template<class PrefixArgs>
class TopExecutorTimersImpl;

template<class... Ts>
using TopExecutorTimers = TopExecutorTimersImpl<prefix_args<Ts...>>;

template<class... Args>
using BasicExecutor = BasicExecutorImpl<prefix_args<Args...>>;

namespace detail
{
    struct GetExecutor
    {
        template<class T>
        auto& operator()(T& x) const
        { return x.executor; }
    };

    constexpr GetExecutor get_executor {};

    struct GetEvent
    {
        template<class T>
        auto& operator()(T& x) const
        { return x.event; }
    };

    constexpr GetEvent get_event {};


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
}


template<class PrefixArgs, class... Ts>
struct Executor2Impl;
template<class PrefixArgs, class... Ts>
struct TopExecutorImpl;

namespace detail { namespace
{
    enum ExecutorType
    {
        Normal,
        Sub,
        Exec
    };

    template<template<class...> class Tpl, class T>
    struct rewrap;

    template<template<class...> class NewTpl, template<class...> class Tpl, class... Ts>
    struct rewrap<NewTpl, Tpl<Ts...>>
    {
        using type = NewTpl<Ts...>;
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
                    return this->executor;
                }
                else {
                    // TODO Executor2ActionContext
                    // return typename rewrap<Executor2ActionContext, Executor>::type{
                    //    this->executor};
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
        decltype(auto) on_timeout(std::chrono::milliseconds ms, F f) && noexcept
        {
            static_assert(!(Mask & 0b100), "on_timeout already set");
            this->executor.set_timeout(ms);
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

template<class PrefixArgs, class... Args>
using Executor2 = Executor2Impl<PrefixArgs, typename detail::decay_and_strip<Args>::type...>;
template<class PrefixArgs, class... Args>
using TopExecutor2 = TopExecutorImpl<PrefixArgs, typename detail::decay_and_strip<Args>::type...>;

template<class PrefixArgs, class... Args>
using TopExecutorBuilder = detail::ExecutorBuilder<
    TopExecutor2<PrefixArgs, Args...>, detail::ExecutorType::Normal>;
template<class PrefixArgs, class... Args>
using SubExecutorBuilder = detail::ExecutorBuilder<
    Executor2<PrefixArgs, Args...>, detail::ExecutorType::Sub, 0b100>;
template<class PrefixArgs, class... Args>
using ExecExecutorBuilder = detail::ExecutorBuilder<
    Executor2<PrefixArgs, Args...>, detail::ExecutorType::Exec, 0b100>;


template<class PrefixArgs, class... Ts>
struct Timer2Impl;

template<class Event>
struct REDEMPTION_CXX_NODISCARD BasicContext
{
    template<class OtherEvent>
    BasicContext(BasicContext<OtherEvent> const& other) noexcept
      : event(reinterpret_cast<Event&>(detail::get_event(other)))
    {
        // TODO strip arguments support (PreviousTs=(int, int), Ts=(int))
        static_assert(detail::is_context_convertible<
            typename Event::tuple_context, typename OtherEvent::tuple_context>::value);
        static_assert(sizeof(Event) == sizeof(detail::get_event(other)));
    }

    explicit BasicContext(Event& event) noexcept
      : event{event}
    {}

    BasicContext(BasicContext const&) = default;
    BasicContext& operator=(BasicContext const&) = delete;

    friend detail::GetEvent;

    ExecutorResult terminate() noexcept
    {
        return ExecutorResult::Terminate;
    }

    template<class F>
    ExecutorResult next_action(F f) noexcept
    {
        this->event.set_on_action(f);
        return ExecutorResult::Nothing;
    }

    template<class F1, class F2>
    ExecutorResult exec_action2(F1 f1, F2 f2)
    {
        this->event.set_on_action(f1);
        return this->event.ctx.invoke(f2, BasicContext{this->event});
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        return this->exec_action2(f, f);
    }

    friend detail::GetExecutor;

protected:
    Event& event;
};

template<class Timer>
struct REDEMPTION_CXX_NODISCARD Executor2TimerContext : BasicContext<Timer>
{
    using BasicContext<Timer>::BasicContext;

    ExecutorResult detach_timer() noexcept
    {
        this->event.detach_timer();
        return ExecutorResult::ExitSuccess;
    }

    ExecutorResult ready() noexcept
    {
        this->event.reset_time();
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready_until(std::chrono::milliseconds ms)
    {
        this->event.update_time(ms);
        return ExecutorResult::Nothing;
    }

    Executor2TimerContext set_time(std::chrono::milliseconds ms)
    {
        this->event.update_time(ms);
        return *this;
    }
};


template<class Executor>
struct REDEMPTION_CXX_NODISCARD Executor2ActionContext : BasicContext<Executor>
{
    ExecutorResult need_more_data() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Ready;
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
    auto create_timer(Args&&... args)
    {
        return this->event.top_executor_timers.create_timer(static_cast<Args&&>(args)...);
    }

//     template<class... Args>
//     SubExecutorBuilder2<Executor> create_sub_executor(Args&&... args)
//     {
//         return this->event.create_sub_executor(static_cast<Args&&>(args)...);
//     }
//
//     BasicExecutorImpl<PrefixArgs>& get_basic_executor() noexcept
//     {
//         return this->executor;
//     }
//
//     template<class... Args>
//     SubExecutorBuilder2<Executor, Args...> create_nested_executor(Args&&... args)
//     {
//         return this->event.create_nested_executor(static_cast<Args&&>(args)...);
//     }
//
//     template<class... Args>
//     ExecExecutorBuilder2<Executor, Args...> exec_sub_executor(Args&&... args)
//     {
//         auto builder = this->event.create_sub_executor(static_cast<Args&&>(args)...);
//         auto& sub_executor = detail::get_executor(builder);
//         return {sub_executor};
//     }
//
//     template<class... Args>
//     ExecExecutorBuilder2<Executor, Args...> exec_nested_executor(Args&&... args)
//     {
//         auto builder = this->event.create_nested_executor(static_cast<Args&&>(args)...);
//         auto& sub_executor = detail::get_executor(builder);
//         return {sub_executor};
//     }

    template<class F>
    Executor2ActionContext set_exit_action(F f) noexcept
    {
        this->event.set_on_exit(f);
        return *this;
    }
};


template<class PrefixArgs, class... Ts>
struct Executor2Impl : public BasicExecutorImpl<PrefixArgs>
{
    friend Executor2ActionContext<Executor2Impl>;

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = [](BasicExecutorImpl<PrefixArgs>& executor, auto... prefix_args) {
            auto& self = static_cast<Executor2Impl&>(executor);
            return self.ctx.invoke(
                make_lambda<F>(),
                Executor2ActionContext<Executor2Impl>(self),
                static_cast<decltype(prefix_args)&&>(prefix_args)...);
        };
    }

    template<class F>
    void set_on_exit(F) noexcept
    {
        this->on_exit = [](BasicExecutorImpl<PrefixArgs>& executor, auto... prefix_args) {
            auto& self = static_cast<Executor2Impl&>(executor);
            // TODO ExecutorExitContext
            return self.ctx.invoke(
                make_lambda<F>(),
                Executor2ActionContext<Executor2Impl>(self),
                static_cast<decltype(prefix_args)&&>(prefix_args)...);
        };
    }

    template<class... Args>
    SubExecutorBuilder<PrefixArgs, Args...> create_sub_executor(Args&&... args)
    {
        auto* sub_executor = Executor2<PrefixArgs, Args...>::New(this->top_executor_timers, static_cast<Args&&>(args)...);
        this->set_next_executor(*sub_executor);
        return {*sub_executor};
    }

    Executor2Impl(Executor2Impl const&) = delete;
    Executor2Impl& operator=(Executor2Impl const&) = delete;

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class... Args>
    Executor2Impl(TopExecutorTimersImpl<PrefixArgs>& top_executor_timers, Args&&... args)
      : BasicExecutorImpl<PrefixArgs>(top_executor_timers)
      , ctx{static_cast<Args&&>(args)...}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    BasicExecutorImpl<PrefixArgs>& base() noexcept
    {
        return *this;
    }

    template<class... Args>
    static Executor2Impl* New(TopExecutorTimersImpl<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new Executor2Impl(top_executor_timers, static_cast<Args&&>(args)...);
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) {
            delete static_cast<Executor2Impl*>(base);
        };
        return p;
    }

// protected:
    detail::tuple<Ts...> ctx;

private:
    void *operator new(size_t n) { return ::operator new(n); }
};


template<class Executor>
struct REDEMPTION_CXX_NODISCARD Executor2TimeoutContext : Executor2ActionContext<Executor>
{
    using Executor2ActionContext<Executor>::Executor2ActionContext;

    template<class F>
    Executor2ActionContext<Executor> set_timeout_action(F f) noexcept
    {
        auto executor_action = static_cast<Executor2ActionContext<Executor>*>(this);
        detail::get_executor(executor_action)->set_on_timeout(f);
        return *executor_action;
    }

    Executor2TimeoutContext set_timeout(std::chrono::milliseconds ms) noexcept
    {
        auto& executor = static_cast<Executor&>(this->executor);
        // TODO update_timeout
        executor.set_timeout(ms);
        return *this;
    }
};

template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2EventContext : BasicContext<Event>
{
    using BasicContext<Event>::BasicContext;

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Nothing;
    }
};

template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2FdContext : BasicContext<Event>
{
    using BasicContext<Event>::BasicContext;

    ExecutorResult need_more_data() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Ready;
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

    template<class F>
    Executor2FdContext set_exit_action(F f) noexcept
    {
        this->event.set_on_exit(f);
        return *this;
    }
};

template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2FdTimeoutContext : BasicContext<Event>
{
    using BasicContext<Event>::BasicContext;

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Ready;
    }

    template<class F>
    Executor2FdTimeoutContext set_timeout_action(F f) noexcept
    {
        this->event.set_on_timeout(f);
    }

    Executor2FdTimeoutContext set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->event.update_timeout(ms);
        return *this;
    }
};

template<class Base>
struct DeleteSelf
{
    void operator()(Base* p) const
    {
        p->delete_self();
    }
};

template<class Base, class T = Base>
using UniquePtr = std::unique_ptr<T, DeleteSelf<Base>>;
template<class T, class Base>
using UniquePtr2 = std::unique_ptr<T, DeleteSelf<Base>>;

template<class Base>
struct Container
{
    template<class T, class... Args>
    T& emplace_back(Args&&... args)
    {
        auto* p = T::New(static_cast<Args&&>(args)...);
        this->xs.emplace_back(&p->base());
        return *p;
    }

    void remove(Base& base)
    {
        auto it = std::find_if(this->xs.begin(), this->xs.end(), [&base](auto const& p){
            return p.get()== &base;
        });
        std::move(*it) = std::move(this->xs.back());
        this->xs.pop_back();
    }

    std::vector<UniquePtr<Base>> xs;
};

template<class TimerPtr, int Mask = 0>
struct REDEMPTION_CXX_NODISCARD TimerBuilder
{
    template<class F>
    REDEMPTION_CXX_NODISCARD decltype(auto) on_action(F f) && noexcept
    {
        static_assert(!(Mask & 0b01), "on_action already set");
        this->timer_ptr->set_on_action(f);
        return this->select_return<0b01>();
    }

    // TODO add set_time
    REDEMPTION_CXX_NODISCARD decltype(auto) set_delay(std::chrono::milliseconds ms) && noexcept
    {
        static_assert(!(Mask & 0b10), "set_delay already set");
        this->timer_ptr->set_time(ms);
        return this->select_return<0b10>();
    }

    TimerBuilder(TimerPtr&& timer_ptr) noexcept
    : timer_ptr(static_cast<TimerPtr&&>(timer_ptr))
    {}

private:
    template<int Mask2>
    decltype(auto) select_return() noexcept
    {
        if constexpr (Mask == (~Mask2 & 0b11)) {
            return static_cast<TimerPtr&&>(this->timer_ptr);
        }
        else {
            return TimerBuilder<TimerPtr, Mask | Mask2>(std::move(this->timer_ptr));
        }
    }

    TimerPtr timer_ptr;
};

template<class ActionPtr>
struct REDEMPTION_CXX_NODISCARD ActionBuilder
{
    template<class F>
    REDEMPTION_CXX_NODISCARD ActionPtr on_action(F f) && noexcept
    {
        this->action_ptr->set_on_action(f);
        return std::move(this->action_ptr);
    }

    ActionBuilder(ActionPtr&& action_ptr) noexcept
    : action_ptr(static_cast<ActionPtr&&>(action_ptr))
    {}

private:
    ActionPtr action_ptr;
};

template<class FdPtr, int Mask = 0>
struct TopFdBuilder
{
    template<int Mask2>
    decltype(auto) select_return()
    {
        if constexpr (Mask == (~Mask2 & 0b1111)) {
            return std::move(this->fd_ptr);
        }
        else {
            return TopFdBuilder<FdPtr, Mask | Mask2>{std::move(this->fd_ptr)};
        }
    }

    template<class F>
    decltype(auto) on_action(F f) && noexcept
    {
        static_assert(!(Mask & 0b001), "on_action already set");
        this->fd_ptr->set_on_action(f);
        return select_return<0b001>();
    }

    template<class F>
    decltype(auto) on_exit(F f) && noexcept
    {
        static_assert(!(Mask & 0b010), "on_exit already set");
        this->fd_ptr->set_on_exit(f);
        return select_return<0b010>();
    }

    template<class F>
    decltype(auto) on_timeout(F f) && noexcept
    {
        static_assert(!(Mask & 0b100), "on_timeout already set");
        this->fd_ptr->set_on_timeout(f);
        return select_return<0b100>();
    }

    decltype(auto) set_timeout(std::chrono::milliseconds ms) && noexcept
    {
        static_assert(!(Mask & 0b1000), "set_timeout already set");
        this->fd_ptr->set_timeout(ms);
        return select_return<0b1000>();
    }

    TopFdBuilder(FdPtr&& fd_ptr) noexcept
    : fd_ptr(static_cast<FdPtr&&>(fd_ptr))
    {}

private:
    FdPtr fd_ptr;
};


// template<class PrefixArgs>
// struct TopExecutorBase : TopExecutorTimersImpl<PrefixArgs>
// {
//     BasicExecutorImpl<PrefixArgs> base_executor;
//
//     void delete_self()
//     {
//         this->base_executor.delete_self();
//     }
// };
// template<class... Ts>
// struct TimedExecutor : Executor2Impl<Ts...>, BasicTimer
// {
//     BasicExecutorImpl& base() noexcept
//     {
//         return this->executor;
//     }
//
//     template<class... Args>
//     static TimedExecutor* New(TopExecutorBase& top_executor, Args&&... args)
//     {
//         auto* p = new TimedExecutor{top_executor, static_cast<Args&&>(args)...};
//         p->deleter = [](BasicTimer* p) { delete static_cast<TimedExecutor<Ts...>*>(p); };
//         return p;
//     }
// };

template<class F, class T, template<class...> class Ctx>
auto wrap_fn()
{
    return [](auto& e, auto... prefix_args) {
        auto& self = static_cast<T&>(e);
        // TODO ExecutorExitContext
        return self.ctx.invoke(
            make_lambda<F>(),
            Ctx<T>(self),
            static_cast<decltype(prefix_args)&&>(prefix_args)...);
    };
}

class propagate_to_base_t {};

template<
    class Inherit,
    class EventContainerType,
    class BaseType,
    template<class...> class EventCtxArg,
    class... Ts>
struct BasicEvent : BaseType
{
    using executor_context = EventCtxArg<Inherit>;
    using event_container_type = EventContainerType;
    using basic_event = BasicEvent;
    using tuple_context = detail::tuple<Ts...>;

    BasicEvent(BasicEvent const&) = delete;
    BasicEvent& operator=(BasicEvent const&) = delete;

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class Cont, class... Args>
    BasicEvent(Cont&& event_container, Args&&... args)
      : ctx{static_cast<Args&&>(args)...}
      , event_container{static_cast<Cont&&>(event_container)}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class Tuple, class Cont, class... Args>
    BasicEvent(propagate_to_base_t, Tuple&& tuple, Cont&& event_container, Args&&... args)
    : BasicEvent(
        std::make_index_sequence<detail::tuple_size<std::remove_reference_t<Tuple>>::value>(),
        static_cast<Tuple&&>(tuple),
        static_cast<Cont&&>(event_container),
        static_cast<Args&&>(args)...)
    {}
    REDEMPTION_DIAGNOSTIC_POP

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<std::size_t... ints, class Tuple, class Cont, class... Args>
    BasicEvent(std::integer_sequence<size_t, ints...>, Tuple&& tuple, Cont&& event_container, Args&&... args)
      : BaseType{detail::get<ints>(static_cast<Tuple&&>(tuple))...}
      , ctx{static_cast<Args&&>(args)...}
      , event_container{static_cast<Cont&&>(event_container)}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    ~BasicEvent()
    {
        this->detach();
    }

    void detach() noexcept
    {
        this->event_container.detach(*this);
    }

    void attach() noexcept
    {
        this->event_container.attach(*this);
    }

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = wrap_fn<F, Inherit, EventCtxArg>();
    }

    tuple_context ctx;
protected:
    event_container_type event_container;

private:
    void *operator new(size_t n) { return ::operator new(n); }

public:
    void *operator new(size_t n, char const*) { return ::operator new(n); }
};

template<class Event>
struct UniquePtrEventWithUPtr;

namespace detail
{
    template<class Event>
    struct event_with_uptr : Event
    {
        using Event::Event;

        std::unique_ptr<Event, DeleteSelf<typename Event::base_type>>* uptr;
    };

    struct UniquePtrEventWithUPtrAccess
    {
        template<class UEvent>
        static typename UEvent::UniquePtr& p(UEvent& x)
        {
            return x.p;
        }

        template<class UEvent>
        static typename UEvent::UniquePtr**& uptr_in_event(UEvent& x)
        {
            return x.uptr_in_event;
        }
    };
}

template<class Event>
struct UniquePtrEventWithUPtr
{
    UniquePtrEventWithUPtr() noexcept = default;

    UniquePtrEventWithUPtr(detail::event_with_uptr<Event>* e) noexcept
    : p(e)
    , uptr_in_event(&e->uptr)
    {
        *this->uptr_in_event = &p;
    }

    template<class InheritEvent>
    UniquePtrEventWithUPtr(UniquePtrEventWithUPtr<InheritEvent>&& other) noexcept
    : p(std::move(detail::UniquePtrEventWithUPtrAccess::p(other)))
    , uptr_in_event(reinterpret_cast<UniquePtr**>(
        detail::UniquePtrEventWithUPtrAccess::uptr_in_event(other)))
    {
        *this->uptr_in_event = &p;
    }

    template<class InheritEvent>
    UniquePtrEventWithUPtr& operator=(UniquePtrEventWithUPtr<InheritEvent>&& other) noexcept
    {
        this->p = std::move(detail::UniquePtrEventWithUPtrAccess::p(other));
        this->uptr_in_event = reinterpret_cast<UniquePtr**>(
            detail::UniquePtrEventWithUPtrAccess::uptr_in_event(other));
        *this->uptr_in_event = &p;
        return *this;
    }

    explicit operator bool () const noexcept
    {
        return bool(this->p);
    }

    Event* operator->() const noexcept
    {
        return this->p.get();
    }

    Event& operator*() const noexcept
    {
        return *this->p;
    }

    void reset() noexcept
    {
        this->p.reset();
    }

    Event* release() noexcept
    {
        return this->p.release();
    }

private:
    friend detail::UniquePtrEventWithUPtrAccess;
    using UniquePtr = std::unique_ptr<Event, DeleteSelf<typename Event::base_type>>;
    UniquePtr p;
    UniquePtr** uptr_in_event; // pointer on event_with_uptr::uptr
};

template<class Event, class... Args>
static UniquePtrEventWithUPtr<Event>
new_event(Args&&... args)
{
    using NewEvent = detail::event_with_uptr<Event>;
    auto* p = new("") NewEvent(static_cast<Args&&>(args)...);
    p->deleter = [](auto* base) noexcept {
        auto* e = static_cast<NewEvent*>(base);
        e->deleter = [](auto*) noexcept {};
        // jln::make_lambda<DeleteEvent>()(ctx...);
        assert(e->uptr->get() == base);
        e->uptr->release();
        delete e;
    };
    UniquePtrEventWithUPtr<Event> uptr(p);
    p->attach();
    return uptr;
}

template<class EventContainer, class PrefixArgs, class... Ts>
struct TimerImpl : BasicEvent<
    TimerImpl<EventContainer, PrefixArgs, Ts...>,
    EventContainer,
    BasicTimer<PrefixArgs>,
    Executor2TimerContext,
    Ts...>
{
    using TimerImpl::basic_event::basic_event;

    void update_time(std::chrono::milliseconds ms)
    {
        this->set_time(ms);
        this->event_container.update_time(*this, ms);
    }
};

template<class EventContainer, class PrefixArgs, class... Args>
using Timer = TimerImpl<EventContainer, PrefixArgs,
    typename detail::decay_and_strip<Args>::type...>;

template<class EventContainer, class PrefixArgs, class... Ts>
struct ActionImpl : BasicEvent<
    ActionImpl<EventContainer, PrefixArgs, Ts...>,
    EventContainer,
    ActionBase<PrefixArgs>,
    Executor2EventContext,
    Ts...>
{
    using ActionImpl::basic_event::basic_event;
};

template<class EventContainer, class PrefixArgs, class... Args>
using Action = ActionImpl<EventContainer, PrefixArgs,
    typename detail::decay_and_strip<Args>::type...>;

template<class EventContainer, class PrefixArgs, class... Ts>
struct FdImpl : BasicEvent<
    FdImpl<EventContainer, PrefixArgs, Ts...>,
    EventContainer,
    BasicExecutorImpl<PrefixArgs>,
    Executor2FdContext,
    Ts...>
{
    using FdImpl::basic_event::basic_event;
};

template<class EventContainer, class PrefixArgs, class... Args>
using Fd = FdImpl<EventContainer, PrefixArgs,
    typename detail::decay_and_strip<Args>::type...>;


template<class F>
auto one_shot(F) noexcept
{
    return [](auto ctx, auto&&... xs){
        make_lambda<F>()(static_cast<decltype(xs)&&>(xs)...);
        return ctx.terminate();
    };
}

namespace detail
{
    template<auto f, class T, class... Args>
    void invoke(T&& o, Args&&... args)
    {
        if constexpr (std::is_member_function_pointer<decltype(f)>::value) {
            if constexpr (std::is_pointer<std::remove_reference_t<decltype(o)>>::value) {
                assert(o);
                (o->*f)(static_cast<Args&&>(args)...);
            }
            else {
                (o.*f)(static_cast<Args&&>(args)...);
            }
        }
        else {
            f(static_cast<T&&>(o), static_cast<Args&&>(args)...);
        }
    }

    template<auto f>
    void invoke()
    {
        f();
    }
}

template<auto f>
auto one_shot() noexcept
{
    return [](auto ctx, auto&&... xs){
        detail::invoke<f>(static_cast<decltype(xs)&&>(xs)...);
        return ctx.terminate();
    };
}

template<char... cs>
struct string_c
{
    static inline char const value[sizeof...(cs)+1]{cs..., '\0'};

    constexpr char const* c_str() const noexcept { return value; }
};

namespace detail
{
    template<class S, class F>
    struct named_type {};

    template<class S>
    struct named_function
    {
        template<class F>
        named_type<S, F> operator()(F) const noexcept
        {
            return {};
        }
    };
}

namespace literals
{
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
    template<class C, C... cs>
    string_c<cs...> operator ""_c () noexcept
    { return {}; }

    template<class C, C... cs>
    detail::named_function<string_c<cs...>> operator ""_f () noexcept
    { return {}; }
    REDEMPTION_DIAGNOSTIC_POP
}

template<auto x>
constexpr auto value = std::integral_constant<decltype(x), x>{};

namespace detail
{
    struct unamed{};

    template<class i, class S, class F>
    struct indexed_type
    {
        using index = i;
        static F func() noexcept { return jln::make_lambda<F>(); }
        static S name() noexcept { return S{}; }
    };

    template<std::size_t i, class S, class F>
    using indexed_c = indexed_type<std::integral_constant<std::size_t, i>, S, F>;

    template<class i, class S, class F>
    auto value_at(indexed_type<i, S, F> x) noexcept
    {
        return x;
    }

    template<class S, class i, class F>
    auto value_by_name(indexed_type<i, S, F> x) noexcept
    {
        return x;
    }
}

template<class i, class Sequencer, class Ctx>
struct REDEMPTION_CXX_NODISCARD FunSequencerExecutorCtx : Ctx
{
    // ExecutorResult terminate() noexcept
    // {
    //     if constexpr (is_sequencer<Ctx>) {
    //         return Ctx::sequence_next();
    //     }
    //     else {
    //         Ctx::terminate();
    //     }
    // }

    constexpr static bool is_final_sequence() noexcept
    {
        return i::value == Sequencer::sequence_size - 1;
    }

    constexpr static i index() noexcept
    {
        return i();
    }

    constexpr static auto sequence_name() noexcept
    {
        return detail::value_at<i>(Sequencer{}).name();
    }

    jln::ExecutorResult sequence_next() noexcept
    {
        return this->sequence_at<i::value+1>();
    }

    jln::ExecutorResult sequence_previous() noexcept
    {
        return this->sequence_at<i::value-1>();
    }

    template<std::size_t I>
    jln::ExecutorResult sequence_at() noexcept
    {
        return this->next_action(this->get_sequence_at<I>());
    }

    template<class S>
    jln::ExecutorResult sequence_at(S) noexcept
    {
        return this->next_action(this->get_sequence_name<S>());
    }

    template<std::size_t I>
    jln::ExecutorResult exec_sequence_at() noexcept
    {
        return this->exec_action(this->get_sequence_at<I>());
    }

    template<class S>
    jln::ExecutorResult exec_sequence_at(S) noexcept
    {
        return this->exec_action(this->get_sequence_name<S>());
    }

    template<std::size_t I>
    auto get_sequence_at() noexcept
    {
        using index = std::integral_constant<std::size_t, I>;
        using NewSequencer = FunSequencerExecutorCtx<index, Sequencer, Ctx>;
        return [](auto ctx, auto&&... xs){
            return detail::value_at<index>(Sequencer{}).func()(
                static_cast<NewSequencer&>(static_cast<Ctx&>(ctx)),
                static_cast<decltype(xs)&&>(xs)...
            );
        };
    }

    template<class S>
    auto get_sequence_name() noexcept
    {
        return [](auto ctx, auto&&... xs){
            auto x = detail::value_by_name<S>(Sequencer{});
            using index = typename decltype(x)::index;
            using NewSequencer = FunSequencerExecutorCtx<index, Sequencer, Ctx>;
            return x.func()(
                static_cast<NewSequencer&>(static_cast<Ctx&>(ctx)),
                static_cast<decltype(xs)&&>(xs)...
            );
        };
    }


    template<class S>
    auto get_sequence_at(S) noexcept
    {
        return get_sequence_name<S>();
    }

    // Only with Executor2TimerContext
    FunSequencerExecutorCtx set_time(std::chrono::milliseconds ms)
    {
        return static_cast<FunSequencerExecutorCtx&&>(Ctx::set_time(ms));
    }
};

template<class... Fs>
struct FunSequencer : Fs...
{
    static constexpr std::size_t sequence_size = sizeof...(Fs);

    template<class Fn>
    FunSequencer<Fs..., detail::indexed_c<sizeof...(Fs), detail::unamed, Fn>>
    then(Fn) noexcept
    {
        return {};
    }

    template<char... Ch, class Fn>
    FunSequencer<Fs..., detail::indexed_c<sizeof...(Fs), string_c<Ch...>, Fn>>
    then(string_c<Ch...>, Fn) noexcept
    {
        return {};
    }

    template<class Ctx, class... Ts>
    jln::ExecutorResult operator()(Ctx ctx, Ts&&... xs)
    {
        using i = std::integral_constant<std::size_t, 0>;
        using NewCtx = FunSequencerExecutorCtx<i, FunSequencer, Ctx>;
        return detail::value_at<i>(*this).func()(
            static_cast<NewCtx&>(ctx),
            static_cast<Ts&&>(xs)...
        );
    }
};

namespace detail
{
    template<class i, class F>
    struct function_to_indexed_type
    {
        using type = indexed_type<i, unamed, F>;
    };

    template<class i, class S, class F>
    struct function_to_indexed_type<i, named_type<S, F>>
    {
        using type = indexed_type<i, S, F>;
    };

    template<class, class...>
    struct cases_to_fun_sequencer;

    template<std::size_t... Ints, class... Fs>
    struct cases_to_fun_sequencer<std::integer_sequence<std::size_t, Ints...>, Fs...>
    {
        using type = FunSequencer<typename function_to_indexed_type<
            std::integral_constant<std::size_t, Ints>,  Fs>::type...>;
    };
}

template<class S, class F>
detail::named_type<S, F> named(S, F)
{ return {}; }

template<class... Fs>
inline auto funcsequencer(Fs...) noexcept
{
    return typename detail::cases_to_fun_sequencer<
        std::index_sequence_for<Fs...>, Fs...>::type{};
}


namespace detail
{
    template<class Before, class T>
    constexpr std::size_t get_offset = sizeof(Before) + sizeof(Before) % alignof(T);

    template<class Real, class Before = Real, class T>
    Real& get_before(T& x)
    {
        return *reinterpret_cast<Real*>(reinterpret_cast<uint8_t*>(&x) - get_offset<Before, T>);
    }
}

template<class PrefixArgs, class... Ts>
struct TopExecutorImpl : Executor2Impl<PrefixArgs, Ts...>
{
    using Executor2Impl<PrefixArgs, Ts...>::Executor2Impl;

//     template<class F>
//     void set_on_timeout(F) noexcept
//     {
//         this->timeout.on_timer = [](BasicTimer<PrefixArgs>& timer, auto... prefix_args) {
//             auto& timer_mem = static_cast<Timer2Impl<PrefixArgs>&>(timer);
//             auto& self = detail::get_before<TopExecutorImpl, Executor2Impl<PrefixArgs, Ts...>>(timer_mem);
//             // TODO ExecutorTimeoutContext
//             return self.ctx.invoke(
//                 make_lambda<F>(),
//                 Executor2TimeoutContext<PrefixArgs, Ts...>(self),
//                 static_cast<decltype(prefix_args)&&>(prefix_args)...);
//         };
//     }

    void set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->timeout.set_time(ms);
    }

    TopExecutorImpl(TopExecutorImpl const&) = delete;
    TopExecutorImpl& operator=(TopExecutorImpl const&) = delete;

    template<class... Args>
    TopExecutorImpl(TopExecutorTimersImpl<PrefixArgs>& top_executor_timers, Args&&... args)
      : Executor2Impl<PrefixArgs, Ts...>(top_executor_timers, static_cast<Args&&>(args)...)
      , timeout(top_executor_timers)
    {
        top_executor_timers.add_timer(this->timeout);
    }

    template<class... Args>
    static TopExecutorImpl* New(TopExecutorTimersImpl<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new TopExecutorImpl{top_executor_timers, static_cast<Args&&>(args)...};
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) noexcept {
            delete static_cast<TopExecutorImpl*>(base);
        };
        return p;
    }

private:
    void *operator new(size_t n) { return ::operator new(n); }

    Timer2Impl<PrefixArgs> timeout;
};

template<class Data>
struct DataExecutor
{
    Data data;

    template<std::size_t... ints, class... Args>
    DataExecutor(
        detail::tuple_impl<std::integer_sequence<size_t, ints...>, Args...> data_arg)
      : data{static_cast<detail::tuple_elem<ints, Args>&>(data_arg).x.get()...}
    {}

    template<class PrefixArgs>
    static Data& get_data_from(BasicExecutorImpl<PrefixArgs>& executor)
    {
        return detail::get_before<DataExecutor<Data>>(executor).data;
    }
};

template<class Data, class PrefixArgs, class... Ts>
struct TopExecutorWithDataImpl : DataExecutor<Data>, TopExecutorImpl<PrefixArgs, Ts...>
{
    TopExecutorWithDataImpl(TopExecutorWithDataImpl const&) = delete;
    TopExecutorWithDataImpl& operator=(TopExecutorWithDataImpl const&) = delete;

    template<class DataArgs, class... Args>
    TopExecutorWithDataImpl(
        DataArgs data_arg,
        TopExecutorTimersImpl<PrefixArgs>& top_executor_timers,
        Args&&... args)
      : DataExecutor<Data>{data_arg}
      , TopExecutorImpl<PrefixArgs, Ts...>(top_executor_timers, static_cast<Args&&>(args)...)
    {}

    template<class DataArgs, class... Args>
    static TopExecutorWithDataImpl* New(DataArgs data_args, TopExecutorTimersImpl<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new TopExecutorWithDataImpl{data_args, top_executor_timers, static_cast<Args&&>(args)...};
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) noexcept {
            delete static_cast<TopExecutorWithDataImpl*>(base);
        };
        return p;
    }

private:
    void *operator new(size_t n) { return ::operator new(n); }
};

template<class DataType, class PrefixArgs, class... Args>
using TopExecutorWithData =
    TopExecutorWithDataImpl<DataType, PrefixArgs, typename detail::decay_and_strip<Args>::type...>;

template<class DataType, class PrefixArgs, class... Args>
using TopExecutorWithDataBuilder = detail::ExecutorBuilder<
    TopExecutorWithData<DataType, PrefixArgs, Args...>, detail::ExecutorType::Normal>;

template<class... Ts>
class Reactor;

namespace detail
{
    template<class T>
    struct data_arg
    {
        T& x;
        T&& get() { return static_cast<T&&>(x); }
    };

    template<class DataType, class DataArgsExecutor, class... Ts>
    struct REDEMPTION_CXX_NODISCARD BuilderDataExecutor
    {
        template<class... Args>
        REDEMPTION_CXX_NODISCARD
        TopExecutorWithDataBuilder<DataType, prefix_args<Ts...>, Args...>
        create_executor(int /*fd*/, Args&&... args)
        {
            return {
                this->reactor.executors.template emplace_back<
                    TopExecutorWithData<DataType, prefix_args<Ts...>, Args...>
                >(this->data_args, this->reactor.timers, static_cast<Args&&>(args)...)
            };
        }

        DataArgsExecutor data_args;
        Reactor<Ts...>& reactor;
    };
}

template<class... Ts>
struct Reactor
{
    template<class... Args>
    TopExecutorBuilder<prefix_args<Ts...>, Args...>
    create_executor(int /*fd*/, Args&&... args)
    {
        return {
            this->executors.template emplace_back<
                TopExecutor2<prefix_args<Ts...>, Args...>
            >(this->timers, static_cast<Args&&>(args)...)
        };
    }

    template<class DataType, class... Args>
    auto set_data_executor(Args&&... args)
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        return detail::BuilderDataExecutor<
            DataType,
            detail::tuple<detail::data_arg<Args>...>,
            Ts...
        >{{detail::data_arg<Args>{args}...}, *this};
        REDEMPTION_DIAGNOSTIC_POP
    }

    template<class Data>
    auto set_data_executor(Data&& data)
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        return detail::BuilderDataExecutor<
            Data,
            detail::tuple<detail::data_arg<Data>>,
            Ts...
        >{{detail::data_arg<Data>{data}}, *this};
        REDEMPTION_DIAGNOSTIC_POP
    }

    template<class Data, class... Us>
    auto set_data_executor(detail::emplace_type<Data, Us...> e)
    {
        return detail::BuilderDataExecutor<
            Data,
            detail::tuple<Us...>,
            Ts...
        >{e.t, *this};
    }

// private:
    Container<BasicExecutorImpl<prefix_args<Ts...>>> executors;
    TopExecutorTimers<Ts...> timers;
};


namespace detail
{
    template<class... Ts>
    ExecutorResult terminate_callee(Ts...)
    {
        assert("call a executor marked 'Terminate'");
        return ExecutorResult::Terminate;
    }
}

template<class PrefixArgs>
template<class... Args>
bool BasicExecutorImpl<PrefixArgs>::exec(Args&&... args)
{
    switch (this->current->exec_action(static_cast<Args&&>(args)...)) {
        case ExecutorResult::ExitSuccess:
            return this->exit_with(ExecutorError::NoError, static_cast<Args&&>(args)...);
        case ExecutorResult::ExitFailure:
            return this->exit_with(ExecutorError::ActionError, static_cast<Args&&>(args)...);
        case ExecutorResult::Terminate:
            this->terminate(static_cast<Args&&>(args)...);
            return false;
            break;
        case ExecutorResult::Nothing:
        case ExecutorResult::NeedMoreData:
        case ExecutorResult::Ready:
            break;
    }

    return this->current;
}

template<class PrefixArgs>
template<class... Args>
void BasicExecutorImpl<PrefixArgs>::terminate(Args&&... args)
{
    while (this->current != this) {
        (void)this->current->exec_exit(ExecutorError::Terminate, static_cast<Args&&>(args)...);
        std::exchange(this->current, this->current->prev)->delete_self();
    }
    (void)this->current->exec_exit(ExecutorError::Terminate, static_cast<Args&&>(args)...);
    this->on_action = detail::terminate_callee;
    this->on_exit = detail::terminate_callee;
    //TODO this->on_timeout = detail::terminate_callee;
}

template<class PrefixArgs>
template<class... Args>
bool BasicExecutorImpl<PrefixArgs>::exit_with(ExecutorError error, Args&&... args)
{
    do {
        switch (this->current->exec_exit(error, static_cast<Args&&>(args)...)) {
            case ExecutorResult::ExitSuccess:
                if (this->current == this) {
                    this->on_action = detail::terminate_callee;
                    this->on_exit = detail::terminate_callee;
                    return false;
                }
                std::exchange(this->current, this->current->prev)->delete_self();
                error = ExecutorError::NoError;
                break;
            case ExecutorResult::ExitFailure:
                if (this->current == this) {
                    this->on_action = detail::terminate_callee;
                    this->on_exit = detail::terminate_callee;
                    return false;
                }
                std::exchange(this->current, this->current->prev)->delete_self();
                error = ExecutorError::ActionError;
                break;
            case ExecutorResult::Terminate:
                this->terminate(static_cast<Args&&>(args)...);
                return false;
                break;
            case ExecutorResult::Nothing:
            case ExecutorResult::NeedMoreData:
            case ExecutorResult::Ready:
                return true;
        }
    } while (this->current);
    return false;
}

REDEMPTION_OSTREAM(out, ExecutorError e) { return out << int(e); }
REDEMPTION_OSTREAM(out, ExecutorResult e) { return out << int(e); }

}
