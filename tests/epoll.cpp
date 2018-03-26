#include <functional>
#include <memory>
#include <cassert>

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

struct TopEvent;
struct EventBase;
struct GroupEvent;

enum class ExitStatus : bool {
    Error,
    Success,
};

class [[nodiscard]] SubExecutorBuilder
{
    struct D
    {
        D(TopEvent& top);

        TopEvent& top;
        std::unique_ptr<GroupEvent> sub_executor;
    };

    struct [[nodiscard]] InitMore1
    {
        InitMore1(D d)
        : d(std::move(d))
        {}

        operator R ();

        template<class F>
        InitMore1 then(F f);

    private:
        D d;
    };

    struct [[nodiscard]] Init1
    {
        Init1(D d)
        : d(std::move(d))
        {}

        operator R ();

        template<class F>
        InitMore1 then(F f);

    private:
        D d;
    };

public:
    explicit SubExecutorBuilder(TopEvent& top);

    template<class F>
    Init1 then(F f);

private:
    D d;
};

struct Context
{
    SubExecutorBuilder create_sub_executor();

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
    friend GroupEvent;

    Context(TopEvent& top, EventBase& current)
    : top(top)
    , current(current)
    {}

    TopEvent& top;
    EventBase& current;
};

struct EventBase
{
    std::function<R(Context)> f;
    std::unique_ptr<EventBase> next;
    EventBase* end_next = nullptr;

    template<class F>
    void then(F f)
    {
        if (!this->end_next) {
            this->end_next = this;
            if (this->f) {
                this->end_next->next = std::make_unique<EventBase>();
                this->end_next = this->end_next->next.get();
                this->end_next->f = this->f;
            }
        }
        this->end_next->next = std::make_unique<EventBase>();
        this->end_next = this->end_next->next.get();
        this->end_next->f = f;
    }

    template<class F2>
    void replace(F2 f)
    {
        assert(this->f);
        this->f = f;
    }
};

struct GroupEvent : EventBase
{
    std::unique_ptr<GroupEvent> group;

    GroupEvent() = default;
    GroupEvent(EventBase&& e) noexcept
      : EventBase(std::move(e))
    {}

    void add_sub_executor(std::unique_ptr<GroupEvent> g)
    {
        assert(bool(g->next));
        g->group = std::move(this->group);
        this->group = std::move(g);
    }

    R exec_f(TopEvent& top, EventBase& e)
    {
        return e.f(Context{top, e});
    }

    R _exec_next(TopEvent& top)
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

    R _exec(TopEvent& top)
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
        else if (this->next) {
            return this->_exec_next(top);
        }
        else {
            std::cout << "f";
            R r = this->exec_f(top, *this);
            switch (r) {
                case R::Terminate:
                case R::ExitError:
                case R::ExitSuccess:
                case R::NeedMoreData:
                    break;
                case R::Substitute:
                    r = R::NeedMoreData;
                    break;
                case R::CreateGroup:
                    this->f = nullptr;
                    r = R::NeedMoreData;
                    break;
                case R::Next:
                    r = R::ExitSuccess;
                    break;
            }
            return r;
        }
    }

    bool exec(TopEvent& top)
    {
        return this->_exec(top) == R::NeedMoreData;
    }
};

struct TopEvent : GroupEvent
{
    TopEvent() = default;
    TopEvent(EventBase&& e) noexcept
      : GroupEvent(std::move(e))
    {}

    bool exec()
    {
        return GroupEvent::exec(*this);
    }
};


SubExecutorBuilder::D::D(TopEvent& top)
: top(top)
, sub_executor(std::make_unique<GroupEvent>())
{}

SubExecutorBuilder::SubExecutorBuilder(TopEvent& top)
: d(top)
{}

template<class F>
SubExecutorBuilder::Init1 SubExecutorBuilder::then(F f)
{
    this->d.sub_executor->then(f);
    return {std::move(this->d)};
}

SubExecutorBuilder::Init1::operator R()
{
    // TODO static_assert prefered a no group context
    this->d.top.add_sub_executor(std::move(this->d.sub_executor));
    return R::CreateGroup;
}

template<class F>
SubExecutorBuilder::InitMore1 SubExecutorBuilder::Init1::then(F f)
{
    this->d.sub_executor->then(f);
    return {std::move(this->d)};
}

SubExecutorBuilder::InitMore1::operator R()
{
    this->d.top.add_sub_executor(std::move(this->d.sub_executor));
    return R::CreateGroup;
}

template<class F>
SubExecutorBuilder::InitMore1 SubExecutorBuilder::InitMore1::then(F f)
{
    this->d.sub_executor->then(f);
    return {std::move(this->d)};
}


SubExecutorBuilder Context::create_sub_executor()
{
    return SubExecutorBuilder(this->top);
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
    TopEvent e;
    e.then([](Context ctx){std::cout << "1\n"; return ctx.next(); });
    e.then([](Context ctx){
        std::cout << "2\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "2.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "2.2\n"; return ctx.next(); });

    });
    e.then([](Context ctx){
        std::cout << "3\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){
            std::cout << "3.1\n";
            return ctx.create_sub_executor()
            .then([](Context ctx){std::cout << "3.1.1\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "3.1.2\n"; return ctx.next(); });
        });
    });
    e.then([&](Context ctx){
        std::cout << "4\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "4.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.2\n"; return ctx.exit_on_success(); })
        .then([](Context ctx){std::cout << "4.3\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.4\n"; return ctx.next(); });
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
