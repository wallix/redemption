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
#include <chrono>
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
    using OnActionPtrFunc = ExecutorResult(*)(AnyCtxPtr&, ExecutorBase&);

    std::chrono::milliseconds ms;
    OnActionPtrFunc on_action;
    AnyCtxPtr ctx;

    TimerEvent() = delete;

    TimerEvent(std::chrono::milliseconds ms, AnyCtxPtr ctx) noexcept
    : ms(ms)
    , ctx(std::move(ctx))
    {}

    ExecutorResult exec_action(ExecutorBase& executor)
    {
        return this->on_action(this->ctx, executor);
    }
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
    class TimerIdCtx;
    class TimerId;

    template<class F, class... Args>
    TimerIdCtx<detail::ctx_arg_type<Args...>> add_timer(
        std::chrono::milliseconds ms, F, Args&&... args)
    {
        using Ctx = detail::ctx_arg_type<Args...>;
        this->timers.emplace_back(
            ms, this->timer_ctx_memory.template create<Ctx>(static_cast<Args&&>(args)...));
        auto& on_action = this->timers.back().on_action;
        on_action = make_on_action<Ctx, F>();
        return {on_action, this};
    }

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
    std::vector<TimerEvent> timers;
    CtxMemory action_ctx_memory;
    CtxMemory timer_ctx_memory;
};

struct Executor
{
    template<class... Args>
    MakeInitialSubExecutorBuilder<Args...>
    create_executor(Args&&... args)
    {
        return MakeInitialSubExecutorBuilder<Args...>{
            this->base.create_ctx_event(static_cast<Args&&>(args)...)};
    }

    template<class F, class... Args>
    ExecutorBase::TimerIdCtx<detail::ctx_arg_type<Args...>>
    add_timer(std::chrono::milliseconds ms, F f, Args&&... args)
    {
        return this->base.add_timer(ms, f, static_cast<Args&&>(args)...);
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
        static auto& get_executor(T& x)
        { return x.executor; }
    };
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
      : executor(detail::GetExecutor::get_executor(other))
    {
        // TODO strip arguments support (PreviousTs=(int, int), Ts=(int))
        static_assert((..., check_is_context_arg_convertible<PreviousTs, Ts>::value));
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
        auto& sub_executor = detail::GetExecutor::get_executor(builder);
        return {sub_executor};
    }

    template<class... Args>
    ExecExecutorBuilder<Ts..., Args...> exec_nested_executor(Args&&... args)
    {
        auto builder = executor.create_nested_executor(static_cast<Args&&>(args)...);
        auto& sub_executor = detail::GetExecutor::get_executor(builder);
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
        return executor.ctx.invoke(f2, executor);
    }

    template<class F>
    ExecutorResult exec_action(F f)
    {
        executor.set_on_action(f);
        return executor.exec_action(f);
    }

    template<class F>
    Executor2ActionContext set_exit_action(F f) noexcept
    {
        executor.set_on_exit(f);
        return *this;
    }

private:
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
        auto& sub_executor = detail::GetExecutor::get_executor(builder);
        sub_executor.current = this->current;
        sub_executor.prev = this;
        this->current->current = &sub_executor;
        return {sub_executor};
    }

    template<class... Args>
    SubExecutorBuilder<Ts..., Args...> create_nested_executor(Args&&... args)
    {
        return this->ctx.invoke([&](auto&&... args2){
            return this->create_sub_executor(
                static_cast<decltype(args2)&&>(args2)...,
                static_cast<Args&&>(args)...
            );
        });
    }

    template<class... Args>
    Executor2Impl(Reactor& reactor, Args&&... args)
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
      : ctx{static_cast<Args&&>(args)...}
      , reactor(reactor)
    REDEMPTION_DIAGNOSTIC_POP
    {
        this->deleter = [](void* p) { static_cast<Executor2Impl*>(p)->~Executor2Impl(); };
    }

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
        detail::GetExecutor::get_executor(executor_action)->set_on_timeout(f);
        return *executor_action;
    }

    Executor2TimeoutContext set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->set_timeout(ms);
        return *this;
    }
};

