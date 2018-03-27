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
            return emplace_type<T, Us&&...>{{static_cast<Us&&>(xs)...}};
            REDEMPTION_DIAGNOSTIC_POP
        }
    };

# define FALCON_RETURN_NOEXCEPT(expr)        \
    noexcept(noexcept(decltype(expr)(expr))) \
    { return expr; }

    template<size_t, class T>
    struct tuple_elem
    {
        T x;

        template<std::size_t... ints, class... Ts>
        constexpr tuple_elem(int, tuple_impl<std::integer_sequence<size_t, ints...>, Ts...>& t)
        noexcept(noexcept(T{static_cast<Ts&&>(static_cast<tuple_elem<ints, Ts>&>(t).x)...}))
          : x{static_cast<Ts&&>(static_cast<tuple_elem<ints, Ts>&>(t).x)...}
        {}

        template<class... Ts>
        constexpr tuple_elem(emplace_type<T, Ts...> e)
        noexcept(noexcept(tuple_elem(1, e.t)))
          : tuple_elem(1, e.t)
        {}

        template<class U>
        constexpr tuple_elem(U&& x)
        noexcept(noexcept(T(static_cast<U&&>(x))))
          : x(static_cast<U&&>(x))
        {}
    };

    template<std::size_t... ints, class... Ts>
    struct tuple_impl<std::integer_sequence<size_t, ints...>, Ts...>
    : tuple_elem<ints, Ts>...
    {
        template<class F, class... Args>
        decltype(auto) invoke(F && f, Args&&... args)
        noexcept(noexcept(f(
            static_cast<Args&&>(args)...,
            static_cast<tuple_elem<ints, Ts>*>(nullptr)->x...)))
        {
            return f(
                static_cast<Args&&>(args)...,
                static_cast<tuple_elem<ints, Ts>&>(*this).x...
            );
        }

        template<class F, class T, class... Args>
        decltype(auto) invoke_fix(F && f, T&& x, Args&&... args)
        noexcept(noexcept(f(
            static_cast<T&&>(x),
            static_cast<tuple_elem<ints, Ts>*>(nullptr)->x...,
            static_cast<Args&&>(args)...)))
        {
            return f(
                static_cast<T&&>(x),
                static_cast<tuple_elem<ints, Ts>&>(*this).x...,
                static_cast<Args&&>(args)...
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

    template<class T>
    using decay_and_strip_t = typename decay_and_strip<T>::type;
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
    CreateGroup,
    Next,
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

    ExecutorResult retry();
    ExecutorResult retry_until(std::chrono::milliseconds ms);
    ExecutorTimerContextConcept_ set_delay(std::chrono::milliseconds ms);
    ExecutorTimerContextConcept_ set_time(timeval const& tv);
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

    template<class T> using ident = T;
}

template<class BaseType, class PrefixArgs, class...Ts>
using MakeFuncPtr = typename detail::make_func_ptr<BaseType, PrefixArgs, Ts...>::type;

struct default_action_function
{
    template<class R, class... Args>
    operator detail::ident<R(*)(Args...)> () noexcept
    {
        return [](Args...) -> R { return jln::ExecutorResult::Nothing; };
    }
};

struct default_delete_function
{
    template<class... Args>
    operator detail::ident<void(*)(Args...)noexcept> () noexcept
    {
        return [](Args...) noexcept -> void {};
    }
};

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

enum class DeleteFrom : bool
{
    Owner,
    Observer,
};


template<class PrefixArgs>
struct ActionBase
{
    using prefix_args = PrefixArgs;
    using base_type = ActionBase;

    template<class... Args>
    ExecutorResult exec_action(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    using OnEventFuncPtr = MakeFuncPtr<ActionBase, PrefixArgs>;

    OnEventFuncPtr on_action = default_action_function();
    ActionBase() = default;
};

template<class PrefixArgs>
struct BasicTimer
{
    using prefix_args = PrefixArgs;
    using base_type = BasicTimer;

    timeval time() const noexcept
    {
        return this->tv;
    }

    void set_delay(std::chrono::milliseconds ms) noexcept
    {
        this->delay = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    }

    void set_time(timeval const& tv) noexcept
    {
        this->tv = tv;
    }

    template<class... Args>
    ExecutorResult exec_action(Args&&... args)
    {
        return this->on_action(*this, static_cast<Args&&>(args)...);
    }

    using OnTimerFuncPtr = MakeFuncPtr<BasicTimer&, PrefixArgs>;

    timeval tv {};
    OnTimerFuncPtr on_action = default_action_function();
    std::chrono::microseconds delay = std::chrono::microseconds(-1);

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
    using OnActionFuncPtr = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs>;
    using OnExitFuncPtr = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs, ExecutorError>;

    OnActionFuncPtr on_action = default_action_function();
    OnExitFuncPtr on_exit = default_action_function();
    BasicExecutorImpl* current = this;
    BasicExecutorImpl* prev = nullptr;

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

template<class Event, class Ctx>
struct REDEMPTION_CXX_NODISCARD BasicContext
{
    template<class OtherEvent, class Ctx2>
    BasicContext(BasicContext<OtherEvent, Ctx2> const& other) noexcept
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

    template<class F1, class F2, class... Args>
    ExecutorResult exec_action2(F1 f1, F2 f2, Args&&... args)
    {
        this->event.set_on_action(f1);
        return this->event.ctx.invoke(f2, Ctx{this->event}, static_cast<Args&&>(args)...);
    }

    template<class F, class... Args>
    ExecutorResult exec_action(F f, Args&&... args)
    {
        return this->exec_action2(f, f, static_cast<Args&&>(args)...);
    }

    timeval get_current_time() const noexcept
    {
        return this->event.get_reactor().get_current_time();
    }

    decltype(auto) get_reactor() noexcept
    {
        return this->event.get_reactor();
    }

    friend detail::GetExecutor;

protected:
    Event& event;

    using context_base = BasicContext;
};

template<class Timer>
struct REDEMPTION_CXX_NODISCARD Executor2TimerContext
  : BasicContext<Timer, Executor2TimerContext<Timer>>
{
    using CtxBase = typename Executor2TimerContext::context_base;
    using CtxBase::CtxBase;

    ExecutorResult ready() noexcept
    {
        this->event.update_next_time();
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready_to(std::chrono::milliseconds ms)
    {
        this->event.update_delay(ms);
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready_at(timeval const& tv)
    {
        this->event.update_time(tv);
        return ExecutorResult::Nothing;
    }

    CtxBase set_delay(std::chrono::milliseconds ms)
    {
        this->event.update_delay(ms);
        return *this;
    }

    CtxBase set_time(timeval const& tv)
    {
        this->event.update_time(tv);
        return *this;
    }
};


template<class Executor>
struct REDEMPTION_CXX_NODISCARD Executor2ActionContext
  : BasicContext<Executor, Executor2ActionContext<Executor>>
{
    using CtxBase = typename Executor2ActionContext::context_base;
    using CtxBase::CtxBase;

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

    // TODO exeit_with_error
    ExecutorResult exit_on_error() noexcept
    {
        return ExecutorResult::ExitFailure;
    }

    // TODO exeit_with_success
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
    noexcept(noexcept(detail::tuple<Ts...>{static_cast<Args&&>(args)...}))
      : BasicExecutorImpl<PrefixArgs>(top_executor_timers)
      , ctx{static_cast<Args&&>(args)...}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    BasicExecutorImpl<PrefixArgs>& base() noexcept
    {
        return *this;
    }

// protected:
    detail::tuple<Ts...> ctx;
};


template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2EventContext
  : BasicContext<Event, Executor2EventContext<Event>>
{
    using Executor2EventContext::context_base::context_base;

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Nothing;
    }
};

template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2FdContext
  : BasicContext<Event, Executor2FdContext<Event>>
{
    using Executor2FdContext::context_base::context_base;

    ExecutorResult need_more_data() noexcept
    {
        return ExecutorResult::NeedMoreData;
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

    void set_fd(int fd) noexcept
    {
        this->event.set_fd(fd);
    }

    int get_fd() const noexcept
    {
        return this->event.get_fd();
    }

    Executor2FdContext disable_timeout()
    {
        this->event.disable_timeout();
        return *this;
    }

    template<class... Args>
    auto create_sub_executor(Args&&... args)
    {
        return this->event.create_sub_executor(static_cast<Args&&>(args)...);
    }

    template<class Sub>
    auto add_sub_executor(Sub&& sub)
    {
        return this->event.add_sub_executor(static_cast<Sub&&>(sub));
    }
};

template<class Event>
struct REDEMPTION_CXX_NODISCARD Executor2FdTimeoutContext
  : BasicContext<Event, Executor2FdTimeoutContext<Event>>
{
    using Executor2FdTimeoutContext::context_base::context_base;

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Ready;
    }

    template<class F>
    Executor2FdTimeoutContext set_timeout_action(F f) noexcept
    {
        this->event.set_on_timeout(f);
        return *this;
    }

    Executor2FdTimeoutContext set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->event.update_timeout(ms);
        return *this;
    }

    Executor2FdTimeoutContext disable_timeout()
    {
        this->event.disable_timeout();
        return *this;
    }

    template<class F>
    Executor2FdTimeoutContext set_or_disable_timeout(std::chrono::milliseconds ms, F f) noexcept
    {
        return ms.count()
            ? this->set_timeout(ms).set_timeout_action(f)
            : this->disable_timeout();
    }

    template<class... Args>
    auto create_sub_executor(Args&&... args)
    {
        return this->event.create_sub_executor(static_cast<Args&&>(args)...);
    }

    template<class Sub>
    auto add_sub_executor(Sub&& sub)
    {
        return this->event.add_sub_executor(static_cast<Sub&&>(sub));
    }
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

    REDEMPTION_CXX_NODISCARD decltype(auto) set_delay(std::chrono::milliseconds ms) && noexcept
    {
        static_assert(!(Mask & 0b10), "set_time/set_delay already set");
        this->timer_ptr->set_delay(ms);
        return this->select_return<0b10>();
    }

    REDEMPTION_CXX_NODISCARD decltype(auto) set_time(timeval const& tv) && noexcept
    {
        static_assert(!(Mask & 0b10), "set_time/set_delay already set");
        this->timer_ptr->set_time(tv);
        return this->select_return<0b10>();
    }

    TimerBuilder(TimerPtr&& timer_ptr) noexcept
    : timer_ptr(std::move(timer_ptr))
    {}

protected:
    TimerPtr& internal_value() noexcept { return this->timer_ptr; }

private:
    template<int Mask2>
    decltype(auto) select_return() noexcept
    {
        if constexpr (Mask == (~Mask2 & 0b11)) {
            return std::move(this->timer_ptr);
        }
        else {
            return TimerBuilder<TimerPtr, Mask | Mask2>(std::move(this->timer_ptr));
        }
    }

    TimerPtr timer_ptr;
};

template<class ActionPtr, int = 0>
struct REDEMPTION_CXX_NODISCARD ActionBuilder
{
    template<class F>
    REDEMPTION_CXX_NODISCARD ActionPtr on_action(F f) && noexcept
    {
        this->action_ptr->set_on_action(f);
        return std::move(this->action_ptr);
    }

    ActionBuilder(ActionPtr&& action_ptr) noexcept
    : action_ptr(std::move(action_ptr))
    {}

protected:
    ActionPtr& internal_value() noexcept { return this->action_ptr; }

private:
    ActionPtr action_ptr;
};

template<class FdPtr, int Mask = 0>
struct TopFdBuilder
{
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
    : fd_ptr(std::move(fd_ptr))
    {}

protected:
    FdPtr& internal_value() noexcept { return this->fd_ptr; }

private:
    template<int Mask2>
    decltype(auto) select_return() noexcept
    {
        if constexpr (Mask == (~Mask2 & 0b1111)) {
            return std::move(this->fd_ptr);
        }
        else {
            return TopFdBuilder<FdPtr, Mask | Mask2>{std::move(this->fd_ptr)};
        }
    }

    FdPtr fd_ptr;
};

#ifdef __clang__
template<class IsInvocable, bool = IsInvocable::value>
struct CheckProto{};

template<class IsInvocable>
struct CheckProto<IsInvocable, false>
{
    static_assert(IsInvocable::value);
};
#endif

template<class F, class T, template<class...> class Ctx>
struct wrap_fn
{
    template<class R, class E, class... Args>
    operator detail::ident<R(*)(E, Args...)> () noexcept
#ifdef IN_IDE_PARSER
    ;
#else
    {
#ifdef __clang__
        struct Context : Ctx<T> {};
        auto lbd = [](auto&&... args){
            return std::is_invocable_r<R, F, Context, Args..., decltype(args)...>{};
        };
        (void)CheckProto<decltype(static_cast<decltype(T::ctx)*>(nullptr)->invoke(lbd))>{};
#endif
        return [](E& e, Args... prefix_args) -> R {
            auto& self = static_cast<T&>(e);
            // TODO ExecutorExitContext
            return self.ctx.invoke(
                make_lambda<F>(),
                Ctx<T>(self),
                static_cast<decltype(prefix_args)&&>(prefix_args)...);
        };
    }
#endif
};

class propagate_to_base_t {};

template<class EventContainer, class BaseType, class... Ts>
struct ContextedEvent : BaseType
{
    using contexted_event = ContextedEvent;
    using tuple_context = detail::tuple<Ts...>;

    ContextedEvent(ContextedEvent const&) = delete;
    ContextedEvent& operator=(ContextedEvent const&) = delete;

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class Cont, class... Args>
    ContextedEvent(Cont&& event_container, Args&&... args)
    noexcept(
        noexcept(tuple_context{static_cast<Args&&>(args)...})
     && noexcept(EventContainer{static_cast<Cont&&>(event_container)}))
    : ctx{static_cast<Args&&>(args)...}
    , event_container{static_cast<Cont&&>(event_container)}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class Tuple, class Cont, class... Args>
    ContextedEvent(propagate_to_base_t, Tuple&& tuple, Cont&& event_container, Args&&... args)
    noexcept(noexcept(ContextedEvent(
        std::make_index_sequence<detail::tuple_size<std::remove_reference_t<Tuple>>::value>(),
        static_cast<Tuple&&>(tuple),
        static_cast<Cont&&>(event_container),
        static_cast<Args&&>(args)...)))
    : ContextedEvent(
        std::make_index_sequence<detail::tuple_size<std::remove_reference_t<Tuple>>::value>(),
        static_cast<Tuple&&>(tuple),
        static_cast<Cont&&>(event_container),
        static_cast<Args&&>(args)...)
    {}
    REDEMPTION_DIAGNOSTIC_POP

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<std::size_t... ints, class Tuple, class Cont, class... Args>
    ContextedEvent(std::integer_sequence<size_t, ints...>, Tuple&& tuple, Cont&& event_container, Args&&... args)
    noexcept(
        noexcept(BaseType{detail::get<ints>(static_cast<Tuple&&>(tuple))...})
     && noexcept(tuple_context{static_cast<Args&&>(args)...})
     && noexcept(EventContainer{static_cast<Cont&&>(event_container)}))
      : BaseType{detail::get<ints>(static_cast<Tuple&&>(tuple))...}
      , ctx{static_cast<Args&&>(args)...}
      , event_container{static_cast<Cont&&>(event_container)}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    decltype(auto) get_reactor() const noexcept
    {
        return this->event_container.get_reactor();
    }

    tuple_context ctx;
protected:
    EventContainer event_container;
};


template<class EventContainer, class PrefixArgs, class... Ts>
struct TimerImpl : ContextedEvent<EventContainer, BasicTimer<PrefixArgs>, Ts...>
{
    using TimerImpl::contexted_event::contexted_event;

    void set_delay(std::chrono::milliseconds ms) noexcept
    {
        TimerImpl::contexted_event::set_delay(ms);
        TimerImpl::contexted_event::set_time(addusectimeval(
            this->delay, this->get_reactor().get_current_time()));
    }

    void set_time(timeval const& tv) noexcept
    {
        TimerImpl::contexted_event::set_delay(std::chrono::milliseconds(-1));
        TimerImpl::contexted_event::set_time(tv);
    }

    void update_delay(std::chrono::milliseconds ms)
    {
        this->set_delay(ms);
        this->event_container.update_delay(*this, ms);
    }

    void update_time(timeval const& tv)
    {
        this->set_time(tv);
        this->event_container.update_time(*this, tv);
    }

    void update_next_time() noexcept
    {
        assert(this->delay.count() > 0);
        this->tv = addusectimeval(this->delay, this->get_reactor().get_current_time());
    }

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = wrap_fn<F, TimerImpl, Executor2TimerContext>();
    }
};

template<class EventContainer, class PrefixArgs, class... Args>
using Timer = TimerImpl<EventContainer, PrefixArgs,
    typename detail::decay_and_strip<Args>::type...>;

template<class EventContainer, class PrefixArgs, class... Ts>
struct ActionImpl : ContextedEvent<EventContainer, ActionBase<PrefixArgs>, Ts...>
{
    using ActionImpl::contexted_event::contexted_event;

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = wrap_fn<F, ActionImpl, Executor2EventContext>();
    }
};

template<class EventContainer, class PrefixArgs, class... Args>
using Action = ActionImpl<EventContainer, PrefixArgs,
    typename detail::decay_and_strip<Args>::type...>;

template<class EventContainer, class PrefixArgs, class... Ts>
struct FdImpl : ContextedEvent<EventContainer, BasicExecutorImpl<PrefixArgs>, Ts...>
{
    using FdImpl::contexted_event::contexted_event;

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = wrap_fn<F, FdImpl, Executor2FdContext>();
    }
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

inline auto always_ready() noexcept
{
    return [](auto ctx, [[maybe_unused]] auto&&... xs){
        return ctx.ready();
    };
}

inline auto exit_with_success() noexcept
{
    return [](auto ctx, [[maybe_unused]] auto&&... xs){
        return ctx.exit_on_success();
    };
}

inline auto exit_with_error() noexcept
{
    return [](auto ctx, [[maybe_unused]] auto&&... xs){
        return ctx.exit_on_error();
    };
}

template<class F>
auto always_ready(F) noexcept
{
    return [](auto ctx, auto&&... xs){
        make_lambda<F>()(static_cast<decltype(xs)&&>(xs)...);
        return ctx.ready();
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
        return ctx.ready();
    };
}

template<auto f>
auto always_ready() noexcept
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

    static constexpr char const* c_str() noexcept { return value; }
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

        template<class F>
        named_type<S, F> operator=(F) const noexcept
        {
            return {};
        }

        named_function& operator=(named_function const&) const noexcept
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
    string_c<cs...> operator ""_s () noexcept
    { return {}; }

    template<class C, C... cs>
    detail::named_function<string_c<cs...>> operator ""_f () noexcept
    { return {}; }
    REDEMPTION_DIAGNOSTIC_POP
}

template<auto x>
constexpr auto value = std::integral_constant<decltype(x), x>{};

template<class I, class Sequencer, class Ctx>
struct REDEMPTION_CXX_NODISCARD FunSequencerExecutorCtx;

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

    template<class Sequencer, class Ctx, class IndexedType>
    auto create_sequence_ctx(IndexedType) noexcept
    {
        return [](auto ctx, auto&&... xs){
            auto x = IndexedType{};
            using index = typename IndexedType::index;
            using NewSequencer = FunSequencerExecutorCtx<index, Sequencer, Ctx>;
            return x.func()(
                static_cast<NewSequencer&>(static_cast<Ctx&>(ctx)),
                static_cast<decltype(xs)&&>(xs)...
            );
        };
    }

    template<class> struct name_or_index;

    template<class i, class S, class F>
    struct name_or_index<indexed_type<i, S, F>>
    { using type = S; };

    template<class i, class F>
    struct name_or_index<indexed_type<i, unamed, F>>
    { using type = i; };

    template<class... Ts>
    struct CheckUniqueName : name_or_index<Ts>::type...
    {
        static const bool value = true;
    };
}

