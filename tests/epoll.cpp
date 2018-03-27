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

struct TopExecutor;
struct GroupExecutor;
struct NodeExecutor;

enum class ExitStatus : bool {
    Error,
    Success,
};

namespace detail
{
    template<bool setError, int stateInit, class Group>
    struct [[nodiscard]] GroupExecutorBuilderImpl
    {
        explicit GroupExecutorBuilderImpl(TopExecutor& top, std::unique_ptr<Group>&& g) noexcept;

        template<class F>
        auto then(F&& f);

        template<class F>
        GroupExecutorBuilderImpl<1, stateInit, Group> on_exit(F&& f);
        GroupExecutorBuilderImpl<1, stateInit, Group> propagate_exit();

        operator R ();

    private:
        TopExecutor& top;
        std::unique_ptr<Group> g;
    };

#ifdef IN_IDE_PARSER
    struct [[nodiscard]] GroupExecutorBuilder_Concept
    {
        explicit GroupExecutorBuilder_Concept(TopExecutor&, std::unique_ptr<GroupExecutor>&& g) noexcept;

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
    friend GroupExecutor;

    Context(TopExecutor& top, NodeExecutor& current)
    : top(top)
    , current(current)
    {}

    TopExecutor& top;
    NodeExecutor& current;
};

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
    friend GroupExecutor;

    ContextError(TopExecutor& top)
    : top(top)
    {}

    TopExecutor& top;
};

inline R propagate_exit(ContextError, ExitR er) noexcept
{
    return static_cast<R>(er.status);
}

struct NodeExecutor
{
    std::function<R(Context)> f;
    std::unique_ptr<NodeExecutor> next;

    template<class F>
    static std::unique_ptr<NodeExecutor> create(F&& f)
    {
        return std::unique_ptr<NodeExecutor>(
            new NodeExecutor{static_cast<F&&>(f), nullptr});
    }
};

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
            this->next = NodeExecutor::create(static_cast<F&&>(f));
            this->end_next = this->next.get();
        }
        else {
            this->end_next->next = NodeExecutor::create(static_cast<F&&>(f));
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
        this->on_exit = &::propagate_exit;
    }

protected:
    // TODO inner to TopExecutor
    R _exec_next(TopExecutor& top);
    R _exec(TopExecutor& top);
    R _exec_exit(TopExecutor& top, R r);
    R _exec_group_exit(TopExecutor& top, R r);
    bool has_group() const noexcept { return bool(this->group); }

private:
    std::unique_ptr<NodeExecutor> next;
    NodeExecutor* end_next = nullptr;
    std::unique_ptr<GroupExecutor> group;
    std::function<R(ContextError, ExitR er)> on_exit;
};

namespace jln {
namespace detail
{
template<class... Ts>
struct GroupExecutorWithValuesImpl : GroupExecutor, private tuple<Ts...>
{
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
    template<class... Us>
    GroupExecutorWithValuesImpl(Us&&... xs)
    noexcept(noexcept(tuple<Ts...>{static_cast<Us&&>(xs)...}))
      : tuple<Ts...>{static_cast<Us&&>(xs)...}
    {}
    REDEMPTION_DIAGNOSTIC_POP

    template<class F>
    void then(F&& f)
    {
        this->GroupExecutor::then([f = static_cast<F&&>(f), this](Context ctx) -> R{
            return this->invoke(f, ctx);
        });
    }

    template<class F>
    void set_on_exit(F&& f)
    {
        this->GroupExecutor::set_on_exit([f = static_cast<F&&>(f), this](
            ContextError ctx, ExitR er) -> R
        {
            return this->invoke(f, ctx, er);
        });
    }

private:
    friend Context;
};

template<>
struct GroupExecutorWithValuesImpl<> : GroupExecutor
{};
}
template<class... Ts>
using GroupExecutorWithValues = detail::GroupExecutorWithValuesImpl<
    detail::decay_and_strip_t<Ts>...>;
}

using jln::GroupExecutorWithValues;


struct TopExecutor : GroupExecutor
{
    TopExecutor() = default;

    bool exec()
    {
        try {
            return this->_exec(*this) == R::NeedMoreData;
        }
        catch (Error const& e) {
            this->error = e;
            if (this->has_group()) {
                return this->_exec_group_exit(*this, R::Exception) == R::NeedMoreData;
            }
            else {
                return this->_exec_exit(*this, R::Exception) == R::NeedMoreData;
            }
        }
    }