struct GenericTopExecutor
{
    using OnTimeoutPtrFunc = ExecutorResult(*)(BasicExecutor&);
    OnTimeoutPtrFunc on_timeout;
    std::chrono::milliseconds ms;

    void set_timeout(std::chrono::milliseconds ms)
    {
        this->ms = ms;
    }
};

template<class... Ts>
struct TopExecutorImpl : public Executor2Impl<Ts...>, GenericTopExecutor
{
    using Executor2Impl<Ts...>::Executor2Impl;

    ExecutorResult exec_timeout()
    {
        return this->on_timeout(*this);
    }

    template<class F>
    void set_on_timeout(F) noexcept
    {
        this->on_timeout = [](BasicExecutor& executor) {
            auto& self = static_cast<TopExecutorImpl&>(executor);
            // TODO ExecutorTimeoutContext
            return self.ctx.invoke(make_lambda<F>(), Executor2TimeoutContext<Ts...>(self));
        };
    }
};



struct Reactor
{
    template<class... Args>
    TopExecutorBuilder<Args...> create_executor(int /*fd*/, Args&&... args)
    {
        auto& list = this->executors.emplace_back();
        return {list.emplace_back<TopExecutor2<Args...>>(*this, static_cast<Args&&>(args)...)};
    }

    template<class... Args>
    ExecutorBuilder<Args...> create_sub_executor(Args&&... args)
    {
        auto& list = this->executors.emplace_back();
        return {list.emplace_back<Executor2<Args...>>(*this, static_cast<Args&&>(args)...)};
    }

//     template<class... Args>
//     TimeId create_timer(std::chrono::milliseconds ms, Args&&... args)
//     {
//         return MakeInitialSubExecutorBuilder<Args...>{
//             this->base.create_ctx_event(static_cast<Args&&>(args)...)};
//     }

private:
    struct intrusive_list
    {
        friend detail::GetExecutor;

        struct node_base
        {
            node_base* next;
        };

        template<class T>
        struct node : node_base
        {
            node_base* prev;
            void(*deleter)(void*);
            T data;
        };

        node_base first_node{};
        node_base* last_node = &first_node;

        template<class T, class... Args>
        T& emplace_back(Args&&... args)
        {
            auto* p = new node<T>{
                {nullptr},
                this->last_node,
                [](void* p) { delete static_cast<node<T>*>(p); },
                T{static_cast<Args&&>(args)...}
            };
            this->last_node->next = p;
            this->last_node = p;
            return p->data;
        }

        intrusive_list() = default;

        intrusive_list(intrusive_list&& other) noexcept
          : first_node(other.first_node)
          , last_node(other.last_node == &other.first_node ? &this->first_node : other.last_node)
        {
            other.first_node.next = nullptr;
            other.last_node = &other.first_node;
        }

        intrusive_list(intrusive_list const&) = delete;

        intrusive_list& operator=(intrusive_list&& other) noexcept
        {
            this->first_node = other.first_node;
            this->last_node = (other.last_node == &other.first_node)
              ? &this->first_node : other.last_node;
            other.first_node.next = nullptr;
            other.last_node = &other.first_node;
            return *this;
        }

        intrusive_list& operator=(intrusive_list const&) = delete;

        ~intrusive_list()
        {
            this->clear();
        }

        void clear()
        {
            node_base* p;
            node_base* next = this->first_node.next;
            while (next) {
                p = std::exchange(next, next->next);
                static_cast<node<char>*>(p)->deleter(p);
            }
            this->first_node.next = nullptr;
            this->last_node = &this->first_node;
        }
    };
    std::vector<intrusive_list> executors;
};


template<class Ctx>
struct REDEMPTION_CXX_NODISCARD ExecutorActionContext
{
    friend detail::GetExecutor;

    template<class PreviousCtx>
    ExecutorActionContext(ExecutorActionContext<PreviousCtx> const& other) noexcept
      : executor(detail::GetExecutor::get_executor(other))
    {
        static_assert(is_context_convertible<PreviousCtx, Ctx>::value);
    }

