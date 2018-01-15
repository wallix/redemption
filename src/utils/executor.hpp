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
#include <cassert>

namespace detail
{
    template<int...>
    class seq_int;

    template<class, class>
    struct cat_seq;

    template<int... i1, int... i2>
    struct cat_seq<seq_int<i1...>, seq_int<i2...>>
    {
        using type = seq_int<i1..., sizeof...(i1)+i2...>;
    };

    template<int n>
    struct make_seq_int_impl
    : cat_seq<
        typename make_seq_int_impl<n/2>::type,
        typename make_seq_int_impl<n - n/2>::type
    >{};

    template<>
    struct make_seq_int_impl<0>
    {
        using type = seq_int<>;
    };

    template<>
    struct make_seq_int_impl<1>
    {
        using type = seq_int<0>;
    };

    template<int n>
    using make_seq_int = typename make_seq_int_impl<n>::type;

    template<int, class T>
    struct tuple_elem
    {
        T x;
    };

    template<class Ints, class... Ts>
    struct tuple_impl;

    template<int... ints, class... Ts>
    struct tuple_impl<seq_int<ints...>, Ts...>
    : tuple_elem<ints, Ts>...
    {
        template<class F, class... Args>
        auto invoke(F && f, Args&&... args)
        -> decltype(f(static_cast<Args&&>(args)..., *static_cast<Ts*>(nullptr)...))
        {
            return f(
                static_cast<Args&&>(args)...,
                static_cast<tuple_elem<ints, Ts>&>(*this).x...
            );
        }

    protected:
        using tuple_base = tuple_impl;

//         template<class... Args>
//         tuple_impl(Args&&... args)
//         : tuple_elem<ints, Ts>{static_cast<Args&&>(args)}...
//         {}
//
//         tuple_impl(tuple_impl &&) = default;
//         tuple_impl(tuple_impl const &) = default;
//         tuple_impl& operator=(tuple_impl &&) = default;
//         tuple_impl& operator=(tuple_impl const &) = default;
    };

//     template<class... Ts>
//     using tuple = tuple_impl<make_seq_int<int(sizeof...(Ts))>, Ts...>;

    template<class... Ts>
    struct tuple : tuple_impl<make_seq_int<int(sizeof...(Ts))>, Ts...>
    {
//         template<class... Args>
//         tuple(Args&&... args)
//         : tuple::tuple_base{static_cast<Args&&>(args)...}
//         {}
//
//         tuple(tuple &&) = default;
//         tuple(tuple const &) = default;
//         tuple& operator=(tuple &&) = default;
//         tuple& operator=(tuple const &) = default;
    };

    template<class... Args>
    using ctx_arg_type = detail::tuple<typename std::decay<Args>::type...>;
}


// #define CXX_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define CXX_WARN_UNUSED_RESULT [[nodiscard]]

class ExecutorResult;
class ExecutorBase;
class Executor;

namespace detail
{
    struct FriendExecutorResult;
}

class REDEMPTION_CXX_NODISCARD ExecutorResult
{
    enum Process : uint8_t
    {
        Nothing,
        ReplaceAction,
        ReplaceTimeout,
        ExitSuccess,
        ExitFailure,
    };

    friend class Executor;
    friend class ExecutorBase;
    friend class detail::FriendExecutorResult;

    constexpr ExecutorResult(Process e) noexcept
      : process(e)
    {}

    Process process;
};

namespace detail
{
    struct FriendExecutorResult
    {
        static constexpr ExecutorResult Nothing = ExecutorResult::Process::Nothing;
    };
}

struct ExecutorEvent
{
    class any {};
    struct real_deleter
    {
        void (*deleter) (void*);
        void operator()(any* x) const
        {
            deleter(x);
        }
    };

    using CtxPtr = std::unique_ptr<any, real_deleter>;
    using OnActionPtrFunc = ExecutorResult(*)(ExecutorBase&, any&);
    using OnTimeoutPtrFunc = ExecutorResult(*)(ExecutorBase&, any&);
    using OnExitPtrFunc = ExecutorResult(*)(ExecutorBase&, bool success, any&);

    OnActionPtrFunc on_action;
    OnTimeoutPtrFunc on_timeout;
    OnExitPtrFunc on_exit;
    std::unique_ptr<any, real_deleter> ctx;

    ExecutorEvent() = delete;

    ExecutorEvent(CtxPtr ctx)
    : ctx(std::move(ctx))
    {}

    ExecutorEvent(any)
    {}
};