template<class I, class Sequencer, class Ctx>
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

    ExecutorResult sequence_next_or_terminate() noexcept
    {
        if constexpr (I::value == Sequencer::sequence_size - 1) {
            return this->terminate();
        }
        else {
            return this->sequence_next();
        }
    }

    constexpr static bool is_final_sequence() noexcept
    {
        return I::value == Sequencer::sequence_size - 1;
    }

    constexpr static I index() noexcept
    {
        return I{};
    }

    constexpr static auto sequence_name() noexcept
    {
        return detail::value_at<I>(Sequencer{}).name();
    }

    ExecutorResult sequence_next() noexcept
    {
        return this->sequence_at<I::value+1>();
    }

    ExecutorResult sequence_previous() noexcept
    {
        return this->sequence_at<I::value-1>();
    }

    template<std::size_t i>
    ExecutorResult sequence_at() noexcept
    {
        return this->next_action(this->get_sequence_at<i>());
    }

    template<class S>
    ExecutorResult sequence_at(S) noexcept
    {
        return this->next_action(this->get_sequence_name<S>());
    }

    template<std::size_t i, class... Args>
    ExecutorResult exec_sequence_at(Args&&... args) noexcept
    {
        return this->exec_action(this->get_sequence_at<i>(), static_cast<Args&&>(args)...);
    }

    template<class S, class... Args>
    ExecutorResult exec_sequence_at(S, Args&&... args) noexcept
    {
        return this->exec_action(this->get_sequence_name<S>(), static_cast<Args&&>(args)...);
    }

    template<std::size_t i>
    auto get_sequence_at() noexcept
    {
        using index = std::integral_constant<std::size_t, i>;
        return detail::create_sequence_ctx<Sequencer, Ctx>(detail::value_at<index>(Sequencer{}));
    }

    template<class S>
    auto get_sequence_name() noexcept
    {
        return detail::create_sequence_ctx<Sequencer, Ctx>(detail::value_by_name<S>(Sequencer{}));
    }


    template<class S>
    auto get_sequence_at(S) noexcept
    {
        return get_sequence_name<S>();
    }

    // Only with Executor2TimerContext
    FunSequencerExecutorCtx set_delay(std::chrono::milliseconds ms)
    {
        return static_cast<FunSequencerExecutorCtx&&>(Ctx::set_delay(ms));
    }

    // Only with Executor2TimerContext
    FunSequencerExecutorCtx set_time(timeval const& tv)
    {
        return static_cast<FunSequencerExecutorCtx&&>(Ctx::set_time(tv));
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
        static_assert(detail::CheckUniqueName<Fs...>::value, "name duplicated");

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
        std::exchange(this->current, this->current->prev)/*->delete_self(DeleteFrom::Owner)*/;
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
                std::exchange(this->current, this->current->prev)/*->delete_self(DeleteFrom::Owner)*/;
                error = ExecutorError::NoError;
                break;
            case ExecutorResult::ExitFailure:
                if (this->current == this) {
                    this->on_action = detail::terminate_callee;
                    this->on_exit = detail::terminate_callee;
                    return false;
                }
                std::exchange(this->current, this->current->prev)/*->delete_self(DeleteFrom::Owner)*/;
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
