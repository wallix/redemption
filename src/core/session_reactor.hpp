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

#include "core/error.hpp"
#include "core/back_event_t.hpp"
#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/difftimeval.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <cassert>
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#ifndef NDEBUG
# define REDEMPTION_DEBUG_ONLY(...) __VA_ARGS__
#else
# define REDEMPTION_DEBUG_ONLY(...)
#endif

class SessionReactor;

namespace jln
{
    using Reactor = SessionReactor;

    template<class... Ts> class TopExecutor;
    template<class... Ts> class GroupExecutor;
    template<class... Ts> class ActionExecutor;
    template<class... Ts> class TimerExecutor;
    template<class T> class SharedData;
    class SharedPtr;
    template<class... Ts> class TopSharedPtr;
    template<class... Ts> class ActionSharedPtr;
    template<class... Ts> class TimerSharedPtr;

    enum class [[nodiscard]] R : char
    {
        Next,
        Terminate,
        ExitSuccess,
        ExitError,
        Exception,
        CreateGroup,
        NeedMoreData,
        SubstituteExit,
        SubstituteAction,
        SubstituteTimeout,
        Ready,
        ReRun,
        CreateContinuation,
    };

    struct ExitR
    {
        enum Status : char {
            Error = char(R::ExitError),
            Success = char(R::Next),
            Exception = char(R::Exception),
            Terminate = char(R::Terminate),
        };

        R to_result() const noexcept
        {
            return static_cast<R>(this->status);
        }

        Status status;
        ::Error& error;
    };

    enum class ExitStatus : bool {
        Error,
        Success,
    };

    template<class F>
    F make_lambda() noexcept
    {
        static_assert(
            std::is_empty<F>::value,
            "F must be an empty class or a lambda expression convertible to pointer of function");
        // big hack for a lambda not default constructible before C++20 :)
        alignas(F) char const f[sizeof(F)]{}; // same as `char f`
        return reinterpret_cast<F const&>(f); /*NOLINT*/
    }

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
            constexpr tuple_elem(int /*unused*/, tuple_impl<std::integer_sequence<size_t, ints...>, Ts...>& t)
            noexcept(noexcept(T{static_cast<Ts&&>(static_cast<tuple_elem<ints, Ts>&>(t).x)...}))
            : x{static_cast<Ts&&>(static_cast<tuple_elem<ints, Ts>&>(t).x)...}
            {}

            template<class... Ts>
            constexpr tuple_elem(emplace_type<T, Ts...> e)
            noexcept(noexcept(tuple_elem(1, e.t)))
            : tuple_elem(1, e.t)
            {}

            template<class U>
            constexpr tuple_elem(U&& x) /*NOLINT*/
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
    }  // namespace detail

    template<class T>
    using decay_and_strip_t = typename detail::decay_and_strip<T>::type;

    template<class T>
    constexpr auto emplace = detail::emplace_type<T>{};

    namespace detail
    {
        template<bool HasAct, bool HasExit, class Top, class Group>
        struct [[nodiscard]] GroupExecutorBuilderImpl
        {
            explicit GroupExecutorBuilderImpl(Top& top, std::unique_ptr<Group>&& g) noexcept;

#ifndef NDEBUG
            ~GroupExecutorBuilderImpl()
            {
                assert(!this->g);
            }
#endif

            template<class F>
            auto on_action(F&& f) &&;

            template<class F>
            auto on_exit(F&& f) &&;

        private:
            Top& top;
            std::unique_ptr<Group> g;
        };

        struct BuilderInit
        {
            enum E
            {
                None,
                Action = 1 << 0,
                Exit = 1 << 1,
                Timer = 1 << 2,
                Timeout = 1 << 3,
                NotifyDelete = 1 << 4,
            };

            static constexpr bool has(int f, int mask) noexcept
            {
                return (f & mask) == mask;
            }
        };

        template<BuilderInit::E Has, class InitCtx>
        struct [[nodiscard]] TopExecutorBuilderImpl
        {
            explicit TopExecutorBuilderImpl(InitCtx&& init_ctx) noexcept;

            template<class F>
            auto on_action(F&& f) &&;

            template<class F>
            auto on_exit(F&& f) &&;

            template<class F>
            auto on_timeout(F&& f) &&;

            auto set_timeout(std::chrono::milliseconds ms) &&;

            auto disable_timeout() &&;

            auto propagate_exit() &&;

            template<class F>
            auto set_notify_delete(F&& /*f*/) && noexcept;

        private:
            InitCtx init_ctx;
        };

        template<BuilderInit::E Has, class InitCtx>
        struct [[nodiscard]] TimerExecutorBuilderImpl
        {
            explicit TimerExecutorBuilderImpl(InitCtx&& /*init_ctx*/) noexcept;

            template<class F>
            auto on_action(F&& f) &&;

            auto set_delay(std::chrono::milliseconds ms) &&;
            auto set_time(timeval tv) &&;

            template<class F>
            auto set_notify_delete(F&& /*f*/) && noexcept;

        private:
            InitCtx init_ctx;
        };

        template<BuilderInit::E Has, class InitCtx>
        struct [[nodiscard]] ActionExecutorBuilderImpl
        {
            explicit ActionExecutorBuilderImpl(InitCtx&& init_ctx) noexcept;

            template<class F>
            auto on_action(F&& f) &&;

            template<class F>
            auto set_notify_delete(F&& /*f*/) && noexcept;

        private:
            InitCtx init_ctx;
        };

    #ifdef IN_IDE_PARSER
        struct Func
        {
            template<class F>
            Func(F) {}
        };

        struct /*[[nodiscard]]*/ GroupExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit GroupExecutorBuilder_Concept(Ts&&...) noexcept;

            GroupExecutorBuilder_Concept on_action(Func);
            GroupExecutorBuilder_Concept on_exit(Func);
            GroupExecutorBuilder_Concept propagate_exit();

            GroupExecutorBuilder_Concept set_notify_delete(Func);

            operator R ();
        };

        struct /*[[nodiscard]]*/ TopExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit TopExecutorBuilder_Concept(Ts&&...) noexcept;

            TopExecutorBuilder_Concept disable_timeout();
            TopExecutorBuilder_Concept set_timeout(std::chrono::milliseconds);
            TopExecutorBuilder_Concept on_timeout(Func);
            TopExecutorBuilder_Concept on_action(Func);
            TopExecutorBuilder_Concept on_exit(Func);
            TopExecutorBuilder_Concept propagate_exit();

            TopExecutorBuilder_Concept set_notify_delete(Func);

            operator SharedPtr ();

            template<class... Ts> operator TopSharedPtr<Ts...> ();
        };

        struct /*[[nodiscard]]*/ TimerExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit TimerExecutorBuilder_Concept(Ts&&...) noexcept;

            TimerExecutorBuilder_Concept on_action(Func);

            TimerExecutorBuilder_Concept set_delay(std::chrono::milliseconds ms);
            TimerExecutorBuilder_Concept set_time(timeval tv);

            TimerExecutorBuilder_Concept set_notify_delete(Func);

            operator SharedPtr ();

            template<class... Ts> operator TimerSharedPtr<Ts...> ();
        };

        struct /*[[nodiscard]]*/ ActionExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit ActionExecutorBuilder_Concept(Ts&&...) noexcept;

            ActionExecutorBuilder_Concept on_action(Func);
            ActionExecutorBuilder_Concept set_notify_delete(Func);

            template<class... Ts> operator ActionSharedPtr<Ts...> ();
        };

        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilder_Concept;

        template<class InitCtx>
        using TopExecutorBuilder = TopExecutorBuilder_Concept;

        template<class InitCtx>
        using TimerExecutorBuilder = TimerExecutorBuilder_Concept;

        template<class InitCtx>
        using ActionExecutorBuilder = ActionExecutorBuilder_Concept;
    #else
        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilderImpl<false, false, Top, Group>;

        template<class InitCtx>
        using TopExecutorBuilder = TopExecutorBuilderImpl<BuilderInit::None, InitCtx>;

        template<class InitCtx>
        using TimerExecutorBuilder = TimerExecutorBuilderImpl<BuilderInit::None, InitCtx>;

        template<class InitCtx>
        using ActionExecutorBuilder = ActionExecutorBuilderImpl<BuilderInit::None, InitCtx>;
    #endif
    }  // namespace detail


