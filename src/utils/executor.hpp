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

#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"

#include <vector>
#include <type_traits>
#include <utility>
#include <functional> // std::reference_wrapper
#include <cassert>

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
        auto invoke(F && f, Args&&... args)
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
    ReplaceAction,
    ExitSuccess,
    ExitFailure,
};


struct AnyCtxPtr
{
    class any {};
    struct real_deleter
    {
        void (*deleter) (void*);
        void operator()(any* x) const noexcept
        {
            deleter(x);
        }
    };

    explicit AnyCtxPtr() = default;

    template<class T, class F>
    explicit AnyCtxPtr(T* p, F f) noexcept
      : p{reinterpret_cast<any*>(p), {f}}
    {}

    void* get() const noexcept
    { return this->p.get(); }

private:
    std::unique_ptr<any, real_deleter> p;
};

struct REDEMPTION_CXX_NODISCARD ExecutorEvent
{
    using OnActionPtrFunc = ExecutorResult(*)(AnyCtxPtr&, ExecutorBase&);
    using OnExitPtrFunc = ExecutorResult(*)(AnyCtxPtr&, ExecutorBase&, bool success);

    OnActionPtrFunc on_action;
    OnExitPtrFunc on_exit;
    AnyCtxPtr ctx;

    ExecutorEvent() = delete;

    ExecutorEvent(AnyCtxPtr ctx) noexcept
    : ctx(std::move(ctx))
    {}

    ExecutorResult exec_action(ExecutorBase& executor)
    {
        return this->on_action(this->ctx, executor);
    }

    ExecutorResult exec_exit(ExecutorBase& executor, bool status)
    {
        return this->on_exit(this->ctx, executor, status);
    }
};

struct REDEMPTION_CXX_NODISCARD TimerEvent
{
    using OnActionPtrFunc = ExecutorResult(*)(ExecutorBase&, AnyCtxPtr&);

    OnActionPtrFunc on_action;
    AnyCtxPtr ctx;

    TimerEvent() = delete;

    TimerEvent(AnyCtxPtr ctx) noexcept
    : ctx(std::move(ctx))
    {}
};

enum class ExitStatus { Error, Success, };

#ifdef IN_IDE_PARSER
struct SubExecutorBuilderConcept_
{
    template<class F> SubExecutorBuilderConcept_ on_action(F&&) && { return *this; }
    template<class F> SubExecutorBuilderConcept_ on_exit  (F&&) && { return *this; }

    template<class T> SubExecutorBuilderConcept_(T const &) noexcept;
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


    template<template<class> class ExecutorContext, class Ctx, class F, class... Args>
    auto make_ctx_function() noexcept
    {
        return [](AnyCtxPtr& any, ExecutorBase& executor, Args... args){
            return static_cast<Ctx*>(any.get())->invoke(
                make_lambda<F>(), ExecutorContext<Ctx>(executor), args...);
        };
    }

    template<class Ctx, class F>
    inline constexpr auto make_on_action = make_ctx_function<ExecutorActionContext, Ctx, F>;

    template<class Ctx, class F>
    inline constexpr auto make_on_exit = make_ctx_function<ExecutorActionContext, Ctx, F, bool>;
}


template<class Ctx, bool Initial, int Mask = 0>
struct REDEMPTION_CXX_NODISCARD SetSubExecutorBuilder
{
    template<class F>
    SetSubExecutorBuilder<Ctx, Initial, Mask | 1>
    on_action(F) &&
    {
        static_assert(!(Mask & 1), "on_action already set");
        this->executor_event.on_action = make_on_action<Ctx, F>();
        return SetSubExecutorBuilder<Ctx, Initial, Mask | 1>{this->executor_event};
    }

    template<class F>
    SetSubExecutorBuilder<Ctx, Initial, Mask | 2>
    on_exit(F) &&
    {
        static_assert(!(Mask & 2), "on_exit already set");
        this->executor_event.on_exit = make_on_exit<Ctx, F>();
        return SetSubExecutorBuilder<Ctx, Initial, Mask | 2>{this->executor_event};
    }

    explicit SetSubExecutorBuilder(ExecutorEvent& executor_event) noexcept
      : executor_event(executor_event)
    {}

private:
    ExecutorEvent& executor_event;
};

// for pretty error
class ExecutorCompleted {};

