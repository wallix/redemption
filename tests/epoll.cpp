#include <functional>
#include <memory>
#include <cassert>
#include <exception>
#include "cxx/diagnostic.hpp"
#include "utils/executor.hpp"
#include "core/error.hpp"


enum class [[nodiscard]] R : char
{
    Next,
    Terminate,
    ExitSuccess,
    ExitError,
    Exception,
    CreateGroup,
    NeedMoreData,
    Substitute,
};

struct ExitR {
    enum Status : char {
        Error = char(R::ExitError),
        Success = char(R::ExitSuccess),
        Exception = char(R::Exception),
        Terminate = char(R::Terminate),
    };

    Status status;
    ::Error& error;
};

template<class... Ts> struct TopExecutor;
template<class... Ts> struct GroupExecutor;
template<class... Ts> struct NodeExecutor;

enum class ExitStatus : bool {
    Error,
    Success,
};

namespace detail
{
    template<bool setError, int stateInit, class Group, class... Ts>
    struct [[nodiscard]] GroupExecutorBuilderImpl
    {
        explicit GroupExecutorBuilderImpl(TopExecutor<Ts...>& top, std::unique_ptr<Group>&& g) noexcept;

        template<class F>
        auto then(F&& f);

        template<class F>
        GroupExecutorBuilderImpl<1, stateInit, Group> on_exit(F&& f);
        GroupExecutorBuilderImpl<1, stateInit, Group> propagate_exit();

        operator R ();

    private:
        TopExecutor<Ts...>& top;
        std::unique_ptr<Group> g;
    };

#ifdef IN_IDE_PARSER
    struct [[nodiscard]] GroupExecutorBuilder_Concept
    {
        template<class... Ts>
        explicit GroupExecutorBuilder_Concept(Ts&&...) noexcept;

        template<class F> GroupExecutorBuilder_Concept then(F) { return *this; }
        template<class F> GroupExecutorBuilder_Concept on_exit(F) { return *this; }
        GroupExecutorBuilder_Concept propagate_exit();

        operator R ();
    };

    template<class...>
    using GroupExecutorBuilder = GroupExecutorBuilder_Concept;
#else
    template<class Group>
    using GroupExecutorBuilder = GroupExecutorBuilderImpl<0, 0, Group>;
#endif
}

template<class... Ts>
struct Context
{
    template<class... Xs>
#ifdef IN_IDE_PARSER
    detail::GroupExecutorBuilder_Concept create_sub_executor(Xs&&...);
#else
    auto create_sub_executor(Xs&&...);
#endif

    R exception(Error const& e) noexcept;
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

    template<class F>
    R replace_action(F&& f);

private:
    friend TopExecutor<Ts...>;

    Context(TopExecutor<Ts...>& top, NodeExecutor<Ts...>& current)
    : top(top)
    , current(current)
    {}

    TopExecutor<Ts...>& top;
    NodeExecutor<Ts...>& current;
};

template<class... Ts>
struct ContextError
{
//     template<class... Xs>
// #ifdef IN_IDE_PARSER
//     detail::GroupExecutorBuilder_Concept create_sub_executor(Xs&&...);
// #else
//     auto create_sub_executor(Xs&&...);
// #endif

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

    // template<class F>
    // R replace_action(F&& f);

private:
    friend TopExecutor<Ts...>;

    ContextError(TopExecutor<Ts...>& top)
    : top(top)
    {}

    TopExecutor<Ts...>& top;
};

template<class... Ts>
inline R propagate_exit(ContextError<Ts...>, ExitR er) noexcept
{
    return static_cast<R>(er.status);
}

template<class... Ts>
struct NodeExecutor
{
    std::function<R(Context<Ts...>, Ts&...)> f;
    std::unique_ptr<NodeExecutor> next;

    template<class F>
    static std::unique_ptr<NodeExecutor> create(F&& f)
    {
        return std::unique_ptr<NodeExecutor>(
            new NodeExecutor{static_cast<F&&>(f), nullptr});
    }
};

template<class... Ts>
struct GroupExecutor
{
    GroupExecutor() = default;
    GroupExecutor(GroupExecutor const&) = delete;
    GroupExecutor& operator=(GroupExecutor const&) = delete;

    virtual ~GroupExecutor() = default;

    void add_group_executor(std::unique_ptr<GroupExecutor>&& g)
    {
        assert(bool(g->next));
        g->group = std::move(this->group);
        this->group = std::move(g);
    }

    template<class F>
    void then(F&& f)
    {
        if (!this->end_next) {
            this->next = NodeExecutor<Ts...>::create(static_cast<F&&>(f));
            this->end_next = this->next.get();
        }
        else {
            this->end_next->next = NodeExecutor<Ts...>::create(static_cast<F&&>(f));
            this->end_next = this->end_next->next.get();
        }
    }

