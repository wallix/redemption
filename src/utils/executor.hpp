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
#include <memory>

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
        auto operator()(Us&&... xs)
        {
            static_assert(0 == sizeof...(Ts));
            return emplace_type<T, Us&&...>{{xs...}};
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

constexpr auto default_function() noexcept
{
    return []([[maybe_unused]] auto... args){
        return ExecutorResult::Nothing;
    };
}

class ExecutorBase;
class Executor;
template<class PrefixArgs, class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2ActionContext;

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

template<class PrefixArgs>
class TopExecutorTimers;

template<class PrefixArgs>
struct BasicExecutorImpl
{
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

    BasicExecutorImpl(TopExecutorTimers<PrefixArgs>& top_executor_timers) noexcept
    : top_executor_timers(top_executor_timers)
    {}

protected:
    using OnActionPtrFunc = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs>;
    using OnExitPtrFunc = MakeFuncPtr<BasicExecutorImpl&, PrefixArgs, ExecutorError>;

    OnActionPtrFunc on_action = default_function();
    OnExitPtrFunc on_exit = default_function();
    BasicExecutorImpl* current = this;
    BasicExecutorImpl* prev = nullptr;
    TopExecutorTimers<PrefixArgs>& top_executor_timers;
    void (*deleter) (BasicExecutorImpl*) = [](BasicExecutorImpl*){};

    void set_next_executor(BasicExecutorImpl& other) noexcept
    {
        other.current = this->current;
        other.prev = this;
        this->current->current = &other;
    }

    BasicExecutorImpl() = default;
};

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
template<class PrefixArgs>
class TopExecutorBase;

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

template<class PrefixArgs, class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2TimerContext
{
    template<class... PreviousTs>
    Executor2TimerContext(Executor2TimerContext<PrefixArgs, PreviousTs...> const& other) noexcept
      : timer(reinterpret_cast<Timer2Impl<PrefixArgs, Ts...>&>(detail::get_executor(other)))
    {
        // TODO strip arguments support (PreviousTs=(int, int), Ts=(int))
        static_assert((true && ... && detail::check_is_context_arg_convertible<PreviousTs, Ts>::value));
        static_assert(sizeof(Timer2Impl<PrefixArgs, Ts...>) == sizeof(detail::get_executor(other)));
    }

    explicit Executor2TimerContext(Timer2Impl<PrefixArgs, Ts...>& timer) noexcept
      : timer{timer}
    {}

    Executor2TimerContext(Executor2TimerContext const&) = default;
    Executor2TimerContext& operator=(Executor2TimerContext const&) = delete;

    friend detail::GetExecutor;

    ExecutorResult detach_timer() noexcept
    {
        this->timer.detach_timer();
        return ExecutorResult::ExitSuccess;
    }

    ExecutorResult retry() noexcept
    {
        this->timer.reset_time();
        return ExecutorResult::Nothing;
    }

    ExecutorResult retry_until(std::chrono::milliseconds ms)
    {
        this->timer.update_time(ms);
        return ExecutorResult::Nothing;
    }

    ExecutorResult terminate() noexcept
    {
        this->timer.detach_timer();
        return ExecutorResult::Terminate;
    }

    template<class F>
    ExecutorResult next_action(F f) noexcept
    {
        this->timer.set_on_action(f);
        return ExecutorResult::Nothing;
    }

    template<class F1, class F2>
    ExecutorResult exec_action2(F1 f1, F2 f2)
    {
        this->timer.set_on_action(f1);
        return this->timer.ctx.invoke(f2, Executor2TimerContext{this->timer});
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        return this->exec_action2(f, f);
    }

    Executor2TimerContext set_time(std::chrono::milliseconds ms)
    {
        this->timer.update_time(ms);
        return *this;
    }

protected:
    Timer2Impl<PrefixArgs, Ts...>& timer;
};


template<class PrefixArgs, class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2ActionContext
{
    friend detail::GetExecutor;

    template<class... PreviousTs>
    Executor2ActionContext(Executor2ActionContext<PrefixArgs, PreviousTs...> const& other) noexcept
      : executor(reinterpret_cast<Executor2Impl<PrefixArgs, Ts...>&>(detail::get_executor(other)))
    {
        // TODO strip arguments support (PreviousTs=(int, int), Ts=(int))
        static_assert((true && ... && detail::check_is_context_arg_convertible<PreviousTs, Ts>::value));
        static_assert(sizeof(Executor2Impl<PrefixArgs, Ts...>) == sizeof(detail::get_executor(other)));
    }

    explicit Executor2ActionContext(Executor2Impl<PrefixArgs, Ts...>& executor) noexcept
      : executor{executor}
    {}

    Executor2ActionContext(Executor2ActionContext const&) = default;
    Executor2ActionContext& operator=(Executor2ActionContext const&) = delete;

    ExecutorResult need_more_data() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult ready() noexcept
    {
        return ExecutorResult::Ready;
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
    auto create_timer(Args&&... args)
    {
        return executor.top_executor_timers.create_timer(static_cast<Args&&>(args)...);
    }

    template<class... Args>
    SubExecutorBuilder<PrefixArgs, Args...> create_sub_executor(Args&&... args)
    {
        return executor.create_sub_executor(static_cast<Args&&>(args)...);
    }

    BasicExecutorImpl<PrefixArgs>& get_basic_executor() noexcept
    {
        return this->executor;
    }

    template<class... Args>
    SubExecutorBuilder<PrefixArgs, Ts..., Args...> create_nested_executor(Args&&... args)
    {
        return executor.create_nested_executor(static_cast<Args&&>(args)...);
    }

    template<class... Args>
    ExecExecutorBuilder<PrefixArgs, Args...> exec_sub_executor(Args&&... args)
    {
        auto builder = executor.create_sub_executor(static_cast<Args&&>(args)...);
        auto& sub_executor = detail::get_executor(builder);
        return {sub_executor};
    }

    template<class... Args>
    ExecExecutorBuilder<PrefixArgs, Ts..., Args...> exec_nested_executor(Args&&... args)
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
    Executor2Impl<PrefixArgs, Ts...>& executor;
};


template<class PrefixArgs, class... Ts>
struct Executor2Impl : public BasicExecutorImpl<PrefixArgs>
{
    friend Executor2ActionContext<PrefixArgs, Ts...>;

    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_action = [](BasicExecutorImpl<PrefixArgs>& executor, auto... prefix_args) {
            auto& self = static_cast<Executor2Impl&>(executor);
            return self.ctx.invoke(
                make_lambda<F>(),
                Executor2ActionContext<PrefixArgs, Ts...>(self),
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
                Executor2ActionContext<PrefixArgs, Ts...>(self),
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
    Executor2Impl(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
      : BasicExecutorImpl<PrefixArgs>(top_executor_timers)
      , ctx{static_cast<Args&&>(args)...}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    BasicExecutorImpl<PrefixArgs>& base() noexcept
    {
        return *this;
    }

    template<class... Args>
    static Executor2Impl* New(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new Executor2Impl(top_executor_timers, static_cast<Args&&>(args)...);
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) {
            delete static_cast<Executor2Impl*>(base);
        };
        return p;
    }

protected:
    detail::tuple<Ts...> ctx;

private:
    void *operator new(size_t n) { return ::operator new(n); }
};


template<class PrefixArgs, class... Ts>
struct REDEMPTION_CXX_NODISCARD Executor2TimeoutContext : Executor2ActionContext<PrefixArgs, Ts...>
{
    using Executor2ActionContext<PrefixArgs, Ts...>::Executor2ActionContext;

    template<class F>
    Executor2ActionContext<PrefixArgs, Ts...> set_timeout_action(F f) noexcept
    {
        auto executor_action = static_cast<Executor2ActionContext<PrefixArgs, Ts...>*>(this);
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

template<class PrefixArgs>
struct BasicTimer
{
    template<class... Args>
    ExecutorResult exec_timer(Args&&... args)
    {
        return this->on_timer(*this, static_cast<Args&&>(args)...);
    }

    void delete_self()
    {
        return this->deleter(this);
    }

    void reset_time()
    {
        this->elapsed_ms = {};
    }

    std::chrono::milliseconds time()
    {
        return this->ms;
    }

    std::chrono::milliseconds elapsed_time()
    {
        return this->elapsed_ms;
    }

    std::chrono::milliseconds remaining_time()
    {
        return this->ms - this->elapsed_ms;
    }

    void set_time(std::chrono::milliseconds ms)
    {
        this->ms = ms;
        if (this->elapsed_ms > ms) {
            this->elapsed_ms = ms;
        }
    }

    bool consume(std::chrono::milliseconds consumed)
    {
        this->elapsed_ms += consumed;
        if (this->elapsed_ms >= this->ms) {
            this->reset_time();
            return true;
        }
        return false;
    }

// protected:
    friend class TopExecutorTimers<PrefixArgs>;

    using OnTimerPtrFunc = MakeFuncPtr<BasicTimer&, PrefixArgs>;
    std::chrono::milliseconds ms;
    std::chrono::milliseconds elapsed_ms = std::chrono::milliseconds::zero();
    OnTimerPtrFunc on_timer = default_function();
    void (*deleter) (BasicTimer*) = [](BasicTimer*){};

    BasicTimer() = default;
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

namespace detail
{
    template<class TimerPtr>
    struct REDEMPTION_CXX_NODISCARD TimerBuilder
    {
        template<class F>
        TimerPtr on_action(std::chrono::milliseconds ms, F f) && noexcept
        {
            this->timer_ptr->set_on_action(f);
            this->timer_ptr->update_time(ms);
            return static_cast<TimerPtr&&>(this->timer_ptr);
        }

        TimerBuilder(TimerPtr&& timer_ptr) noexcept
        : timer_ptr(static_cast<TimerPtr&&>(timer_ptr))
        {}

    private:
        TimerPtr timer_ptr;
    };
}

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

template<class PrefixArgs, class... Args>
using Timer2 = Timer2Impl<PrefixArgs, typename detail::decay_and_strip<Args>::type...>;

template<class PrefixArgs, class... Args>
using TimerBuilder = detail::TimerBuilder<UniquePtr<BasicTimer<PrefixArgs>, Timer2<PrefixArgs, Args...>>>;

template<class PrefixArgs>
struct TopExecutorTimers
{
    template<class... Args>
    TimerBuilder<PrefixArgs, Args...> create_timer(Args&&... args)
    {
        using TimerType = Timer2<PrefixArgs, Args...>;
        using UniqueTimerPtr = UniquePtr<BasicTimer<PrefixArgs>, TimerType>;
        UniqueTimerPtr uptr(TimerType::New(*this, static_cast<Args&&>(args)...));
        this->timers.emplace_back(uptr.get());
        return std::move(uptr);
    }

//     template<class... Args>
//     TimedExecutorBuilder<Args...> create_timed_executor(Args&&... args)
//     {}

    void add_timer(BasicTimer<PrefixArgs>& timer)
    {
        this->timers.emplace_back(&timer);
    }

    void update_time(BasicTimer<PrefixArgs>& timer, std::chrono::milliseconds ms)
    {
        (void)timer;
        (void)ms;
    }

    void detach_timer(BasicTimer<PrefixArgs>& timer)
    {
        this->timers.erase(
            std::find_if(this->timers.begin(), this->timers.end(), [&timer](auto* p){
                return p == &timer;
            }),
            this->timers.end()
        );
    }

    std::chrono::milliseconds get_next_timeout() const noexcept
    {
        auto r = std::chrono::milliseconds::max();
        for (auto& timer : this->timers) {
            r = std::min(r, timer->remaining_time());
        }
        return r;
    }

    template<class... Args>
    void exec_timeout(Args&&... args);

private:
    std::vector<BasicTimer<PrefixArgs>*> timers;
    // std::chrono::milliseconds next_timeout;
};

template<class PrefixArgs>
struct TopExecutorBase : TopExecutorTimers<PrefixArgs>
{
    BasicExecutorImpl<PrefixArgs> base_executor;

    void delete_self()
    {
        this->base_executor.delete_self();
    }
};

template<class PrefixArgs>
template<class... Args>
void TopExecutorTimers<PrefixArgs>::exec_timeout(Args&&... args)
{
    auto ms = this->get_next_timeout();
    for (std::size_t i = 0; i < this->timers.size(); ) {
        auto* timer = this->timers[i];
        if (timer->consume(ms)) {
            switch (timer->exec_timer(static_cast<Args&&>(args)...)) {
                case ExecutorResult::ExitSuccess:
                case ExecutorResult::ExitFailure:
                    this->timers.erase(this->timers.begin() + i);
                    break;
                case ExecutorResult::Terminate:
                    break;
                case ExecutorResult::Nothing:
                case ExecutorResult::NeedMoreData:
                case ExecutorResult::Ready:
                    ++i;
                    break;
            }
        }
        else {
            ++i;
        }
    }
}

template<class PrefixArgs, class... Ts>
struct Timer2Impl : BasicTimer<PrefixArgs>
{
    template<class F>
    void set_on_action(F) noexcept
    {
        this->on_timer = [](BasicTimer<PrefixArgs>& timer, auto... prefix_args) {
            auto& self = static_cast<Timer2Impl&>(timer);
            return self.ctx.invoke(
                make_lambda<F>(),
                Executor2TimerContext<PrefixArgs, Ts...>(self),
                static_cast<decltype(prefix_args)&&>(prefix_args)...);
        };
    }

    void detach_timer()
    {
        this->top_executor_timers.detach_timer(*this);
    }

    void update_time(std::chrono::milliseconds ms)
    {
        this->set_time(ms);
        this->top_executor_timers.update_time(*this, ms);
    }

    Timer2Impl(Timer2Impl const&) = delete;
    Timer2Impl& operator=(Timer2Impl const&) = delete;

    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class... Args>
    Timer2Impl(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
      : ctx{static_cast<Args&&>(args)...}
      , top_executor_timers(top_executor_timers)
    {}
    REDEMPTION_DIAGNOSTIC_POP

    BasicTimer<PrefixArgs>& base() noexcept
    {
        return *this;
    }

    template<class... Args>
    static Timer2Impl* New(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new Timer2Impl(top_executor_timers, static_cast<Args&&>(args)...);
        p->deleter = [](BasicTimer<PrefixArgs>* base) {
            auto* timer_ptr = static_cast<Timer2Impl*>(base);
            timer_ptr->top_executor_timers.detach_timer(*timer_ptr);
            delete timer_ptr;
        };
        return p;
    }

protected:
    detail::tuple<Ts...> ctx;

private:
    TopExecutorTimers<PrefixArgs>& top_executor_timers;

private:
    void *operator new(size_t n) { return ::operator new(n); }
};

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

    template<class F>
    void set_on_timeout(F) noexcept
    {
        this->timeout.on_timer = [](BasicTimer<PrefixArgs>& timer, auto... prefix_args) {
            auto& timer_mem = static_cast<Timer2Impl<PrefixArgs>&>(timer);
            auto& self = detail::get_before<TopExecutorImpl, Executor2Impl<PrefixArgs, Ts...>>(timer_mem);
            // TODO ExecutorTimeoutContext
            return self.ctx.invoke(
                make_lambda<F>(),
                Executor2TimeoutContext<PrefixArgs, Ts...>(self),
                static_cast<decltype(prefix_args)&&>(prefix_args)...);
        };
    }

    void set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->timeout.set_time(ms);
    }

    TopExecutorImpl(TopExecutorImpl const&) = delete;
    TopExecutorImpl& operator=(TopExecutorImpl const&) = delete;

    template<class... Args>
    TopExecutorImpl(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
      : Executor2Impl<PrefixArgs, Ts...>(top_executor_timers, static_cast<Args&&>(args)...)
      , timeout(top_executor_timers)
    {
        top_executor_timers.add_timer(this->timeout);
    }

    template<class... Args>
    static TopExecutorImpl* New(TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new TopExecutorImpl{top_executor_timers, static_cast<Args&&>(args)...};
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) {
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
        TopExecutorTimers<PrefixArgs>& top_executor_timers,
        Args&&... args)
      : DataExecutor<Data>{data_arg}
      , TopExecutorImpl<PrefixArgs, Ts...>(top_executor_timers, static_cast<Args&&>(args)...)
    {}

    template<class DataArgs, class... Args>
    static TopExecutorWithDataImpl* New(DataArgs data_args, TopExecutorTimers<PrefixArgs>& top_executor_timers, Args&&... args)
    {
        auto* p = new TopExecutorWithDataImpl{data_args, top_executor_timers, static_cast<Args&&>(args)...};
        p->deleter = [](BasicExecutorImpl<PrefixArgs>* base) {
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
    TopExecutorTimers<prefix_args<Ts...>> timers;
};


namespace detail { namespace {
    template<class... Ts>
    static ExecutorResult terminate_callee(Ts...)
    {
        assert("call a executor marked 'Terminate'");
        return ExecutorResult::Terminate;
    }
} }

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