#define MK_SubExecutorBuilderFinal(i, mem)                               \
    template<class Ctx>                                                  \
    struct REDEMPTION_CXX_NODISCARD SetSubExecutorBuilder<Ctx, false, i> \
    {                                                                    \
        template<class F>                                                \
        ExecutorResult mem(F) && noexcept                                \
        {                                                                \
            this->executor_event.mem = make_##mem<Ctx, F>();             \
            return ExecutorResult::Nothing;                              \
        }                                                                \
                                                                         \
        SetSubExecutorBuilder(ExecutorEvent& executor_event) noexcept    \
        : executor_event(executor_event)                                 \
        {}                                                               \
                                                                         \
    private:                                                             \
        ExecutorEvent& executor_event;                                   \
    };                                                                   \
                                                                         \
    template<class Ctx>                                                  \
    struct SetSubExecutorBuilder<Ctx, true, i>                           \
    {                                                                    \
        template<class F>                                                \
        ExecutorCompleted mem(F) && noexcept                             \
        {                                                                \
            this->executor_event.mem = make_##mem<Ctx, F>();             \
            return {};                                                   \
        }                                                                \
                                                                         \
        SetSubExecutorBuilder(ExecutorEvent& executor_event) noexcept    \
        : executor_event(executor_event)                                 \
        {}                                                               \
                                                                         \
    private:                                                             \
        ExecutorEvent& executor_event;                                   \
    }

MK_SubExecutorBuilderFinal(0b10, on_action);
MK_SubExecutorBuilderFinal(0b01, on_exit);

#undef MK_SubExecutorBuilderFinal

#ifdef IN_IDE_PARSER
template<class... Args>
using MakeSubExecutorBuilder = SubExecutorBuilderConcept_;

template<class... Args>
using MakeInitialSubExecutorBuilder = SubExecutorBuilderConcept_;
#else
template<class... Args>
using MakeSubExecutorBuilder = SetSubExecutorBuilder<detail::ctx_arg_type<Args...>, false>;

template<class... Args>
using MakeInitialSubExecutorBuilder = SetSubExecutorBuilder<detail::ctx_arg_type<Args...>, true>;
#endif

struct ExecutorBase
{
    template<class Ctx>
    class TimerRefCtx;
    class TimerRef;

    template<class F, class... Args>
    TimerRefCtx<detail::ctx_arg_type<Args...>> add_timeout(F, Args&&... args);

    template<class... Args>
    ExecutorEvent& create_ctx_event(Args&&... args)
    {
        using Ctx = detail::ctx_arg_type<Args...>;
        this->events.emplace_back(
            this->action_ctx_memory.template create<Ctx>(static_cast<Args&&>(args)...));
        return {this->events.back()};
    }

    struct CtxMemory
    {
        template<class Ctx, class... Args>
        AnyCtxPtr create(Args&&... args)
        {
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
            // TODO intrusive_list
            auto * p = new Ctx{static_cast<Args&&>(args)...};
            REDEMPTION_DIAGNOSTIC_POP
            return AnyCtxPtr{p, [](void* p){ delete static_cast<Ctx*>(p); }};
        }

        template<class Ctx>
        AnyCtxPtr create()
        {
            return AnyCtxPtr{};
        }
    };

    std::vector<ExecutorEvent> events;
    std::vector<TimerEvent> timeouts;
    CtxMemory action_ctx_memory;
    CtxMemory timeout_ctx_memory;
};

struct Executor
{
    template<class... Args>
    MakeInitialSubExecutorBuilder<Args...>
    initial_executor(Args&&... args)
    {
        return MakeInitialSubExecutorBuilder<Args...>{
            this->base.create_ctx_event(static_cast<Args&&>(args)...)};
    }

    template<class... Args>
    MakeInitialSubExecutorBuilder<Args...>
    add_timeout(Args&&... args)
    {
        return MakeInitialSubExecutorBuilder<Args...>{
            this->base.create_ctx_event(static_cast<Args&&>(args)...)};
    }

    bool exec();

    void exec_all()
    {
        while (this->exec()) {
        }
    }

private:
    ExecutorBase base;
};


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

namespace detail
{
    struct GetExecutor
    {
        template<class T>
        static ExecutorBase& get_executor(T& x)
        { return x.executor; }
    };
}

template<class Ctx>
struct REDEMPTION_CXX_NODISCARD ExecutorActionContext
{
    friend detail::GetExecutor;

    template<class PreviousCtx>
    ExecutorActionContext(ExecutorActionContext<PreviousCtx> other) noexcept
      : executor(detail::GetExecutor::get_executor(other))
    {
        static_assert(is_context_convertible<PreviousCtx, Ctx>::value);
    }