    Error error = Error(NO_ERROR);
};

R GroupExecutor::_exec_exit(TopExecutor& top, R r)
{
    switch (R re = this->on_exit(
        ContextError{top},
        ExitR{static_cast<ExitR::Status>(r), top.error}
    )) {
        case R::ExitSuccess:
        case R::Exception:
        case R::Terminate:
        case R::ExitError:
            return re;
        case R::NeedMoreData:
        case R::Next:
            return bool(this->next) ? R::NeedMoreData : r;
        case R::CreateGroup:
        case R::Substitute:
            return R::NeedMoreData;
    }
    REDEMPTION_UNREACHABLE();
}

R GroupExecutor::_exec_group_exit(TopExecutor& top, R r)
{
    do {
        R const re = this->group->on_exit(
            ContextError{top},
            ExitR{static_cast<ExitR::Status>(r), top.error});
        this->group = std::move(this->group->group);
        switch (re) {
            case R::ExitSuccess:
                if (!this->group && this->next) {
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
                return bool(this->next) ? R::NeedMoreData : r;
            case R::CreateGroup:
            case R::Substitute:
                return R::NeedMoreData;
        }
    } while (this->group);
    return r;
}

R GroupExecutor::_exec_next(TopExecutor& top)
{
    R const r = this->next->f(Context{top, *this->next});
    switch (r) {
        case R::Terminate:
        case R::Exception:
        case R::ExitError:
        case R::ExitSuccess:
        case R::NeedMoreData:
            return r;
        case R::Next:
            this->next = std::move(this->next->next);
            return (bool(this->next) ? R::NeedMoreData : R::ExitSuccess);
        case R::CreateGroup:
            this->next = std::move(this->next->next);
            return R::NeedMoreData;
        case R::Substitute:
            return R::NeedMoreData;
    }
    REDEMPTION_UNREACHABLE();
}

R GroupExecutor::_exec(TopExecutor& top)
{
    if (this->group) {
        R const r = this->group->_exec_next(top);
        switch (r) {
            case R::Exception:
            case R::Terminate:
            case R::ExitError:
            case R::ExitSuccess:
                return this->_exec_group_exit(top, r);
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
        assert(this->next);
        R const r = this->_exec_next(top);
        switch (r) {
            case R::Exception:
            case R::Terminate:
            case R::ExitError:
            case R::ExitSuccess:
                return this->_exec_exit(top, r);
            case R::NeedMoreData:
            case R::Next:
            case R::Substitute:
            case R::CreateGroup:
                return r;
        }
        REDEMPTION_UNREACHABLE();
    }
}


template<bool setError, int stateInit, class Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::GroupExecutorBuilderImpl(
    TopExecutor& top, std::unique_ptr<Group>&& g) noexcept
: top(top)
, g(std::move(g))
{}

template<bool setError, int stateInit, class Group>
template<class F>
auto detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::then(F&& f)
{
    this->g->then(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<setError, (stateInit == 1 ? stateInit : stateInit+1), Group>{
        this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group>
template<class F>
detail::GroupExecutorBuilderImpl<1, stateInit, Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::on_exit(F&& f)
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_on_exit(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<1, stateInit, Group>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group>
detail::GroupExecutorBuilderImpl<1, stateInit, Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::propagate_exit()
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_propagate_exit();
    return GroupExecutorBuilderImpl<1, stateInit, Group>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::operator R()
{
    static_assert(stateInit >= 1, "empty group");
    static_assert(setError, "missing builder.on_error(f) or builder.propagate_exit()");
    this->top.add_group_executor(std::move(this->g));
    return R::CreateGroup;
}


template<class... Xs>
#ifdef IN_IDE_PARSER
detail::GroupExecutorBuilder_Concept Context::create_sub_executor(Xs&&...)
#else
auto Context::create_sub_executor(Xs&&... xs)
#endif
{
    using G = GroupExecutorWithValues<Xs...>;
    return detail::GroupExecutorBuilder<G>{
        this->top, std::make_unique<G>(static_cast<Xs&&>(xs)...)};
}

R Context::exception(Error const& e) noexcept
{
    this->top.error = e;
    return R::Exception;
}

template<class F>
R Context::replace_action(F&& f)
{
    this->current.f = static_cast<F&&>(f);
    return R::Substitute;
}


#include <iostream>

int main()
{
    TopExecutor e;
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