    template<class F>
    void set_on_exit(F&& f)
    {
        this->on_exit = static_cast<F&&>(f);
    }

    void set_propagate_exit() noexcept
    {
        this->on_exit = &::propagate_exit<Ts...>;
    }

protected:
    bool has_group() const noexcept { return bool(this->group); }
    friend TopExecutor<Ts...>;

private:
    std::unique_ptr<NodeExecutor<Ts...>> next;
    NodeExecutor<Ts...>* end_next = nullptr;
    std::unique_ptr<GroupExecutor> group;
    std::function<R(ContextError<Ts...>, ExitR er, Ts&...)> on_exit;
};


template<class Tuple, class... Ts>
struct GroupExecutorWithValues : GroupExecutor<Ts...>, private Tuple
{
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class... Us>
    GroupExecutorWithValues(Us&&... xs)
    noexcept(noexcept(Tuple{static_cast<Us&&>(xs)...}))
      : Tuple{static_cast<Us&&>(xs)...}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    template<class F>
    void then(F&& f)
    {
        this->GroupExecutor<Ts...>::then([f = static_cast<F&&>(f), this](Context<Ts...> ctx) -> R{
            return this->invoke(f, ctx);
        });
    }

    template<class F>
    void set_on_exit(F&& f)
    {
        this->GroupExecutor<Ts...>::set_on_exit([f = static_cast<F&&>(f), this](
            ContextError<Ts...> ctx, ExitR er) -> R
        {
            return this->invoke(f, ctx, er);
        });
    }

private:
    friend Context<Ts...>;
};

template<class... Ts>
struct GroupExecutorWithValues<jln::detail::tuple<>, Ts...> : GroupExecutor<Ts...>
{};


template<class... Ts>
struct TopExecutor : GroupExecutor<Ts...>
{
    TopExecutor() = default;

    bool exec(Ts&... xs)
    {
        try {
            return this->_exec(*this, xs...) == R::NeedMoreData;
        }
        catch (Error const& e) {
            this->error = e;
            R const r = this->has_group()
                ? this->_exec_group_exit(*this, R::Exception, xs...)
                : this->_exec_exit(*this, R::Exception, xs...);
            if (r == R::Exception) {
                throw this->error;
            }
            return (r == R::NeedMoreData);
        }
    }

    Error error = Error(NO_ERROR);

private:
    R _exec_next(GroupExecutor<Ts...>& top, Ts&... xs);
    R _exec(GroupExecutor<Ts...>& top, Ts&... xs);
    R _exec_exit(GroupExecutor<Ts...>& top, R r, Ts&... xs);
    R _exec_group_exit(GroupExecutor<Ts...>& top, R r, Ts&... xs);
};


template<class... Ts>
R TopExecutor<Ts...>::_exec_exit(GroupExecutor<Ts...>& g, R r, Ts&... xs)
{
    switch (R re = g.on_exit(
        ContextError<Ts...>{*this},
        ExitR{static_cast<ExitR::Status>(r), this->error},
        xs...
    )) {
        case R::ExitSuccess:
        case R::Exception:
        case R::Terminate:
        case R::ExitError:
            return re;
        case R::NeedMoreData:
        case R::Next:
            return bool(g.next) ? R::NeedMoreData : r;
        case R::CreateGroup:
        case R::Substitute:
            return R::NeedMoreData;
    }
    REDEMPTION_UNREACHABLE();
}

template<class... Ts>
R TopExecutor<Ts...>::_exec_group_exit(GroupExecutor<Ts...>& g, R r, Ts&... xs)
{
    do {
        R const re = g.group->on_exit(
            ContextError<Ts...>{*this},
            ExitR{static_cast<ExitR::Status>(r), this->error},
            xs...);
        g.group = std::move(g.group->group);
        switch (re) {
            case R::ExitSuccess:
                if (!g.group && g.next) {
                    return R::NeedMoreData;
                }
                r = re;
                break;
            case R::Exception:
            case R::Terminate:
            case R::ExitError:
                r = re;
                break;
            case R::NeedMoreData:
            case R::Next:
                return bool(g.next) ? R::NeedMoreData : r;
            case R::CreateGroup:
            case R::Substitute:
                return R::NeedMoreData;
        }
    } while (g.group);
    return r;
}

template<class... Ts>
R TopExecutor<Ts...>::_exec_next(GroupExecutor<Ts...>& g, Ts&... xs)
{
    R const r = g.next->f(Context<Ts...>{*this, *g.next}, xs...);
    switch (r) {
        case R::Terminate:
        case R::Exception:
        case R::ExitError:
        case R::ExitSuccess:
        case R::NeedMoreData:
            return r;
        case R::Next:
            g.next = std::move(g.next->next);
            return (bool(g.next) ? R::NeedMoreData : R::ExitSuccess);
        case R::CreateGroup:
            g.next = std::move(g.next->next);
            return R::NeedMoreData;
        case R::Substitute:
            return R::NeedMoreData;
    }
    REDEMPTION_UNREACHABLE();
}