    ExecutorActionContext(ExecutorActionContext &&) = default;
    ExecutorActionContext(ExecutorActionContext const &) = default;
    ExecutorActionContext& operator=(ExecutorActionContext &&) = default;
    ExecutorActionContext& operator=(ExecutorActionContext const &) = default;

    ExecutorResult retry() noexcept
    {
        return ExecutorResult::Nothing;
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
    MakeSubExecutorBuilder<Args...>
    sub_executor(Args&&... args)
    {
        return MakeSubExecutorBuilder<Args...>{
            this->executor.create_ctx_event(static_cast<Args&&>(args)...)};
    }

    template<class F>
    ExecutorResult next_action(F) noexcept
    {
        ExecutorEvent& event = this->executor.events.back();
        event.on_action = make_on_action<Ctx, F>();
        return ExecutorResult::ReplaceAction;
    }

    template<class F1, class F2>
    ExecutorResult exec_action2(F1, F2)
    {
        ExecutorEvent& event = this->executor.events.back();
        event.on_action = make_on_action<Ctx, F1>();
        return make_on_action<Ctx, F2>()(event.ctx, this->executor);
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        return this->exec_action2(f, f);
    }

    template<class F>
    ExecutorActionContext set_exit_action(F) noexcept
    {
        ExecutorEvent& event = this->executor.events.back();
        event.on_exit = make_on_exit<Ctx, F>();
        return *this;
    }

    explicit ExecutorActionContext(ExecutorBase& executor) noexcept
      : executor{executor}
    {}

private:
    ExecutorBase& executor;
};

bool Executor::exec()
{
    auto process_exit = [this](bool status) {
        while (!this->base.events.empty()) {
            switch (this->base.events.back().exec_exit(this->base, status)) {
                case ExecutorResult::ExitSuccess:
                    status = true;
                    this->base.events.pop_back();
                    break;
                case ExecutorResult::ExitFailure:
                    status = false;
                    this->base.events.pop_back();
                    break;
                case ExecutorResult::ReplaceAction:
                case ExecutorResult::Nothing:
                    return;
            }
        }
    };

    switch (this->base.events.back().exec_action(this->base)) {
        case ExecutorResult::ExitSuccess:
            process_exit(true);
            break;
        case ExecutorResult::ExitFailure:
            process_exit(false);
            break;
        case ExecutorResult::ReplaceAction:
        case ExecutorResult::Nothing:
            break;
    }

    return !this->base.events.empty();
}


struct ExecutorBase::TimerRef
{
    TimerEvent::OnActionPtrFunc on_action;
    ExecutorBase* executor;

    TimerRef(TimerRef const&) = delete;
    TimerRef& operator=(TimerRef const&) = delete;

    TimerRef(TimerRef&& other) noexcept
      : on_action(std::exchange(other.on_action, nullptr))
      , executor(other.executor)
    {}

    TimerRef& operator=(TimerRef&& other) noexcept
    {
        assert(this != &other);
        this->on_action = std::exchange(other.on_action, nullptr);
        this->executor = other.executor;
        return *this;
    }

    ~TimerRef()
    {
        this->reset();
    }

    void reset() noexcept
    {
        this->executor->timeouts.erase(this->timer_it());
        this->on_action = nullptr;
    }

protected:
    std::vector<TimerEvent>::iterator timer_it()
    {
        return std::find_if(this->executor->timeouts.begin(), this->executor->timeouts.end(),
            [this](auto& timer) { return timer.on_action == this->on_action; });
    }

    CtxMemory& ctx_memory()
    {
        return this->executor->timeout_ctx_memory;
    }
};

template<class Ctx>
struct ExecutorBase::TimerRefCtx : TimerRef
{
    template<class F>
    void set_action(F) noexcept
    {
        this->on_action = make_on_action<Ctx, F>();
    }

    using TimerRef::reset;

    template<class F, class... Args>
    void reset(F, Args&&... args)
    {
        auto it = this->timer_it();
        it->ctx = this->ctx_memory().template create<Ctx>(static_cast<Args&&>(args)...);
        this->on_action = make_on_action<Ctx, F>();
        it->on_action = this->on_action;
    }
};

template<class F, class... Args>
ExecutorBase::TimerRefCtx<detail::ctx_arg_type<Args...>>
ExecutorBase::add_timeout(F, Args&&... args)
{
    using Ctx = detail::ctx_arg_type<Args...>;
    this->timeouts.emplace_back(
        this->timeout_ctx_memory.template create<Ctx>(static_cast<Args&&>(args)...));
    auto& on_action = this->timeouts.back().on_action;
    on_action = make_on_action<Ctx, F>();
    return {on_action, this};
}