#ifdef IN_IDE_PARSER
# define REDEMPTION_JLN_CONCEPT(C) C
#else
# define REDEMPTION_JLN_CONCEPT(C) auto
#endif

    enum class NextMode { ChildToNext, CreateContinuation, };

    template<class... Ts>
    struct GroupContext
    {
        GroupContext(TopExecutor<Ts...>& top, GroupExecutor<Ts...>& current_group) noexcept
        : top(top)
        , current_group(current_group)
        {}

        template<class... Us>
        REDEMPTION_JLN_CONCEPT(detail::GroupExecutorBuilder_Concept)
        create_sub_executor(Us&&... /*xs*/);

        R exception(Error const& e) noexcept;
        R ready() noexcept { return R::Ready; }
        R need_more_data() noexcept { return R::NeedMoreData; }
        R terminate() noexcept { return R::Terminate; }
        R next() noexcept { return R::Next; }
        R exit_on_error() noexcept { return R::ExitError; }
        R exit_on_success() noexcept { return R::ExitSuccess; }
        R exit(ExitStatus status) noexcept {
            return (status == ExitStatus::Success)
                ? this->exit_on_success()
                : this->exit_on_error();
        }

        GroupContext& enable_timeout(bool enable = true) noexcept;
        GroupContext& disable_timeout() noexcept
        {
            this->enable_timeout(false);
            return *this;
        }

        int get_fd() const noexcept;
        void set_fd(int fd) noexcept;

        Reactor& get_reactor() const noexcept;

        timeval get_current_time() const noexcept;

        // void set_timeout(std::chrono::milliseconds ms) noexcept
        // {
        //     this->basic_fd.set_timeout(ms);
        // }
        //
        // void restart_timeout()
        // {
        //     this->basic_fd.restart_timeout();
        // }

    protected:
        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;
    };

    template<class Tuple, class... Ts>
    struct TopContext : GroupContext<Ts...>
    {
        template<class F>
        R replace_action(F&& f);

        template<class F>
        TopContext& replace_exit(F&& f);

        TopContext& enable_timeout(bool enable = true) noexcept
        {
            this->GroupContext<Ts...>::enable_timeout(enable);
            return *this;
        }

        TopContext& disable_timeout() noexcept
        {
            this->GroupContext<Ts...>::disable_timeout();
            return *this;
        }
    };

    template<class... Ts>
    struct TimerContext
    {
        R next() noexcept { return R::Next; }
        R ready() noexcept { return R::Ready; }
        R terminate() noexcept { return R::Terminate; }

        TimerContext& set_delay(std::chrono::milliseconds ms) noexcept
        {
            timer.set_delay(ms);
            return *this;
        }

        TimerContext& set_time(timeval tv) noexcept
        {
            timer.set_time(tv);
            return *this;
        }

        R ready_to(std::chrono::milliseconds ms) noexcept
        {
            timer.set_delay(ms);
            return R::Ready;
        }

        R ready_at(timeval tv) noexcept
        {
            timer.set_time(tv);
            return R::Ready;
        }

        Reactor& get_reactor() const noexcept;

        timeval get_current_time() const noexcept;

        TimerContext(TimerExecutor<Ts...>& timer) noexcept
        : timer(timer)
        {}

    private:
        TimerExecutor<Ts...>& timer;
    };

    template<class... Ts>
    struct ActionContext
    {
        R next() noexcept { return R::Next; }
        R ready() noexcept { return R::Ready; }
        R terminate() noexcept { return R::Terminate; }
    };

    template<class... Ts>
    struct ExitContext
    {
        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;

        // R need_more_data() noexcept { return R::NeedMoreData; }
        // R terminate() noexcept { return R::Terminate; }
        // R next() noexcept { return R::Next; }
        // R exit_on_error() noexcept { return R::ExitError; }
        // R exit_on_success() noexcept { return R::ExitSuccess; }
        // R exit(ExitStatus status) noexcept {
        //     return (status == ExitStatus::Success)
        //         ? this->exit_on_success()
        //         : this->exit_on_error();
        // }

        // template<class F>
        // R replace_action(F&& f);
    };

    // TODO GroupTimerContext
    template<class... Ts>
    struct GroupTimerContext
    {
        R exception(Error const& e) noexcept;
        R ready() noexcept { return R::Ready; }
        R terminate() noexcept { return R::Terminate; }
        R exit_on_error() noexcept { return R::ExitError; }
        R exit_on_success() noexcept { return R::ExitSuccess; }
        R exit(ExitStatus status) noexcept {
            return (status == ExitStatus::Success)
                ? this->exit_on_success()
                : this->exit_on_error();
        }

        GroupTimerContext& disable_timeout() noexcept;

        int get_fd() const noexcept;
        void set_fd(int fd) noexcept;

        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;
    };

    template<class Tuple, class... Ts>
    struct TopTimerContext : GroupTimerContext<Ts...>
    {
        template<class F>
        TopTimerContext& replace_action(F&& f);

        template<class F>
        TopTimerContext& replace_exit(F&& f);

        template<class F>
        R replace_timeout(F&& f);

        template<class F>
        R set_or_disable_timeout(std::chrono::milliseconds ms, F&& f);

        TopTimerContext& disable_timeout() noexcept
        {
            GroupTimerContext<Ts...>::disable_timeout();
            return *this;
        }
    };

