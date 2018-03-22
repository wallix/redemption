#include <functional>
#include <memory>
#include <cassert>

#include <iostream>

enum class R
{
    Next,
    Group,
    ExitSuccess,
    ExitError,
    NeedMoreData,
};


struct Event
{
    std::function<R()> f;
    std::unique_ptr<Event> next;
    Event* end_next = nullptr;
    std::unique_ptr<Event> group;

    Event() = default;

    template<class F>
    void add(F f)
    {
        if (!this->end_next) {
            this->end_next = this;
        }
        this->end_next->next = std::make_unique<Event>();
        this->end_next = this->end_next->next.get();
        this->end_next->f = f;
    }

    void sub(Event e)
    {
        assert(bool(e.next));
        auto p = std::make_unique<Event>(std::move(e));
        p->group = std::move(this->group);
        this->group = std::move(p);
    }

    R _exec_next()
    {
        std::cout << "n";
        R r = this->next->f();
        switch (r) {
            case R::ExitError:
            case R::ExitSuccess:
            case R::NeedMoreData:
                break;
            case R::Next:
                this->next = std::move(this->next->next);
                r = (bool(this->next) ? R::NeedMoreData : R::ExitSuccess);
                break;
            case R::Group:
                this->next = std::move(this->next->next);
                r = R::NeedMoreData;
                break;
        }
        return r;
    }

    R _exec()
    {
        if (this->group) {
            std::cout << "g";
            R r = this->group->_exec_next();
            switch (r) {
                case R::ExitError:
                case R::ExitSuccess:
                    this->group = std::move(this->group->group);
                    while (this->group && !this->group->next) {
                        this->group = std::move(this->group->group);
                    }
                    r = (bool(this->next) ? R::NeedMoreData : R::Next);
                    break;
                case R::NeedMoreData:
                    break;
                case R::Next:
                case R::Group:
                    assert(false);
            }
            return r;
        }
        else if (this->next) {
            return this->_exec_next();
        }
        else {
            std::cout << "f";
            assert(false);
            return this->f();
        }
    }

    bool exec()
    {
        return this->_exec() == R::NeedMoreData;
    }
};

#include <iostream>

int main()
{
    Event e;
    e.add([]{std::cout << "1\n"; return R::Next; });
    e.add([&]{
        std::cout << "2\n";
        {
            Event sub;
            sub.add([]{std::cout << "2.1\n"; return R::Next; });
            sub.add([]{std::cout << "2.2\n"; return R::Next; });
            e.sub(std::move(sub));
        }
        return R::Group;
    });
    e.add([&]{
        std::cout << "3\n";
        {
            Event sub;
            sub.add([&]{
                std::cout << "3.1\n";
                {
                    Event sub;
                    sub.add([]{std::cout << "3.1.1\n"; return R::Next; });
                    sub.add([]{std::cout << "3.1.2\n"; return R::Next; });
                    e.sub(std::move(sub));
                }
                return R::Group;
            });
            e.sub(std::move(sub));
        }
        return R::Group;
    });
    e.add([&]{
        std::cout << "4\n";
        {
            Event sub;
            sub.add([]{std::cout << "4.1\n"; return R::Next; });
            sub.add([]{std::cout << "4.2\n"; return R::ExitSuccess; });
            sub.add([]{std::cout << "4.3\n"; return R::Next; });
            sub.add([]{std::cout << "4.4\n"; return R::Next; });
            e.sub(std::move(sub));
        }
        return R::Group;
    });
    e.add([]{std::cout << "5\n"; return R::Next; });

    for (int i = 0; i < 22; ++i) {
        std::cout << ".";
        if (!e.exec()) {
            break;
        }
    }
}
