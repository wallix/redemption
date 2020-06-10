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
#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/difftimeval.hpp"
#include "utils/string_c.hpp"
#include "utils/sugar/scope_exit.hpp"

#include "utils/timebase.hpp"

#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#ifndef NDEBUG
# define REDEMPTION_DEBUG_ONLY(...) __VA_ARGS__
#else
# define REDEMPTION_DEBUG_ONLY(...)
#endif


namespace jln
{
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

    namespace detail
    {
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
    }
}

namespace jln
{
    class TopExecutor;
    class GroupExecutor;
    template<class... Ts> class ActionExecutor;
    template<class T> class SharedData;
    class SharedPtr;
    class TopSharedPtr;
    class ActionSharedPtr;

    struct ExitR
    {
        enum Status : char {
            Error = char(R::ExitError),
            Success = char(R::Next),
            Exception = char(R::Exception),
            Terminate = char(R::Terminate),
        };

        [[nodiscard]] R to_result() const noexcept
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
//        static_assert(
//            std::is_empty<F>::value,
//            "F must be an empty class or a lambda expression convertible to pointer of function");
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


        template<class InitCtx>
        using TopExecutorBuilder = TopExecutorBuilderImpl<BuilderInit::None, InitCtx>;

    }  // namespace detail


    enum class NextMode { ChildToNext, CreateContinuation, };

    struct GroupContext
    {
        GroupContext(TopExecutor& top, GroupExecutor& current_group) noexcept
        : top(top)
        , current_group(current_group)
        {}

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

        [[nodiscard]] int get_fd() const noexcept;
        void set_fd(int fd) noexcept;

        [[nodiscard]] timeval get_current_time() const noexcept;

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
        TopExecutor& top;
        GroupExecutor& current_group;
    };

    template<class Tuple, class... Ts>
    struct TopContext : GroupContext
    {
        template<class F>
        R replace_action(F&& f);

        template<class F>
        TopContext& replace_exit(F&& f);

        TopContext& enable_timeout(bool enable = true) noexcept
        {
            this->GroupContext::enable_timeout(enable);
            return *this;
        }

        TopContext& disable_timeout() noexcept
        {
            this->GroupContext::disable_timeout();
            return *this;
        }
    };

    template<class... Ts>
    struct ActionContext
    {
        R next() noexcept { return R::Next; }
        R ready() noexcept { return R::Ready; }
        R terminate() noexcept { return R::Terminate; }
    };

    struct ExitContext
    {
        TopExecutor& top;
        GroupExecutor& current_group;

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

        [[nodiscard]] int get_fd() const noexcept;
        void set_fd(int fd) noexcept;

        TopExecutor& top;
        GroupExecutor& current_group;
    };

    template<class Tuple, class... Ts>
    struct TopTimerContext : GroupTimerContext
    {
        template<class F>
        TopTimerContext& replace_action(F&& f);

        template<class F>
        TopTimerContext& replace_exit(F&& f);

        template<class F>
        R replace_timeout(F&& f);

        TopTimerContext& disable_timeout() noexcept
        {
            GroupTimerContext::disable_timeout();
            return *this;
        }
    };

    struct GroupExecutor
    {
        std::function<R(GroupContext)> on_action;
        std::function<R(ExitContext, ExitR er)> on_exit;
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
    struct GroupExecutorWithValues final : GroupExecutor
    {
        using Base = GroupExecutor;

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        GroupExecutorWithValues(Us&&... xs)
        : GroupExecutor([](GroupExecutor* p) noexcept{
            delete static_cast<GroupExecutorWithValues*>(p); /*NOLINT*/
        })
        , t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f, this](GroupContext ctx, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(
                    f, TopContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
            };
        }

        template<class F>
        void on_exit(F&& f)
        {
            Base::on_exit = [f, this](ExitContext ctx, ExitR er, Ts... xs) mutable /*-> R*/ {
                return this->t.invoke(f, ctx, er, static_cast<Ts&>(xs)...);
            };
        }

        Tuple t;

    protected:
        ~GroupExecutorWithValues() = default;
    };

    template<template<class...> class Tuple, class... Ts>
    struct GroupExecutorWithValues<Tuple<>, Ts...> final : GroupExecutor
    {
        using Base = GroupExecutor;