template<class... Ts>
R TopExecutor<Ts...>::_exec(GroupExecutor<Ts...>& g, Ts&... xs)
{
    if (g.group) {
        R const r = this->_exec_next(*g.group, xs...);
        switch (r) {
            case R::Exception:
            case R::Terminate:
            case R::ExitError:
            case R::ExitSuccess:
                return this->_exec_group_exit(g, r, xs...);
            case R::NeedMoreData:
                return r;
            case R::Next:
            case R::Substitute:
            case R::CreateGroup:
                assert(false);
        }
        REDEMPTION_UNREACHABLE();
    }
    else {
        assert(g.next);
        R const r = this->_exec_next(g, xs...);
        switch (r) {
            case R::Exception:
            case R::Terminate:
            case R::ExitError:
            case R::ExitSuccess:
                return this->_exec_exit(g, r, xs...);
            case R::NeedMoreData:
            case R::Next:
            case R::Substitute:
            case R::CreateGroup:
                return r;
        }
        REDEMPTION_UNREACHABLE();
    }
}


template<bool setError, int stateInit, class Group, class... Ts>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group, Ts...>::GroupExecutorBuilderImpl(
    TopExecutor<Ts...>& top, std::unique_ptr<Group>&& g) noexcept
: top(top)
, g(std::move(g))
{}

template<bool setError, int stateInit, class Group, class... Ts>
template<class F>
auto detail::GroupExecutorBuilderImpl<setError, stateInit, Group, Ts...>::then(F&& f)
{
    this->g->then(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<
        setError, (stateInit == 1 ? stateInit : stateInit+1), Group, Ts...>{
            this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group, class... Ts>
template<class F>
detail::GroupExecutorBuilderImpl<1, stateInit, Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group, Ts...>::on_exit(F&& f)
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_on_exit(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<1, stateInit, Group, Ts...>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group, class... Ts>
detail::GroupExecutorBuilderImpl<1, stateInit, Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group, Ts...>::propagate_exit()
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_propagate_exit();
    return GroupExecutorBuilderImpl<1, stateInit, Group, Ts...>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group, class... Ts>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group, Ts...>::operator R()
{
    static_assert(stateInit >= 1, "empty group");
    static_assert(setError, "missing builder.on_error(f) or builder.propagate_exit()");
    this->top.add_group_executor(std::move(this->g));
    return R::CreateGroup;
}


template<class... Ts>
#ifdef IN_IDE_PARSER
detail::GroupExecutorBuilder_Concept Context<Ts...>::create_sub_executor(Xs&&...)
#else
template<class... Xs>
auto Context<Ts...>::create_sub_executor(Xs&&... xs)
#endif
{
    using G = GroupExecutorWithValues<
        jln::detail::tuple<jln::detail::decay_and_strip_t<Xs>...>,
        Ts...>;
    return detail::GroupExecutorBuilder<G>{
        this->top, std::make_unique<G>(static_cast<Xs&&>(xs)...)};
}

template<class... Ts>
R Context<Ts...>::exception(Error const& e) noexcept
{
    this->top.error = e;
    return R::Exception;
}

template<class... Ts>
template<class F>
R Context<Ts...>::replace_action(F&& f)
{
    this->current.f = static_cast<F&&>(f);
    return R::Substitute;
}


#include <iostream>

int main()
{
    using Context = ::Context<>;
    TopExecutor<> e;
    e.set_propagate_exit();
    e.then([](Context ctx){std::cout << "1\n"; return ctx.next(); });
    e.then([](Context ctx){
        std::cout << "2\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "2.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "2.2\n"; return ctx.next(); })
        .propagate_exit();

    });
    e.then([](Context ctx){
        std::cout << "3\n";
        return ctx.create_sub_executor(1)
        .then([](Context ctx, int i){
            std::cout << "3.1  " << i << "\n";
            return ctx.create_sub_executor()
            .then([](Context ctx){std::cout << "3.1.1\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "3.1.2\n"; return ctx.next(); })
            .propagate_exit();
        })
        .propagate_exit();
    });
    e.then([&](Context ctx){
        std::cout << "4\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "4.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.2\n"; return ctx.exit_on_success(); })
        .then([](Context ctx){std::cout << "4.3\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.4\n"; return ctx.next(); })
        .propagate_exit();
    });
    e.then([](Context ctx){
        std::cout << "5\n";
        return ctx.replace_action([](Context ctx){
            std::cout << "5bis\n";
            return ctx.next();
        });
    });


    for (int i = 0; ; ++i) {
        if (i >= 22) {
            std::cout << "KO\n";
            break;
        }
        std::cout << ".";
        if (!e.exec()) {
            break;
        }
    }
}