    explicit ExecutorActionContext(ExecutorBase& executor) noexcept
      : executor{executor}
    {}

    ExecutorActionContext(ExecutorActionContext const&) = default;
    ExecutorActionContext& operator=(ExecutorActionContext const&) = delete;

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
        return ExecutorResult::Nothing;
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

private:
    ExecutorBase& executor;
};

template<class Ctx>
struct REDEMPTION_CXX_NODISCARD ExecutorTimerContext
{
    friend detail::GetExecutor;

    template<class PreviousCtx>
    ExecutorTimerContext(ExecutorTimerContext<PreviousCtx> const& other) noexcept
      : timerid(detail::GetExecutor::get_executor(other))
    {
        static_assert(is_context_convertible<PreviousCtx, Ctx>::value);
    }

    explicit ExecutorTimerContext(ExecutorBase::TimerId& timerid) noexcept
      : timerid{static_cast<ExecutorBase::TimerIdCtx<Ctx>&>(timerid)}
    {}

    ExecutorTimerContext(ExecutorTimerContext const&) = default;
    ExecutorTimerContext& operator=(ExecutorTimerContext const&) = delete;

    void set_time(std::chrono::milliseconds ms)
    {
        this->timerid.set_time(ms);
    }

    ExecutorResult retry() noexcept
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult remove_timer() noexcept
    {
        this->timerid.reset();
        return ExecutorResult::Nothing;
    }

    template<class F>
    ExecutorResult next_timer(F f) noexcept
    {
        this->timerid.set_action(f);
    }

    template<class F1, class F2>
    ExecutorResult exec_timer2(F1 f, F2)
    {
        this->timerid.set_action(f);
        return make_on_action<Ctx, F2>()(
            static_cast<AnyCtxPtr&>(this->timerid.ctx), this->executor);
    }

    template<class F>
    ExecutorResult exec_timer(F f)
    {
        return this->exec_timer2(f, f);
    }

private:
    ExecutorBase::TimerIdCtx<Ctx>& timerid;
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
                case ExecutorResult::Terminate:
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
        case ExecutorResult::Terminate:
        case ExecutorResult::Nothing:
            break;
    }

    return !this->base.events.empty();
}

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


struct ExecutorBase::TimerId
{
    void* ctx;
    ExecutorBase* executor;

    TimerId(TimerId const&) = delete;
    TimerId& operator=(TimerId const&) = delete;

    TimerId(TimerId&& other) noexcept
      : ctx(std::exchange(other.ctx, nullptr))
      , executor(other.executor)
    {}

    TimerId& operator=(TimerId&& other) noexcept
    {
        assert(this != &other);
        this->ctx = std::exchange(other.ctx, nullptr);
        this->executor = other.executor;
        return *this;
    }

    ~TimerId()
    {
        this->reset();
    }

    void reset() noexcept
    {
        this->executor->timers.erase(this->timer_it());
        this->ctx = nullptr;
    }

    ExecutorResult exec_action()
    {
        return this->timer_it()->exec_action(*this->executor);
    }

    void set_time(std::chrono::milliseconds ms)
    {
        this->timer_it()->ms = ms;
    }

protected:
    std::vector<TimerEvent>::iterator timer_it()
    {
        auto it = this->executor->timers.begin();
        while (it->ctx.get() != this->ctx) {
            ++it;
        }
        return it;
    }
};

template<class Ctx>
struct ExecutorBase::TimerIdCtx : private TimerId
{
    friend ExecutorTimerContext<Ctx>;

    template<class F>
    void set_action(F) noexcept
    {
        this->on_action = make_on_action<Ctx, F>();
    }

    using TimerId::reset;

    template<class F, class... Args>
    void reset(F, Args&&... args)
    {
        auto it = this->timer_it();
        it->ctx = this->executor->timer_ctx_memory.
            template create<Ctx>(static_cast<Args&&>(args)...);
        this->ctx = it->ctx.get();
        it->on_action = make_on_action<Ctx, F>();
    }
};