template<class Ctx>
struct EventInitializer
{
    ExecutorEvent& executor_event;

    template<class F> void init_on_action(F f);
    template<class F> void init_on_exit(F f);
    template<class F> void init_on_timeout(F f);
};

enum class ExitStatus { Error, Success, };

#ifdef IN_IDE_PARSER
struct SubExecutorBuilderConcept_
{
    template<class F> SubExecutorBuilderConcept_ on_action (F&&) && { return *this; }
    template<class F> SubExecutorBuilderConcept_ on_timeout(F&&) && { return *this; }
    template<class F> SubExecutorBuilderConcept_ on_exit   (F&&) && { return *this; }

    template<class T> SubExecutorBuilderConcept_(T const &) noexcept;
};

struct ExecutorContextConcept_
{
    ExecutorResult retry();
    ExecutorResult exit(ExitStatus status);
    ExecutorResult exit_on_error();
    ExecutorResult exit_on_success();

    template<class... Args> SubExecutorBuilderConcept_ sub_executor(Args&&...);
};

struct ExecutorTimeoutContextConcept_ : ExecutorContextConcept_
{
    template<class F> ExecutorResult next_timeout(F);
    template<class F> void set_action(F);
};

struct ExecutorActionContextConcept_ : ExecutorContextConcept_
{
    template<class F> ExecutorResult next_action(F);
    template<class F> void set_timeout(F);
};

struct ExecutorExitContextConcept_ : ExecutorContextConcept_
{
    template<class F> ExecutorResult set_action(F);
    template<class F> void set_timeout(F);
};
#endif


template<class EventCtx, bool Initial, int Mask = 0>
struct REDEMPTION_CXX_NODISCARD SetSubExecutorBuilder
{
    template<class F>
    SetSubExecutorBuilder<EventCtx, Initial, Mask | 1>
    on_action(F&& f) &&
    {
        static_assert(!(Mask & 1), "on_action already set");
        this->event_initializer.init_on_action(static_cast<F&&>(f));
        return SetSubExecutorBuilder<EventCtx, Initial, Mask | 1>{this->event_initializer};
    }

    template<class F>
    SetSubExecutorBuilder<EventCtx, Initial, Mask | 2>
    on_timeout(F&& f) &&
    {
        static_assert(!(Mask & 2), "on_timeout already set");
        this->event_initializer.init_on_timeout(static_cast<F&&>(f));
        return SetSubExecutorBuilder<EventCtx, Initial, Mask | 2>{this->event_initializer};
    }

    template<class F>
    SetSubExecutorBuilder<EventCtx, Initial, Mask | 4>
    on_exit(F&& f) &&
    {
        static_assert(!(Mask & 4), "on_exit already set");
        this->event_initializer.init_on_exit(static_cast<F&&>(f));
        return SetSubExecutorBuilder<EventCtx, Initial, Mask | 4>{this->event_initializer};
    }

    explicit SetSubExecutorBuilder(EventInitializer<EventCtx> event_initializer) noexcept
      : event_initializer(event_initializer)
    {}

private:
    EventInitializer<EventCtx> event_initializer;
};

// for pretty error
class ExecutorCompleted {};

#define MK_SubExecutorBuilderFinal(i, mem)                                           \
    template<class EventCtx>                                                         \
    struct REDEMPTION_CXX_NODISCARD SetSubExecutorBuilder<EventCtx, false, i>        \
    {                                                                                \
        template<class F>                                                            \
        ExecutorResult mem(F&& f) &&                                                 \
        {                                                                            \
            this->event_initializer.init_##mem(static_cast<F&&>(f));                 \
            return detail::FriendExecutorResult::Nothing;                            \
        }                                                                            \
                                                                                     \
        SetSubExecutorBuilder(EventInitializer<EventCtx> event_initializer) noexcept \
        : event_initializer(event_initializer)                                       \
        {}                                                                           \
                                                                                     \
    private:                                                                         \
        EventInitializer<EventCtx> event_initializer;                                \
    };                                                                               \
                                                                                     \
    template<class EventCtx>                                                         \
    struct SetSubExecutorBuilder<EventCtx, true, i>                                  \
    {                                                                                \
        template<class F>                                                            \
        ExecutorCompleted mem(F&& f) &&                                              \
        {                                                                            \
            this->event_initializer.init_##mem(static_cast<F&&>(f));                 \
            return {};                                                               \
        }                                                                            \
                                                                                     \
        SetSubExecutorBuilder(EventInitializer<EventCtx> event_initializer) noexcept \
        : event_initializer(event_initializer)                                       \
        {}                                                                           \
                                                                                     \
    private:                                                                         \
        EventInitializer<EventCtx> event_initializer;                                \
    }

