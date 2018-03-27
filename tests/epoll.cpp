#include <functional>
#include <memory>
#include <cassert>
#include <exception>

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
    template<bool setError, int stateInit>
    struct [[nodiscard]] GroupExecutorBuilder
    {
        explicit GroupExecutorBuilder(TopExecutor& top) noexcept;
        explicit GroupExecutorBuilder(TopExecutor& top, std::unique_ptr<GroupExecutor>&& g) noexcept;

        template<class F>
        auto then(F f);

        template<class F>
        GroupExecutorBuilder<1, stateInit> on_error(F f);
        GroupExecutorBuilder<1, stateInit> propagate_error();

        operator R ();

    private:
        TopExecutor& top;
        std::unique_ptr<GroupExecutor> g;
    };

#ifdef IN_IDE_PARSER
    struct [[nodiscard]] GroupExecutorBuilder_Concept
    {
        explicit GroupExecutorBuilder_Concept(TopExecutor&) noexcept;

        template<class F> GroupExecutorBuilder_Concept then(F) { return *this; }

        template<class F>
        GroupExecutorBuilder_Concept on_error(F) { return *this; }
        GroupExecutorBuilder_Concept propagate_error() { return *this; }

        operator R ();
    };
#endif
}

struct Context
{
#ifdef IN_IDE_PARSER
    detail::GroupExecutorBuilder_Concept create_sub_executor() noexcept;
#else
    detail::GroupExecutorBuilder<0, 0> create_sub_executor() noexcept
    {
        return detail::GroupExecutorBuilder<0, 0>(this->top);
    }
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
    R replace_action(F f);

private:
    friend GroupExecutor;

    Context(TopExecutor& top, NodeExecutor& current)
    : top(top)
    , current(current)
    {}

    TopExecutor& top;
    NodeExecutor& current;
};

inline R propagate_error(Context, ExitR)
{
    return R::ExitError;
}

struct NodeExecutor
{
    std::function<R(Context)> f;
    std::unique_ptr<NodeExecutor> next;
};

struct GroupExecutor
{
    std::unique_ptr<NodeExecutor> next;
    NodeExecutor* end_next = nullptr;
    std::unique_ptr<GroupExecutor> group;
    std::function<R(Context, ExitR er)> on_error;

    GroupExecutor() = default;

    template<class F>
    void then(F f)
    {
        if (!this->end_next) {
            this->next = std::make_unique<NodeExecutor>();
            this->end_next = this->next.get();
        }
        else {
            this->end_next->next = std::make_unique<NodeExecutor>();
            this->end_next = this->end_next->next.get();
        }
        this->end_next->f = f;
    }

    template<class F>
    void set_on_error(F f)
    {
        this->on_error = f;
    }

    void add_group_executor(std::unique_ptr<GroupExecutor> g)
    {
        assert(bool(g->next));
        g->group = std::move(this->group);
        this->group = std::move(g);
    }

    R exec_f(TopExecutor& top, NodeExecutor& e)
    {
        return e.f(Context{top, e});
    }

    R _exec_next(TopExecutor& top)
    {
        std::cout << "n";
        R r = this->exec_f(top, *this->next);
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

    bool exec(TopExecutor& top)
    {
        return this->_exec(top) == R::NeedMoreData;
    }
};

struct TopExecutor : GroupExecutor
{
    TopExecutor() = default;

    bool exec()
    {
        return GroupExecutor::exec(*this);
    }
};


template<bool setError, int stateInit>
detail::GroupExecutorBuilder<setError, stateInit>::GroupExecutorBuilder(
    TopExecutor& top) noexcept
: GroupExecutorBuilder(top, std::make_unique<GroupExecutor>())
{}

template<bool setError, int stateInit>
detail::GroupExecutorBuilder<setError, stateInit>::GroupExecutorBuilder(
    TopExecutor& top, std::unique_ptr<GroupExecutor>&& g) noexcept
: top(top)
, g(std::move(g))
{}

template<bool setError, int stateInit>
template<class F>
auto detail::GroupExecutorBuilder<setError, stateInit>::then(F f)
{
    this->g->then(f);
    return GroupExecutorBuilder<setError, (stateInit == 1 ? stateInit : stateInit+1)>{
        this->top, std::move(this->g)};
}

template<bool setError, int stateInit>
template<class F>
detail::GroupExecutorBuilder<1, stateInit>
detail::GroupExecutorBuilder<setError, stateInit>::on_error(F f)
{
    static_assert(!setError, "on_error is already used");
    this->g->set_on_error(f);
    return GroupExecutorBuilder<1, stateInit>{this->top, std::move(this->g)};
}

template<bool setError, int stateInit>
detail::GroupExecutorBuilder<1, stateInit>
detail::GroupExecutorBuilder<setError, stateInit>::propagate_error()
{
    return this->on_error(&::propagate_error);
}

template<bool setError, int stateInit>
detail::GroupExecutorBuilder<setError, stateInit>::operator R()
{
    static_assert(stateInit >= 1, "empty group");
    static_assert(setError, "missing builder.on_error(f) or builder.propagate_error()");
    this->top.add_group_executor(std::move(this->g));
    return R::CreateGroup;
}


template<class F>
R Context::replace_action(F f)
{
    this->current.f = f;
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
        return ctx.create_sub_executor()
        .then([](Context ctx){
            std::cout << "3.1\n";
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