#ifdef IN_IDE_PARSER
# define JLN_GROUP_CTX ::jln::GroupContext<>
# define JLN_TOP_CTX ::jln::TopContext < ::jln::detail::tuple<>>
# define JLN_TIMER_CTX ::jln::TimerContext<>
# define JLN_ACTION_CTX ::jln::ActionContext<>
# define JLN_EXIT_CTX ::jln::ExitContext<>
# define JLN_GROUP_TIMER_CTX ::jln::GroupTimerContext<>
# define JLN_TOP_TIMER_CTX ::jln::TopTimerContext < ::jln::detail::tuple<>>
#else
# define JLN_GROUP_CTX auto
# define JLN_TOP_CTX auto
# define JLN_TIMER_CTX auto
# define JLN_ACTION_CTX auto
# define JLN_EXIT_CTX auto
# define JLN_GROUP_TIMER_CTX auto
# define JLN_TOP_TIMER_CTX auto
#endif

    template<class... Ts>
    struct TimerData
    {
        bool is_enabled = true;
        timeval tv {};
        std::chrono::milliseconds delay = std::chrono::milliseconds(-1);
        std::function<R(GroupTimerContext<Ts...>, Ts...)> on_timeout;

        void initialize_delay(std::chrono::milliseconds ms)
        {
            // TODO initialize tv
            this->delay = ms;
        }
    };


    template<class... Ts>
    struct GroupExecutor
    {
        std::function<R(GroupContext<Ts...>, Ts...)> on_action;
        std::function<R(ExitContext<Ts...>, ExitR er, Ts...)> on_exit;
        NextMode next_mode = NextMode::ChildToNext;
        GroupExecutor* next;

        void delete_self() noexcept
        {
            this->deleter(this);
        }

    protected:
        using DeleterFun = void(*)(GroupExecutor*) noexcept;

        GroupExecutor(DeleterFun deleter) noexcept
        : deleter(deleter)
        {}

        DeleterFun deleter;
    };

    template<class Tuple, class... Ts>
    struct GroupExecutorWithValues final : GroupExecutor<Ts...>
    {
        using Base = GroupExecutor<Ts...>;

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        GroupExecutorWithValues(Us&&... xs)
        : GroupExecutor<Ts...>([](GroupExecutor<Ts...>* p) noexcept{
            delete static_cast<GroupExecutorWithValues*>(p); /*NOLINT*/
        })
        , t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f, this](GroupContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(
                    f, TopContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
            };
        }

        template<class F>
        void on_exit(F&& f)
        {
            Base::on_exit = [f, this](ExitContext<Ts...> ctx, ExitR er, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(f, ctx, er, static_cast<Ts&>(xs)...);
            };
        }

        Tuple t;

    protected:
        ~GroupExecutorWithValues() = default;
    };

    template<template<class...> class Tuple, class... Ts>
    struct GroupExecutorWithValues<Tuple<>, Ts...> final : GroupExecutor<Ts...>
    {
        using Base = GroupExecutor<Ts...>;

        GroupExecutorWithValues() noexcept
        : GroupExecutor<Ts...>([](GroupExecutor<Ts...>* p) noexcept{
            delete static_cast<GroupExecutorWithValues*>(p); /*NOLINT*/
        })
        {}

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f](GroupContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                return f(TopContext<Tuple<>, Ts...>{ctx}, static_cast<Ts&>(xs)...);
            };
        }

        template<class F>
        void on_exit(F&& f)
        {
            Base::on_exit = static_cast<F&&>(f);
        }

    protected:
        ~GroupExecutorWithValues() = default;
    };

    template<class... Ts>
    using GroupExecutorDefault = GroupExecutorWithValues<detail::tuple<>, Ts...>;


    template<class... Ts>
    struct TimerExecutor
    {
        std::function<R(TimerContext<Ts...>, Ts...)> on_timer;
        timeval tv {};
        std::chrono::milliseconds delay = std::chrono::milliseconds(-1);
        SessionReactor& reactor;

        TimerExecutor(SessionReactor& reactor) noexcept
        : reactor(reactor)
        {}

        Reactor& get_reactor() const noexcept
        {
            return this->reactor;
        }

        void set_delay(std::chrono::milliseconds ms) noexcept;

        void set_time(timeval tv) noexcept
        {
            this->delay = std::chrono::milliseconds(-1);
            this->tv = tv;
        }

        bool exec_action(Ts&... xs)
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            switch (this->on_timer(TimerContext<Ts...>{*this}, xs...)) {
                case R::Terminate:
                case R::Next:
                    return false;
                case R::Ready:
                    assert(this->delay.count() >= 0);
                    this->set_delay(this->delay);
                    return true;
                case R::ReRun:
                    REDEMPTION_UNREACHABLE();
                case R::Exception:
                    REDEMPTION_UNREACHABLE();
                case R::ExitSuccess:
                    REDEMPTION_UNREACHABLE();
                case R::ExitError:
                    REDEMPTION_UNREACHABLE();
                case R::NeedMoreData:
                    REDEMPTION_UNREACHABLE();
                case R::CreateGroup:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteTimeout:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteAction:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteExit:
                    REDEMPTION_UNREACHABLE();
                case R::CreateContinuation:
                    REDEMPTION_UNREACHABLE();
            }

            REDEMPTION_UNREACHABLE();
            return false;
        }

    private:
        REDEMPTION_DEBUG_ONLY(bool exec_is_running = false;)
    };

    template<class Tuple, class... Ts>
    struct TimerExecutorWithValues final : TimerExecutor<Ts...>
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        TimerExecutorWithValues(SessionReactor& reactor, Us&&... xs)
        : TimerExecutor<Ts...>(reactor)
        , t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            this->on_timer = [f, this](TimerContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(f, ctx, static_cast<Ts&>(xs)...);
            };
        }

        template<class F, class... Args>
        void invoke(F&& f, Args&&... args)
        noexcept(noexcept(std::declval<Tuple&>().invoke(
            static_cast<F&&>(f), static_cast<Args&&>(args)...)))
        {
            this->t.invoke(static_cast<F&&>(f), static_cast<Args&&>(args)...);
        }

        Tuple t;
    };

    template<template<class...> class Tuple, class... Ts>
    struct TimerExecutorWithValues<Tuple<>, Ts...> final : TimerExecutor<Ts...>
    {
        using Base = TimerExecutor<Ts...>;
        using Base::Base;

        template<class F>
        void on_action(F&& f)
        {
            this->on_timer = static_cast<F&&>(f);
        }

        template<class F, class... Args>
        void invoke(F&& f, Args&&... args)
        FALCON_RETURN_NOEXCEPT(static_cast<F&&>(f)(static_cast<Args&&>(args)...))
    };


    template<class... Ts>
    struct ActionExecutor
    {
        ActionExecutor() = default;
        ActionExecutor(const ActionExecutor&) = delete;
        ActionExecutor& operator=(const ActionExecutor&) = delete;

        std::function<R(ActionContext<Ts...>, Ts...)> on_action;

        bool exec_action(Ts&... xs)
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            switch (this->on_action(ActionContext<Ts...>{}, xs...)) {
                case R::Terminate:
                case R::Next:
                    return false;
                case R::Ready:
                    return true;
                case R::ReRun:
                    REDEMPTION_UNREACHABLE();
                case R::Exception:
                    REDEMPTION_UNREACHABLE();
                case R::ExitSuccess:
                    REDEMPTION_UNREACHABLE();
                case R::ExitError:
                    REDEMPTION_UNREACHABLE();
                case R::NeedMoreData:
                    REDEMPTION_UNREACHABLE();
                case R::CreateGroup:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteTimeout:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteAction:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteExit:
                    REDEMPTION_UNREACHABLE();
                case R::CreateContinuation:
                    REDEMPTION_UNREACHABLE();
            }

            REDEMPTION_UNREACHABLE();
            return false;
        }

    private:
        REDEMPTION_DEBUG_ONLY(bool exec_is_running = false;)
    };

    template<class Tuple, class... Ts>
    struct ActionExecutorWithValues final : ActionExecutor<Ts...>
    {
        using Base = ActionExecutor<Ts...>;

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        ActionExecutorWithValues(Us&&... xs)
        : t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f, this](ActionContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(f, ctx, static_cast<Ts&>(xs)...);
            };
        }

        template<class F, class... Args>
        void invoke(F&& f, Args&&... args)
        noexcept(noexcept(std::declval<Tuple&>().invoke(
            static_cast<F&&>(f), static_cast<Args&&>(args)...)))
        {
            this->t.invoke(static_cast<F&&>(f), static_cast<Args&&>(args)...);
        }

        Tuple t;
    };

    template<template<class...> class Tuple, class... Ts>
    struct ActionExecutorWithValues<Tuple<>, Ts...> final : ActionExecutor<Ts...>
    {
        using Base = ActionExecutor<Ts...>;

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = static_cast<F&&>(f);
        }

        template<class F, class... Args>
        void invoke(F&& f, Args&&... args)
        FALCON_RETURN_NOEXCEPT(static_cast<F&&>(f)(static_cast<Args&&>(args)...))
    };

    template<class F>
    auto one_shot(F f)
    {
        return [f](auto ctx, auto&&... xs) mutable {
            f(static_cast<decltype(xs)&&>(xs)...);
            return ctx.terminate();
        };
    }

    template<class F>
    auto always_ready(F f)
    {
        return [f](auto ctx, auto&&... xs) mutable {
            f(static_cast<decltype(xs)&&>(xs)...);
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
                else { /*NOLINT*/
                    (o.*f)(static_cast<Args&&>(args)...);
                }
            }
            else { /*NOLINT*/
                f(static_cast<T&&>(o), static_cast<Args&&>(args)...);
            }
        }

        template<auto f>
        void invoke()
        {
            f();
        }
    }  // namespace detail

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


    namespace detail
    {
        template<class S, class F>
        struct named_function
        {
            F f;

            template<class... Ts>
            decltype(auto) operator()(Ts&&... xs)
            {
                return this->f(static_cast<Ts&&>(xs)...);
            }

            template<class... Ts>
            decltype(auto) operator()(Ts&&... xs) const
            {
                return this->f(static_cast<Ts&&>(xs)...);
            }
        };

        template<class S>
        struct named_type
        {
            template<class F>
            named_function<S, std::decay_t<F>> operator()(F&& f) const noexcept
            {
                return {static_cast<F&&>(f)};
            }

            template<class F>
            named_function<S, std::decay_t<F>> operator=(F&& f) const noexcept /*NOLINT*/
            {
                return {static_cast<F&&>(f)};
            }
        };

        struct unamed{};

        template<class I, class S>
        struct named_indexed
        {
            static I index() noexcept { return I{}; }
            static S name() noexcept { return S{}; }
        };

        template<class S, class I>
        named_indexed<I, S> named_indexed_by_name(named_indexed<I, S> x) noexcept
        {
            return x;
        }

        template<class i, class F>
        struct function_to_named_index
        {
            using type = named_indexed<i, unamed>;
        };

        template<class i, class S, class F>
        struct function_to_named_index<i, named_function<S, F>>
        {
            using type = named_indexed<i, S>;
        };
    }  // namespace detail

    namespace literals
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
        template<class C, C... cs>
        detail::named_type<string_c<cs...>> operator ""_f () noexcept
        { return {}; }
        REDEMPTION_DIAGNOSTIC_POP
    }

    template<auto x>
    constexpr auto value = std::integral_constant<decltype(x), x>{};

    enum class IndexSequence : int;

    template<class Ctx, class NamedIndexPack>
    struct FuncSequencerCtx
    {
        Ctx& base() noexcept
        {
            return this->ctx;
        }

        R terminate() const noexcept
        {
            return R::Terminate;
        }

        bool is_final_sequence() const noexcept
        {
            return this->i == int{NamedIndexPack::count} - 1;
        }

        IndexSequence index() const noexcept
        {
            return IndexSequence(this->i);
        }

        char const* sequence_name() const noexcept
        {
            return NamedIndexPack::strings[this->i];
        }

        R ready() noexcept
        {
            return R::Ready;
        }

        R next() noexcept
        {
            return this->ctx.next();
        }

        Ctx& previous() noexcept
        {
            return this->at(this->i-1);
        }

        R exec_next() noexcept
        {
            return this->exec_at(this->i+1);
        }

        R exec_previous() noexcept
        {
            return this->exec_at(this->i-1);
        }

        Ctx& at(int i) noexcept
        {
            // assert(i >= 0);
            assert(i < int{NamedIndexPack::count});
            this->i = i;
            return this->ctx;
        }

        template<char... cs>
        Ctx& at(string_c<cs...> /*unused*/) noexcept
        {
            this->i = detail::named_indexed_by_name<string_c<cs...>>(NamedIndexPack{}).index();
            return this->ctx;
        }

        R exec_at(int i) noexcept
        {
            this->i = i;
            return R::ReRun;
        }

        template<char... cs>
        R exec_at(string_c<cs...> /*unused*/) noexcept
        {
            this->i = detail::named_indexed_by_name<string_c<cs...>>(NamedIndexPack{}).index();
            return R::ReRun;
        }

        FuncSequencerCtx& set_delay(std::chrono::milliseconds ms) noexcept
        {
            (void)this->ctx.set_delay(ms);
            return *this;
        }

        FuncSequencerCtx& set_time(std::chrono::milliseconds ms) noexcept
        {
            (void)this->ctx.set_time(ms);
            return *this;
        }

        FuncSequencerCtx(Ctx& ctx, unsigned& i) noexcept
        : ctx(ctx)
        , i(i)
        {}

        operator Ctx& () noexcept { return this->ctx; }

    private:
        Ctx& ctx;
        unsigned& i;
    };

#ifdef IN_IDE_PARSER
    namespace detail { struct UnknownCtx : ActionContext<> {}; }
# define JLN_FUNCSEQUENCER_CTX ::jln::FuncSequencerCtx< \
    ::jln::detail::UnknownCtx,                          \
    ::jln::detail::named_indexed_pack<                  \
        ::jln::detail::named_indexed<                   \
            ::std::integral_constant<std::size_t, 0>,   \
            ::jln::detail::unamed>>>