MK_SubExecutorBuilderFinal(6/*0b110*/, on_action);
MK_SubExecutorBuilderFinal(5/*0b101*/, on_timeout);
MK_SubExecutorBuilderFinal(3/*0b011*/, on_exit);

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
    template<class... Args>
    CXX_WARN_UNUSED_RESULT
    MakeSubExecutorBuilder<Args...>
    sub_executor(Args&&... args)
    {
        return MakeSubExecutorBuilder<Args...>{
            this->create_ctx_events(static_cast<Args&&>(args)...)};
    }

    template<class... Args>
    EventInitializer<detail::ctx_arg_type<Args...>> create_ctx_events(Args&&... args)
    {
        using Ctx = detail::ctx_arg_type<Args...>;
        this->events.emplace_back(
            this->ctx_memory.template create<Ctx>(static_cast<Args&&>(args)...));
        return {this->events.back()};
    }

    ExecutorResult result_sub_executor()
    {
        return ExecutorResult::Nothing;
    }

    ExecutorResult result_exit_failure()
    {
        return ExecutorResult::ExitFailure;
    }

    ExecutorResult result_exit_success()
    {
        return ExecutorResult::ExitSuccess;
    }

    template<class Ctx, class F>
    ExecutorResult result_replace_action(F&& f)
    {
        EventInitializer<Ctx>{this->events.back()}
          .init_on_action(static_cast<F&&>(f));
        return ExecutorResult::ReplaceAction;
    }

    template<class Ctx, class F>
    ExecutorResult result_exec_action(F&& f)
    {
        EventInitializer<Ctx>{this->events.back()}
          .init_on_action(static_cast<F&&>(f));
        return this->events.back().on_action(*this, *this->events.back().ctx);
    }

    template<class Ctx, class F>
    ExecutorResult result_replace_timeout(F&& f)
    {
        EventInitializer<Ctx>{this->events.back()}
          .init_on_timeout(static_cast<F&&>(f));
        return ExecutorResult::ReplaceTimeout;
    }

    template<class Ctx, class F>
    void set_action(F&& f)
    {
        EventInitializer<Ctx>{this->events.back()}
          .init_on_action(static_cast<F&&>(f));
    }

    template<class Ctx, class F>
    void set_timeout(F&& f)
    {
        EventInitializer<Ctx>{this->events.back()}
          .init_on_timeout(static_cast<F&&>(f));
    }

    ExecutorResult retry()
    {
        return ExecutorResult::Nothing;
    }

    using Event = ExecutorEvent;

    struct CtxMemory
    {
        template<class Ctx, class... Args>
        Event::CtxPtr create(Args&&... args)
        {
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
            // TODO intrusive_list
            auto * p = new Ctx{static_cast<Args&&>(args)...};
            REDEMPTION_DIAGNOSTIC_POP
            return Event::CtxPtr{
                reinterpret_cast<Event::any*>(p),
                Event::real_deleter{[](void* p){ delete static_cast<Ctx*>(p); }}
            };
        }

        template<class Ctx>
        Event::CtxPtr create()
        {
            return Event::CtxPtr{
                reinterpret_cast<Event::any*>(this),
                Event::real_deleter{[](void*){ }}
            };
        }
    };

    std::vector<Event> events;
    CtxMemory ctx_memory;
    Event garbage {Event::any{}};
};