        GroupExecutorWithValues() noexcept
        : GroupExecutor([](GroupExecutor* p) noexcept{
            delete static_cast<GroupExecutorWithValues*>(p); /*NOLINT*/
        })
        {}

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f](GroupContext ctx, Ts... xs) mutable /*-> R*/ {
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
    struct ActionExecutor
    {
        ActionExecutor() = default;
        ActionExecutor(const ActionExecutor&) = delete;
        ActionExecutor& operator=(const ActionExecutor&) = delete;

        std::function<R(ActionContext<Ts...>, Ts...)> on_action;

        bool exec_action()
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            switch (this->on_action(ActionContext<Ts...>{})) {
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

    }  // namespace detail

    template<auto x>
    inline constexpr auto value = std::integral_constant<decltype(x), x>{};

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
        void operator()(GroupExecutor* p) noexcept
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

    struct TopExecutor
    {
        using GroupPtr = std::unique_ptr<GroupExecutor, GroupDeleter<>>;

        TopExecutor(TimeBase& timebase, int fd)
        : fd(fd)
        , timebase(timebase)
        {}

        ~TopExecutor()
        {
//            LOG(LOG_INFO, "~TopExecutor()");
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

        [[nodiscard]] int get_fd() const noexcept
        {
            return this->fd;
        }

        void set_timeout(std::chrono::milliseconds ms) noexcept
        {
            this->timer_data.delay = ms;
            this->update_next_time();
        }

        void update_next_time() noexcept
        {
            assert(this->timer_data.delay.count() >= 0);
            this->timer_data.tv = addusectimeval(this->timer_data.delay, this->timebase.get_current_time());
        }

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
//            LOG(LOG_INFO, "add_group() TopExecutor");
            group->next = this->group;
            this->group = group.release();
        }

        void sub_group(GroupPtr&& group)
        {
            this->loaded_group = std::move(group);
        }

        bool exec_timeout()
        {
//            LOG(LOG_INFO, "exec_timeout() TopExecutor");

            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            R r;

            try {
                auto& on_timeout = this->timer_data.on_timeout;
                do {
                    switch ((r = on_timeout(GroupTimerContext{*this, *this->group}))) {
                        case R::Terminate:
                        case R::Exception:
                        case R::ExitError:
                        case R::ExitSuccess:
                        case R::Next:
                            r = this->_exec_exit(r);
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
                r = this->_exec_exit(R::Exception);
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

        bool exec_action()
        {
//            LOG(LOG_INFO, "exec_action() A TopExecutor");
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            R r;

            try {
                do {
                    switch ((r = this->_exec_action())) {
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
                r = this->_exec_exit(R::Exception);
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
        R _exec_action()
        {
//            LOG(LOG_INFO, "exec_action() B TopExecutor");

            R const r = this->group->on_action(GroupContext{*this, *this->group});
            switch (r) {
                case R::Terminate:
                case R::Exception:
                case R::ExitError:
                case R::ExitSuccess:
                case R::Next:
                    return this->_exec_exit(r);
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

        R _exec_exit(R r)
        {
//            LOG(LOG_INFO, "exec_exit() TopExecutor");
            do {
                R const re = this->group->on_exit(
                    ExitContext{*this, *this->group},
                    ExitR{static_cast<ExitR::Status>(r), this->error});
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
        GroupExecutor* group = nullptr;
        GroupPtr loaded_group;
    public: // TODO to private
        TimeBase& timebase;

        REDEMPTION_DEBUG_ONLY(bool exec_is_running = false;)

        struct TimerData
        {
            bool is_enabled = true;
            timeval tv {};
            std::chrono::milliseconds delay = std::chrono::milliseconds(-1);
            std::function<R(GroupTimerContext)> on_timeout;
        } timer_data;

    // private:
        std::function<R(GroupTimerContext)> on_timeout_switch;

    public:
        Error error = Error(NO_ERROR);
    };


    inline GroupTimerContext& GroupTimerContext::disable_timeout() noexcept
    {
        this->top.disable_timeout();
        return *this;
    }

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

        [[nodiscard]] bool has_value() const noexcept
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

    class TopSharedPtr : public SharedPtr
    {
        using Top = TopExecutor;
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

    class ActionSharedPtr : public SharedPtr
    {
        using SharedPtr::SharedPtr;
    };

    namespace detail
    {
        template<class Tuple, class... Ts, class F>
        auto create_on_timeout(GroupExecutorWithValues<Tuple>& g, F&& f)
        {
            if constexpr (std::is_same<Tuple, detail::tuple<>>::value) { /*NOLINT*/
                (void)g;
                return [f](GroupTimerContext ctx) mutable /*-> R*/ {
                    return f(TopTimerContext<Tuple>{ctx});
                };
            }
            else { /*NOLINT*/
                return [f, &g](
                    GroupTimerContext ctx) mutable /*-> R*/ {
                    return g.t.invoke(
                        f, TopTimerContext<Tuple>{ctx});
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


    class TopContainer
    {
        using Top = TopExecutor;
        using Group = GroupExecutor;
        using TopData = SharedData<Top>;

        using GroupDeleter = ::jln::GroupDeleter<>;

    public:
        using Ptr = TopSharedPtr;
        TopContainer(const TopContainer&) = delete;

    private:
        template<class Tuple>
        struct InitContext
        {
            std::unique_ptr<GroupExecutorWithValues<Tuple>, GroupDeleter> g;
            std::unique_ptr<TopData, SharedDataDeleter> data_ptr;
            TopContainer& cont;

            GroupExecutorWithValues<Tuple>& group() noexcept
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

            TopSharedPtr terminate_init()
            {
                assert(this->data_ptr);
                this->top().add_group(std::move(this->g));
                return detail::add_shared_ptr_from_data<TopSharedPtr>(
                    this->cont.node_executors, std::move(this->data_ptr));
            }

        };

    public:
        template<class... Us>
        auto create_top_executor(TimeBase& timebase, int fd, Us&&... xs)
        {
            using Tuple = detail::tuple<decay_and_strip_t<Us>...>;
            using Group = GroupExecutorWithValues<Tuple>;
            using InitCtx = InitContext<Tuple>;
            return detail::TopExecutorBuilder<InitCtx>{
                InitCtx{
                    std::unique_ptr<Group, GroupDeleter>(new Group{static_cast<Us&&>(xs)...}),
                    std::unique_ptr<TopData, SharedDataDeleter>(new TopData{timebase, fd}),
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

        [[nodiscard]] bool is_empty() const noexcept
        {
            return !bool(this->node_executors.next);
        }

        template<class Predicate>
        bool exec_action(Predicate&& predicate)
        {
//            LOG(LOG_INFO, "exec_action() C TopContainer");
            return this->_exec(predicate, [&](Top& top) {
                return top.exec_action();
            });
        }

        bool exec_timeout(timeval const end_tv)
        {
//            LOG(LOG_INFO, "exec_timeout() C TopContainer");

            auto predicate = [&](int /*fd*/, Top& top){
                return top.timer_data.is_enabled && top.timer_data.tv <= end_tv;
            };
            return this->_exec(predicate, [&](Top& top) {
                return top.exec_timeout();
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

    inline int GroupTimerContext::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    inline void GroupTimerContext::set_fd(int fd) noexcept
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

    inline R GroupContext::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
    }

    inline int GroupContext::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    inline void GroupContext::set_fd(int fd) noexcept
    {
        return this->top.set_fd(fd);
    }

    inline GroupContext& GroupContext::enable_timeout(bool enable) noexcept
    {
        this->top.enable_timeout(enable);
        return *this;
    }

    inline timeval GroupContext::get_current_time() const noexcept
    {
        return this->top.timebase.get_current_time();
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
            g->GroupExecutor::on_action = [f, &group](GroupContext ctx, Ts... xs) mutable /*-> R*/ {
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


    inline R GroupTimerContext::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
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



}  // namespace jln

struct TopFdContainer : jln::TopContainer {};
using TopFdPtr = TopFdContainer::Ptr;


namespace jln
{
    class TimerExecutor;

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
            static constexpr I index() noexcept { return I{}; }
            static constexpr S name() noexcept { return S{}; }
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

            static constexpr std::size_t count = sizeof...(NamedIndexed);

            static constexpr char const* const strings[sizeof...(NamedIndexed)]
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
    }

    namespace literals
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
        REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wpedantic")
        template<class C, C... cs>
        detail::named_type<jln::string_c<cs...>> operator ""_f () noexcept
        { return {}; }
        REDEMPTION_DIAGNOSTIC_POP
    }



    namespace detail {
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
    }


    template<class S, class F>
    detail::named_function<S, F> named(S /*unused*/, F /*unused*/)
    { return {}; }

    enum class IndexSequence : int;

    template<class Ctx, class NamedIndexPack>
    struct FuncSequencerCtx
    {
        Ctx& base() noexcept
        {
            return this->ctx;
        }

        [[nodiscard]] R terminate() const noexcept
        {
            return R::Terminate;
        }

        [[nodiscard]] bool is_final_sequence() const noexcept
        {
            return this->i == NamedIndexPack::count - 1u;
        }

        [[nodiscard]] IndexSequence index() const noexcept
        {
            return IndexSequence(this->i);
        }

        [[nodiscard]] char const* sequence_name() const noexcept
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
        Ctx& at(jln::string_c<cs...> /*unused*/) noexcept
        {
            this->i = detail::named_indexed_by_name<jln::string_c<cs...>>(NamedIndexPack{}).index();
            return this->ctx;
        }

        R exec_at(int i) noexcept
        {
            this->i = i;
            return R::ReRun;
        }

        template<char... cs>
        R exec_at(jln::string_c<cs...> /*unused*/) noexcept
        {
            this->i = detail::named_indexed_by_name<jln::string_c<cs...>>(NamedIndexPack{}).index();
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

    namespace detail
    {
        template<detail::BuilderInit::E Has, class InitCtx>
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

        template<class InitCtx>
        using TimerExecutorBuilder = TimerExecutorBuilderImpl<detail::BuilderInit::None, InitCtx>;
    }



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

    struct TimerContext
    {
        R next() noexcept { return R::Next; }
        R ready() noexcept { return R::Ready; }
        R terminate() noexcept { return R::Terminate; }
        R ready_to(std::chrono::milliseconds ms) noexcept;

        TimerContext& set_delay(std::chrono::milliseconds ms) noexcept;
        TimerContext& set_time(timeval tv) noexcept;
        timeval get_current_time() const noexcept;

        TimerContext(TimerExecutor& timer) noexcept
        : timer(timer)
        {}

    private:
        TimerExecutor& timer;
    };


    struct TimerExecutor
    {
        std::function<R(TimerContext)> on_timer;
        timeval tv {};
        std::chrono::milliseconds delay = std::chrono::milliseconds(-1);
        TimeBase& timebase;

        TimerExecutor(TimeBase& timebase) noexcept
        : timebase(timebase)
        {}

        void set_delay(std::chrono::milliseconds ms) noexcept;
        void set_time(timeval tv) noexcept;

        bool exec_action()
        {
            REDEMPTION_DEBUG_ONLY(
                this->exec_is_running = true;
                SCOPE_EXIT(this->exec_is_running = false);
            )

            switch (this->on_timer(TimerContext{*this})) {
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



    template<class Tuple>
    struct TimerExecutorWithValues final : TimerExecutor
    {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        TimerExecutorWithValues(TimeBase& timebase, Us&&... xs)
        : TimerExecutor(timebase)
        , t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            this->on_timer = [f, this](TimerContext ctx) mutable /*-> R*/ {
                return this->t.invoke(f, ctx);
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

    template<template<class...> class Tuple>
    struct TimerExecutorWithValues<Tuple<>> final : TimerExecutor
    {
        using Base = TimerExecutor;
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


    class TimerSharedPtr : public jln::SharedPtr
    {
        using Timer = TimerExecutor;
        using Data = jln::SharedData<Timer>;

        struct PtrInterface : protected jln::SharedPtr
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
        TimerSharedPtr(jln::SharedDataBase* p = nullptr) noexcept
        : jln::SharedPtr(p)
        {}

        PtrInterface* operator->() noexcept
        {
            return static_cast<PtrInterface*>(static_cast<jln::SharedPtr*>(this));
        }
    };

    class TimerContainer
    {
        using Timer = TimerExecutor;

    public:
        using Ptr = TimerSharedPtr;

    private:
        template<class LocalTimerData, class Tuple>
        struct InitContext
        {
            std::unique_ptr<LocalTimerData, jln::SharedDataDeleter> data_ptr;
            TimerContainer& cont;

            TimerExecutorWithValues<Tuple>& timer() noexcept
            {
                return this->data_ptr->value();
            }

            template<class F>
            void set_notify_delete(F /*unused*/) noexcept
            {
                this->data_ptr->set_notify_delete([](jln::NotifyDeleteType t, auto& act) noexcept{
                    act.invoke(jln::make_lambda<F>(), t);
                });
            }

            TimerSharedPtr terminate_init()
            {
                assert(this->data_ptr);
                jln::SharedDataBase* data_ptr = this->data_ptr.release();
                data_ptr->next = std::exchange(this->cont.node_executors.next, data_ptr);
                data_ptr->shared_ptr = nullptr;
                return TimerSharedPtr(static_cast<LocalTimerData*>(data_ptr));
            }
        };

    public:
        template<class... Us>
        auto create_timer_executor(TimeBase& timebase, Us&&... xs)
        {
            using Tuple = jln::detail::tuple<jln::decay_and_strip_t<Us>...>;
            using Timer = TimerExecutorWithValues<Tuple>;
            using LocalTimerData = jln::SharedData<Timer>;
            using InitCtx = InitContext<LocalTimerData, Tuple>;
            return detail::TimerExecutorBuilder<InitCtx>{
                InitCtx{
                    std::unique_ptr<LocalTimerData, jln::SharedDataDeleter>(
                        new LocalTimerData{timebase, static_cast<Us&&>(xs)...}),
                    *this}
            };
        }

        TimerContainer(const TimerContainer&) = delete;

        TimerContainer() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~TimerContainer()
        {
            this->clear();
        }

    private:
        static Timer& to_timer(jln::SharedDataBase* d) noexcept
        {
            return static_cast<jln::SharedData<Timer>*>(d)->value();
        }

    public:
        template<class F>
        void for_each(F&& f)
        {
            jln::SharedDataBase* node = this->node_executors.next;
            for (; node; node = node->next) {
                if (node->shared_ptr) {
                    f(to_timer(node));
                }
            }
        }

        void clear() noexcept
        {
            while (this->node_executors.next) {
                jln::SharedDataBase* p = &this->node_executors;
                while (p->next) {
                    jln::SharedDataBase*const node = p->next;
                    jln::SharedDataBase*const next = p->next->next;
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

        [[nodiscard]] bool is_empty() const noexcept
        {
            return !bool(this->node_executors.next);
        }

        bool exec_timer(timeval tv)
        {
            jln::SharedDataBase* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->shared_ptr) {
                    auto& timer = to_timer(cur);
                    if (timer.tv > tv || timer.exec_action()) {
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

        jln::SharedDataBase node_executors;
    };

    inline timeval TimerContext::get_current_time() const noexcept
    {
        return this->timer.timebase.get_current_time();
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
        static_assert(!(Has & detail::BuilderInit::Timeout), "set_delay or set_time are already used");
        this->init_ctx.timer().set_delay(ms);
        return select_timer_result<Has | detail::BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::set_time(timeval tv) &&
    {
        static_assert(!(Has & detail::BuilderInit::Timeout), "set_delay or set_time are already used");
        this->init_ctx.timer().set_time(tv);
        return select_timer_result<Has | detail::BuilderInit::Timeout>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::on_action(F&& f) &&
    {
        static_assert(!(Has & detail::BuilderInit::Timer), "on_action is already used");
        this->init_ctx.timer().on_action(static_cast<F&&>(f));
        return select_timer_result<Has | detail::BuilderInit::Timer>(this->init_ctx);
    }

    template<detail::BuilderInit::E Has, class InitCtx>
    template<class F>
    auto detail::TimerExecutorBuilderImpl<Has, InitCtx>::set_notify_delete(F&& f) && noexcept
    {
        static_assert(!(Has & detail::BuilderInit::NotifyDelete), "set_notify_delete is already used");
        this->init_ctx.set_notify_delete(static_cast<F&&>(f));
        return select_timer_result<Has | detail::BuilderInit::NotifyDelete>(this->init_ctx);
    }

    inline TimerContext& TimerContext::set_delay(std::chrono::milliseconds ms) noexcept
    {
        this->timer.set_delay(ms);
        return *this;
    }

    inline TimerContext& TimerContext::set_time(timeval tv) noexcept
    {
        this->timer.set_time(tv);
        return *this;
    }

    inline R TimerContext::ready_to(std::chrono::milliseconds ms) noexcept
    {
        this->timer.set_delay(ms);
        return R::Ready;
    }

    inline void TimerExecutor::set_delay(std::chrono::milliseconds ms) noexcept
    {
        assert(ms.count() >= 0);
        this->delay = ms;
        this->tv = addusectimeval(this->delay, this->timebase.get_current_time());
    }

    inline void TimerExecutor::set_time(timeval tv) noexcept
    {
        this->delay = std::chrono::milliseconds(-1);
        this->tv = tv;
    }

}

struct TimerContainer : jln::TimerContainer {};
using TimerPtr = TimerContainer::Ptr;