#else
# define JLN_FUNCSEQUENCER_CTX auto
#endif

    namespace detail
    {
        template<class> struct name_or_index;

        template<class i, class S>
        struct name_or_index<named_indexed<i, S>>
        { using type = S; };

        template<class i>
        struct name_or_index<named_indexed<i, unamed>>
        { using type = i; };

        template<class... Ts>
        struct CheckUniqueName : name_or_index<Ts>::type...
        {
            static const bool value = true;
        };

        template<class... NamedIndexed>
        struct named_indexed_pack : NamedIndexed...
        {
            static_assert(CheckUniqueName<NamedIndexed...>::value, "name duplicated");

            static const std::size_t count = sizeof...(NamedIndexed);

            static inline char const* const strings[sizeof...(NamedIndexed)]
                = {NamedIndexed::name().c_str()...};
        };

        template<class Ints, class... Fs>
        struct create_named_indexed_pack;

        template<std::size_t... Ints, class... Fs>
        struct create_named_indexed_pack<std::integer_sequence<std::size_t, Ints...>, Fs...>
        {
            using type = named_indexed_pack<typename function_to_named_index<
                std::integral_constant<std::size_t, Ints>,  Fs>::type...>;
        };


        template<std::size_t... Ints, class... Fs>
        R switch_(unsigned i, std::integer_sequence<std::size_t, Ints...> /*unused*/, Fs&&... fs)
        {
            R r;
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wparentheses")
            REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-value")
            (((i == Ints) && ((void)(r = fs()), true)) || ...)
            || ((void)(r = R::ExitError), true);
            REDEMPTION_DIAGNOSTIC_POP
            return r;
        }
    }  // namespace detail

    template<class S, class F>
    detail::named_function<S, F> named(S /*unused*/, F /*unused*/)
    { return {}; }

    template<class... Fs>
    auto sequencer(Fs&&... fs)
    {
        static_assert(sizeof...(Fs) > 1);

        using Pack = typename detail::create_named_indexed_pack<
            std::index_sequence_for<Fs...>, std::decay_t<Fs>...>::type;

        return [=, i = 0u](auto ctx, auto&&... xs) mutable /*-> R*/ {
            // gcc < 8.0 bug: explicit capture otherwise xs is misaligned
            auto wrap = [&i, &ctx, &xs...](auto& f) {
                return [&]() {
                    return f(
                        FuncSequencerCtx<decltype(ctx), Pack>{ctx, i},
                        static_cast<decltype(xs)&&>(xs)...
                    );
                };
            };

            for (;;) {
                R const r = detail::switch_(
                    i, std::make_index_sequence<sizeof...(Fs)>{}, wrap(fs)...);

                switch (r) {
                    case R::ReRun:
                        break;
                    case R::Next:
                        return i < sizeof...(fs)-1 ? ((void)++i, R::Ready) : R::Next;
                    case R::CreateGroup:
                        ++i;
                        return R::CreateContinuation;
                    case R::SubstituteTimeout:
                    case R::SubstituteAction:
                    case R::SubstituteExit:
                        ++i;
                        return r;
                    default:
                        return r;
                }
            }
        };
    }

    template<class F>
    auto propagate_exit(F&& f) noexcept
    {
        return [=](auto /*ctx*/, ExitR er, auto&&... xs) mutable /*-> R*/ {
            f(static_cast<decltype(xs)&&>(xs)...);
            return er.to_result();
        };
    }

    inline auto propagate_exit() noexcept
    {
        return [](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs) /*-> R*/ {
            return er.to_result();
        };
    }

    template<error_type id, int errnum = 0>
    inline auto exit_with_error()
    {
        return [](auto ctx, [[maybe_unused]] auto&&... xs) /*-> R*/ {
            return ctx.exception(Error{id, errnum});
        };
    }

    template<class... Ts>
    struct GroupDeleter
    {
        void operator()(GroupExecutor<Ts...>* p) noexcept
        {
            p->delete_self();
        }
    };

    template<class... Ts>
    struct ActionDeleter
    {
        void operator()(ActionExecutor<Ts...>* p) noexcept
        {
            p->delete_self();
        }
    };

    template<class... Ts>
    struct TimerDeleter
    {
        void operator()(TimerExecutor<Ts...>* p) noexcept
        {
            p->delete_self();
        }
    };

    template<class... Ts>
    struct TopExecutor
    {
        using GroupPtr = std::unique_ptr<GroupExecutor<Ts...>, GroupDeleter<Ts...>>;

        TopExecutor(Reactor& reactor, int fd)
        : fd(fd)
        , reactor(reactor)
        {}

        ~TopExecutor()
        {
            assert(!this->exec_is_running);

            auto* p = this->group;
            while (p) {
                std::exchange(p, p->next)->delete_self();
            }
        }

        void set_fd(int fd) noexcept
        {
            assert(fd >= 0);
            this->fd = fd;
        }

        int get_fd() const noexcept
        {
            return this->fd;
        }

        Reactor& get_reactor() const noexcept
        {
            return this->reactor;
        }

        void set_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->timer_data.delay = ms;
            this->update_next_time();
        }

        void update_next_time() noexcept;

        void disable_timeout() noexcept
        {
            this->enable_timeout(false);
        }

        void enable_timeout(bool enable = true) noexcept
        {
            this->timer_data.is_enabled = enable;
        }

        void set_delay(std::chrono::milliseconds ms) noexcept
        {
            this->timer_data.delay = ms;
        }

        void set_time(timeval const& tv) noexcept
        {
            this->timer_data.tv = tv;
        }

        void add_group(GroupPtr&& group)
        {
            group->next = this->group;
            this->group = group.release();
        }

        void sub_group(GroupPtr&& group)
        {
            this->loaded_group = std::move(group);
        }

        bool exec_timeout(Ts&... xs)
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            R r;

            try {
                auto& on_timeout = this->timer_data.on_timeout;
                do {
                    switch ((r = on_timeout(GroupTimerContext<Ts...>{*this, *this->group}, xs...))) {
                        case R::Terminate:
                        case R::Exception:
                        case R::ExitError:
                        case R::ExitSuccess:
                        case R::Next:
                            r = this->_exec_exit(r, xs...);
                            break;
                        case R::NeedMoreData:
                        case R::Ready:
                            break;
                        case R::SubstituteTimeout:
                            on_timeout = std::move(this->on_timeout_switch);
                            r = R::Ready;
                            break;
                        case R::ReRun:
                            REDEMPTION_UNREACHABLE();
                        case R::CreateContinuation:
                            REDEMPTION_UNREACHABLE();
                        case R::CreateGroup:
                            REDEMPTION_UNREACHABLE();
                        case R::SubstituteAction:
                            REDEMPTION_UNREACHABLE();
                        case R::SubstituteExit:
                            REDEMPTION_UNREACHABLE();
                    }
                } while(false);
            }
            catch (Error const& e) {
                this->error = e;
                r = this->_exec_exit(R::Exception, xs...);
            }

            switch (r) {
                case R::Exception:
                    throw this->error;
                case R::ExitSuccess:
                case R::Terminate:
                case R::ExitError:
                case R::Next:
                    return false;
                case R::NeedMoreData:
                case R::Ready:
                    return true;
                case R::ReRun:
                    REDEMPTION_UNREACHABLE();
                case R::CreateGroup:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteTimeout:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteAction:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteExit:
                    REDEMPTION_UNREACHABLE();
                case R::CreateContinuation:
                    REDEMPTION_UNREACHABLE();
            }

            REDEMPTION_UNREACHABLE();
            return false;
        }

        bool exec_action(Ts&... xs)
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            R r;

            try {
                do {
                    switch ((r = this->_exec_action(xs...))) {
                        case R::ExitSuccess:
                        case R::Exception:
                        case R::Terminate:
                        case R::ExitError:
                        case R::NeedMoreData:
                            break;
                        case R::Ready:
                        case R::Next:
                            continue;
                        case R::ReRun:
                            REDEMPTION_UNREACHABLE();
                        case R::CreateContinuation:
                            REDEMPTION_UNREACHABLE();
                        case R::CreateGroup:
                            REDEMPTION_UNREACHABLE();
                        case R::SubstituteTimeout:
                            REDEMPTION_UNREACHABLE();
                        case R::SubstituteAction:
                            REDEMPTION_UNREACHABLE();
                        case R::SubstituteExit:
                            REDEMPTION_UNREACHABLE();
                    }
                } while(false);
            }
            catch (Error const& e) {
                this->error = e;
                r = this->_exec_exit(R::Exception, xs...);
            }

            switch (r) {
                case R::Exception:
                    throw this->error;
                case R::ExitSuccess:
                case R::Terminate:
                case R::ExitError:
                case R::Next:
                    return false;
                case R::NeedMoreData:
                case R::Ready:
                    return true;
                case R::ReRun:
                    REDEMPTION_UNREACHABLE();
                case R::CreateGroup:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteTimeout:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteAction:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteExit:
                    REDEMPTION_UNREACHABLE();
                case R::CreateContinuation:
                    REDEMPTION_UNREACHABLE();
            }

            REDEMPTION_UNREACHABLE();
            return false;
        }

    private:
        R _exec_action(Ts&... xs)
        {
            R const r = this->group->on_action(GroupContext<Ts...>{*this, *this->group}, xs...);
            switch (r) {
                case R::Terminate:
                case R::Exception:
                case R::ExitError:
                case R::ExitSuccess:
                case R::Next:
                    return this->_exec_exit(r, xs...);
                case R::NeedMoreData:
                case R::Ready:
                    return r;
                case R::CreateContinuation:
                    this->loaded_group->next = this->group;
                    this->group = this->loaded_group.release();
                    this->group->next_mode = NextMode::CreateContinuation;
                    return R::Ready;
                case R::CreateGroup:
                    this->loaded_group->next = this->group;
                    this->group = this->loaded_group.release();
                    return R::Ready;
                case R::SubstituteExit:
                    REDEMPTION_UNREACHABLE();
                    // this->group->on_exit = std::move(this->loaded_group->on_exit);
                    // this->loaded_group.reset();
                    // return R::Ready;
                case R::SubstituteAction:
                    this->group->on_action = std::move(this->loaded_group->on_action);
                    this->loaded_group.reset();
                    return R::Ready;
                case R::ReRun:
                    REDEMPTION_UNREACHABLE();
                case R::SubstituteTimeout:
                    REDEMPTION_UNREACHABLE();
                    // return R::Ready;
            }
            REDEMPTION_UNREACHABLE();
            return R::Terminate;
        }

        R _exec_exit(R r, Ts&... xs)
        {
            do {
                R const re = this->group->on_exit(
                    ExitContext<Ts...>{*this, *this->group},
                    ExitR{static_cast<ExitR::Status>(r), this->error},
                    xs...);
                NextMode next_mode;
                switch (re) {
                    case R::ExitSuccess:
                    case R::Next:
                        next_mode = this->group->next_mode;
                        std::exchange(this->group, this->group->next)->delete_self();
                        switch (next_mode) {
                            case NextMode::ChildToNext:
                                r = re;
                                break;
                            case NextMode::CreateContinuation:
                                return this->group ? R::Ready : R::Terminate;
                        }
                        break;
                    case R::Exception:
                    case R::Terminate:
                    case R::ExitError:
                        std::exchange(this->group, this->group->next)->delete_self();
                        r = re;
                        break;
                    case R::Ready:
                    case R::NeedMoreData:
                        return re;
                    case R::CreateGroup:
                    case R::SubstituteExit:
                    case R::SubstituteAction:
                    case R::SubstituteTimeout:
                        return R::Ready;
                    case R::ReRun:
                        REDEMPTION_UNREACHABLE();
                    case R::CreateContinuation:
                        REDEMPTION_UNREACHABLE();
                }
            } while (this->group);

            return r;
        }

        int fd;
        GroupExecutor<Ts...>* group = nullptr;
        GroupPtr loaded_group;
        Reactor& reactor;

        REDEMPTION_DEBUG_ONLY(bool exec_is_running = false;)

    public: // TODO to private
        TimerData<Ts...> timer_data;

    // private:
        std::function<R(GroupTimerContext<Ts...>, Ts...)> on_timeout_switch;

    public:
        Error error = Error(NO_ERROR);
    };

    enum class NotifyDeleteType
    {
        DeleteByDtor,
        DeleteByAction,
    };

    struct SharedDataBase
    {
        enum class FreeCat {
            ValueByWrapper = int(NotifyDeleteType::DeleteByDtor),
            Value = int(NotifyDeleteType::DeleteByAction),
            Self,
        };
        SharedPtr* shared_ptr;

    protected:
        void (*deleter) (SharedDataBase*, FreeCat) noexcept;

    public:
        SharedDataBase* next;

        void free_value() noexcept
        {
            this->deleter(this, FreeCat::Value);
        }

        void free_by_wrapper() noexcept
        {
            this->deleter(this, FreeCat::ValueByWrapper);
        }

        void delete_self() noexcept
        {
            this->deleter(this, FreeCat::Self);
        }

        bool has_value() const noexcept
        {
            return bool(this->shared_ptr);
        }

    protected:
        void release_shared_ptr() noexcept;

        // TODO ~SharedDataBase() = default;
    };

    struct [[nodiscard]] SharedPtr
    {
        SharedPtr(SharedDataBase* p = nullptr) noexcept
          : p(p)
        {
            assert(!p || !p->shared_ptr);

            if (this->p) {
                this->p->shared_ptr = this;
            }
        }

        SharedPtr(SharedPtr const&) = delete;
        SharedPtr& operator=(SharedPtr const&) = delete;

        SharedPtr(SharedPtr&& other) noexcept
        : p(std::exchange(other.p, nullptr))
        {
            this->p->shared_ptr = this;
        }

        SharedPtr& operator=(SharedPtr&& other) noexcept
        {
            assert(other.p != this->p && "unimplemented");
            this->reset();
            this->p = std::exchange(other.p, nullptr);
            this->p->shared_ptr = this;
            return *this;
        }

        ~SharedPtr()
        {
            if (this->p) {
                std::exchange(this->p, nullptr)->free_by_wrapper();
            }
        }

        explicit operator bool() const noexcept
        {
            return bool(this->p);
        }

        void reset() noexcept
        {
            if (this->p) {
                std::exchange(this->p, nullptr)->free_value();
            }
        }

        void detach() noexcept
        {
            if (this->p) {
                this->p->shared_ptr = nullptr;
                this->p = nullptr;
            }
        }

    private:
        friend class SharedDataBase;

        SharedDataBase* release() noexcept
        {
            return std::exchange(this->p, nullptr);
        }

    protected:
        SharedDataBase* p;
    };

    inline void SharedDataBase::release_shared_ptr() noexcept
    {
        if (this->shared_ptr) {
            this->shared_ptr->release();
            this->shared_ptr = nullptr;
        }
    }

    namespace detail
    {
        inline auto default_notify_delete() noexcept
        {
            return [](NotifyDeleteType, auto&)noexcept{};
        }
    }

    template<class T>
    struct SharedData : SharedDataBase
    {
        using value_type = T;

        template<class... Ts>
        SharedData(Ts&&... xs)
        : u(static_cast<Ts&&>(xs)...)
        {
            this->set_notify_delete(detail::default_notify_delete());
        }

        template<class F>
        void set_notify_delete(F /*unused*/)
        {
            this->deleter = [](SharedDataBase* p, FreeCat cat) noexcept {
                auto* self = static_cast<SharedData*>(p);
                switch (cat) {
                    case FreeCat::ValueByWrapper:
                    case FreeCat::Value:
                        assert(!self->is_deleted);
                        REDEMPTION_DEBUG_ONLY(self->is_deleted = true;)
                        self->release_shared_ptr();
                        make_lambda<F>()(static_cast<NotifyDeleteType>(cat), self->u.value);
                        self->u.value.~T();
                        break;
                    case FreeCat::Self:
                        assert(self->is_deleted);
                        delete self; /*NOLINT*/
                        break;
                }
            };
        }

        T* operator->() { return &this->u.value; }
        T& operator*() { return this->u.value; }

        T& value() { return this->u.value; }

    private:
        // TODO ~SharedData() = default;

        union U{
            T value;

            template<class... Ts>
            U(Ts&&... xs) : value(static_cast<Ts&&>(xs)...){}
            ~U() { /* removed by this->deleter */} /*NOLINT*/
        } u;

        REDEMPTION_DEBUG_ONLY(bool is_deleted = false;)
    };

    struct SharedDataDeleter
    {
        void operator()(SharedDataBase* p) const noexcept
        {
            p->free_value();
            p->delete_self();
        }
    };

    template<class... Ts>
    class TopSharedPtr : public SharedPtr
    {
        using Top = TopExecutor<Ts...>;
        using Data = SharedData<Top>;

        struct PtrInterface : protected SharedPtr
        {
            friend class TopSharedPtr;

            void set_timeout(std::chrono::milliseconds ms) noexcept
            {
                this->top().set_timeout(ms);
            }

            void set_time(timeval const& tv) noexcept
            {
                this->top().set_time(tv);
            }

            void set_delay(std::chrono::milliseconds ms) noexcept
            {
                this->top().set_delay(ms);
            }

            void disable_timeout() noexcept
            {
                this->top().disable_timeout();
            }

            void enable_timeout(bool enable = true) noexcept
            {
                this->top().enable_timeout(enable);
            }

        private:
            Top& top() noexcept
            {
                return static_cast<Data*>(this->p)->value();
            }
        };

    public:
        TopSharedPtr(Data* p = nullptr) noexcept
        : SharedPtr(p)
        {}

        PtrInterface* operator->() noexcept
        {
            return static_cast<PtrInterface*>(static_cast<SharedPtr*>(this));
        }
    };

    template<class... Ts>
    class TimerSharedPtr : public SharedPtr
    {
        using Timer = TimerExecutor<Ts...>;
        using Data = SharedData<Timer>;

        struct PtrInterface : protected SharedPtr
        {
            friend class TimerSharedPtr;

            void set_time(timeval const& tv) noexcept
            {
                this->timer().set_time(tv);
            }

            void set_delay(std::chrono::milliseconds ms) noexcept
            {
                this->timer().set_delay(ms);
            }

        private:
            Timer& timer() noexcept
            {
                return static_cast<Data*>(this->p)->value();
            }
        };

    public:
        TimerSharedPtr(SharedDataBase* p = nullptr) noexcept
        : SharedPtr(p)
        {}

        PtrInterface* operator->() noexcept
        {
            return static_cast<PtrInterface*>(static_cast<SharedPtr*>(this));
        }
    };

    template<class... Ts>
    class ActionSharedPtr : public SharedPtr
    {
        using SharedPtr::SharedPtr;
    };

    namespace detail
    {
        template<class Tuple, class... Ts, class F>
        auto create_on_timeout(GroupExecutorWithValues<Tuple, Ts...>& g, F&& f)
        {
            if constexpr (std::is_same<Tuple, detail::tuple<>>::value) { /*NOLINT*/
                (void)g;
                return [f](GroupTimerContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                    return f(TopTimerContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
                };
            }
            else { /*NOLINT*/
                return [f, &g](
                    GroupTimerContext<Ts...> ctx, Ts... xs
                ) mutable /*-> R*/ {
                    return g.t.invoke(
                        f, TopTimerContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
                };
            }
        }

        template<class InheritSharedPtr, class SharedData>
        InheritSharedPtr add_shared_ptr_from_data(
            SharedDataBase& node_executors,
            std::unique_ptr<SharedData, SharedDataDeleter>&& data_uptr) /*noexcept*/
        {
            SharedData* data_ptr = data_uptr.release();
            data_ptr->next = std::exchange(node_executors.next, data_ptr);
            data_ptr->shared_ptr = nullptr;
            return InheritSharedPtr(data_ptr);
        }
    }  // namespace detail


    template<class... Ts>
    class TopContainer
    {
        using Top = TopExecutor<Ts...>;
        using Group = GroupExecutor<Ts...>;
        using TopData = SharedData<Top>;

        using GroupDeleter = ::jln::GroupDeleter<Ts...>;

    public:
        using Ptr = TopSharedPtr<Ts...>;

    private:
        template<class Tuple>
        struct InitContext
        {
            std::unique_ptr<GroupExecutorWithValues<Tuple, Ts...>, GroupDeleter> g;
            std::unique_ptr<TopData, SharedDataDeleter> data_ptr;
            TopContainer& cont;

            GroupExecutorWithValues<Tuple, Ts...>& group() noexcept
            {
                return *this->g;
            }

            Top& top() noexcept
            {
                return this->data_ptr->value();
            }

            template<class F>
            void set_notify_delete(F f) noexcept
            {
                this->data_ptr->set_notify_delete(f);
            }

            template<class F>
            void on_timeout(F&& f)
            {
                this->top().timer_data.on_timeout = detail::create_on_timeout(
                    *this->g, static_cast<F&&>(f));
            }

            TopSharedPtr<Ts...> terminate_init()
            {
                assert(this->data_ptr);
                this->top().add_group(std::move(this->g));
                return detail::add_shared_ptr_from_data<TopSharedPtr<Ts...>>(
                    this->cont.node_executors, std::move(this->data_ptr));
            }
        };

    public:
        template<class... Us>
        REDEMPTION_JLN_CONCEPT(detail::TopExecutorBuilder_Concept)
        create_top_executor(Reactor& reactor, int fd, Us&&... xs)
        {
            using Tuple = detail::tuple<decay_and_strip_t<Us>...>;
            using Group = GroupExecutorWithValues<Tuple, Ts...>;
            using InitCtx = InitContext<Tuple>;
            return detail::TopExecutorBuilder<InitCtx>{
                InitCtx{
                    std::unique_ptr<Group, GroupDeleter>(new Group{static_cast<Us&&>(xs)...}),
                    std::unique_ptr<TopData, SharedDataDeleter>(new TopData{reactor, fd}),
                    *this
                },
            };
        }

        TopContainer() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~TopContainer()
        {
            this->clear();
        }

        void clear() noexcept
        {
            while (this->node_executors.next) {
                SharedDataBase* p = &this->node_executors;
                while (p->next) {
                    SharedDataBase*const node = p->next;
                    SharedDataBase*const next = p->next->next;
                    if (node->has_value()) {
                        // TODO static_cast<Data&>(*p)->external_exit();
                        node->free_value();
                        p = node;
                    }
                    else {
                        p->next->delete_self();
                        p->next = next;
                    }
                }
            }
        }

    private:
        template<class F>
        static auto apply_f(F& f, SharedDataBase* node)
        {
            auto* data = static_cast<TopData*>(node);
            return f(data->value().get_fd(), data->value());
        }

    public:
        template<class F>
        void for_each(F&& f)
        {
            SharedDataBase* node = this->node_executors.next;
            for (; node; node = node->next) {
                if (node->shared_ptr) {
                    apply_f(f, node);
                }
            }
        }

        bool is_empty() const noexcept
        {
            return !bool(this->node_executors.next);
        }

        template<class Predicate>
        bool exec_action(Predicate&& predicate, Ts... xs)
        {
            return this->_exec(predicate, [&](Top& top) {
                return top.exec_action(xs...);
            });
        }

        bool exec_timeout(timeval const end_tv, Ts... xs)
        {
            auto predicate = [&](int /*fd*/, Top& top){
                return top.timer_data.is_enabled && top.timer_data.tv <= end_tv;
            };
            return this->_exec(predicate, [&](Top& top) {
                return top.exec_timeout(xs...);
            });
        }

        SharedDataBase node_executors;

    private:
        template<class Pred, class F>
        bool _exec(Pred& predicate, F f)
        {
            SharedDataBase* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->shared_ptr) {
                    Top& top = static_cast<TopData&>(*cur).value();
                    if (!apply_f(predicate, cur)) {
                        node = node->next;
                    }
                    else if (f(top)) {
                        top.update_next_time();
                        node = node->next;
                    }
                    else {
                        while ((node->next != cur) && node->next) {
                            node = node->next;
                        }
                        node->next = cur->next;
                        cur->free_value();
                        cur->delete_self();
                    }
                }
                else {
                    node->next = cur->next;
                    cur->delete_self();
                }
            }

            return bool(this->node_executors.next);
        }
    };

    template<class... Ts>
    class TimerContainer
    {
        using Timer = TimerExecutor<Ts...>;

        using TimerDeleter = ::jln::TimerDeleter<Ts...>;

    public:
        using Ptr = TimerSharedPtr<Ts...>;

    private:
        template<class TimerData, class Tuple>
        struct InitContext
        {
            std::unique_ptr<TimerData, SharedDataDeleter> data_ptr;
            TimerContainer& cont;

            TimerExecutorWithValues<Tuple, Ts...>& timer() noexcept
            {
                return this->data_ptr->value();
            }

            template<class F>
            void set_notify_delete(F /*unused*/) noexcept
            {
                this->data_ptr->set_notify_delete([](NotifyDeleteType t, auto& act) noexcept{
                    act.invoke(make_lambda<F>(), t);
                });
            }

            TimerSharedPtr<Ts...> terminate_init()
            {
                assert(this->data_ptr);
                SharedDataBase* data_ptr = this->data_ptr.release();
                data_ptr->next = std::exchange(this->cont.node_executors.next, data_ptr);
                data_ptr->shared_ptr = nullptr;
                return TimerSharedPtr<Ts...>(static_cast<TimerData*>(data_ptr));
            }
        };

    public:
        template<class... Us>
        REDEMPTION_JLN_CONCEPT(detail::TimerExecutorBuilder_Concept)
        create_timer_executor(Reactor& reactor, Us&&... xs)
        {
            using Tuple = detail::tuple<decay_and_strip_t<Us>...>;
            using Timer = TimerExecutorWithValues<Tuple, Ts...>;
            using TimerData = SharedData<Timer>;
            using InitCtx = InitContext<TimerData, Tuple>;
            return detail::TimerExecutorBuilder<InitCtx>{
                InitCtx{
                    std::unique_ptr<TimerData, SharedDataDeleter>(
                        new TimerData{reactor, static_cast<Us&&>(xs)...}),
                    *this}
            };
        }

        TimerContainer() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~TimerContainer()
        {
            this->clear();
        }

    private:
        static Timer& to_timer(SharedDataBase* d) noexcept
        {
            return static_cast<SharedData<Timer>*>(d)->value();
        }

    public:
        template<class F>
        void for_each(F&& f)
        {
            SharedDataBase* node = this->node_executors.next;
            for (; node; node = node->next) {
                if (node->shared_ptr) {
                    f(to_timer(node));
                }
            }
        }

        void clear() noexcept
        {
            while (this->node_executors.next) {
                SharedDataBase* p = &this->node_executors;
                while (p->next) {
                    SharedDataBase*const node = p->next;
                    SharedDataBase*const next = p->next->next;
                    if (node->has_value()) {
                        node->free_value();
                        p = node;
                    }
                    else {
                        p->next->delete_self();
                        p->next = next;
                    }
                }
            }
        }

        bool is_empty() const noexcept
        {
            return !bool(this->node_executors.next);
        }

        bool exec_timer(timeval tv, Ts... xs)
        {
            SharedDataBase* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->shared_ptr) {
                    auto& timer = to_timer(cur);
                    if (timer.tv > tv || timer.exec_action(xs...)) {
                        node = node->next;
                    }
                    else {
                        while ((node->next != cur) && node->next) {
                            node = node->next;
                        }
                        node->next = cur->next;
                        cur->free_value();
                        cur->delete_self();
                    }
                }
                else {
                    node->next = cur->next;
                    cur->delete_self();
                }
            }

            return bool(this->node_executors.next);
        }

        SharedDataBase node_executors;
    };

    template<class... Ts>
    class ActionContainer
    {
        using Action = ActionExecutor<Ts...>;

        using ActionDeleter = ::jln::ActionDeleter<Ts...>;

    public:
        using Ptr = ActionSharedPtr<Ts...>;

    private:
        template<class ActionData, class Tuple>
        struct InitContext
        {
            std::unique_ptr<ActionData, SharedDataDeleter> data_ptr;
            ActionContainer& cont;

            ActionExecutorWithValues<Tuple, Ts...>& action() noexcept
            {
                return this->data_ptr->value();
            }

            template<class F>
            void set_notify_delete(F /*unused*/) noexcept
            {
                this->data_ptr->set_notify_delete([](NotifyDeleteType t, auto& act) noexcept{
                    act.invoke(make_lambda<F>(), t);
                });
            }

            ActionSharedPtr<Ts...> terminate_init()
            {
                assert(this->data_ptr);
                return detail::add_shared_ptr_from_data<ActionSharedPtr<Ts...>>(
                    this->cont.node_executors, std::move(this->data_ptr));
            }
        };

    public:
        template<class... Us>
        REDEMPTION_JLN_CONCEPT(detail::ActionExecutorBuilder_Concept)
        create_action_executor(Reactor& /*reactor*/, Us&&... xs)
        {
            using Tuple = detail::tuple<decay_and_strip_t<Us>...>;
            using Action = ActionExecutorWithValues<Tuple, Ts...>;
            using ActionData = SharedData<Action>;
            using InitCtx = InitContext<ActionData, Tuple>;
            return detail::ActionExecutorBuilder<InitCtx>{
                InitCtx{
                    std::unique_ptr<ActionData, SharedDataDeleter>(
                        new ActionData{/*reactor, */static_cast<Us&&>(xs)...}),
                    *this}
            };
        }

        ActionContainer() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~ActionContainer()
        {
            this->clear();
        }

        void clear() noexcept
        {
            while (this->node_executors.next) {
                SharedDataBase* p = &this->node_executors;
                while (p->next) {
                    SharedDataBase*const node = p->next;
                    SharedDataBase*const next = p->next->next;
                    if (node->has_value()) {
                        node->free_value();
                        p = node;
                    }
                    else {
                        p->next->delete_self();
                        p->next = next;
                    }
                }
            }
        }

        bool is_empty() const noexcept
        {
            return !bool(this->node_executors.next);
        }

        bool exec_action(Ts... xs)
        {
            SharedDataBase* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->shared_ptr) {
                    Action& action = static_cast<SharedData<Action>&>(*cur).value();
                    if (action.exec_action(xs...)) {
                        node = node->next;
                    }
                    else {
                        while ((node->next != cur) && node->next) {
                            node = node->next;
                        }
                        node->next = cur->next;
                        cur->free_value();
                        cur->delete_self();
                    }
                }
                else {
                    node->next = cur->next;
                    cur->delete_self();
                }
            }

            return bool(this->node_executors.next);
        }

        SharedDataBase node_executors;
    };



    template<class... Ts>
    GroupTimerContext<Ts...>& GroupTimerContext<Ts...>::disable_timeout() noexcept
    {
        this->top.disable_timeout();
        return *this;
    }

    template<class... Ts>
    int GroupTimerContext<Ts...>::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    template<class... Ts>
    void GroupTimerContext<Ts...>::set_fd(int fd) noexcept
    {
        this->top.set_fd(fd);
    }

    template<class Tuple, class... Ts>
    template<class F>
    TopTimerContext<Tuple, Ts...>& TopTimerContext<Tuple, Ts...>::replace_action(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        group.on_action(static_cast<F&&>(f));
        return *this;
    }

    template<class Tuple, class... Ts>
    template<class F>
    TopTimerContext<Tuple, Ts...>& TopTimerContext<Tuple, Ts...>::replace_exit(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        group.on_exit(static_cast<F&&>(f));
        return *this;
    }

    template<class Tuple, class... Ts>
    template<class F>
    R TopTimerContext<Tuple, Ts...>::replace_timeout(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        this->top.on_timeout_switch = detail::create_on_timeout(group, static_cast<F&&>(f));
        return R::SubstituteTimeout;
    }

    template<class Tuple, class... Ts>
    template<class F>
    R TopTimerContext<Tuple, Ts...>::set_or_disable_timeout(std::chrono::milliseconds ms, F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        this->top.set_timeout(ms);
        if (ms.count()) {
            this->top.on_timeout_switch = detail::create_on_timeout(group, static_cast<F&&>(f));
            return R::SubstituteTimeout;
        }
        this->top.disable_timeout();
        return R::Ready;
    }


    template<class... Ts>
    template<class... Us>
    REDEMPTION_JLN_CONCEPT(detail::GroupExecutorBuilder_Concept)
    GroupContext<Ts...>::create_sub_executor(Us&&... xs)
    {
        using Top = TopExecutor<Ts...>;
        using Group = GroupExecutorWithValues<
            detail::tuple<decay_and_strip_t<Us>...>, Ts...>;
        return detail::GroupExecutorBuilder<Top, Group>{
            this->top, std::make_unique<Group>(static_cast<Us&&>(xs)...)};
    }

    template<class... Ts>
    R GroupContext<Ts...>::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
    }

    template<class... Ts>
    int GroupContext<Ts...>::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    template<class... Ts>
    void GroupContext<Ts...>::set_fd(int fd) noexcept
    {
        return this->top.set_fd(fd);
    }

    template<class... Ts>
    GroupContext<Ts...>& GroupContext<Ts...>::enable_timeout(bool enable) noexcept
    {
        this->top.enable_timeout(enable);
        return *this;
    }

    template<class... Ts>
    Reactor& GroupContext<Ts...>::get_reactor() const noexcept
    {
        return this->top.get_reactor();
    }

    template<class... Ts>
    timeval GroupContext<Ts...>::get_current_time() const noexcept
    {
        return this->get_reactor().get_current_time();
    }


    template<class Tuple, class... Ts>
    template<class F>
    R TopContext<Tuple, Ts...>::replace_action(F&& f)
    {
        // TODO inefficient
        auto g = std::unique_ptr<GroupExecutorDefault<Ts...>, GroupDeleter<Ts...>>(
            new GroupExecutorDefault<Ts...>);
        // TODO same in GroupExecutorWithValues
        if constexpr (std::is_same<Tuple, detail::tuple<>>::value) { /*NOLINT*/
            g->on_action(static_cast<F&&>(f));
        }
        else { /*NOLINT*/
            auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(
                this->current_group);
            g->GroupExecutor<Ts...>::on_action = [f, &group](GroupContext<Ts...> ctx, Ts... xs) mutable /*-> R*/ {
                return group.t.invoke(f, TopContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
            };
        }
        this->top.sub_group(std::move(g));
        return R::SubstituteAction;
    }

    template<class Tuple, class... Ts>
    template<class F>
    TopContext<Tuple, Ts...>& TopContext<Tuple, Ts...>::replace_exit(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        group.on_exit(static_cast<F&&>(f));
        return *this;
    }


    template<class... Ts>
    R GroupTimerContext<Ts...>::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
    }


    template<class... Ts>
    Reactor& TimerContext<Ts...>::get_reactor() const noexcept
    {
        return this->timer.get_reactor();
    }

    template<class... Ts>
    timeval TimerContext<Ts...>::get_current_time() const noexcept
    {
        return this->get_reactor().get_current_time();
    }


    template<bool HasAct, bool HasExit, class Top, class Group>
    detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::GroupExecutorBuilderImpl(
        Top& top, std::unique_ptr<Group>&& g) noexcept
    : top(top)
    , g(std::move(g))
    {}

    template<bool HasAct, bool HasExit, class Top, class Group>
    auto select_group_result(Top& top, std::unique_ptr<Group>&& g)
    {
        if constexpr (HasExit && HasAct) {
            top.sub_group(std::move(g));
            return R::CreateGroup;
        }
        else { /*NOLINT*/
            return detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>{top, std::move(g)};
        }
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_action(F&& f) &&
    {
        static_assert(!HasAct, "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_group_result<1, HasExit, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_exit(F&& f) &&
    {
        static_assert(!HasExit, "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_group_result<HasAct, 1, Top, Group>(this->top, std::move(this->g));
    }


    template<detail::BuilderInit::E Has, class InitCtx>
    detail::TopExecutorBuilderImpl<Has, InitCtx>::TopExecutorBuilderImpl(
        InitCtx&& init_ctx) noexcept
    : init_ctx(std::move(init_ctx))
    {}

    template<int Has, class InitCtx>
    auto select_top_result(InitCtx& init_ctx)
    {
        using E = detail::BuilderInit::E;
        if constexpr (detail::BuilderInit::has(Has, E::Action | E::Exit | E::Timer | E::Timeout)) { /*NOLINT*/
            return init_ctx.terminate_init();
        }
        else { /*NOLINT*/
            return detail::TopExecutorBuilderImpl<E(Has), InitCtx>{std::move(init_ctx)};
        }
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::on_action(F&& f) &&
    {
        static_assert(!(Has & BuilderInit::Action), "on_action is already used");
        this->init_ctx.group().on_action(static_cast<F&&>(f));
        return select_top_result<Has | BuilderInit::Action>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::on_exit(F&& f) &&
    {
        static_assert(!(Has & BuilderInit::Exit), "on_exit or propagate_exit is already used");
        this->init_ctx.group().on_exit(static_cast<F&&>(f));
        return select_top_result<Has | BuilderInit::Exit>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::set_timeout(std::chrono::milliseconds ms) &&
    {
        static_assert(!(Has & BuilderInit::Timeout), "set_timeout is already used");
        this->init_ctx.top().set_timeout(ms);
        return select_top_result<Has | BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::disable_timeout() &&
    {
        static_assert(!(Has & BuilderInit::Timer), "disable_timeout or on_timeout are already used");
        this->init_ctx.top().disable_timeout();
        return select_top_result<Has | BuilderInit::Timer | BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::on_timeout(F&& f) &&
    {
        static_assert(!(Has & BuilderInit::Timer), "disable_timeout or on_timeout are already used");
        this->init_ctx.on_timeout(static_cast<F&&>(f));
        return select_top_result<Has | BuilderInit::Timer>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::propagate_exit() &&
    {
        return this->on_exit([](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs){
            return er.to_result();
        });
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx>::set_notify_delete(F&& f) && noexcept
    {
        static_assert(!(Has & BuilderInit::NotifyDelete), "set_notify_delete is already used");
        this->init_ctx.set_notify_delete(static_cast<F&&>(f));
        return select_top_result<Has | BuilderInit::NotifyDelete>(this->init_ctx);
    }


    template<detail::BuilderInit::E Has, class InitCtx>
    detail::TimerExecutorBuilderImpl<Has, InitCtx>::TimerExecutorBuilderImpl(
        InitCtx&& init_ctx) noexcept
    : init_ctx(std::move(init_ctx))
    {}

    template<int Has, class InitCtx>
    auto select_timer_result(InitCtx& init_ctx)
    {
        using E = detail::BuilderInit::E;
        if constexpr (detail::BuilderInit::has(Has, E::Timer | E::Timeout)) { /*NOLINT*/
            return init_ctx.terminate_init();
        }
        else { /*NOLINT*/
            return detail::TimerExecutorBuilderImpl<E(Has), InitCtx>{std::move(init_ctx)};
        }
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::set_delay(std::chrono::milliseconds ms) &&
    {
        static_assert(!(Has & BuilderInit::Timeout), "set_delay or set_time are already used");
        this->init_ctx.timer().set_delay(ms);
        return select_timer_result<Has | BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::set_time(timeval tv) &&
    {
        static_assert(!(Has & BuilderInit::Timeout), "set_delay or set_time are already used");
        this->init_ctx.timer().set_time(tv);
        return select_timer_result<Has | BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::on_action(F&& f) &&
    {
        static_assert(!(Has & BuilderInit::Timer), "on_action is already used");
        this->init_ctx.timer().on_action(static_cast<F&&>(f));
        return select_timer_result<Has | BuilderInit::Timer>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::set_notify_delete(F&& f) && noexcept
    {
        static_assert(!(Has & BuilderInit::NotifyDelete), "set_notify_delete is already used");
        this->init_ctx.set_notify_delete(static_cast<F&&>(f));
        return select_timer_result<Has | BuilderInit::NotifyDelete>(this->init_ctx);
    }


    template<detail::BuilderInit::E Has, class InitCtx>
    detail::ActionExecutorBuilderImpl<Has, InitCtx>::ActionExecutorBuilderImpl(
        InitCtx&& init_ctx) noexcept
    : init_ctx(std::move(init_ctx))
    {}

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::ActionExecutorBuilderImpl<Has, InitCtx>::on_action(F&& f) &&
    {
        this->init_ctx.action().on_action(static_cast<F&&>(f));
        return this->init_ctx.terminate_init();
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::ActionExecutorBuilderImpl<Has, InitCtx>::set_notify_delete(F&& f) && noexcept
    {
        static_assert(!(Has & BuilderInit::NotifyDelete), "set_notify_delete is already used");
        this->init_ctx.set_notify_delete(static_cast<F&&>(f));
        return ActionExecutorBuilderImpl<
            BuilderInit::E(Has | BuilderInit::NotifyDelete), InitCtx>(std::move(this->init_ctx));
    }
}  // namespace jln


class mod_api;
class Callback;
class Inifile;
namespace gdi
{
    class GraphicApi;
}

struct SessionReactor
{
    using TimerContainer = jln::TimerContainer<>;
    using TimerPtr = TimerContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::TimerExecutorBuilder_Concept)
    create_timer(Args&&... args)
    {
        return this->timer_events_.create_timer_executor(*this, static_cast<Args&&>(args)...);
    }


    using GraphicTimerContainer = jln::TimerContainer<gdi::GraphicApi&>;
    using GraphicTimerPtr = GraphicTimerContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::TimerExecutorBuilder_Concept)
    create_graphic_timer(Args&&... args)
    {
        return this->graphic_timer_events_.create_timer_executor(*this, static_cast<Args&&>(args)...);
    }


    using CallbackEventContainer = jln::ActionContainer<Callback&>;
    using CallbackEventPtr = CallbackEventContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::ActionExecutorBuilder_Concept)
    create_callback_event(Args&&... args)
    {
        return this->front_events_.create_action_executor(*this, static_cast<Args&&>(args)...);
    }


    using GraphicEventContainer = jln::ActionContainer<gdi::GraphicApi&>;
    using GraphicEventPtr = GraphicEventContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::ActionExecutorBuilder_Concept)
    create_graphic_event(Args&&... args)
    {
        return this->graphic_events_.create_action_executor(*this, static_cast<Args&&>(args)...);
    }

    using SesmanEventContainer = jln::ActionContainer<Inifile&>;
    using SesmanEventPtr = SesmanEventContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::ActionExecutorBuilder_Concept)
    create_sesman_event(Args&&... args)
    {
        return this->sesman_events_.create_action_executor(*this, static_cast<Args&&>(args)...);
    }


    using TopFdContainer = jln::TopContainer<>;
    using TopFdPtr = TopFdContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::TopExecutorBuilder_Concept)
    create_fd_event(int fd, Args&&... args)
    {
        return this->fd_events_.create_top_executor(*this, fd, static_cast<Args&&>(args)...);
    }


    using GraphicFdContainer = jln::TopContainer<gdi::GraphicApi&>;
    using GraphicFdPtr = GraphicFdContainer::Ptr;

    template<class... Args>
    REDEMPTION_JLN_CONCEPT(jln::detail::TopExecutorBuilder_Concept)
    create_graphic_fd_event(int fd, Args&&... args)
    {
        return this->graphic_fd_events_.create_top_executor(*this, fd, static_cast<Args&&>(args)...);
    }


    CallbackEventContainer front_events_;
    GraphicEventContainer graphic_events_;
    SesmanEventContainer sesman_events_;
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


    // return a valid timeout, current_time + maxdelay if must wait more than maxdelay
    timeval get_next_timeout(EnableGraphics enable_gd, std::chrono::milliseconds maxdelay) /* const : can't because of _for_each */
    {
        timeval tv = this->get_current_time() + maxdelay;
        if ((enable_gd && !this->graphic_events_.is_empty())
         || !this->front_events_.is_empty()) {
            return tv;
        }

        auto update_tv = [&](timeval const& tv2){
            if (tv2.tv_sec >= 0) {
                tv = std::min(tv, tv2);
            }
        };
        auto top_update_tv = [&](int /*fd*/, auto& top){
            if (top.timer_data.is_enabled) {
                update_tv(top.timer_data.tv);
            }
        };
        auto timer_update_tv = [&](auto& timer){
            update_tv(timer.tv);
        };

        this->timer_events_.for_each(timer_update_tv);
        this->fd_events_.for_each(top_update_tv);
        if (enable_gd) {
            this->graphic_timer_events_.for_each(timer_update_tv);
            this->graphic_fd_events_.for_each(top_update_tv);
        }

        return tv;
    }

    template<class F>
    void for_each_fd(EnableGraphics enable_gd, F f)
    {
        auto g = [&f](int fd, auto& /*top*/){
            assert(fd != -1);
            f(fd);
        };
        this->fd_events_.for_each(g);
        if (enable_gd) {
            this->graphic_fd_events_.for_each(g);
        }
    }

    template<class GetGd>
    void execute_timers(EnableGraphics enable_gd, GetGd get_gd)
    {
        auto const end_tv = this->get_current_time();
        this->timer_events_.exec_timer(end_tv);
        this->fd_events_.exec_timeout(end_tv);
        if (enable_gd) {
            this->graphic_timer_events_.exec_timer(end_tv, get_gd());
            this->graphic_fd_events_.exec_timeout(end_tv, get_gd());
        }
    }

    template<class GetGd>
    void execute_timers_at(EnableGraphics enable_gd, timeval const& end_tv, GetGd get_gd)
    {
        this->set_current_time(end_tv);
        this->execute_timers(enable_gd, get_gd);
    }

    template<class IsSetElem>
    void execute_graphics(IsSetElem is_set, gdi::GraphicApi& gd)
    {
        this->graphic_events_.exec_action(gd);
        this->graphic_fd_events_.exec_action(is_set, gd);
    }

    template<class IsSetElem>
    void execute_events(IsSetElem is_set)
    {
        this->fd_events_.exec_action(is_set);
    }

    void execute_sesman(Inifile& ini)
    {
        this->sesman_events_.exec_action(ini);
    }

    bool has_front_event() const noexcept
    {
        return !this->front_events_.is_empty();
    }

    void execute_callbacks(Callback& callback)
    {
        this->front_events_.exec_action(callback);
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

    bool has_graphics_event() const noexcept
    {
        return !this->graphic_events_.is_empty() || !this->graphic_fd_events_.is_empty();
    }

    int signal = 0;
    void set_next_event(/*BackEvent_t*/int signal)
    {
        // LOG(LOG_DEBUG, "SessionReactor::set_next_event %d", signal);
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

namespace jln
{
    template<class... Ts>
    void TopExecutor<Ts...>::update_next_time() noexcept
    {
        assert(this->timer_data.delay.count() >= 0);
        this->timer_data.tv = addusectimeval(
            this->timer_data.delay, this->reactor.get_current_time());
    }

    template<class... Ts>
    void TimerExecutor<Ts...>::set_delay(std::chrono::milliseconds ms) noexcept
    {
        assert(ms.count() >= 0);
        this->delay = ms;
        this->tv = addusectimeval(this->delay, this->reactor.get_current_time());
    }
}  // namespace jln