struct Executor
{
    template<class... Args>
    CXX_WARN_UNUSED_RESULT
    MakeInitialSubExecutorBuilder<Args...>
    initial_executor(Args&&... args)
    {
        return MakeInitialSubExecutorBuilder<Args...>{
            this->base.create_ctx_events(static_cast<Args&&>(args)...)};
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

// TODO private implementation
struct ExecutorContext
{
    ExecutorResult retry()
    {
        return this->executor.retry();
    }

    ExecutorResult exit(ExitStatus status)
    {
        return (status == ExitStatus::Success) ? this->exit_on_success() : this->exit_on_error();
    }

    ExecutorResult exit_on_error()
    {
        return this->executor.result_exit_failure();
    }

    ExecutorResult exit_on_success()
    {
        return this->executor.result_exit_success();
    }

    template<class... Args>
    CXX_WARN_UNUSED_RESULT
    MakeSubExecutorBuilder<Args...>
    sub_executor(Args&&... args)
    {
        return this->executor.sub_executor(static_cast<Args&&>(args)...);
    }

    explicit ExecutorContext(ExecutorBase & executor) noexcept
      : executor(executor)
    {}

protected:
    ExecutorBase & executor;
};


template<class Ctx>
struct ExecutorTimeoutContext : ExecutorContext
{
    template<class F>
    ExecutorResult next_timeout(F&& f)
    {
        return this->executor.template result_replace_timeout<Ctx>(static_cast<F&&>(f));
    }

    template<class F>
    void set_action(F&& f)
    {
        this->executor.template set_action<Ctx>(static_cast<F&&>(f));
    }

    explicit ExecutorTimeoutContext(ExecutorBase& executor) noexcept
      : ExecutorContext(executor)
    {}
};

template<class Ctx>
struct ExecutorActionContext : ExecutorContext
{
    template<class F>
    ExecutorResult next_action(F&& f)
    {
        return this->executor.template result_replace_action<Ctx>(static_cast<F&&>(f));
    }

    template<class F>
    ExecutorResult exec_action(F&& f)
    {
        return this->executor.template result_exec_action<Ctx>(static_cast<F&&>(f));
    }

    template<class F>
    void set_timeout(F&& f)
    {
        this->executor.template set_timeout<Ctx>(static_cast<F&&>(f));
    }

    explicit ExecutorActionContext(ExecutorBase& executor) noexcept
      : ExecutorContext(executor)
    {}
};

template<class Ctx>
struct ExecutorExitContext : ExecutorContext
{
    template<class F>
    void set_action(F&& f)
    {
        return this->executor.template set_action<Ctx>(static_cast<F&&>(f));
    }

    template<class F>
    void set_timeout(F&& f)
    {
        this->executor.template set_timeout<Ctx>(static_cast<F&&>(f));
    }

    explicit ExecutorExitContext(ExecutorBase& executor) noexcept
      : ExecutorContext(executor)
    {}
};

bool Executor::exec()
{
    auto process_exit = [this](bool status) {
        while (!this->base.events.empty()) {
            ExecutorResult r = this->base.events.back().on_exit(
                this->base, status, *this->base.events.back().ctx);
            switch (r.process) {
                case ExecutorResult::ExitSuccess:
                    status = true;
                    this->base.events.pop_back();
                    break;
                case ExecutorResult::ExitFailure:
                    status = false;
                    this->base.events.pop_back();
                    break;
                case ExecutorResult::ReplaceAction:
                case ExecutorResult::ReplaceTimeout:
                case ExecutorResult::Nothing:
                    return;
            }
        }
    };

    ExecutorResult r = this->base.events.back().on_action(
        this->base, *this->base.events.back().ctx);
    switch (r.process) {
        case ExecutorResult::ExitSuccess:
            process_exit(true);
            break;
        case ExecutorResult::ExitFailure:
            process_exit(false);
            break;
        case ExecutorResult::ReplaceAction:
        case ExecutorResult::ReplaceTimeout:
        case ExecutorResult::Nothing:
            break;
    }

    return !this->base.events.empty();
}

template<class F>
F make_lambda() noexcept
{
    static_assert(
        std::is_empty<F>::value,
        "F must be an empty class or a lambda expression convertible to pointer of function");
    // big hack :)
    alignas(F) char const f[sizeof(F)]{}; // same as `char f`
    return reinterpret_cast<F const&>(f);
}

template<class Ctx>
template<class F>
void EventInitializer<Ctx>::init_on_action(F)
{
    this->executor_event.on_action = [](ExecutorBase& executor, ExecutorEvent::any& any){
        return reinterpret_cast<Ctx&>(any).invoke(
            make_lambda<F>(), ExecutorActionContext<Ctx>(executor));
    };
}

template<class Ctx>
template<class F>
void EventInitializer<Ctx>::init_on_exit(F)
{
    this->executor_event.on_exit = [](ExecutorBase& executor, bool success, ExecutorEvent::any& any){
        return reinterpret_cast<Ctx&>(any).invoke(
            make_lambda<F>(), ExecutorExitContext<Ctx>(executor), success);
    };
}

template<class Ctx>
template<class F>
void EventInitializer<Ctx>::init_on_timeout(F)
{
    this->executor_event.on_timeout = [](ExecutorBase& executor, ExecutorEvent::any& any){
        return reinterpret_cast<Ctx&>(any).invoke(
            make_lambda<F>(), ExecutorTimeoutContext<Ctx>(executor));
    };
}
