#include <functional>
#include <memory>
#include <cassert>
#include <exception>
#include "cxx/diagnostic.hpp"
#include "utils/executor.hpp"

#include <iostream>


enum class [[nodiscard]] R : char
{
    Next,
    CreateGroup,
    ExitSuccess,
    ExitError,
    NeedMoreData,
    Terminate,
    Substitute,
};

struct ExitR {
    enum class Status : char {
        Error,
        Success,
        Exception,
    };

    Status status;
    std::exception_ptr eptr;
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
        GroupExecutorBuilderImpl<1, stateInit, Group> on_error(F&& f);
        GroupExecutorBuilderImpl<1, stateInit, Group> propagate_error();

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
        template<class F> GroupExecutorBuilder_Concept on_error(F) { return *this; }
        GroupExecutorBuilder_Concept propagate_error();

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

inline R propagate_error(Context, ExitR) noexcept
{
    return R::ExitError;
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
    std::unique_ptr<NodeExecutor> next;
    NodeExecutor* end_next = nullptr;
    std::unique_ptr<GroupExecutor> group;
    std::function<R(Context, ExitR er)> on_error;

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
    void set_on_error(F&& f)
    {
        this->on_error = static_cast<F&&>(f);
    }

    void propagate_error() noexcept
    {
        this->on_error = &::propagate_error;
    }

    bool exec(TopExecutor& top)
    {
        return this->_exec(top) == R::NeedMoreData;
    }

protected:
    R _exec_next(TopExecutor& top)
    {
        std::cout << "n";
        R r = this->next->f(Context{top, *this->next});
        switch (r) {
            case R::Terminate:
            case R::ExitError:
            case R::ExitSuccess:
            case R::NeedMoreData:
                break;
            case R::Next:
                this->next = std::move(this->next->next);
                r = (bool(this->next) ? R::NeedMoreData : R::ExitSuccess);
                break;
            case R::CreateGroup:
                this->next = std::move(this->next->next);
                r = R::NeedMoreData;
                break;
            case R::Substitute:
                r = R::NeedMoreData;
                break;
        }
        return r;
    }

    R _exec(TopExecutor& top)
    {
        if (this->group) {
            std::cout << "g";
            R r = this->group->_exec_next(top);
            switch (r) {
                case R::Terminate:
                case R::ExitError:
                case R::ExitSuccess:
                    this->group = std::move(this->group->group);
                    while (this->group && !this->group->next) {
                        this->group = std::move(this->group->group);
                    }
                    if (bool(this->next)) {
                        r = R::NeedMoreData;
                    }
                    break;
                case R::NeedMoreData:
                    break;
                case R::Next:
                case R::Substitute:
                case R::CreateGroup:
                    assert(false);
            }
            return r;
        }
        else {
            assert(this->next);
            return this->_exec_next(top);
        }
    }
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
            return this->invoke_fix(f, ctx);
        });
    }

    template<class F>
    void set_on_error(F&& f)
    {
        this->GroupExecutor::set_on_error([f = static_cast<F&&>(f), this](
            Context ctx, ExitR er) -> R
        {
            return this->invoke_fix(f, ctx, er);
        });
    }

private:
    friend Context;
};
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
        return this->GroupExecutor::exec(*this);
    }
};


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
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::on_error(F&& f)
{
    static_assert(!setError, "on_error or propagate_error is already used");
    this->g->set_on_error(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<1, stateInit, Group>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group>
detail::GroupExecutorBuilderImpl<1, stateInit, Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::propagate_error()
{
    static_assert(!setError, "on_error or propagate_error is already used");
    this->g->propagate_error();
    return GroupExecutorBuilderImpl<1, stateInit, Group>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit, class Group>
detail::GroupExecutorBuilderImpl<setError, stateInit, Group>::operator R()
{
    static_assert(stateInit >= 1, "empty group");
    static_assert(setError, "missing builder.on_error(f) or builder.propagate_error()");
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
    e.then([](Context ctx){std::cout << "1\n"; return ctx.next(); });
    e.then([](Context ctx){
        std::cout << "2\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "2.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "2.2\n"; return ctx.next(); })
        .propagate_error();

    });
    e.then([](Context ctx){
        std::cout << "3\n";
        return ctx.create_sub_executor(1)
        .then([](Context ctx, int i){
            std::cout << "3.1  " << i << "\n";
            return ctx.create_sub_executor()
            .then([](Context ctx){std::cout << "3.1.1\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "3.1.2\n"; return ctx.next(); })
            .propagate_error();
        })
        .propagate_error();
    });
    e.then([&](Context ctx){
        std::cout << "4\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "4.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.2\n"; return ctx.exit_on_success(); })
        .then([](Context ctx){std::cout << "4.3\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.4\n"; return ctx.next(); })
        .propagate_error();
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
