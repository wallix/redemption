#include <functional>
#include <memory>
#include <cassert>

#include <iostream>

enum class R
{
    Next,
    CreateGroup,
    ExitSuccess,
    ExitError,
    NeedMoreData,
    Terminate,
    Substitude,
};

struct TopEvent;
struct EventBase;

struct Context
{
    TopEvent& top;
    EventBase& current;
};

struct EventBase
{
    std::function<R(Context)> f;
    std::unique_ptr<EventBase> next;
    EventBase* end_next = nullptr;

    template<class F>
    void add(F f)
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

    void sub(EventBase e)
    {
        assert(bool(e.next));
        auto p = std::make_unique<GroupEvent>(std::move(e));
        p->group = std::move(this->group);
        this->group = std::move(p);
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
            case R::Substitude:
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
                case R::Substitude:
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
                case R::Substitude:
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

#include <iostream>

int main()
{
    TopEvent e;
    e.add([](auto /*ctx*/){std::cout << "1\n"; return R::Next; });
    e.add([](auto ctx){
        std::cout << "2\n";
        {
            EventBase sub;
            sub.add([](auto /*ctx*/){std::cout << "2.1\n"; return R::Next; });
            sub.add([](auto /*ctx*/){std::cout << "2.2\n"; return R::Next; });
            ctx.top.sub(std::move(sub));
        }
        return R::CreateGroup;
    });
    e.add([](auto ctx){
        std::cout << "3\n";
        {
            EventBase sub;
            sub.add([](auto ctx){
                std::cout << "3.1\n";
                {
                    EventBase sub;
                    sub.add([](auto /*ctx*/){std::cout << "3.1.1\n"; return R::Next; });
                    sub.add([](auto /*ctx*/){std::cout << "3.1.2\n"; return R::Next; });
                    ctx.top.sub(std::move(sub));
                }
                return R::CreateGroup;
            });
            ctx.top.sub(std::move(sub));
        }
        return R::CreateGroup;
    });
    e.add([&](auto ctx){
        std::cout << "4\n";
        {
            EventBase sub;
            sub.add([](auto /*ctx*/){std::cout << "4.1\n"; return R::Next; });
            sub.add([](auto /*ctx*/){std::cout << "4.2\n"; return R::ExitSuccess; });
            sub.add([](auto /*ctx*/){std::cout << "4.3\n"; return R::Next; });
            sub.add([](auto /*ctx*/){std::cout << "4.4\n"; return R::Next; });
            ctx.top.sub(std::move(sub));
        }
        return R::CreateGroup;
    });
    e.add([](auto ctx){
        std::cout << "5\n";
        ctx.current.replace([](auto /*ctx*/){
            std::cout << "5bis\n";
            return R::Next;
        });
        return R::Substitude;
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
